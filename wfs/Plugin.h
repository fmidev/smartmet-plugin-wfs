#pragma once

// ======================================================================
/*!
 * \brief SmartMet WFS plugin interface
 */
// ======================================================================

#include "Config.h"
//#include "GeoServerDB.h"
#include "PluginImpl.h"
//#include "RequestFactory.h"
//#include "StoredQueryMap.h"
//#include "XmlEnvInit.h"
//#include "XmlParser.h"
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <ctpp2/CDT.hpp>
#include <macgyver/TimedCache.h>
#include <macgyver/CacheStats.h>
#include <spine/HTTP.h>
#include <spine/HTTPAuthentication.h>
#include <spine/Reactor.h>
#include <spine/SmartMetPlugin.h>
#include <condition_variable>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class Plugin : public SmartMetPlugin,
               private Xml::EnvInit,
               private SmartMet::Spine::HTTP::Authentication
{
 public:
  Plugin(SmartMet::Spine::Reactor* theReactor, const char* theConfig);
  Plugin(const Plugin&) = delete;
  Plugin& operator = (const Plugin&) = delete;
  ~Plugin() override;

  const std::string& getPluginName() const override;
  int getRequiredAPIVersion() const override;

  bool reload(const char* theConfig);

 protected:
  void init() override;
  void shutdown() override;
  void requestHandler(SmartMet::Spine::Reactor& theReactor,
                      const SmartMet::Spine::HTTP::Request& theRequest,
                      SmartMet::Spine::HTTP::Response& theResponse) override;
  std::string getRealm() const override;

 private:
  Plugin() = delete;

  bool queryIsFast(const SmartMet::Spine::HTTP::Request& theRequest) const override;

 private:
  void realRequestHandler(SmartMet::Spine::Reactor& theReactor,
                          const std::string& language,
                          const SmartMet::Spine::HTTP::Request& theRequest,
                          SmartMet::Spine::HTTP::Response& theResponse);

  void adminHandler(SmartMet::Spine::Reactor& theReactor,
                    const SmartMet::Spine::HTTP::Request& theRequest,
                    SmartMet::Spine::HTTP::Response& theResponse);

  void updateLoop();

  void ensureUpdateLoopStarted();
  void stopUpdateLoop();
  
 private:
  Fmi::Cache::CacheStatistics getCacheStats() const override;

  const std::string itsModuleName;

  boost::atomic_shared_ptr<PluginImpl> plugin_impl;

  SmartMet::Spine::Reactor* itsReactor;

  const char* itsConfig;

  std::atomic<bool> itsShuttingDown;
  std::atomic<bool> itsReloading;
  std::unique_ptr<std::thread> itsUpdateLoopThread;
  std::condition_variable itsUpdateNotifyCond;
  std::mutex itsUpdateNotifyMutex;
};  // class Plugin

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
