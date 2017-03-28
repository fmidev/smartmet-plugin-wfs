#include "stored_queries/StoredEnvMonitoringNetworkQueryHandler.h"
#include "StoredQueryHandlerFactoryDef.h"
#include "SupportsLocationParameters.h"
#include "FeatureID.h"
#include <smartmet/spine/Exception.h>
#include <smartmet/engines/observation/DBRegistry.h>
#include <boost/icl/type_traits/to_string.hpp>
#include <memory>

namespace bw = SmartMet::Plugin::WFS;
namespace bo = SmartMet::Engine::Observation;

namespace
{
const char* P_CLASS_ID = "networkClassId";
const char* P_NETWORK_ID = "networkId";
const char* P_NETWORK_NAME = "networkName";
const char* P_STATION_ID = "stationId";
const char* P_STATION_NAME = "stationName";
const char* P_MISSING_TEXT = "missingText";
const char* P_INSPIRE_NAMESPACE = "inspireNamespace";
const char* P_AUTHORITY_DOMAIN = "authorityDomain";
}

bw::StoredEnvMonitoringNetworkQueryHandler::StoredEnvMonitoringNetworkQueryHandler(
    SmartMet::Spine::Reactor* reactor,
    boost::shared_ptr<StoredQueryConfig> config,
    PluginData& plugin_data,
    boost::optional<std::string> template_file_name)
    : bw::SupportsExtraHandlerParams(config),
      bw::StoredQueryHandlerBase(reactor, config, plugin_data, template_file_name),
      m_geoEngine(NULL),
      m_obsEngine(NULL)
{
  try
  {
    register_array_param<int64_t>(P_CLASS_ID, *config);
    register_array_param<int64_t>(P_NETWORK_ID, *config);
    register_array_param<std::string>(P_NETWORK_NAME, *config, false);
    register_array_param<int64_t>(P_STATION_ID, *config);
    register_array_param<std::string>(P_STATION_NAME, *config, false);
    register_scalar_param<std::string>(P_INSPIRE_NAMESPACE, *config);
    register_scalar_param<std::string>(P_AUTHORITY_DOMAIN, *config);
    m_missingText = config->get_optional_config_param<std::string>(P_MISSING_TEXT, "NaN");
    m_debugLevel = config->get_debug_level();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

bw::StoredEnvMonitoringNetworkQueryHandler::~StoredEnvMonitoringNetworkQueryHandler()
{
}

void bw::StoredEnvMonitoringNetworkQueryHandler::init_handler()
{
  try
  {
    auto* reactor = get_reactor();

    void* engine;

    // Get Geonames
    engine = reactor->getSingleton("Geonames", NULL);
    if (engine == NULL)
      throw SmartMet::Spine::Exception(BCP, "No Geonames engine available");

    m_geoEngine = reinterpret_cast<SmartMet::Engine::Geonames::Engine*>(engine);

    // Get Observation
    engine = reactor->getSingleton("Observation", NULL);
    if (engine == NULL)
      throw SmartMet::Spine::Exception(BCP, "No Observation engine available");

    m_obsEngine = reinterpret_cast<SmartMet::Engine::Observation::Engine*>(engine);

    // Set Geonames into Observation
    m_obsEngine->setGeonames(m_geoEngine);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::StoredEnvMonitoringNetworkQueryHandler::query(const StoredQuery& query,
                                                       const std::string& language,
                                                       std::ostream& output) const
{
  try
  {
    const auto& params = query.get_param_map();
    auto inspireNamespace = params.get_single<std::string>(P_INSPIRE_NAMESPACE);
    auto authorityDomain = params.get_single<std::string>(P_AUTHORITY_DOMAIN);

    // Get the sequence number of query in the request
    auto sqId = query.get_query_id();
    FeatureID featureId(get_config()->get_query_id(), params.get_map(), sqId);

    // Removing some feature id parameters
    const char* removeParams[] = {P_NETWORK_ID, P_NETWORK_NAME, P_STATION_ID, P_STATION_NAME};
    for (unsigned i = 0; i < sizeof(removeParams) / sizeof(*removeParams); i++)
    {
      featureId.erase_param(removeParams[i]);
    }

    if (m_debugLevel > 0)
      query.dump_query_info(std::cout);

    std::string lang = language;
    bw::SupportsLocationParameters::engOrFinToEnOrFi(lang);

    // Get request parameters.
    std::vector<int64_t> classIdVector;
    params.get<int64_t>(P_CLASS_ID, std::back_inserter(classIdVector));

    std::vector<int64_t> networkIdVector;
    params.get<int64_t>(P_NETWORK_ID, std::back_inserter(networkIdVector));

    std::vector<std::string> networkNameVector;
    params.get<std::string>(P_NETWORK_NAME, std::back_inserter(networkNameVector));

    std::vector<int64_t> stationIdVector;
    params.get<int64_t>(P_STATION_ID, std::back_inserter(stationIdVector));

    std::vector<std::string> stationNameVector;
    params.get<std::string>(P_STATION_NAME, std::back_inserter(stationNameVector));

    // Using GROUP_MEMBERS_V1 as a base configuration
    bo::MastQueryParams emnQueryParams(dbRegistryConfig("GROUP_MEMBERS_V1"));
    emnQueryParams.addField("GROUP_ID");

    // Join on NETWORKS_V1 view
    emnQueryParams.addJoinOnConfig(dbRegistryConfig("STATION_GROUPS_V2"), "GROUP_ID");
    emnQueryParams.addField("GROUP_CODE");
    emnQueryParams.addField("GROUP_NAME");
    emnQueryParams.addField("GROUP_DESC");

    if (not stationNameVector.empty())
    {
      // Join on STATION_NAMES_V1 view. Needed for search of netwprk by STATION_NAME
      emnQueryParams.addJoinOnConfig(dbRegistryConfig("STATION_NAMES_V1"), "STATION_ID");
    }

    emnQueryParams.addOrderBy("GROUP_ID", "ASC");
    emnQueryParams.useDistinct();

    for (auto& classId : classIdVector)
      emnQueryParams.addOperation("OR_GROUP_class_id", "CLASS_ID", "PropertyIsEqualTo", classId);

    for (std::vector<int64_t>::const_iterator it = networkIdVector.begin();
         it != networkIdVector.end();
         ++it)
      emnQueryParams.addOperation("OR_GROUP_network", "GROUP_ID", "PropertyIsEqualTo", *it);

    for (std::vector<std::string>::const_iterator it = networkNameVector.begin();
         it != networkNameVector.end();
         ++it)
      emnQueryParams.addOperation("OR_GROUP_network", "GROUP_NAME", "PropertyIsLike", *it);

    for (std::vector<int64_t>::const_iterator it = stationIdVector.begin();
         it != stationIdVector.end();
         ++it)
      emnQueryParams.addOperation("OR_GROUP_network", "STATION_ID", "PropertyIsEqualTo", *it);

    if (not stationNameVector.empty())
    {
      for (std::vector<std::string>::const_iterator it = stationNameVector.begin();
           it != stationNameVector.end();
           ++it)
        emnQueryParams.addOperation("OR_GROUP_network", "STATION_NAME", "PropertyIsLike", *it);
    }

    emnQueryParams.addOperation(
        "OR_GROUP_language_code", "LANGUAGE_CODE", "PropertyIsEqualTo", lang);

    // bo::EnvironmentalMonitoringFacilityQuery emnQuery;
    bo::MastQuery emnQuery;
    emnQuery.setQueryParams(&emnQueryParams);
    m_obsEngine->makeQuery(&emnQuery);

    CTPP::CDT hash;
    params.dump_params(hash["query_parameters"]);
    dump_named_params(params, hash["named_parameters"]);
    hash["authorityDomain"] = authorityDomain;
    hash["responseTimestamp"] =
        boost::posix_time::to_iso_extended_string(get_plugin_data().get_time_stamp()) + "Z";
    hash["queryId"] = query.get_query_id();

    // Container with the data fetched from obsengine.
    std::shared_ptr<bo::QueryResult> resultContainer = emnQuery.getQueryResultContainer();

    int networkCount = 0;

    if (resultContainer)
    {
      bo::QueryResult::ValueVectorType::const_iterator netIdIt = resultContainer->begin("GROUP_ID");
      bo::QueryResult::ValueVectorType::const_iterator netIdItEnd =
          resultContainer->end("GROUP_ID");
      bo::QueryResult::ValueVectorType::const_iterator netCodeIt =
          resultContainer->begin("GROUP_CODE");
      bo::QueryResult::ValueVectorType::const_iterator netNameIt =
          resultContainer->begin("GROUP_NAME");
      bo::QueryResult::ValueVectorType::const_iterator netDescIt =
          resultContainer->begin("GROUP_DESC");

      size_t netNameVectorSize = resultContainer->size("GROUP_NAME");
      size_t netIdVectorSize = netIdItEnd - netIdIt;
      std::string networkId;

      if (netIdVectorSize == netNameVectorSize)
      {
        // Filling the network and station data
        int networkCounter = 0;
        for (; netIdIt != netIdItEnd; ++netIdIt, ++netCodeIt, ++netNameIt, ++netDescIt)
        {
          const std::string netId = bo::QueryResult::toString(netIdIt);
          if (networkId != netId)
          {
            networkCount++;
            networkCounter++;
            hash["networks"][networkCounter - 1]["id"] = netId;
            const std::string netCode = bo::QueryResult::toString(netCodeIt);
            if (not netCode.empty())
              hash["networks"][networkCounter - 1]["code"] = netCode;
            hash["networks"][networkCounter - 1]["name"] = bo::QueryResult::toString(netNameIt);
            hash["networks"][networkCounter - 1]["description"] =
                bo::QueryResult::toString(netDescIt);
            hash["networks"][networkCounter - 1]["inspireNamespace"] = inspireNamespace;

            featureId.add_param(P_NETWORK_ID, netId);
            hash["networks"][networkCounter - 1]["featureId"] = featureId.get_id();
            featureId.erase_param(P_NETWORK_ID);
          }
          networkId = netId;
        }
      }
      else
      {
        std::ostringstream msg;
        msg << "warning: bw::StoredEnvMonitoringNetworkQueryHandler::query - varying size data "
               "vectors!\n";
        std::cerr << msg.str();
      }
    }

    const std::string networksMatched = boost::to_string(networkCount);
    hash["networksMatched"] = networksMatched;
    hash["networksReturned"] = networksMatched;

    hash["fmi_apikey"] = QueryBase::FMI_APIKEY_SUBST;
    hash["fmi_apikey_prefix"] = bw::QueryBase::FMI_APIKEY_PREFIX_SUBST;
    hash["hostname"] = QueryBase::HOSTNAME_SUBST;
    hash["language"] = language;

    // Format output
    format_output(hash, output, query.get_use_debug_format());

  }  // In case of some other failures
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation processing failed!", NULL);
    if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == NULL)
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
    exception.addParameter(WFS_LANGUAGE, language);
    throw exception;
  }
}

const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig>
bw::StoredEnvMonitoringNetworkQueryHandler::dbRegistryConfig(const std::string& configName) const
{
  try
  {
    // Get database registry from Observation

    const std::shared_ptr<SmartMet::Engine::Observation::DBRegistry> dbRegistry =
        m_obsEngine->dbRegistry();
    if (not dbRegistry)
    {
      SmartMet::Spine::Exception exception(BCP, "Database registry is not available!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      throw exception;
    }

    const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbrConfig =
        dbRegistry->dbRegistryConfig(configName);
    if (not dbrConfig)
    {
      SmartMet::Spine::Exception exception(BCP,
                                           "Database registry configuration is not available!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      exception.addParameter("Config name", configName);
      throw exception;
    }

    return dbrConfig;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase>
wfs_stored_env_monitoring_network_handler_create(SmartMet::Spine::Reactor* reactor,
                                                 boost::shared_ptr<StoredQueryConfig> config,
                                                 PluginData& plugin_data,
                                                 boost::optional<std::string> template_file_name)
{
  try
  {
    bw::StoredEnvMonitoringNetworkQueryHandler* qh = new bw::StoredEnvMonitoringNetworkQueryHandler(
        reactor, config, plugin_data, template_file_name);
    boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> instance(qh);
    instance->init_handler();
    return instance;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}
}

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef
    wfs_stored_env_monitoring_network_handler_factory(
        &wfs_stored_env_monitoring_network_handler_create);

/**

@page WFS_SQ_ENV_MONITORING_NETWORK_QUERY_HANDLER Stored Query handler for querying Environmental
Monitoring Facilities

@section WFS_SQ_ENV_MONITORING_NETWORK_QUERY_HANDLER_INTRO Introduction

This handler adds support for stored queries for ....

<table border="1">
  <tr>
     <td>Implementation</td>
     <td>SmartMet::Plugin::WFS::StoredEnvMonitoringNetworkQueryHandler</td>
  </tr>
  <tr>
    <td>Constructor name (for stored query configuration)</td>
    <td>@b wfs_stored_env_monitoring_network_handler_factory</td>
  </tr>
</table>

@section WFS_SQ_ENV_MONITORING_NETWORK_QUERY_HANDLER_PARAMS Query handler built-in parameters

The following stored query handler are in use

<table border="1">

<tr>
<th>Entry name</th>
<th>Type</th>
<th>Data type</th>
<th>Description</th>
</tr>

<tr>
  <td>networkname</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>string</td>
  <td>Network name.</td>
</tr>

<tr>
  <td>networkid</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>int</td>
  <td>Network id.</td>
</tr>


</table>

*/
