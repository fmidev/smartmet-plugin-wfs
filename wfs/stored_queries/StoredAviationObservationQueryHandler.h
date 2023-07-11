#pragma once

#ifndef WITHOUT_OBSERVATION

#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "SupportsTimeZone.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>

namespace SmartMet
{
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
                                        StoredQueryConfig::Ptr config,
                                        PluginImpl& plugin_impl,
                                        boost::optional<std::string> template_file_name);

  ~StoredAviationObservationQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const boost::optional<std::string> &hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<boost::shared_ptr<RequestParameterMap> >& result) const;

  bool m_sqRestrictions;
  double m_maxHours;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
