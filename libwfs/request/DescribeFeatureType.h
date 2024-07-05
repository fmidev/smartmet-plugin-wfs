#pragma once

#include "PluginImpl.h"
#include "RequestBase.h"
#include <memory>
#include <smartmet/spine/HTTP.h>
#include <xercesc/dom/DOMDocument.hpp>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Request
{
/**
 *   @brief Represents DescribeFeatureType WFS request
 */
class DescribeFeatureType : public RequestBase
{
 public:
  DescribeFeatureType(const std::string& language,
                      const std::vector<std::pair<std::string, std::string> >& qualified_names,
                      const PluginImpl& plugin_impl);

  ~DescribeFeatureType() override;

  RequestType get_type() const override;

  void execute(std::ostream& output) const override;

  bool may_validate_xml() const override;

  static std::shared_ptr<DescribeFeatureType> create_from_kvp(
      const std::string& language,
      const SmartMet::Spine::HTTP::Request& http_request,
      const PluginImpl& plugin_impl);

  static std::shared_ptr<DescribeFeatureType> create_from_xml(
      const std::string& language,
      const xercesc::DOMDocument& document,
      const PluginImpl& plugin_impl);

  int get_response_expires_seconds() const override;

 private:
  std::vector<std::pair<std::string, std::string> > type_names;
};

}  // namespace Request
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
