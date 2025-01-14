#include "StoredQueryParamDef.h"
#include "WfsConvenience.h"
#include "WfsException.h"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/qi.hpp>
#include <macgyver/TimeParser.h>
#include <macgyver/TypeName.h>
#include <macgyver/StringConversion.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>

#include <iostream>

namespace qi = boost::spirit::qi;
namespace ns = boost::spirit::standard;
namespace ba = boost::algorithm;
namespace bl = boost::lambda;
namespace p = boost::phoenix;

using qi_rule = qi::rule<std::string::const_iterator, std::string
        (), ns::space_type>;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
StoredQueryParamDef::StoredQueryParamDef()
     
= default;

void StoredQueryParamDef::parse_def(const std::string& desc)
{
  try
  {
    ValueType value_type;
    bool is_array = false;
    unsigned min_size = 1;
    unsigned max_size = 1;

    qi_rule string_p = qi::string("string");
    qi_rule int_p = qi::string("integer") | qi::string("int");
    qi_rule uint_p = qi::string("uint");
    qi_rule double_p = qi::string("double");
    qi_rule time_p = qi::string("time");
    qi_rule bool_p = qi::string("bool");
    qi_rule point_p = qi::string("point");
    qi_rule bbox_p = qi::string("bbox");

    qi_rule type_p = (string_p   [bl::var(value_type) = STRING] |
                      int_p      [bl::var(value_type) = INT] |
                      uint_p     [bl::var(value_type) = UINT] |
                      double_p   [bl::var(value_type) = DOUBLE] |
                      time_p     [bl::var(value_type) = TIME] |
                      bool_p     [bl::var(value_type) = BOOL] |
                      point_p    [bl::var(value_type) = POINT] |
                      bbox_p     [bl::var(value_type) = BBOX]);
    ;

    qi_rule array_dim_p =
        qi::char_('[') >> qi::uint_[(bl::var(min_size) = bl::_1, bl::var(max_size) = bl::_1)] >>
        -(ns::string("..") >> qi::uint_[bl::var(max_size) = bl::_1]) >> qi::char_(']');

    qi_rule param_p = type_p >> -(array_dim_p[bl::var(is_array) = true]) >> qi::eoi;

    bool ok = qi::phrase_parse(desc.begin(), desc.end(), param_p, ns::space);

    if (ok)
    {
      this->value_type = value_type;
      this->is_array = is_array;
      this->min_size = min_size;
      this->max_size = max_size;

      // std::cout << "PARAM_TYPE: '" << desc << "' ==> (" << (int)value_type
      //			<< " " << (is_array ? "ARRAY " : "SCALAR ")
      //			<< min_size << " " << max_size << ")" << std::endl;
    }
    else
    {
      throw Fmi::Exception(
          BCP, "Failed top parse parameter type specification '" + desc + "'!");
    }

    if (is_array && (min_size > max_size))
    {
      throw Fmi::Exception(BCP, "Invalid array size specification in '" + desc + "'!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

StoredQueryParamDef::~StoredQueryParamDef() = default;

SmartMet::Spine::Value StoredQueryParamDef::readValue(const std::string& value) const
{
  try
  {
    SmartMet::Spine::Value result;

    try
    {
      switch (value_type)
      {
        case DOUBLE:
          result.set_double(Fmi::stod(value));
          break;

        case INT:
          result.set_int(Fmi::stoi(value));
          break;

        case UINT:
          result.set_uint(Fmi::stoul(value));
          break;

        case TIME:
          result.set_ptime(string2ptime(value));
          break;

        case BOOL:
          result.set_bool(string2bool(value));
          break;

        case STRING:
          result.set_string(value);
          break;

        case POINT:
        {
          SmartMet::Spine::Point p(value);
          result.set_point(p);
        }
        break;

        case BBOX:
        {
          SmartMet::Spine::BoundingBox b(value);
          result.set_bbox(b);
        }
        break;

        default:
        {
          Fmi::Exception exception(
              BCP, "INTERNAL ERROR: incorrect type code '" + std::to_string(value_type) + "'!");
          exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
          throw exception;
        }
      }

      return result;
    }
    catch (const boost::bad_lexical_cast& err)
    {
      Fmi::Exception exception(
          BCP, "Invalid stored query parameter value in '" + value + "'!", nullptr);
      if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception.disableStackTrace();
    }
    catch (...)
    {
      Fmi::Exception exception(BCP, "Operation failed!", nullptr);
      if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == nullptr)
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception.disableStackTrace();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

SmartMet::Spine::Value StoredQueryParamDef::convValue(const SmartMet::Spine::Value& value) const
{
  try
  {
    SmartMet::Spine::Value result;

    switch (value_type)
    {
      case DOUBLE:
        result.set_double(value.get_double());
        break;

      case INT:
        result.set_int(value.get_int());
        break;

      case UINT:
        result.set_uint(value.get_uint());
        break;

      case TIME:
        result.set_ptime(value.get_ptime());
        break;

      case BOOL:
        result.set_bool(value.get_bool());
        break;

      case STRING:
        result.set_string(value.get_string());
        break;

      default:
      {
        Fmi::Exception exception(
            BCP, "INTERNAL ERROR: incorrect type code '" + std::to_string(value_type) + "'!");
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
        throw exception;
      }
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<SmartMet::Spine::Value> StoredQueryParamDef::readValues(
    const std::vector<std::string>& values) const
{
  try
  {
    if ((values.size() >= min_size) && (values.size() <= max_size))
    {
      std::vector<SmartMet::Spine::Value> result;
      std::transform(values.begin(),
                     values.end(),
                     std::back_inserter(result),
          boost::bind(&StoredQueryParamDef::readValue, this, boost::placeholders::_1));
      return result;
    }
    else
    {
      std::ostringstream msg;
      msg << "Expected: " << min_size;
      if (min_size != max_size)
        msg << ".." << max_size;

      Fmi::Exception exception(
          BCP, "Invalid array size '" + std::to_string(values.size()) + "' in parameter!");
      exception.addDetail(msg.str());
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void StoredQueryParamDef::dump(std::ostream& stream) const
{
  try
  {
    switch (value_type)
    {
      default:
      case UNDEFINED:
        stream << "?";
        break;
      case STRING:
        stream << "string";
        break;
      case INT:
        stream << "int";
        break;
      case UINT:
        stream << "uint";
        break;
      case DOUBLE:
        stream << "double";
        break;
      case TIME:
        stream << "time";
        break;
      case BOOL:
        stream << "bool";
        break;
    }

    if (is_array)
    {
      stream << '[' << min_size;
      if (max_size != min_size)
        stream << ".." << max_size;
      stream << "]";
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
