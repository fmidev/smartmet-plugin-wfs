#pragma once

#include <memory>
#include <macgyver/WorkerPool.h>
#include <macgyver/PostgreSQLConnection.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class GeoServerDB
{
 public:
  typedef Fmi::Database::PostgreSQLConnection Connection;
  typedef Fmi::Database::PostgreSQLConnectionOptions ConnectionOpt;
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef Fmi::Database::PostgreSQLConnection::Transaction Transaction;

 public:
  GeoServerDB(const std::string& conn_str, std::size_t keep_conn = 5);
  GeoServerDB(const GeoServerDB&) = delete;
  GeoServerDB& operator = (const GeoServerDB&) = delete;

  virtual ~GeoServerDB();

  ConnectionPtr get_conn();

 private:
  ConnectionPtr create_new_conn();

 private:
  ConnectionOpt conn_opt;
  Fmi::WorkerPool<Connection> conn_pool;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
