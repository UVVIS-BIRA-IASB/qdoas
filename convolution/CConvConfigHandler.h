/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONVCONFIGHANDLER_H_GUARD
#define _CCONVCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QString>

#include "CConfigHandler.h"

#include "mediate_convolution.h"

//------------------------------------------------------------------

class CConvConfigHandler : public CConfigHandler
{
 public:
  CConvConfigHandler();
  virtual ~CConvConfigHandler();

  virtual bool startElement(const QString &namespaceURI, const QString &localName,
                const QString &qName, const QXmlAttributes &atts);

  const mediate_convolution_t* properties(void) const;

 private:
  mediate_convolution_t m_properties;
};

inline const mediate_convolution_t* CConvConfigHandler::properties(void) const { return &m_properties; }

//------------------------------------------------------------------

class CConvGeneralSubHandler : public CConfigSubHandler
{
 public:
  CConvGeneralSubHandler(CConfigHandler *master, mediate_conv_general_t *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_conv_general_t *m_d;
};

//------------------------------------------------------------------

class CConvSlitSubHandler : public CConfigSubHandler
{
 public:
  CConvSlitSubHandler(CConfigHandler *master, mediate_slit_function_t *d);

  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_slit_function_t *m_d;
};

#endif
