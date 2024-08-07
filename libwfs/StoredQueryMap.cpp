#include "StoredQueryMap.h"
#include "HandlerFactorySummary.h"
#include "StoredQueryHandlerFactoryDef.h"
#include <boost/algorithm/string.hpp>
#include <boost/bind/bind.hpp>
#include <boost/chrono.hpp>
#include <filesystem>
#include <spine/Convenience.h>
#include <macgyver/Base64.h>
#include <macgyver/Exception.h>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <openssl/sha.h>

namespace ba = boost::algorithm;
namespace bw = SmartMet::Plugin::WFS;
namespace fs = std::filesystem;
namespace ph = boost::placeholders;

bw::StoredQueryMap::StoredQueryMap(SmartMet::Spine::Reactor* theReactor, PluginImpl& plugin_impl)
  : bw::FileContentChecker(131072, (plugin_impl.get_debug_level() >= 2))
  , 
   reload_required(false)
  , loading_started(false)
  , load_failed(false)
  , theReactor(theReactor)
  , plugin_impl(plugin_impl)
{
}

bw::StoredQueryMap::~StoredQueryMap()
{
  if (directory_monitor_thread.joinable()) {
    directory_monitor.stop();
    directory_monitor_thread.join();
  }
}

void bw::StoredQueryMap::shutdown()
{
  if (init_tasks) {
    init_tasks->stop();
  }
}

void bw::StoredQueryMap::set_background_init(bool value)
{
  background_init = value;
  if (background_init) {
    init_tasks.reset(new Fmi::AsyncTaskGroup(10));
    init_tasks->on_task_error(
        [this] (const std::string& name) -> void
        {
            try {
                throw;
            } catch (...) {
                Fmi::Exception error = Fmi::Exception::SquashTrace(BCP, "Operation Failed");
                error.addDetail("Stored query loading failed - task name " + name);
                std::cout << error << std::endl;
                // This is only checked in wait_for_init(). Later changes fave no effect
                load_failed = true;
            }
        });
  } else {
    init_tasks.reset();
  }
}

void bw::StoredQueryMap::add_config_dir(const std::filesystem::path& config_dir,
					const std::filesystem::path& template_dir)
{
  ConfigDirInfo ci;
  ci.num_updates = 0;
  ci.config_dir = config_dir;
  ci.template_dir = template_dir;
  ci.watcher = directory_monitor
    .watch(config_dir,
	   boost::bind(&bw::StoredQueryMap::on_config_change, this,  ph::_1,  ph::_2,  ph::_3, ph::_4),
	   boost::bind(&bw::StoredQueryMap::on_config_error, this,  ph::_1,  ph::_2,  ph::_3, ph::_4),
	   5, Fmi::DirectoryMonitor::CREATE | Fmi::DirectoryMonitor::DELETE | Fmi::DirectoryMonitor::MODIFY);
  if (config_dirs.empty()) {
    std::thread tmp(std::bind(&bw::StoredQueryMap::directory_monitor_thread_proc, this));
    directory_monitor_thread.swap(tmp);
  }

  boost::unique_lock<boost::shared_mutex> lock(mutex);
  config_dirs[ci.watcher] = ci;
}

void bw::StoredQueryMap::wait_for_init()
{
  const auto done = [this]() -> bool
                             {
                                 return Spine::Reactor::isShuttingDown()
                                     or directory_monitor.ready()
                                     or load_failed;
                             };
  do {
    std::time_t start = std::time(nullptr);
    std::unique_lock<std::mutex> lock(mutex2);
    while (not cond.wait_for(lock, std::chrono::seconds(1), done)) {
        if (not loading_started and (std::time(nullptr) - start > 180)) {
            throw Fmi::Exception::Trace(BCP, "Timed out while waiting for stored query"
                " configuration loading to start");
      }
    }
  } while (false);

  if (init_tasks) {
    if (Spine::Reactor::isShuttingDown() or load_failed) {
      init_tasks->stop();
    }

    init_tasks->wait();
  }

  if (Spine::Reactor::isShuttingDown()) {
      // No need to say anything in this case
  } else if (load_failed) {
      throw Fmi::Exception(BCP, "Failed to load one or more stored query configuration");
  } else {
      std::cout << SmartMet::Spine::log_time_str() << ": [WFS] Initial loading of stored query configuration files finished"
                << std::endl;
  }
}

bool bw::StoredQueryMap::is_reload_required(bool reset)
{
  if (reset) {
    bool response = reload_required.exchange(false);
    if (response) {
      std::cout << SmartMet::Spine::log_time_str() << ": [WFS] Cleared reload required flag" << std::endl;
    }
    return response;
  } else {
    return reload_required;
  }
}

void bw::StoredQueryMap::add_handler(std::shared_ptr<StoredQueryHandlerBase> handler)
{
  try
  {
    boost::unique_lock<boost::shared_mutex> lock(mutex);
    const std::string name = handler->get_query_name();
    std::string lname = Fmi::ascii_tolower_copy(handler->get_query_name());

    auto find_iter = handler_map.find(lname);
    if (find_iter != handler_map.end())
    {
      // Override already defined stored query with the current one
      handler_map.erase(find_iter);
    }

    handler_map.insert(std::make_pair(lname, handler));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<bw::StoredQueryHandlerBase> bw::StoredQueryMap::get_handler_by_name(
    const std::string name) const
{
  try
  {
    const std::string& lname = Fmi::ascii_tolower_copy(name);
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    auto loc = handler_map.find(lname);
    if (loc == handler_map.end())
    {
      Fmi::Exception exception(BCP, "No handler for '" + name + "' found!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception.disableLogging();
    }
    else
    {
      return loc->second;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::string> bw::StoredQueryMap::get_return_type_names() const
{
  try
  {
    std::set<std::string> return_type_set;
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    for (auto& handler_map_item : handler_map)
    {
      // NOTE: Cannot call StoredQueryHandlerBase::get_return_type_names() here to
      //       avoid recursion as hander itself may call StoredQueryMap::get_return_type_names().
      //       One must take types from stored queries configuration instead.
      const std::vector<std::string> return_types =
          handler_map_item.second->get_config()->get_return_type_names();

      std::copy(return_types.begin(),
                return_types.end(),
                std::inserter(return_type_set, return_type_set.begin()));
    }
    return std::vector<std::string>(return_type_set.begin(), return_type_set.end());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::add_handler(StoredQueryConfig::Ptr sqh_config,
                                     const std::filesystem::path& template_dir)
{
  try
  {
    try
    {
      std::optional<std::string> sqh_template_fn;
      if (sqh_config->have_template_fn())
      {
        sqh_template_fn = (template_dir / sqh_config->get_template_fn()).string();
      }

      std::shared_ptr<StoredQueryHandlerBase> p_handler = StoredQueryHandlerFactoryDef::construct(
          sqh_config->get_constructor_name(), theReactor, sqh_config, plugin_impl, sqh_template_fn);

      add_handler(p_handler);

      if (plugin_impl.get_debug_level() < 1)
        return;

      if (not Spine::Reactor::isShuttingDown()) {
	std::ostringstream msg;
	std::string prefix;
	if (sqh_config->is_demo())
	  prefix = "DEMO ";
	if (sqh_config->is_test())
	  prefix = "TEST ";
	msg << SmartMet::Spine::log_time_str() << ": [WFS] ["
	    << prefix << "stored query ready] id='"
	    << p_handler->get_query_name() << "' config='"
	    << sqh_config->get_file_name() << "'\n";
	std::cout << msg.str() << std::flush;
      }
    }
    catch (const std::exception&)
    {
      if (not Spine::Reactor::isShuttingDown()) {
	std::ostringstream msg;
	msg << SmartMet::Spine::log_time_str()
	    << ": [WFS] [ERROR] Failed to add stored query handler. Configuration '"
	    << sqh_config->get_file_name() << "\n";
	std::cout << msg.str() << std::flush;

	throw Fmi::Exception::Trace(BCP, "Failed to add stored query handler!");
      }
    }
  }
  catch (...)
  {
    notify_failed();
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::on_config_change(Fmi::DirectoryMonitor::Watcher watcher,
					  const std::filesystem::path& path,
					  const boost::regex& pattern,
					  const Fmi::DirectoryMonitor::Status& status)
{
  try {
    assert(config_dirs.count(watcher) > 0);

    loading_started = true;

    int have_errors = 0;
    const bool initial_update = [this, &watcher]() {
        boost::shared_lock<boost::shared_mutex> lock(mutex);
        return config_dirs.at(watcher).num_updates == 0; } ();

    const auto template_dir = config_dirs.at(watcher).template_dir;

    (void)path;
    (void)pattern;

    if (directory_monitor.ready() and not plugin_impl.get_config().getEnableConfigurationPolling()) {
      // Ignore changes when initial load is done and configuration polling is disabled
      return;
    }

    Fmi::DirectoryMonitor::Status s = status;

    for (auto it = s->begin(); not reload_required and it != s->end(); ++it) {
      try {
	const fs::path& fn = it->first;
	const std::string name = fn.string();
	Fmi::DirectoryMonitor::Change change = it->second;

	if ( ((change == Fmi::DirectoryMonitor::DELETE) or fs::is_regular_file(fn))
	     and not ba::starts_with(name, ".")
	     and not ba::starts_with(name, "#")
	     and ba::ends_with(name, ".conf"))
	  {
	    switch (change) {
	    case Fmi::DirectoryMonitor::DELETE:
	      handle_query_remove(name);
	      break;

	    case Fmi::DirectoryMonitor::CREATE:
	      handle_query_add(name, template_dir, initial_update, false);
	      break;

	    case Fmi::DirectoryMonitor::MODIFY:
	      handle_query_modify(name, template_dir);
	      break;

	    default:
	      if (change & (change - 1)) { // Is any 1-bit value in variable change?
		throw Fmi::Exception::Trace(BCP, "INTERNAL ERROR: Unsupported change type "
							+ std::to_string(int(change)));
	      }
	      break;
	    }
	  }

	config_dirs.at(watcher).num_updates++;
      } catch (...) {
        if (not Spine::Reactor::isShuttingDown()) {
	  have_errors++;
	  auto err = Fmi::Exception::SquashTrace(BCP, "Operation failed!");
	  std::cout << err << std::endl;
	  if (initial_update) {
	    notify_failed();
	  }
        }
      }
    }

    if (not initial_update) {
      auto tmp = duplicate;
      for (const auto& fn : tmp) {
	try {
	  handle_query_add(fn, template_dir, initial_update, true);
	} catch (...) {
            if (not Spine::Reactor::isShuttingDown()) {
                auto err = Fmi::Exception::SquashTrace(BCP, "Operation failed!");
                std::cout << err << std::endl;
            }
	}
      }
    }

    if (have_errors) {
      std::ostringstream msg;
      msg << "Failed to process " << have_errors << " store query configuration files";
      auto err = Fmi::Exception::Trace(BCP, msg.str());
      if (not Spine::Reactor::isShuttingDown()) {
          if (initial_update) {
              load_failed = true;
              throw err;
          } else {
              std::cout << err.disableStackTraceRecursive() << std::endl;
          }
      }
    }

    std::unique_lock<std::mutex> lock(mutex2);
    cond.notify_all();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::string> bw::StoredQueryMap::get_handler_names() const
{
  try {
    std::vector<std::string> result;
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    for (const auto& item : handler_map) {
      result.push_back(item.first);
    }
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::on_config_error(Fmi::DirectoryMonitor::Watcher watcher,
					 const std::filesystem::path& path,
					 const boost::regex& pattern,
					 const std::string& message)
{
  (void)watcher;
  (void)path;
  (void)pattern;
  (void)message;
}

std::shared_ptr<const bw::StoredQueryHandlerBase>
bw::StoredQueryMap::get_handler_by_file_name(const std::string& config_file_name) const
{
  try {
    std::shared_ptr<const StoredQueryHandlerBase> result;
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    for (auto it2 = handler_map.begin(); not result and it2 != handler_map.end(); ++it2) {
      if (it2->second->get_config()->get_file_name() == config_file_name) {
	result = it2->second;
      }
    }
    return result;
  }
  catch (...) {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<const bw::StoredQueryConfig>
bw::StoredQueryMap::get_query_config_by_file_name(const std::string& config_file_name) const
{
  try {
    std::shared_ptr<const StoredQueryConfig> result;
    auto handler = get_handler_by_file_name(config_file_name);
    if (handler) {
      result = handler->get_config();
    }
    return result;
  }  catch (...) {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::handle_query_remove(const std::string& config_file_name)
{
  try {
    remove_file_hash(config_file_name);
    auto config = get_query_config_by_file_name(config_file_name);
    duplicate.erase(config_file_name);
    if (config) {
      std::ostringstream msg;
      msg << SmartMet::Spine::log_time_str() << ": [WFS] [INFO] Removing storedquery_id='"
	  << config->get_query_id() << "' (File '" << config_file_name << "' deleted)\n";
      std::cout << msg.str() << std::flush;
      boost::unique_lock<boost::shared_mutex> lock(mutex);
      handler_map.erase(Fmi::ascii_tolower_copy(config->get_query_id()));
    }
  } catch (...) {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace {
  void notify_duplicate(const bw::StoredQueryConfig& new_config, const bw::StoredQueryConfig& found_config)
  {
    std::ostringstream msg;
    msg << SmartMet::Spine::log_time_str() << ": [WFS] [WARNING] Duplicate storedquery_id '"
	<< new_config.get_query_id() << "' from file '" << new_config.get_file_name() << "' is ignored."
	<< " Earlier found in '" << found_config.get_file_name() << "'\n";
    std::cout << msg.str() << std::flush;
  }
}

bool bw::StoredQueryMap::should_be_ignored(const StoredQueryConfig& config) const
{
  const bool enable_test = plugin_impl.get_config().getEnableTestQueries();
  const bool enable_demo = plugin_impl.get_config().getEnableDemoQueries();
  return
    config.is_disabled()
    or (not enable_test and config.is_test())
    or (not enable_demo and config.is_demo());
}

std::optional<std::string>
bw::StoredQueryMap::get_ignore_reason(const StoredQueryConfig& config) const
{
  std::optional<std::string> reason;
  const bool enable_demo = plugin_impl.get_config().getEnableDemoQueries();
  if (config.is_disabled()) {
    reason = std::string("Disabled stored query");
  } else if (not enable_demo and config.is_demo()) {
    reason = std::string("Disabled DEMO stored query");
  }
  return reason;
}

void bw::StoredQueryMap::handle_query_add(const std::string& config_file_name,
					  const std::filesystem::path& template_dir,
					  bool initial_update,
					  bool silent_duplicate)
{
  try {
    if (not check_file_hash(config_file_name)) {
      return;
    }
    const int debug_level = plugin_impl.get_debug_level();
    const bool verbose = not initial_update or debug_level > 0;
    StoredQueryConfig::Ptr sqh_config(new StoredQueryConfig(config_file_name, &plugin_impl.get_config()));

    auto prev_handler = get_handler_by_name_nothrow(sqh_config->get_query_id());
    if (prev_handler) {
      if (not silent_duplicate) {
	notify_duplicate(*sqh_config, *prev_handler->get_config());
      }
      duplicate.insert(config_file_name);
    } else {
      duplicate.erase(config_file_name);
      if (should_be_ignored(*sqh_config)) {
	handle_query_ignore(*sqh_config, initial_update);
      } else {
	if (verbose and not Spine::Reactor::isShuttingDown()) {
	  std::ostringstream msg;
	  msg << SmartMet::Spine::log_time_str() << ": [WFS] Adding stored query: id='"
	      << sqh_config->get_query_id() << "' config='" << config_file_name << "'\n";
	  std::cout << msg.str() << std::flush;
	}

	enqueue_query_add(sqh_config, template_dir, initial_update);
      }
    }
  } catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::handle_query_modify(const std::string& config_file_name,
					     const std::filesystem::path& template_dir)
{
  try {
    if (not check_file_hash(config_file_name)) {
      return;
    }
    StoredQueryConfig::Ptr sqh_config(new StoredQueryConfig(config_file_name, &plugin_impl.get_config()));
    const std::string id = sqh_config->get_query_id();

    if (should_be_ignored(*sqh_config)) {
      duplicate.erase(config_file_name);
      handle_query_ignore(*sqh_config, false);
    } else {
      const auto id = sqh_config->get_query_id();
      auto ph1 = get_handler_by_file_name(config_file_name);
      auto ph2 = get_handler_by_name_nothrow(id);
      if (ph1) {
	const auto id2 = ph1->get_config()->get_query_id();
	if (ph2 and (ph1 != ph2)) {
	  request_reload("");
	} else {
	  std::ostringstream msg;
	  msg << SmartMet::Spine::log_time_str() << ": [WFS] [INFO] Adding stored query: id='"
	      << id << "' config='" << config_file_name << "'\n";
	  std::cout << msg.str() << std::flush;
	  enqueue_query_add(sqh_config, template_dir, false);
	  if (ph2 != ph1) {
	    msg.str("");
	    msg << SmartMet::Spine::log_time_str() << ": [WFS] [INFO] Removing stored query: id='" << id2 << "'\n";
	    std::cout << msg.str() << std::flush;
	    boost::unique_lock<boost::shared_mutex> lock(mutex);
	    handler_map.erase(id2);
	  }
	}
      } else {
	// May happen if previous configuration had demo or test setting enabled.
	if (ph2) {
	  // Should add: storedquery_id already present
	  request_reload("");
	} else {
	  // Should add: storedquery_id not in use
	  std::ostringstream msg;
	  msg << SmartMet::Spine::log_time_str() << ": [WFS] [INFO] Adding stored query: id='"
	      << id << "' config='" << config_file_name << "'\n";
	  std::cout << msg.str() << std::flush;
	  enqueue_query_add(sqh_config, template_dir, false);
	}
      }
    }
  } catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::handle_query_ignore(const StoredQueryConfig& sqh_config, bool initial_update)
{
  try {
    const int debug_level = plugin_impl.get_debug_level();
    const auto reason = get_ignore_reason(sqh_config);
    const auto id = sqh_config.get_query_id();
    auto prev_handler = get_handler_by_name_nothrow(sqh_config.get_query_id());
    if (prev_handler) {
      if (sqh_config.get_file_name() == prev_handler->get_config()->get_file_name()) {
	if (not Spine::Reactor::isShuttingDown()) {
	  std::ostringstream msg;
	  msg << SmartMet::Spine::log_time_str() << ": [WFS] ";
	  if (reason) {
	    msg << '[' << *reason << "] ";
	  } else {
	    msg << "[Disabled stored query] ";
	  }
	  msg << "Removing previous handler for " << sqh_config.get_query_id() << '\n';
	  std::cout << msg.str() << std::flush;
	}

	boost::unique_lock<boost::shared_mutex> lock(mutex);
	handler_map.erase(Fmi::ascii_tolower_copy(id));
      } else {
	// ID changed since last update: request reload
	request_reload("");
      }
    } else {
      if ((not initial_update or (debug_level > 0)) and reason) {
	std::ostringstream msg;
	msg << SmartMet::Spine::log_time_str() << ": [WFS] [" << *reason << ']'
	    << " id='" << sqh_config.get_query_id() << "' config='" << sqh_config.get_file_name() << "'\n";
	std::cout << msg.str() << std::flush;
      }
    }
  } catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::request_reload(const std::string& reason)
{
  std::ostringstream msg;
  msg << SmartMet::Spine::log_time_str() << ": [WFS] [INFO] Requiring WFS reload due to config change";
  if (!reason.empty()) {
    msg << " (" << reason << ')';
  }
  std::cout << msg.str() << std::endl;
  reload_required = true;
}

std::shared_ptr<bw::StoredQueryHandlerBase> bw::StoredQueryMap::get_handler_by_name_nothrow(
    const std::string name) const
{
  try
  {
    std::shared_ptr<bw::StoredQueryHandlerBase> handler;
    const std::string& lname = Fmi::ascii_tolower_copy(name);
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    auto loc = handler_map.find(lname);
    if (loc != handler_map.end()) {
     handler = loc->second;
    }
    return handler;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredQueryMap::enqueue_query_add(StoredQueryConfig::Ptr sqh_config,
					   const std::filesystem::path& template_dir,
					   bool initial_update)
{
  if (background_init and initial_update) {
    init_tasks->add(sqh_config->get_query_id(),
		    [this, sqh_config, template_dir]() { add_handler(sqh_config, template_dir); });
  } else {
    add_handler(sqh_config, template_dir);
  }
}

void bw::StoredQueryMap::directory_monitor_thread_proc()
{
  std::cout << SmartMet::Spine::log_time_str()
	    << ": [WFS] SmartMet::Plugin::WFS::StoredQueryMap::directory_monitor_proc started"
	    << std::endl;

  directory_monitor.run();

  std::cout << SmartMet::Spine::log_time_str()
	    << ": [WFS] SmartMet::Plugin::WFS::StoredQueryMap::directory_monitor_proc ended"
	    << std::endl;
}

std::shared_ptr<const bw::HandlerFactorySummary>
bw::StoredQueryMap::get_handler_factory_summary() const
{
    const std::string url_base = plugin_impl.get_config().defaultUrl();
    std::shared_ptr<HandlerFactorySummary> result = std::make_shared<HandlerFactorySummary>(url_base);
    boost::shared_lock<boost::shared_mutex> lock(mutex);

    for (const auto& map_item : handler_map) {
        std::shared_ptr<StoredQueryHandlerBase> handler_ptr = map_item.second;
        result->add_handler(*handler_ptr);
    }
    return result;
 }

bool bw::StoredQueryMap::use_case_sensitive_params() const
{
  return plugin_impl.get_config().use_case_sensitive_params();
}

void bw::StoredQueryMap::notify_failed()
{
    std::unique_lock<std::mutex> lock(mutex2);
    load_failed = true;
    cond.notify_all();
}
