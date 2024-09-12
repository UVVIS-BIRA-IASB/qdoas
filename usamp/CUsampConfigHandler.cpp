/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA
*/


#include "CUsampConfigHandler.h"
#include "CPathSubHandler.h"
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

using std::map;

CUsampConfigHandler::CUsampConfigHandler() :
  CConfigHandler()
{
  initializeMediateUsamp(&m_properties);
}

void CUsampConfigHandler::start_subhandler(const Glib::ustring& name,
                                          const map<Glib::ustring, QString>& atts)
{
  if (name == "general") {
    // new General handler
    install_subhandler(new CUsampGeneralSubHandler(this, &m_properties), atts);
  }
  else if (name == "paths") {
    // new Path handler
    install_subhandler(new CPathSubHandler(this), atts);
  }
}

//------------------------------------------------------------------------
//
// Handler for <general> element (and sub elements)

CUsampGeneralSubHandler::CUsampGeneralSubHandler(CConfigHandler *master, mediate_usamp_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CUsampGeneralSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  QString str;

  str = atts.at("type");
  if (str == "ODF") {
    m_d->methodType = OPTICAL_DENSITY_FIT;
  }
  else if (str == "ML+SVD") {
    m_d->methodType = INTENSITY_FIT;
  }
  else
    throw std::runtime_error("Invalid analysis method");

  m_d->shift = atts.at("shift").toDouble();

  m_d->noheader = (atts.at("rmhdr") == "true") ? 1 : 0;

  str = atts.at("outphase1");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->outputPhaseOneFile))
      strcpy(m_d->outputPhaseOneFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Output Phase 1 Filename too long");
  }
  str = atts.at("outphase2");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->outputPhaseTwoFile))
      strcpy(m_d->outputPhaseTwoFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Output Phase 2 Filename too long");
  }
  str = atts.at("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->calibrationFile))
      strcpy(m_d->calibrationFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Calibration Filename too long");
  }
  str = atts.at("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->solarRefFile))
      strcpy(m_d->solarRefFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Solar Reference Filename too long");
  }
}

void CUsampGeneralSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  if (element == "slit_func")
    m_master->install_subhandler(new CSlitFunctionSubHandler(m_master, &(m_d->slit)), atts);
}

