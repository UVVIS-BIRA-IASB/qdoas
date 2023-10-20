/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA
*/


#ifndef _CUSAMPCONFIGHANDLER_H_GUARD
#define _CUSAMPCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QString>

#include "CConfigHandler.h"

#include "mediate_usamp.h"

//------------------------------------------------------------------

class CUsampConfigHandler : public CConfigHandler
{
 public:
  CUsampConfigHandler();

  virtual bool startElement(const QString &namespaceURI, const QString &localName,
                const QString &qName, const QXmlAttributes &atts);

  const mediate_usamp_t* properties(void) const;

 private:
  mediate_usamp_t m_properties;
};

inline const mediate_usamp_t* CUsampConfigHandler::properties(void) const { return &m_properties; }

//------------------------------------------------------------------

class CUsampGeneralSubHandler : public CConfigSubHandler
{
 public:
  CUsampGeneralSubHandler(CConfigHandler *master, mediate_usamp_t *d);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_usamp_t *m_d;
};

#endif
