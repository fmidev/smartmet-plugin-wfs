#pragma once

#include "RequestBase.h"
#include <xercesc/dom/DOMDocument.hpp>
#include <iosfwd>
#include <memory>
#include <string>

namespace SmartMet::Plugin::WFS
{
class PluginImpl;
}  // namespace SmartMet::Plugin::WFS
namespace SmartMet::Spine::HTTP
{
class Request;
}  // namespace SmartMet::Spine::HTTP

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Request
{
/**
 *  @brief Represents ListStoredQueries request
 */
class ListStoredQueries : public RequestBase
{
 private:
  ListStoredQueries(const std::string& language, const PluginImpl& plugin_impl);

 public:
  ~ListStoredQueries() override;

  RequestType get_type() const override;

  void execute(std::ostream& output) const override;

  static std::shared_ptr<ListStoredQueries> create_from_kvp(
      const std::string& language,
      const SmartMet::Spine::HTTP::Request& http_request,
      const PluginImpl& plugin_impl);

  static std::shared_ptr<ListStoredQueries> create_from_xml(const std::string& language,
                                                              const xercesc::DOMDocument& document,
                                                              const PluginImpl& plugin_impl);

  int get_response_expires_seconds() const override;
};

}  // namespace Request
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
