#include "StoredQueryParamRegistry.h"
#include "ArrayParameterTemplate.h"
#include "ScalarParameterTemplate.h"
#include "SupportsExtraHandlerParams.h"
#include <macgyver/DistanceParser.h>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <sstream>
#include <boost/bind/bind.hpp>

namespace bw = SmartMet::Plugin::WFS;
namespace ph = boost::placeholders;

using bw::StoredQueryParamRegistry;
using SmartMet::Spine::Value;

namespace
{
enum TypeInd
{
  P_INT = 1,
  P_UINT,
  P_DOUBLE,
  P_STRING,
  P_TIME,
  P_POINT,
  P_BBOX,
  P_BOOL
};
}

StoredQueryParamRegistry::StoredQueryParamRegistry(StoredQueryConfig::Ptr config)
    : bw::StoredQueryConfig::Wrapper(config)
    , silence_param_init_warnings_(false)
{
  try
  {
    supported_type_names[typeid(int64_t).name()] = P_INT;
    supported_type_names[typeid(uint64_t).name()] = P_UINT;
    supported_type_names[typeid(double).name()] = P_DOUBLE;
    supported_type_names[typeid(std::string).name()] = P_STRING;
    supported_type_names[typeid(boost::posix_time::ptime).name()] = P_TIME;
    supported_type_names[typeid(SmartMet::Spine::Point).name()] = P_POINT;
    supported_type_names[typeid(SmartMet::Spine::BoundingBox).name()] = P_BBOX;
    supported_type_names[typeid(bool).name()] = P_BOOL;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

StoredQueryParamRegistry::~StoredQueryParamRegistry() = default;
StoredQueryParamRegistry::ParamRecBase::~ParamRecBase() = default;
StoredQueryParamRegistry::ScalarParameterRec::~ScalarParameterRec() = default;
StoredQueryParamRegistry::ArrayParameterRec::~ArrayParameterRec() = default;


boost::shared_ptr<bw::RequestParameterMap> StoredQueryParamRegistry::resolve_handler_parameters(
    const bw::RequestParameterMap& src, const SupportsExtraHandlerParams* extra_params) const
{
  try
  {
    bool case_sensitive_params = get_config()->use_case_sensitive_params();
    boost::shared_ptr<bw::RequestParameterMap> result(
	new bw::RequestParameterMap(case_sensitive_params));

    for (const auto& map_item : param_map)
    {
      const std::string& name = map_item.first;
      auto& item_ref = *map_item.second;
      int type_ind = supported_type_names.at(item_ref.type_name);
      if (typeid(item_ref) == typeid(ScalarParameterRec))
      {
        const ScalarParameterRec& rec = dynamic_cast<ScalarParameterRec&>(item_ref);

        SmartMet::Spine::Value value;
        if (rec.required)
        {
          value = rec.param_def->get_value(src, extra_params);
        }
        else if (not rec.param_def->get_value(value, src, extra_params))
        {
          // Optional scalar parameter ommited.
          continue;
        }

        switch (type_ind)
        {
          case P_INT:
            result->add(name, value.get_int());
            break;

          case P_UINT:
            result->add(name, value.get_uint());
            break;

          case P_DOUBLE:
  			{
			  if(Fmi::ascii_tolower_copy(name) == "maxdistance")
				{
				  try
					{
					  result->add(name, value.get_double());
					}
				  catch(...)
					{
					  std::string maxdistance = value.get_string();
					  result->add(name, Fmi::DistanceParser::parse_meter(maxdistance));
					}
				}
			  else
				{
				  result->add(name, value.get_double());
				}
			}
            break;

          case P_STRING:
            result->add(name, value.get_string());
            break;

          case P_TIME:
            result->add(name, value.get_ptime(true));
            break;

          case P_POINT:
            result->add(name, value.get_point());
            break;

          case P_BBOX:
            result->add(name, value.get_bbox());
            break;

          case P_BOOL:
            result->add(name, value.get_bool());
            break;

          default:
            // Not supposed to happen
            throw Fmi::Exception(BCP,
                                             "INTERNAL ERROR at line " + Fmi::to_string(__LINE__));
        }
      }
      else if (typeid(item_ref) == typeid(ArrayParameterRec))
      {
        const ArrayParameterRec& rec = dynamic_cast<ArrayParameterRec&>(item_ref);

        std::vector<SmartMet::Spine::Value> values = rec.param_def->get_value(src, extra_params);
        if (values.size() < rec.min_size or values.size() > rec.max_size)
        {
          std::ostringstream msg;
          msg << "Number of the values '" << values.size() << "' for the parameter '" << name
              << "' is out of the range [" << rec.min_size << ".." << rec.max_size << "]!";
          throw Fmi::Exception(BCP, msg.str());
        }
        if (rec.step > 1 and ((values.size() - rec.min_size) % rec.step != 0))
        {
          int cnt = 1 + (rec.max_size - rec.min_size) / rec.step;
          std::ostringstream msg;
          msg << "Number of the values '" << values.size() << "' for the parameter '" << name
              << "' is invalid!";
          switch (cnt)
          {
            case 1:
              msg << " (" << rec.min_size << " values expected)";
              break;

            case 2:
            case 3:
            case 4:
            case 5:
              msg << " (";
              for (int i = 0; i < cnt; i++)
                msg << (i == 0 ? "" : i == cnt - 1 ? " or " : ", ") << rec.min_size + i * rec.step;
              msg << " values expected)";
              break;

            default:
              msg << " (" << rec.min_size << ", " << rec.min_size + rec.step << ",... , "
                  << rec.min_size + (cnt - 1) * rec.step << " values expected)";
              break;
          }

          throw Fmi::Exception(BCP, msg.str());
        }

        for (const SmartMet::Spine::Value& value : values)
        {
          switch (type_ind)
          {
            case P_INT:
              result->add(name, value.get_int());
              break;

            case P_UINT:
              result->add(name, value.get_uint());
              break;

            case P_DOUBLE:
              result->add(name, value.get_double());
              break;

            case P_STRING:
              result->add(name, value.get_string());
              break;

            case P_TIME:
              result->add(name, value.get_ptime(true));
              break;

            default:
              // Not supposed to happen
              throw Fmi::Exception(
                  BCP, "INTERNAL ERROR at line " + Fmi::to_string(__LINE__));
          }
        }
      }
      else
      {
        // Not supposed to happen
        throw Fmi::Exception(BCP, "INTERNAL ERROR at line " + Fmi::to_string(__LINE__));
      }
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::set<std::string> StoredQueryParamRegistry::get_param_names() const
{
  try
  {
    std::set<std::string> result;
    std::transform(
        param_map.begin(),
        param_map.end(),
        std::inserter(result, result.begin()),
        boost::bind(&std::pair<const std::string, boost::shared_ptr<ParamRecBase> >::first, ph::_1));
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Json::Value StoredQueryParamRegistry::get_param_info() const
{
    // Explicit specifications of readable type names. For others C++ type name demangling is being used
    static std::map<std::string, std::string> name_remap =
        {
            { typeid(std::string).name(), "string" }
            , { typeid(boost::posix_time::ptime).name(), "posix_time" }
            , { typeid(SmartMet::Spine::BoundingBox).name(), "bounding_box" }
        };

    Json::Value result(Json::objectValue);
    for (const auto& map_item : param_map) {
        const ParamRecBase* p1 = map_item.second.get();
        auto iter = name_remap.find(p1->type_name);
        Json::Value& param_info = result[p1->name];
        param_info["type"] = iter == name_remap.end()
            ? Fmi::demangle_cpp_type_name(p1->type_name)
            : iter->second;
        param_info["description"] = p1->description;
        const auto* p_scalar = dynamic_cast<const ScalarParameterRec*>(p1);
        if (p_scalar) {
            param_info["is_array"] = false;
            param_info["mandatory"] = p_scalar->required;
        } else {
            const auto* p_array = dynamic_cast<const ArrayParameterRec*>(p1);
            if (p_array) {
                param_info["is_array"] = true;
                param_info["min_size"] = p_array->min_size;
                param_info["max_size"] = p_array->max_size;
                param_info["step"] = p_array->step;
            }
        }
    }
    return result;
}

void StoredQueryParamRegistry::register_scalar_param(
    const std::string& name,
    const std::string& description,
    boost::shared_ptr<ScalarParameterTemplate> param_def,
    bool required)
{
  try
  {
    boost::shared_ptr<ScalarParameterRec> rec(new ScalarParameterRec);
    rec->name = name;
    rec->description = description;
    rec->param_def = param_def;
    rec->type_name = typeid(std::string).name();
    rec->required = required;
    add_param_rec(rec);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/* Full scope here only to satisfy Doxygen */
void SmartMet::Plugin::WFS::StoredQueryParamRegistry::register_array_param(
    const std::string& name,
    const std::string& description,
    boost::shared_ptr<ArrayParameterTemplate> param_def,
    std::size_t min_size,
    std::size_t max_size)
{
  try
  {
    boost::shared_ptr<ArrayParameterRec> rec(new ArrayParameterRec);
    rec->name = name;
    rec->description = description;
    rec->param_def = param_def;
    rec->type_name = typeid(std::string).name();
    rec->min_size = min_size;
    rec->max_size = max_size, rec->step = 1;
    add_param_rec(rec);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void StoredQueryParamRegistry::add_param_rec(boost::shared_ptr<ParamRecBase> rec)
{
  try
  {
    using Fmi::demangle_cpp_type_name;

    // Verify that template argument value is supported
    const std::string& type_name = rec->type_name;
    if (supported_type_names.count(type_name) == 0)
    {
      std::string sep = "";
      std::ostringstream msg;
      std::ostringstream msg2;
      msg << "Not supported type '" << demangle_cpp_type_name(type_name) << "' "
          << "['" << type_name << "']";

      msg2 << "Use one of ";
      for (const auto& map_item : supported_type_names)
      {
        msg2 << sep << '\'' << demangle_cpp_type_name(map_item.first) << '\'';
        sep = ", ";
      }
      Fmi::Exception exception(BCP, msg.str());
      exception.addDetail(msg2.str());
      throw exception;
    }

    const std::string& name = rec->name;
    if (not param_map.insert(std::make_pair(name, rec)).second)
    {
      throw Fmi::Exception(BCP, "Duplicate parameter name '" + name + "'!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
