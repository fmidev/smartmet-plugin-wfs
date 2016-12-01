#ifndef SMARTMET_WFS_STORED_OBS_QUERY_HANLDER_H_
#define SMARTMET_WFS_STORED_OBS_QUERY_HANLDER_H_

#include <engines/observation/Interface.h>
#include <engines/geonames/Engine.h>
#include "StoredQueryHandlerBase.h"
#include "ArrayParameterTemplate.h"
#include "ScalarParameterTemplate.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "SupportsBoundingBox.h"
#include "SupportsTimeZone.h"
#include "SupportsQualityParameters.h"
#include "SupportsMeteoParameterOptions.h"
namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredObsQueryHandler : public StoredQueryHandlerBase,
                              protected SupportsLocationParameters,
                              protected SupportsBoundingBox,
                              protected SupportsTimeZone,
                              protected SupportsQualityParameters,
                              protected SupportsMeteoParameterOptions,
                              protected virtual SupportsExtraHandlerParams
{
 public:
  StoredObsQueryHandler(SmartMet::Spine::Reactor* reactor,
                        boost::shared_ptr<StoredQueryConfig> config,
                        PluginData& plugin_data,
                        boost::optional<std::string> template_file_name);

  virtual ~StoredObsQueryHandler();

  virtual void init_handler();

  virtual void query(const StoredQuery& query,
                     const std::string& language,
                     std::ostream& output) const;

 private:
  struct GroupRec
  {
    int group_id;
    std::string feature_id;
    std::map<std::string, std::string> param_ids;
  };

  /**
   *   Contains index of result lines for each station
   */
  struct SiteRec
  {
    int group_id;
    int ind_in_group;
    std::vector<int> row_index_vect;
  };

 private:
  SmartMet::Engine::Geonames::Engine* geo_engine;

  SmartMet::Engine::Observation::Interface* obs_engine;

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
  int time_ind;
  int name_ind;
  int dist_ind;
  int region_ind;
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

#endif /* SMARTMET_WFS_STORED_OBS_QUERY_HANLDER_H_ */
