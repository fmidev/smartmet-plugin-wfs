#pragma once

#include "PluginImpl.h"
#include "RequestBase.h"
#include <memory>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Request
{
/**
 *   @brief Class for representing GetCapabilities WFS request
 */
class GetCapabilities : public RequestBase
{
 public:
  GetCapabilities(const std::string& language, const PluginImpl& plugin_impl);

  ~GetCapabilities() override;

  RequestType get_type() const override;

  void execute(std::ostream& ost) const override;

  static std::shared_ptr<GetCapabilities> create_from_kvp(
      const std::string& language,
      const SmartMet::Spine::HTTP::Request& http_request,
      const PluginImpl& plugin_impl);

  static std::shared_ptr<GetCapabilities> create_from_xml(const std::string& language,
                                                            const xercesc::DOMDocument& document,
                                                            const PluginImpl& plugin_impl);

  int get_response_expires_seconds() const override;

 private:
  std::set<std::string> languages;
  std::optional<std::string> requested_language;
};

}  // namespace Request
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
