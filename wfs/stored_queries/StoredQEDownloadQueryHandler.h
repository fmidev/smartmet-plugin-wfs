#pragma once

#include "SupportsBoundingBox.h"
#include "stored_queries/StoredAtomQueryHandlerBase.h"
#include "RequiresGeoEngine.h"
#include "RequiresQEngine.h"
#include <boost/geometry/geometry.hpp>
#include <memory>
#include <ogr_geometry.h>

namespace SmartMet
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Engine
{
namespace Geonames
{
class Engine;
}
namespace Querydata
{
class Engine;
struct MetaData;
}  // namespace Querydata
}  // namespace Engine
#endif

namespace Plugin
{
namespace WFS
{
class StoredQEDownloadQueryHandler : public StoredAtomQueryHandlerBase,
                                     protected SupportsBoundingBox,
                                     protected virtual RequiresGeoEngine,
                                     protected virtual RequiresQEngine

{
  using point_t = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>;
  using box_t = boost::geometry::model::box<point_t>;
  using ring_t = boost::geometry::model::ring<point_t>;

 public:
  StoredQEDownloadQueryHandler(SmartMet::Spine::Reactor* reactor,
                               StoredQueryConfig::Ptr config,
                               PluginImpl& plugin_impl,
                               std::optional<std::string> template_file_name);

  ~StoredQEDownloadQueryHandler() override;

  std::string get_handler_description() const override;

protected:
  void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<std::shared_ptr<RequestParameterMap> >& result) const override;

 private:
  std::shared_ptr<OGRPolygon> get_model_boundary(
      const SmartMet::Engine::Querydata::MetaData& meta_info,
      const std::string& crs_name,
      int num_side_points = 10) const;

  std::shared_ptr<OGRGeometry> bbox_intersection(
      const SmartMet::Spine::BoundingBox& bbox,
      const SmartMet::Engine::Querydata::MetaData& meta_info) const;

  void add_bbox_info(RequestParameterMap* param_map,
                     const std::string& name,
                     OGRGeometry& geom) const;

  void add_boundary(RequestParameterMap* param_map,
                    const std::string& name,
                    OGRGeometry& polygon) const;

 private:
  std::set<std::string> producers;
  std::set<std::string> formats;
  std::string default_format;
  const int debug_level;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
