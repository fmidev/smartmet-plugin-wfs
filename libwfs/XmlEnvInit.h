#pragma once

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Xml
{
/**
 *   @brief Wraps Xerces-C library initialization and deinitialization
 */
class EnvInit
{
 public:
  EnvInit();
  EnvInit(const EnvInit&) = delete;
  EnvInit& operator = (const EnvInit&) = delete;

  virtual ~EnvInit();
};

}  // namespace Xml
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
