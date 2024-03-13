#include "stored_queries/StoredObsQueryHandler.h"
#include "FeatureID.h"
#include "StoredQueryHandlerFactoryDef.h"
#include "WfsConst.h"
#include "WfsConvenience.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <spine/Convenience.h>
#include <spine/Value.h>
#include <timeseries/ParameterTools.h>
#include <timeseries/TimeSeriesInclude.h>
#include <algorithm>
#include <functional>

#define P_BEGIN_TIME "beginTime"
#define P_END_TIME "endTime"
#define P_METEO_PARAMETERS "meteoParameters"
#define P_STATION_TYPE "stationType"
#define P_TIME_STEP "timeStep"
#define P_LPNNS "lpnns"
#define P_NUM_OF_STATIONS "numOfStations"
#define P_HOURS "hours"
#define P_WEEK_DAYS "weekDays"
#define P_LOCALE "locale"
#define P_MISSING_TEXT "missingText"
#define P_MAX_EPOCHS "maxEpochs"
#define P_CRS "crs"
#define P_LATEST "latest"
#define P_LANGUAGE "language"

namespace ba = boost::algorithm;
namespace bl = boost::lambda;
using namespace SmartMet::Spine;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
using boost::format;
using boost::str;

StoredObsQueryHandler::StoredObsQueryHandler(SmartMet::Spine::Reactor* reactor,
                                             StoredQueryConfig::Ptr config,
                                             PluginImpl& plugin_data,
                                             boost::optional<std::string> template_file_name)
    : StoredQueryParamRegistry(config),
      SupportsExtraHandlerParams(config, false),
      RequiresGeoEngine(reactor),
      RequiresObsEngine(reactor),
      StoredQueryHandlerBase(reactor, config, plugin_data, template_file_name),
      SupportsLocationParameters(
          reactor,
          config,
          SUPPORT_KEYWORDS | INCLUDE_GEOIDS | INCLUDE_FMISIDS | INCLUDE_WMOS | INCLUDE_LPNNS),
      SupportsBoundingBox(config, plugin_data.get_crs_registry()),
      SupportsTimeZone(reactor, config),
      SupportsQualityParameters(config),
      SupportsMeteoParameterOptions(config),
      SupportsTimeParameters(config),
      initial_bs_param(),
      fmisid_ind(SmartMet::add_param(
          initial_bs_param, "fmisid", Parameter::Type::DataIndependent, kFmiFMISID)),
      geoid_ind(SmartMet::add_param(
          initial_bs_param, "geoid", Parameter::Type::DataIndependent, kFmiGEOID)),
      lon_ind(SmartMet::add_param(
          initial_bs_param, "stationlon", Parameter::Type::DataDerived, kFmiStationLongitude)),
      lat_ind(SmartMet::add_param(
          initial_bs_param, "stationlat", Parameter::Type::DataDerived, kFmiStationLatitude)),
      height_ind(SmartMet::add_param(
          initial_bs_param, "elevation", Parameter::Type::DataIndependent, kFmiStationElevation)),
      name_ind(SmartMet::add_param(
          initial_bs_param, "stationname", Parameter::Type::DataIndependent, kFmiStationName)),
      dist_ind(SmartMet::add_param(
          initial_bs_param, "distance", Parameter::Type::DataIndependent, kFmiDistance)),
      direction_ind(SmartMet::add_param(
          initial_bs_param, "direction", Parameter::Type::DataIndependent, kFmiDirection)),
      wmo_ind(SmartMet::add_param(
          initial_bs_param, "wmo", Parameter::Type::DataIndependent, kFmiWmoStationNumber))
{
  try
  {
    register_scalar_param<bool>(P_LATEST, "");

    register_array_param<std::string>(
        P_METEO_PARAMETERS,
        "array of fields whose values should be returned in the response.",
        true);

    register_scalar_param<std::string>(
        P_STATION_TYPE,
        "The type of the observation station (defined in the ObsEngine configuration)");

    register_scalar_param<uint64_t>(
        P_NUM_OF_STATIONS,
        "The maximum number of the observation stations returned around the given"
        " geographical location (inside the radius of \"maxDistance\")");

    register_array_param<int64_t>(P_WEEK_DAYS, "requested times expressed in the list of weekdays");

    register_scalar_param<std::string>(P_LOCALE, "value of the 'Locale' (for example fi_FI.utf8).");

    register_scalar_param<std::string>(
        P_MISSING_TEXT, "value that is returned when the value of the requested field is missing.");

    register_scalar_param<uint64_t>(
        P_MAX_EPOCHS,
        "maximum number of time epochs that can be returned. If the estimated number before"
        " query is larger than the specified one then the query is aborted. This parameter"
        " is not alid if the \"storedQueryRestrictions\" parameter is set to \"false\" in"
        " the WFS Plugin configuration file.");

    register_scalar_param<std::string>(P_CRS, "coordinate projection used in the response.");

    register_scalar_param<std::string>(P_LANGUAGE, "The language to use", false, true);

    max_hours = config->get_optional_config_param<double>("maxHours", 7.0 * 24.0);
    max_station_count = config->get_optional_config_param<unsigned>("maxStationCount", 0);
    separate_groups = config->get_optional_config_param<bool>("separateGroups", false);
    sq_restrictions = plugin_data.get_config().getSQRestrictions();
    m_support_qc_parameters = config->get_optional_config_param<bool>("supportQCParameters", false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

StoredObsQueryHandler::~StoredObsQueryHandler() = default;

std::string StoredObsQueryHandler::get_handler_description() const
{
  return "Observation data (general, no lightning, aviation or multi-sensor)";
}

void StoredObsQueryHandler::query(const StoredQuery& query,
                                  const std::string& language,
                                  const boost::optional<std::string>& /*hostname*/,
                                  std::ostream& output) const
{
  std::string curr_lang = language;
  try
  {
    namespace pt = boost::posix_time;
    using namespace SmartMet;
    using namespace SmartMet::Plugin::WFS;

    const int debug_level = get_config()->get_debug_level();
    if (debug_level > 0)
      query.dump_query_info(std::cout);

    // If 'data_source'-field exists
    // extend it so that for every meteo.parameter
    // a corresponding '*data_source'-field is added,
    // for example, Temperature -> Temperature_data_source
    auto params = query.get_param_map();

    std::set<std::string> keys = params.get_keys();
    for (const auto& k : keys)
    {
      if (k == "meteoParameters")
      {
        std::vector<SmartMet::Spine::Value> values = params.get_values(k);
        std::vector<std::string> mparams;
        bool dataSourceFound = false;
        for (const auto& v : values)
        {
          std::string val = v.to_string();
          if (val == "data_source")
          {
            dataSourceFound = true;
          }
          else
          {
            mparams.push_back(val);
          }
        }
        if (dataSourceFound && !mparams.empty())
        {
          params.remove("meteoParameters");
          for (const auto& p : mparams)
          {
            std::string fieldname = p + "_data_source";
            params.add("meteoParameters", p);
            params.add("meteoParameters", fieldname);
          }
        }
        break;
      }
    }

    try
    {
      std::vector<int64_t> tmp_vect;
      SmartMet::Engine::Observation::Settings query_params;
      query_params.useDataCache = true;
      query_params.localTimePool = std::make_shared<TS::LocalTimePool>();

      const char* DATA_CRS_NAME = "urn:ogc:def:crs:EPSG::4326";

      query_params.starttime = params.get_single<Fmi::DateTime>(P_BEGIN_TIME);
      query_params.endtime = params.get_single<Fmi::DateTime>(P_END_TIME);

      if (params.get_optional<bool>(P_LATEST, false))
        query_params.wantedtime = query_params.endtime;

      if (sq_restrictions)
        check_time_interval(query_params.starttime, query_params.endtime, max_hours);

      std::vector<std::string> param_names;
      bool have_meteo_param =
          params.get<std::string>(P_METEO_PARAMETERS, std::back_inserter(param_names), 1);

      query_params.stationtype =
          Fmi::ascii_tolower_copy(params.get_single<std::string>(P_STATION_TYPE));

      const bool support_quality_info = SupportsQualityParameters::supportQualityInfo(params);

      std::vector<ExtParamIndexEntry> param_index;
      query_params.parameters = initial_bs_param;
      have_meteo_param &= add_parameters(params, param_names, query_params, param_index);

      if (not have_meteo_param)
      {
        Fmi::Exception exception(BCP, "Operation processin failed!");
        exception.addDetail("At least one meteo parameter must be specified");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
        exception.disableStackTrace();
        throw exception;
      }

      const auto crs = params.get_single<std::string>(P_CRS);
      auto transformation = crs_registry.create_transformation(DATA_CRS_NAME, crs);
      bool show_height = false;
      std::string proj_uri = "UNKNOWN";
      std::string proj_epoch_uri = "UNKNOWN";
      crs_registry.get_attribute(crs, "showHeight", &show_height);
      crs_registry.get_attribute(crs, "projUri", &proj_uri);
      crs_registry.get_attribute(crs, "projEpochUri", &proj_epoch_uri);

      curr_lang = params.get_optional<std::string>(P_LANGUAGE, curr_lang);

      auto timestep = params.get_single<uint64_t>(P_TIME_STEP);
      // query_params.timestep = (timestep > 0 ? timestep : 1);
      query_params.timestep = (timestep > 0 ? timestep : 0);

      query_params.allplaces = false;

      query_params.maxdistance = params.get_single<double>(P_MAX_DISTANCE);

      query_params.timeformat = "iso";

      query_params.timezone = "UTC";

      query_params.numberofstations = params.get_single<uint64_t>(P_NUM_OF_STATIONS);

      query_params.missingtext = params.get_single<std::string>(P_MISSING_TEXT);

      query_params.localename = params.get_single<std::string>(P_LOCALE);

      query_params.starttimeGiven = true;

      query_params.language = curr_lang;

      params.get<int64_t>(P_HOURS, std::back_inserter(query_params.hours));

      params.get<int64_t>(P_WEEK_DAYS, std::back_inserter(query_params.weekdays));

      const std::string tz_name = get_tz_name(params);

      std::unique_ptr<std::locale> curr_locale;

      SmartMet::Engine::Observation::StationSettings stationSettings;

      params.get<int64_t>(P_WMOS, std::back_inserter(stationSettings.wmos));

      params.get<int64_t>(P_LPNNS, std::back_inserter(stationSettings.lpnns));

      params.get<int64_t>(P_FMISIDS, std::back_inserter(stationSettings.fmisids));

      if (query_params.stationtype != ICEBUOY_PRODUCER &&
          query_params.stationtype != COPERNICUS_PRODUCER)
      {
        std::list<std::pair<std::string, SmartMet::Spine::LocationPtr> > locations_list;
        get_location_options(params, curr_lang, &locations_list);

        for (const auto& item : locations_list)
        {
          stationSettings.nearest_station_settings.emplace_back(item.second->longitude,
                                                                item.second->latitude,
                                                                query_params.maxdistance,
                                                                query_params.numberofstations,
                                                                item.first,
                                                                item.second->fmisid);
        }
      }

      params.get<int64_t>(P_GEOIDS, std::back_inserter(stationSettings.geoid_settings.geoids));
      if (!stationSettings.geoid_settings.geoids.empty())
      {
        stationSettings.geoid_settings.maxdistance = query_params.maxdistance;
        stationSettings.geoid_settings.numberofstations = query_params.numberofstations;
        stationSettings.geoid_settings.language = curr_lang;
      }

      using SmartMet::Spine::BoundingBox;
      BoundingBox requested_bbox;
      bool have_bbox = get_bounding_box(params, crs, &requested_bbox);
      std::unique_ptr<BoundingBox> query_bbox;
      if (have_bbox)
      {
        query_bbox.reset(new BoundingBox);
        *query_bbox = transform_bounding_box(requested_bbox, DATA_CRS_NAME);
        stationSettings.bounding_box_settings["minx"] = query_bbox->xMin;
        stationSettings.bounding_box_settings["miny"] = query_bbox->yMin;
        stationSettings.bounding_box_settings["maxx"] = query_bbox->xMax;
        stationSettings.bounding_box_settings["maxy"] = query_bbox->yMax;

        query_params.boundingBox["minx"] = query_bbox->xMin;
        query_params.boundingBox["miny"] = query_bbox->yMin;
        query_params.boundingBox["maxx"] = query_bbox->xMax;
        query_params.boundingBox["maxy"] = query_bbox->yMax;

        if (debug_level > 0)
        {
          std::ostringstream msg;
          msg << SmartMet::Spine::log_time_str() << ": [WFS] [DEBUG] Original bounding box: ("
              << requested_bbox.xMin << " " << requested_bbox.yMin << " " << requested_bbox.xMax
              << " " << requested_bbox.yMax << " " << requested_bbox.crs << ")\n";
          msg << "                                   "
              << " Converted bounding box: (" << query_bbox->xMin << " " << query_bbox->yMin << " "
              << query_bbox->xMax << " " << query_bbox->yMax << " " << query_bbox->crs << ")\n";
          std::cout << msg.str() << std::flush;
        }
      }

      if (query_params.starttime > query_params.endtime)
      {
        Fmi::Exception exception(BCP, "Invalid time interval!");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
        exception.addParameter("Start time", Fmi::date_time::to_simple_string(query_params.starttime));
        exception.addParameter("End time", Fmi::date_time::to_simple_string(query_params.endtime));
        throw exception.disableStackTrace();
      }

      // Avoid an attempt to dump too much data.
      unsigned maxEpochs = params.get_single<uint64_t>(P_MAX_EPOCHS);
      unsigned ts1 = (timestep ? timestep : 60);
      if (sq_restrictions and
          query_params.starttime + Fmi::Minutes(maxEpochs * ts1) < query_params.endtime)
      {
        Fmi::Exception exception(BCP, "Too many time epochs in the time interval!");
        exception.addDetail("Use shorter time interval or larger time step.");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
        exception.addParameter("Start time", Fmi::date_time::to_simple_string(query_params.starttime));
        exception.addParameter("End time", Fmi::date_time::to_simple_string(query_params.endtime));
        throw exception.disableStackTrace();
      }

      if (sq_restrictions and max_station_count > 0 and have_bbox)
      {
        Stations stations;
        obs_engine->getStationsByBoundingBox(stations, query_params);
        if (stations.size() > max_station_count)
        {
          Fmi::Exception exception(
              BCP,
              "Too many stations (" + std::to_string(stations.size()) + ") in the bounding box!");
          exception.addDetail("No more than " + std::to_string(max_station_count) + " is allowed.");
          exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
          throw exception.disableStackTrace();
        }
      }

      /*******************************************
       *  Finally perform the query              *
       *******************************************/
      query_params.taggedFMISIDs = obs_engine->translateToFMISID(query_params, stationSettings);

      TS::TimeSeriesVectorPtr obsengine_result(obs_engine->values(query_params));

      const bool emptyResult = (!obsengine_result || obsengine_result->empty());

      CTPP::CDT hash;

      // Create index of all result rows (by observation site)
      std::map<std::string, SiteRec> site_map;
      std::map<int, GroupRec> group_map;

      Fmi::DateTime now = get_plugin_impl().get_time_stamp();
      std::shared_ptr<Fmi::TimeFormatter> tfmt(Fmi::TimeFormatter::create("iso"));

      // Formatting the TS::Value values.
      Fmi::ValueFormatter fmt{Fmi::ValueFormatterParam()};
      fmt.setMissingText(query_params.missingtext);
      TS::StringVisitor sv(fmt, 3);

      if (not emptyResult)
      {
        const TS::TimeSeries& ts_fmisid = obsengine_result->at(fmisid_ind);
        for (std::size_t i = 0; i < ts_fmisid.size(); i++)
        {
          // Fmisids are doubles for some reason, fix this!!!!
          sv.setPrecision(0);
          const std::string fmisid = boost::apply_visitor(sv, ts_fmisid[i].value);
          site_map[fmisid].row_index_vect.push_back(i);
        }
      }

      // Get the sequence number of query in the request
      int sq_id = query.get_query_id();

      const int num_groups = separate_groups ? site_map.size() : ((!site_map.empty()) ? 1 : 0);

      FeatureID feature_id(get_config()->get_query_id(), params.get_map(true), sq_id);

      if (separate_groups)
      {
        // Assign group IDs to stations for separate groups requested
        int group_cnt = 0;
        for (auto& item : site_map)
        {
          const std::string& fmisid = item.first;
          const int group_id = group_cnt++;
          item.second.group_id = group_id;
          item.second.ind_in_group = 0;

          const char* place_params[] = {
              P_WMOS, P_LPNNS, P_FMISIDS, P_PLACES, P_LATLONS, P_GEOIDS, P_KEYWORD, P_BOUNDING_BOX};
          for (auto& place_param : place_params)
          {
            feature_id.erase_param(place_param);
          }
          feature_id.add_param(P_FMISIDS, fmisid);
          group_map[group_id].feature_id = feature_id.get_id();

          for (const std::string& param_name : param_names)
          {
            feature_id.erase_param(P_METEO_PARAMETERS);
            feature_id.add_param(P_METEO_PARAMETERS, param_name);
            group_map[group_id].param_ids[param_name] = feature_id.get_id();
          }
        }
      }
      else
      {
        const int group_id = 0;
        int ind_in_group = 0;
        for (auto& item : site_map)
        {
          item.second.group_id = group_id;
          item.second.ind_in_group = ind_in_group++;
        }

        group_map[group_id].feature_id = feature_id.get_id();

        for (const std::string& param_name : param_names)
        {
          feature_id.erase_param(P_METEO_PARAMETERS);
          feature_id.add_param(P_METEO_PARAMETERS, param_name);
          group_map[group_id].param_ids[param_name] = feature_id.get_id();
        }
      }

      hash["responseTimestamp"] = Fmi::to_iso_extended_string(now) + "Z";
      hash["numMatched"] = num_groups;
      hash["numReturned"] = num_groups;
      hash["numParam"] = param_names.size();
      hash["language"] = curr_lang;
      hash["projSrsDim"] = (show_height ? 3 : 2);
      hash["projSrsName"] = proj_uri;
      hash["projEpochSrsDim"] = (show_height ? 4 : 3);
      hash["projEpochSrsName"] = proj_epoch_uri;
      hash["queryNum"] = query.get_query_id();
      hash["queryName"] = get_query_name();

      params.dump_params(hash["query_parameters"]);
      dump_named_params(params, hash["named_parameters"]);

      hash["fmi_apikey"] = QueryBase::FMI_APIKEY_SUBST;
      hash["fmi_apikey_prefix"] = QueryBase::FMI_APIKEY_PREFIX_SUBST;
      hash["hostname"] = QueryBase::HOSTNAME_SUBST;
      hash["protocol"] = QueryBase::PROTOCOL_SUBST;

      if (not emptyResult)
      {
        const TS::TimeSeries& ts_lat = obsengine_result->at(lat_ind);
        const TS::TimeSeries& ts_lon = obsengine_result->at(lon_ind);
        const TS::TimeSeries& ts_height = obsengine_result->at(height_ind);
        const TS::TimeSeries& ts_name = obsengine_result->at(name_ind);
        const TS::TimeSeries& ts_dist = obsengine_result->at(dist_ind);
        const TS::TimeSeries& ts_direction = obsengine_result->at(direction_ind);
        const TS::TimeSeries& ts_geoid = obsengine_result->at(geoid_ind);
        const TS::TimeSeries& ts_wmo = obsengine_result->at(wmo_ind);

        std::map<std::string, SmartMet::Spine::LocationPtr> sites;

        for (const auto& it1 : site_map)
        {
          const std::string& fmisid = it1.first;
          const int group_id = it1.second.group_id;
          const int ind = it1.second.ind_in_group;
          const int row_1 = it1.second.row_index_vect.at(0);

          CTPP::CDT& group = hash["groups"][group_id];

          group["obsStationList"][ind]["fmisid"] = fmisid;

          sv.setPrecision(5);
          const std::string lat = boost::apply_visitor(sv, ts_lat[row_1].value);
          const std::string lon = boost::apply_visitor(sv, ts_lon[row_1].value);

          sv.setPrecision(1);
          const std::string height = boost::apply_visitor(sv, ts_height[row_1].value);
          const std::string name = boost::apply_visitor(sv, ts_name[row_1].value);
          const std::string distance = boost::apply_visitor(sv, ts_dist[row_1].value);
          const std::string bearing = boost::apply_visitor(sv, ts_direction[row_1].value);
          const std::string geoid = boost::apply_visitor(sv, ts_geoid[row_1].value);
          const std::string wmo = boost::apply_visitor(sv, ts_wmo[row_1].value);

          if (not lat.empty() and not lon.empty())
            set_2D_coord(transformation, lat, lon, group["obsStationList"][ind]);
          else
            throw Fmi::Exception(BCP, "wfs: Internal LatLon query error.");

          // Region solving
          std::string region;
          SmartMet::Spine::LocationPtr geoLoc;
          if (not geoid.empty())
          {
            try
            {
              const long geoid_long = Fmi::stol(geoid);
              std::string langCode = curr_lang;
              SupportsLocationParameters::engOrFinToEnOrFi(langCode);
              geoLoc = geo_engine->idSearch(geoid_long, langCode);
              region = (geoLoc ? geoLoc->area : "");
            }
            catch (...)
            {
            }

            sites[geoid] = geoLoc;
          }

          if (show_height)
            group["obsStationList"][ind]["height"] =
                (height.empty() ? query_params.missingtext : height);
          group["obsStationList"][ind]["distance"] =
              (distance.empty() ? query_params.missingtext : distance);
          group["obsStationList"][ind]["bearing"] =
              ((distance.empty() or bearing.empty() or (distance == query_params.missingtext))
                   ? query_params.missingtext
                   : bearing);
          group["obsStationList"][ind]["name"] = name;

          if (not region.empty())
            group["obsStationList"][ind]["region"] = region;

          if (not wmo.empty() && wmo != "0" && wmo != "NaN")
            group["obsStationList"][ind]["wmo"] = wmo;

          (geoid.empty() or geoid == "-1")
              ? group["obsStationList"][ind]["geoid"] = query_params.missingtext
              : group["obsStationList"][ind]["geoid"] = geoid;
        }

        for (int group_id = 0; group_id < num_groups; group_id++)
        {
          int ind = 0;
          CTPP::CDT& group = hash["groups"][group_id];

          // Time values of a group.
          group["obsPhenomenonStartTime"] =
              Fmi::to_iso_extended_string(query_params.starttime) + "Z";
          group["obsPhenomenonEndTime"] = Fmi::to_iso_extended_string(query_params.endtime) + "Z";
          group["obsResultTime"] = Fmi::to_iso_extended_string(query_params.endtime) + "Z";

          group["featureId"] = group_map.at(group_id).feature_id;
          group["crs"] = crs;
          if (tz_name != "UTC")
            group["timezone"] = tz_name;
          group["timestep"] = query_params.timestep;
          if (support_quality_info)
            group["qualityInfo"] = "on";

          for (std::size_t k = 0; k < param_index.size(); k++)
          {
            const auto& entry = param_index[k];
            const std::string& name = entry.p.name;
            group["obsParamList"][k]["name"] =
                (entry.p.sensor_name ? *entry.p.sensor_name : entry.p.name);
            group["obsParamList"][k]["featureId"] = group_map.at(group_id).param_ids.at(name);

            // Mark QC parameters
            if (SupportsQualityParameters::isQCParameter(name))
            {
              group["obsParamList"][k]["isQCParameter"] = "true";
            }
          }

          // Reference id of om:phenomenonTime and om:resultTime in
          // a group (a station). The first parameter in a list
          // get the time values (above) and other ones get only
          // references to the values.
          const std::string group_id_str = str(format("%1%-%2%") % sq_id % (group_id + 1));
          group["groupId"] = group_id_str;
          group["groupNum"] = group_id + 1;

          for (const auto& it1 : site_map)
          {
            const int row_1 = it1.second.row_index_vect.at(0);

            if (it1.second.group_id == group_id)
            {
              bool first = true;
              Fmi::TimeZonePtr tzp;

              const TS::TimeSeries& ts_epoch = obsengine_result->at(initial_bs_param.size());
              for (int row_num : it1.second.row_index_vect)
              {
                static const long ref_jd = Fmi::Date(1970, 1, 1).julian_day();

                sv.setPrecision(5);
                // Use first row instead of row_num for static parameter values
                // SHOULD FIX Delfoi instead!
                const std::string latitude = boost::apply_visitor(sv, ts_lat[row_1].value);
                const std::string longitude = boost::apply_visitor(sv, ts_lon[row_1].value);
                const std::string geoid = boost::apply_visitor(sv, ts_geoid[row_1].value);

                if (first)
                {
                  tzp = get_tz_for_site(std::stod(longitude), std::stod(latitude), tz_name);
                }

                CTPP::CDT obs_rec;
                set_2D_coord(transformation, latitude, longitude, obs_rec);

                if (show_height)
                {
                  sv.setPrecision(1);
                  const std::string height = boost::apply_visitor(sv, ts_height[row_num].value);
                  obs_rec["height"] = (height.empty() ? query_params.missingtext : height);
                }

                const auto ldt = ts_epoch.at(row_num).time;
                Fmi::DateTime epoch = ldt.utc_time();
                // Cannot use pt::time_difference::total_seconds() directly as it returns long and
                // could overflow.
                long long jd = epoch.date().julian_day();
                long seconds = epoch.time_of_day().total_seconds();
                INT_64 s_epoch = 86400LL * (jd - ref_jd) + seconds;
                obs_rec["epochTime"] = s_epoch;
                obs_rec["epochTimeStr"] = format_local_time(epoch, tzp);

                for (std::size_t k = 0; k < param_index.size(); k++)
                {
                  const auto& entry = param_index[k];
                  const std::string& name = entry.p.name;
                  const int ind = entry.p.ind;
                  if (ind >= 0)
                  {
                    const TS::TimeSeries& ts_k = obsengine_result->at(ind);
                    const uint precision = get_meteo_parameter_options(name)->precision;
                    sv.setPrecision(precision);
                    const std::string value = boost::apply_visitor(sv, ts_k[row_num].value);
                    obs_rec["data"][k]["value"] = value;
                    if (entry.qc)
                    {
                      const int qc_ind = entry.qc->ind;
                      std::stringstream qc_value;
                      const TS::TimeSeries& ts_qc_k = obsengine_result->at(qc_ind);
                      sv.setPrecision(0);
                      const std::string value_qc = boost::apply_visitor(sv, ts_qc_k[row_num].value);
                      obs_rec["data"][k]["qcValue"] = value_qc;
                    }
                  }
                  else
                  {
                    auto geoLoc = sites.at(geoid);
                    if (geoLoc)
                    {
                      if (SmartMet::TimeSeries::is_location_parameter(name))
                      {
                        const std::string val = SmartMet::TimeSeries::location_parameter(
                            geoLoc,
                            name,
                            fmt,
                            tz_name,
                            get_meteo_parameter_options(name)->precision);
                        obs_rec["data"][k]["value"] = val;
                      }
                      else if (SmartMet::TimeSeries::is_time_parameter(name))
                      {
                        const std::string timestring = "Not supported";
                        if (not curr_locale)
                        {
                          curr_locale.reset(new std::locale(query_params.localename.c_str()));
                        }
                        const auto val =
                            SmartMet::TimeSeries::time_parameter(name,
                                                                 ldt,
                                                                 now,
                                                                 *geoLoc,
                                                                 tz_name,
                                                                 geo_engine->getTimeZones(),
                                                                 *curr_locale,
                                                                 *tfmt,
                                                                 timestring);
                        std::ostringstream val_str;
                        val_str << val;
                        obs_rec["data"][k]["value"] = val_str.str();
                      }
                      else
                      {
                        assert(0 /* Not supposed to be here */);
                      }
                    }
                    else
                    {
                      obs_rec["data"][k]["value"] = query_params.missingtext;
                    }
                  }
                }

                group["obsReturnArray"][ind++] = obs_rec;
              }
            }
          }
        }
      }

      format_output(hash, output, query.get_use_debug_format());
    }
    catch (...)
    {
      Fmi::Exception exception(BCP, "Operation processing failed!", nullptr);
      if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      exception.addParameter(WFS_LANGUAGE, curr_lang);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void StoredObsQueryHandler::check_parameter_names(const RequestParameterMap& params,
                                                  const std::vector<std::string>& param_names) const
{
  std::set<std::string> lowerCaseParamNames;
  const bool support_quality_info = SupportsQualityParameters::supportQualityInfo(params);
  for (const std::string& name : param_names)
  {
    const std::string l_name = Fmi::ascii_tolower_copy(name);
    if (not lowerCaseParamNames.insert(l_name).second)
    {
      throw Fmi::Exception::Trace(BCP, "Duplicate parameter name '" + name + "'!")
          .addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
    }

    // FIXME: handle possible conflict case when QC paremeter explicit specification is allowed
    //        and sama QC parameter may also be added automatically

    if ((not m_support_qc_parameters or not support_quality_info) and
        SupportsQualityParameters::isQCParameter(name))
    {
      throw Fmi::Exception::Trace(BCP, "Invalid parameter!")
          .addDetail("Quality code parameter '" + name + "' is not allowed in this query.")
          .addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
    }
  }
}

int StoredObsQueryHandler::lookup_initial_parameter(const std::string& name) const
{
  for (std::size_t i = 0; i < initial_bs_param.size(); i++)
  {
    if (name == initial_bs_param[i].name())
    {
      return i;
    }
  }
  return -1;
}

bool StoredObsQueryHandler::add_parameters(const RequestParameterMap& params,
                                           const std::vector<std::string>& param_names,
                                           SmartMet::Engine::Observation::Settings& query_params,
                                           std::vector<ExtParamIndexEntry>& parameter_index) const
{
  using SmartMet::TimeSeries::is_location_parameter;
  using SmartMet::TimeSeries::is_special_parameter;
  using SmartMet::TimeSeries::is_time_parameter;
  using SmartMet::TimeSeries::makeParameter;

  try
  {
    bool have_meteo_param = false;
    std::set<std::string> lowerCaseParamNames;
    const bool support_quality_info = SupportsQualityParameters::supportQualityInfo(params);

    auto qc_param_name_ref = SupportsQualityParameters::firstQCParameter(param_names);
    const bool have_explicit_qc_params = qc_param_name_ref != param_names.end();
    if ((not m_support_qc_parameters or not support_quality_info) and have_explicit_qc_params)
    {
      throw Fmi::Exception::Trace(BCP, "Invalid parameter!")
          .addDetail("Quality code parameter '" + *qc_param_name_ref +
                     "' is not allowed in this query.")
          .addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
    }

    for (const auto& name : param_names)
    {
      const std::string l_name = Fmi::ascii_tolower_copy(name);

      // Check for duplicates
      if (not lowerCaseParamNames.insert(l_name).second)
      {
        throw Fmi::Exception::Trace(BCP, "Duplicate parameter name '" + name + "'!")
            .addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
      }

      if (obs_engine->isParameter(name, query_params.stationtype))
      {
        // Normal parameter
        ExtParamIndexEntry entry;
        entry.p.ind = query_params.parameters.size();
        entry.p.name = name;
        SmartMet::Spine::Parameter param = makeParameter(name);
        bool sensor_parameter_exists = false;
        if (not SmartMet::TimeSeries::special(param))
        {
          have_meteo_param = true;
          auto parameter_option = get_meteo_parameter_options(name);
          if ((parameter_option->sensor_first != 1) || (parameter_option->sensor_last != 1))
          {
            for (unsigned short index = parameter_option->sensor_first;
                 index < parameter_option->sensor_last + 1;
                 index += parameter_option->sensor_step)
            {
              if (index < parameter_option->sensor_last + 1)
              {
                param = makeParameter(name);
                param.setSensorNumber(index);
                entry.p.ind = query_params.parameters.size();
                entry.p.sensor_name = (entry.p.name + "(:" + Fmi::to_string(index) + ")");
                query_params.parameters.push_back(param);
                if (not have_explicit_qc_params and support_quality_info)
                {
                  const std::string qc_name = "qc_" + name;
                  ParamIndexEntry qc_entry;
                  qc_entry.ind = query_params.parameters.size();
                  qc_entry.name = qc_name;
                  entry.qc = qc_entry;
                  SmartMet::Spine::Parameter param = makeParameter(qc_name);
                  query_params.parameters.push_back(param);
                }

                sensor_parameter_exists = true;
                parameter_index.push_back(entry);
              }
            }
          }
        }

        // If there is no sensors defined, add default parameter
        if (!sensor_parameter_exists)
        {
          query_params.parameters.push_back(param);

          if (not have_explicit_qc_params and support_quality_info)
          {
            const std::string qc_name = "qc_" + name;
            ParamIndexEntry qc_entry;
            qc_entry.ind = query_params.parameters.size();
            qc_entry.name = qc_name;
            entry.qc = qc_entry;
            SmartMet::Spine::Parameter param = makeParameter(qc_name);
            query_params.parameters.push_back(param);
          }

          parameter_index.push_back(entry);
        }
      }
      else if (is_special_parameter(name))
      {
        int ind = lookup_initial_parameter(l_name);
        if (ind >= 0)
        {
          ExtParamIndexEntry entry;
          entry.p.ind = ind;
          entry.p.name = name;
          parameter_index.push_back(entry);
        }
        else
        {
          if (is_location_parameter(name) or is_time_parameter(name))
          {
            ExtParamIndexEntry entry;
            entry.p.ind = -1;
            entry.p.name = name;
            parameter_index.push_back(entry);
          }
          else
          {
            ExtParamIndexEntry entry;
            entry.p.ind = query_params.parameters.size();
            entry.p.name = name;
            SmartMet::Spine::Parameter param = makeParameter(name);
            query_params.parameters.push_back(param);
            parameter_index.push_back(entry);
          }
        }
      }
      else
      {
        throw Fmi::Exception::Trace(BCP, "Unrecognized parameter '" + name + "'")
            .disableStackTrace();
      }
    }

    return have_meteo_param;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation processing failed!");
  }
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

namespace
{
using namespace SmartMet::Plugin::WFS;

boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> wfs_obs_handler_create(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    boost::optional<std::string> template_file_name)
{
  try
  {
    auto* qh = new StoredObsQueryHandler(reactor, config, plugin_data, template_file_name);
    boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> result(qh);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_obs_handler_factory(
    &wfs_obs_handler_create);

/**

   @page WFS_SQ_GENERIC_OBS Stored query handler for querying observation data

   @section WFS_SQ_GENERIC_OBS_INTRO Introduction

   Stored query handler for accessing observation data provides access to weather
   observations (except lightning observations)

   <table border="1">
   <tr>
   <td>Implementation</td>
   <td>SmartMet::Plugin::WFS::StoredObsQueryHandler</td>
   </tr>
   <tr>
   <td>Constructor name (for stored query configuration)</td>
   <td>@b wfs_obs_handler_factory</td>
   </tr>
   </table>

   @section WFS_SQ_GENERIC_OBS_PARAM Stored query handler built-in parameters

   The following stored query handler parameter groups are being used by this stored query handler:
   - @ref WFS_SQ_LOCATION_PARAMS
   - @ref WFS_SQ_PARAM_BBOX
   - @ref WFS_SQ_TIME_ZONE
   - @ref WFS_SQ_QUALITY_PARAMS
   - @ref WFS_SQ_METEO_PARAM_OPTIONS

   Additionally to parameters from these groups the following parameters are also in use

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
   <td>Specifies the meteo parameters to query. At least one parameter must be specified</td>
   </tr>

   <tr>
   <td>stationType</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>string</td>
   <td>Specifies station type.</td>
   </tr>

   <tr>
   <td>timeStep</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>unsigned integer</td>
   <td>Specifies time stamp for querying observations</td>
   </tr>

   <tr>
   <td>wmos</td>
   <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
   <td>integer</td>
   <td>Specifies stations WMO codes</td>
   </tr>

   <tr>
   <td>lpnns</td>
   <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
   <td>integer</td>
   <td>Specifies stations LPPN codes</td>
   </tr>

   <tr>
   <td>fmisids</td>
   <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
   <td>integer</td>
   <td>Specifies stations FMISID codes</td>
   </tr>

   <tr>
   <td>numOfStations</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>unsigned integer</td>
   <td>Specifies how many nearest stations to select with distance less than @b maxDistance
   (see @ref WFS_SQ_LOCATION_PARAMS "location parameters" for details)</td>
   </tr>

   <tr>
   <td>hours</td>
   <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
   <td>integer</td>
   <td>Specifies hours for which to return data if non empty array provided</td>
   </tr>

   <tr>
   <td>weekDays</td>
   <td>@ref WFS_CFG_ARRAY_PARAM_TMPL</td>
   <td>integer</td>
   <td>Week days for which to return data if non empty array is provided</td>
   </tr>

   <tr>
   <td>locale</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>string</td>
   <td>The locale code (like 'fi_FI.utf8')</td>
   </tr>

   <tr>
   <td>missingText</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>string</td>
   <td>Text to use to indicate missing value</td>
   </tr>

   <tr>
   <td>maxEpochs</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>unsigned integer</td>
   <td>Maximal number of epochs that can be returned. If the estimated number
   before query is larger than the specified one, query is aborted with
   and processing error. This parameter is not valid, if the *optional*
   storedQueryRestrictions parameter is set to false in WFS Plugin
   configurations.</td>
   </tr>

   <tr>
   <td>CRS</td>
   <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
   <td>string</td>
   <td>Specifies coordinate projection to use</td>
   </tr>

   </table>

   @section WFS_SQ_GENERIC_OBS_EXTRA_CFG_PARAM Additional parameters

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
   <td>maxHours</td>
   <td>double</td>
   <td>optional</td>
   <td>Specifies maximal permitted time interval in hours. The default value
   is 168 hours (7 days). This parameter is not valid, if the *optional*
   storedQueryRestrictions parameter is set to false in WFS Plugin
   configurations.</td>
   </tr>

   <tr>
   <td>maxStationCount</td>
   <td>unsigned integer</td>
   <td>optional</td>
   <td>Specifies maximal permitted number of stations. This count is currently checked for bounding
   boxes only.
      The ProcessingError exception report is returned if the actual count exceeds the specified
one.
   The default value 0 means unlimited. The default value is also used,
   if the *optional* storedQueryRestrictions parameter is set to false in
   WFS Plugin configurations.</td>
   </tr>

   <tr>
   <td>separateGroups</td>
   <td>boolean</td>
   <td>optional (default @b false)</td>
   <td>Providing value @b true tells handler to split returned observations into separate groups
   by station (separate group for each station)</td>
   </tr>

   <tr>
   <td>supportQCParameters</td>
   <td>boolean</td>
   <td>optional (default @b false)</td>
   <td>Providing value @b true tells handler to allow meteoParameters with "qc_" prefix to
   query.</td>
   </tr>

   </table>

*/
