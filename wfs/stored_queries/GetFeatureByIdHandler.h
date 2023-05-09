#pragma once

#include "PluginImpl.h"
#include "ScalarParameterTemplate.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerBase.h"
#include "StoredQueryHandlerFactoryDef.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *   @brief Handler for GetFeatureById stored query
 */
class GetFeatureByIdHandler : public StoredQueryHandlerBase
{
 public:
  GetFeatureByIdHandler(SmartMet::Spine::Reactor* reactor,
                        StoredQueryConfig::Ptr config,
                        PluginImpl& plugin_impl);

  ~GetFeatureByIdHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const boost::optional<std::string>& hostname,
                     std::ostream& output) const override;

  std::vector<std::string> get_return_types() const override;

  std::string get_handler_description() const override;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_get_feature_by_id_handler_factory;
