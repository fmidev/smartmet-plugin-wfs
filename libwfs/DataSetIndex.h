#pragma once

#include <macgyver/DateTime.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <spine/ConfigBase.h>
#include <set>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class DataSetQuery
{
 public:
  DataSetQuery();

  virtual ~DataSetQuery();

  void add_name(const std::string& name);

  void add_level(int level);

  void add_parameter(const std::string& parameter);

  void set_interval(const Fmi::DateTime& begin, const Fmi::DateTime& end);

  inline const std::set<std::string>& get_names() const { return names; }
  inline const boost::posix_time::time_period& get_time_interval() const { return period; }
  inline const std::set<std::string>& get_parameters() const { return parameters; }
  inline const std::set<int>& get_levels() const { return levels; }

 private:
  std::set<std::string> names;
  boost::posix_time::time_period period;
  std::set<std::string> parameters;
  std::set<int> levels;
};

class DataSetDefinition : public boost::enable_shared_from_this<DataSetDefinition>
{
 public:
  using point_t = boost::geometry::model::d2::point_xy<double>;
  using box_t = boost::geometry::model::box<point_t>;

 private:
  DataSetDefinition(SmartMet::Spine::ConfigBase& config, libconfig::Setting& setting);

 public:
  static boost::shared_ptr<DataSetDefinition> create(SmartMet::Spine::ConfigBase& config,
                                                     libconfig::Setting& setting);

  virtual ~DataSetDefinition();

  inline const std::string& get_name() const { return name; }
  inline const boost::filesystem::path& get_dir() const { return dir; }
  inline const std::string& get_server_dir() const { return server_dir; }
  inline const std::set<std::string>& get_params() const { return params; }
  inline const std::set<int>& get_levels() const { return levels; }
  inline const box_t& get_bbox() const { return bbox; }
  bool intersects(const box_t& bbox) const;

  std::vector<boost::filesystem::path> query_files(
      const Fmi::DateTime& begin = boost::date_time::neg_infin,
      const Fmi::DateTime& end = boost::date_time::pos_infin) const;

  Fmi::DateTime extract_origintime(const boost::filesystem::path& p) const;

 private:
  std::string name;
  boost::filesystem::path dir;
  std::string server_dir;
  boost::basic_regex<char> file_regex;
  std::set<std::string> params;
  std::set<int> levels;
  box_t bbox;
  boost::basic_regex<char> origin_time_extract;
  boost::basic_regex<char> origin_time_match;
  std::string origin_time_replace;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
