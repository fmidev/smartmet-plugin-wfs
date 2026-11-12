#pragma once

#ifndef WITHOUT_OBSERVATION

#include "RequestParameterMap.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsLocationParameters.h"
#include <spine/Location.h>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Locus
{
class QueryOptions;
}  // namespace Locus
namespace SmartMet::Plugin::WFS
{
class PluginImpl;
}  // namespace SmartMet::Plugin::WFS
namespace SmartMet::Spine
{
class Reactor;
}  // namespace SmartMet::Spine

namespace SmartMet
{
namespace Engine
{
namespace Observation
{
class VerifiableMessageQuery;
class VerifiableMessageQueryParams;
}  // namespace Observation
}  // namespace Engine

namespace Plugin
{
namespace WFS
{
/**
 *  @brief This handler class is designed to fetch IWXXM messages from ObsEngine
 *
 *  FIXME: why SupportsTimeParameters is not used
 */
class StoredAviationObservationQueryHandler : public StoredQueryHandlerBase,
                                              protected virtual RequiresGeoEngine,
                                              protected virtual RequiresObsEngine,
                                              protected SupportsLocationParameters,
                                              protected SupportsBoundingBox
{
 public:
  StoredAviationObservationQueryHandler(SmartMet::Spine::Reactor* reactor,
                                        const StoredQueryConfig::Ptr& config,
                                        PluginImpl& plugin_impl,
                                        const std::optional<std::string>& template_file_name);

  ~StoredAviationObservationQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  void add_requested_icao_codes(
      std::map<std::string, SmartMet::Spine::LocationPtr>& validIcaoCodes,
      const std::vector<std::string>& icaoCodeVector,
      const Locus::QueryOptions& nameSearchOptions) const;

  void make_message_query(
      SmartMet::Engine::Observation::VerifiableMessageQuery& verifiableMessageQuery,
      SmartMet::Engine::Observation::VerifiableMessageQueryParams& queryParams,
      const std::map<std::string, SmartMet::Spine::LocationPtr>& validIcaoCodes,
      const RequestParameterMap& params,
      bool returnOnlyLatest) const;

  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<std::shared_ptr<RequestParameterMap> >& result) const;

  bool m_sqRestrictions;
  double m_maxHours;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
