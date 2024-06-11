#include "BStream.h"
#include <boost/bind/bind.hpp>
#include <macgyver/Base64.h>
#include <macgyver/TypeName.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <cstdio>
#include <set>

namespace bw = SmartMet::Plugin::WFS;
namespace ph = boost::placeholders;

using bw::IBStream;
using bw::OBStream;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace
{
const int BLOCK_SIZE = 256;

enum DataTypeInd
{
  V_BOOL = 1,
  V_INT = 2,
  V_UINT = 3,
  V_DOUBLE = 4,
  V_STRING = 5,
  V_PTIME = 6,
  V_POINT = 7,
  V_BBOX = 8
};
}  // namespace

OBStream::OBStream() : reserved(BLOCK_SIZE), data(new uint8_t[BLOCK_SIZE]) 
{
  try
  {
    std::fill_n(data.get(), reserved, 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

OBStream::~OBStream() = default;

std::string OBStream::raw_data() const
{
  try
  {
    std::string tmp(data.get(), data.get() + ind + 1);
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

OBStream::operator std::string() const
{
  try
  {
    return Fmi::Base64::encode(raw_data());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_bit(bool val)
{
  try
  {
    if (num_bits >= 8)
    {
      ind++;
      if (ind >= reserved)
      {
        boost::shared_array<uint8_t> tmp = data;
        data.reset(new uint8_t[reserved + BLOCK_SIZE]);
        std::copy(tmp.get(), tmp.get() + reserved, data.get());
        std::fill_n(data.get() + reserved, BLOCK_SIZE, 0);
        reserved += BLOCK_SIZE;
      }
      num_bits = 0;
    }

    if (val)
    {
      uint8_t tmp = 0x80U >> num_bits;
      data[ind] |= tmp;
    }

    num_bits++;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_bits(unsigned value, int num)
{
  try
  {
    unsigned mask = 1 << num;
    for (int i = 0; i < num; i++)
    {
      mask >>= 1;
      put_bit(value & mask);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_int(int64_t value)
{
  try
  {
    put_bit(value < 0 ? true : false);
    put_unsigned(static_cast<uint64_t>(value >= 0 ? value : -value));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_unsigned(uint64_t value)
{
  try
  {
    while (value)
    {
      unsigned part = value & 0x1F;
      put_bit(value ? true : false);
      put_bits(part, 5);
      value >>= 5;
    }

    put_bit(false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_double(double value)
{
  try
  {
    int first;
    union {
      uint8_t c[8];
      double d;
    } tmp;
    tmp.d = value;

    for (first = 0; (first < 7) and (tmp.c[first] == 0); first++)
    {
    }

    put_bits(first, 3);
    for (int i = first; i <= 7; i++)
    {
      put_bits(tmp.c[i], 8);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_char(char c)
{
  try
  {
    auto u = static_cast<uint8_t>(c);
    if (u > 126)
    {
      put_bits(127U, 7);
      put_bits(u, 8);
    }
    else
    {
      put_bits(u, 7);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_string(const std::string& text)
{
  try
  {
    const std::size_t len = text.length();
    put_unsigned(len);
    const char* tmp = text.c_str();
    for (std::size_t i = 0; i < len; i++)
      put_char(tmp[i]);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_ptime(const Fmi::DateTime& tm)
{
  try
  {
    if (tm.is_special()) {
      put_bit(false);
      if (tm.is_not_a_date_time()) {
	put_bit(false);
      } else {
	put_bit(true);
	if (tm.is_neg_infinity()) {
	  put_bit(false);
	} else if (tm.is_pos_infinity()) {
	  put_bit(true);
	} else {
	  throw Fmi::Exception::Trace(BCP, "Not supported special time value '"
						  + Fmi::date_time::to_simple_string(tm));
	}
      }
    } else {
      // FIXME: it would be more efficient to use duration since epoch in microseconds
      //        Not done currently to avoid change in last number of test results
      const auto d = tm.date();
      const long sec = tm.time_of_day().total_seconds();
      put_bit(true);  // For support of non-time values in the future
      put_int(d.year());
      put_unsigned(d.month());
      put_unsigned(d.day());
      put_bit(sec != 0);
      if (sec)
	{
	  put_unsigned(sec);
	}
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_value(const SmartMet::Spine::Value& value)
{
  try
  {
    const auto& type = value.type();
    if (type == typeid(bool))
    {
      bool tmp = value.get_bool();
      put_bits(V_BOOL, 4);
      put_bit(tmp);
    }
    else if (type == typeid(int64_t))
    {
      int64_t tmp = value.get_int();
      put_bits(V_INT, 4);
      put_int(tmp);
    }
    else if (type == typeid(uint64_t))
    {
      uint64_t tmp = value.get_uint();
      put_bits(V_UINT, 4);
      put_unsigned(tmp);
    }
    else if (type == typeid(double))
    {
      double tmp = value.get_double();
      put_bits(V_DOUBLE, 4);
      put_double(tmp);
    }
    else if (type == typeid(std::string))
    {
      const std::string tmp = value.get_string();
      put_bits(V_STRING, 4);
      put_string(tmp);
    }
    else if (type == typeid(Fmi::DateTime))
    {
      const auto tm = value.get_ptime();
      put_bits(V_PTIME, 4);
      put_ptime(tm);
    }
    else if (type == typeid(SmartMet::Spine::Point))
    {
      const auto p = value.get_point();
      put_bits(V_POINT, 4);
      put_double(p.x);
      put_double(p.y);
      put_string(p.crs);
    }
    else if (type == typeid(SmartMet::Spine::BoundingBox))
    {
      const auto b = value.get_bbox();
      put_bits(V_BBOX, 4);
      put_double(b.xMin);
      put_double(b.yMin);
      put_double(b.xMax);
      put_double(b.yMax);
      put_string(b.crs);
    }
    else
    {
      std::ostringstream msg;
      msg << "Unsupported type '" << Fmi::demangle_cpp_type_name(type.name()) << "'";
      throw Fmi::Exception(BCP, msg.str());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void OBStream::put_value_map(const std::multimap<std::string, SmartMet::Spine::Value>& arg)
{
  try
  {
    std::set<std::string> keys;
    std::transform(arg.begin(),
                   arg.end(),
                   std::inserter(keys, keys.begin()),
                   boost::bind(&std::pair<const std::string, SmartMet::Spine::Value>::first, ph::_1));

    for (const auto& key : keys)
    {
      put_bit(true);
      put_string(key);
      auto range = arg.equal_range(key);
      for (auto it = range.first; it != range.second; ++it)
      {
        put_bit(true);
        put_value(it->second);
      }
      put_bit(false);
    }
    put_bit(false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

IBStream::IBStream(const uint8_t* data, std::size_t length)
    : length(length),  data(new uint8_t[length > 0 ? length : 1]) 
{
  try
  {
    std::copy(data, data + length, this->data.get());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

IBStream::~IBStream() = default;

unsigned IBStream::get_bit()
{
  try
  {
    if (bit_pos >= 8)
    {
      pos++;
      bit_pos = 0;
    }

    if (pos >= length)
    {
      throw Fmi::Exception(BCP, "Unexpected end of bitstream!");
    }

    const uint8_t mask = 0x80 >> bit_pos;
    uint8_t ret_val = (data[pos] & mask) ? 1 : 0;
    bit_pos++;

    return ret_val;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

unsigned IBStream::get_bits(int num)
{
  try
  {
    unsigned result = 0U;
    unsigned mask = 1 << num;
    for (int i = 0; i < num; i++)
    {
      mask >>= 1;
      if (get_bit())
      {
        result |= mask;
      }
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

uint64_t IBStream::get_unsigned()
{
  try
  {
    uint64_t result = 0;
    int off = 0;
    while (get_bit())
    {
      uint64_t part = get_bits(5);
      result |= part << off;
      off += 5;
    }
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

int64_t IBStream::get_int()
{
  try
  {
    unsigned s = get_bit();
    auto tmp = static_cast<int64_t>(get_unsigned());
    return s ? -tmp : tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double IBStream::get_double()
{
  try
  {
    union {
      uint8_t c[8];
      double d;
    } tmp;
    std::fill(tmp.c, tmp.c + 8, 0);
    unsigned first = get_bits(3);
    for (unsigned i = first; i <= 7; i++)
    {
      tmp.c[i] = static_cast<uint8_t>(get_bits(8));
    }

    return tmp.d;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

char IBStream::get_char()
{
  try
  {
    unsigned v1 = get_bits(7);
    if (v1 == 127)
    {
      v1 = get_bits(8);
    }
    char c = static_cast<char>(v1);
    return c;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string IBStream::get_string()
{
  try
  {
    unsigned len = get_unsigned();
    std::string out;
    for (unsigned i = 0; i < len; i++)
      out.push_back(get_char());
    return out;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::DateTime IBStream::get_ptime()
{
  try
  {
    // FIXME: support non time values
    if (get_bit() == 1)
    {
      unsigned sec = 0;
      int year = get_int();
      unsigned month = get_unsigned();
      unsigned day = get_unsigned();
      unsigned have_sec = get_bit();
      if (have_sec)
        sec = get_unsigned();
      Fmi::Date d(year, month, day);
      return Fmi::DateTime(d, Fmi::Seconds(sec));
    }
    else
    {
      if (get_bit()) {
	if (get_bit()) {
            return Fmi::DateTime::POS_INFINITY;
	} else {
            return Fmi::DateTime::NEG_INFINITY;
	}
      } else {
	return Fmi::DateTime::NOT_A_DATE_TIME;
      }
      throw Fmi::Exception(BCP, "Special time values are not yet supported");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

SmartMet::Spine::Value IBStream::get_value()
{
  try
  {
    SmartMet::Spine::Value result;
    unsigned type_code = get_bits(4);
    switch (type_code)
    {
      case V_BOOL:
        result.set_bool(get_bit());
        return result;

      case V_INT:
        result.set_int(get_int());
        return result;

      case V_UINT:
        result.set_uint(get_unsigned());
        return result;

      case V_DOUBLE:
        result.set_double(get_double());
        return result;

      case V_STRING:
        result.set_string(get_string());
        return result;

      case V_PTIME:
        result.set_ptime(get_ptime());
        return result;

      case V_POINT:
      {
        SmartMet::Spine::Point p;
        p.x = get_double();
        p.y = get_double();
        p.crs = get_string();
        result.set_point(p);
      }
        return result;

      case V_BBOX:
      {
        SmartMet::Spine::BoundingBox b;
        b.xMin = get_double();
        b.yMin = get_double();
        b.xMax = get_double();
        b.yMax = get_double();
        b.crs = get_string();
        result.set_bbox(b);
      }
        return result;

      default:
        throw Fmi::Exception(BCP, "Unknown type code in input bitstream!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::multimap<std::string, SmartMet::Spine::Value> IBStream::get_value_map()
{
  try
  {
    std::multimap<std::string, SmartMet::Spine::Value> result;
    while (get_bit())
    {
      const std::string key = get_string();
      while (get_bit())
      {
        SmartMet::Spine::Value value = get_value();
        result.insert(std::make_pair(key, value));
      }
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
