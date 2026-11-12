#pragma once

#include "RequiresGeoEngine.h"
#include "RequiresQEngine.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsLocationParameters.h"
#include "SupportsTimeParameters.h"
#include "SupportsTimeZone.h"
#include <engines/querydata/Producer.h>
#include <engines/querydata/Q.h>
#include <macgyver/DateTime.h>
#include <newbase/NFmiParameterName.h>
#include <spine/Location.h>
#include <spine/Value.h>
#include <timeseries/TimeSeriesGenerator.h>
#include <timeseries/TimeSeriesInclude.h>
#include <iosfwd>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class OGRSpatialReference;
namespace CTPP
{
class CDT;
}  // namespace CTPP
namespace SmartMet::Plugin::WFS
{
class PluginImpl;
}  // namespace SmartMet::Plugin::WFS
namespace SmartMet::Spine
{
class CRSRegistry;
class Parameter;
class Reactor;
}  // namespace SmartMet::Spine

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
struct AirportLocation
{
  std::string icao_code;
  SmartMet::Spine::LocationPtr loc;

  AirportLocation(std::string icao, SmartMet::Spine::LocationPtr l)
      : icao_code(std::move(icao)), loc(std::move(l))
  {
  }
};

using AirportLocationList = std::list<AirportLocation>;

struct ProbabilityConfigParam
{
  std::string precipitationType;
  FmiParameterName idLight = kFmiBadParameter;
  FmiParameterName idModerate = kFmiBadParameter;
  FmiParameterName idHeavy = kFmiBadParameter;
};

using ProbabilityConfigParamVector = std::vector<ProbabilityConfigParam>;

struct ProbabilityConfigParams
{
  std::string probabilityUnit;
  std::string intensityLight;
  std::string intensityModerate;
  std::string intensityHeavy;
  ProbabilityConfigParamVector params;
};

struct ProbabilityQueryParam
{
  const SmartMet::Spine::Parameter& paramLight;
  const SmartMet::Spine::Parameter& paramModerate;
  const SmartMet::Spine::Parameter& paramHeavy;
  SmartMet::Engine::Querydata::Q& q;
  OGRSpatialReference& sr;
  SmartMet::Spine::BoundingBox bbox;
  std::locale outputLocale;
  bool nearestValid = false;
  std::string tz_name;
  TS::TimeSeriesGenerator::LocalTimeList tlist;
  SmartMet::Spine::LocationPtr loc;
  SmartMet::Engine::Querydata::Producer producer;
  std::string missingText;
  std::string language;
  std::string precipitationType;

  ProbabilityQueryParam(const SmartMet::Spine::Parameter& lp,
                        const SmartMet::Spine::Parameter& mp,
                        const SmartMet::Spine::Parameter& hp,
                        SmartMet::Engine::Querydata::Q& qe,
                        OGRSpatialReference& spref)
      : paramLight(lp), paramModerate(mp), paramHeavy(hp), q(qe), sr(spref), tz_name("UTC")
  {
  }
};

// contains timestamp and probability for all intesities
// for example probability for light/moderate/heavy snow 12:00
struct WinterWeatherProbability
{
  Fmi::DateTime timestamp;
  double light_probability;
  double moderate_probability;
  double heavy_probability;

  WinterWeatherProbability(const Fmi::DateTime& t, double lp, double mp, double hp)
      : timestamp(t), light_probability(lp), moderate_probability(mp), heavy_probability(hp)
  {
  }
};

// Winter Weather probability time series
// for example light/moderate/heavy snow probability at 12:00, 13:00, 14:00
using WinterWeatherIntensityProbabilities = std::vector<WinterWeatherProbability>;

// probabilities precipitation types
using WinterWeatherTypeProbabilities = std::map<std::string,
        WinterWeatherIntensityProbabilities>;

// result set for the whole query: contains probabilities for all
// locations, precipitation types, intensities, timesteps
using ProbabilityQueryResultSet = std::map<SmartMet::Spine::LocationPtr,
        WinterWeatherTypeProbabilities>;

/**
 *   @brief Handler for StoredWWProbabilityQuery stored query
 */
class StoredWWProbabilityQueryHandler : public StoredQueryHandlerBase,
                                        protected virtual RequiresGeoEngine,
                                        protected virtual RequiresQEngine,
                                        protected SupportsLocationParameters,
                                        protected SupportsBoundingBox,
                                        protected SupportsTimeParameters,
                                        protected SupportsTimeZone
{
 public:
  StoredWWProbabilityQueryHandler(SmartMet::Spine::Reactor* reactor,
                                  const StoredQueryConfig::Ptr& config,
                                  PluginImpl& plugin_impl,
                                  std::optional<std::string> templateFileileName);

  ~StoredWWProbabilityQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string>& hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  void parseQueryResults(const ProbabilityQueryResultSet& query_results,
                         const SmartMet::Spine::BoundingBox& bbox,
                         const AirportLocationList& airp_llist,
                         const std::string& language,
                         const SmartMet::Spine::CRSRegistry& crsRegistry,
                         const std::string& requestedCRS,
                         const Fmi::DateTime& origintime,
                         const Fmi::DateTime& modificationtime,
                         const std::string& tz_name,
                         CTPP::CDT& hash) const;

  WinterWeatherIntensityProbabilities getProbabilities(
      const ProbabilityQueryParam& queryParam) const;

  ProbabilityConfigParams itsProbabilityConfigParams;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
