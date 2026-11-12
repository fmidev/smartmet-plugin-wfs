#pragma once

#include "StoredContourHandlerBase.h"
#include "StoredContourQueryStructs.h"
#include "StoredQueryConfig.h"
#include <engines/querydata/Q.h>
#include <gis/Types.h>
#include <macgyver/DateTime.h>
#include <optional>
#include <string>
#include <vector>

namespace SmartMet::Plugin::WFS
{
class PluginImpl;
}  // namespace SmartMet::Plugin::WFS
namespace SmartMet::Spine
{
class Reactor;
}  // namespace SmartMet::Spine

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *   @brief Handler for StoredCoverageQuery stored query
 */
class StoredCoverageQueryHandler : public StoredContourQueryHandler
{
 public:
  StoredCoverageQueryHandler(SmartMet::Spine::Reactor* reactor,
                             const StoredQueryConfig::Ptr& config,
                             PluginImpl& plugin_impl,
                             const std::optional<std::string>& template_file_name);

  std::string get_handler_description() const override;

 protected:
  void clipGeometry(OGRGeometryPtr& pGeom, Fmi::Box& bbox) const override;
  std::vector<ContourQueryResultPtr> processQuery(ContourQueryParameter& queryParameter) const override;
  SmartMet::Engine::Contour::Options getContourEngineOptions(
      const Fmi::DateTime& time, const ContourQueryParameter& queryParameter) const override;
  std::shared_ptr<ContourQueryParameter> getQueryParameter(
      const SmartMet::Spine::Parameter& parameter,
      const SmartMet::Engine::Querydata::Q& q,
      OGRSpatialReference& sr) const override;
  void setResultHashValue(CTPP::CDT& resultHash, const ContourQueryResult& resultItem) const override;

  std::vector<double> itsLimits;

 private:
  std::string itsUnit;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
