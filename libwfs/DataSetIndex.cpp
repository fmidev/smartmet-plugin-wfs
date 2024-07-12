#include "DataSetIndex.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <macgyver/TimeParser.h>
#include <macgyver/TypeName.h>
#include <spine/Convenience.h>
#include <macgyver/Exception.h>
#include <stdexcept>
#include <vector>

namespace ba = boost::algorithm;
namespace bw = SmartMet::Plugin::WFS;
namespace fs = std::filesystem;

namespace
{
template <typename T>
std::set<T> vect2set(const std::vector<T>& src)
{
  try
  {
    std::set<T> result;
    for (const auto& item : src)
    {
      if (not result.insert(item).second)
      {
        std::ostringstream msg;
        msg << "Duplicate item '" << item << "' in value list";
        throw Fmi::Exception(BCP, msg.str());
      }
    }
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

bw::DataSetQuery::DataSetQuery()
    : period(Fmi::DateTime(Fmi::DateTime::NEG_INFINITY), Fmi::DateTime(Fmi::DateTime::POS_INFINITY))
{
}

bw::DataSetQuery::~DataSetQuery() = default;

void bw::DataSetQuery::add_name(const std::string& name)
{
  try
  {
    if (not names.insert(Fmi::ascii_tolower_copy(name)).second)
    {
      throw Fmi::Exception(BCP, "Duplicate name '" + name + "'!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::DataSetQuery::add_level(int level)
{
  try
  {
    if (not levels.insert(level).second)
    {
      throw Fmi::Exception(BCP, "Duplicate level '" + std::to_string(level) + "'!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::DataSetQuery::add_parameter(const std::string& parameter)
{
  try
  {
    if (not parameters.insert(Fmi::ascii_tolower_copy(parameter)).second)
    {
      throw Fmi::Exception(BCP, "Duplicate parameter '" + parameter + "'!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::DataSetQuery::set_interval(const Fmi::DateTime& begin,
                                    const Fmi::DateTime& end)
{
  try
  {
    if (begin.is_not_a_date_time() or end.is_not_a_date_time() or end < begin)
    {
      Fmi::Exception exception(BCP, "Invalid time interval!");
      exception.addParameter("Start time", Fmi::date_time::to_simple_string(begin));
      exception.addParameter("End time", Fmi::date_time::to_simple_string(end));
      throw exception;
    }

    period = Fmi::TimePeriod(begin, end);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::DataSetDefinition::DataSetDefinition(SmartMet::Spine::ConfigBase& config,
                                         libconfig::Setting& setting)
{
  try
  {
    try
    {
      name = config.get_mandatory_config_param<std::string>(setting, "name");
      dir = fs::path(config.get_mandatory_config_param<std::string>(setting, "dir"));
      server_dir = config.get_optional_config_param<std::string>(setting, "serverDir", "");
      file_regex.assign(config.get_mandatory_config_param<std::string>(setting, "file_regex"));
      origin_time_extract.assign(
          config.get_mandatory_config_param<std::string>(setting, "origin_time_extract"));

      if ((!server_dir.empty()) and (*server_dir.rbegin() != '/'))
        server_dir += "/";

      std::string s_time_extract(R"(^(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2}))");
      std::string s_time_subst(R"(\1\2\3T\4\5\6Z)");
      std::vector<std::string> v_translate;
      if (config.get_config_array(setting, "origin_time_translate", v_translate, 2, 2))
      {
        s_time_extract = v_translate[0];
        s_time_subst = v_translate[1];
      }

      origin_time_match.assign(s_time_extract);
      origin_time_replace = s_time_subst;

      config.get_mandatory_config_param<libconfig::Setting&>(setting, "params");
      std::vector<std::string> v_params;
      config.get_config_array(setting, "params", v_params, 1);
      params = vect2set(v_params);

      config.get_mandatory_config_param<libconfig::Setting&>(setting, "levels");
      std::vector<int> v_levels;
      config.get_config_array(setting, "levels", v_levels, 1);
      levels = vect2set(v_levels);

      std::vector<double> tmp;
      config.get_config_array<double>(setting, "bbox", tmp, 4, 4);
      bbox = box_t(point_t(tmp.at(0), tmp.at(1)), point_t(tmp.at(2), tmp.at(3)));

      if (not fs::exists(dir))
      {
        throw Fmi::Exception(BCP, "Directory '" + dir.string() + "' not found!");
      }

      if (not fs::is_directory(dir))
      {
        throw Fmi::Exception(BCP, "Not a directory: '" + dir.string() + "'!");
      }
    }
    catch (...)
    {
      Fmi::Exception exception(BCP, "Error while processing libconfig::Setting!", nullptr);

      std::ostringstream msg;
      msg << exception.getStackTrace();
      SmartMet::Spine::ConfigBase::dump_setting(msg, setting, 8);

      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<bw::DataSetDefinition> bw::DataSetDefinition::create(
    SmartMet::Spine::ConfigBase& config, libconfig::Setting& setting)
{
  try
  {
    return std::shared_ptr<bw::DataSetDefinition>(new bw::DataSetDefinition(config, setting));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::DataSetDefinition::~DataSetDefinition() = default;

bool bw::DataSetDefinition::intersects(const bw::DataSetDefinition::box_t& bbox) const
{
  try
  {
    return boost::geometry::intersects(bbox, this->bbox);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::filesystem::path> bw::DataSetDefinition::query_files(
    const Fmi::DateTime& begin, const Fmi::DateTime& end) const
{
  try
  {
    std::vector<std::filesystem::path> result;
    for (auto it = fs::directory_iterator(dir); it != fs::directory_iterator(); ++it)
    {
      fs::path entry = *it;
      const std::string fn = entry.filename().string();
      boost::match_results<std::string::const_iterator> what;
      if (boost::regex_match(fn, what, file_regex))
      {
        try
        {
          Fmi::DateTime origintime = extract_origintime(fn);
          if (begin <= origintime and origintime <= end)
          {
            result.push_back(entry);
          }
        }
        catch (...)
        {
          Fmi::Exception exception(
              BCP, "Failed to extract origin time from the file name!", nullptr);
          exception.addDetail("File ignored.");
          exception.addParameter("File name", entry.string());
          std::cout << exception.getStackTrace();
        }
      }
    }
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::DateTime bw::DataSetDefinition::extract_origintime(const std::filesystem::path& p) const
{
  try
  {
    std::string s_t0;
    boost::match_results<std::string::const_iterator> what;
    const std::string fn = p.filename().string();
    if (boost::regex_search(fn, what, origin_time_extract, boost::match_default))
    {
      s_t0.assign(what[0].first, what[0].second);
    }
    else
    {
      throw Fmi::Exception(BCP, "Failed to extract time string from '" + fn + "'!");
    }

    std::string s_tm =
        boost::regex_replace(s_t0, origin_time_match, origin_time_replace, boost::match_default);
    return Fmi::TimeParser::parse(s_tm);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/**

@page WFS_CFG_FD_DATA_SET Data set specification for file download handler

This configuration data type describes file set available for download

<table border="1">

<tr>
<th>Entry name</th>
<th>Type</th>
<th>Use</th>
<th>Description</th>
</tr>

<tr>
  <td>name</td>
  <td>string</td>
  <td>mandatory</td>
  <td>The name of file set</td>
</tr>

<tr>
  <td>dir</td>
  <td>string</td>
  <td>mandatory</td>
  <td>The directory where files are located. '/' is appended to string if needed</td>
</tr>

<tr>
  <td>serverDir</td>
  <td>string</td>
  <td>optional (default "")</td>
  <td>Specifies directory for server (URL directory part)</td>
</tr>

<tr>
  <td>fileRegex</td>
  <td>string</td>
  <td>mandatory</td>
  <td>regular expressions for filtering file names (only base name)</td>
</tr>

<tr>
  <td>origin_time_extract</td>
  <td>string</td>
  <td>mandatory</td>
  <td>Specifies REGEX to extract origin time string</td>
</tr>

<tr>
  <td>origin_time_translate</td>
  <td>array of strings of size 2</td>
  <td>optional</td>
  <td>First element of array specifies how to extract parts of time from origin time string,
      the second one specifies how to translate origin tim eto ISO extended time
      format. The default value is:
      @verbatim
      ["^(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})", "\\1\\2\\3T\\4\\5\\6T"]
      @endverbatim
      </td>
</tr>

<tr>
  <td>params</td>
  <td>array of strig</td>
  <td>mandatory</td>
  <td>The parameters included in the files</td>
</tr>

<tr>
  <td>levels</td>
  <td>array of integer</td>
  <td>mandatory</td>
  <td>Levels included in the files</td>
</tr>

<tr>
  <td>bbox</td>
  <td>array of double with size 4</td>
  <td>optional</td>
  <td>Bounding box for files</td>
</tr>

</table>

<hr>

Example

@verbatim
{
        name: "hirlam_eurooppa";
        dir: "/smartmet/data/hirlam/eurooppa/pinta/querydata";
        serverDir: "hirlam/eurooppa/pinta/querydata";
        file_regex: ".*";
        origin_time_extract: "^\\d{12}_";
        origin_time_translate: [ "^(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})_",
"\\1-\\2-\\3T\\4:\\5Z"];
        levels: [0, 1, 151, 167];
        bbox: [-15.0, 0.0, 60.0, 75.0];
        params: ["Temperature", "SnowDepth", "Pressure"];
}
@endverbatim

 */
