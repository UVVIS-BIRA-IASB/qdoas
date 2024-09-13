/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CProjectConfigItem.h"
#include "CQdoasConfigHandler.h"
#include "CPathSubHandler.h"
#include "CProjectConfigSubHandlers.h"
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CConfigSubHandlerUtils.h"

#include "debugutil.h"

using std::map;

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

void CQdoasConfigHandler::start_subhandler(const Glib::ustring& name,
                                           const map<Glib::ustring, QString>& atts) {

  if (name == "project") {
    // new Project handler
    install_subhandler(new CProjectSubHandler(this), atts);
  }
  else if (name == "paths") {
    // new Path handler
    install_subhandler(new CPathSubHandler(this), atts);
  }
  else if (name == "sites") {
    // new Site handler
    install_subhandler(new CSiteSubHandler(this), atts);
  }
  else if (name == "symbols") {
    // new symbol handler
    install_subhandler(new CSymbolSubHandler(this), atts);
  }
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

void CSiteSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, QString> &atts)
{
  if (element == "site") {
    QString str;
    bool ok;
    double tmpDouble;

    // create a new config item for the site
    CSiteConfigItem *item = new CSiteConfigItem;

    str = value(atts, "name");
    if (str.isEmpty()) {
      delete item;
      throw std::runtime_error("Missing site name");
    }
    else
      item->setSiteName(str);

    str = value(atts, "abbrev");
    if (!str.isEmpty())
      item->setAbbreviation(str);

    tmpDouble = value(atts, "long").toDouble(&ok);
    if (ok)
      item->setLongitude(tmpDouble);

    tmpDouble = value(atts, "lat").toDouble(&ok);
    if (ok)
      item->setLatitude(tmpDouble);

    tmpDouble = value(atts, "alt").toDouble(&ok);
    if (ok)
      item->setAltitude(tmpDouble);

    master()->addSiteItem(item);

    return;
  }
  throw std::runtime_error("Incorrect element name " + element);
}

//------------------------------------------------------------------------
//
// Handler for <symbol> element

CSymbolSubHandler::CSymbolSubHandler(CQdoasConfigHandler *master) :
  CQdoasConfigSubHandler(master)
{
}

void CSymbolSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  if (element == "symbol") {
    QString name;

    name = value(atts, "name");
    if (name.isEmpty()) {
      throw std::runtime_error("Missing symbol name");
    }

    master()->addSymbol(name, value(atts, "descr"));

    return;
  }

  throw std::runtime_error("Incorrect element name " + element);
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

void CProjectSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  // the project element - must have a name

  m_project->setName(value(atts, "name"));
  m_project->setEnabled(value(atts, "disable") != "true");

  if (m_project->name().isEmpty()) {
    throw std::runtime_error("Project with empty name.");
  }
}

void CProjectSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  // a sub element of project ... create a specialized handler and delegate
  mediate_project_t *prop = m_project->properties();

  if (element == "display") {
    return m_master->install_subhandler(new CProjectDisplaySubHandler(m_master, &(prop->display)), atts);
  }
  else if (element == "selection") {
    return m_master->install_subhandler(new CProjectSelectionSubHandler(m_master, &(prop->selection)), atts);
  }
  else if (element == "analysis") {
    return m_master->install_subhandler(new CProjectAnalysisSubHandler(m_master, &(prop->analysis)), atts);
  }
  else if (element == "raw_spectra") {
    return m_master->install_subhandler(new CProjectRawSpectraSubHandler(m_master, m_project->rootNode()), atts);
  }
  else if (element == "lowpass_filter") {
    return m_master->install_subhandler(new CFilteringSubHandler(m_master, &(prop->lowpass)), atts);
  }
  else if (element == "highpass_filter") {
    return m_master->install_subhandler(new CFilteringSubHandler(m_master, &(prop->highpass)), atts);
  }
  else if (element == "calibration") {
    return m_master->install_subhandler(new CProjectCalibrationSubHandler(m_master, &(prop->calibration)), atts);
  }
  else if (element == "undersampling") {
    return m_master->install_subhandler(new CProjectUndersamplingSubHandler(m_master, &(prop->undersampling)), atts);
  }
  else if (element == "instrumental") {
    return m_master->install_subhandler(new CProjectInstrumentalSubHandler(m_master, &(prop->instrumental)), atts);
  }
  else if (element == "slit") {
    return m_master->install_subhandler(new CProjectSlitSubHandler(m_master, &(prop->slit)), atts);
  }
  else if (element == "output") {
    return m_master->install_subhandler(new CProjectOutputSubHandler(m_master, &(prop->output)), atts);
  }
  else if (element == "export") {
    return m_master->install_subhandler(new CProjectExportSubHandler(m_master, &(prop->export_spectra)), atts);
  }
  else if (element == "analysis_window") {
    // allocate a new item in the project for this AW
    CAnalysisWindowConfigItem *awItem = m_project->issueNewAnalysisWindowItem();
    if (awItem)
      return m_master->install_subhandler(new CAnalysisWindowSubHandler(m_master, awItem), atts);
  }
}

void CProjectSubHandler::end()
{
  // end of project ... hand project data over to the master handler

  master()->addProjectItem(m_project);
  m_project = NULL; // releases ownership responsibility
}
