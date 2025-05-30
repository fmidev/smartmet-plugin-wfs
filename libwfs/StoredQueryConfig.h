#pragma once

#include "Config.h"
#include "StoredQueryParamDef.h"
#include "StoredQueryConfigWrapper.h"
#include <optional>
#include <ctpp2/CDT.hpp>
#include <spine/ConfigBase.h>
#include <spine/MultiLanguageString.h>
#include <libconfig.h++>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredQueryHandlerBase;

/**
 *   @brief The stored query configuration
 *
 *   An example of configuration is given below.
 *
 *   Note that part of data are for DescribeStoredQuery request only
 *   and are not used in any other way (parameters descriptions)
 *
 *   @verbatim
 *
 *   // disable = true;
 *
 *   id = "fmi::wfs::test";
 *
 *   constructor_name = "fmi_wfs_handler_create";
 *
 *   // Configuration is searched from template directory provided in
 *   // WFS plugin configuration
 *   template = "wfs_test.t2c";
 *
 *   title = { en = "WFS test request"; fi = "WFS testikysely"; lv = "WFS testa pieprasījums"; }
 *
 *   abstract = { en = "More detailed description of WFS test request";
 *                fi = "Yksinkohtaisempi selitys WFS testkyselystä";
 *                lv = "Pilnīgāks WFS testa pieprasījuma apraksts"; }
 *
 *   parametrs = (
 *      {
 *         name = "foo";
 *         title = { en = "Parameter foo"; }
 *         abstract = { en = "More detailed description of parameter foo"; }
 *         xmlType = "anyURI";
 *         type = "string";
 *      },
 *
 *      {
 *         name = "bar";
 *         title = { en = "Parameter bar"; }
 *         abstract = { en = "More detailed description of parameter  bar"; }
 *         xmlType = "string";
 *         type = "double[4..12]"
 *      }
 *   )
 *
 *   returnTypeNames = [ "fooResult" ];
 *   returnLanguge = "urn:ogc:def:queryLanguage:OGC-WFS::WFS_QueryExpression";
 *   isPrivate = true;
 *   defaultLanguage = "en";
 *
 *   @endverbatim
 */
class StoredQueryConfig : public SmartMet::Spine::ConfigBase
{
 public:
  using Ptr = StoredQueryConfigPtr;
  using Wrapper = StoredQueryConfigWrapper;

  struct ParamDesc
  {
    std::string name;
    SmartMet::Spine::MultiLanguageStringP title;
    SmartMet::Spine::MultiLanguageStringP abstract;
    int min_occurs;
    int max_occurs;
    std::string xml_type;
    StoredQueryParamDef param_def;
    std::optional<SmartMet::Spine::Value> lower_limit;
    std::optional<SmartMet::Spine::Value> upper_limit;
    std::set<std::string> conflicts_with;

    inline ParamDesc()  = default;
    inline bool isArray() const { return max_occurs > 1 or param_def.isArray(); }
    inline unsigned getMinSize() const { return min_occurs * param_def.getMinSize(); }
    inline unsigned getMaxSize() const { return max_occurs * param_def.getMaxSize(); }
    void dump(std::ostream& stream) const;

    inline void set_used() const { used = true; }
    inline bool get_used() const { return used; }

   private:
    mutable bool used{false};
  };

 public:
  StoredQueryConfig(const std::string& fn, const Config* plugin_config);
  StoredQueryConfig(std::shared_ptr<libconfig::Config> config, const Config* plugin_config);
  ~StoredQueryConfig() override;

  inline bool is_disabled() const { return disabled; }
  inline bool is_demo() const { return demo; }
  inline bool is_test() const { return test; }
  inline const std::string& get_query_id() const { return query_id; }
  inline const std::string& get_constructor_name() const { return constructor_name; }
  /**
   *   @brief Query whether CTPP2 template file name is specified
   */
  inline bool have_template_fn() const { return !!template_fn; }
  /**
   *   @brief Query CTPP2 template file name
   *
   *   Throws std::runtime_error if the template file name is not specified.
   */
  std::string get_template_fn() const;

  inline const std::string& get_default_language() const { return default_language; }
  std::string get_title(const std::string& language) const;

  std::string get_abstract(const std::string& language) const;

  const ParamDesc* get_param_desc(const std::string& name) const;

  inline const std::vector<std::string>& get_return_type_names() const
  {
    return return_type_names;
  }

  /**
   *  @brief Get reference to internal map with parameter descriptions
   *
   *  NOTE: MUST NOT be used for looking up parameter descriptions.
   *        Use method get_param_desc(const std::string&) instead
   */
  inline const std::map<std::string, ParamDesc>& get_param_descriptions() const
  {
    return param_map;
  }

  inline std::size_t get_num_param_desc() const { return param_names.size(); }
  inline const ParamDesc& get_param_desc(int ind) const
  {
    return param_map.at(param_names.at(ind));
  }

  inline const std::string& get_param_name(int ind) const
  {
    return param_map.at(param_names.at(ind)).name;
  }

  std::string get_param_title(int ind, const std::string& language) const;

  std::string get_param_abstract(int ind, const std::string& language) const;

  inline const std::string& get_param_xml_type(int ind) const
  {
    return param_map.at(param_names.at(ind)).xml_type;
  }

  inline int get_debug_level() const { return debug_level; }
  inline int get_expires_seconds() const { return expires_seconds; }
  void dump_params(std::ostream& stream) const;

  void warn_about_unused_params(const StoredQueryHandlerBase* handler = nullptr);

  /**
   *  @brief Get last write time of the stored query configuration file.
   *  If the file is removed the time stored to an object is returned.
   *  @return The last write time if file.
   */
  std::time_t config_write_time() const;

  /**
   *  @brief Test if the stored query configuration file is changed.
   *  @retval true Last write time has changed.
   *  @retval false Last write time has not changed.
   */
  bool last_write_time_changed() const;

  const std::string& get_locale_name() const { return locale_name; }

  /**
   *  Returns locale to be used for stored query
   */
  std::shared_ptr<const std::locale> get_locale() const;

  /**
   *  Set locale with specified name for stored query handler
   */
  void set_locale(const std::string& name);

  const Hosts& get_hosts() const { return hosts; }

  std::string guess_fallback_encoding(const std::string& language) const;

  bool use_case_sensitive_params() const;

 private:
  void parse_config();

 private:
  std::string locale_name;
  std::shared_ptr<std::locale> locale;
  std::string query_id;
  std::time_t config_last_write_time;

  int expires_seconds;  ///< For the expires entity-header field. After that the response is
                        /// considered stale.
                        /**
                         *  @brief The name of factory method procedure for creating request handler object
                         */
  std::string constructor_name;

  std::optional<std::string> template_fn;
  std::string default_language;
  SmartMet::Spine::MultiLanguageStringP title;
  SmartMet::Spine::MultiLanguageStringP abstract;
  std::vector<std::string> return_type_names;
  std::map<std::string, ParamDesc> param_map;

  /**
   *  @brief Map case provided parameter name to actual query parameter name
   *
   *  Use in case when plugin configuration parameters case_sensitive_params value
   *  is false (defauls). Otherwise empty.
   */
  std::map<std::string, std::string> case_map;

  std::vector<std::string> param_names;
  bool disabled;
  bool demo;
  bool test;
  int debug_level;
  const Hosts& hosts;
  const Config* plugin_config;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
