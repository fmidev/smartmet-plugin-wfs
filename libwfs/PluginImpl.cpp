#include "PluginImpl.h"
#include "ErrorResponseGenerator.h"
#include "HandlerFactorySummary.h"
#include "WfsConst.h"
#include "XmlParser.h"
#include "request/DescribeFeatureType.h"
#include "request/DescribeStoredQueries.h"
#include "request/GetCapabilities.h"
#include "request/GetFeature.h"
#include "request/GetPropertyValue.h"
#include "request/ListStoredQueries.h"
#include <boost/algorithm/string.hpp>
#include <boost/bind/bind.hpp>
#include <macgyver/DateTime.h>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <json/json.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <macgyver/TimeParser.h>
#include <spine/CRSRegistry.h>
#include <spine/Convenience.h>
#include <spine/FmiApiKey.h>
#include <spine/HostInfo.h>

using namespace SmartMet::Plugin::WFS;
namespace ba = boost::algorithm;
namespace bl = boost::lambda;
namespace ph = boost::placeholders;

struct PluginImpl::RequestResult
{
  SmartMet::Spine::HTTP::Status status{SmartMet::Spine::HTTP::not_a_status};
  bool may_validate_xml{true};
  std::ostringstream output;
  std::optional<int> expires_seconds;

 public:
  RequestResult() :  output() {}
};

PluginImpl::PluginImpl(SmartMet::Spine::Reactor* theReactor,
                       const char* theConfig,
                       Spine::CRSRegistry& crs_registry)

    : itsConfig(theConfig), itsCRSRegistry(crs_registry), wfs_capabilities(new WfsCapabilities)
{
  try
  {
    if (theConfig == nullptr)
    {
      std::ostringstream msg;
      msg << "ERROR: No configuration provided for WFS plugin";
      throw Fmi::Exception(BCP, msg.str());
    }

    query_cache.reset(new QueryResponseCache(
        itsConfig.getCacheSize(), std::chrono::seconds(itsConfig.getCacheTimeConstant())));

    request_factory.reset(new RequestFactory(*this));

    request_factory

        ->register_request_type(
            "GetCapabilities",
            "",
            boost::bind(&PluginImpl::parse_kvp_get_capabilities_request, this, ph::_1, ph::_2),
            boost::bind(
                &PluginImpl::parse_xml_get_capabilities_request, this, ph::_1, ph::_2, ph::_3))

        .register_request_type(
            "DescribeFeatureType",
            "supportsDescribeFeatureType",
            boost::bind(&PluginImpl::parse_kvp_describe_feature_type_request, this, ph::_1, ph::_2),
            boost::bind(
                &PluginImpl::parse_xml_describe_feature_type_request, this, ph::_1, ph::_2, ph::_3))

        .register_request_type(
            "GetFeature",
            "supportsGetFeature",
            boost::bind(&PluginImpl::parse_kvp_get_feature_request, this, ph::_1, ph::_2),
            boost::bind(&PluginImpl::parse_xml_get_feature_request, this, ph::_1, ph::_2, ph::_3))

        .register_request_type(
            "GetPropertyValue",
            "supportsGetPropertyValue",
            boost::bind(&PluginImpl::parse_kvp_get_property_value_request, this, ph::_1, ph::_2),
            boost::bind(
                &PluginImpl::parse_xml_get_property_value_request, this, ph::_1, ph::_2, ph::_3))

        .register_request_type(
            "ListStoredQueries",
            "supportsListStoredQueries",
            boost::bind(&PluginImpl::parse_kvp_list_stored_queries_request, this, ph::_1, ph::_2),
            boost::bind(
                &PluginImpl::parse_xml_list_stored_queries_request, this, ph::_1, ph::_2, ph::_3))

        .register_request_type(
            "DescribeStoredQueries",
            "supportsDescribeStoredQueries",
            boost::bind(
                &PluginImpl::parse_kvp_describe_stored_queries_request, this, ph::_1, ph::_2),
            boost::bind(&PluginImpl::parse_xml_describe_stored_queries_request,
                        this,
                        ph::_1,
                        ph::_2,
                        ph::_3))

        .register_unimplemented_request_type("LockFeature")
        .register_unimplemented_request_type("GetFeatureWithLock")
        .register_unimplemented_request_type("CreateStoredQuery")
        .register_unimplemented_request_type("DropStoredQuery")
        .register_unimplemented_request_type("Transaction");

    // itsGisEngine = theReactor->getEngine<SmartMet::Engine::Gis::Engine>("Gis");

    debug_level = itsConfig.get_optional_config_param<int>("debugLevel", 1);
    fallback_hostname =
        itsConfig.get_optional_config_param<std::string>("fallback_hostname", "localhost");
    fallback_protocol =
        itsConfig.get_optional_config_param<std::string>("fallback_protocol", "http");
    primary_data_source =
        itsConfig.get_optional_config_param<std::string>("primaryForecastSource", "querydata");
    gridengine_disabled = itsConfig.get_optional_config_param<bool>("gridengine_disabled", false);

    create_template_formatters();
    create_xml_parser();
    init_geo_server_access();

    const auto& feature_vect = itsConfig.read_features_config(itsCRSRegistry);
    for (auto feature : feature_vect)
    {
      wfs_capabilities->register_feature(feature);
    }

    create_typename_stored_query_map();

    create_stored_query_map(theReactor);

    std::string s_locked_timestamp;
    if (itsConfig.get_config().lookupValue("lockedTimeStamp", s_locked_timestamp))
    {
      locked_time_stamp = Fmi::TimeParser::parse(s_locked_timestamp);
      std::cout << "*****************************************************************\n";
      std::cout << "Using fixed timestamp "
                << (Fmi::to_iso_extended_string(*locked_time_stamp) + "Z") << std::endl;
      std::cout << "*****************************************************************\n";
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::shutdown()
{
  stored_query_map->shutdown();
}

PluginImpl::~PluginImpl() = default;

Fmi::DateTime PluginImpl::get_time_stamp() const
{
  try
  {
    if (locked_time_stamp)
    {
      return *locked_time_stamp;
    }
    else
    {
      return Fmi::SecondClock::universal_time();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::DateTime PluginImpl::get_local_time_stamp() const
{
  try
  {
    if (locked_time_stamp)
    {
      return *locked_time_stamp;
    }
    else
    {
      return Fmi::SecondClock::universal_time();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<GeoServerDB> PluginImpl::get_geo_server_database() const
{
  if (geo_server_db)
  {
    return geo_server_db;
  }
  else
  {
    Fmi::Exception exception(BCP, "GeoServer database is not available");
    exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
    throw exception;
  }
}

void PluginImpl::create_template_formatters()
{
  try
  {
    const auto gctn =
        itsConfig.get_mandatory_config_param<std::string>("getCapabilitiesTemplate");

    const auto lsqtn =
        itsConfig.get_mandatory_config_param<std::string>("listStoredQueriesTemplate");

    const auto dsqtn =
        itsConfig.get_mandatory_config_param<std::string>("describeStoredQueriesTemplate");

    const auto fttn =
        itsConfig.get_mandatory_config_param<std::string>("featureTypeTemplate");

    const auto etn = itsConfig.get_mandatory_config_param<std::string>("exceptionTemplate");

    const auto dfn = itsConfig.get_mandatory_config_param<std::string>("ctppDumpTemplate");

    const auto template_dir = itsConfig.get_template_directory();

    // We do not actually create the formatters since using them is not
    // thread safe. Instead, we store the name and ask the formatter
    // factory for a thread local copy based on the name

    getCapabilitiesFormatterPath = template_dir / gctn;
    listStoredQueriesFormatterPath = template_dir / lsqtn;
    describeStoredQueriesFormatterPath = template_dir / dsqtn;
    featureTypeFormatterPath = template_dir / fttn;
    exceptionFormatterPath = template_dir / etn;
    ctppDumpFormatterPath = template_dir / dfn;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::create_xml_parser()
{
  try
  {
    const std::string xml_grammar_pool_fn = itsConfig.getXMLGrammarPoolDumpFn();
    std::cout << "\t\t+ [Using XML grammar pool dump file '" << xml_grammar_pool_fn << "']"
              << std::endl;
    xml_parser.reset(new Xml::ParserMT(xml_grammar_pool_fn, false));

    std::string serialized_xml_schemas = itsConfig.get_optional_path("serializedXmlSchemas", "");
    if (!serialized_xml_schemas.empty())
    {
      xml_parser->load_schema_cache(serialized_xml_schemas);
    }

    if (itsConfig.getValidateXmlOutput())
    {
      std::cout << "\t\t+ [Enabling XML schema download (";
      if (!itsConfig.getProxy().empty())
      {
        std::cout << "proxy='" << itsConfig.getProxy() << '\'';
      }
      if (!itsConfig.getNoProxy().empty())
      {
        std::cout << "no_proxy='" << itsConfig.getNoProxy() << '\'';
      }
      std::cout << ")]" << std::endl;

      xml_parser->enable_schema_download(itsConfig.getProxy(), itsConfig.getNoProxy());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::init_geo_server_access()
{
  try
  {
    try
    {
      const std::string geoserver_conn_str = itsConfig.get_geoserver_conn_string();
      if (!geoserver_conn_str.empty())
      {
        geo_server_db.reset(new GeoServerDB(itsConfig.get_geoserver_conn_string(), 5));
      }
    }
    catch (...)
    {
      Fmi::Exception exception(BCP, "Failed to connect to GeoServer!", nullptr);
      exception.addParameter("Connection string", itsConfig.get_geoserver_conn_string());
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::create_typename_stored_query_map()
{
  try
  {
    type_name_stored_query_map.reset(new TypeNameStoredQueryMap);

    std::map<std::string, std::string> typename_storedqry;
    itsConfig.read_typename_config(typename_storedqry);

    type_name_stored_query_map->init(typename_storedqry);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::create_stored_query_map(SmartMet::Spine::Reactor* theReactor)
{
  try
  {
    const std::vector<std::string>& sq_config_dirs = itsConfig.getStoredQueriesConfigDirs();

    stored_query_map.reset(new StoredQueryMap(theReactor, *this));

    int parallel = itsConfig.get_optional_config_param<int>("parallelInit", 0);
    stored_query_map->set_background_init(parallel);

    for (const auto& sq_config_dir : sq_config_dirs)
    {
      stored_query_map->add_config_dir(sq_config_dir, itsConfig.get_template_directory());
    }

    stored_query_map->wait_for_init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_kvp_get_capabilities_request(
    const std::string& language, const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    return Request::GetCapabilities::create_from_kvp(language, request, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_xml_get_capabilities_request(const std::string& language,
                                                            const xercesc::DOMDocument& document,
                                                            const xercesc::DOMElement& root)
{
  try
  {
    (void)root;

    return Request::GetCapabilities::create_from_xml(language, document, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_kvp_describe_feature_type_request(
    const std::string& language, const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    return Request::DescribeFeatureType::create_from_kvp(language, request, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_xml_describe_feature_type_request(
    const std::string& language,
    const xercesc::DOMDocument& document,
    const xercesc::DOMElement& root)
{
  try
  {
    (void)root;
    return Request::DescribeFeatureType::create_from_xml(language, document, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_kvp_get_feature_request(
    const std::string& language, const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    return Request::GetFeature::create_from_kvp(language, request, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_xml_get_feature_request(const std::string& language,
                                                       const xercesc::DOMDocument& document,
                                                       const xercesc::DOMElement& root)
{
  try
  {
    (void)root;
    return Request::GetFeature::create_from_xml(language, document, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_kvp_get_property_value_request(
    const std::string& language, const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    return Request::GetPropertyValue::create_from_kvp(language, request, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_xml_get_property_value_request(const std::string& language,
                                                              const xercesc::DOMDocument& document,
                                                              const xercesc::DOMElement& root)
{
  try
  {
    (void)root;
    return Request::GetPropertyValue::create_from_xml(language, document, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_kvp_list_stored_queries_request(
    const std::string& language, const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    return Request::ListStoredQueries::create_from_kvp(language, request, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_xml_list_stored_queries_request(const std::string& language,
                                                               const xercesc::DOMDocument& document,
                                                               const xercesc::DOMElement& root)
{
  try
  {
    (void)root;
    return Request::ListStoredQueries::create_from_xml(language, document, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_kvp_describe_stored_queries_request(
    const std::string& language, const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    return Request::DescribeStoredQueries::create_from_kvp(language, request, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

RequestBaseP PluginImpl::parse_xml_describe_stored_queries_request(
    const std::string& language,
    const xercesc::DOMDocument& document,
    const xercesc::DOMElement& root)
{
  try
  {
    (void)root;
    return Request::DescribeStoredQueries::create_from_xml(language, document, *this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/**
 *  @brief Perform actual WFS request and generate the response
 */
void PluginImpl::query(const std::string& req_language,
                       const SmartMet::Spine::HTTP::Request& req,
                       PluginImpl::RequestResult& result)
{
  try
  {
    const auto method = req.getMethod();

    std::string hostname;
    if (const auto header_x_forwarded_host = req.getHeader("X-Forwarded-Host"))
      hostname = *header_x_forwarded_host;
    else if (const auto header_host = req.getHeader("Host"))
      hostname = *header_host;
    else
      hostname = get_fallback_hostname();

    std::string protocol;
    if (const auto header_x_forwarded_protocol = req.getProtocol())
      protocol = *header_x_forwarded_protocol;
    else
      protocol = get_fallback_protocol();

    const std::string fmi_apikey_prefix = "/fmi-apikey/";

    std::string language =
        req_language.empty() ? *get_config().get_languages().begin() : req_language;

    if (method == SmartMet::Spine::HTTP::RequestMethod::GET)
    {
      std::shared_ptr<RequestBase> request = request_factory->parse_kvp(language, req);
      request->set_hostname(hostname);
      request->set_protocol(protocol);
      auto fmi_apikey = get_fmi_apikey(req);
      if (fmi_apikey)
      {
        request->set_fmi_apikey_prefix(fmi_apikey_prefix);
        request->set_fmi_apikey(*fmi_apikey);
      }
      result.may_validate_xml = request->may_validate_xml();
      request->execute(result.output);
      result.expires_seconds = request->get_response_expires_seconds();
      if (request->get_http_status())
        result.status = request->get_http_status();
    }

    else if (method == SmartMet::Spine::HTTP::RequestMethod::POST)
    {
      const std::string content_type = get_mandatory_header(req, "Content-Type");

      auto fmi_apikey = get_fmi_apikey(req);

      if (content_type == "application/x-www-form-urlencoded")
      {
        std::shared_ptr<RequestBase> request = request_factory->parse_kvp(language, req);
        request->set_hostname(hostname);
        request->set_protocol(protocol);
        if (fmi_apikey)
        {
          request->set_fmi_apikey_prefix(fmi_apikey_prefix);
          request->set_fmi_apikey(*fmi_apikey);
        }
        result.may_validate_xml = request->may_validate_xml();
        request->execute(result.output);
        result.expires_seconds = request->get_response_expires_seconds();
        if (request->get_http_status())
          result.status = request->get_http_status();
      }
      else if (content_type == "text/xml")
      {
        const std::string& content = req.getContent();
        if (content.empty())
        {
          Fmi::Exception exception(BCP, "No request content available!");
          exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
          throw exception;
        }

        std::shared_ptr<xercesc::DOMDocument> xml_doc;
        Xml::Parser::RootElemInfo root_info;
        try
        {
          Xml::Parser::root_element_cb_t root_element_cb = (bl::var(root_info) = bl::_1);
          xml_doc = xml_parser->get()->parse_string(content, "WFS", root_element_cb);
        }
        catch (const Xml::XmlError& orig_err)
        {
          const std::list<std::string>& messages = orig_err.get_messages();

          if ((orig_err.get_error_level() != Xml::XmlError::FATAL_ERROR) and
              (!root_info.nqname.empty()) and (root_info.ns_uri == WFS_NAMESPACE_URI) and
              request_factory->check_request_name(root_info.nqname))
          {
            if (root_info.attr_map.count("service") == 0)
            {
              Fmi::Exception exception(BCP, "Missing the 'service' attribute!", nullptr);
              exception.addDetails(messages);
              if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
                exception.addParameter(WFS_EXCEPTION_CODE, WFS_MISSING_PARAMETER_VALUE);
              exception.addParameter(WFS_LOCATION, "service");
              exception.addParameter(WFS_LANGUAGE, req_language);
              throw exception;
            }
            else if (root_info.attr_map.at("service") != "WFS")
            {
              Fmi::Exception exception(
                  BCP, "Incorrect value for the 'service' attribute!", nullptr);
              exception.addDetails(messages);
              if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
                exception.addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
              exception.addParameter(WFS_LOCATION, "service");
              exception.addParameter(WFS_LANGUAGE, req_language);
              exception.addParameter("Attribute value", root_info.attr_map.at("service"));
              exception.addParameter("Expected value", "WFS");
              throw exception;
            }

            if (root_info.attr_map.count("version") == 0)
            {
              Fmi::Exception exception(BCP, "Missing the 'version' attribute!", nullptr);
              exception.addDetails(messages);
              if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
                exception.addParameter(WFS_EXCEPTION_CODE, WFS_MISSING_PARAMETER_VALUE);
              exception.addParameter(WFS_LOCATION, "version");
              exception.addParameter(WFS_LANGUAGE, req_language);
              throw exception;
            }
            else if (root_info.attr_map.at("version") != "2.0.0")
            {
              Fmi::Exception exception(
                  BCP, "Incorrect value for the 'version' attribute!", nullptr);
              exception.addDetails(messages);
              if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
                exception.addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
              exception.addParameter(WFS_LOCATION, "version");
              exception.addParameter(WFS_LANGUAGE, req_language);
              exception.addParameter("Attribute value", root_info.attr_map.at("version"));
              exception.addParameter("Expected value", "2.0.0");
              throw exception;
            }
          }

          Fmi::Exception exception(BCP, "Parsing of the incoming XML request failed", nullptr);
          exception.addDetails(messages);
          if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
            exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
          exception.addParameter(WFS_LANGUAGE, req_language);
          throw exception;
        }

        std::shared_ptr<RequestBase> request = request_factory->parse_xml(language, *xml_doc);
        request->set_hostname(hostname);
        request->set_protocol(protocol);
        if (fmi_apikey)
        {
          request->set_fmi_apikey_prefix(fmi_apikey_prefix);
          request->set_fmi_apikey(*fmi_apikey);
        }
        result.may_validate_xml = request->may_validate_xml();
        request->execute(result.output);
        result.expires_seconds = request->get_response_expires_seconds();
        if (request->get_http_status())
          result.status = request->get_http_status();
      }
      else
      {
        Fmi::Exception exception(BCP, "Unsupported content type!");
        if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
          exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
        exception.addParameter(WFS_LANGUAGE, req_language);
        exception.addParameter("Requested content type", content_type);
        throw exception;
      }
    }
    else
    {
      Fmi::Exception exception(BCP,
                               "HTTP method '" + req.getMethodString() + "' is not supported!");
      if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      exception.addParameter(WFS_LANGUAGE, req_language);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Query failed!");
  }
}

std::optional<std::string> PluginImpl::get_fmi_apikey(
    const SmartMet::Spine::HTTP::Request& theRequest) const
{
  try
  {
    return SmartMet::Spine::FmiApiKey::getFmiApiKey(theRequest, true);
  }
  catch (...)
  {
    throw;
  }
}

void PluginImpl::realRequestHandler(SmartMet::Spine::Reactor& /* theReactor */,
                                    const std::string& language,
                                    const SmartMet::Spine::HTTP::Request& theRequest,
                                    SmartMet::Spine::HTTP::Response& theResponse)
{
  try
  {
    // Now

    Fmi::DateTime t_now = Fmi::SecondClock::universal_time();

    try
    {
      std::ostringstream output;
      RequestResult result;
      query(language, theRequest, result);
      const std::string& content = result.output.str();
      theResponse.setContent(content);
      auto status = result.status ? result.status : SmartMet::Spine::HTTP::ok;
      theResponse.setStatus(status);

      // Latter (false) should newer happen.
      const int expires_seconds = (result.expires_seconds)
                                      ? *result.expires_seconds
                                      : get_config().getDefaultExpiresSeconds();

      // Build cache expiration time info

      Fmi::DateTime t_expires = t_now + Fmi::Seconds(expires_seconds);

      // The headers themselves

      std::shared_ptr<Fmi::TimeFormatter> tformat(Fmi::TimeFormatter::create("http"));

      // std::string mime = "text/xml; charset=UTF-8";
      const std::string mime =
          content.substr(0, 6) == "<html>" ? "text/html; charset=UTF-8" : "text/xml; charset=UTF-8";
      theResponse.setHeader("Content-Type", mime);

      if (theResponse.getContentLength() == 0)
      {
        std::ostringstream msg;
        msg << "Warning: Empty input for request " << theRequest.getQueryString() << " from "
            << theRequest.getClientIP() << std::endl;
        Fmi::Exception exception(BCP, msg.str());
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
        throw exception;
      }

      if (status == SmartMet::Spine::HTTP::ok)
      {
        std::string cachecontrol = "public, max-age=" + Fmi::to_string(expires_seconds);
        std::string expiration = tformat->format(t_expires);
        std::string modification = tformat->format(t_now);

        theResponse.setHeader("Cache-Control", cachecontrol);
        theResponse.setHeader("Expires", expiration);
        theResponse.setHeader("Last-Modified", modification);
        theResponse.setHeader("Access-Control-Allow-Origin", "*");
      }

      if (result.may_validate_xml)
        try
        {
          maybe_validate_output(theRequest, theResponse);
        }
        catch (...)
        {
          auto err = Fmi::Exception::Trace(BCP, "Response XML validation failed");
          if (get_config().getFailOnValidateErrors())
          {
            std::ostringstream msg;
            const std::string content = theResponse.getContent();
            std::vector<std::string> lines;
            ba::split(lines, content, ba::is_any_of("\n"));
            msg << "########################################################################\n"
                << "# Validation of XML response has failed\n"
                << "########################################################################\n"
                << theRequest.toString() << '\n'
                << "########################################################################\n";
            for (std::size_t i = 0; i < lines.size(); i++)
            {
              msg << (boost::format("%06d: %s\n") % (i + 1) % lines.at(i)).str();
            }
            msg << "########################################################################\n";
            msg << err.getStackTrace();
            msg << "########################################################################\n";
            std::cout << msg.str();
            err.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
            throw err;
          }
          else
          {
            Fmi::Exception::Trace(BCP, "Response validation failed!").printError();
          }
        }
    }
    catch (...)
    {
      // Catching all exceptions

      Fmi::Exception exception(BCP, "Request processing exception!", nullptr);
      exception.addParameter("URI", theRequest.getURI());
      exception.addParameter("ClientIP", theRequest.getClientIP());
      exception.addParameter("HostName", Spine::HostInfo::getHostName(theRequest.getClientIP()));

      const bool check_token = true;
      auto apikey = Spine::FmiApiKey::getFmiApiKey(theRequest, check_token);
      exception.addParameter("Apikey", (apikey ? *apikey : std::string("-")));

      exception.printError();

      // FIXME: implement correct processing phase support (parsing, processing)
      ErrorResponseGenerator error_response_generator(*this);
      const auto error_response = error_response_generator.create_error_response(
          ErrorResponseGenerator::REQ_PROCESSING, &theRequest);
      theResponse.setContent(error_response.response);
      theResponse.setStatus(error_response.status);
      theResponse.setHeader("Content-Type", "text/xml; charset=UTF8");
      theResponse.setHeader("Access-Control-Allow-Origin", "*");
      theResponse.setHeader("X-WFS-Error", error_response.wfs_err_code);
      maybe_validate_output(theRequest, theResponse);

      if (!exception.loggingDisabled())
        std::cerr << error_response.log_message;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::maybe_validate_output(const SmartMet::Spine::HTTP::Request& req,
                                       SmartMet::Spine::HTTP::Response& response) const
{
  try
  {
    if (get_config().getValidateXmlOutput())
    {
      try
      {
        const std::string content = response.getContent();
        std::size_t pos = content.find_first_not_of(" \t\r\n");
        if (pos != std::string::npos)
        {
          if (ba::iequals(content.substr(pos, 6), "<html>") or content.substr(pos, 1) != "<")
          {
            return;
          }
        }
        else
        {
          return;
        }

        xml_parser->get()->parse_string(content);
      }
      catch (const Xml::XmlError& err)
      {
        std::ostringstream msg;
        msg << SmartMet::Spine::log_time_str()
            << " [WFS] [ERROR] XML Response validation failed: " << err.what() << '\n';
        for (const std::string& err_msg : err.get_messages())
        {
          msg << "       XML: " << err_msg << std::endl;
        }
        const std::string req_str = req.toString();
        std::vector<std::string> lines;
        ba::split(lines, req_str, ba::is_any_of("\n"));
        msg << "   WFS request:\n";
        for (const auto& line : lines)
        {
          msg << "       " << ba::trim_right_copy_if(line, ba::is_any_of(" \t\r\n")) << '\n';
        }
        std::cout << msg.str() << std::flush;
      }
    }
#ifdef WFS_DEBUG
    else
    {
      try
      {
        const std::string content = response.getContent();
        if (content.substr(0, 5) == "<?xml")
        {
          Xml::str2xmldom(response.getContent(), "wfs_query_tmp");
        }
      }
      catch (const Xml::XmlError& err)
      {
        std::cout << "\nXML: non validating XML response read failed\n";
        std::cout << "XML: " << err.what() << std::endl;
        for (const std::string& msg : err.get_messages())
        {
          std::cout << "XML: " << msg << std::endl;
        }
      }
    }
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PluginImpl::dump_xml_schema_cache(std::ostream& os)
{
  xml_parser->dump_schema_cache(os);
}

void PluginImpl::dump_constructor_map(std::ostream& os, const std::optional<std::string>& handler)
{
    const std::string result =
        get_stored_query_map()
        .get_handler_factory_summary()
        ->as_json(handler)
        .toStyledString();
    os << result;
}

void PluginImpl::dump_constructor_map_html(std::ostream& os, const std::optional<std::string>& handler)
{
  try
  {
      const std::string url_base = get_config().defaultUrl();
      const auto& sqm = get_stored_query_map();
      auto summary_ptr = sqm.get_handler_factory_summary();
      summary_ptr->write_html(os, url_base, handler);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool PluginImpl::is_reload_required(bool reset)
{
  return stored_query_map->is_reload_required(reset);
}

// Convert TimedCache statistics to regular statistics
template <typename T>
Fmi::Cache::CacheStats convert_stats(const T& cache)
{
  auto stats = cache.getCacheStatistics();
  auto time = Fmi::date_time::from_time_t(std::chrono::duration_cast<std::chrono::seconds>(
                                                 stats.getConstructionTime().time_since_epoch())
                                                 .count());
  return {time,
          cache.maxSize(),
          cache.size(),
          stats.getHits(),
          stats.getMisses(),
          stats.getInsertSuccesses()};
}

Fmi::Cache::CacheStatistics PluginImpl::getCacheStats() const
{
  Fmi::Cache::CacheStatistics ret;
  ret["Wfs::query_cache"] = convert_stats(*query_cache);
  return ret;
}
