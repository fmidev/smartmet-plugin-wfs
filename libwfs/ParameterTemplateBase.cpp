#include "ParameterTemplateBase.h"
#include "RequestParameterMap.h"
#include "SupportsExtraHandlerParams.h"
#include <macgyver/StringConversion.h>
#include <macgyver/Exception.h>
#include <sstream>
#include <string>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
const char* ParameterTemplateBase::HANDLER_PARAM_NAME = "handler_params";

ParameterTemplateBase::ParameterTemplateBase(StoredQueryConfig& config,
                                             const std::string& base_path,
                                             const std::string& config_path)
    : config(config), base_path(base_path), config_path(config_path)
{
}

ParameterTemplateBase::~ParameterTemplateBase() {}

libconfig::Setting* ParameterTemplateBase::get_setting_root()
{
  try
  {
    auto& root = base_path == ""
        ? config.get_config().getRoot()
        : config.get_mandatory_config_param<libconfig::Setting&>(base_path);
    return config.find_setting(root, config_path, false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const StoredQueryConfig::ParamDesc& ParameterTemplateBase::get_param_desc(
    const std::string& link_name) const
{
  try
  {
    namespace ba = boost::algorithm;

    const auto* param_desc = config.get_param_desc(link_name);
    if (not param_desc)
    {
      std::ostringstream msg;
      throw Fmi::Exception(
          BCP, "The description of the '" + link_name + "' parameter is not found!");
    }
    else
    {
      param_desc->set_used();
      return *param_desc;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void ParameterTemplateBase::handle_exceptions(const std::string& location) const
{
  try
  {
    // FIXME: catch other exceptions if needed

    try
    {
      throw;
    }
    catch (const libconfig::ConfigException& err)
    {
      std::ostringstream msg;
      msg << location << ": error getting data from configuration:"
          << " path='" << config_path << "' exception='" << Fmi::current_exception_type()
          << "' message='" << err.what() << "'";
      throw Fmi::Exception(BCP, msg.str());
    }
    catch (const boost::bad_lexical_cast& err)
    {
      std::ostringstream msg;
      msg << location
          << " [INTERNAL ERROR]: lexical_cast<> failed (configuration error suspected): "
          << err.what();
      Fmi::Exception exception(BCP, msg.str());
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
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
