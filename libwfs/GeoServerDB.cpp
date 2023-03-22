#include "GeoServerDB.h"
#include <macgyver/Exception.h>

namespace bw = SmartMet::Plugin::WFS;

bw::GeoServerDB::GeoServerDB(const std::string& conn_str, std::size_t  /*keep_conn*/)
    : conn_opt(conn_str)
    , conn_pool( [this]() -> ConnectionPtr { return create_new_conn(); },
		 10, 50, 5)
{
  try
  {
    // Ensure that one can connect
    (void)conn_pool.reserve();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::GeoServerDB::~GeoServerDB() = default;

bw::GeoServerDB::ConnectionPtr bw::GeoServerDB::get_conn()
{
  try
  {
    return conn_pool.reserve();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::GeoServerDB::ConnectionPtr bw::GeoServerDB::create_new_conn()
{
  try
  {
    return std::make_shared<Connection>(conn_opt);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
