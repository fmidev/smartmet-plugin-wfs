#pragma once

#include "PluginData.h"
#include "RequestBase.h"
#include <boost/shared_ptr.hpp>
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
                      const PluginData& plugin_data);

  virtual ~DescribeFeatureType();

  virtual RequestType get_type() const;

  virtual void execute(std::ostream& output) const;

  virtual bool may_validate_xml() const;

  static boost::shared_ptr<DescribeFeatureType> create_from_kvp(
      const std::string& language,
      const SmartMet::Spine::HTTP::Request& http_request,
      const PluginData& plugin_data);

  static boost::shared_ptr<DescribeFeatureType> create_from_xml(
      const std::string& language,
      const xercesc::DOMDocument& document,
      const PluginData& plugin_data);

  virtual int get_response_expires_seconds() const;

 private:
  std::vector<std::pair<std::string, std::string> > type_names;
  const PluginData& plugin_data;
};

}  // namespace Request
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
