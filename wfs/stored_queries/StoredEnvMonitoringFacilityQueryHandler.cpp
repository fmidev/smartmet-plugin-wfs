#ifndef WITHOUT_OBSERVATION

#include "stored_queries/StoredEnvMonitoringFacilityQueryHandler.h"
#include "FeatureID.h"
#include "MediaMonitored.h"
#include "StoredQueryHandlerFactoryDef.h"
#include "SupportsLocationParameters.h"
#include <smartmet/engines/observation/DBRegistry.h>
#include <smartmet/macgyver/Exception.h>

#include <boost/icl/type_traits/to_string.hpp>
#include <functional>
#include <future>
#include <memory>
#include <unordered_map>

namespace bw = SmartMet::Plugin::WFS;
namespace bo = SmartMet::Engine::Observation;

namespace
{
const char *P_CLASS_ID = "classId";
const char *P_GROUP_ID = "groupId";
const char *P_STATION_ID = "stationId";
const char *P_STATION_NAME = "stationName";
const char *P_MISSING_TEXT = "missingText";
const char *P_BEGIN_TIME = "beginTime";
const char *P_END_TIME = "endTime";
const char *P_BASE_PHENOMENON = "basePhenomenon";
const char *P_AGGREGATE_FUNCTION = "aggregateFunction";
const char *P_AGGREGATE_PERIOD = "aggregatePeriod";
const char *P_MEASURAND_CODE = "measurandCode";
const char *P_STORAGE_ID = "storageId";
const char *P_INSPIRE_NAMESPACE = "inspireNamespace";
const char *P_AUTHORITY_DOMAIN = "authorityDomain";
const char *P_SHOW_OBSERVING_CAPABILITY = "showObservingCapability";
}  // namespace

bw::StoredEnvMonitoringFacilityQueryHandler::StoredEnvMonitoringFacilityQueryHandler(
    SmartMet::Spine::Reactor *reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl &plugin_data,
    std::optional<std::string> template_file_name)

    : StoredQueryParamRegistry(config),
      SupportsExtraHandlerParams(config),
      RequiresGeoEngine(reactor),
      RequiresObsEngine(reactor),
      StoredQueryHandlerBase(reactor, config, plugin_data, template_file_name)
{
  try
  {
    register_scalar_param<Fmi::DateTime>(P_BEGIN_TIME, "");
    register_scalar_param<Fmi::DateTime>(P_END_TIME, "");
    register_array_param<int64_t>(P_CLASS_ID, "");
    register_array_param<int64_t>(P_GROUP_ID, "");
    register_array_param<int64_t>(P_STATION_ID, "");
    register_array_param<std::string>(P_STATION_NAME, "", false);
    register_array_param<std::string>(P_AGGREGATE_FUNCTION, "");
    register_array_param<std::string>(P_AGGREGATE_PERIOD, "");
    register_array_param<std::string>(P_BASE_PHENOMENON, "");
    register_array_param<std::string>(P_MEASURAND_CODE, "");
    register_array_param<int64_t>(P_STORAGE_ID, "");
    register_scalar_param<std::string>(P_INSPIRE_NAMESPACE, "");
    register_scalar_param<std::string>(P_AUTHORITY_DOMAIN, "");
    register_scalar_param<bool>(P_SHOW_OBSERVING_CAPABILITY, "", false);
    m_missingText = config->get_optional_config_param<std::string>(P_MISSING_TEXT, "NaN");
    m_debugLevel = config->get_debug_level();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::StoredEnvMonitoringFacilityQueryHandler::~StoredEnvMonitoringFacilityQueryHandler() = default;

std::string bw::StoredEnvMonitoringFacilityQueryHandler::get_handler_description() const
{
    return "";
}

void bw::StoredEnvMonitoringFacilityQueryHandler::query(const StoredQuery &query,
                                                        const std::string &language,
							const std::optional<std::string> & /*hostname*/,
                                                        std::ostream &output) const
{
  try
  {
    const RequestParameterMap &params = query.get_param_map();
    auto inspireNamespace = params.get_single<std::string>(P_INSPIRE_NAMESPACE);
    auto showObservingCapability = params.get_optional<bool>(P_SHOW_OBSERVING_CAPABILITY, false);
    auto authorityDomain = params.get_single<std::string>(P_AUTHORITY_DOMAIN);

    // Get the sequence number of query in the request
    auto sqId = query.get_query_id();
    FeatureID featureId(get_config()->get_query_id(), params.get_map(true), sqId);

    // Removing some feature id parameters
    const char *removeParams[] = {P_STATION_ID, P_STATION_NAME, P_GROUP_ID};
    for (auto & removeParam : removeParams)
    {
      featureId.erase_param(removeParam);
    }

    if (m_debugLevel > 0)
      query.dump_query_info(std::cout);

    bool LPNN_active = false;
    bool WMO_active = true;
    bool LIVI_active = false;
    bool ICAO_active = true;
    bool AWSMETAR_active = false;
    bool SYKE_active = false;
    bool NORDLIST_active = false;
    bool AIRQUALITY_active = false;
    bool RADAR_active = true;
    bool IMAGE_active = false;

    // Get valid stations
    bo::MastQuery validStationsQuery;
    StationDataMap validStations;
    getValidStations(validStationsQuery, validStations, params);

    // Get capability data from obsengine.
    StationCapabilityMap stationCapabilityMap;
    std::future<void> fObservingCapability;
    bo::MastQuery scQuery;
    if (showObservingCapability) {
      fObservingCapability =
	std::async(std::launch::async,
		   [this, &scQuery, &stationCapabilityMap, &params, &validStations]() {
		     getStationCapabilities(scQuery, stationCapabilityMap, params, validStations);
		   });
    }

    // Get station group data from Observation
    StationGroupMap stationGroupMap;
    bo::MastQuery sgQuery;
    std::future<void> fStationGroupData =
      std::async(std::launch::async,
		 [this, &sgQuery, &language, &stationGroupMap, &params, &validStations]() {
		   getStationGroupData(language, sgQuery, stationGroupMap, params, validStations);
		 });

    // Get network membership data from Observation
    NetworkMembershipMap networkMemberShipMap;
    bo::MastQuery emfQuery;
    std::future<void> fNetworkMembershipMap =
      std::async(std::launch::async,
		 [this, &emfQuery, &language, &networkMemberShipMap, &params, &validStations]() {
		   getStationNetworkMembershipData(language, emfQuery, networkMemberShipMap, params, validStations);
		 });

    if (showObservingCapability) {
      fObservingCapability.wait();
    }
    fStationGroupData.wait();
    fNetworkMembershipMap.wait();

    CTPP::CDT hash;
    params.dump_params(hash["query_parameters"]);
    dump_named_params(params, hash["named_parameters"]);
    hash["authorityDomain"] = authorityDomain;
    int stationCounter = 0;

    if (not validStations.empty())
    {
      auto &crs_registry = get_plugin_impl().get_crs_registry();

      // Stations are bounded by..
      const std::string boundedByCRS = "EPSG:4258";
      bool bbShowHeight = false;
      std::string bbProjUri;
      std::string bbProjEpochUri;
      std::string bbAxisLabels;
      crs_registry.get_attribute(boundedByCRS, "showHeight", &bbShowHeight);
      crs_registry.get_attribute(boundedByCRS, "projUri", &bbProjUri);
      crs_registry.get_attribute(boundedByCRS, "projEpochUri", &bbProjEpochUri);
      crs_registry.get_attribute(boundedByCRS, "axisLabels", &bbAxisLabels);

      hash["bbProjUri"] = bbProjUri;
      hash["bbProjEpoch_uri"] = bbProjEpochUri;
      hash["bbAxisLabels"] = bbAxisLabels;
      hash["bbProjSrsDim"] = (bbShowHeight ? 3 : 2);

      // FIXME!! Use output CRS requested
      hash["axisLabels"] = bbAxisLabels;
      hash["projUri"] = bbProjUri;
      hash["projSrsDim"] = (bbShowHeight ? 3 : 2);

      // Filling the network and station data
      std::string lang = language;
      SupportsLocationParameters::engOrFinToEnOrFi(lang);

      Locus::QueryOptions opts;
      opts.SetCountries("all");
      opts.SetSearchVariants(true);
      opts.SetLanguage("fmisid");
      opts.SetFeatures("SYNOP,STUK");
      opts.SetResultLimit(1);

      for (auto & validStation : validStations)
      {
        auto scmIt = stationCapabilityMap.find(validStation.first);
        auto stationGroupMapIt = stationGroupMap.find(validStation.first);

        // A station must be part of a station group
        if (stationGroupMapIt == stationGroupMap.end())
          continue;

        featureId.add_param(P_STATION_ID, validStation.first);
        hash["stations"][stationCounter]["featureId"] = featureId.get_id();
        featureId.erase_param(P_STATION_ID);

        // Station data
        hash["stations"][stationCounter]["fmisid"] = validStation.first;
        hash["stations"][stationCounter]["mobile"] =
            (bo::QueryResult::toString(validStation.second.stationary) == "N" ? "true" : "false");
        hash["stations"][stationCounter]["beginPosition"] =
            bo::QueryResult::toString(validStation.second.station_start);
        hash["stations"][stationCounter]["endPosition"] =
            bo::QueryResult::toString(validStation.second.station_end);
        hash["stations"][stationCounter]["name"] =
            bo::QueryResult::toString(validStation.second.station_name);
        hash["stations"][stationCounter]["opActivityPeriods"][0]["beginPosition"] =
            bo::QueryResult::toString(validStation.second.station_start);
        hash["stations"][stationCounter]["opActivityPeriods"][0]["endPosition"] =
            bo::QueryResult::toString(validStation.second.station_end);
        hash["stations"][stationCounter]["inspireNamespace"] = inspireNamespace;
        hash["stations"][stationCounter]["country-ISO-3166-Numeric"] =
            bo::QueryResult::toString(validStation.second.country_id, 0);

        std::string countryName;

        // Data from Geonames
        SmartMet::Spine::LocationList locList = geo_engine->nameSearch(opts, validStation.first);
        if (not locList.empty())
        {
          SmartMet::Spine::LocationPtr stationLocPtr = locList.front();
          if (stationLocPtr->geoid != 0)
            hash["stations"][stationCounter]["geoid"] = Fmi::to_string(stationLocPtr->geoid);
          if (not stationLocPtr->area.empty())
            hash["stations"][stationCounter]["region"] = stationLocPtr->area;
          if (not stationLocPtr->country.empty())
            hash["stations"][stationCounter]["country"] = stationLocPtr->country;
        }

        // Geonames does not support country name search by using ISO-3166 Numeric value.
        // So here we do special handling for the stations in Finland and Sweden.
        std::string tmpCountryName;
        const std::string countryId = bo::QueryResult::toString(validStation.second.country_id);
        if (countryId == "246")
          tmpCountryName = geo_engine->countryName("FI", lang);
        else if (countryId == "752")
          tmpCountryName = geo_engine->countryName("SE", lang);

        if (not tmpCountryName.empty())
          hash["stations"][stationCounter]["country"] = tmpCountryName;

        std::string pos = bo::QueryResult::toString(validStation.second.latitude, 6) + " " +
                          bo::QueryResult::toString(validStation.second.longitude, 6);
        hash["stations"][stationCounter]["position"] = pos;
        if (bbShowHeight)
          hash["stations"][stationCounter]["elevation"] =
              bo::QueryResult::toString(validStation.second.elevation, 0);

        // Capability data from Observation
        if (scmIt != stationCapabilityMap.end())
        {
          CTPP::CDT &capabilityCDT = hash["stations"][stationCounter]["observingCapabilities"];
          auto scmDataIt = scmIt->second.begin();
          int ocId = 0;
          MediaMonitored mediaMonitored;

          for (; scmDataIt != scmIt->second.end(); ++scmDataIt)
          {
            capabilityCDT[ocId]["measurandId"] = bo::QueryResult::toString(scmDataIt->measurand_id);
            capabilityCDT[ocId]["measurandCode"] =
                bo::QueryResult::toString(scmDataIt->measurand_code);
            capabilityCDT[ocId]["measurandName"] =
                bo::QueryResult::toString(scmDataIt->measurand_name);
            capabilityCDT[ocId]["measurandAggregatePeriod"] =
                bo::QueryResult::toString(scmDataIt->aggregate_period);
            std::string aggregate_function =
                bo::QueryResult::toString(scmDataIt->aggregate_function);
            if (not aggregate_function.empty())
              capabilityCDT[ocId]["measurandAggregateFunction"] = aggregate_function;
            capabilityCDT[ocId]["beginPosition"] =
                bo::QueryResult::toString(scmDataIt->earliest_data);
            capabilityCDT[ocId]["endPosition"] = bo::QueryResult::toString(scmDataIt->latest_data);
            ++ocId;

            // mediaMonitored object discards the invalid values.
            mediaMonitored.add(bo::QueryResult::toString(scmDataIt->measurand_layer));
          }

          int mediaValueIndex = 0;
          for (const auto & it : mediaMonitored)
            hash["stations"][stationCounter]["mediaMonitored"][mediaValueIndex++] = it;
        }

        // Some of the stations are in member groups.
        auto networkMemberShipMapIt =
            networkMemberShipMap.find(validStation.first);
        if (networkMemberShipMapIt != networkMemberShipMap.end())
        {
          for (const auto & it : (*networkMemberShipMapIt).second)
          {
            const std::string networkMemberCodeStr = bo::QueryResult::toString(it.member_code);
            const std::string networkIdStr = bo::QueryResult::toString(it.network_id, 0);
            if (LPNN_active and networkIdStr == "10")
              hash["stations"][stationCounter]["lpnn"] = networkMemberCodeStr;
            else if (WMO_active and networkIdStr == "20")
              hash["stations"][stationCounter]["wmo"] = networkMemberCodeStr;
            else if (LIVI_active and networkIdStr == "30")
              hash["stations"][stationCounter]["livi"] = networkMemberCodeStr;
            else if (ICAO_active and networkIdStr == "40")
              hash["stations"][stationCounter]["icao"] = networkMemberCodeStr;
            else if (AWSMETAR_active and networkIdStr == "45")
              hash["stations"][stationCounter]["awsmetar"] = networkMemberCodeStr;
            else if (SYKE_active and networkIdStr == "51")
              hash["stations"][stationCounter]["syke"] = networkMemberCodeStr;
            else if (NORDLIST_active and networkIdStr == "60")
              hash["stations"][stationCounter]["nordlist"] = networkMemberCodeStr;
            else if (AIRQUALITY_active and networkIdStr == "62")
              hash["stations"][stationCounter]["airquality"] = networkMemberCodeStr;
            else if (RADAR_active and networkIdStr == "70")
              hash["stations"][stationCounter]["radar"] = networkMemberCodeStr;
            else if (IMAGE_active and networkIdStr == "90")
              hash["stations"][stationCounter]["image"] = networkMemberCodeStr;
          }
        }

        // Every station is part of a station group.
        if (stationGroupMapIt != stationGroupMap.end())
        {
          int64_t groupCounter = 0;
          for (const auto & it : (*stationGroupMapIt).second)
          {
            hash["stations"][stationCounter]["networks"][groupCounter]["id"] =
                bo::QueryResult::toString(it.group_id, 0);
            hash["stations"][stationCounter]["networks"][groupCounter]["name"] =
                bo::QueryResult::toString(it.group_name);
            groupCounter++;
          }
        }

        // Count of the stations
        stationCounter++;
      }
    }

    const std::string stationsMatched = boost::to_string(stationCounter);
    hash["stationsMatched"] = stationsMatched;
    hash["stationsReturned"] = stationsMatched;
    hash["responseTimestamp"] =
        Fmi::date_time::to_iso_extended_string(get_plugin_impl().get_time_stamp()) + "Z";
    hash["queryId"] = query.get_query_id();
    hash["fmi_apikey"] = QueryBase::FMI_APIKEY_SUBST;
    hash["fmi_apikey_prefix"] = bw::QueryBase::FMI_APIKEY_PREFIX_SUBST;
    hash["hostname"] = QueryBase::HOSTNAME_SUBST;
    hash["protocol"] = QueryBase::PROTOCOL_SUBST;
    hash["language"] = language;

    // Format output
    format_output(hash, output, query.get_use_debug_format());

  }  // In case of some other failures
  catch (...)
  {
    // Set language for exception and re-throw it
    Fmi::Exception exception(BCP, "Operation processing failed!", nullptr);
    if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
    exception.addParameter(WFS_LANGUAGE, language);
    throw exception;
  }
}

const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig>
bw::StoredEnvMonitoringFacilityQueryHandler::dbRegistryConfig(const std::string &configName) const
{
  try
  {
    // Get database registry from Observation
    const std::shared_ptr<SmartMet::Engine::Observation::DBRegistry> dbRegistry =
        obs_engine->dbRegistry();
    if (not dbRegistry)
    {
      std::ostringstream msg;
      msg << "Database registry is not available!";
      Fmi::Exception exception(BCP, msg.str());
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      throw exception;
    }

    const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbrConfig =
        dbRegistry->dbRegistryConfig(configName);
    if (not dbrConfig)
    {
      std::ostringstream msg;
      msg << "Database registry configuration '" << configName << "' is not available!";
      Fmi::Exception exception(BCP, msg.str());
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      throw exception;
    }

    return dbrConfig;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredEnvMonitoringFacilityQueryHandler::getValidStations(
    bo::MastQuery &stationQuery,
    bw::StoredEnvMonitoringFacilityQueryHandler::StationDataMap &validStations,
    const RequestParameterMap &params) const
{
  try
  {
    std::vector<int64_t> groupIdVector;
    params.get<int64_t>(P_GROUP_ID, std::back_inserter(groupIdVector));

    std::vector<int64_t> stationIdVector;
    params.get<int64_t>(P_STATION_ID, std::back_inserter(stationIdVector));

    std::vector<std::string> stationNameVector;
    params.get<std::string>(P_STATION_NAME, std::back_inserter(stationNameVector));

    std::vector<std::string> basePhenomenonVector;
    params.get<std::string>(P_BASE_PHENOMENON, std::back_inserter(basePhenomenonVector));

    std::vector<std::string> aggregateFunctions;
    params.get<std::string>(P_AGGREGATE_FUNCTION, std::back_inserter(aggregateFunctions));

    std::vector<std::string> aggregatePeriods;
    params.get<std::string>(P_AGGREGATE_PERIOD, std::back_inserter(aggregatePeriods));

    std::vector<std::string> measurandCodeVector;
    params.get<std::string>(P_MEASURAND_CODE, std::back_inserter(measurandCodeVector));

    auto startTime = params.get_single<Fmi::DateTime>(P_BEGIN_TIME);
    auto endTime = params.get_single<Fmi::DateTime>(P_END_TIME);

    bo::MastQueryParams stationQueryParams(dbRegistryConfig("OBSERVATIONS_V2"));
    stationQueryParams.addJoinOnConfig(dbRegistryConfig("STATIONS_V1"), "STATION_ID");
    stationQueryParams.addJoinOnConfig(dbRegistryConfig("GROUP_MEMBERS_V1"), "STATION_ID");
    stationQueryParams.addJoinOnConfig(dbRegistryConfig("LOCATIONS_V2"), "STATION_ID");
    stationQueryParams.useDistinct();

    if (not basePhenomenonVector.empty() or not aggregateFunctions.empty() or
        not aggregatePeriods.empty() or not measurandCodeVector.empty())
      stationQueryParams.addJoinOnConfig(dbRegistryConfig("MEASURANDS_V1"), "MEASURAND_ID");

    for (const auto & it : basePhenomenonVector)
      stationQueryParams.addOperation(
          "OR_GROUP_measurand_id", "BASE_PHENOMENON", "PropertyIsEqualTo", it);

    for (const auto & aggregateFunction : aggregateFunctions)
      stationQueryParams.addOperation("OR_GROUP_aggregate_function",
                                      "AGGREGATE_FUNCTION",
                                      "PropertyIsEqualTo",
                                      Fmi::ascii_tolower_copy(aggregateFunction));

    for (const auto & aggregatePeriod : aggregatePeriods)
      stationQueryParams.addOperation("OR_GROUP_aggregate_period",
                                      "AGGREGATE_PERIOD",
                                      "PropertyIsEqualTo",
                                      Fmi::ascii_toupper_copy(aggregatePeriod));

    for (const auto & it : measurandCodeVector)
      stationQueryParams.addOperation("OR_GROUP_measurand_code",
                                      "MEASURAND_CODE",
                                      "PropertyIsEqualTo",
                                      Fmi::ascii_toupper_copy(it));

    for (long it : groupIdVector)
      stationQueryParams.addOperation("OR_GROUP_group_id", "GROUP_ID", "PropertyIsEqualTo", it);

    // Station name and id will be put into the same group.
    for (long it : stationIdVector)
      stationQueryParams.addOperation(
          "OR_GROUP_station_id", "STATION_ID", "PropertyIsEqualTo", it);
    for (const auto & it : stationNameVector)
      stationQueryParams.addOperation("OR_GROUP_station_id", "STATION_NAME", "PropertyIsLike", it);

    stationQueryParams.addOperation(
        "OR_GROUP_station_end", "STATION_END", "PropertyIsGreaterThanOrEqualTo", startTime);
    stationQueryParams.addOperation(
        "OR_GROUP_station_start", "STATION_START", "PropertyIsLessThanOrEqualTo", endTime);

    stationQueryParams.addField("STATION_ID");
    stationQueryParams.addField("STATION_NAME");
    stationQueryParams.addField("STATION_START");
    stationQueryParams.addField("STATION_END");
    stationQueryParams.addField("STATION_GEOMETRY.sdo_point.x", "LONGITUDE");
    stationQueryParams.addField("STATION_GEOMETRY.sdo_point.y", "LATITUDE");
    stationQueryParams.addField("STATION_ELEVATION");
    stationQueryParams.addField("COUNTRY_ID");
    stationQueryParams.addField("STATIONARY");

    stationQueryParams.addOrderBy("STATION_ID", "ASC");

    // Making the query
    // bo::MastQuery stationQuery;
    stationQuery.setQueryParams(&stationQueryParams);
    obs_engine->makeQuery(&stationQuery);

    // Store fmisids
    std::shared_ptr<bo::QueryResult> stationQueryResultContainer =
        stationQuery.getQueryResultContainer();
    if (not stationQueryResultContainer)
      return;

    try
    {
      auto stationIdIt =
          stationQueryResultContainer->begin("STATION_ID");
      auto stationIdItEnd =
          stationQueryResultContainer->end("STATION_ID");
      auto stationNameIt =
          stationQueryResultContainer->begin("STATION_NAME");
      auto stationStartIt =
          stationQueryResultContainer->begin("STATION_START");
      auto stationEndIt =
          stationQueryResultContainer->begin("STATION_END");
      auto longitudeIt =
          stationQueryResultContainer->begin("LONGITUDE");
      auto latitudeIt =
          stationQueryResultContainer->begin("LATITUDE");
      auto elevationIt =
          stationQueryResultContainer->begin("STATION_ELEVATION");
      auto countryIdIt =
          stationQueryResultContainer->begin("COUNTRY_ID");
      auto stationaryIt =
          stationQueryResultContainer->begin("STATIONARY");

      for (; stationIdIt != stationIdItEnd; ++stationIdIt,
                                            ++stationNameIt,
                                            ++stationStartIt,
                                            ++stationEndIt,
                                            ++longitudeIt,
                                            ++latitudeIt,
                                            ++elevationIt,
                                            ++countryIdIt,
                                            ++stationaryIt)
      {
        if ((*stationIdIt).type() == typeid(int64_t))
        {
          StationData s;
          s.station_id = std::any_cast<int64_t>(*stationIdIt);
          s.station_name = stationNameIt;
          s.station_start = stationStartIt;
          s.station_end = stationEndIt;
          s.longitude = longitudeIt;
          s.latitude = latitudeIt;
          s.elevation = elevationIt;
          s.country_id = countryIdIt;
          s.stationary = stationaryIt;

          validStations.insert(std::make_pair(std::to_string(s.station_id), s));
        }
      }
    }
    catch (...)
    {
      (void) Fmi::Exception::Trace(BCP, "Ignore exceptions here");
      return;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredEnvMonitoringFacilityQueryHandler::getStationCapabilities(
    bo::MastQuery &scQuery,
    StationCapabilityMap &stationCapabilityMap,
    const RequestParameterMap &params,
    const StationDataMap &validStations) const
{
  try
  {
    if (validStations.empty())
      return;

    std::vector<int64_t> storageIdVector;
    params.get<int64_t>(P_STORAGE_ID, std::back_inserter(storageIdVector));

    auto startTime = params.get_single<Fmi::DateTime>(P_BEGIN_TIME);
    auto endTime = params.get_single<Fmi::DateTime>(P_END_TIME);

    // Station capability query
    bo::MastQueryParams scQueryParams(dbRegistryConfig("OBSERVATIONS_V2"));
    scQueryParams.addJoinOnConfig(dbRegistryConfig("MEASURANDS_V1"), "MEASURAND_ID");

    scQueryParams.addField("STATION_ID");
    scQueryParams.addField("MEASURAND_ID");
    scQueryParams.addField("MEASURAND_CODE");
    scQueryParams.addField("MEASURAND_NAME");
    scQueryParams.addField("MEASURAND_LAYER");
    scQueryParams.addField("AGGREGATE_PERIOD");
    scQueryParams.addField("AGGREGATE_FUNCTION");
    scQueryParams.addField("EARLIEST_DATA");
    scQueryParams.addField("LATEST_DATA");
    scQueryParams.useDistinct();

    for (long it : storageIdVector)
      scQueryParams.addOperation("OR_GROUP_storage_id", "STORAGE_ID", "PropertyIsEqualTo", it);

    for (const auto & validStation : validStations)
      scQueryParams.addOperation(
          "OR_GROUP_station_id", "STATION_ID", "PropertyIsEqualTo", validStation.second.station_id);

    // Station operational activity period
    scQueryParams.addOperation(
        "OR_GROUP_station_latest_data", "LATEST_DATA", "PropertyIsGreaterThanOrEqualTo", startTime);
    scQueryParams.addOperation(
        "OR_GROUP_station_earliest_data", "EARLIEST_DATA", "PropertyIsLessThanOrEqualTo", endTime);

    // FIXME!! Language support for MEASURAND_NAME's

    scQueryParams.addOrderBy("STATION_ID", "ASC");
    scQueryParams.addOrderBy("MEASURAND_ID", "ASC");
    scQueryParams.addOrderBy("EARLIEST_DATA", "ASC");

    // Making the query
    scQuery.setQueryParams(&scQueryParams);
    obs_engine->makeQuery(&scQuery);

    // Mapping the query result
    std::shared_ptr<bo::QueryResult> scResultContainer = scQuery.getQueryResultContainer();
    auto scStationIdBeginIt =
        scResultContainer->begin("STATION_ID");
    auto scStationIdEndIt =
        scResultContainer->end("STATION_ID");
    auto scMeasurandIdIt =
        scResultContainer->begin("MEASURAND_ID");
    auto scMeasurandCodeIt =
        scResultContainer->begin("MEASURAND_CODE");
    auto scMeasurandNameIt =
        scResultContainer->begin("MEASURAND_NAME");
    auto scMeasurandLayerIt =
        scResultContainer->begin("MEASURAND_LAYER");
    auto scMeasurandAggregatePeriodIt =
        scResultContainer->begin("AGGREGATE_PERIOD");
    auto scMeasurandAggregateFunctionIt =
        scResultContainer->begin("AGGREGATE_FUNCTION");
    auto scEarliestDataIt =
        scResultContainer->begin("EARLIEST_DATA");
    auto scLatestDataIt =
        scResultContainer->begin("LATEST_DATA");

    for (auto scStationIdIt = scStationIdBeginIt;
         scStationIdIt != scStationIdEndIt;
         ++scStationIdIt,
                                                          ++scMeasurandIdIt,
                                                          ++scMeasurandCodeIt,
                                                          ++scMeasurandNameIt,
                                                          ++scMeasurandLayerIt,
                                                          ++scMeasurandAggregatePeriodIt,
                                                          ++scMeasurandAggregateFunctionIt,
                                                          ++scEarliestDataIt,
                                                          ++scLatestDataIt)
    {
      CapabilityData capabilityData;
      const std::string station_id = bo::QueryResult::toString(scStationIdIt);
      if (station_id.empty())
        continue;

      capabilityData.station_id = scStationIdIt;
      capabilityData.measurand_id = scMeasurandIdIt;
      capabilityData.measurand_code = scMeasurandCodeIt;
      capabilityData.measurand_name = scMeasurandNameIt;
      capabilityData.measurand_layer = scMeasurandLayerIt;
      capabilityData.aggregate_period = scMeasurandAggregatePeriodIt;
      capabilityData.aggregate_function = scMeasurandAggregateFunctionIt;
      capabilityData.earliest_data = scEarliestDataIt;
      capabilityData.latest_data = scLatestDataIt;

      auto mapIt = stationCapabilityMap.find(station_id);
      if (mapIt == stationCapabilityMap.end())
      {
        stationCapabilityMap.insert(std::make_pair(station_id, StationCapabilityData()));
        mapIt = stationCapabilityMap.find(station_id);
      }
      mapIt->second.push_back(capabilityData);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredEnvMonitoringFacilityQueryHandler::getStationGroupData(
    const std::string & /*language*/,
    SmartMet::Engine::Observation::MastQuery &sgQuery,
    bw::StoredEnvMonitoringFacilityQueryHandler::StationGroupMap &stationGroupMap,
    const RequestParameterMap &params,
    const bw::StoredEnvMonitoringFacilityQueryHandler::StationDataMap &validStations) const
{
  try
  {
    if (validStations.empty())
      return;

    auto startTime = params.get_single<Fmi::DateTime>(P_BEGIN_TIME);
    auto endTime = params.get_single<Fmi::DateTime>(P_END_TIME);

    std::vector<int64_t> classIdVector;
    params.get<int64_t>(P_CLASS_ID, std::back_inserter(classIdVector));

    bo::MastQueryParams sgQueryParams(dbRegistryConfig("GROUP_MEMBERS_V1"));
    sgQueryParams.addJoinOnConfig(dbRegistryConfig("STATION_GROUPS_V2"), "GROUP_ID");

    sgQueryParams.addField("STATION_ID");
    sgQueryParams.addField("GROUP_ID");
    sgQueryParams.addField("GROUP_NAME");

    // Ordering of the result
    sgQueryParams.addOrderBy("STATION_ID", "ASC");
    sgQueryParams.addOrderBy("GROUP_ID", "ASC");

    for (const auto & validStation : validStations)
      sgQueryParams.addOperation(
          "OR_GROUP_station", "STATION_ID", "PropertyIsEqualTo", validStation.second.station_id);

    for (auto &classId : classIdVector)
      sgQueryParams.addOperation("OR_GROUP_class_id", "CLASS_ID", "PropertyIsEqualTo", classId);

    // Select stations that match with the given time interval.
    sgQueryParams.addOperation(
        "OR_GROUP_valid_to", "VALID_TO", "PropertyIsGreaterThanOrEqualTo", startTime);
    sgQueryParams.addOperation(
        "OR_GROUP_valid_from", "VALID_FROM", "PropertyIsLessThanOrEqualTo", endTime);

    // Show group name in the requested language.
    // std::string lang = language;
    // SupportsLocationParameters::engOrFinToEnOrFi(lang);
    // sgQueryParams.addOperation("OR_GROUP_language_code", "LANGUAGE_CODE", "PropertyIsEqualTo",
    // lang);

    sgQuery.setQueryParams(&sgQueryParams);
    obs_engine->makeQuery(&sgQuery);

    std::shared_ptr<bo::QueryResult> resultContainer = sgQuery.getQueryResultContainer();
    auto stationIdBeginIt =
        resultContainer->begin("STATION_ID");
    auto stationIdEndIt =
        resultContainer->end("STATION_ID");
    auto groupIdIt = resultContainer->begin("GROUP_ID");
    auto groupNameIt =
        resultContainer->begin("GROUP_NAME");

    for (auto stationIdIt = stationIdBeginIt;
         stationIdIt != stationIdEndIt;
         ++stationIdIt, ++groupIdIt, ++groupNameIt)
    {
      StationGroup stationGroup;
      const std::string station_id = bo::QueryResult::toString(stationIdIt);
      if (station_id.empty())
        continue;

      stationGroup.station_id = bo::QueryResult::castTo<int64_t>(stationIdIt);
      stationGroup.group_id = groupIdIt;
      stationGroup.group_name = groupNameIt;

      auto mapIt = stationGroupMap.find(station_id);
      if (mapIt == stationGroupMap.end())
      {
        stationGroupMap.insert(std::make_pair(station_id, StationGroupVector()));
        mapIt = stationGroupMap.find(station_id);
      }
      mapIt->second.push_back(stationGroup);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredEnvMonitoringFacilityQueryHandler::getStationNetworkMembershipData(
    const std::string & /*language*/,
    SmartMet::Engine::Observation::MastQuery &emfQuery,
    NetworkMembershipMap &networkMemberShipMap,
    const RequestParameterMap &params,
    const bw::StoredEnvMonitoringFacilityQueryHandler::StationDataMap &validStations) const
{
  try
  {
    if (validStations.empty())
      return;

    auto startTime = params.get_single<Fmi::DateTime>(P_BEGIN_TIME);
    auto endTime = params.get_single<Fmi::DateTime>(P_END_TIME);

    bo::MastQueryParams emfQueryParams(dbRegistryConfig("NETWORK_MEMBERS_V1"));

    emfQueryParams.addField("STATION_ID");
    emfQueryParams.addField("NETWORK_ID");
    emfQueryParams.addField("MEMBER_CODE");

    // Ordering of the result
    emfQueryParams.addOrderBy("STATION_ID", "ASC");
    emfQueryParams.addOrderBy("NETWORK_ID", "ASC");

    for (const auto & validStation : validStations)
      emfQueryParams.addOperation(
          "OR_GROUP_station", "STATION_ID", "PropertyIsEqualTo", validStation.second.station_id);

    // Select stations that match with the given time interval.
    emfQueryParams.addOperation(
        "OR_GROUP_membership_end", "MEMBERSHIP_END", "PropertyIsGreaterThanOrEqualTo", startTime);
    emfQueryParams.addOperation(
        "OR_GROUP_membership_start", "MEMBERSHIP_START", "PropertyIsLessThanOrEqualTo", endTime);

    emfQuery.setQueryParams(&emfQueryParams);
    obs_engine->makeQuery(&emfQuery);

    std::shared_ptr<bo::QueryResult> resultContainer = emfQuery.getQueryResultContainer();
    auto stationIdBeginIt =
        resultContainer->begin("STATION_ID");
    auto stationIdEndIt =
        resultContainer->end("STATION_ID");
    auto networkIdIt =
        resultContainer->begin("NETWORK_ID");
    auto memberCodeIt =
        resultContainer->begin("MEMBER_CODE");

    for (auto stationIdIt = stationIdBeginIt;
         stationIdIt != stationIdEndIt;
         ++stationIdIt, ++networkIdIt, ++memberCodeIt)
    {
      NetworkMembership networkMemberShip;
      const std::string station_id = bo::QueryResult::toString(stationIdIt);
      if (station_id.empty())
        continue;

      networkMemberShip.station_id = std::any_cast<int64_t>(*stationIdIt);
      networkMemberShip.network_id = networkIdIt;
      networkMemberShip.member_code = memberCodeIt;

      auto mapIt = networkMemberShipMap.find(station_id);
      if (mapIt == networkMemberShipMap.end())
      {
        networkMemberShipMap.insert(std::make_pair(station_id, NetworkMembershipVector()));
        mapIt = networkMemberShipMap.find(station_id);
      }
      mapIt->second.push_back(networkMemberShip);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase>
wfs_stored_env_monitoring_facility_handler_create(SmartMet::Spine::Reactor *reactor,
                                                  StoredQueryConfig::Ptr config,
                                                  PluginImpl &plugin_data,
                                                  std::optional<std::string> template_file_name)
{
  try
  {
    auto *qh =
        new bw::StoredEnvMonitoringFacilityQueryHandler(
            reactor, config, plugin_data, template_file_name);
    std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> instance(qh);
    return instance;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef
    wfs_stored_env_monitoring_facility_handler_factory(
        &wfs_stored_env_monitoring_facility_handler_create);

/**

@page WFS_SQ_ENV_MONITORING_FACILITY_QUERY_HANDLER Stored Query handler for querying Environmental
Monitoring Facilities

@section WFS_SQ_ENV_MONITORING_FACILITY_QUERY_HANDLER_INTRO Introduction

This handler adds support for stored queries for ....

<table border="1">
  <tr>
     <td>Implementation</td>
     <td>SmartMet::Plugin::WFS::StoredEnvMonitoringFacilityQueryHandler</td>
  </tr>
  <tr>
    <td>Constructor name (for stored query configuration)</td>
    <td>@b wfs_stored_env_monitoring_facility_handler_factory</td>
  </tr>
</table>

@section WFS_SQ_ENV_MONITORING_FACILITY_QUERY_HANDLER_PARAMS Query handler built-in parameters

The following stored query handler parameters are in use

<table border="1">

<tr>
<th>Entry name</th>
<th>Type</th>
<th>Data type</th>
<th>Description</th>
</tr>

<tr>
  <td>classId</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>int</td>
  <td>Station groups are classified into classes. ClassId is a unique identity for a class.</td>
</tr>

<tr>
  <td>groupId</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>int</td>
  <td>A station is placed in groups depending on which kind of measurents it does. GroupId is a
unique identity for a group.</td>
</tr>

<tr>
  <td>stationId</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>int</td>
  <td>StationId is a unique local identity for a station.</td>
</tr>

<tr>
  <td>stationName</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>string</td>
  <td>StationName is just a name for a station.</td>
</tr>

<tr>
  <td>beginTime</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>time</td>
  <td>Specifies the begin time position of operational activity period.</td>
</tr>

<tr>
  <td>endTime</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>time</td>
  <td>Specifies the end time position of operational activity period.</td>
</tr>

<tr>
  <td>basePhenomenon</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>string</td>
  <td>The parameter can be used to restrict result to stations measuring the specific
phenomenon.</td>
</tr>

<tr>
  <td>aggregateFunction</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>string</td>
  <td>The parameter can be used to restrict result to stations that have measurands with the
specific aggregate function.</td>
</tr>

<tr>
  <td>aggregatePeriod</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>string</td>
  <td>The parameter can be used to restrict result to stations that have measurands with the
specific aggregate period.</td>
</tr>

<tr>
  <td>measurandCode</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>The parameter can be used to restrict result to stations that have measurands with the
specific measurand code.</td>
  <td></td>
</tr>

<tr>
  <td>storageId</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>int</td>
  <td>StoregeId can be used to restrict database search to specific storages </td>
</tr>

<tr>
  <td>inspireNamespace</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>string</td>
  <td></td>
</tr>

<tr>
  <td>authorityDomain</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>string</td>
  <td>Authority domain used for example in codeSpace names of XML response.</td>
</tr>

<tr>
  <td>showObservingCapability</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>bool</td>
  <td>Enable or disable observing capabilities to be shown in XML response.</td>
</tr>


</table>

*/

#endif  // WITHOUT_OBSERVATION
