#pragma once

#ifndef WITHOUT_OBSERVATION

#include "ArrayParameterTemplate.h"
#include "ScalarParameterTemplate.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsBoundingBox.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsTimeZone.h"
#include "RequiresGeoEngine.h"
#include "RequiresObsEngine.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredFlashQueryHandler : public StoredQueryHandlerBase,
                                protected SupportsBoundingBox,
                                protected SupportsTimeZone,
                                protected virtual RequiresGeoEngine,
                                protected virtual RequiresObsEngine
{
 public:
  StoredFlashQueryHandler(SmartMet::Spine::Reactor *reactor,
                          const StoredQueryConfig::Ptr& config,
                          PluginImpl &plugin_impl,
                          const std::optional<std::string>& template_file_name);

  ~StoredFlashQueryHandler() override;

  std::string get_handler_description() const override;

  void query(const StoredQuery &query,
                     const std::string &language,
		     const std::optional<std::string> &hostname,
                     std::ostream &output) const override;

 private:
  struct FlashRowContext;
  struct FlashRows;

  static bool outside_bbox(const FlashRowContext& ctx, double lon, double lat);
  static std::string format_flash_value(const TS::TimeSeries& data_vect,
                                        std::size_t i,
                                        const FlashRowContext& ctx);
  void generate_rows(const FlashRowContext& ctx, FlashRows& rows) const;

  std::vector<SmartMet::Spine::Parameter> bs_param;
  int stroke_time_ind;
  int lon_ind;
  int lat_ind;
  std::string station_type;

  double max_hours;
  std::string missing_text;
  bool sq_restrictions;
  int time_block_size;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
