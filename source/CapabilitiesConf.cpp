#include "CapabilitiesConf.h"
#include <set>
#include <spine/Convenience.h>
#include <spine/Exception.h>
#include <boost/algorithm/string.hpp>

using SmartMet::Spine::Exception;
using SmartMet::Plugin::WFS::CapabilitiesConf;

namespace
{
  libconfig::Setting*
  get_setting(libconfig::Setting* parent, const std::string& name)
  {
    if (parent->exists(name)) {
      return &(*parent)[name];
    } else {
      return nullptr;
    }
  }

  void checkSettings(const std::string& prefix, libconfig::Setting* parent,
		     const std::set<std::string>& names,
		     std::vector<std::string>& unknown)
  {
    if (parent->isGroup()) {
      for (int i = 0; i < int(parent->getLength()); i++) {
	const std::string name = (*parent)[i].getName();
	if (names.count(name) == 0) {
	  unknown.push_back(prefix + name);
	}
      }
    } else {
      throw SmartMet::Spine::Exception::Trace(BCP, "Setting is not a group")
	.addParameter("name", parent->getName());
    }
  }

  SmartMet::Plugin::WFS::MultiLanguageStringP
  get_ml_string(libconfig::Setting* parent,
		const std::string& name,
		const std::string& default_language)
  {
    try {
      SmartMet::Plugin::WFS::MultiLanguageStringP result;
      libconfig::Setting* setting = get_setting(parent, name);
      if (setting) {
	result = SmartMet::Plugin::WFS::MultiLanguageString::create(default_language, *setting);
      }
      return result;
    } catch (...) {
      throw SmartMet::Spine::Exception(BCP, "Failed to parse SmartMet::Plugin::WFS::MultiLanguageString")
	.addParameter("name", name);
    }
  }

  SmartMet::Plugin::WFS::MultiLanguageStringArray::Ptr
  get_ml_string_array(libconfig::Setting* parent,
		      const std::string& name,
		      const std::string& default_language)
  {
    try {
      SmartMet::Plugin::WFS::MultiLanguageStringArray::Ptr result;
      libconfig::Setting* setting = get_setting(parent, name);
      if (setting) {
	result = SmartMet::Plugin::WFS::MultiLanguageStringArray::create(default_language, *setting);
      }
      return result;
    } catch (...) {
      throw SmartMet::Spine::Exception(BCP, "Failed to parse SmartMet::Plugin::WFS::MultiLanguageStringArray")
	.addParameter("name", name);
    }
  }

  void put(CTPP::CDT& dest, const std::string& name, SmartMet::Plugin::WFS::MultiLanguageStringP value,
	   const std::string& language)
  {
    if (value) {
      dest[name] = value->get(language);
    }
  }

  void put(CTPP::CDT& dest, const std::string& name, SmartMet::Plugin::WFS::MultiLanguageStringArray::Ptr value,
	   const std::string& language)
  {
    if (value) {
      int i = 0;
      CTPP::CDT& f = dest[name];
      for (const auto& item : value->get(language)) {
	f[i++] = item;
      }
    }
  }

} // anonymous namespace

CapabilitiesConf::CapabilitiesConf()
{
}

CapabilitiesConf::~CapabilitiesConf()
{
}

void CapabilitiesConf::parse(const std::string& default_language, libconfig::Setting& setting)
{
  try
  {
    std::vector<std::string> unknown;
    checkSettings("", &setting, {"title", "abstract", "keywords", "fees", "providerName",
	  "providerSite", "phone", "address", "accessConstraints", "onlineResource",
	  "contactInstructions"}, unknown);
    title = get_ml_string(&setting, "title", default_language);
    abstract = get_ml_string(&setting, "abstract", default_language);
    keywords = get_ml_string_array(&setting, "keywords", default_language);
    fees = get_ml_string(&setting, "fees", default_language);
    providerName = get_ml_string(&setting, "providerName", default_language);
    providerSite = get_ml_string(&setting, "providerSite", default_language);
    auto* s_phone = get_setting(&setting, "phone");
    if (s_phone) {
      std::shared_ptr<Phone> tmp_phone(new Phone);
      checkSettings("phone.", s_phone, {"voice", "fax"}, unknown);
      tmp_phone->voice = get_ml_string(s_phone, "voice", default_language);
      tmp_phone->fax = get_ml_string(s_phone, "fax", default_language);
      phone = tmp_phone;
    }
    auto* s_address = get_setting(&setting, "address");
    if (s_address) {
      std::shared_ptr<Address> tmp_addr(new Address);
      checkSettings("address.", s_address, {"deliveryPoint", "city", "area", "postalCode",
	    "country", "email"}, unknown);
      tmp_addr->deliveryPoint = get_ml_string(s_address, "deliveryPoint", default_language);
      tmp_addr->city = get_ml_string(s_address, "city", default_language);
      tmp_addr->area = get_ml_string(s_address, "area", default_language);
      tmp_addr->postalCode = get_ml_string(s_address, "postalCode", default_language);
      tmp_addr->country = get_ml_string(s_address, "country", default_language);
      tmp_addr->email = get_ml_string(s_address, "email", default_language);
      address = tmp_addr;
    } else {
      address.reset();
    }
    accessConstraints = get_ml_string(&setting, "accessConstraints", default_language);
    onlineResource = get_ml_string(&setting, "onlineResource", default_language);
    contactInstructions = get_ml_string(&setting, "contactInstructions", default_language);

    if (not unknown.empty()) {
      std::cout << SmartMet::Spine::log_time_str()
		<< " [WFS][WARNING][Capabilities] Unknown settings "
		<< boost::algorithm::join(unknown, std::string(", "))
		<< std::endl;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

void CapabilitiesConf::apply(CTPP::CDT& hash, const std::string& language) const
{
  put(hash, "title", title, language);
  put(hash, "abstract", abstract, language);
  put(hash, "keywords", keywords, language);
  put(hash, "providerName", providerName, language);
  put(hash, "providerSite", providerSite, language);
  if (phone) {
    CTPP::CDT& p = hash["phone"];
    put(p, "voice", phone->voice, language);
    put(p, "fax", phone->voice, language);
  }
  if (address) {
    CTPP::CDT& f = hash["address"];
    put(f, "deliveryPoint", address->deliveryPoint, language);
    put(f, "city", address->city, language);
    put(f, "area", address->area, language);
    put(f, "postalCode", address->postalCode, language);
    put(f, "country", address->country, language);
    put(f, "email", address->email, language);
  }
  put(hash, "accessConstraints", accessConstraints, language);
  put(hash, "onlineResource", onlineResource, language);
  put(hash, "contactInstructions", contactInstructions, language);
}