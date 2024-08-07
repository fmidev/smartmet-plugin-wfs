#pragma once

#include "PluginImpl.h"
#include "QueryBase.h"
#include "RequestBase.h"
#include "StandardPresentationParameters.h"
#include <macgyver/TimedCache.h>
#include <xercesc/dom/DOMDocument.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Request
{
/**
 *   @brief Represents GetFeature request
 */
class GetFeature : public RequestBase
{
 private:
  GetFeature(const std::string& language, PluginImpl& plugin_impl);

 public:
  ~GetFeature() override;

  RequestType get_type() const override;

  void execute(std::ostream& ost) const override;

  int get_response_expires_seconds() const override;

  static std::shared_ptr<GetFeature> create_from_kvp(
      const std::string& language,
      const SmartMet::Spine::HTTP::Request& http_request,
      PluginImpl& plugin_impl);

  static std::shared_ptr<GetFeature> create_from_xml(const std::string& language,
                                                       const xercesc::DOMDocument& document,
                                                       PluginImpl& plugin_impl);

 private:
  bool get_cached_responses();

  void execute_single_query(std::ostream& ost) const;

  void execute_multiple_queries(std::ostream& ost) const;

  std::shared_ptr<xercesc::DOMDocument> create_hits_only_response(const std::string& src) const;

  /**
   *   @brief Collects responses of all queries from the GetFeature request as strings
   *
   *   @param query_responses A vector where to put the response strings
   *   @param handle_errors Specifies whether to handle C++ exceptions or simply rethrow
   *   @retval true at least one query succeeded
   *   @retval false none of queries suceeded
   */
  bool collect_query_responses(std::vector<std::string>& query_responses,
                               bool handle_errors = true) const;

  /**
   *   @brief Verifies that the default output format is being used or throw and exception otherwise
   */
  void assert_use_default_format() const;

 private:
  std::vector<std::shared_ptr<QueryBase> > queries;
  StandardPresentationParameters spp;
  QueryResponseCache& query_cache;
  bool fast;
};

}  // namespace Request
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
