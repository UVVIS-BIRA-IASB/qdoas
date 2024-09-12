/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONFIGHANDLER_H_GUARD
#define _CCONFIGHANDLER_H_GUARD

#include <libxml++/libxml++.h>

#include <iostream>

#include <QList>
#include <QString>
#include <QVector>

class CConfigHandler;

using AttributeList = xmlpp::SaxParser::AttributeList;

class CConfigSubHandler
{
 public:
  CConfigSubHandler(CConfigHandler *master) : m_master(master) {};
  virtual ~CConfigSubHandler() {};

  virtual void start(const Glib::ustring& name, const AttributeList& atts) {};
  virtual void start(const std::map<Glib::ustring, QString>& atts) {};
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString>& atts) {};
  virtual void character(const QString &ch) {};
  virtual void end(const Glib::ustring &element) {};
  virtual void end() {};

  virtual CConfigHandler* master() { return m_master; };

 protected:
  CConfigHandler *m_master;

  bool postErrorMessage(const QString &msg); // always returns false
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
  virtual QString errorString() const;
  virtual bool ignorableWhitespace(const QString &ch);
  void install_subhandler(CConfigSubHandler *newHandler, const std::map<Glib::ustring, QString>& attributes);

  void setPath(int index, const QString &pathPrefix);
  QString getPath(int index) const;
  QString pathExpand(const QString &name);

  QString messages(void) const; // messages collected during parsing

 protected:
  virtual void start_subhandler(const Glib::ustring& name,
                                const std::map<Glib::ustring, QString>& attributes) = 0;

  //overrides:
  void on_start_element(const Glib::ustring& name,
                        const AttributeList& properties) override;
  void on_end_element(const Glib::ustring& name) override;
  void on_characters(const Glib::ustring& characters) override;

 private:
  friend class CConfigSubHandler;

  void setSubErrorMessage(const QString &msg);

  std::vector<Glib::ustring> element_stack;
  CConfigSubHandler *m_activeSubHandler;
  QList<SSubHandlerItem> m_subHandlerStack;
  QVector<QString> m_paths;
  QString m_subErrorMessage;
  QString m_errorMessages;
  Glib::ustring collated_str;
};

inline void CConfigHandler::setSubErrorMessage(const QString &msg) { m_subErrorMessage = msg; }

#endif
