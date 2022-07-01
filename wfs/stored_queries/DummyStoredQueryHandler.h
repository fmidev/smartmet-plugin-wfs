#pragma once

#include "ParameterTemplateBase.h"
#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *  This is dummy stored query handler intended for parameter handling testing only
 */
class DummyStoredQueryHandler : public StoredQueryHandlerBase
{
 public:
  DummyStoredQueryHandler(SmartMet::Spine::Reactor* reactor,
                          StoredQueryConfig::Ptr config,
                          PluginImpl& plugin_impl,
                          boost::optional<std::string> template_file_name);

  ~DummyStoredQueryHandler() override;

  void init_handler() override;

  void query(const StoredQuery& query,
	     const std::string& language,
	     const boost::optional<std::string>& hostname,
	     std::ostream& output) const override;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
