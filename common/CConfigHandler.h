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

class CConfigSubHandler
{
 public:
  CConfigSubHandler(CConfigHandler *master) : m_master(master) {};
  virtual ~CConfigSubHandler() {};

  virtual void start(const Glib::ustring& name, const AttributeList& atts) {};
  virtual void start(const std::map<Glib::ustring, std::string>& atts) {};
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, std::string>& atts) {};
  virtual void character(const std::string &ch) {};
  virtual void end(const Glib::ustring &element) {};
  virtual void end() {};

  virtual CConfigHandler* master() { return m_master; };

 protected:
  CConfigHandler *m_master;
};

struct SSubHandlerItem
{
  CConfigSubHandler *handler;
  int depth;

  SSubHandlerItem(CConfigSubHandler *h, int d) : handler(h), depth(d) {}
};

class CConfigHandler : public xmlpp::SaxParser
{
 public:
  CConfigHandler();
  virtual ~CConfigHandler();

  // content handling
  virtual std::string errorString() const;
  void install_subhandler(CConfigSubHandler *newHandler, const std::map<Glib::ustring, std::string>& attributes);

  void setPath(int index, const std::string &pathPrefix);
  std::string getPath(int index) const;
  std::string pathExpand(const std::string &name);

  std::string messages(void) const; // messages collected during parsing

 protected:
  virtual void start_subhandler(const Glib::ustring& name,
                                const std::map<Glib::ustring, std::string>& attributes) = 0;

  //overrides:
  void on_start_element(const Glib::ustring& name,
                        const AttributeList& properties) override;
  void on_end_element(const Glib::ustring& name) override;
  void on_characters(const Glib::ustring& characters) override;

 private:
  friend class CConfigSubHandler;

  void setSubErrorMessage(const std::string &msg);

  std::vector<Glib::ustring> element_stack;
  CConfigSubHandler *m_activeSubHandler;
  std::vector<SSubHandlerItem> m_subHandlerStack;
  std::vector<std::string> m_paths;
  std::string m_subErrorMessage;
  std::string m_errorMessages;
  Glib::ustring collated_str;
};

inline void CConfigHandler::setSubErrorMessage(const std::string &msg) { m_subErrorMessage = msg; }

// Get value from a map of xml attributes, or an empty string.  Mimics the behaviour of QXmlAttributes::value().
inline std::string value(const std::map<Glib::ustring, std::string>& attributes, const Glib::ustring& key) {
  auto i_val = attributes.find(key);
  if (i_val != attributes.end()) {
    return i_val->second;
  } else {
    return "";
  }
};

#endif
