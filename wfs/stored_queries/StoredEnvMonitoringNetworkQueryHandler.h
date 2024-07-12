#pragma once

#ifndef WITHOUT_OBSERVATION

#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>
#include <engines/observation/MastQuery.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredEnvMonitoringNetworkQueryHandler : protected virtual RequiresGeoEngine,
                                               protected virtual RequiresObsEngine,
                                               public StoredQueryHandlerBase

{
 public:
  StoredEnvMonitoringNetworkQueryHandler(SmartMet::Spine::Reactor* reactor,
                                         StoredQueryConfig::Ptr config,
                                         PluginImpl& plugin_impl,
                                         std::optional<std::string> template_file_name);
  ~StoredEnvMonitoringNetworkQueryHandler() override;

  std::string get_handler_description() const override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

 private:
  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  std::string m_missingText;
  int m_debugLevel;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
