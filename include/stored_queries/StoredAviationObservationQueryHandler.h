#pragma once

#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>
#include "SupportsLocationParameters.h"
#include "SupportsBoundingBox.h"
#include "SupportsTimeZone.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *  @brief This handler class is designed to fetch IWXXM messages from ObsEngine
 */
class StoredAviationObservationQueryHandler : protected virtual SupportsExtraHandlerParams,
                                              public StoredQueryHandlerBase,
                                              protected SupportsLocationParameters,
                                              protected SupportsBoundingBox
{
 public:
  StoredAviationObservationQueryHandler(SmartMet::Spine::Reactor* reactor,
                                        boost::shared_ptr<StoredQueryConfig> config,
                                        PluginData& plugin_data,
                                        boost::optional<std::string> template_file_name);

  virtual ~StoredAviationObservationQueryHandler();

  virtual void init_handler();

  virtual void query(const StoredQuery& query,
                     const std::string& language,
                     std::ostream& output) const;

 private:
  virtual void update_parameters(const RequestParameterMap& request_params,
                                 int seq_id,
                                 std::vector<boost::shared_ptr<RequestParameterMap> >& result)
      const;

  SmartMet::Engine::Geonames::Engine* m_geoEngine;
  SmartMet::Engine::Observation::Engine* m_obsEngine;

  bool m_sqRestrictions;
  double m_maxHours;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
