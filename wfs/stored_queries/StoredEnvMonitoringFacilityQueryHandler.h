#pragma once
#ifndef WITHOUT_OBSERVATION

#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>
#include <engines/observation/MastQuery.h>
#include <unordered_map>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredEnvMonitoringFacilityQueryHandler : public StoredQueryHandlerBase,
                                                protected virtual RequiresGeoEngine,
                                                protected virtual RequiresObsEngine
{
 public:
  StoredEnvMonitoringFacilityQueryHandler(SmartMet::Spine::Reactor* reactor,
                                          StoredQueryConfig::Ptr config,
                                          PluginImpl& plugin_impl,
                                          std::optional<std::string> template_file_name);
  ~StoredEnvMonitoringFacilityQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:
  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  struct CapabilityData
  {
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator station_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator measurand_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator measurand_code;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator measurand_name;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator measurand_layer;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator aggregate_period;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator aggregate_function;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator earliest_data;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator latest_data;
  };
  using StationCapabilityData = std::vector<CapabilityData>;
  using StationCapabilityMap = std::map<std::string, StationCapabilityData>;

  struct StationData
  {
    int64_t station_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator station_name;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator station_start;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator station_end;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator longitude;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator latitude;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator elevation;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator country_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator stationary;
  };
  using StationDataMap = std::map<std::string, StationData>;

  struct StationGroup
  {
    int64_t station_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator group_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator group_name;
  };
  using StationGroupVector = std::vector<StationGroup>;
  using StationGroupMap = std::map<std::string, StationGroupVector>;

  struct NetworkMembership
  {
    int64_t station_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator network_id;
    SmartMet::Engine::Observation::QueryResult::ValueVectorType::const_iterator member_code;
  };
  using NetworkMembershipVector = std::vector<NetworkMembership>;
  using NetworkMembershipMap = std::map<std::string, NetworkMembershipVector>;

  void getValidStations(SmartMet::Engine::Observation::MastQuery& stationQuery,
                        StationDataMap& validStations,
                        const RequestParameterMap& params) const;
  void getStationCapabilities(SmartMet::Engine::Observation::MastQuery& scQuery,
                              StationCapabilityMap& stationCapabilityMap,
                              const RequestParameterMap& params,
                              const StationDataMap& validStations) const;

  void getStationGroupData(const std::string& language,
                           SmartMet::Engine::Observation::MastQuery& emfQuery,
                           StationGroupMap& stationGroupMap,
                           const RequestParameterMap& params,
                           const StationDataMap& validStations) const;
  void getStationNetworkMembershipData(const std::string& language,
                                       SmartMet::Engine::Observation::MastQuery& emfQuery,
                                       NetworkMembershipMap& networkMemberShipMap,
                                       const RequestParameterMap& params,
                                       const StationDataMap& validStations) const;

  std::string m_missingText;
  int m_debugLevel;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
