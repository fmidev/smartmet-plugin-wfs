#include "ErrorResponseGenerator.h"
#include "AdHocQuery.h"
#include "PluginImpl.h"
#include "StoredQuery.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <macgyver/TypeName.h>
#include <spine/Convenience.h>
#include <macgyver/Exception.h>

using SmartMet::Plugin::WFS::ErrorResponseGenerator;

namespace ba = boost::algorithm;

ErrorResponseGenerator::ErrorResponseGenerator(const SmartMet::Plugin::WFS::PluginImpl& plugin_impl)
    : plugin_impl(plugin_impl)
{
}

ErrorResponseGenerator::~ErrorResponseGenerator() = default;

ErrorResponseGenerator::ErrorResponse ErrorResponseGenerator::create_error_response(
    processing_phase_t phase,
    std::variant<const SmartMet::Spine::HTTP::Request*, StoredQuery*> query_info)
{
  try
  {
    ErrorResponse response;

    const std::exception_ptr eptr = std::current_exception();
    if (!eptr)
      throw Fmi::Exception(BCP, "[INTERNAL ERROR]: must not be be called outside an exception context");

    try
    {
      std::rethrow_exception(eptr);
    }
    catch (Fmi::Exception& err)
    {
      auto hash = handle_wfs_exception(err);
      add_query_info(hash, query_info);
      response.status = SmartMet::Spine::HTTP::bad_request;
      response.response = format_message(hash);
      response.log_message = format_log_message(hash);
      response.wfs_err_code = hash["exceptionList"][0]["exceptionCode"].GetString();
      return response;
    }
    catch (const std::exception& err)
    {
      auto hash = handle_std_exception(err, phase);
      add_query_info(hash, query_info);
      response.status = SmartMet::Spine::HTTP::bad_request;
      response.response = format_message(hash);
      response.log_message = format_log_message(hash);
      response.wfs_err_code = hash["exceptionList"][0]["exceptionCode"].GetString();
      return response;
    }
    catch (...)
    {
      auto hash = handle_unknown_exception(phase);
      add_query_info(hash, query_info);
      response.status = SmartMet::Spine::HTTP::internal_server_error;
      response.response = format_message(hash);
      response.log_message = format_log_message(hash);
      response.wfs_err_code = hash["exceptionList"][0]["exceptionCode"].GetString();
      return response;
    }

    std::cerr << METHOD_NAME << " [INTERNAL ERROR]: is only expected to be called"
              << " from a catch block" << std::endl;
    abort();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

CTPP::CDT ErrorResponseGenerator::handle_wfs_exception(Fmi::Exception& err)
{
  // std::cerr << err;

  CTPP::CDT hash;

  const Fmi::Exception* e = err.getExceptionByParameterName(WFS_LANGUAGE);
  if (e)
  {
    // The language information might be in a different exception
    // than the actual exception code.

    const char* language = e->getParameterValue(WFS_LANGUAGE);
    if (language)
      hash["language"] = language;
  }

  e = err.getExceptionByParameterName(WFS_EXCEPTION_CODE);
  if (e)
  {
    const char* exceptionCode = e->getParameterValue(WFS_EXCEPTION_CODE);
    if (exceptionCode)
      hash["exceptionList"][0]["exceptionCode"] = exceptionCode;
    else
      hash["exceptionList"][0]["exceptionCode"] = "InvalidExceptionCode";

    const char* location = e->getParameterValue(WFS_LOCATION);
    if (location)
      hash["exceptionList"][0]["location"] = location;
  }
  else
  {
    hash["exceptionList"][0]["exceptionCode"] = "InvalidExceptionCode";
  }

  const Fmi::Exception* fe = err.getFirstException();
  hash["exceptionList"][0]["textList"][0] = fe->getWhat();

  unsigned int cnt = fe->getDetailCount();
  for (unsigned int i = 0; i < cnt; i++)
  {
    hash["exceptionList"][0]["textList"][i + 1] = fe->getDetailByIndex(i);
  }

  return hash;
}

CTPP::CDT ErrorResponseGenerator::handle_std_exception(const std::exception& err,
                                                       processing_phase_t phase)
{
  try
  {
    std::ostringstream msg;
    msg << Fmi::date_time::to_simple_string(plugin_impl.get_local_time_stamp()) << " C++ exception '"
        << Fmi::get_type_name(&err) << "': " << err.what();

    CTPP::CDT hash;
    hash["exceptionList"][0]["exceptionCode"] = get_wfs_err_code(phase);
    hash["exceptionList"][0]["textList"][0] = msg.str();
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

CTPP::CDT ErrorResponseGenerator::handle_unknown_exception(processing_phase_t phase)
{
  try
  {
    std::ostringstream msg;
    std::cerr << Fmi::date_time::to_simple_string(plugin_impl.get_local_time_stamp()) << " error: "
              << "Unknown exception '" << Fmi::current_exception_type() << "'";

    CTPP::CDT hash;
    hash["exceptionList"][0]["exceptionCode"] = get_wfs_err_code(phase);
    hash["exceptionList"][0]["textList"][0] = msg.str();
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string ErrorResponseGenerator::get_wfs_err_code(processing_phase_t phase)
{
  try
  {
    switch (phase)
    {
      case REQ_PARSING:
        return "OperationParsingFailed";

      case REQ_PROCESSING:
      default:
        return "OperationProcessingFailed";
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void ErrorResponseGenerator::add_query_info(
    CTPP::CDT& hash, std::variant<const SmartMet::Spine::HTTP::Request*, StoredQuery*> query_info)
{
  try
  {
    switch (query_info.index())
    {
      case 0:
        add_http_request_info(hash, *std::get<const SmartMet::Spine::HTTP::Request*>(query_info));
        break;

      case 1:
        add_stored_query_info(hash, *std::get<StoredQuery*>(query_info));
        break;

      default:
        break;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void ErrorResponseGenerator::add_http_request_info(CTPP::CDT& hash,
                                                   const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    using boost::format;
    using boost::str;

    const auto method = request.getMethod();
    if (method == SmartMet::Spine::HTTP::RequestMethod::POST)
    {
      auto content_type = request.getHeader("Content-Type");
      auto content = request.getContent();
      hash["exceptionList"][0]["textList"].PushBack(str(format("URI: %1%") % request.getURI()));

      if (content_type)
      {
        hash["exceptionList"][0]["textList"].PushBack(
            str(format("Content-Type: %1%") % *content_type));
      }

      if (!content.empty())
      {
        hash["exceptionList"][0]["textList"].PushBack(
            str(format("Content: %1%)") % request.getContent()));
      }
    }
    else if (method == SmartMet::Spine::HTTP::RequestMethod::GET)
    {
      hash["exceptionList"][0]["textList"].PushBack(str(format("URI: %1%") % request.getURI()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void ErrorResponseGenerator::add_stored_query_info(CTPP::CDT& hash, const StoredQuery& query)
{
  try
  {
    using boost::format;
    using boost::str;

    const std::string& query_id = query.get_stored_query_id();
    const auto& param_map = query.get_param_map();

    hash["exceptionList"][0]["textList"].PushBack(str(format("ID: %1%") % query_id));
    hash["exceptionList"][0]["textList"].PushBack(
        str(format("Parameters: %1%") % param_map.as_string()));

    if (query.get_type() == QueryBase::QUERY)
    {
      const auto* adhoc_query = dynamic_cast<const AdHocQuery*>(&query);

      if (adhoc_query)
      {
        hash["exceptionList"][0]["textList"].PushBack(
            str(format("Filter-expression: %1%") % adhoc_query->get_filter_expression()));
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string ErrorResponseGenerator::format_message(CTPP::CDT& hash)
{
  try
  {
    std::ostringstream output, log_messages;
    try
    {
      auto exception_formatter = plugin_impl.get_exception_formatter();
      exception_formatter->process(hash, output, log_messages);
    }
    catch (const std::exception& err)
    {
      std::cerr << METHOD_NAME << ": failed to format error message. CTTP2 error messages are:"
                << log_messages.str() << std::endl;
      return "INTERNAL ERROR";
    }
    const std::string content = output.str();
    return content;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string ErrorResponseGenerator::format_log_message(CTPP::CDT& hash)
{
  try
  {
    std::ostringstream msg;
    const std::string content = hash["exceptionList"][0].RecursiveDump(10);
    msg << SmartMet::Spine::log_time_str() << ": ERROR: " << content << std::endl;
    return msg.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
