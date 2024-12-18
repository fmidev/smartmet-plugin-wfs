#pragma once

#include "XmlError.h"
#include "XmlErrorHandler.h"
#include <boost/function.hpp>
#include <memory>
#include <boost/thread.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <list>
#include <ostream>
#include <stdexcept>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Xml
{
class EntityResolver;

/**
 *   @brief Xerces-C based validating XML DOM parser with XML schema caching feature
 */
class Parser : public xercesc::XercesDOMParser
{
  using inherited = xercesc::XercesDOMParser;

 public:
  struct RootElemInfo
  {
    std::string prefix;
    std::string nqname;
    std::string ns_uri;
    std::map<std::string, std::string> attr_map;
  };

  using root_element_cb_t = std::function<void(RootElemInfo&)>;

 public:
  /**
   *   @brief Constructor for XMLParser object
   *
   *   @param stop_on_error Specifies whether to terminate parsing XML
   *          input on the first error (prefered for production server.
   *          It could however be useful to set it to false for development.
   *   @param grammar_pool XML grammar pool to use
   */
  Parser(bool stop_on_error = true, xercesc::XMLGrammarPool* grammar_pool = nullptr);

  ~Parser() override;

  /**
   *   @brief Parse XML document from provided file
   */
  std::shared_ptr<xercesc::DOMDocument> parse_file(
      const std::string& file_name, root_element_cb_t root_element_cb = root_element_cb_t());

  /**
   *   @brief Parse XML document from provided string
   */
  std::shared_ptr<xercesc::DOMDocument> parse_string(
      const std::string& xml_data,
      const std::string& doc_id = "XML",
      root_element_cb_t root_element_cb = root_element_cb_t());

  std::shared_ptr<xercesc::DOMDocument> parse_input(
      xercesc::InputSource& input, root_element_cb_t root_element_cb = root_element_cb_t());

  std::list<std::string> get_messages() const;

 protected:
  void startElement(const xercesc::XMLElementDecl& elemDecl,
                            const unsigned int uriId,
                            const XMLCh* const prefixName,
                            const xercesc::RefVectorOf<xercesc::XMLAttr>& attrList,
                            const XMLSize_t attrCount,
                            const bool isEmpty,
                            const bool isRoot) override;

 private:
  std::unique_ptr<XmlErrorHandler> error_handler;

  /**
   *  @brief Root element callback for the current parse
   */
  root_element_cb_t root_element_cb;
};

class ParserMT
{
 public:
  ParserMT(const std::string& grammar_pool_file_name, bool stop_on_error = true);
  ParserMT(const ParserMT&) = delete;
  ParserMT& operator = (const ParserMT&) = delete;

  virtual ~ParserMT();

  Parser* get();

  void load_schema_cache(const std::string& file_name);
  void enable_schema_download(const std::string& httpProxy, const std::string& no_proxy);

  void dump_schema_cache(std::ostream& os);

 private:
  const std::string grammar_pool_file_name;
  const bool stop_on_error;
  std::unique_ptr<EntityResolver> entity_resolver;
  std::unique_ptr<xercesc::XMLGrammarPool> grammar_pool;
  boost::thread_specific_ptr<Parser> t_parser;
};

/**
 *  @brief Parses XML document from std::string without validation.
 */
std::shared_ptr<xercesc::DOMDocument> str2xmldom(const std::string& src,
                                                   const std::string& doc_id);

}  // namespace Xml
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
