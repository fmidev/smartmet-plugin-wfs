#pragma once

#ifndef WITHOUT_OBSERVATION

#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "SupportsQualityParameters.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/DBRegistry.h>
#include <engines/observation/Engine.h>
#include <engines/observation/QueryResult.h>
#include <map>
#include <tuple>
#include <macgyver/DateTime.h>

// Stored query configuration parameter names.
const std::string P_BEGIN_TIME = "beginTime";
const std::string P_END_TIME = "endTime";
const std::string P_METEO_PARAMETERS = "meteoParameters";
const std::string P_STATION_TYPE = "stationType";
const std::string P_WMOS = "wmos";
const std::string P_FMISIDS = "fmisids";
const std::string P_NUM_OF_STATIONS = "numOfStations";
const std::string P_MISSING_TEXT = "missingText";
const std::string P_CRS = "crs";
const std::string P_LATEST = "latest";
const std::string P_SOUNDING_TYPE = "soundingType";
const std::string P_PUBLICITY = "publicity";
const std::string P_ALTITUDE_RANGES = "altitudeRanges";
const std::string P_PRESSURE_RANGES = "pressureRanges";
// The old misspelt name of pressureRanges, still accepted in the configuration
const std::string P_PRESSURE_RANGES_OLD = "pressureRages";

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class FeatureID;

class StoredSoundingQueryHandler : public StoredQueryHandlerBase,
                                   protected virtual RequiresGeoEngine,
                                   protected virtual RequiresObsEngine,
                                   protected SupportsLocationParameters,
                                   protected SupportsBoundingBox,
                                   protected SupportsQualityParameters
{
  const char* DATA_CRS_NAME = "urn:ogc:def:crs:EPSG::4326";

  using QueryResultShared = std::shared_ptr<SmartMet::Engine::Observation::QueryResult>;
  using ValueVectorConstIt =
      SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator;
  using ParamIdStr = std::string;
  using ParamId = uint64_t;
  using ParamName = std::string;
  using ParamQCName = ParamName;
  using ShowQualityCode = bool;
  using ShowValue = bool;
  using ParamTuple = std::tuple<ParamId, ParamName, ParamQCName, ShowValue, ShowQualityCode>;
  using MeteoParameterMap = std::map<ParamIdStr, ParamTuple>;
  class RadioSounding
  {
   public:
    using Id = int;
    std::string stationId;
    Fmi::DateTime messageTime;
    Fmi::DateTime launchTime;
    Fmi::DateTime soundingEnd;
    int soundingType = 0;
  };
  using RadioSoundingMap = std::map<RadioSounding::Id, RadioSounding>;

 public:
  StoredSoundingQueryHandler(SmartMet::Spine::Reactor* reactor,
                             const StoredQueryConfig::Ptr& config,
                             PluginImpl& pluginData,
                             std::optional<std::string> templateFileName);

  ~StoredSoundingQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
                     const std::optional<std::string>& hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<std::shared_ptr<RequestParameterMap> >& result) const;

  std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  std::string solveCrs(const RequestParameterMap& params) const;

  void parseSoundingQuery(const RequestParameterMap& params,
                          const QueryResultShared& soundingQueryResult,
                          RadioSoundingMap& radioSoundingMap) const;

  void makeSoundingQuery(const RequestParameterMap& params,
                         const SmartMet::Spine::Stations& stations,
                         QueryResultShared& soundingQueryResult) const;

  void validateAndPopulateMeteoParametersToMap(const RequestParameterMap& params,
                                               MeteoParameterMap& meteoParameterMap,
                                               std::string& pressureParameterName) const;

  void makeSoundingDataQuery(const RequestParameterMap& params,
                             const RadioSoundingMap& radioSoundingMap,
                             const MeteoParameterMap& meteoParameterMap,
                             QueryResultShared& dataContainer) const;

  void getStationSearchSettings(SmartMet::Engine::Observation::Settings& settings,
                                const RequestParameterMap& params,
                                const std::string& language) const;

  void checkMaxSoundings(const Fmi::DateTime& startTime,
                         const Fmi::DateTime& endTime,
                         const RadioSoundingMap& radioSoundingMap) const;

  struct SoundingContext;

  // The iterators to the level information of the sounding data
  struct LevelIterators
  {
    ValueVectorConstIt levelTime;
    ValueVectorConstIt altitude;
    ValueVectorConstIt pressure;
    ValueVectorConstIt longitude;
    ValueVectorConstIt latitude;
    ValueVectorConstIt significance;
  };

  INT_64 set_sounding_group(CTPP::CDT& group,
                            const SoundingContext& ctx,
                            int64_t soundingId,
                            int64_t soundingType,
                            FeatureID& featureId,
                            std::string& currentStationId) const;

  void set_sounding_station(CTPP::CDT& group,
                            const SoundingContext& ctx,
                            const std::string& currentStationId,
                            long double& stationLatitude,
                            long double& stationLongitude) const;

  static void set_group_parameters(CTPP::CDT& group,
                                   const SoundingContext& ctx,
                                   const std::string& measurandIdStr,
                                   int& paramListCount,
                                   bool& showValue,
                                   bool& showDataQuality);

  void set_level_info(CTPP::CDT& row,
                      const SoundingContext& ctx,
                      SmartMet::Engine::Observation::QueryResult& dataContainer,
                      const LevelIterators& its,
                      INT_64 sEpoch,
                      long double stationLatitude,
                      long double stationLongitude,
                      int paramcount) const;

  int fill_sounding_data(CTPP::CDT& hash,
                         SmartMet::Engine::Observation::QueryResult& dataContainer,
                         const SoundingContext& ctx,
                         FeatureID& featureId) const;

  double mMaxHours;
  uint64_t mMaxSoundings;
  bool mSqRestrictions;
  bool mSupportQCParameters;
  std::string mPressureRangesParam;  // P_PRESSURE_RANGES or P_PRESSURE_RANGES_OLD
};
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
