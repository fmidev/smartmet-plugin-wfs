// ======================================================================
/*!
 * \brief SmartMet Wfs plugin implementation
 */
// ======================================================================

#include "WfsConst.h"
#include "WfsException.h"
#include <json/json.h>
#include <spine/Convenience.h>
#include <macgyver/Exception.h>
#include <spine/Reactor.h>
#include <spine/SmartMet.h>
#include <Plugin.h>
#include <functional>
#include <sstream>
#include <thread>
#include <boost/algorithm/string.hpp>

namespace p = std::placeholders;
namespace ba = boost::algorithm;
using namespace std::literals;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/*
 *  @brief Plugin constructor
 */
Plugin::Plugin(SmartMet::Spine::Reactor* theReactor, const char* theConfig)
    : SmartMetPlugin(),
      itsModuleName("WFS"),
      itsReactor(theReactor),
      itsConfig(theConfig),
      itsShuttingDown(false),
      itsReloading(false)
{
}

void Plugin::init()
{
  // x-www-form-urlencoded is always supported and there is no need to specify it here.
  // Specify it anyway for better readability (it does not cause performance issues)
  const std::set<std::string> content_types = {"x-www-form-urlencoded", "text/xml"};

  try
  {
    try
    {
      if (itsReactor->getRequiredAPIVersion() != SMARTMET_API_VERSION)
      {
        std::ostringstream msg;
        msg << "WFS Plugin and Server SmartMet API version mismatch"
            << " (plugin API version is " << SMARTMET_API_VERSION << ", server requires "
            << itsReactor->getRequiredAPIVersion() << ")";
        throw Fmi::Exception(BCP, msg.str());
      }

      auto itsGisEngine = itsReactor->getEngine<SmartMet::Engine::Gis::Engine>("Gis");

      std::shared_ptr<PluginImpl> new_impl(
          new PluginImpl(
              itsReactor,
              itsConfig,
              itsGisEngine->getCRSRegistry()));

      itsReactor->removeContentHandlers(this);

      const std::vector<std::string>& languages = new_impl->get_languages();
      for (const auto& language : languages)
      {
        const std::string url = new_impl->get_config().defaultUrl() + "/" + language;
        if (!itsReactor->addContentHandler(
                this,
                url,
                std::bind(&Plugin::realRequestHandler, this, p::_1, language, p::_2, p::_3),
                content_types))
        {
          std::ostringstream msg;
          msg << "Failed to register WFS content handler for language '" << language << "'";
          throw Fmi::Exception(BCP, msg.str());
        }
      }

      ensureUpdateLoopStarted();
      plugin_impl.store(new_impl);
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }

    if (!itsReactor->addContentHandler(
            this,
            plugin_impl.load()->get_config().defaultUrl(),
            std::bind(&Plugin::realRequestHandler, this, p::_1, "", p::_2, p::_3),
            content_types))
    {
      throw Fmi::Exception(
          BCP, "Failed to register WFS content handler for default language");
    }

    using AdminRequestAccess = Spine::ContentHandlerMap::AdminRequestAccess;

    itsReactor->addAdminBoolRequestHandler(
        this,
        "wfs:reload",
        AdminRequestAccess::RequiresAuthentication,
        [this](Spine::Reactor&, const Spine::HTTP::Request&) -> bool
        {
          return reload(itsConfig);
        },
        "Reloads the WFS plugin configuration");

    itsReactor->addAdminCustomRequestHandler(
        this,
        "wfs:xmlSchemaCache",
        AdminRequestAccess::Private,
        std::bind(&Plugin::adminListCacheContents, this, p::_2, p::_3),
        "Dumps the XML schema cache");

    itsReactor->addAdminCustomRequestHandler(
        this,
        "wfs:constructors",
        AdminRequestAccess::Public,
        std::bind(&Plugin::adminListConstructors, this, p::_2, p::_3),
        "Dumps the stored query constructor map");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Init failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Shutdown the plugin
 */
// ----------------------------------------------------------------------

void Plugin::shutdown()
{
  // FIXME: do shutdown in thread safe way
  stopUpdateLoop();
  auto impl = plugin_impl.load();
  if (impl) {
      impl->shutdown();
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Plugin::~Plugin()
{
  stopUpdateLoop();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the plugin name
 */
// ----------------------------------------------------------------------

const std::string& Plugin::getPluginName() const
{
  return itsModuleName;
}
// ----------------------------------------------------------------------
/*!
 * \brief Return the required version
 */
// ----------------------------------------------------------------------

int Plugin::getRequiredAPIVersion() const
{
  return SMARTMET_API_VERSION;
}

bool Plugin::reload(const char* theConfig)
{
  if (itsReloading)
  {
    return false;
  }
  else
  {
    // FIXME: Use atomic
    itsReloading = true;
    std::cout << "Plugin reload requested" << std::endl;
    try
    {
      itsConfig = theConfig;
      init();
    } catch (...) {
      throw Fmi::Exception(BCP, "Reload failed");
    }
    itsReloading = false;
    return true;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Main content handler
 */
// ----------------------------------------------------------------------

void Plugin::requestHandler(SmartMet::Spine::Reactor& theReactor,
                            const SmartMet::Spine::HTTP::Request& theRequest,
                            SmartMet::Spine::HTTP::Response& theResponse)
{
  try
  {
    auto impl = plugin_impl.load();
    std::string language = "eng";
    std::string defaultPath = impl->get_config().defaultUrl();
    std::string requestPath = theRequest.getResource();

    if ((defaultPath.length() + 2) < requestPath.length())
      language = requestPath.substr(defaultPath.length() + 1, 3);

    realRequestHandler(theReactor, language, theRequest, theResponse);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string Plugin::getRealm() const
{
  return "SmartMet WFS2 plugin";
}

void Plugin::realRequestHandler(SmartMet::Spine::Reactor& theReactor,
                                const std::string& language,
                                const SmartMet::Spine::HTTP::Request& theRequest,
                                SmartMet::Spine::HTTP::Response& theResponse)
{
  try
  {
    auto impl = plugin_impl.load();
    std::string data_source = Spine::optional_string(theRequest.getParameter("source"),
                                                     impl->get_primary_data_source());
    impl->set_data_source(data_source);
    impl->realRequestHandler(theReactor, language, theRequest, theResponse);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool Plugin::queryIsFast(const SmartMet::Spine::HTTP::Request& /* theRequest */) const
{
  // FIXME: implement test
  return false;
}

void Plugin::updateLoop()
{
  auto updateCheck = [this]() -> bool {
    std::unique_lock<std::mutex> lock(itsUpdateNotifyMutex);
    if (not itsShuttingDown)
    {
      itsUpdateNotifyCond.wait_for(lock, std::chrono::seconds(1));
    }
    return not itsShuttingDown;
  };

  try
  {
    while (updateCheck())
    {
      try
      {
        auto impl = plugin_impl.load();
        if (impl and impl->get_config().getEnableConfigurationPolling() and
            impl->is_reload_required(true))
        {
          bool ok = reload(itsConfig);
          std::cout << SmartMet::Spine::log_time_str() << ": [WFS] [INFO] Plugin reload "
                    << (ok ? "succeeded" : "failed") << std::endl;
        }
      }
      catch (...)
      {
        Fmi::Exception exception(BCP, "Stored query map update failed!", nullptr);
        exception.printError();
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Update loop failed!");
  }
}

void Plugin::ensureUpdateLoopStarted()
{
  if (not itsShuttingDown)
  {
    itsShuttingDown = false;
    // Begin the update loop if enabled
    std::unique_lock<std::mutex> lock(itsUpdateNotifyMutex);
    if (not itsUpdateLoopThread)
    {
      itsUpdateLoopThread.reset(new std::thread(std::bind(&Plugin::updateLoop, this)));
    }
  }
}

void Plugin::stopUpdateLoop()
{
  itsShuttingDown = true;
  std::unique_ptr<std::thread> tmp;
  std::unique_lock<std::mutex> lock(itsUpdateNotifyMutex);
  if (itsUpdateLoopThread)
  {
    std::swap(tmp, itsUpdateLoopThread);
    itsUpdateNotifyCond.notify_all();
  }
  lock.unlock();
  if (tmp and tmp->joinable())
  {
    tmp->join();
  }
  itsShuttingDown = false;
}

Fmi::Cache::CacheStatistics Plugin::getCacheStats() const
{
  return plugin_impl.load()->getCacheStats();
}


void Plugin::adminListConstructors(const SmartMet::Spine::HTTP::Request& theRequest,
                                   SmartMet::Spine::HTTP::Response& theResponse)
try
{
  using namespace Spine;
  using namespace Spine::HTTP;
  const auto handler = theRequest.getParameter("handler");
  const auto format = theRequest.getParameter("format");
  std::ostringstream content;
  if (format and *format == "json") {
    plugin_impl.load()->dump_constructor_map(content, handler);
    theResponse.setStatus(200);
    theResponse.setHeader("Content-type", "application/json");
  } else if (not format or *format == "HTML" or *format == "html") {
    std::string uri = theRequest.getResource();
    std::vector<std::string> params;
    params.push_back("what=wfs:constructors");
    if (format) params.push_back("format=" + urlencode(*format));
    if (handler) params.push_back("handler=" + urlencode(*handler));
    uri += "?"s + ba::join(params, "&"s);
    plugin_impl.load()->dump_constructor_map_html(content, uri, handler);
    theResponse.setStatus(200);
    theResponse.setHeader("Content-type", "text/html; charset=UTF-8");
  } else {
    std::cout << "#### BAD format \n";
    throw std::runtime_error("format " + *format + " is not supported");
  }
  theResponse.setContent(content.str());
}
catch (...)
{
  auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
  error.addParameter("Request", theRequest.getURI());
  throw error;
}


void Plugin::adminListCacheContents(const SmartMet::Spine::HTTP::Request& theRequest,
                                    SmartMet::Spine::HTTP::Response& theResponse)
try
{
  std::ostringstream content;
  plugin_impl.load()->dump_xml_schema_cache(content);
  theResponse.setStatus(200);
  theResponse.setHeader("Content-type", "application/octet-stream");
  theResponse.setContent(content.str());
}
catch (...)
{
  auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
  error.addParameter("Request", theRequest.getURI());
  throw error;
}


}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

/*
 * Server knows us through the 'SmartMetPlugin' virtual interface, which
 * the 'Plugin' class implements.
 */

extern "C" SmartMetPlugin* create(SmartMet::Spine::Reactor* them, const char* config)
{
  return new SmartMet::Plugin::WFS::Plugin(them, config);
}

extern "C" void destroy(SmartMetPlugin* us)
{
  // This will call 'Plugin::~Plugin()' since the destructor is virtual
  delete us;
}

// ======================================================================
