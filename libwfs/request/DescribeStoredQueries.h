#pragma once

#include "PluginImpl.h"
#include "RequestBase.h"
#include <xercesc/dom/DOMDocument.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

namespace Request
{
/**
 *  @brief Represents DescribeStoredQueries request
 */
class DescribeStoredQueries : public RequestBase
{
  std::vector<std::string> ids;
  bool show_hidden;
  bool show_all;

 private:
  DescribeStoredQueries(const std::string& language,
                        const std::vector<std::string>& ids,
                        const PluginImpl& plugin_impl);

 public:
  ~DescribeStoredQueries() override;

  RequestType get_type() const override;

  void execute(std::ostream& ost) const override;

  static boost::shared_ptr<DescribeStoredQueries> create_from_kvp(
      const std::string& language,
      const SmartMet::Spine::HTTP::Request& http_request,
      const PluginImpl& plugin_impl);

  static boost::shared_ptr<DescribeStoredQueries> create_from_xml(
      const std::string& language,
      const xercesc::DOMDocument& document,
      const PluginImpl& plugin_impl);

  int get_response_expires_seconds() const override;
};

}  // namespace Request
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
