#pragma once

#include <future>
#include <mutex>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <optional>
#include <boost/serialization/map.hpp>
#include <boost/serialization/map.hpp>
#include <curl/curl.h>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>

namespace SmartMet {
  namespace Plugin {
    namespace WFS {
      namespace Xml {

	class EntityResolver : public xercesc::XMLEntityResolver
	{
	  struct Download
	  {
	    std::shared_future<std::string> task;
	    int num_failed{0};
	    std::time_t last_failed{0};

            Download()  = default;
	  };

	  std::mutex mutex;
	  std::string proxy;
	  std::string no_proxy;
          std::map<std::string, std::string> cache;
	  std::map<std::string, Download> download_map;
	  bool enable_download{false};

	public:
          EntityResolver();

          ~EntityResolver() override;

	  void init_schema_download(const std::string& http_proxy = "", const std::string& no_proxy = "");

	  bool merge_downloaded_schemas();

	  // SECURITY: schemas may be downloaded only while a DownloadScope is alive in
	  // the calling thread. The plugin opens one when validating its own XML
	  // output. Client requests are parsed without one, so attacker-chosen
	  // xsi:schemaLocation URLs are resolved from the preloaded cache only and
	  // never fetched (blind SSRF, unbounded download_map growth).
	  class DownloadScope
	  {
	  public:
	    DownloadScope();
	    ~DownloadScope();
	    DownloadScope(const DownloadScope&) = delete;
	    DownloadScope& operator=(const DownloadScope&) = delete;

	  private:
	    bool previous;
	  };

          xercesc::InputSource *resolveEntity(xercesc::XMLResourceIdentifier *resource_identifier) override;



          template <class Archive>
          void serialize(Archive &ar, const unsigned int version);

	private:
	  xercesc::InputSource* get_schema(const std::string& remote_uri);

	  bool get_schema(const std::string& remote_uri, std::string& result);

	  std::string download(const std::string& uri) const;
	};


	template <class Archive>
	void EntityResolver::serialize(Archive &ar, const unsigned int version)
	{
	  (void)version;
	  ar &BOOST_SERIALIZATION_NVP(cache);
	}
      } // namespace Xml
    } // namespace WFS
  } // Plugin
} // namespace Xml
