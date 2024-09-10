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

CConvConfigHandler::CConvConfigHandler() :
  CConfigHandler()
{
  initializeMediateConvolution(&m_properties);
}

CConvConfigHandler::~CConvConfigHandler()
{
}

void CConvConfigHandler::start_subhandler(const Glib::ustring& name,
                                          const map<Glib::ustring, QString>& atts) {
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

void CConvGeneralSubHandler::start(const map<Glib::ustring, QString> &atts) {
   // convolution type
  QString str = atts.at("convol");
  if (str == "std")
    m_d->convolutionType = CONVOLUTION_TYPE_STANDARD;
  else if (str == "iocorr")
    m_d->convolutionType = CONVOLUTION_TYPE_I0_CORRECTION;
  else
    m_d->convolutionType = CONVOLUTION_TYPE_NONE;

  // conversion type
  str = atts.at("conver");
  if (str == "air2vac")
    m_d->conversionType = CONVOLUTION_CONVERSION_AIR2VAC;
  else if (str == "vac2air")
    m_d->conversionType = CONVOLUTION_CONVERSION_VAC2AIR;
  else
    m_d->conversionType = CONVOLUTION_CONVERSION_NONE;

  // output format
  str = atts.at("output_format");
  if (str == "ascii")
    m_d->formatType = CONVOLUTION_FORMAT_ASCII;
  else if (str == "netcdf")
    m_d->formatType = CONVOLUTION_FORMAT_NETCDF;
  else
    m_d->formatType = CONVOLUTION_FORMAT_ASCII;

  // number of ground pixels
  str = atts.at("pixels");
  m_d->n_groundpixel=(!str.isEmpty())?str.toInt():1;

  m_d->shift = atts.at("shift").toDouble();
  m_d->conc = atts.at("conc").toDouble();

  m_d->noheader = (atts.at("rmhdr") == "true") ? 1 : 0;

  str = atts.at("input");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->inputFile))
      strcpy(m_d->inputFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Input Filename too long");
  }
  str = atts.at("output");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->outputFile))
      strcpy(m_d->outputFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Output Filename too long");
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

//------------------------------------------------------------------------
//
// Handler for <con_slit> and <dec_slit> elements.

CConvSlitSubHandler::CConvSlitSubHandler(CConfigHandler *master, mediate_slit_function_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CConvSlitSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, QString>& atts)
{
  if (element == "slit_func")
    return m_master->install_subhandler(new CSlitFunctionSubHandler(m_master, m_d), atts);

}
