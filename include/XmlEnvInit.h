#ifndef SMARTMET_WFS_XML_ENV_INIT_H__
#define SMARTMET_WFS_XML_ENV_INIT_H__

#include <boost/noncopyable.hpp>

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
class EnvInit : virtual private boost::noncopyable
{
 public:
  EnvInit();

  virtual ~EnvInit();
};

}  // namespace Xml
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif /* SMARTMET_WFS_XML_ENV_INIT_H__ */
