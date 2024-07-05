#include "RequestFactory.h"
#include "PluginImpl.h"
#include "WfsException.h"
#include <boost/algorithm/string.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <macgyver/Exception.h>
#include <sstream>
#include <stdexcept>

namespace bw = SmartMet::Plugin::WFS;
namespace ba = boost::algorithm;

bw::RequestFactory::RequestFactory(PluginImpl& plugin_impl) : plugin_impl(plugin_impl) {}

bw::RequestFactory::~RequestFactory() = default;

bw::RequestFactory& bw::RequestFactory::register_request_type(const std::string& name,
                                                              const std::string& feature_id,
                                                              parse_kvp_t create_from_kvp,
                                                              parse_xml_t create_from_xml)
{
  try
  {
    TypeRec rec;
    const std::string lname = Fmi::ascii_tolower_copy(name);
    rec.kvp_parser = create_from_kvp;
    rec.xml_parser = create_from_xml;

    if (unimplemented_requests.count(lname) > 0)
    {
      throw Fmi::Exception(
          BCP, "WFS request '" + name + "' already registred as unimplemented!");
    }

    if (not type_map.insert(std::make_pair(lname, rec)).second)
    {
      throw Fmi::Exception(BCP, "Duplicate WFS request type '" + name + "'!");
    }

    request_names.insert(name);

    if (!feature_id.empty())
    {
      plugin_impl.get_capabilities().register_operation(feature_id);
    }

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::RequestFactory& bw::RequestFactory::register_unimplemented_request_type(const std::string& name)
{
  try
  {
    const std::string lname = Fmi::ascii_tolower_copy(name);
    if (type_map.count(lname) > 0)
    {
      throw Fmi::Exception(
          BCP, "Implementation of WFS request '" + name + "' is already registred!");
    }

    request_names.insert(name);
    unimplemented_requests.insert(Fmi::ascii_tolower_copy(name));
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<bw::RequestBase> bw::RequestFactory::parse_kvp(
    const std::string& language, const SmartMet::Spine::HTTP::Request& http_request) const
{
  try
  {
    auto service = http_request.getParameter("service");
    if (service and *service != "WFS")
    {
      Fmi::Exception exception(BCP, "Incorrect service '" + *service + "'!");
      exception.addDetail("Expected 'WFS'.");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }

    const auto name = bw::RequestBase::extract_request_name(http_request);
    if (unimplemented_requests.count(Fmi::ascii_tolower_copy(name)))
    {
      Fmi::Exception exception(BCP,
                                           "WFS request '" + name + "' is not yet implemented!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_NOT_SUPPORTED);
      throw exception;
    }

    auto type_rec = get_type_rec(name);
    if (type_rec.kvp_parser)
    {
      return (type_rec.kvp_parser)(language, http_request);
    }
    else
    {
      Fmi::Exception exception(
          BCP, "KVP format not supported for the WFS request '" + name + "'!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<bw::RequestBase> bw::RequestFactory::parse_xml(
    const std::string& language, const xercesc::DOMDocument& document) const
{
  try
  {
    const xercesc::DOMElement* root = document.getDocumentElement();
    if (root == nullptr)
    {
      Fmi::Exception exception(BCP, "The XML root element missing!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }

    const auto name = bw::RequestBase::extract_request_name(document);
    if (unimplemented_requests.count(Fmi::ascii_tolower_copy(name)))
    {
      Fmi::Exception exception(BCP, "WFS request '" + name + "' is not implemented!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_NOT_SUPPORTED);
      throw exception;
    }

    auto type_rec = get_type_rec(name);
    if (type_rec.xml_parser)
    {
      return (type_rec.xml_parser)(language, document, *root);
    }
    else
    {
      Fmi::Exception exception(BCP, "Unsupported WFS request!");
      exception.addDetail("The XML format is not (yet) supported for the current WFS request!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      exception.addParameter("WFS request", name);
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const bw::RequestFactory::TypeRec& bw::RequestFactory::get_type_rec(const std::string& name) const
{
  try
  {
    const std::string lname = Fmi::ascii_tolower_copy(name);
    auto item_it = type_map.find(lname);
    if (item_it == type_map.end())
    {
      Fmi::Exception exception(BCP, "Unrecognized WFS request!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
      exception.addParameter("WFS request", name);
      throw exception;
    }
    return item_it->second;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
