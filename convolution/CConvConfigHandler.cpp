/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "CConvConfigHandler.h"
#include "CPathSubHandler.h"
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

CConvConfigHandler::CConvConfigHandler() :
  CConfigHandler()
{
  initializeMediateConvolution(&m_properties);
}

CConvConfigHandler::~CConvConfigHandler()
{
}

bool CConvConfigHandler::startElement(const QString &namespaceURI, const QString &localName,
				       const QString &qName, const QXmlAttributes &atts)
{
  bool result;

  if (delegateStartElement(qName, atts, result)) {
    // handled by sub handler ...
    return result;
  }
  else {
    // a sub handler is not active ...

    if (qName == "general") {
      // new General handler
      return installSubHandler(new CConvGeneralSubHandler(this, &(m_properties.general)), atts);
    }
    else if (qName == "paths") {
      // new Path handler
      return installSubHandler(new CPathSubHandler(this), atts);
    }
    else if (qName == "lowpass_filter") {
      return installSubHandler(new CFilteringSubHandler(this, &(m_properties.lowpass)), atts);
    }
    else if (qName == "highpass_filter") {
      return installSubHandler(new CFilteringSubHandler(this, &(m_properties.highpass)), atts);
    }
    else if (qName == "con_slit") {
      return installSubHandler(new CConvSlitSubHandler(this, &(m_properties.conslit)), atts);
    }
    else if (qName == "dec_slit") {
      return installSubHandler(new CConvSlitSubHandler(this, &(m_properties.decslit)), atts);
    }
  }

  return true;
}

//------------------------------------------------------------------------
//
// Handler for <general> element (and sub elements)

CConvGeneralSubHandler::CConvGeneralSubHandler(CConfigHandler *master, mediate_conv_general_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CConvGeneralSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  // convolution type
  str = atts.value("convol");
  if (str == "std")
    m_d->convolutionType = CONVOLUTION_TYPE_STANDARD;
  else if (str == "iocorr")
    m_d->convolutionType = CONVOLUTION_TYPE_I0_CORRECTION;
  else
    m_d->convolutionType = CONVOLUTION_TYPE_NONE;

  // conversion type
  str = atts.value("conver");
  if (str == "air2vac")
    m_d->conversionType = CONVOLUTION_CONVERSION_AIR2VAC;
  else if (str == "vac2air")
    m_d->conversionType = CONVOLUTION_CONVERSION_VAC2AIR;
  else
    m_d->conversionType = CONVOLUTION_CONVERSION_NONE;

  m_d->shift = atts.value("shift").toDouble();
  m_d->conc = atts.value("conc").toDouble();

  m_d->noheader = (atts.value("rmhdr") == "true") ? 1 : 0;

  str = atts.value("input");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->inputFile))
      strcpy(m_d->inputFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Input Filename too long");
  }
  str = atts.value("output");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->outputFile))
      strcpy(m_d->outputFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Output Filename too long");
  }
  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->calibrationFile))
      strcpy(m_d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }
  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->solarRefFile))
      strcpy(m_d->solarRefFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }

  return true;
}

//------------------------------------------------------------------------
//
// Handler for <con_slit> and <dec_slit> elements.

CConvSlitSubHandler::CConvSlitSubHandler(CConfigHandler *master, mediate_slit_function_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CConvSlitSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "slit_func")
    return m_master->installSubHandler(new CSlitFunctionSubHandler(m_master, m_d), atts);

  return true;
}
