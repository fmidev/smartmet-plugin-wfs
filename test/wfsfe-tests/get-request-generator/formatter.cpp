#include <boost/function_output_iterator.hpp>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xqilla/xqilla-dom3.hpp>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>

XERCES_CPP_NAMESPACE_USE;

namespace
{
std::string encimpl(std::string::value_type v)
{
  if (isalnum(v)) return std::string() + v;

  std::ostringstream enc;
  enc << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase
      << int(static_cast<unsigned char>(v));
  return enc.str();
}
}

std::string get_file_contents(const boost::filesystem::path& filename)
{
  std::string content;
  std::ifstream in(filename.c_str());

  if (in) content.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());

  return content;
}

void put_file_contents(const boost::filesystem::path& filename, const std::string& data)
{
  std::ofstream out(filename.c_str());
  if (out)
  {
    out << data;
    out.close();
  }
  else
    std::cerr << "Failed to open " << filename << " for writing" << std::endl;
  return;
}

std::string parse_xml_file_to_string(const std::string& uri)
{
  std::string result;

  try
  {
    XQillaPlatformUtils::initialize();

    DOMImplementation* xqillaImplementation =
        DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
    AutoRelease<DOMLSParser> parser(
        xqillaImplementation->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0));
    parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesSchema,
                                         false);  // TODO: Find schemas somewhere?????
    parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMElementContentWhitespace, false);

    DOMDocument* document = parser->parseURI(uri.c_str());

    if (document == 0)
    {
      std::cerr << "File not found" << std::endl;
      result = "\r\n";
    }
    else
    {
      AutoRelease<DOMLSSerializer> serializer(xqillaImplementation->createLSSerializer());
      serializer->getDomConfig()->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, false);

      const xercesc::DOMElement* root = document->getDocumentElement();

      xercesc::Janitor<XMLCh> x_result(serializer->writeToString(root));
      xercesc::Janitor<char> tmp(xercesc::XMLString::transcode(x_result.get()));
      result = tmp.get();
      boost::algorithm::trim_if(result, boost::algorithm::is_any_of(" \t\r\n"));
    }
  }
  catch (const XMLException& eXerces)
  {
    std::cerr << "Xerces init error: " << std::endl
              << "Error Message: " << UTF8(eXerces.getMessage()) << std::endl;
  }

  XQillaPlatformUtils::terminate();

  return result;
}

std::string string_encode(const std::string& str)
{
  // store the modified query string
  std::string qstr;

  std::transform(
      str.begin(),
      str.end(),
      // Append the transform result to qstr
      boost::make_function_output_iterator(boost::bind(
          static_cast<std::string& (std::string::*)(const std::string&)>(&std::string::append),
          &qstr,
          boost::placeholders::_1)),
      encimpl);
  return qstr;
}

// Decode percent encoded characters. Ignores failed conversions so control characters
// can be sent onwards.
std::string decode_percents(std::string const& url_path)
{
  std::string::size_type pos = 0, next;
  std::string result;
  result.reserve(url_path.length());

  while ((next = url_path.find('%', pos)) != std::string::npos)
  {
    result.append(url_path, pos, next - pos);
    pos = next;
    switch (url_path[pos])
    {
      case '%':
      {
        if (url_path.length() - next < 3)
        {
          result.append(url_path, pos, url_path.length() - next);
          pos = url_path.length();
          break;
        }

        char hex[3] = {url_path[next + 1], url_path[next + 2], '\0'};
        char* end_ptr;
        char res = static_cast<char>(std::strtol(hex, &end_ptr, 16));
        if (*end_ptr)
        {
          result += "%";
          pos = next + 1;
          break;
        }
        result += res;
        pos = next + 3;
        break;
      }
    }
  }

  result.append(url_path, pos, url_path.length());
  return result;
}

std::string compose_get(const std::string& request, const std::string& query_string)
{
  std::string composed_get = "GET /wfs?";

  composed_get += "request=" + request;

  if (!query_string.empty()) composed_get += "&" + query_string;

  composed_get += " HTTP/1.1\r\nHost: brainstormgw.fmi.fi\r\nfmi-apikey: foobar\r\n";

  return composed_get;
}

std::string combine_value_references(const std::vector<std::string>& value_references)
{
  return string_encode(boost::algorithm::join(value_references, " | "));
}

std::string compose_get_feature(const std::string& parameters)
{
  return compose_get("GetFeature", parameters);
}

std::string compose_get_property_value(const std::string& parameters)
{
  return compose_get("GetPropertyValue", parameters);
}

std::string compose_get_adhoc(const std::string& typenames,
                              const std::string& selection,
                              const std::string& propertynames,
                              const std::vector<std::string>& value_references)
{
  std::string parameters = "TYPENAMES=" + string_encode(typenames);

  if (!selection.empty()) parameters += "&" + selection;

  if (!propertynames.empty()) parameters += "&PROPERTYNAME=" + string_encode(propertynames);

  if (value_references.empty())
    return compose_get_feature(parameters);
  else
  {
    parameters += "&VALUEREFERENCE=" + combine_value_references(value_references);
    return compose_get_property_value(parameters);
  }
}

std::string compose_get_adhoc_filter(const std::string& typenames,
                                     const std::string& filter_language,
                                     const std::string& filter,
                                     const std::string& propertynames,
                                     const std::vector<std::string>& value_references)
{
  std::string selection_clause = "FILTER=" + string_encode(filter);

  if (!filter_language.empty())
    selection_clause += "&FILTER_LANGUAGE=" + string_encode(filter_language);

  return compose_get_adhoc(typenames, selection_clause, propertynames, value_references);
}

std::string compose_get_adhoc_bbox(const std::string& typenames,
                                   const std::string& bbox,
                                   const std::string& propertynames,
                                   const std::vector<std::string>& value_references)
{
  return compose_get_adhoc(
      typenames, "BBOX=" + string_encode(bbox), propertynames, value_references);
}

std::string compose_get_adhoc_resource_id(const std::string& typenames,
                                          const std::string& resource_id,
                                          const std::string& propertynames,
                                          const std::vector<std::string>& value_references)
{
  return compose_get_adhoc(
      typenames, "RESOURCEID=" + string_encode(resource_id), propertynames, value_references);
}

std::string compose_get_stored_query(const std::string& stored_query_id,
                                     const std::vector<std::string>& stored_query_parameters,
                                     const std::vector<std::string>& value_references)
{
  std::string parameters = "STOREDQUERY_ID=" + string_encode(stored_query_id);

  if (!stored_query_parameters.empty())
  {
    std::vector<std::string> tmp;
    for (std::vector<std::string>::const_iterator ii = stored_query_parameters.begin();
         ii != stored_query_parameters.end();
         ++ii)
    {
      tmp.clear();
      boost::algorithm::split(tmp, *ii, boost::algorithm::is_any_of("="));
      if (tmp.size() == 2) parameters += "&" + string_encode(tmp[0]) + "=" + string_encode(tmp[1]);
    }
  }

  if (value_references.empty())
    return compose_get_feature(parameters);
  else
  {
    parameters += "&VALUEREFERENCE=" + combine_value_references(value_references);
    return compose_get_property_value(parameters);
  }
}

void print_usage(const std::string& execName)
{
  std::cout << "Usage: " << execName << std::endl
            << "\t-i Input-file" << std::endl
            << "\t[-o Output-file, default stdout]" << std::endl
            << "\t[-t typenames, default \"avi:VerifiableMessage\"]" << std::endl
            << "\t[-l filter language, default \"urn:ogc:def:query Language:OGC-FES:Filter\"]"
            << std::endl
            << "\t[-b BBox generate bounding box query]" << std::endl
            << "\t[-p PROPERTYNAME limit output to selected properties]" << std::endl
            << "\t[-r resource_d generate resource id query]" << std::endl
            << "\t[-v value property for GetPropertyValue-request]" << std::endl
            << "\t[-s stored query id]" << std::endl
            << "\t[-q stored query parameters key1=value1&key2=value2 etc.]" << std::endl;
}

int main(int argc, char* argv[])
{
  std::string inputFile = "";
  std::string output_file = "";
  bool bounding_box_query = false;
  bool resource_id_query = false;
  std::string typenames = "avi:VerifiableMessage";
  std::string filter_language = "urn:ogc:def:query Language:OGC-FES:Filter";
  std::string bbox = "";
  std::string resource_id = "";
  std::string propertynames = "";
  std::vector<std::string> value_references;
  std::string stored_query_id = "";
  std::vector<std::string> query_parms;

  for (int n = 1; n < argc; n++)
  {
    switch ((int)argv[n][0])
    {
      case '-':

        switch ((int)argv[n][1])
        {
          case 'i':
            inputFile = argv[++n];
            break;
          case 'o':
            output_file = argv[++n];
            break;
          case 't':
            typenames = argv[++n];
            break;
          case 'p':
            propertynames = argv[++n];
            break;
          case 'l':
            filter_language = argv[++n];
            break;
          case 'v':
            value_references.push_back(argv[++n]);
            break;
          case 's':
            stored_query_id = argv[++n];
            break;
          case 'q':
            query_parms.push_back(argv[++n]);
            break;
          case 'b':
            bounding_box_query = true;
            bbox = argv[++n];
            break;
          case 'r':
            resource_id_query = true;
            resource_id = argv[++n];
            break;
          case '?':
            print_usage(argv[0]);
            break;
          default:
            std::cout << "Illegal option -" << argv[n][1] << std::endl;
            print_usage(argv[0]);
            break;
        }

        break;
      default:
        std::cout << "Can not understand \"" << argv[n] << "\"" << std::endl;
        print_usage(argv[0]);
        break;
    }
  }

  bool can_execute = false;

  if (bounding_box_query || resource_id_query || !inputFile.empty() || !stored_query_id.empty())
    can_execute = true;

  if (can_execute)
  {
    std::string get_request = "";

    if (stored_query_id.empty())
    {
      if (bounding_box_query)
      {
        // TODO: get bounding box from xml-file
        get_request = compose_get_adhoc_bbox(typenames, bbox, propertynames, value_references);
      }
      else if (resource_id_query)
      {
        std::cout << "Resource id query not yet implemented." << std::endl;
        get_request =
            compose_get_adhoc_resource_id(typenames, resource_id, propertynames, value_references);
      }
      else
      {
        std::string input = parse_xml_file_to_string(inputFile);
        get_request = compose_get_adhoc_filter(
            typenames, filter_language, input, propertynames, value_references);
      }
    }
    else
    {
      get_request = compose_get_stored_query(stored_query_id, query_parms, value_references);
    }

    if (!output_file.empty())
    {
      boost::filesystem::path output(output_file);
      put_file_contents(output, get_request);
    }
    else
    {
      std::cout << get_request << std::endl;
    }
  }
  else
  {
    std::cout << "Parameters missing, can not execute";
  }
}
