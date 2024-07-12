#include "ArrayParameterTemplate.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <typeinfo>
#include <boost/bind/bind.hpp>
#include <fmt/format.h>

namespace ph = boost::placeholders;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
ArrayParameterTemplate::ArrayParameterTemplate(StoredQueryConfig& config,
                                               const std::string& config_path,
                                               std::size_t min_size,
                                               std::size_t max_size,
					       bool silent)
    : ParameterTemplateBase(config, HANDLER_PARAM_NAME, config_path),
      min_size(min_size),
      max_size(max_size)
{
  try
  {
    try
    {
      init(silent);
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

ArrayParameterTemplate::ArrayParameterTemplate(StoredQueryConfig& config,
                                               const std::string& base_path,
                                               const std::string& config_path,
                                               std::size_t min_size,
                                               std::size_t max_size,
					       bool silent)
    : ParameterTemplateBase(config, base_path, config_path), min_size(min_size), max_size(max_size)
{
  try
  {
    try
    {
      init(silent);
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<SmartMet::Spine::Value> ArrayParameterTemplate::get_value(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    std::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> > result;
    get_value(result, req_param_map, extra_params, true);
    return std::get<std::vector<SmartMet::Spine::Value> >(result);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

boost::tribool ArrayParameterTemplate::get_value(
    std::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> >& result,
    const RequestParameterMap& req_param_map,
    const SupportsExtraHandlerParams* extra_params,
    bool strict) const
{
  try
  {
    bool some_resolved = false;
    bool all_resolved = true;
    // bool check_lower_limit = true;
    std::vector<SmartMet::Spine::Value> tmp_result;

    for (const auto& item : items)
    {
      bool found = true;
      std::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> > item_value;
      if (strict)
      {
        item_value = item.get_value(req_param_map, extra_params, true);
      }
      else
      {
        found = item.get_value(item_value, req_param_map, extra_params, true);
      }

      if (found)
      {
        if (item_value.index() == 0)
        {
          tmp_result.push_back(std::get<SmartMet::Spine::Value>(item_value));
        }
        else if (item_value.index() == 1)
        {
          auto& tmp = std::get<std::vector<SmartMet::Spine::Value> >(item_value);
          std::copy(tmp.begin(), tmp.end(), std::back_inserter(tmp_result));
        }
        else
        {
          // Not supposed to happen
          assert(0);
        }
        some_resolved = true;
      }
      else
      {
        all_resolved = false;
      }
    }

    if (tmp_result.size() > max_size)
    {
      const std::string msg = fmt::format("Result array size {} exceeds upper limit {} of array size",
                                          tmp_result.size(), max_size);
      Fmi::Exception exception(BCP, msg);
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }

    if (tmp_result.size() < min_size)
    {
      const std::string msg = fmt::format("Result array size {} is smaller than lower limit {} of array size",
                                          tmp_result.size(), min_size);
      Fmi::Exception exception(BCP, msg);
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }

    result = tmp_result;

    if (all_resolved)
    {
      return true;
    }
    else if (some_resolved)
    {
      return boost::indeterminate;
    }
    else
    {
      return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<int64_t> ArrayParameterTemplate::get_int_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<int64_t> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_int,  ph::_1));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<uint64_t> ArrayParameterTemplate::get_uint_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<uint64_t> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_uint,  ph::_1));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<double> ArrayParameterTemplate::get_double_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<double> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_double,  ph::_1));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::string> ArrayParameterTemplate::get_string_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<std::string> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_string,  ph::_1));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<Fmi::DateTime> ArrayParameterTemplate::get_ptime_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<Fmi::DateTime> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_ptime,  ph::_1, true));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<SmartMet::Spine::Point> ArrayParameterTemplate::get_point_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<SmartMet::Spine::Point> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_point,  ph::_1));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<SmartMet::Spine::BoundingBox> ArrayParameterTemplate::get_bbox_array(
    const RequestParameterMap& req_param_map, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    try
    {
      std::vector<SmartMet::Spine::Value> tmp = get_value(req_param_map, extra_params);
      std::vector<SmartMet::Spine::BoundingBox> result;
      std::transform(tmp.begin(),
                     tmp.end(),
                     std::back_inserter(result),
                     boost::bind(&SmartMet::Spine::Value::get_bbox,  ph::_1));
      return result;
    }
    catch (...)
    {
      handle_exceptions(METHOD_NAME);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void ArrayParameterTemplate::init(bool silent)
{
  try
  {
    std::string base_prefix = get_base_path();
    if (!base_prefix.empty())
      base_prefix += ".";

    libconfig::Setting* setting_root = get_setting_root();

    if (not setting_root) {
        if (min_size > 0) {
	  // Definition of array not provided and zero size array is NOT permitted:
	  // -> interpret as an configuration error
	  std::ostringstream msg;
	  msg << "Definition of array parameter '" << get_config_path()
	      << "' with minimal size " << min_size
	      << " not provided in stored query configuration '" << get_config().get_file_name() << "'";
            throw Fmi::Exception::Trace(BCP, msg.str());
        } else {
	  // Definition of array not provided and zero size array is permitted:
	  // -> interpret as empty array and optionally provide a warning
	    if (not silent) {
                std::cout << "WARNING: Definition of array parameter '" << get_config_path()
			  << "' with minimal size 0 is not provided in stored query configuration '"
			  << get_config().get_file_name() << "'" << std::endl;
	    }
	    return;
        }
    }

    std::vector<libconfig::Setting*> setting_array;

    if (setting_root->isArray()) {
        if ((setting_root->getLength() == 0) and (min_size > 0))
        {
            const std::string msg = fmt::format("The length {} of configuration parameter '{}' is out of allowed range {}..{}",
                                                 setting_root->getLength(), base_prefix + get_config_path(), min_size, max_size);
            throw Fmi::Exception(BCP, msg);
        }

        for (int i = 0; i < setting_root->getLength(); i++)
        {
            libconfig::Setting* setting = &(*setting_root)[i];
            setting_array.push_back(setting);
        }
    } else {
        setting_array.push_back(setting_root);
    }

    bool have_weak_ref = false;
    std::size_t calc_min_size = 0;
    std::size_t calc_max_size = 0;
    const int len = setting_array.size();
    for (int i = 0; i < len; i++)
    {
        ParameterTemplateItem item;
        libconfig::Setting& setting = *setting_array.at(i);
        SmartMet::Spine::Value item_def = SmartMet::Spine::Value::from_config(setting);
        item.parse(item_def);

        have_weak_ref |= item.weak;

        // If the template item is a reference to WFS request parameter
        // then verify that it is correct. Copying both scalar value and
        // entire array are permitted in in this case
        if (not item.weak and item.param_ref)
        {
            const StoredQueryConfig::ParamDesc& param = get_param_desc(*item.param_ref);

            if (param.isArray())
            {
                if (item.param_ind && *item.param_ind >= param.param_def.getMaxSize())
                {
                    // The requested index is above max. possible size of parameter array
                    const std::string msg = fmt::format("The array index {} is out of the range 0..{}",
                                                        *item.param_ind, param.param_def.getMaxSize());
                    throw Fmi::Exception(BCP, msg);
                }
            }

            if (item.param_ind)
            {
                calc_min_size++;
                calc_max_size++;
            }
            else if (param.isArray() /* and not item.param_ind */)
            {
                // Minimal size check is skipped later if parameter
                // template refers to entire optional request array parameter
                calc_min_size += param.param_def.getMinSize();
                calc_max_size += param.param_def.getMaxSize();
            }
            else
            {
                calc_min_size++;
                calc_max_size++;
            }
        }
        else if (item.plain_text)
        {
            calc_min_size++;
            calc_max_size++;
        }

        items.push_back(item);
    }

    if (not have_weak_ref)
    {
        if (calc_min_size > max_size)
        {
            const std::string msg = fmt::format("The calculated minimal array size {} exceeds the upper limit {} of array parameter",
                                                calc_min_size, max_size);
            throw Fmi::Exception(BCP, msg);
        }

        if (calc_max_size < min_size)
        {
            const std::string msg = fmt::format("The calculated maximal array size {} is lower than the lower limit {} of array parameter",
                                                calc_max_size, min_size);
            throw Fmi::Exception(BCP, msg);
        }
    }
  }
catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

/**

@page WFS_CFG_ARRAY_PARAM_TMPL Stored query handler array parameter template

@section WFS_CFG_ARRAY_PARAM_TMPL_INTRO Introduction

Stored query handler array parameter template describes how to get values for
stored query handler array parameter. Support of array parameter template
is implemented in C++ class SmartMet::Plugin::WFS::ArrayParameterTemplate .

The value of configuration entry which describes stored query handler array
parameter template is an array of @ref WFS_CFG_PARAM_TMPL_ITEM "cfgParameterTemplateItem".
Note that it is not permitted to mix different types in libconfig array. As result one
may need to use strings for values as conversion from strings to aother parameter types
is supported

@section WFS_CFG_ARRAY_PARAM_TMPL_EXAMPLES Examples

Let us assumen that the following stored query array parameters are defined:
@verbatim
parameters:
(
{
        name = "parameters";
        title = { eng = "Parameters to return"; fin = "Meteorologiset parametrit"; };
        abstract = { eng = "Comma separated list of meteorological parameters to return."; fin =
"Meteorologiset paraemtrit pilkulla erotettuna.";};
        xmlType = "gml:NameList";
        type = "string[1..99]";
        minOccurs = 0;
        maxOccurs = 999;
},

{
    name = "foo";
    title = { eng = "Parameter foo"; };
    abstract = { eng = "Parameter foo"; }
    xmlType = "doubleList";
    type = "double[4]";
    minOccurs = 1;
    naxOccurs = 1;
}
);
@endverbatim

Additionally let us assume that following extra named parameter are defined:

@verbatim
handler_params = (
    {
        name = "defaultMeteoParam";
        def = ["t2m","ws_10min","wg_10min",
"wd_10min","rh","td","r_1h","ri_10min","snow_aws","p_sea","vis"];
    }
);
@endverbatim

Several examples of mappping stored query handler parameters are provided in the table below:

<table border="1">

<tr>
<th>Mapping</th>
<th>Description</th>
</tr>

<tr>
<td>
@verbatim
meteoParam = ["${parameters}"];
@endverbatim
</td>
<td>
Unconditionally map stored query handler parameter @b meteoParam to stored query parameter @b
parameters. No default values
are provided.
</td>
</tr>

<tr>
<td>
@verbatim
meteoParam = ["${parameters > defaultMeteoParam}"];
@endverbatim
</td>
<td>
Pap stored query handler parameter @b meteoParam to stored query parameter @b parameters. Use the
value from named parameter
@b defaultMeteoParam as the default
</td>
</tr>

<tr>
<td>
@verbatim
meteoParam = ["longitude", "latitude", "${parameters > defaultMeteoParam}"];
@endverbatim
</td>
<td>
Pap stored query handler parameter @b meteoParam to stored query parameter @b parameters. Use the
value from named parameter
@b defaultMeteoParam as the default. Additionally always insert meteo parameters @b longitude and @b
latitude at
the start of the array parameter
</td>
</tr>

<tr>
<td>
@verbatim
paramFoo = ["${foo[3]}", "${foo[2]}", "${foo[1]}", "${foo[0]}"];
@endverbatim
</td>
<td>
Maps stored query handler parameter @b paramFoo to stored query parameter @b foo reordering array
members.
Note that there is no implemented way to do the same with an arrya of unknwon before length
</td>
</tr>

</table>

 */
