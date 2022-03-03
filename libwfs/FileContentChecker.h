#pragma once

#include <map>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

class FileContentChecker
{
 public:
  FileContentChecker(std::size_t max_size = 131077U, int debug_level = 0);
  virtual ~FileContentChecker() = default;

  bool check_file_hash(const std::string& fn);

  void remove_file_hash(const std::string& fn);

 private:
  std::map<std::string, std::string> file_hash_map;
  std::size_t max_size;
  int debug_level;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
