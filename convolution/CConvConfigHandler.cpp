/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <map>

#include "CConvConfigHandler.h"

#include "CPathSubHandler.h"
#include "CConfigSubHandlerUtils.h"
#include "constants.h"
#include "debugutil.h"

using std::map;
using std::string;

CConvConfigHandler::CConvConfigHandler() :
  CConfigHandler()
{
  initializeMediateConvolution(&m_properties);
}

CConvConfigHandler::~CConvConfigHandler()
{
}

void CConvConfigHandler::start_subhandler(const Glib::ustring& name,
                                          const map<Glib::ustring, string>& atts) {
  if (name == "general") {
    // new General handler
    install_subhandler(new CConvGeneralSubHandler(this, &(m_properties.general)), atts);
  }
  else if (name == "paths") {
    // new Path handler
    install_subhandler(new CPathSubHandler(this), atts);
  }
  else if (name == "lowpass_filter") {
    install_subhandler(new CFilteringSubHandler(this, &(m_properties.lowpass)), atts);
  }
  else if (name == "highpass_filter") {
    install_subhandler(new CFilteringSubHandler(this, &(m_properties.highpass)), atts);
  }
  else if (name == "con_slit") {
    install_subhandler(new CConvSlitSubHandler(this, &(m_properties.conslit)), atts);
  }
  else if (name == "dec_slit") {
    install_subhandler(new CConvSlitSubHandler(this, &(m_properties.decslit)), atts);
  }
}

//------------------------------------------------------------------------
//
// Handler for <general> element (and sub elements)

CConvGeneralSubHandler::CConvGeneralSubHandler(CConfigHandler *master, mediate_conv_general_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CConvGeneralSubHandler::start(const map<Glib::ustring, string> &atts) {
   // convolution type
  string str = value(atts, "convol");
  if (str == "std")
    m_d->convolutionType = CONVOLUTION_TYPE_STANDARD;
  else if (str == "iocorr")
    m_d->convolutionType = CONVOLUTION_TYPE_I0_CORRECTION;
  else
    m_d->convolutionType = CONVOLUTION_TYPE_NONE;

  // conversion type
  str = value(atts, "conver");
  if (str == "air2vac")
    m_d->conversionType = CONVOLUTION_CONVERSION_AIR2VAC;
  else if (str == "vac2air")
    m_d->conversionType = CONVOLUTION_CONVERSION_VAC2AIR;
  else
    m_d->conversionType = CONVOLUTION_CONVERSION_NONE;

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

  m_d->shift = stod(value(atts, "shift"));
  m_d->conc = stod(value(atts, "conc"));

  m_d->noheader = (value(atts, "rmhdr") == "true") ? 1 : 0;

  str = value(atts, "input");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->inputFile))
      strcpy(m_d->inputFile, str.c_str());
    else
      throw std::runtime_error("Input Filename too long");
  }
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

//------------------------------------------------------------------------
//
// Handler for <con_slit> and <dec_slit> elements.

CConvSlitSubHandler::CConvSlitSubHandler(CConfigHandler *master, mediate_slit_function_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CConvSlitSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, string>& atts)
{
  if (element == "slit_func")
    return m_master->install_subhandler(new CSlitFunctionSubHandler(m_master, m_d), atts);

}
