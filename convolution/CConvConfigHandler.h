/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONVCONFIGHANDLER_H_GUARD
#define _CCONVCONFIGHANDLER_H_GUARD

#include <libxml++/libxml++.h>

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

  const mediate_convolution_t* properties(void) const;

protected:
  virtual void start_subhandler(const Glib::ustring& name,
                                const std::map<Glib::ustring, QString>& attributes) override;
 private:
  mediate_convolution_t m_properties;
};

inline const mediate_convolution_t* CConvConfigHandler::properties(void) const { return &m_properties; }

//------------------------------------------------------------------

class CConvGeneralSubHandler : public CConfigSubHandler
{
 public:
  CConvGeneralSubHandler(CConfigHandler *master, mediate_conv_general_t *d);

  virtual void start(const std::map<Glib::ustring, QString>& attributes) override;

 private:
  mediate_conv_general_t *m_d;
};

//------------------------------------------------------------------

class CConvSlitSubHandler : public CConfigSubHandler
{
 public:
  CConvSlitSubHandler(CConfigHandler *master, mediate_slit_function_t *d);

  virtual void start(const Glib::ustring& name,
                     const std::map<Glib::ustring, QString>& attributes) override;

 private:
  mediate_slit_function_t *m_d;
};

#endif
