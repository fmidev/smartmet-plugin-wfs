#include "stored_queries/StoredFlashQueryHandler.h"
#include "FeatureID.h"
#include "StoredQueryHandlerFactoryDef.h"
#include "WfsConst.h"
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <spine/Convenience.h>
#include <spine/Value.h>
#include <timeseries/ParameterTools.h>

namespace bw = SmartMet::Plugin::WFS;
namespace pt = boost::posix_time;
namespace bg = boost::gregorian;

namespace
{
const char* P_BEGIN_TIME = "beginTime";
const char* P_END_TIME = "endTime";
const char* P_PARAM = "meteoParameters";
const char* P_CRS = "crs";

// Known stored queries are detected directly because formatting 100,000 rows
// of lightning data is very slow using a CTPP2 TMPL-loop.

const char* P_SIMPLE_QUERY = "lightning_simple.c2t";
const char* P_MULTIPOINTCOVERAGE_QUERY = "lightning_multipointcoverage.c2t";

}  // namespace

bw::StoredFlashQueryHandler::StoredFlashQueryHandler(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    boost::optional<std::string> template_file_name)

    : StoredQueryParamRegistry(config),
      SupportsExtraHandlerParams(config, false),
      RequiresGeoEngine(reactor),
      RequiresObsEngine(reactor),
      StoredQueryHandlerBase(reactor, config, plugin_data, template_file_name),
      SupportsBoundingBox(config, plugin_data.get_crs_registry()),
      SupportsTimeZone(reactor, config),
      bs_param(),
      stroke_time_ind(
          add_param(bs_param, "utctime", SmartMet::Spine::Parameter::Type::DataIndependent)),
      lon_ind(add_param(bs_param, "longitude", SmartMet::Spine::Parameter::Type::DataDerived)),
      lat_ind(add_param(bs_param, "latitude", SmartMet::Spine::Parameter::Type::DataDerived))
{
  try
  {
    register_scalar_param<Fmi::DateTime>(P_BEGIN_TIME, "The start time of the requested time period.");

    register_scalar_param<Fmi::DateTime>(P_END_TIME, "The end time of the requested time period.");

    register_array_param<std::string>(
        P_PARAM, "An array of fields whose values should be returned in the response.", 1, 999);

    register_scalar_param<std::string>(
        P_CRS,
        "Specifies the coordinate projection. For example crs = \"${crs:EPSG::4326}\" maps"
        " to request parameter crs with the default value EPSG::4326.");

    station_type = config->get_optional_config_param<std::string>("stationType", "flash");
    max_hours = config->get_optional_config_param<double>("maxHours", 7.0 * 24.0);
    missing_text = config->get_optional_config_param<std::string>("missingText", "NaN");

    sq_restrictions = plugin_data.get_config().getSQRestrictions();
    time_block_size = 1;

    if (sq_restrictions)
      time_block_size = config->get_optional_config_param<unsigned>("timeBlockSize", 300);

    if ((time_block_size < 1) or ((86400 % time_block_size) != 0))
    {
      Fmi::Exception exception(
          BCP, "Invalid time block size '" + std::to_string(time_block_size) + " seconds'!");
      exception.addDetail("Value must be a divisor of 24*60*60 (86400).");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::StoredFlashQueryHandler::~StoredFlashQueryHandler() = default;

std::string bw::StoredFlashQueryHandler::get_handler_description() const
{
  return "Observation data: Lightning";
}

namespace
{
Fmi::DateTime round_time(const Fmi::DateTime& t0, unsigned step, int offset = 0)
{
  try
  {
    Fmi::DateTime t = t0 + Fmi::Seconds(offset);
    long sec = t.time_of_day().total_seconds();
    Fmi::DateTime result(t.date(), Fmi::Seconds(step * (sec / step)));
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

void bw::StoredFlashQueryHandler::query(const StoredQuery& query,
                                        const std::string& language,
                                        const boost::optional<std::string>& /*hostname*/,
                                        std::ostream& output) const
{
  try
  {
    using namespace SmartMet;
    using namespace SmartMet::Plugin::WFS;
    using SmartMet::Spine::BoundingBox;

    const int debug_level = get_config()->get_debug_level();
    if (debug_level > 0)
      query.dump_query_info(std::cout);

    // Determine how to generated the content rows from the template name, a C++ loop
    // is much faster than CTPP2

    bool is_simple_query = false;

    if (!template_file)
      throw Fmi::Exception(BCP, "No template name given to stored flash query");
    if (boost::algorithm::ends_with(*template_file, P_SIMPLE_QUERY))
      is_simple_query = true;
    else if (boost::algorithm::ends_with(*template_file, P_MULTIPOINTCOVERAGE_QUERY))
      is_simple_query = false;
    else
      throw Fmi::Exception(BCP, "Unknown template for stored flash query")
          .addParameter("template", *template_file);

    const auto& params = query.get_param_map();

    try
    {
      SmartMet::Engine::Observation::Settings query_params;
      query_params.localTimePool = boost::make_shared<TS::LocalTimePool>();

      const char* DATA_CRS_NAME = "urn:ogc:def:crs:EPSG::4326";
      const auto crs = params.get_single<std::string>(P_CRS);
      auto transformation = crs_registry.create_transformation(DATA_CRS_NAME, crs);
      // FIXME: shouldn't we transform at first from FMI sphere to WGS84?

      bool show_height = false;
      std::string proj_uri = "UNKNOWN";
      std::string proj_epoch_uri = "UNKNOWN";
      bool swap_coord = false;
      crs_registry.get_attribute(crs, "showHeight", &show_height);
      crs_registry.get_attribute(crs, "projUri", &proj_uri);
      crs_registry.get_attribute(crs, "projEpochUri", &proj_epoch_uri);
      crs_registry.get_attribute(crs, "swapCoord", &swap_coord);
      if (show_height)
      {
        Fmi::Exception exception(BCP, "Projection '" + crs + "' not supported for lightning data!");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
        throw exception.disableStackTrace();
      }

      auto begin = params.get_single<Fmi::DateTime>(P_BEGIN_TIME);
      auto end = params.get_single<Fmi::DateTime>(P_END_TIME);
      Fmi::DateTime now = Fmi::SecondClock::universal_time();
      Fmi::DateTime last = now - Fmi::Seconds(time_block_size);

      query_params.starttime = round_time(begin, time_block_size, 0);
      query_params.endtime = round_time(std::min(end, last), time_block_size, time_block_size - 1);

      // This "fixes" queries into the future so that they don't crowd error logs
      query_params.endtime = std::max(query_params.endtime, query_params.starttime);

      if (sq_restrictions)
        check_time_interval(query_params.starttime, query_params.endtime, max_hours);

      query_params.parameters = bs_param;
      bool have_meteo_param = false;
      std::vector<std::string> param_names;
      params.get<std::string>(P_PARAM, std::back_inserter(param_names));
      int first_param = 0, last_param = 0;
      for (const std::string& name : param_names)
      {
        SmartMet::Spine::Parameter param = SmartMet::TimeSeries::makeParameter(name);
        query_params.parameters.push_back(param);
        int ind = query_params.parameters.size() - 1;
        if (first_param == 0)
          first_param = ind;
        last_param = ind;
        if (!TimeSeries::special(param))
          have_meteo_param = true;
      }

      query_params.allplaces = false;
      query_params.stationtype = station_type;
      query_params.maxdistance = 20000.0;
      query_params.timeformat = "iso";
      query_params.timezone = "UTC";
      query_params.numberofstations = 1;
      query_params.missingtext = missing_text;
      query_params.localename = get_config()->get_locale_name();
      query_params.starttimeGiven = true;

      const std::string tz_name = get_tz_name(params);
      Fmi::TimeZonePtr tzp = get_time_zone(tz_name);

      boost::shared_ptr<SmartMet::Spine::CRSRegistry::Transformation> to_bbox_transform;

      // boost::optional<> aiheuttaa täällä strict aliasing varoituksen jos gcc-4.4.X
      // on käytössä. Sen vuoksi std::unique_ptr on käytetty boost::optionla tilalle.

      SmartMet::Spine::BoundingBox requested_bbox;
      SmartMet::Spine::BoundingBox query_bbox;
      bool have_bbox = get_bounding_box(params, crs, &requested_bbox);
      bool bb_swap_coord = false;
      std::string bb_proj_uri;

      if (have_bbox)
      {
        query_bbox = transform_bounding_box(requested_bbox, DATA_CRS_NAME);
        to_bbox_transform = crs_registry.create_transformation(DATA_CRS_NAME, requested_bbox.crs);
        query_params.boundingBox["minx"] = query_bbox.xMin;
        query_params.boundingBox["miny"] = query_bbox.yMin;
        query_params.boundingBox["maxx"] = query_bbox.xMax;
        query_params.boundingBox["maxy"] = query_bbox.yMax;

        crs_registry.get_attribute(requested_bbox.crs, "swapCoord", &bb_swap_coord);
        crs_registry.get_attribute(requested_bbox.crs, "projUri", &bb_proj_uri);

        if (debug_level > 0)
        {
          std::ostringstream msg;
          msg << SmartMet::Spine::log_time_str() << ": [WFS] [DEBUG] Original bounding box: ("
              << requested_bbox.xMin << " " << requested_bbox.yMin << " " << requested_bbox.xMax
              << " " << requested_bbox.yMax << " " << requested_bbox.crs << ")\n";
          msg << "                                   "
              << " Converted bounding box: (" << query_bbox.xMin << " " << query_bbox.yMin << " "
              << query_bbox.xMax << " " << query_bbox.yMax << " " << query_bbox.crs << ")\n";
          std::cout << msg.str() << std::flush;
        }
      }

      Fmi::ValueFormatterParam vf_param;
      vf_param.missingText = query_params.missingtext;
      boost::shared_ptr<Fmi::ValueFormatter> value_formatter(new Fmi::ValueFormatter(vf_param));

      if (query_params.starttime > query_params.endtime)
      {
        Fmi::Exception exception(BCP, "Invalid time interval!");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
        exception.addParameter("Start time", pt::to_simple_string(query_params.starttime));
        exception.addParameter("End time", pt::to_simple_string(query_params.endtime));
        throw exception;
      }

      // FIXME: add parameter to restrict number of days per query

      if (not have_meteo_param)
      {
        Fmi::Exception exception(BCP, "No meteo parameter found!");
        exception.addDetail("At least one meteo parameter must be specified!");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
        throw exception.disableStackTrace();
      }

      // Fetch the values

      const auto result_ptr = obs_engine->values(query_params);

      CTPP::CDT hash;

      // Get the sequence number of query in the request
      int sq_id = query.get_query_id();

      auto projSrsDim = (show_height ? 3 : 2);

      bw::FeatureID feature_id(get_config()->get_query_id(), params.get_map(true), sq_id);

      hash["language"] = language;

      hash["responseTimestamp"] =
          Fmi::to_iso_extended_string(get_plugin_impl().get_time_stamp()) + "Z";
      hash["queryNum"] = query.get_query_id();

      hash["featureId"] = feature_id.get_id();
      // FIXME: Do we need separate feature ID for each parameter?

      hash["fmi_apikey"] = bw::QueryBase::FMI_APIKEY_SUBST;
      hash["fmi_apikey_prefix"] = bw::QueryBase::FMI_APIKEY_PREFIX_SUBST;
      hash["hostname"] = QueryBase::HOSTNAME_SUBST;
      hash["protocol"] = QueryBase::PROTOCOL_SUBST;
      hash["srsName"] = proj_uri;
      hash["projSrsDim"] = projSrsDim;
      hash["srsEpochName"] = proj_epoch_uri;
      hash["projEpochSrsDim"] = (show_height ? 4 : 3);

      hash["memberId"] = "enn-m-1";
      hash["resultTimeId"] = "time-1";
      hash["samplingFeatureId"] = "flash-s-1";
      hash["sampledFeatureTargetId"] = "area-1";
      hash["sampledFeatureTargetPolygonId"] = "polygon-1";
      hash["resultCoverageId"] = "mpcv-1";
      hash["resultCoverageMultiPointId"] = "mp-1";

      for (int k = first_param; k <= last_param; k++)
      {
        const int k0 = k - first_param;
        const std::string& name = param_names.at(k0);
        feature_id.erase_param(P_PARAM);
        feature_id.add_param(P_PARAM, name);
        hash["paramList"][k0]["name"] = param_names.at(k0);
        hash["paramList"][k0]["featureID"] = feature_id.get_id();
      }

      if (have_bbox)
      {
        // FIXME: missä projektion pitäisi käyttää täälä. Todennäköisesti alkuperainen
        //        bbox projektio olisi parempi
        CTPP::CDT p_bb;
        double x_ll = bb_swap_coord ? requested_bbox.yMin : requested_bbox.xMin;
        double y_ll = bb_swap_coord ? requested_bbox.xMin : requested_bbox.yMin;
        double x_ur = bb_swap_coord ? requested_bbox.yMax : requested_bbox.xMax;
        double y_ur = bb_swap_coord ? requested_bbox.xMax : requested_bbox.yMax;

        p_bb["lowerLeft"]["x"] = x_ll;
        p_bb["lowerLeft"]["y"] = y_ll;

        p_bb["lowerRight"]["x"] = x_ur;
        p_bb["lowerRight"]["y"] = y_ll;

        p_bb["upperLeft"]["x"] = x_ll;
        p_bb["upperLeft"]["y"] = y_ur;

        p_bb["upperRight"]["x"] = x_ur;
        p_bb["upperRight"]["y"] = y_ur;

        p_bb["projUri"] = bb_proj_uri;
        p_bb["srsDim"] = 2;

        hash["metadata"]["boundingBox"] = p_bb;
      }

      std::size_t used_rows = 0;
      std::size_t num_skipped = 0;
      std::size_t num_rows = 0;

      // generated multipoint mode coordinates and values
      std::string multipoint_position_rows;  // lat lon epoch
      std::string multipoint_data_rows;      // param1 param2 ...

      // generated simple query xml
      std::string simple_rows;  // xml block 1, xml block 2 ...

      if (result_ptr && !result_ptr->empty())
      {
        static const long ref_jd = Fmi::Date(1970, 1, 1).julian_day();

        const auto& result = *result_ptr;
        num_rows = result[1].size();

        for (std::size_t i = 0; i < num_rows; i++)
        {
          // I have no clue why the ordering is this, this changed code
          // does the same as the original (sx = lat, sy = lon) - Mika

          double lon = boost::get<double>(result[lat_ind][i].value);
          double lat = boost::get<double>(result[lon_ind][i].value);

          if (to_bbox_transform)
          {
            try
            {
              NFmiPoint p1(lon, lat);
              NFmiPoint p2 = to_bbox_transform->transform(p1);

              double x = bb_swap_coord ? p2.Y() : p2.X();
              double y = bb_swap_coord ? p2.X() : p2.Y();

              if ((x < requested_bbox.xMin) or (x > requested_bbox.xMax) or
                  (y < requested_bbox.yMin) or (y > requested_bbox.yMax))
              {
                if (debug_level > 3)
                {
                  std::ostringstream msg;
                  msg << SmartMet::Spine::log_time_str() << ": [WFS] [DEBUG] Event (" << p1.X()
                      << " " << p1.Y() << ") ==> (" << p2.X() << " " << p2.Y()
                      << ") skipped due to bounding box check\n";
                  std::cout << msg.str() << std::flush;
                }
                num_skipped++;
                continue;
              }
            }
            catch (...)
            {
              // Ignore errors while extra bounding box checks
              //(void)err;
            }
          }

          auto str_xy = get_2D_coord(transformation, lon, lat);

          const Fmi::DateTime epoch = result[lon_ind][i].time.utc_time();
          long long jd = epoch.date().julian_day();
          long seconds = epoch.time_of_day().total_seconds();
          INT_64 s_epoch = 86400LL * (jd - ref_jd) + seconds;
          auto stroke_time_str = format_local_time(epoch, tzp);

          ++used_rows;

          if (!is_simple_query)
          {
            multipoint_position_rows +=
                str_xy.first + ' ' + str_xy.second + ' ' + Fmi::to_string(s_epoch) + '\n';
          }

          for (int k = first_param; k <= last_param; k++)
          {
            std::string value;
            if (boost::get<int>(&result[k][i].value))
              value = Fmi::to_string(boost::get<int>(result[k][i].value));
            else if (boost::get<double>(&result[k][i].value))
            {
              auto tmp = boost::get<double>(result[k][i].value);
              if (static_cast<int>(tmp) != tmp)
                value = value_formatter->format(tmp, 1);
              else
                value = Fmi::to_string(tmp);
            }
            else
              value = query_params.missingtext;

            if (!is_simple_query)
            {
              multipoint_data_rows += remove_trailing_0(value);
              multipoint_data_rows += (k < last_param ? ' ' : '\n');
            }
            else
            {
              std::string fmt = R"(
<wfs:member>
 <BsWfs:BsWfsElement gml:id="BsWfsElement.{0}.{1}">
  <BsWfs:Location>
   <gml:Point gml:id="BsWfsElementP.{0}.{1}" srsDimension="{2}" srsName="{3}">
    <gml:pos>{4} {5}</gml:pos>
   </gml:Point>
  </BsWfs:Location>
  <BsWfs:Time>{6}</BsWfs:Time>
  <BsWfs:ParameterName>{7}</BsWfs:ParameterName>
  <BsWfs:ParameterValue>{8}</BsWfs:ParameterValue>
 </BsWfs:BsWfsElement>
</wfs:member>)";
              simple_rows += fmt::format(fmt::runtime(fmt),
                                         used_rows,
                                         k - first_param + 1,
                                         projSrsDim,
                                         proj_uri,
                                         str_xy.first,
                                         str_xy.second,
                                         stroke_time_str,
                                         param_names.at(k - first_param),
                                         value);
            }
          }
        }
      }

      if (debug_level > 1 and num_skipped)
      {
        std::ostringstream msg;
        msg << SmartMet::Spine::log_time_str() << " [WFS] [DEBUG] " << num_skipped << " of "
            << num_rows << " lightning observations skipped due to bounding box check\n";
        std::cout << msg.str() << std::flush;
      }

      if (is_simple_query)
        hash["dataRows"] = simple_rows;
      else
      {
        if (!multipoint_data_rows.empty())
          hash["dataRows"] = multipoint_data_rows;  // the template behaves differently when not set
        hash["positionRows"] = multipoint_position_rows;
      }

      hash["numMatched"] = used_rows == 0 ? 0 : 1;
      hash["numReturned"] = used_rows == 0 ? 0 : 1;

      hash["numberMatched"] = used_rows * (last_param - first_param + 1);
      hash["numberReturned"] = used_rows * (last_param - first_param + 1);

      hash["phenomenonTimeId"] = "time-interval-1";
      hash["phenomenonStartTime"] = Fmi::to_iso_extended_string(query_params.starttime) + "Z";
      hash["phenomenonEndTime"] = Fmi::to_iso_extended_string(query_params.endtime) + "Z";

      // std::cout << "Hash = \n" << hash.RecursiveDump() << std::endl;

      format_output(hash, output, query.get_use_debug_format());
    }
    catch (...)
    {
      Fmi::Exception exception(BCP, "Operation failed!", nullptr);
      // Set language for exception and re-throw it
      exception.addParameter(WFS_LANGUAGE, language);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> wfs_flash_handler_create(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    boost::optional<std::string> template_file_name)
{
  try
  {
    auto* qh = new StoredFlashQueryHandler(reactor, config, plugin_data, template_file_name);
    boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> result(qh);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_flash_handler_factory(
    &wfs_flash_handler_create);

/**

@page WFS_SQ_FLASH_OBS_HANDLER Stored Query handler for querying lightning observations

@section WFS_FLASH_OBS_HANDLER_INTRO Introduction

This stored query handler provides access to FMI lightning observations

<table border="1">
  <tr>
     <td>Implementation</td>
     <td>SmartMet::Plugin::WFS::StoredFlashQueryHandler</td>
  </tr>
  <tr>
    <td>Constructor name (for stored query configuration)</td>
    <td>@b wfs_flash_handler_factory</td>
  </tr>
</table>


@section WFS_SQ_FLASH_OBS_HANDLER_PARAMS Query handler built-in parameters

The following stored query handler parameter groups are being used by this stored query handler:
- @ref WFS_SQ_PARAM_BBOX
- @ref WFS_SQ_TIME_ZONE

Additionally to the parameters from these stored query handler parameter groups the
following built-in handler parameters are being used:

<table border="1">

<tr>
<th>Entry name</th>
<th>Type</th>
<th>Data type</th>
<th>Description</th>
</tr>

<tr>
  <td>beginTime</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>time</td>
  <td>Specifies the time of the begin of time interval</td>
</tr>

<tr>
  <td>endTime</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>time</td>
  <td>Specifies the time of the end of time interval</td>
</tr>

<tr>
  <td>meteoParameters</td>
  <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
  <td>string</td>
  <td>Specifies the meteo parameters to query</td>
</tr>

<tr>
  <td>CRS</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>string</td>
  <td>The coordinate projection to use</td>
</tr>

</table>

@section WFS_SQ_FLASH_OBS_HANLDER_EXTRA_CFG_PARAM Additional parameters

This section describes this stored query handler configuration parameters which does not map to
stored query parameters
and must be specified on top level of stored query configuration file when present

<table border="1">

<tr>
<th>Entry name</th>
<th>Type</th>
<th>Use</th>
<th>Description</th>
</tr>

<tr>
  <td>stationType</td>
  <td>string</td>
  <td>optional (default @b flash)</td>
  <td>Specifies station type. Intended to be used if distinction between commercial and open data
services
      will be implemented as different station types</td>
</tr>

<tr>
  <td>maxHours</td>
  <td>double</td>
  <td>optional</td>
  <td>Specifies maximal permitted time interval in hours. The default value
      is 168 hours (7 days). This restriction is not valid, if the optional
      storedQueryRestrictions parameter is set to false in WFS Plugin
      configurations.</td>
</tr>

<tr>
  <td>missingText</td>
  <td>string</td>
  <td>optional</td>
  <td>Specifies text to use in response for missing values.</td>
</tr>

<tr>
  <td>timeBlockSize</td>
  <td>integer</td>
  <td>optional</td>
  <td>Specifies the time step for lightning observations in seconds. The observations
      are returned in blocks of this length. Only full blocks are supported.
      This parameter is not valid, if the *optional* storedQueryRestrictions
      parameter is set to false in WFS Plugin configurations.</td>
</tr>

</table>


*/
