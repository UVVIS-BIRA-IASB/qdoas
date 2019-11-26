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


#include "CProjectConfigItem.h"
#include "CQdoasConfigHandler.h"
#include "CPathSubHandler.h"
#include "CProjectConfigSubHandlers.h"
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CConfigSubHandlerUtils.h"

#include "debugutil.h"

CQdoasConfigHandler::~CQdoasConfigHandler()
{
  while (!m_projectItemList.isEmpty()) {
    delete m_projectItemList.takeFirst();
  }

  while (!m_siteItemList.isEmpty()) {
    delete m_siteItemList.takeFirst();
  }

  while (!m_symbolList.isEmpty()) {
    delete m_symbolList.takeFirst();
  }
}

bool CQdoasConfigHandler::startElement(const QString &namespaceURI, const QString &localName,
				       const QString &qName, const QXmlAttributes &atts)
{
  bool result;

  if (delegateStartElement(qName, atts, result)) {
    // handled by sub handler ...
    return result;
  }
  else {
    // a sub handler is not active ...

    if (qName == "project") {
      // new Project handler
      return installSubHandler(new CProjectSubHandler(this), atts);
    }
    else if (qName == "paths") {
      // new Path handler
      return installSubHandler(new CPathSubHandler(this), atts);
    }
    else if (qName == "sites") {
      // new Site handler
      return installSubHandler(new CSiteSubHandler(this), atts);
    }
    else if (qName == "symbols") {
      // new symbol handler
      return installSubHandler(new CSymbolSubHandler(this), atts);
    }
  }

  return true;
}

void CQdoasConfigHandler::addProjectItem(CProjectConfigItem *item)
{
  m_projectItemList.push_back(item);
}

QList<const CProjectConfigItem*> CQdoasConfigHandler::projectItems(void) const
{
  return m_projectItemList;
}

QList<const CProjectConfigItem*> CQdoasConfigHandler::takeProjectItems(void)
{
  QList<const CProjectConfigItem*> items = m_projectItemList; // copy the pointers...

  m_projectItemList.clear(); // responsibility was passed to 'items' list

  return items;
}

void CQdoasConfigHandler::addSiteItem(CSiteConfigItem *item)
{
  m_siteItemList.push_back(item);
}

QList<const CSiteConfigItem*> CQdoasConfigHandler::siteItems(void) const
{
  return m_siteItemList;
}

void CQdoasConfigHandler::addSymbol(const QString &symbolName, const QString &symbolDescription)
{
  m_symbolList.push_back(new CSymbolConfigItem(symbolName, symbolDescription));
}

QList<const CSymbolConfigItem*> CQdoasConfigHandler::symbolItems(void) const
{
  return m_symbolList;
}

//------------------------------------------------------------------------
//
// Handler for <sites> element (and sub elements)

CSiteSubHandler::CSiteSubHandler(CQdoasConfigHandler *master) :
  CQdoasConfigSubHandler(master)
{
}

bool CSiteSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "site") {
    QString str;
    bool ok;
    double tmpDouble;

    // create a new config item for the site
    CSiteConfigItem *item = new CSiteConfigItem;

    str = atts.value("name");
    if (str.isEmpty()) {
      delete item;
      return postErrorMessage("Missing site name");
    }
    else
      item->setSiteName(str);

    str = atts.value("abbrev");
    if (!str.isEmpty())
      item->setAbbreviation(str);

    tmpDouble = atts.value("long").toDouble(&ok);
    if (ok)
      item->setLongitude(tmpDouble);

    tmpDouble = atts.value("lat").toDouble(&ok);
    if (ok)
      item->setLatitude(tmpDouble);

    tmpDouble = atts.value("alt").toDouble(&ok);
    if (ok)
      item->setAltitude(tmpDouble);

    master()->addSiteItem(item);

    return true;
  }

  return false;
}

//------------------------------------------------------------------------
//
// Handler for <symbol> element

CSymbolSubHandler::CSymbolSubHandler(CQdoasConfigHandler *master) :
  CQdoasConfigSubHandler(master)
{
}

bool CSymbolSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "symbol") {
    QString name;

    name = atts.value("name");
    if (name.isEmpty()) {
      return postErrorMessage("Missing symbol name");
    }

    master()->addSymbol(name, atts.value("descr"));

    return true;
  }

  return false;
}

//------------------------------------------------------------------------
//
// Handler for <project> element


CProjectSubHandler::CProjectSubHandler(CQdoasConfigHandler *master) :
  CQdoasConfigSubHandler(master)
{
  m_project = new CProjectConfigItem;
}

CProjectSubHandler::~CProjectSubHandler()
{
  delete m_project;
}

bool CProjectSubHandler::start(const QXmlAttributes &atts)
{
  // the project element - must have a name

  m_project->setName(atts.value("name"));
  m_project->setEnabled(atts.value("disable") != "true");

  return !m_project->name().isEmpty();
}

bool CProjectSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // a sub element of project ... create a specialized handler and delegate
  mediate_project_t *prop = m_project->properties();

  if (element == "display") {
    return m_master->installSubHandler(new CProjectDisplaySubHandler(m_master, &(prop->display)), atts);
  }
  else if (element == "selection") {
    return m_master->installSubHandler(new CProjectSelectionSubHandler(m_master, &(prop->selection)), atts);
  }
  else if (element == "analysis") {
    return m_master->installSubHandler(new CProjectAnalysisSubHandler(m_master, &(prop->analysis)), atts);
  }
  else if (element == "raw_spectra") {
    return m_master->installSubHandler(new CProjectRawSpectraSubHandler(m_master, m_project->rootNode()), atts);
  }
  else if (element == "lowpass_filter") {
    return m_master->installSubHandler(new CFilteringSubHandler(m_master, &(prop->lowpass)), atts);
  }
  else if (element == "highpass_filter") {
    return m_master->installSubHandler(new CFilteringSubHandler(m_master, &(prop->highpass)), atts);
  }
  else if (element == "calibration") {
    return m_master->installSubHandler(new CProjectCalibrationSubHandler(m_master, &(prop->calibration)), atts);
  }
  else if (element == "undersampling") {
    return m_master->installSubHandler(new CProjectUndersamplingSubHandler(m_master, &(prop->undersampling)), atts);
  }
  else if (element == "instrumental") {
    return m_master->installSubHandler(new CProjectInstrumentalSubHandler(m_master, &(prop->instrumental)), atts);
  }
  else if (element == "slit") {
    return m_master->installSubHandler(new CProjectSlitSubHandler(m_master, &(prop->slit)), atts);
  }
  else if (element == "output") {
    return m_master->installSubHandler(new CProjectOutputSubHandler(m_master, &(prop->output)), atts);
  }
  else if (element == "export") {
    return m_master->installSubHandler(new CProjectExportSubHandler(m_master, &(prop->export_spectra)), atts);
  }
  else if (element == "analysis_window") {
    // allocate a new item in the project for this AW
    CAnalysisWindowConfigItem *awItem = m_project->issueNewAnalysisWindowItem();
    if (awItem)
      return m_master->installSubHandler(new CAnalysisWindowSubHandler(m_master, awItem), atts);

    return false; // fall through failure
  }

  return true; // TODO - false unknown element ... or just ignore...
}

bool CProjectSubHandler::end()
{
  // end of project ... hand project data over to the master handler

  master()->addProjectItem(m_project);
  m_project = NULL; // releases ownership responsibility

  return true;
}
