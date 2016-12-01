#pragma once

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <spine/ConfigBase.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class MultiLanguageString
{
 public:
  MultiLanguageString(const std::string& default_language, libconfig::Setting& setting);

  static boost::shared_ptr<MultiLanguageString> create(const std::string& default_language,
                                                       libconfig::Setting& setting);

  virtual ~MultiLanguageString();

  inline std::string get_default_language() const { return default_language; }
  inline std::string get() const { return get(default_language); }
  std::string get(const std::string& language) const;

  inline const std::map<std::string, std::string>& get_content() const { return data; }
 private:
  const std::string default_language;
  std::map<std::string, std::string> data;
};

typedef boost::shared_ptr<MultiLanguageString> MultiLanguageStringP;

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
