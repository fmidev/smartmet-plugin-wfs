#pragma once

#include "PluginData.h"
#include "StoredQuery.h"
#include "StoredQueryHandlerBase.h"
#include <boost/filesystem.hpp>
#include <macgyver/TemplateDirectory.h>
#include <spine/Reactor.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class PluginData;
class StoredQueryHandlerBase;

/**
 *  @brief A C++ class for holding information about available WFS stored queries
 *         and mapping query names to actual objects which handles the queries.
 */
class StoredQueryMap
{
  class Init;

 public:
  StoredQueryMap();

  virtual ~StoredQueryMap();

  void set_background_init(bool value);

  void add_handler(boost::shared_ptr<StoredQueryHandlerBase> handler);

  void add_handler(SmartMet::Spine::Reactor* theReactor,
                   const boost::filesystem::path& stored_query_config_file,
                   const Fmi::TemplateDirectory& template_dir,
                   PluginData& plugin_data);

  void read_config_dir(SmartMet::Spine::Reactor* theReactor,
                       const boost::filesystem::path& config_dir,
                       const Fmi::TemplateDirectory& template_dir,
                       PluginData& plugin_data);

  inline std::vector<std::string> get_handler_names() const { return handler_names; }
  boost::shared_ptr<StoredQueryHandlerBase> get_handler_by_name(const std::string name) const;

  virtual std::vector<std::string> get_return_type_names() const;

 private:
  void add_handler(SmartMet::Spine::Reactor* theReactor,
                   boost::shared_ptr<StoredQueryConfig> sqh_config,
                   const Fmi::TemplateDirectory& template_dir,
                   PluginData& plugin_data);

  void add_handler_thread_proc(SmartMet::Spine::Reactor* theReactor,
                               boost::shared_ptr<StoredQueryConfig> config,
                               const Fmi::TemplateDirectory& template_dir,
                               PluginData& plugin_data);

 private:
  bool background_init;
  mutable SmartMet::Spine::MutexType mutex;
  std::vector<std::string> handler_names;
  std::map<std::string, boost::shared_ptr<StoredQueryHandlerBase> > handler_map;
  std::unique_ptr<Init> init;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
