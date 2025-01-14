/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <algorithm>
#include <vector>

#include <cstring>

#include "CWorkSpace.h"

#include "debugutil.h"

using std::string;
using std::vector;

CWorkSpace *CWorkSpace::m_instance = NULL;

CWorkSpace* CWorkSpace::instance(void)
{
  if (m_instance == NULL)
    m_instance = new CWorkSpace;

  return m_instance;
}

CWorkSpace::~CWorkSpace()
{
  // free all dynamic memory

  // Project ...
  std::map<string,SProjBucket>::iterator pIt = m_projMap.begin();
  while (pIt != m_projMap.end()) {
    std::vector<SAnlysWinBucket>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      delete wIt->aw; // delete the analysis window data
      ++wIt;
    }
    (pIt->second).window.clear();
    delete (pIt->second).project; // delete the project data
    ++pIt;
  }
  m_projMap.clear(); // happens anyway

  // site
  std::map<string,mediate_site_t*>::iterator sIt = m_siteMap.begin();
  while (sIt != m_siteMap.end()) {
    delete sIt->second;
    ++sIt;
  }
  m_siteMap.clear();

  m_instance = NULL;
}

void CWorkSpace::removeAllContent(void)
{
  // Projects ...  Dont worry about reference count handling because
  // the symbol list will be cleared as well.

  std::map<string,SProjBucket>::iterator pIt = m_projMap.begin();
  while (pIt != m_projMap.end()) {
    // notify the observers
    std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
    while (obs != m_projectObserverList.end()) {
      (*obs)->updateDeleteProject(pIt->first);
      ++obs;
    }

    std::vector<SAnlysWinBucket>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      delete wIt->aw; // delete the analysis window data
      ++wIt;
    }
    (pIt->second).window.clear();

    delete (pIt->second).project; // delete the project data
    ++pIt;
  }
  m_projMap.clear();

  // sites
  std::map<string,mediate_site_t*>::iterator sIt = m_siteMap.begin();
  while (sIt != m_siteMap.end()) {
    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateDeleteSite(sIt->first);
      ++obs;
    }

    delete sIt->second;
    ++sIt;
  }
  m_siteMap.clear();

  // symbols
  symbolmap_t::iterator mIt = m_symbolMap.begin();
  while (mIt != m_symbolMap.end()) {
    // notify the observers
    std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
    while (obs != m_symbolObserverList.end()) {
      (*obs)->updateDeleteSymbol(mIt->first);
      ++obs;
    }
    ++mIt;
  }
  m_symbolMap.clear();

}

mediate_project_t* CWorkSpace::findProject(const string &projectName) const
{
  std::map<string,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end())
    return (pIt->second).project;

  return NULL; // not found
}

mediate_analysis_window_t* CWorkSpace::findAnalysisWindow(const string &projectName,
                              const string &windowName) const
{
  std::map<string,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    std::vector<SAnlysWinBucket>::const_iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end() && windowName != wIt->aw->name) ++wIt;
    if (wIt != (pIt->second).window.end())
      return wIt->aw;
  }

  return NULL; // not found
}

const mediate_site_t* CWorkSpace::findSite(const string &siteName) const
{
  std::map<string,mediate_site_t*>::const_iterator it = m_siteMap.find(siteName);
  if (it != m_siteMap.end()) {
    return it->second;
  }

  return NULL; // not found
}

string CWorkSpace::findSymbol(const string &symbolName) const
{
  symbolmap_t::const_iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end()) {
    // symbol exists
    return (it->second).description; // The string returned may be empty, but is not null.
  }

  return string(); // a NULL string
}

void CWorkSpace::incrementUseCount(const string &symbolName)
{
  symbolmap_t::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end())
    ++((it->second).useCount);
}

void CWorkSpace::decrementUseCount(const string &symbolName)
{
  symbolmap_t::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end() && (it->second).useCount)
    --((it->second).useCount);
}

mediate_project_t* CWorkSpace::createProject(const string &newProjectName)
{
  if (newProjectName.empty())
    return NULL;

  mediate_project_t *tmp = NULL;

  // does it already exist?
  std::map<string,SProjBucket>::iterator pIt = m_projMap.find(newProjectName);
  if (pIt == m_projMap.end()) {
    // does not exist
    tmp = new mediate_project_t;

    initializeMediateProject(tmp, m_configFile.c_str(), newProjectName.c_str() );

    // insert the project (with no windows)
    m_projMap.insert(std::map<string,SProjBucket>::value_type(newProjectName,SProjBucket(tmp)));

    // notify the observers
    std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
    while (obs != m_projectObserverList.end()) {
      (*obs)->updateNewProject(newProjectName);
      ++obs;
    }
  }

  return tmp;
}

mediate_analysis_window_t*  CWorkSpace::createAnalysisWindow(const string &projectName, const string &newWindowName,
                                 const string &preceedingWindowName)
{
  if (newWindowName.empty())
    return NULL;

  mediate_analysis_window_t *tmp = NULL;

  // project must exist
  std::map<string,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    std::vector<SAnlysWinBucket>::iterator nextIt = (pIt->second).window.begin();
    std::vector<SAnlysWinBucket>::iterator wIt = (pIt->second).window.begin();
    // check that the new window does not already exist and locate the preceeding window
    // (and set nextIt to the element after the preceeding window)
    while (wIt != (pIt->second).window.end() && newWindowName != wIt->aw->name) {
      if (preceedingWindowName == wIt->aw->name)
    nextIt = ++wIt;
      else
    ++wIt;
    }
    if (wIt == (pIt->second).window.end()) {
      // analysis window does not already exist
      tmp = new mediate_analysis_window_t;

      if (newWindowName.length() < (int)sizeof(tmp->name)) {
    initializeMediateAnalysisWindow(tmp);
    // set its name ...
    strcpy(tmp->name, newWindowName.c_str());

    // insert at the specified position - ie. before nextIt
    if (nextIt == (pIt->second).window.end())
      (pIt->second).window.push_back(tmp);
    else
      (pIt->second).window.insert(nextIt, tmp);

    // notify the observers - Treated as a modification to the project
    notifyProjectObserversModified(projectName);
      }
      else {
    delete tmp; // name too long
    tmp = NULL;
      }
    }
  }

  return tmp;
}

bool CWorkSpace::createSite(const string &newSiteName, const string &abbr,
                double longitude, double latitude, double altitude)
{
  mediate_site_t *tmp;

  // limit check ...
  if (newSiteName.empty() || newSiteName.length() >= (int)sizeof(tmp->name) || abbr.length() >= (int)sizeof(tmp->abbreviation))
    return false;

  std::map<string,mediate_site_t*>::iterator it = m_siteMap.find(newSiteName);
  if (it == m_siteMap.end()) {
    tmp = new mediate_site_t;

    strcpy(tmp->name, newSiteName.c_str());
    strcpy(tmp->abbreviation, abbr.c_str());
    tmp->longitude = longitude;
    tmp->latitude = latitude;
    tmp->altitude = altitude;

    m_siteMap.insert(std::map<string,mediate_site_t*>::value_type(newSiteName, tmp));

    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateNewSite(newSiteName);
      ++obs;
    }
    return true;
  }
  return false;
}

bool CWorkSpace::createSymbol(const string &newSymbolName, const string &description)
{
  mediate_symbol_t *tmp; // just for size checking ....

  if (newSymbolName.empty() || newSymbolName.length() >= (int)sizeof(tmp->name) || description.length() >= (int)sizeof(tmp->description) || (newSymbolName.find(';') != string::npos))
    return false;

  symbolmap_t::iterator it = m_symbolMap.find(newSymbolName);
  if (it == m_symbolMap.end()) {
    if (description.empty()) {
      // make an empty description
      string emptyStr = "";
      m_symbolMap.insert(symbolmap_t::value_type(newSymbolName, SSymbolBucket(emptyStr)));
    }
    else
      m_symbolMap.insert(symbolmap_t::value_type(newSymbolName, SSymbolBucket(description)));

    // notify the observers
    std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
    while (obs != m_symbolObserverList.end()) {
      (*obs)->updateNewSymbol(newSymbolName);
      ++obs;
    }
    return true;
  }
  return false;
}

bool CWorkSpace::renameProject(const string &oldProjectName, const string &newProjectName)
{
  // project must exist - locate by old name
  std::map<string,SProjBucket>::iterator oldIt = m_projMap.find(oldProjectName);
  if (oldIt != m_projMap.end()) {
    // no change is OK
    if (oldProjectName == newProjectName)
      return true;

    // check that the new name is not in use
    std::map<string,SProjBucket>::iterator newIt = m_projMap.find(newProjectName);
    if (newIt == m_projMap.end()) {
      // ok to rename - first notify, then touch the map, then notify again

      // notify the observers of a delete
      std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
      while (obs != m_projectObserverList.end()) {
    (*obs)->updateDeleteProject(oldProjectName);
    ++obs;
      }

      // change of key so insert for the new key then remove the old entry
      SProjBucket inserted = m_projMap.insert(std::map<string,SProjBucket>::value_type(newProjectName, oldIt->second)).first->second;
      m_projMap.erase(oldIt);

      // update project name metadata in mediate_project_t structure:
      strncpy(inserted.project->project_name, newProjectName.c_str(), PROJECT_NAME_BUFFER_LENGTH-1);

      // notify the observers again
      obs = m_projectObserverList.begin();
      while (obs != m_projectObserverList.end()) {
    (*obs)->updateNewProject(newProjectName);
    ++obs;
      }

      return true;
    }
  }

  return false;
}

bool CWorkSpace::renameAnalysisWindow(const string &projectName, const string &oldWindowName,
                      const string &newWindowName)
{
  // project must exist
  std::map<string,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // locate the window by the old name - must exist
    std::vector<SAnlysWinBucket>::iterator oldIt = (pIt->second).window.begin();
    while (oldIt != (pIt->second).window.end() && oldWindowName != oldIt->aw->name) ++oldIt;
    if (oldIt != (pIt->second).window.end() && newWindowName.length() < (int)sizeof(oldIt->aw->name)) {
      // no change in the name is ok
      if (oldWindowName == newWindowName)
    return true;

      std::vector<SAnlysWinBucket>::iterator newIt = (pIt->second).window.begin();
      while (newIt != (pIt->second).window.end() && newWindowName != newIt->aw->name) ++ newIt;
      if (newIt == (pIt->second).window.end()) {
    // new name is not in use .. ok to change the name
    strcpy(oldIt->aw->name, newWindowName.c_str());

    // notify the observers - Treated as a modification to the project
    notifyProjectObserversModified(projectName);
    return true;
      }
    }
  }
  return false;
}

void CWorkSpace::setConfigFile(const string &fileName) {
  m_configFile = fileName;
}

const string& CWorkSpace::getConfigFile() {
  return m_configFile;
}

bool CWorkSpace::modifySite(const string &siteName, const string &abbr,
                double longitude, double latitude, double altitude)
{
  mediate_site_t *tmp;

  // limit check ...
  if (siteName.length() >= (int)sizeof(tmp->name) || abbr.length() >= (int)sizeof(tmp->abbreviation))
    return false;

  std::map<string,mediate_site_t*>::iterator it = m_siteMap.find(siteName);
  if (it != m_siteMap.end()) {

    tmp = it->second;

    // cant change the name
    strcpy(tmp->abbreviation, abbr.c_str());
    tmp->longitude = longitude;
    tmp->latitude = latitude;
    tmp->altitude = altitude;

    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateModifySite(siteName);
      ++obs;
    }

    return true;
  }
  return false;
}

bool CWorkSpace::modifySymbol(const string &symbolName, const string &description)
{
  symbolmap_t::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end()) {
    // symbol exists - and should - only the description can be modified.
    if (description.empty())
      (it->second).description = ""; // empty
    else
      (it->second).description = description;

    // notify the observers
    std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
    while (obs != m_symbolObserverList.end()) {
      (*obs)->updateModifySymbol(symbolName);
      ++obs;
    }

    return true;
  }

  return false;
}


// data returned must be freed by the client with 'operator delete []'

mediate_site_t* CWorkSpace::siteList(int &listLength) const
{
  size_t n = m_siteMap.size();

  if (n > 0) {
    mediate_site_t *siteList = new mediate_site_t[n];
    mediate_site_t *tmp = siteList;

    // walk the list and copy
    std::map<string,mediate_site_t*>::const_iterator it = m_siteMap.begin();
    while (it != m_siteMap.end()) {
      *tmp = *(it->second);
      ++tmp;
      ++it;
    }

    listLength = (int)n;

    return siteList;
  }

  listLength = 0;
  return NULL;
}

// data returned must be freed by the client with 'operator delete []'

mediate_symbol_t* CWorkSpace::symbolList(int &listLength) const
{
  size_t n = m_symbolMap.size();

  if (n > 0) {
    mediate_symbol_t *symbolList = new mediate_symbol_t[n];
    mediate_symbol_t *tmp = symbolList;

    // walk the list and copy
    symbolmap_t::const_iterator it = m_symbolMap.begin();
    while (it != m_symbolMap.end()) {
      strcpy(tmp->name, (it->first).c_str());
      strcpy(tmp->description, (it->second).description.c_str());
      ++tmp;
      ++it;
    }

    listLength = (int)n;

    return symbolList;
  }

  listLength = 0;
  return NULL;
}

// Only provides 'enabled' windows. Will return NULL if no windows are enabled.

mediate_analysis_window_t* CWorkSpace::analysisWindowList(const string &projectName, int &listLength) const
{
  std::map<string,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    size_t n = (pIt->second).window.size();
    if (n > 0) {
      mediate_analysis_window_t *data = new mediate_analysis_window_t[n];
      mediate_analysis_window_t *p = data;

      listLength = 0;

      // walk the vector and copy ... order is important
      std::vector<SAnlysWinBucket>::const_iterator wIt = (pIt->second).window.begin();
      while (wIt != (pIt->second).window.end()) {
    if (wIt->enabled) {
      *p = *(wIt->aw); // blot copy
      ++p;
      ++listLength;
    }
    ++wIt;
      }
      if (listLength == 0) {
    delete [] data;
    data = NULL;
      }
      return data;
    }
  }

  listLength = 0;
  return NULL;
}

vector<mediate_analysis_window_t*> CWorkSpace::analysisWindowList(const string &projectName) const
{
  vector<mediate_analysis_window_t*> result;

  std::map<string,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    std::vector<SAnlysWinBucket>::const_iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      mediate_analysis_window_t *data = new mediate_analysis_window_t;
      *data = *(wIt->aw); // blot copy
      result.push_back(data);
      ++wIt;
    }
  }

  return result;
}

vector<string> CWorkSpace::symbolList(void) const
{
  vector<string> symbolList;

  symbolmap_t::const_iterator it = m_symbolMap.begin();
  while (it != m_symbolMap.end()) {
    symbolList.push_back(it->first);
    ++it;
  }

  return symbolList;
}

vector<string> CWorkSpace::analysisWindowsWithSymbol(const string &projectName, const string &symbol) const
{
  // buld a list of names of analysis windows that contains 'symbol' in the crossSectionList...
  vector<string> result;

  std::map<string,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    std::vector<SAnlysWinBucket>::const_iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      const cross_section_list_t *d = &(wIt->aw->crossSectionList);
      int i = 0;
      while (i < d->nCrossSection) {
    if (symbol == string(d->crossSection[i].symbol)) {
      result.push_back(wIt->aw->name); // this AW contains the symbol as a cross section
      break;
    }
    ++i;
      }
      ++wIt;
    }
  }

  return result;
}

bool CWorkSpace::destroyProject(const string &projectName)
{
  // project must exist
  std::map<string,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {

    // notify the observers
    std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
    while (obs != m_projectObserverList.end()) {
      (*obs)->updateDeleteProject(projectName);
      ++obs;
    }

    // delete all analysis windows ...
    std::vector<SAnlysWinBucket>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      // update the useCount for symbols
      for (int i=0; i < wIt->aw->crossSectionList.nCrossSection; ++i)
    decrementUseCount(wIt->aw->crossSectionList.crossSection[i].symbol);

      delete wIt->aw;
      ++wIt;
    }
    (pIt->second).window.clear(); // would happen when the project bucket is erased from the map...

    // update the useCount for symbols
    for (int i=0; i < (pIt->second).project->calibration.crossSectionList.nCrossSection; ++i)
      decrementUseCount((pIt->second).project->calibration.crossSectionList.crossSection[i].symbol);

    delete (pIt->second).project;

    m_projMap.erase(pIt);
    return true;
  }
  return false;
}

bool CWorkSpace::destroyAnalysisWindow(const string &projectName, const string &windowName)
{
  // project must exist
  std::map<string,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    std::vector<SAnlysWinBucket>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end() && windowName != wIt->aw->name) ++wIt;
    if (wIt != (pIt->second).window.end()) {

      // update the useCount for symbols
      for (int i=0; i < wIt->aw->crossSectionList.nCrossSection; ++i)
    decrementUseCount(wIt->aw->crossSectionList.crossSection[i].symbol);

      delete wIt->aw;
      (pIt->second).window.erase(wIt);

      // notify observers - treated as a modification to the project
      notifyProjectObserversModified(projectName);
      return true;
    }
  }

  return false;
}

bool CWorkSpace::destroySite(const string &siteName)
{
  std::map<string,mediate_site_t*>::iterator it = m_siteMap.find(siteName);
  if (it != m_siteMap.end()) {

    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateDeleteSite(siteName);
      ++obs;
    }

    delete it->second;
    m_siteMap.erase(it);
    return true;
  }
  return false;
}

bool CWorkSpace::destroySymbol(const string &symbolName)
{
  symbolmap_t::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end()) {
    // Only permitted if the useCount is zero ...
    if ((it->second).useCount == 0) {
      // notify the observers
      std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
      while (obs != m_symbolObserverList.end()) {
    (*obs)->updateDeleteSymbol(symbolName);
    ++obs;
      }
      m_symbolMap.erase(it);
      return true;
    }
  }
  return false;
}

bool CWorkSpace::setAnalysisWindowEnabled(const string &projectName,
                      const string &windowName, bool enabled)
{
  std::map<string,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    std::vector<SAnlysWinBucket>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end() && windowName != wIt->aw->name) ++wIt;
    if (wIt != (pIt->second).window.end()) {
      wIt->enabled = enabled;
      return true;
    }
  }

  return false;
}

void CWorkSpace::attach(CSitesObserver *observer)
{
  if (std::find(m_sitesObserverList.begin(), m_sitesObserverList.end(), observer) == m_sitesObserverList.end())
    m_sitesObserverList.push_back(observer);
}

void CWorkSpace::detach(CSitesObserver *observer)
{
  m_sitesObserverList.remove(observer);
}

void CWorkSpace::attach(CSymbolObserver *observer)
{
  if (std::find(m_symbolObserverList.begin(), m_symbolObserverList.end(), observer) == m_symbolObserverList.end())
    m_symbolObserverList.push_back(observer);
}

void CWorkSpace::detach(CSymbolObserver *observer)
{
  m_symbolObserverList.remove(observer);
}

void CWorkSpace::attach(CProjectObserver *observer)
{
  if (std::find(m_projectObserverList.begin(), m_projectObserverList.end(), observer) == m_projectObserverList.end())
    m_projectObserverList.push_back(observer);
}
void CWorkSpace::detach(CProjectObserver *observer)
{
  m_projectObserverList.remove(observer);
}

void CWorkSpace::notifyProjectObserversModified(const string &projectName)
{
  // notify the observers - Treated as a modification to the project
  std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
  while (obs != m_projectObserverList.end()) {
    (*obs)->updateModifyProject(projectName);
    ++obs;
  }
}

//-----------------------------------------------------------------------

CSitesObserver::CSitesObserver()
{
  CWorkSpace::instance()->attach(this);
}

CSitesObserver::~CSitesObserver()
{
  CWorkSpace::instance()->detach(this);
}

void CSitesObserver::updateNewSite(const string &newSiteName)
{
}

void CSitesObserver::updateModifySite(const string &siteName)
{
}

void CSitesObserver::updateDeleteSite(const string &siteName)
{
}

//-----------------------------------------------------------------------

CSymbolObserver::CSymbolObserver()
{
  CWorkSpace::instance()->attach(this);
}

CSymbolObserver::~CSymbolObserver()
{
  CWorkSpace::instance()->detach(this);
}

void CSymbolObserver::updateNewSymbol(const string &newSymbolName)
{
}

void CSymbolObserver::updateModifySymbol(const string &symbolName)
{
}

void CSymbolObserver::updateDeleteSymbol(const string &symbolName)
{
}

//-----------------------------------------------------------------------

CProjectObserver::CProjectObserver()
{
  CWorkSpace::instance()->attach(this);
}

CProjectObserver::~CProjectObserver()
{
  CWorkSpace::instance()->detach(this);
}

void CProjectObserver::updateNewProject(const string &newProjectName)
{
}

void CProjectObserver::updateModifyProject(const string &projectName)
{
}

void CProjectObserver::updateDeleteProject(const string &projectName)
{
}

