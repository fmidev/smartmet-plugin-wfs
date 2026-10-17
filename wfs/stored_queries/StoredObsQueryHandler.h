#pragma once

#ifndef WITHOUT_OBSERVATION

#include "ArrayParameterTemplate.h"
#include "ScalarParameterTemplate.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "SupportsMeteoParameterOptions.h"
#include "SupportsQualityParameters.h"
#include "SupportsTimeZone.h"
#include "RequiresGeoEngine.h"
#include "SupportsTimeParameters.h"
#include "RequiresObsEngine.h"
namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

//  FIXME: why SupportsLocationparameters and SupportsTimeParameters are not used

class StoredObsQueryHandler : public StoredQueryHandlerBase,
                              protected SupportsLocationParameters,
                              protected SupportsBoundingBox,
                              protected SupportsTimeZone,
                              protected SupportsQualityParameters,
                              protected SupportsMeteoParameterOptions,
                              protected SupportsTimeParameters,
                              protected virtual RequiresGeoEngine,
                              protected virtual RequiresObsEngine
{
 public:
  StoredObsQueryHandler(SmartMet::Spine::Reactor* reactor,
                        const StoredQueryConfig::Ptr& config,
                        PluginImpl& plugin_impl,
                        const std::optional<std::string>& template_file_name);

  ~StoredObsQueryHandler() override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

  std::string get_handler_description() const override;

 private:

  struct ParamIndexEntry
  {
	/** Parameter index in obsengine response or -1 for special parameters generated separately */
	int ind = 0;
	
	/** Parameter name */
	std::string name;
	
	/** Sensor number */
	std::optional<std::string> sensor_name;	
  };

  struct ExtParamIndexEntry
  {
      ParamIndexEntry p;
      std::optional<ParamIndexEntry> qc;
  };

  /**
   *   @brief Verifies requested parameter names
   *
   *   Throws an exception if on of the following is detected
   *   - duplicate parameter name (case insensitive)
   *   - qc_parameter requested explictitly but QC parameters support is not enabled
   */
  void check_parameter_names(const RequestParameterMap& params,
                             const std::vector<std::string>& param_names) const;

  int lookup_initial_parameter(const std::string& name) const;

  bool add_parameters(const RequestParameterMap& params,
                      const std::vector<std::string>& names,
                      SmartMet::Engine::Observation::Settings& query_params,
                      std::vector<ExtParamIndexEntry>& parameter_index) const;

  static void add_qc_parameter(const std::string& name,
                               SmartMet::Engine::Observation::Settings& query_params,
                               ExtParamIndexEntry& entry);

  bool add_meteo_parameter(const std::string& name,
                           bool add_qc,
                           SmartMet::Engine::Observation::Settings& query_params,
                           std::vector<ExtParamIndexEntry>& parameter_index) const;

  void add_special_parameter(const std::string& name,
                             const std::string& l_name,
                             SmartMet::Engine::Observation::Settings& query_params,
                             std::vector<ExtParamIndexEntry>& parameter_index) const;

  struct GroupRec
  {
    int group_id = 0;
    std::string feature_id;
    std::map<std::string, std::string> param_ids;
  };

  /**
   *   Contains index of result lines for each station
   */
  struct SiteRec
  {
    int group_id = 0;
    int ind_in_group = 0;
    std::vector<int> row_index_vect;
  };

  /**
   *   @brief The vector of initial parameters which are queried always
   */
  std::vector<SmartMet::Spine::Parameter> initial_bs_param;

  /**
   *    @brief The indexes if always queried parameters
   *    @{
   */
  int fmisid_ind;
  int geoid_ind;
  int lon_ind;
  int lat_ind;
  int height_ind;
  int name_ind;
  int dist_ind;
  int direction_ind;
  int wmo_ind;

  /**
   *  @brief Largest allowed time interval in hours
   */
  double max_hours;

  /**
   *  @brief Max. allowed stations cound per request
   */
  std::size_t max_station_count;

  bool separate_groups;

  /**
   * @brief Allow to turn restrictions off.
   */
  bool sq_restrictions;

  /**
   * @brief Support parameters with "qc_" prefix
   */
  bool m_support_qc_parameters;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
