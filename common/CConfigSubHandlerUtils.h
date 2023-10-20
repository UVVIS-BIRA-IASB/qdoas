/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCCONFIGSUBHANDLERUTILS_H_GUARD
#define _CCCONFIGSUBHANDLERUTILS_H_GUARD

#include "CConfigHandler.h"
#include "mediate_general.h"

//-------------------------------------------------------------------

class CFilteringSubHandler : public CConfigSubHandler
{
 public:
  CFilteringSubHandler(CConfigHandler *master,
               mediate_filter_t *filter);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_filter_t *m_filter;
};

//-------------------------------------------------------------------

class CSlitFunctionSubHandler : public CConfigSubHandler
{
 public:
  CSlitFunctionSubHandler(CConfigHandler *master,
              mediate_slit_function_t *function);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_slit_function_t *m_function;
};

//-------------------------------------------------------------------

#endif

