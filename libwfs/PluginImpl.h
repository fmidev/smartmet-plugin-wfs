
#pragma once

#include "Config.h"
#include "GeoServerDB.h"
#include "RequestBase.h"
#include "RequestFactory.h"
#include "StoredQueryMap.h"
#include "TypeNameStoredQueryMap.h"
#include "WfsCapabilities.h"
#include "XmlEnvInit.h"
#include "XmlParser.h"
#include <macgyver/DateTime.h>
#include <memory>
#include <json/json.h>
#include <engines/geonames/Engine.h>
#include <engines/gis/Engine.h>
#include <engines/querydata/Engine.h>
#include <spine/CRSRegistry.h>
#include <macgyver/DirectoryMonitor.h>
#include <macgyver/TemplateFactory.h>
#include <macgyver/TimedCache.h>
#include <macgyver/CacheStats.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredQueryMap;

using QueryResponseCache = Fmi::TimedCache::Cache<std::string,
        std::string>;

class PluginImpl
{
  struct RequestResult;

 public:
  PluginImpl(
      SmartMet::Spine::Reactor* theReactor,
      const char* theConfig,
      Spine::CRSRegistry& crs_registry);

  PluginImpl(const PluginImpl&) = delete;
  PluginImpl& operator = (const PluginImpl&) = delete;

  virtual ~PluginImpl();

  void shutdown();

  virtual void realRequestHandler(SmartMet::Spine::Reactor& theReactor,
                                  const std::string& language,
                                  const SmartMet::Spine::HTTP::Request& theRequest,
                                  SmartMet::Spine::HTTP::Response& theResponse);

  inline SmartMet::Spine::CRSRegistry& get_crs_registry() const
  {
    return itsCRSRegistry;
  }

  void set_data_source(const std::string& ds) { data_source = ds; }
  inline const std::string& get_data_source() const { return data_source; }
  inline const std::string& get_primary_data_source() const { return primary_data_source; }
  inline bool is_gridengine_disabled() const { return gridengine_disabled; }

  inline Config& get_config() { return itsConfig; }

  inline const Config& get_config() const { return itsConfig; }

  inline int get_debug_level() const { return debug_level; }

  inline WfsCapabilities& get_capabilities() { return *wfs_capabilities; }

  inline const WfsCapabilities& get_capabilities() const { return *wfs_capabilities; }

  inline const std::vector<std::string>& get_languages() const { return itsConfig.get_languages(); }

  Fmi::DateTime get_time_stamp() const;

  Fmi::DateTime get_local_time_stamp() const;

  inline const StoredQueryMap& get_stored_query_map() const { return *stored_query_map; }

  inline const TypeNameStoredQueryMap& get_typename_stored_query_map() const
  {
    return *type_name_stored_query_map;
  }

  std::shared_ptr<GeoServerDB> get_geo_server_database() const;

  inline std::string get_fallback_hostname() const { return fallback_hostname; }

  inline std::string get_fallback_protocol() const { return fallback_protocol; }

  inline QueryResponseCache& get_query_cache() { return *query_cache; }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_get_capabilities_formater() const
  {
    return itsTemplateFactory.get(getCapabilitiesFormatterPath);
  }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_list_stored_queries_formatter() const
  {
    return itsTemplateFactory.get(listStoredQueriesFormatterPath);
  }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_describe_stored_queries_formatter() const
  {
    return itsTemplateFactory.get(describeStoredQueriesFormatterPath);
  }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_feature_type_formatter() const
  {
    return itsTemplateFactory.get(featureTypeFormatterPath);
  }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_exception_formatter() const
  {
    return itsTemplateFactory.get(exceptionFormatterPath);
  }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_ctpp_dump_formatter() const
  {
    return itsTemplateFactory.get(ctppDumpFormatterPath);
  }

  inline std::shared_ptr<Fmi::TemplateFormatter> get_stored_query_formatter(
      const std::filesystem::path& filename) const
  {
    return itsTemplateFactory.get(filename);
  }

  void dump_xml_schema_cache(std::ostream& os);

  /**
   *  @brief Dump stored query handler map in JSON format
   *
   *  @param os output stream to which to write JSON document
   *  @param handler stored query handler for which to output information or
   *         all handlers when empty string is specified
   */
  void dump_constructor_map(std::ostream& os, const std::optional<std::string>& handler = {});

  void dump_constructor_map_html(std::ostream& os,
          const std::string& uri,
          const std::optional<std::string>& handler = {});

  bool is_reload_required(bool reset = false);

  Fmi::Cache::CacheStatistics getCacheStats() const;

 private:
  void query(const std::string& language,
             const SmartMet::Spine::HTTP::Request& req,
             RequestResult& result);

  RequestBaseP parse_kvp_get_capabilities_request(const std::string& language,
                                                  const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parse_xml_get_capabilities_request(const std::string& language,
                                                  const xercesc::DOMDocument& document,
                                                  const xercesc::DOMElement& root);

  RequestBaseP parse_kvp_describe_feature_type_request(
      const std::string& language, const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parse_xml_describe_feature_type_request(const std::string& language,
                                                       const xercesc::DOMDocument& document,
                                                       const xercesc::DOMElement& root);

  RequestBaseP parse_kvp_get_feature_request(const std::string& language,
                                             const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parse_xml_get_feature_request(const std::string& language,
                                             const xercesc::DOMDocument& document,
                                             const xercesc::DOMElement& root);

  RequestBaseP parse_kvp_get_property_value_request(const std::string& language,
                                                    const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parse_xml_get_property_value_request(const std::string& language,
                                                    const xercesc::DOMDocument& document,
                                                    const xercesc::DOMElement& root);

  RequestBaseP parse_kvp_list_stored_queries_request(const std::string& language,
                                                     const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parse_xml_list_stored_queries_request(const std::string& language,
                                                     const xercesc::DOMDocument& document,
                                                     const xercesc::DOMElement& root);

  RequestBaseP parse_kvp_describe_stored_queries_request(
      const std::string& language, const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parse_xml_describe_stored_queries_request(const std::string& language,
                                                         const xercesc::DOMDocument& document,
                                                         const xercesc::DOMElement& root);

  std::optional<std::string> get_fmi_apikey(
      const SmartMet::Spine::HTTP::Request& theRequest) const;

  void maybe_validate_output(const SmartMet::Spine::HTTP::Request& req,
                             SmartMet::Spine::HTTP::Response& response) const;

  // inline std::shared_ptr<Xml::ParserMT> get_xml_parser() const { return xml_parser; }

 private:
  void create_template_formatters();
  void create_xml_parser();
  void init_geo_server_access();
  void create_stored_query_map(SmartMet::Spine::Reactor* theReactor);
  void create_typename_stored_query_map();

 private:
  Config itsConfig;

  std::unique_ptr<QueryResponseCache> query_cache;

  /**
   *   @brief An object that reads actual requests and creates request objects
   */
  std::unique_ptr<RequestFactory> request_factory;

  SmartMet::Spine::CRSRegistry& itsCRSRegistry;

  Fmi::TemplateFactory itsTemplateFactory;

  std::filesystem::path getCapabilitiesFormatterPath;
  std::filesystem::path listStoredQueriesFormatterPath;
  std::filesystem::path describeStoredQueriesFormatterPath;
  std::filesystem::path featureTypeFormatterPath;
  std::filesystem::path exceptionFormatterPath;
  std::filesystem::path ctppDumpFormatterPath;

  std::shared_ptr<Xml::ParserMT> xml_parser;
  std::unique_ptr<WfsCapabilities> wfs_capabilities;
  std::shared_ptr<GeoServerDB> geo_server_db;
  std::unique_ptr<StoredQueryMap> stored_query_map;
  std::unique_ptr<TypeNameStoredQueryMap> type_name_stored_query_map;
  int debug_level;
  std::string fallback_hostname;
  std::string fallback_protocol;
  std::string data_source;
  std::string primary_data_source;
  bool gridengine_disabled{false};

  /**
   *   @brief Locked timestamp for testing only
   */
  std::optional<Fmi::DateTime> locked_time_stamp;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
