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


#ifndef _CWORKSPACE_H_GUARD
#define _CWORKSPACE_H_GUARD

#include <map>
#include <list>
#include <vector>

#include <QList>
#include <QString>
#include <QStringList>

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
  QString description;
  int useCount;
  
  SSymbolBucket(const QString &descr) : description(descr), useCount(0) {}
};

// The symbol map forces unqiueness of symbols at the case-insensive level.
  
struct SymbolCmp
{
  bool operator()(const QString &lhs, const QString &rhs) const { return (QString::compare(lhs, rhs, Qt::CaseInsensitive) < 0); }
};

class CWorkSpace
{
 public:
  static CWorkSpace* instance(void);

  ~CWorkSpace();

  void removeAllContent(void);

  mediate_project_t* findProject(const QString &projectName) const;
  mediate_analysis_window_t* findAnalysisWindow(const QString &projectName, const QString &windowName) const;

  const mediate_site_t* findSite(const QString &siteName) const;
  QString findSymbol(const QString &symbolName) const;

  void incrementUseCount(const QString &symbolName);
  void decrementUseCount(const QString &symbolName);

  // create item and return pointer the storage allocated (and managed) by the workspace. NULL is returned on failure.
  mediate_project_t* createProject(const QString &newProjectName);
  mediate_analysis_window_t* createAnalysisWindow(const QString &projectName, const QString &newWindowName,
						  const QString &preceedingWindowName = QString());
  bool createSite(const QString &newSiteName, const QString &abbr, double longitude, double latitude, double altitude);
  bool createSymbol(const QString &newSymbolName, const QString &description);

  bool renameProject(const QString &oldProjectName, const QString &newProjectName);
  bool renameAnalysisWindow(const QString &projectName, const QString &oldWindowName, const QString &newWindowName);

  void setConfigFile(const QString &fileName);
  const QString& getConfigFile();

  bool modifySite(const QString &siteName, const QString &abbr, double longitude, double latitude, double altitude);
  bool modifySymbol(const QString &symbolName, const QString &description);
  void modifiedProjectProperties(const QString &projectName);

  // return arrays ( allocated with new [] ).
  mediate_site_t* siteList(int &listLength) const;
  mediate_symbol_t* symbolList(int &listLength) const;
  mediate_analysis_window_t* analysisWindowList(const QString &projectName, int &listLength) const;
  
  // return deep-copied list of analysis window properties
  QList<mediate_analysis_window_t*> analysisWindowList(const QString &projectName) const;

  QStringList symbolList(void) const;
  QStringList analysisWindowsWithSymbol(const QString &projectName, const QString &symbol) const;
  bool setAnalysisWindowEnabled(const QString &projectName,
				const QString &windowName, bool enabled); 

  bool destroyProject(const QString &projectName);
  bool destroyAnalysisWindow(const QString &projectName, const QString &newWindowName);
  bool destroySite(const QString &siteName);
  bool destroySymbol(const QString &symbolName);

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

  void notifyProjectObserversModified(const QString &projectName);

  friend class CSitesObserver;
  friend class CSymbolObserver;
  friend class CProjectObserver;

 private:
  typedef std::map<QString,SSymbolBucket,SymbolCmp> symbolmap_t;

  static CWorkSpace *m_instance;

  QString m_configFile;
  std::map<QString,SProjBucket> m_projMap;
  std::map<QString,mediate_site_t*> m_siteMap;
  symbolmap_t m_symbolMap;
  std::list<CSitesObserver*> m_sitesObserverList;
  std::list<CSymbolObserver*> m_symbolObserverList;
  std::list<CProjectObserver*> m_projectObserverList;
};

inline void CWorkSpace::modifiedProjectProperties(const QString &projectName) {
  notifyProjectObserversModified(projectName);
}


class CSitesObserver {
 public:
  CSitesObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CSitesObserver();

  virtual void updateNewSite(const QString &newSiteName);
  virtual void updateModifySite(const QString &siteName);
  virtual void updateDeleteSite(const QString &siteName);
};

class CSymbolObserver {
 public:
  CSymbolObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CSymbolObserver();

  virtual void updateNewSymbol(const QString &newSymbolName);
  virtual void updateModifySymbol(const QString &symbolName);
  virtual void updateDeleteSymbol(const QString &symbolName);
};

class CProjectObserver {
 public:
  CProjectObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CProjectObserver();

  virtual void updateNewProject(const QString &newProjectName);
  virtual void updateModifyProject(const QString &projectName);
  virtual void updateDeleteProject(const QString &projectName);
};

#endif

