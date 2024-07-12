#include "stored_queries/StoredIsolineQueryHandler.h"
#include <gis/Box.h>
#include <newbase/NFmiEnumConverter.h>

#include <boost/algorithm/string/replace.hpp>
#include <iomanip>

namespace bw = SmartMet::Plugin::WFS;

bw::StoredIsolineQueryHandler::StoredIsolineQueryHandler(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    std::optional<std::string> template_file_name)

    : StoredQueryParamRegistry(config),
      SupportsExtraHandlerParams(config, false),
      RequiresGridEngine(reactor),
      RequiresContourEngine(reactor),
      RequiresQEngine(reactor),
      RequiresGeoEngine(reactor),
      StoredContourQueryHandler(reactor, config, plugin_data, template_file_name)
{
  try
  {
    itsName = config->get_mandatory_config_param<std::string>("contour_param.name");
    itsUnit = config->get_optional_config_param<std::string>("contour_param.unit", "");
    itsIsoValues = config->get_mandatory_config_array<double>("contour_param.isovalues");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string bw::StoredIsolineQueryHandler::get_handler_description() const
{
    return "Forecast data: areas as GML isolines";
}

void bw::StoredIsolineQueryHandler::clipGeometry(OGRGeometryPtr& pGeom, Fmi::Box& bbox) const
{
  try
  {
    if (pGeom && !pGeom->IsEmpty())
      pGeom.reset(Fmi::OGR::lineclip(*pGeom, bbox));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<bw::ContourQueryResultPtr> bw::StoredIsolineQueryHandler::processQuery(
    ContourQueryParameter& queryParameter) const
{
  try
  {
    std::vector<ContourQueryResultPtr> query_results;

    for (double itsIsoValue : itsIsoValues)
      (static_cast<IsolineQueryParameter&>(queryParameter)).isovalues.push_back(itsIsoValue);

    // contains result for all isolines
    ContourQueryResultSet result_set = getContours(queryParameter);

    // order result set primarily by isoline value and secondarily by timestep
    unsigned int max_geoms_per_timestep = 0;
    for (const auto& result_item : result_set)
      if (max_geoms_per_timestep < result_item->area_geoms.size())
        max_geoms_per_timestep = result_item->area_geoms.size();

    for (std::size_t i = 0; i < max_geoms_per_timestep; i++)
    {
      for (const auto& result_item : result_set)
        if (i < result_item->area_geoms.size())
        {
          IsolineQueryResultPtr result(new IsolineQueryResult);
          WeatherAreaGeometry wag = result_item->area_geoms[i];
          result->name = itsName;
          result->unit = itsUnit;
          result->isovalue = itsIsoValues[i];
          result->area_geoms.push_back(wag);
          query_results.emplace_back(result);
        }
    }

    return query_results;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

SmartMet::Engine::Contour::Options bw::StoredIsolineQueryHandler::getContourEngineOptions(
    const Fmi::DateTime& time, const ContourQueryParameter& queryParameter) const
{
  try
  {
    return SmartMet::Engine::Contour::Options(
        queryParameter.parameter,
        time,
        (reinterpret_cast<const IsolineQueryParameter&>(queryParameter)).isovalues);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<bw::ContourQueryParameter> bw::StoredIsolineQueryHandler::getQueryParameter(
    const SmartMet::Spine::Parameter& parameter,
    const SmartMet::Engine::Querydata::Q& q,
    OGRSpatialReference& sr) const
{
  try
  {
    std::shared_ptr<ContourQueryParameter> ret(new bw::IsolineQueryParameter(parameter, q, sr));

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredIsolineQueryHandler::setResultHashValue(CTPP::CDT& resultHash,
                                                       const ContourQueryResult& resultItem) const
{
  try
  {
    const auto& isolineResultItem =
        reinterpret_cast<const IsolineQueryResult&>(resultItem);

    resultHash["name"] = isolineResultItem.name;
    if (!isolineResultItem.unit.empty())
      resultHash["unit"] = isolineResultItem.unit;
    resultHash["isovalue"] = isolineResultItem.isovalue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> wfs_isoline_query_handler_create(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    std::optional<std::string> template_file_name)
{
  try
  {
    auto* qh =
        new StoredIsolineQueryHandler(reactor, config, plugin_data, template_file_name);
    std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> result(qh);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_isoline_query_handler_factory(
    &wfs_isoline_query_handler_create);
