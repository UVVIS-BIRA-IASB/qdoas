/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONFIGHANDLER_H_GUARD
#define _CCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QList>
#include <QString>
#include <QVector>

class CConfigHandler;

class CConfigSubHandler
{
 public:
  CConfigSubHandler(CConfigHandler *master) : m_master(master) {};
  virtual ~CConfigSubHandler() {};

  virtual bool start(const QXmlAttributes &atts) { return true; };
  virtual bool start(const QString &element, const QXmlAttributes &atts) { return true; };
  virtual bool character(const QString &ch) { return true; };
  virtual bool end(const QString &element) { return true; };
  virtual bool end() {return true; };

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

class CConfigHandler : public QXmlDefaultHandler
{
 public:
  CConfigHandler();
  virtual ~CConfigHandler();

  // content handling
  virtual bool characters(const QString &ch);
  virtual bool endDocument();
  virtual bool endElement(const QString &namespaceURI, const QString &localName,
              const QString &qName);
  virtual QString errorString() const;
  virtual bool ignorableWhitespace(const QString &ch);
  virtual bool startDocument();
  virtual bool startElement(const QString &namespaceURI, const QString &localName,
                const QString &qName, const QXmlAttributes &atts);

  bool installSubHandler(CConfigSubHandler *newHandler, const QXmlAttributes &atts);

  void setPath(int index, const QString &pathPrefix);
  QString getPath(int index) const;
  QString pathExpand(const QString &name);

  QString messages(void) const; // messages collected during parsing

 protected:
  bool delegateStartElement(const QString &qName, const QXmlAttributes &atts, bool &result);

 private:
  friend class CConfigSubHandler;

  void setSubErrorMessage(const QString &msg);

 private:
  QList<QString> m_elementStack;
  CConfigSubHandler *m_activeSubHandler;
  QList<SSubHandlerItem> m_subHandlerStack;
  QVector<QString> m_paths;
  QString m_subErrorMessage;
  QString m_errorMessages;
  QString m_collatedStr;
};

inline void CConfigHandler::setSubErrorMessage(const QString &msg) { m_subErrorMessage = msg; }

#endif
