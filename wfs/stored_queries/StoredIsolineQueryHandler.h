#pragma once

#include "StoredContourHandlerBase.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *   @brief Handler for StoredIsolineQuery stored query
 */
class StoredIsolineQueryHandler : public StoredContourQueryHandler
{
 public:
  StoredIsolineQueryHandler(SmartMet::Spine::Reactor* reactor,
                            StoredQueryConfig::Ptr config,
                            PluginImpl& plugin_impl,
                            boost::optional<std::string> template_file_name);

  std::string get_handler_description() const override;

 protected:
  void clipGeometry(OGRGeometryPtr& pGeom, Fmi::Box& bbox) const override;
  std::vector<ContourQueryResultPtr> processQuery(ContourQueryParameter& queryParameter) const override;
  SmartMet::Engine::Contour::Options getContourEngineOptions(
      const boost::posix_time::ptime& time, const ContourQueryParameter& queryParameter) const override;
  boost::shared_ptr<ContourQueryParameter> getQueryParameter(
      const SmartMet::Spine::Parameter& parameter,
      const SmartMet::Engine::Querydata::Q& q,
      OGRSpatialReference& sr) const override;
  void setResultHashValue(CTPP::CDT& resultHash, const ContourQueryResult& resultItem) const override;

 private:
  std::string itsName;
  std::string itsUnit;
  std::vector<double> itsIsoValues;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
