#pragma once

#include "PluginImpl.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerBase.h"
#include <optional>
#include <memory>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class PluginImpl;

class StoredQueryHandlerFactoryDef
{
 public:
  using factory_t = std::shared_ptr<StoredQueryHandlerBase>
        (*)(SmartMet::Spine::Reactor *, StoredQueryConfig::Ptr, PluginImpl &, std::optional<std::string>);

 private:
  unsigned char signature[20];
  factory_t factory;

 public:
  StoredQueryHandlerFactoryDef(factory_t factory);

  virtual ~StoredQueryHandlerFactoryDef();

  static std::shared_ptr<StoredQueryHandlerBase> construct(
      const std::string& symbol_name,
      SmartMet::Spine::Reactor* reactor,
      StoredQueryConfig::Ptr config,
      PluginImpl& plugin_impl,
      std::optional<std::string> template_file_name);

 private:
  static void create_signature(unsigned char* md);
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
