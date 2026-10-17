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
#include <engines/observation/QueryResult.h>

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
class FeatureID;

class StoredAirNuclideQueryHandler : public StoredQueryHandlerBase,
                                     protected virtual RequiresGeoEngine,
                                     protected virtual RequiresObsEngine,
                                     protected SupportsLocationParameters,
                                     protected SupportsBoundingBox,
                                     protected SupportsQualityParameters
{
 public:
  StoredAirNuclideQueryHandler(SmartMet::Spine::Reactor* reactor,
                               const StoredQueryConfig::Ptr& config,
                               PluginImpl& plugin_impl,
                               const std::optional<std::string>& template_file_name);

  ~StoredAirNuclideQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  using StationMap = std::map<std::string, SmartMet::Spine::Station>;
  using NuclideNameMap = std::map<std::string, std::string>;
  struct AnalysisContext;

  StationMap find_stations(
      const RequestParameterMap& params,
      const std::string& crs,
      const std::string& stationType,
      double maxDistance,
      uint64_t numberOfStations,
      const Fmi::DateTime& startTime,
      const Fmi::DateTime& endTime,
      const std::list<std::pair<std::string, SmartMet::Spine::LocationPtr> >& locations_list,
      const std::string& langCode) const;

  NuclideNameMap get_nuclide_names(const std::vector<std::string>& nuclideCodes,
                                   const std::string& langCode,
                                   bool queryInitializationOK) const;

  static void add_analysis_ids(SmartMet::Engine::Observation::MastQueryParams& dataQueryParams,
                               SmartMet::Engine::Observation::QueryResult& profileContainer,
                               bool latest,
                               bool queryInitializationOK);

  void set_group_station(CTPP::CDT& group,
                         const SmartMet::Spine::Station& station,
                         const AnalysisContext& ctx) const;

  int fill_analyses(CTPP::CDT& hash,
                    SmartMet::Engine::Observation::QueryResult& dataContainer,
                    const AnalysisContext& ctx,
                    FeatureID& feature_id) const;

  std::string prepare_nuclide(const std::string& nuclide) const;
  std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  double m_maxHours;
  bool m_sqRestrictions;
  bool m_supportQCParameters;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif
