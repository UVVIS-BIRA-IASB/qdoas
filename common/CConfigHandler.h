/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONFIGHANDLER_H_GUARD
#define _CCONFIGHANDLER_H_GUARD

#include <libxml++/libxml++.h>

#include <string>
#include <vector>

class CConfigHandler;

using AttributeList = xmlpp::SaxParser::AttributeList;

// libxml++ API depends on the major version: 5.x uses xmlpp::ustring, 4.x and 2.6 use Glib::ustring:
#if defined(LIBXMLXX_5)
#define XMLSTRING xmlpp::ustring
#else
#define XMLSTRING Glib::ustring
#endif
using xmlstring = XMLSTRING;

class CConfigSubHandler
{
 public:
  CConfigSubHandler(CConfigHandler *master) : m_master(master) {};
  virtual ~CConfigSubHandler() {};

  virtual void start([[maybe_unused]] const std::map<xmlstring, std::string>& atts) {};
  virtual void start([[maybe_unused]] const xmlstring& element, [[maybe_unused]] const std::map<xmlstring, std::string>& atts) {};
  virtual void character([[maybe_unused]] const std::string &ch) {};
  virtual void end([[maybe_unused]] const xmlstring &element) {};
  virtual void end() {};

  virtual CConfigHandler* master() { return m_master; };

 protected:
  CConfigHandler *m_master;
};

struct SSubHandlerItem
{
  CConfigSubHandler *handler;
  size_t depth;

  SSubHandlerItem(CConfigSubHandler *h, size_t d) : handler(h), depth(d) {}
};

class CConfigHandler : public xmlpp::SaxParser
{
 public:
  CConfigHandler();
  virtual ~CConfigHandler();

  // content handling
  virtual std::string errorString() const;
  void install_subhandler(CConfigSubHandler *newHandler, const std::map<xmlstring, std::string>& attributes);

  void setPath(int index, const std::string &pathPrefix);
  std::string getPath(int index) const;
  std::string pathExpand(const std::string &name);

  std::string messages(void) const; // messages collected during parsing

 protected:
  virtual void start_subhandler(const xmlstring& name,
                                const std::map<xmlstring, std::string>& attributes) = 0;

  //overrides:
  void on_start_element(const xmlstring& name,
                        const AttributeList& properties) override;
  void on_end_element(const xmlstring& name) override;
  void on_characters(const xmlstring& characters) override;

 private:
  friend class CConfigSubHandler;

  void setSubErrorMessage(const std::string &msg);

  std::vector<xmlstring> element_stack;
  CConfigSubHandler *m_activeSubHandler;
  std::vector<SSubHandlerItem> m_subHandlerStack;
  std::vector<std::string> m_paths;
  std::string m_subErrorMessage;
  std::string m_errorMessages;
  xmlstring collated_str;
};

inline void CConfigHandler::setSubErrorMessage(const std::string &msg) { m_subErrorMessage = msg; }

inline std::string value(const std::map<xmlstring, std::string>& attributes, const xmlstring& key) {
  auto i_val = attributes.find(key);
  if (i_val != attributes.end()) {
    return i_val->second;
  } else {
    return "";
  }
}

template<typename T>
inline T parse_str(const std::string& val);

template<>
inline double parse_str(const std::string& val) {
  return stod(val);
}

template<>
inline int parse_str(const std::string& val) {
  return stoi(val);
}

template<>
inline unsigned long parse_str(const std::string& val) {
  return stoul(val);
}

// Look up an attribute and parse its value as a number. Return a default value if the attribute is missing.
template<typename T>
inline T parse_value(const std::map<xmlstring, std::string>& attributes, const xmlstring& key, T default_value=0) {
  auto i_val = attributes.find(key);
  if (i_val != attributes.end()) {
    return parse_str<T>(i_val->second);
  } else {
    return default_value;
  }
}

#endif
