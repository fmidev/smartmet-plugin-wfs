#include "StoredQueryHandlerFactoryDef.h"
#include <openssl/sha.h>
#include <macgyver/Exception.h>
#include <dlfcn.h>
#include <sstream>
#include <string>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
StoredQueryHandlerFactoryDef::StoredQueryHandlerFactoryDef(
    StoredQueryHandlerFactoryDef::factory_t factory)
    : factory(factory)
{
  try
  {
    create_signature(signature);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

StoredQueryHandlerFactoryDef::~StoredQueryHandlerFactoryDef() = default;

std::shared_ptr<StoredQueryHandlerBase> StoredQueryHandlerFactoryDef::construct(
    const std::string &symbol_name,
    SmartMet::Spine::Reactor *reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl &plugin_data,
    std::optional<std::string> template_file_name)
{
  try
  {
    void *sym_ptr = dlsym(RTLD_DEFAULT, symbol_name.c_str());
    if (sym_ptr == nullptr)
    {
      std::ostringstream msg;
      msg << "Symbol '" << symbol_name << "' is not found";
      throw Fmi::Exception(BCP, msg.str());
    }

    auto *factory_def = reinterpret_cast<StoredQueryHandlerFactoryDef *>(sym_ptr);

    unsigned char curr_md[20];
    create_signature(curr_md);
    if (memcmp(curr_md, factory_def->signature, sizeof(curr_md)) != 0)
    {
      std::ostringstream msg;
      msg << "Signature of the found symbol '" << symbol_name << "' does not match!";
      throw Fmi::Exception(BCP, msg.str());
    }

    auto result = factory_def->factory(reactor, config, plugin_data, template_file_name);
    config->warn_about_unused_params(result.get());
    result->perform_init();
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::SquashTrace(BCP, "Operation failed!");
  }
}

void StoredQueryHandlerFactoryDef::create_signature(unsigned char *md)
{
  try
  {
    assert(sizeof(StoredQueryHandlerFactoryDef::signature) == SHA_DIGEST_LENGTH);
    std::ostringstream sample;
    const char *n1 = typeid(StoredQueryHandlerFactoryDef::factory_t).name();
    const char *n2 = typeid(&StoredQueryHandlerBase::query).name();
    const char *n3 = typeid(&StoredQueryHandlerBase::redirect).name();
    sample << n1 << '\n' << n2 << '\n' << n3 << '\n';
    const std::string s_sample = sample.str();
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, s_sample.c_str(), s_sample.length());
    SHA1_Final(md, &ctx);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
