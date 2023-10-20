/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CRINGCONFIGHANDLER_H_GUARD
#define _CRINGCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QString>

#include "CConfigHandler.h"

#include "mediate_ring.h"

//------------------------------------------------------------------

class CRingConfigHandler : public CConfigHandler
{
 public:
  CRingConfigHandler();

  virtual bool startElement(const QString &namespaceURI, const QString &localName,
                const QString &qName, const QXmlAttributes &atts);

  const mediate_ring_t* properties(void) const;

 private:
  mediate_ring_t m_properties;
};

inline const mediate_ring_t* CRingConfigHandler::properties(void) const { return &m_properties; }

//------------------------------------------------------------------

class CRingGeneralSubHandler : public CConfigSubHandler
{
 public:
  CRingGeneralSubHandler(CConfigHandler *master, mediate_ring_t *d);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_ring_t *m_d;
};

#endif
