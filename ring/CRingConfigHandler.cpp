/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <cstring>

#include "CRingConfigHandler.h"
#include "CPathSubHandler.h"
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

using std::map;
using std::string;

CRingConfigHandler::CRingConfigHandler() :
  CConfigHandler()
{
  initializeMediateRing(&m_properties);
}

void CRingConfigHandler::start_subhandler(const xmlstring& name,
                                          const map<xmlstring, string>& atts)
{
  if (name == "general") {
    // new General handler
    install_subhandler(new CRingGeneralSubHandler(this, &m_properties), atts);
  }
  else if (name == "paths") {
    // new Path handler
    install_subhandler(new CPathSubHandler(this), atts);
  }
}

//------------------------------------------------------------------------
//
// Handler for <general> element (and sub elements)

CRingGeneralSubHandler::CRingGeneralSubHandler(CConfigHandler *master, mediate_ring_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CRingGeneralSubHandler::start(const map<xmlstring, string> &atts)
{
  string str;

  m_d->temperature = stod(value(atts, "temp"));

  str = value(atts, "normalize");
  if (str.empty())
    m_d->normalize=1;
  else
    m_d->normalize = (str == "true") ? 1 : 0;
  m_d->noheader = (value(atts, "rmhdr") == "true") ? 1 : 0;
  m_d->saveraman = (value(atts, "save_raman") == "true") ? 1 : 0;

  // output format
  str = value(atts, "output_format");
  if (str == "ascii")
    m_d->formatType = CONVOLUTION_FORMAT_ASCII;
  else if (str == "netcdf")
    m_d->formatType = CONVOLUTION_FORMAT_NETCDF;
  else
    m_d->formatType = CONVOLUTION_FORMAT_ASCII;

  // number of ground pixels
  str = value(atts, "pixels");
  m_d->n_groundpixel=(!str.empty())?stoi(str):1;

  str = value(atts, "output");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->outputFile))
      strcpy(m_d->outputFile, str.c_str());
    else
      throw std::runtime_error("Output Filename too long");
  }
  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->calibrationFile))
      strcpy(m_d->calibrationFile, str.c_str());
    else
      throw std::runtime_error("Calibration Filename too long");
  }
  str = value(atts, "ref");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->solarRefFile))
      strcpy(m_d->solarRefFile, str.c_str());
    else
      throw std::runtime_error("Solar Reference Filename too long");
  }
}

void CRingGeneralSubHandler::start(const xmlstring &element, const map<xmlstring, string> &atts)
{
  if (element == "slit_func")
    m_master->install_subhandler(new CSlitFunctionSubHandler(m_master, &(m_d->slit)), atts);
}
