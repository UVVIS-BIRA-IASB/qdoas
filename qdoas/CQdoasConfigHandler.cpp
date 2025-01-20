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
using std::string;
using std::vector;

void CQdoasConfigHandler::start_subhandler(const Glib::ustring& name,
                                           const map<Glib::ustring, string>& atts) {

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

void CQdoasConfigHandler::addProjectItem(CProjectConfigItem item)
{
  m_projectItemList.push_back(item);
}

const vector<CProjectConfigItem>& CQdoasConfigHandler::projectItems() const
{
  return m_projectItemList;
}

void CQdoasConfigHandler::addSiteItem(CSiteConfigItem item)
{
  m_siteItemList.push_back(item);
}

const vector<CSiteConfigItem>& CQdoasConfigHandler::siteItems() const
{
  return m_siteItemList;
}

void CQdoasConfigHandler::addSymbol(const string &symbolName, const string &symbolDescription)
{
  m_symbolList.emplace_back(symbolName, symbolDescription);
}

const vector<CSymbolConfigItem>& CQdoasConfigHandler::symbolItems() const
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

void CSiteSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, string> &atts)
{
  if (element == "site") {
    // create a new config item for the site
    CSiteConfigItem item;

    string str = value(atts, "name");
    if (str.empty()) {
      throw std::runtime_error("Missing site name");
    }
    else
      item.setSiteName(str);

    str = value(atts, "abbrev");
    if (!str.empty())
      item.setAbbreviation(str);

    item.setLongitude(parse_value<double>(atts, "long"));

    item.setLatitude(parse_value<double>(atts, "lat"));

    item.setAltitude(parse_value<double>(atts, "alt"));

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

void CSymbolSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  if (element == "symbol") {
    string name;

    name = value(atts, "name");
    if (name.empty()) {
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
}

void CProjectSubHandler::start(const map<Glib::ustring, string> &atts)
{
  // the project element - must have a name

  m_project.setName(value(atts, "name"));
  m_project.setEnabled(value(atts, "disable") != "true");

  if (m_project.name().empty()) {
    throw std::runtime_error("Project with empty name.");
  }
}

void CProjectSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  // a sub element of project ... create a specialized handler and delegate
  mediate_project_t *prop = m_project.properties();

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
    return m_master->install_subhandler(new CProjectRawSpectraSubHandler(m_master, m_project.rootNode()), atts);
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
    CAnalysisWindowConfigItem *awItem = m_project.issueNewAnalysisWindowItem();
    if (awItem)
      return m_master->install_subhandler(new CAnalysisWindowSubHandler(m_master, awItem), atts);
  }
}

void CProjectSubHandler::end()
{
  // end of project ... hand project data over to the master handler
  master()->addProjectItem(m_project);
}
