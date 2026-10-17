#pragma once

#include "StoredGridForecastQueryHandler.h"
#include "StoredForecastQueryHandler.h"
#include "RequiresGridEngine.h"
#include "RequiresGeoEngine.h"
#include <grid-files/common/AdditionalParameters.h>
#include <engines/grid/Engine.h>
#include <engines/gis/GeometryStorage.h>
#include <newbase/NFmiSvgTools.h>
#include <newbase/NFmiSvgPath.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

using Query = StoredForecastQueryHandler::Query;
using ModelParameters = std::map<std::string, Engine::Querydata::ModelParameter>;
using Table_sptr = std::shared_ptr<Spine::Table>;



class StoredGridForecastQueryHandler: public StoredQueryHandlerBase,
                                      protected virtual RequiresGridEngine,
                                      protected virtual RequiresGeoEngine,
                                      protected SupportsLocationParameters,
                                      protected SupportsTimeParameters,
                                      protected SupportsTimeZone
{

  public:

                StoredGridForecastQueryHandler(
                    Spine::Reactor* reactor,
                    const std::shared_ptr<StoredQueryConfig>& config,
                    PluginImpl& plugin_impl,
                    const std::optional<std::string>& template_file_name);

        ~StoredGridForecastQueryHandler() override;

    void        init_handler() override;
    void        query(const StoredQuery& query, const std::string& language, const std::optional<std::string>& hostname, std::ostream& output) const override;

  std::string get_handler_description() const override;

  std::string get_cache_key_qualifier(const RequestParameterMap& params) const override;

  protected:


    void        parse_models(const RequestParameterMap& params, Query& dest) const;
    void        parse_level_heights(const RequestParameterMap& param, Query& dest) const;
    void        parse_levels(const RequestParameterMap& param, Query& dest) const;
    void        parse_times(const RequestParameterMap& param, Query& dest) const;
    void        parse_params(const RequestParameterMap& param, Query& dest) const;

    Table_sptr  extract_forecast(Query& query) const;

    uint        processGridQuery(
                    Query& wfsQuery,
                    const std::string& tag,
                    const Spine::LocationPtr& loc,
                    const std::string& country,
                    QueryServer::Query& gridQuery,
                    const Table_sptr& output,
                    uint rowCount) const;

  private:
    using TransformationPtr = std::shared_ptr<Spine::CRSRegistry::Transformation>;

    // The common data for the special parameter values of a grid query
    struct SpecialValueContext
    {
      Query& wfsQuery;
      const std::string& tag;
      const Spine::LocationPtr& loc;
      const std::string& country;
      QueryServer::Query& gridQuery;
      const Table_sptr& output;
      AdditionalParameters& additionalParameters;
      const Fmi::TimeZonePtr& tz;
      int pLen;
    };

    void        set_special_values(
                    const SpecialValueContext& ctx,
                    int p,
                    uint rLen,
                    uint col,
                    uint rows,
                    uint& lastRow) const;
    bool        find_producer_name(
                    QueryServer::Query& gridQuery,
                    int pLen,
                    uint t,
                    std::string& name) const;
    std::string default_producer_name(QueryServer::Query& gridQuery) const;
    std::string producer_list(const Query& wfsQuery) const;
    std::string grid_param_list(const Query& wfsQuery) const;
    bool        find_origin_time(const SpecialValueContext& ctx, uint t, std::string& value) const;
    std::string multi_match_value(
                    const QueryServer::QueryParameter& param,
                    uint rLen,
                    time_t forecastTime) const;
    void        set_origin_time_and_area(
                    Query& wfsQuery,
                    T::GenerationId generationId,
                    uint geometryId) const;

    void        set_group_info(CTPP::CDT& group, const Query& query, FeatureID& feature_id) const;
    void        set_group_data(
                    CTPP::CDT& group,
                    const Query& query,
                    const std::pair<std::multimap<int, std::string>::const_iterator,
                                    std::multimap<int, std::string>::const_iterator>& site_range,
                    const std::multimap<std::string, std::size_t>& result_map,
                    const TransformationPtr& transformation,
                    bool show_height) const;
    Fmi::TimeZonePtr set_station_info(
                    CTPP::CDT& station,
                    const Query& query,
                    std::size_t i,
                    const std::string& geo_id,
                    const TransformationPtr& transformation,
                    bool show_height) const;
    Fmi::DateTime set_row_data(
                    CTPP::CDT& row_data,
                    const Query& query,
                    std::size_t i,
                    const TransformationPtr& transformation,
                    bool show_height,
                    const Fmi::TimeZonePtr& tzp) const;

    Engine::Gis::GeometryStorage  itsGeometryStorage;
    Fmi::TimeZones                itsTimezones;
    std::vector<Spine::Parameter> common_params;
    double                        max_np_distance;
    bool                          separate_groups;

    std::size_t                   ind_geoid;
    std::size_t                   ind_epoch;
    std::size_t                   ind_place;
    std::size_t                   ind_lat;
    std::size_t                   ind_lon;
    std::size_t                   ind_elev;
    std::size_t                   ind_level;
    std::size_t                   ind_region;
    std::size_t                   ind_country;
    std::size_t                   ind_country_iso;
    std::size_t                   ind_localtz;

    static const char*            P_MODEL;
    static const char*            P_ORIGIN_TIME;
    static const char*            P_LEVEL_HEIGHTS;
    static const char*            P_LEVEL;
    static const char*            P_LEVEL_TYPE;
    static const char*            P_PARAM;
    static const char*            P_FIND_NEAREST_VALID;
    static const char*            P_LOCALE;
    static const char*            P_MISSING_TEXT;
    static const char*            P_CRS;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
