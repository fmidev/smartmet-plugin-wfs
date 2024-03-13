#pragma once

#ifndef WITHOUT_OBSERVATION

#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "SupportsQualityParameters.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include <boost/algorithm/string.hpp>
#include <macgyver/DateTime.h>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>
#include <engines/observation/MastQueryParams.h>

namespace ba = boost::algorithm;
namespace bl = boost::lambda;

// Stored query configuration parameter names.
const std::string P_BEGIN_TIME = "beginTime";
const std::string P_END_TIME = "endTime";
const std::string P_METEO_PARAMETERS = "meteoParameters";
const std::string P_STATION_TYPE = "stationType";
const std::string P_TIME_STEP = "timeStep";
// const std::string P_WMOS                ="wmos";
// const std::string P_LPNNS               ="lpnns";
// const std::string P_FMISIDS             ="fmisids";
const std::string P_NUM_OF_STATIONS = "numOfStations";
// const std::string P_HOURS               ="hours";
// const std::string P_WEEK_DAYS           ="weekDays";
// const std::string P_LOCALE              ="locale";
const std::string P_MISSING_TEXT = "missingText";
const std::string P_MAX_EPOCHS = "maxEpochs";
const std::string P_CRS = "crs";
const std::string P_PRODUCER_ID = "producerId";

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
//  FIXME: why SupportsTimeParameters is not used
class StoredMastQueryHandler : public StoredQueryHandlerBase,
                               protected virtual RequiresGeoEngine,
                               protected virtual RequiresObsEngine,
                               protected SupportsLocationParameters,
                               protected SupportsBoundingBox,
                               protected SupportsQualityParameters
{
 public:
  StoredMastQueryHandler(SmartMet::Spine::Reactor* reactor,
                         StoredQueryConfig::Ptr config,
                         PluginImpl& plugin_impl,
                         boost::optional<std::string> template_file_name);

  ~StoredMastQueryHandler() override;

  std::string get_handler_description() const override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const boost::optional<std::string> &hostname,
                     std::ostream& output) const override;

 private:
  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<boost::shared_ptr<RequestParameterMap> >& result) const;

  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  double m_maxHours;
  bool m_sqRestrictions;
  bool m_supportQCParameters;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
