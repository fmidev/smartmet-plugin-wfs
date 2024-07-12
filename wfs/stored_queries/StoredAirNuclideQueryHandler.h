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
const std::string P_NUM_OF_STATIONS = "numOfStations";
const std::string P_CRS = "crs";
const std::string P_PRODUCER_ID = "producerId";
const std::string P_LATEST = "latest";
const std::string P_NUCLIDE_CODES = "nuclideCodes";

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredAirNuclideQueryHandler : public StoredQueryHandlerBase,
                                     protected virtual RequiresGeoEngine,
                                     protected virtual RequiresObsEngine,
                                     protected SupportsLocationParameters,
                                     protected SupportsBoundingBox,
                                     protected SupportsQualityParameters
{
 public:
  StoredAirNuclideQueryHandler(SmartMet::Spine::Reactor* reactor,
                               StoredQueryConfig::Ptr config,
                               PluginImpl& plugin_impl,
                               std::optional<std::string> template_file_name);

  ~StoredAirNuclideQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  std::string prepare_nuclide(const std::string& nuclide) const;
  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  double m_maxHours;
  bool m_sqRestrictions;
  bool m_supportQCParameters;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif
