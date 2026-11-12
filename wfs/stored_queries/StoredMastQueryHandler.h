#pragma once

#ifndef WITHOUT_OBSERVATION

#include "RequestParameterMap.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsLocationParameters.h"
#include "SupportsQualityParameters.h"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <engines/observation/DBRegistryConfig.h>
#include <spine/Location.h>
#include <spine/Station.h>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace CTPP
{
class CDT;
}  // namespace CTPP
namespace SmartMet::Engine::Observation
{
class QueryResult;
}  // namespace SmartMet::Engine::Observation
namespace SmartMet::Plugin::WFS
{
class PluginImpl;
}  // namespace SmartMet::Plugin::WFS
namespace SmartMet::Spine
{
class Reactor;
}  // namespace SmartMet::Spine

namespace ba = boost::algorithm;
namespace bl = boost::lambda;

// Stored query configuration parameter names.
const std::string P_BEGIN_TIME = "beginTime";
const std::string P_END_TIME = "endTime";
const std::string P_METEO_PARAMETERS = "meteoParameters";
const std::string P_STATION_TYPE = "stationType";
const std::string P_TIME_STEP = "timeStep";
// const std::string P_WMOS                ="wmos";
// const std::string P_LPNNS               ="lpnns";
// const std::string P_FMISIDS             ="fmisids";
const std::string P_NUM_OF_STATIONS = "numOfStations";
// const std::string P_HOURS               ="hours";
// const std::string P_WEEK_DAYS           ="weekDays";
// const std::string P_LOCALE              ="locale";
const std::string P_MISSING_TEXT = "missingText";
const std::string P_MAX_EPOCHS = "maxEpochs";
const std::string P_CRS = "crs";
const std::string P_PRODUCER_ID = "producerId";

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class FeatureID;

//  FIXME: why SupportsTimeParameters is not used
class StoredMastQueryHandler : public StoredQueryHandlerBase,
                               protected virtual RequiresGeoEngine,
                               protected virtual RequiresObsEngine,
                               protected SupportsLocationParameters,
                               protected SupportsBoundingBox,
                               protected SupportsQualityParameters
{
 public:
  StoredMastQueryHandler(SmartMet::Spine::Reactor* reactor,
                         const StoredQueryConfig::Ptr& config,
                         PluginImpl& plugin_impl,
                         const std::optional<std::string>& template_file_name);

  ~StoredMastQueryHandler() override;

  std::string get_handler_description() const override;

  void query(const StoredQuery& query,
                     const std::string& language,
		     const std::optional<std::string> &hostname,
                     std::ostream& output) const override;

 private:
  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<std::shared_ptr<RequestParameterMap> >& result) const;

  std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  // map: id, (id, paramName, paramQCName, showValue, showQualityCode)
  using MastParameterMap =
      std::map<std::string, std::tuple<uint64_t, std::string, std::string, bool, bool>>;
  struct MastContext;

  SmartMet::Spine::Stations find_stations(
      const RequestParameterMap& params,
      const std::string& crs,
      const std::string& stationType,
      double maxDistance,
      uint64_t numberOfStations,
      const std::list<std::pair<std::string, SmartMet::Spine::LocationPtr> >& locations_list,
      const std::string& langCode) const;

  MastParameterMap get_meteo_parameters(const RequestParameterMap& params,
                                        const std::string& stationType) const;

  void set_station_info(CTPP::CDT& station_cdt,
                        const MastContext& ctx,
                        const std::string& fmisidStr,
                        double& bottomHeight) const;

  static void set_group_parameters(CTPP::CDT& group,
                                   FeatureID& feature_id,
                                   const MastContext& ctx,
                                   const std::string& measurandIdStr,
                                   bool& showValue,
                                   bool& showDataQuality);

  int fill_mast_data(CTPP::CDT& hash,
                     SmartMet::Engine::Observation::QueryResult& dataContainer,
                     const MastContext& ctx,
                     FeatureID& feature_id) const;

  double m_maxHours;
  bool m_sqRestrictions;
  bool m_supportQCParameters;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
