#pragma once

#include "ArrayParameterTemplate.h"
#include "RequiresGeoEngine.h"
#include "RequiresQEngine.h"
#include "ScalarParameterTemplate.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsLocationParameters.h"
#include "SupportsMeteoParameterOptions.h"
#include "SupportsTimeParameters.h"
#include "SupportsTimeZone.h"
#include <macgyver/DateTime.h>
#include <engines/geonames/Engine.h>
#include <engines/querydata/Engine.h>
#include <engines/querydata/MetaData.h>
#include <newbase/NFmiPoint.h>
#include <timeseries/TimeSeriesInclude.h>
#include <list>
#include <utility>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredForecastQueryHandler : public StoredQueryHandlerBase,
                                   protected virtual RequiresGeoEngine,
                                   protected virtual RequiresQEngine,
                                   protected SupportsLocationParameters,
                                   protected SupportsMeteoParameterOptions,
                                   protected SupportsTimeParameters,
                                   protected SupportsTimeZone
{
 public:
  struct Query
  {
    std::list<std::string> models;
    std::set<int> levels;
    std::set<float> level_heights;
    std::string level_type;
    std::list<std::pair<std::string, SmartMet::Spine::LocationPtr> > locations;
    std::vector<SmartMet::Spine::Parameter> data_params;
    std::string keyword;
    double max_distance;
    double max_np_distance;
    std::string missing_text;
    std::string language;
    std::shared_ptr<const std::locale> output_locale;
    bool find_nearest_valid_point;
    std::string tz_name;

    std::unique_ptr<Fmi::DateTime> origin_time;
    Fmi::DateTime modification_time = Fmi::DateTime::NOT_A_DATE_TIME;

   public:
    std::shared_ptr<SmartMet::Spine::Table> result;

    std::unique_ptr<Fmi::ValueFormatter> value_formatter;
    std::unique_ptr<Fmi::TimeFormatter> time_formatter;
    std::shared_ptr<TS::TimeSeriesGeneratorOptions> toptions;

    bool have_model_area;
    NFmiPoint top_left;
    NFmiPoint top_right;
    NFmiPoint bottom_left;
    NFmiPoint bottom_right;

    std::string producer_name;
    std::string model_path;

    mutable NFmiPoint lastpoint;

    std::size_t first_data_ind;
    std::size_t last_data_ind;

   public:
    Query();
    Query(std::shared_ptr<const StoredQueryConfig> config);
    virtual ~Query();
    void set_locale(const std::string& locale_name);
    void set_value_formatter(const Fmi::ValueFormatterParam& vf_param);
  };

 public:
  StoredForecastQueryHandler(SmartMet::Spine::Reactor* reactor,
                             StoredQueryConfig::Ptr config,
                             PluginImpl& plugin_impl,
                             std::optional<std::string> template_file_name);

  ~StoredForecastQueryHandler() override;

  std::string get_handler_description() const override;

  void query(const StoredQuery& query,
                     const std::string& language,
                     const std::optional<std::string>& hostname,
                     std::ostream& output) const override;

 private:
  std::shared_ptr<SmartMet::Spine::Table> extract_forecast(Query& query) const;

  SmartMet::Engine::Querydata::Producer select_producer(const SmartMet::Spine::Location& loc,
                                                        const Query& query) const;

  void parse_models(const RequestParameterMap& params, Query& dest) const;
  void parse_level_heights(const RequestParameterMap& param, Query& dest) const;
  void parse_levels(const RequestParameterMap& param, Query& dest) const;
  void parse_times(const RequestParameterMap& param, Query& dest) const;
  void parse_params(const RequestParameterMap& param, Query& dest) const;

  std::map<std::string, SmartMet::Engine::Querydata::ModelParameter> get_model_parameters(
      const std::string& producer, const Fmi::DateTime& origin_time) const;

 private:
  std::vector<SmartMet::Spine::Parameter> common_params;
  double max_np_distance;
  bool separate_groups;

  std::size_t ind_geoid;
  std::size_t ind_epoch;
  std::size_t ind_place;
  std::size_t ind_lat;
  std::size_t ind_lon;
  std::size_t ind_elev;
  std::size_t ind_level;
  std::size_t ind_region;
  std::size_t ind_country;
  std::size_t ind_country_iso;
  std::size_t ind_localtz;

 protected:
  static const char* P_MODEL;
  static const char* P_ORIGIN_TIME;
  static const char* P_LEVEL_HEIGHTS;
  static const char* P_LEVEL;
  static const char* P_LEVEL_TYPE;
  static const char* P_PARAM;
  static const char* P_FIND_NEAREST_VALID;
  static const char* P_LOCALE;
  static const char* P_MISSING_TEXT;
  static const char* P_CRS;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
