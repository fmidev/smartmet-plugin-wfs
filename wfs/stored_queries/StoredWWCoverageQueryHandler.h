#pragma once

#include "StoredCoverageQueryHandler.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *   @brief Handler for StoredWWCoverageQuery stored query
 */
class StoredWWCoverageQueryHandler : public StoredCoverageQueryHandler
{
 public:
  StoredWWCoverageQueryHandler(SmartMet::Spine::Reactor* reactor,
                               StoredQueryConfig::Ptr config,
                               PluginImpl& plugin_impl,
                               boost::optional<std::string> template_file_name);

 protected:
  std::vector<ContourQueryResultPtr> processQuery(ContourQueryParameter& queryParameter) const override;
  void setResultHashValue(CTPP::CDT& resultHash, const ContourQueryResult& resultItem) const override;

 private:
  std::vector<std::string> itsLimitNames;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
