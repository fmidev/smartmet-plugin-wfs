
#include "XmlUtils.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_array.hpp>
#include <macgyver/Exception.h>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>

namespace ba = boost::algorithm;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Xml
{

std::pair<std::string, bool> to_opt_string(const XMLCh* src)
{
  try
  {
    if (src)
    {
      return std::make_pair(to_string(src), true);
    }
    else
    {
      return std::make_pair<std::string, bool>("", false);
      ;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const XMLCh* src)
{
  try
  {
    if (src)
    {
      std::shared_ptr<char> tmp(
          xercesc::XMLString::transcode(src),
          [] (char *ptr) -> void { xercesc::XMLString::release(&ptr); });

      return tmp.get();
    }
    else
    {
      throw Fmi::Exception(
          BCP, "SmartMet::Plugin::WFS::Xml::to_string: XML string expected but got nullptr");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<XMLCh> to_xmlch(const char* src)
{
  try
  {
    std::shared_ptr<XMLCh> tmp(
        xercesc::XMLString::transcode(src),
        [] (XMLCh* ptr) -> void { xercesc::XMLString::release(&ptr); });

    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::pair<std::string, std::string> get_name_info(const xercesc::DOMNode* node)
{
  try
  {
    if (node)
    {
      const XMLCh* x_name = node->getLocalName();
      if (x_name == nullptr)
        x_name = node->getNodeName();
      const std::string name = to_string(x_name);
      const std::string ns_info = to_opt_string(node->getNamespaceURI()).first;
      return std::make_pair(name, ns_info);
    }
    else
    {
      throw Fmi::Exception(
          BCP,
          "SmartMet::Plugin::WFS::Xml::get_name_info(): nullptr is not permitted as the argument");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void check_name_info(const xercesc::DOMNode* node,
                     const std::string& ns,
                     const std::string& name,
                     const char* given_location)
{
  try
  {
    if (node == nullptr)
    {
      throw Fmi::Exception(BCP, "No XML DOM node aavailble (nullptr specified)");
    }

    const auto name_info = get_name_info(node);
    const char* location = given_location ? given_location : __PRETTY_FUNCTION__;

    if ((name_info.first != name) or (name_info.second != ns))
    {
      const std::string msg = fmt::format("{}: invalid element name {{}}{} ({{}}{} expected)",
                                          location,
                                          name_info.second,
                                          name_info.first,
                                          ns,
                                          name);
      throw Fmi::Exception(BCP, msg);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string check_name_info(const xercesc::DOMNode* node,
                            const std::string& ns,
                            const std::set<std::string>& allowed_names,
                            const char* given_location)
{
  try
  {
    const auto name_info = get_name_info(node);
    const char* location = given_location ? given_location : __PRETTY_FUNCTION__;

    if ((allowed_names.count(name_info.first) == 0) or (name_info.second != ns))
    {
      std::string d;
      std::string msg = fmt::format("{}: invalid element name {{}}{} ({{}}{} expected)",
                                    location,
                                    name_info.second,
                                    name_info.first,
                                    ns);
      for (const std::string& nm : allowed_names)
      {
        msg += d + nm;
        d = "|";
      }
      msg + ") expected";
      throw Fmi::Exception(BCP, msg);
    }
    else
    {
      return name_info.first;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::pair<std::string, bool> get_attr(const xercesc::DOMElement& elem,
                                      const std::string& ns,
                                      const std::string& name)
{
  try
  {
    (void)ns;
    auto x_name = to_xmlch(name.c_str());
    if (elem.hasAttribute(x_name.get()))
    {
      const XMLCh* x_val = elem.getAttribute(x_name.get());
      return std::make_pair(to_string(x_val), true);
    }
    else
    {
      return std::make_pair<std::string, bool>("", false);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string get_mandatory_attr(const xercesc::DOMElement& elem,
                               const std::string& ns,
                               const std::string& name)
{
  try
  {
    (void)ns;
    auto x_name = to_xmlch(name.c_str());
    if (elem.hasAttribute(x_name.get()))
    {
      const XMLCh* x_val = elem.getAttribute(x_name.get());
      return to_string(x_val);
    }
    else
    {
      const std::string msg = fmt::format("SmartMet::Plugin::WFS::Xml::get_mandatory_attr():"
                                          " mandatory attribute {} missing",
                                          name);
      throw Fmi::Exception(BCP, msg);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string get_optional_attr(const xercesc::DOMElement& elem,
                              const std::string& ns,
                              const std::string& name,
                              const std::string& default_value)
{
  try
  {
    auto tmp = get_attr(elem, ns, name);
    return tmp.second ? tmp.first : default_value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void verify_mandatory_attr_value(const xercesc::DOMElement& elem,
                                 const std::string& ns,
                                 const std::string& name,
                                 const std::string& exp_value)
{
  const std::string value = get_mandatory_attr(elem, ns, name);
  if (value != exp_value)
  {
    throw Fmi::Exception(BCP,
      fmt::format("Incorrect value '{}' of mandatory fixed attribute {{}}{} ('{}' expected)",
                  value,
                  ns,
                  name,
                  exp_value));
  }
}

void verify_mandatory_attr_value(const xercesc::DOMElement& elem,
                                 const std::string& ns,
                                 const std::string& name,
                                 std::function<void(const std::string&)> checker)
{
  const std::string value = get_mandatory_attr(elem, ns, name);
  checker(value);
}

std::string extract_text(const xercesc::DOMElement& element)
{
  try
  {
    std::ostringstream text;
    for (xercesc::DOMNode* curr = element.getFirstChild(); curr; curr = curr->getNextSibling())
    {
      xercesc::DOMNode::NodeType type = curr->getNodeType();
      if (type == xercesc::DOMNode::ATTRIBUTE_NODE or type == xercesc::DOMNode::COMMENT_NODE)
      {
        // We are not interested about these
      }
      else if (type == xercesc::DOMNode::TEXT_NODE or type == xercesc::DOMNode::CDATA_SECTION_NODE)
      {
        const XMLCh* data = curr->getNodeValue();
        // Should not be nullptr, but let us play safe
        const std::string tmp_str = to_opt_string(data).first;
        text << tmp_str;
      }
      else
      {
        throw Fmi::Exception(BCP, "Unsupported XML node!");
      }
    }

    return text.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

xercesc::DOMLSSerializer* create_dom_serializer()
{
  try
  {
      auto features = to_xmlch("LS");;

    xercesc::DOMImplementationLS* impl = nullptr;
    xercesc::DOMImplementation* impl_base =
        xercesc::DOMImplementationRegistry::getDOMImplementation(features.get());

    if (impl_base != nullptr)
    {
      impl = dynamic_cast<xercesc::DOMImplementationLS*>(impl_base);
    }

    if (impl == nullptr)
    {
      throw Fmi::Exception(BCP, "Failed to get instance of xercesc::DOMImplementationLS");
    }

    return impl->createLSSerializer();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string xml2string(const xercesc::DOMNode* node)
{
  try
  {
    xercesc::Janitor<xercesc::DOMLSSerializer> serializer(create_dom_serializer());

    // Make the output more human readable by inserting line feeds.
    if (serializer->getDomConfig()->canSetParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint,
                                                    true))
      serializer->getDomConfig()->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true);

    std::shared_ptr<XMLCh> x_result(
        serializer->writeToString(node),
        [] (XMLCh * ptr) -> void { xercesc::XMLString::release(&ptr); }) ;
    std::string result = to_string(x_result.get());
    ba::trim_if(result, ba::is_any_of(" \t\r\n"));
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<xercesc::DOMElement*> get_child_elements(const xercesc::DOMElement& element,
                                                     const std::string& ns,
                                                     const std::string& name)
{
  try
  {
    std::vector<xercesc::DOMElement*> result;
    auto x_ns = to_xmlch(ns.c_str());
    auto x_name =  to_xmlch(name.c_str());
    for (xercesc::DOMNode* child = element.getFirstChild(); child; child = child->getNextSibling())
    {
      if (child->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {
        auto& child_element = dynamic_cast<xercesc::DOMElement&>(*child);
        if ((ns == "*") or xercesc::XMLString::equals(x_ns.get(), child_element.getNamespaceURI()))
        {
          if ((name == "*") or
              xercesc::XMLString::equals(x_name.get(), child_element.getLocalName()))
          {
            result.push_back(&child_element);
          }
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

boost::shared_ptr<xercesc::DOMDocument> create_dom_document(const std::string& ns,
                                                            const std::string& name)
{
  try
  {
    auto features =  to_xmlch("LS");

    xercesc::DOMImplementation* impl =
        xercesc::DOMImplementationRegistry::getDOMImplementation(features.get());

    if (impl == nullptr)
    {
      throw Fmi::Exception(BCP, "Failed to get xercesc::DOMImplementation instance");
    }
    else
    {
      boost::shared_ptr<xercesc::DOMDocument> doc(impl->createDocument());
      doc->appendChild(create_element(*doc, ns, name));
      return doc;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

xercesc::DOMElement* create_element(xercesc::DOMDocument& doc,
                                    const std::string& ns,
                                    const std::string& name)
{
  try
  {
    auto x_ns =  to_xmlch(ns.c_str());
    auto x_name = to_xmlch(name.c_str());
    xercesc::DOMElement* element = doc.createElementNS(x_ns.get(), x_name.get());
    return element;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

xercesc::DOMElement* append_child_element(xercesc::DOMElement& parent,
                                          const std::string& ns,
                                          const std::string& name)
{
  try
  {
    auto x_ns =  to_xmlch(ns.c_str());
    auto x_name =  to_xmlch(name.c_str());
    xercesc::DOMElement* child =
        parent.getOwnerDocument()->createElementNS(x_ns.get(), x_name.get());
    parent.appendChild(child);
    return child;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

xercesc::DOMElement* append_child_text_element(xercesc::DOMElement& parent,
                                               const std::string& ns,
                                               const std::string& name,
                                               const std::string& value)
{
  try
  {
    auto x_value = to_xmlch(value.c_str());
    xercesc::DOMElement* child = append_child_element(parent, ns, name);
    xercesc::DOMText* text = parent.getOwnerDocument()->createTextNode(x_value.get());
    child->appendChild(text);
    return child;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void set_attr(xercesc::DOMElement& element, const std::string& name, const std::string& value)
{
  try
  {
    auto x_name =  to_xmlch(name.c_str());
    auto x_value = to_xmlch(value.c_str());
    element.setAttribute(x_name.get(), x_value.get());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace Xml
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
