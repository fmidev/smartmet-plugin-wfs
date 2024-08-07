#pragma once

#include <boost/shared_array.hpp>

#include <macgyver/DateTime.h>
#include <spine/Value.h>
#include <map>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class OBStream
{
 public:
  OBStream();
  virtual ~OBStream();
  std::string raw_data() const;
  operator std::string() const;
  void put_bit(bool val);
  void put_bits(unsigned value, int num);
  void put_int(int64_t value);
  void put_unsigned(uint64_t value);
  void put_double(double value);
  void put_char(char c);
  void put_string(const std::string& text);
  void put_ptime(const Fmi::DateTime& tm);
  void put_value(const SmartMet::Spine::Value& value);
  void put_value_map(const std::multimap<std::string, SmartMet::Spine::Value>& data);

 private:
  std::size_t reserved;
  boost::shared_array<uint8_t> data;
  std::size_t ind{0U};
  int num_bits{0};
};

class IBStream
{
 public:
  IBStream(const uint8_t* data, std::size_t length);
  virtual ~IBStream();
  unsigned get_bit();
  unsigned get_bits(int num);
  uint64_t get_unsigned();
  int64_t get_int();
  double get_double();
  char get_char();
  std::string get_string();
  Fmi::DateTime get_ptime();
  SmartMet::Spine::Value get_value();
  std::multimap<std::string, SmartMet::Spine::Value> get_value_map();

 private:
  std::size_t length;
  std::size_t pos{0};
  boost::shared_array<uint8_t> data;
  int bit_pos{0};
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
