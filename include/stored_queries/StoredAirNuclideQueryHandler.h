#pragma once

#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsBoundingBox.h"
#include "SupportsLocationParameters.h"
#include "SupportsQualityParameters.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>
#include <engines/observation/MastQueryParams.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace pt = boost::posix_time;
namespace lt = boost::local_time;
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
class StoredAirNuclideQueryHandler : protected virtual SupportsExtraHandlerParams,
                                     public StoredQueryHandlerBase,
                                     protected SupportsLocationParameters,
                                     protected SupportsBoundingBox,
                                     protected SupportsQualityParameters
{
 public:
  StoredAirNuclideQueryHandler(SmartMet::Spine::Reactor* reactor,
                               boost::shared_ptr<StoredQueryConfig> config,
                               PluginData& plugin_data,
                               boost::optional<std::string> template_file_name);

  virtual ~StoredAirNuclideQueryHandler();

  virtual void init_handler();

  virtual void query(const StoredQuery& query,
                     const std::string& language,
                     std::ostream& output) const;

 private:
  std::string prepare_nuclide(const std::string& nuclide) const;
  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  double m_maxHours;
  bool m_sqRestrictions;
  bool m_supportQCParameters;

  SmartMet::Engine::Observation::Engine* m_obsEngine;
  SmartMet::Engine::Geonames::Engine* m_geoEngine;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
