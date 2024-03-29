#pragma once

#include "DataSetIndex.h"
#include "stored_queries/StoredAtomQueryHandlerBase.h"
#include <list>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredFileQueryHandler : public StoredAtomQueryHandlerBase
{
 public:
  StoredFileQueryHandler(SmartMet::Spine::Reactor* reactor,
                         StoredQueryConfig::Ptr config,
                         PluginImpl& plugin_impl,
                         boost::optional<std::string> template_file_name);

  ~StoredFileQueryHandler() override;

  std::string get_handler_description() const override;

 private:
  void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<boost::shared_ptr<RequestParameterMap> >& result) const override;

 private:
  std::list<boost::shared_ptr<DataSetDefinition> > ds_list;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
