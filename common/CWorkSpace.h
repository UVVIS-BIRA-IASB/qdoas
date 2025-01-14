/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWORKSPACE_H_GUARD
#define _CWORKSPACE_H_GUARD

#include <map>
#include <list>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "mediate.h"

class CSitesObserver;
class CSymbolObserver;
class CProjectObserver;

struct SAnlysWinBucket
{
  mediate_analysis_window_t *aw;
  bool enabled;

  SAnlysWinBucket(mediate_analysis_window_t *anlyswin) : aw(anlyswin), enabled(true) {}
  SAnlysWinBucket(const SAnlysWinBucket &c) : aw(c.aw), enabled(c.enabled) {}
};

struct SProjBucket
{
  mediate_project_t *project;
  std::vector<SAnlysWinBucket> window;

  // takes ownership of projectData
  SProjBucket(mediate_project_t *projectData) : project(projectData) {}
  // NOTE: shallow copy of dynamic memory. Use for std::map<>::insert operations ONLY.
  SProjBucket(const SProjBucket &c) : project(c.project), window(c.window) {}
};

struct SSymbolBucket
{
  std::string description;
  int useCount;

  SSymbolBucket(const std::string &descr) : description(descr), useCount(0) {}
};

// The symbol map forces unqiueness of symbols at the case-insensive level.

struct SymbolCmp
{
  bool operator()(const std::string &lhs, const std::string &rhs) const { return boost::algorithm::ilexicographical_compare(lhs, rhs); }
};

class CWorkSpace
{
 public:
  static CWorkSpace* instance(void);

  ~CWorkSpace();

  void removeAllContent(void);

  mediate_project_t* findProject(const std::string &projectName) const;
  mediate_analysis_window_t* findAnalysisWindow(const std::string &projectName, const std::string &windowName) const;

  const mediate_site_t* findSite(const std::string &siteName) const;
  std::string findSymbol(const std::string &symbolName) const;

  void incrementUseCount(const std::string &symbolName);
  void decrementUseCount(const std::string &symbolName);

  // create item and return pointer the storage allocated (and managed) by the workspace. NULL is returned on failure.
  mediate_project_t* createProject(const std::string &newProjectName);
  mediate_analysis_window_t* createAnalysisWindow(const std::string &projectName, const std::string &newWindowName,
                          const std::string &preceedingWindowName = std::string());
  bool createSite(const std::string &newSiteName, const std::string &abbr, double longitude, double latitude, double altitude);
  bool createSymbol(const std::string &newSymbolName, const std::string &description);

  bool renameProject(const std::string &oldProjectName, const std::string &newProjectName);
  bool renameAnalysisWindow(const std::string &projectName, const std::string &oldWindowName, const std::string &newWindowName);

  void setConfigFile(const std::string &fileName);
  const std::string& getConfigFile();

  bool modifySite(const std::string &siteName, const std::string &abbr, double longitude, double latitude, double altitude);
  bool modifySymbol(const std::string &symbolName, const std::string &description);
  void modifiedProjectProperties(const std::string &projectName);

  // return arrays ( allocated with new [] ).
  mediate_site_t* siteList(int &listLength) const;
  mediate_symbol_t* symbolList(int &listLength) const;
  mediate_analysis_window_t* analysisWindowList(const std::string &projectName, int &listLength) const;

  // return deep-copied list of analysis window properties
  std::vector<mediate_analysis_window_t*> analysisWindowList(const std::string &projectName) const;

  std::vector<std::string> symbolList(void) const;
  std::vector<std::string> analysisWindowsWithSymbol(const std::string &projectName, const std::string &symbol) const;
  bool setAnalysisWindowEnabled(const std::string &projectName,
                const std::string &windowName, bool enabled);

  bool destroyProject(const std::string &projectName);
  bool destroyAnalysisWindow(const std::string &projectName, const std::string &newWindowName);
  bool destroySite(const std::string &siteName);
  bool destroySymbol(const std::string &symbolName);

 private:
  // singleton => no copies permitted
  CWorkSpace() {}
  CWorkSpace(const CWorkSpace &) {}
  CWorkSpace& operator=(const CWorkSpace &) { return *this; }

  void attach(CSitesObserver *observer);
  void detach(CSitesObserver *observer);

  void attach(CSymbolObserver *observer);
  void detach(CSymbolObserver *observer);

  void attach(CProjectObserver *observer);
  void detach(CProjectObserver *observer);

  void notifyProjectObserversModified(const std::string &projectName);

  friend class CSitesObserver;
  friend class CSymbolObserver;
  friend class CProjectObserver;

 private:
  typedef std::map<std::string,SSymbolBucket,SymbolCmp> symbolmap_t;

  static CWorkSpace *m_instance;

  std::string m_configFile;
  std::map<std::string,SProjBucket> m_projMap;
  std::map<std::string,mediate_site_t*> m_siteMap;
  symbolmap_t m_symbolMap;
  std::list<CSitesObserver*> m_sitesObserverList;
  std::list<CSymbolObserver*> m_symbolObserverList;
  std::list<CProjectObserver*> m_projectObserverList;
};

inline void CWorkSpace::modifiedProjectProperties(const std::string &projectName) {
  notifyProjectObserversModified(projectName);
}


class CSitesObserver {
 public:
  CSitesObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CSitesObserver();

  virtual void updateNewSite(const std::string &newSiteName);
  virtual void updateModifySite(const std::string &siteName);
  virtual void updateDeleteSite(const std::string &siteName);
};

class CSymbolObserver {
 public:
  CSymbolObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CSymbolObserver();

  virtual void updateNewSymbol(const std::string &newSymbolName);
  virtual void updateModifySymbol(const std::string &symbolName);
  virtual void updateDeleteSymbol(const std::string &symbolName);
};

class CProjectObserver {
 public:
  CProjectObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CProjectObserver();

  virtual void updateNewProject(const std::string &newProjectName);
  virtual void updateModifyProject(const std::string &projectName);
  virtual void updateDeleteProject(const std::string &projectName);
};

#endif
