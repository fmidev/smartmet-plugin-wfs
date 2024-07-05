#pragma once

#include <memory>

namespace SmartMet
{
  namespace Plugin
  {
    namespace WFS
    {

      class StoredQueryConfig;

      using StoredQueryConfigPtr = std::shared_ptr<StoredQueryConfig>;

      class StoredQueryConfigWrapper
      {
      public:
        StoredQueryConfigWrapper(StoredQueryConfigPtr config_p) : config_p(config_p) {}
          virtual ~StoredQueryConfigWrapper();
        StoredQueryConfigPtr get_config() const { return config_p; }
      private:
        StoredQueryConfigPtr config_p;
      };

    }  // namespace WFS
  }  // namespace Plugin
}  // namespace SmartMet
