#pragma once

#include <xercesc/dom/DOM.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Xml
{
class XmlDomErrorHandler : public xercesc::DOMErrorHandler
{
 public:
  XmlDomErrorHandler();

  ~XmlDomErrorHandler() override;

  bool handleError(const xercesc::DOMError &dom_error) override;
};

}  // namespace Xml
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
