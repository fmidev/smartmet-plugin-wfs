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
                               boost::shared_ptr<StoredQueryConfig> config,
                               PluginData& plugin_data,
                               boost::optional<std::string> template_file_name);

 protected:
  std::vector<ContourQueryResultPtr> processQuery(ContourQueryParameter& queryParameter) const;
  void setResultHashValue(CTPP::CDT& resultHash, const ContourQueryResult& resultItem) const;

 private:
  std::vector<std::string> itsLimitNames;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
