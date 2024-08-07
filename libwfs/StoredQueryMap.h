#pragma once

#include "FileContentChecker.h"
#include "HandlerFactorySummary.h"
#include "PluginImpl.h"
#include "StoredQuery.h"
#include "StoredQueryHandlerBase.h"
#include <condition_variable>
#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <boost/thread/shared_mutex.hpp>
#include <json/json.h>
#include <spine/Reactor.h>
#include <macgyver/DirectoryMonitor.h>
#include <macgyver/AsyncTaskGroup.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
struct HandlerFactorySummary;
class PluginImpl;
class StoredQueryConfig;
class StoredQueryHandlerBase;

/**
 *  @brief A C++ class for holding information about available WFS stored queries
 *         and mapping query names to actual objects which handles the queries.
 */
class StoredQueryMap final
  : private FileContentChecker
{
 public:
  StoredQueryMap(SmartMet::Spine::Reactor* theReactor, PluginImpl& plugin_impl);

  ~StoredQueryMap() override;

  void shutdown();

  void set_background_init(bool value);

  void add_config_dir(const std::filesystem::path& config_dir,
		      const std::filesystem::path& template_dir);

  void wait_for_init();

  std::vector<std::string> get_handler_names() const;

  std::shared_ptr<StoredQueryHandlerBase> get_handler_by_name(const std::string name) const;

  virtual std::vector<std::string> get_return_type_names() const;

  bool is_reload_required(bool reset = false);

  std::shared_ptr<const HandlerFactorySummary> get_handler_factory_summary() const;

  bool use_case_sensitive_params() const;

 private:
  void add_handler(std::shared_ptr<StoredQueryHandlerBase> handler);

    void add_handler(std::shared_ptr<StoredQueryConfig> sqh_config,
                   const std::filesystem::path& template_dir);

  void on_config_change(Fmi::DirectoryMonitor::Watcher watcher,
			const std::filesystem::path& path,
			const boost::regex& pattern,
			const Fmi::DirectoryMonitor::Status& status);

  void on_config_error(Fmi::DirectoryMonitor::Watcher watcher,
		       const std::filesystem::path& path,
		       const boost::regex& pattern,
		       const std::string& message);

  std::shared_ptr<const StoredQueryHandlerBase>
  get_handler_by_file_name(const std::string& config_file_name) const;

  std::shared_ptr<const StoredQueryConfig>
  get_query_config_by_file_name(const std::string& name) const;

  bool should_be_ignored(const StoredQueryConfig& config) const;

  std::optional<std::string> get_ignore_reason(const StoredQueryConfig& config) const;

  void handle_query_remove(const std::string& config_file_name);

  void handle_query_add(const std::string& config_file_name,
			const std::filesystem::path& template_dir,
			bool initial_update,
			bool silent_duplicate);

  void handle_query_modify(const std::string& config_file_name,
			   const std::filesystem::path& template_dir);

  void handle_query_ignore(const StoredQueryConfig& config, bool initial_update);

  void request_reload(const std::string& reason);

  void enqueue_query_add(std::shared_ptr<StoredQueryConfig> sqh_config,
			 const std::filesystem::path& template_dir,
			 bool initial_update);

  std::shared_ptr<StoredQueryHandlerBase> get_handler_by_name_nothrow(const std::string name) const;

  void directory_monitor_thread_proc();

  void notify_failed();

 private:
  bool background_init{false};
  std::atomic<bool> reload_required;
  std::atomic<bool> loading_started;
  std::atomic<bool> load_failed;
  mutable boost::shared_mutex mutex;
  mutable std::mutex mutex2;
  std::condition_variable cond;
  SmartMet::Spine::Reactor* theReactor;
  PluginImpl& plugin_impl;
  std::unique_ptr<Fmi::AsyncTaskGroup> init_tasks;
  std::map<std::string, std::shared_ptr<StoredQueryHandlerBase> > handler_map;
  std::set<std::string> duplicate;

  struct ConfigDirInfo {
    std::filesystem::path config_dir;
    std::filesystem::path template_dir;
    Fmi::DirectoryMonitor::Watcher watcher;
    int num_updates;
  };

  std::map<Fmi::DirectoryMonitor::Watcher, ConfigDirInfo> config_dirs;
  Fmi::DirectoryMonitor directory_monitor;
  std::thread directory_monitor_thread;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
