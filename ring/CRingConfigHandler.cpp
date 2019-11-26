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


#include "CRingConfigHandler.h"
#include "CPathSubHandler.h"
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

CRingConfigHandler::CRingConfigHandler() :
  CConfigHandler()
{
  initializeMediateRing(&m_properties);
}

bool CRingConfigHandler::startElement(const QString &namespaceURI, const QString &localName,
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
      return installSubHandler(new CRingGeneralSubHandler(this, &m_properties), atts);
    }
    else if (qName == "paths") {
      // new Path handler
      return installSubHandler(new CPathSubHandler(this), atts);
    }
  }

  return true;
}

//------------------------------------------------------------------------
//
// Handler for <general> element (and sub elements)

CRingGeneralSubHandler::CRingGeneralSubHandler(CConfigHandler *master, mediate_ring_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CRingGeneralSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  m_d->temperature = atts.value("temp").toDouble();

  str = atts.value("normalize");
  if (str.isEmpty())
   m_d->normalize=1;
  else
   m_d->normalize = (str == "true") ? 1 : 0;

  m_d->noheader = (atts.value("rmhdr") == "true") ? 1 : 0;
  m_d->saveraman = (atts.value("save_raman") == "true") ? 1 : 0;

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

bool CRingGeneralSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "slit_func")
    return m_master->installSubHandler(new CSlitFunctionSubHandler(m_master, &(m_d->slit)), atts);

  return true;
}

