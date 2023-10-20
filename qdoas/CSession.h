/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CSESSION_H_GUARD
#define _CSESSION_H_GUARD

// A data bucket for holding a snap-shot of the state
// requirede for a browse or analysis session
#include <map>

#include <QList>
#include <QStringList>
#include <QFileInfo>

#include "RefCountPtr.h"

#include "mediate_general.h"
#include "mediate_project.h"
#include "mediate_analysis_window.h"

class CSession;
class CSessionIterator;

class CSessionItem
{
 public:
  CSessionItem(const mediate_project_t *project);
  ~CSessionItem();

  void addFile(const QFileInfo &file);
  void giveAnalysisWindowList(mediate_analysis_window_t *windows, int nWindows); // takes ownership of windows (array)

 private:
  mediate_project_t m_project;
  mediate_analysis_window_t *m_windows;
  int m_nWindows;

  QList<QFileInfo> m_files;

  friend class CSession;
  friend class CSessionIterator;
};

class CSession
{
 public:
  enum eMode { Browse = 0x1, Calibrate = 0x2, Analyse = 0x4, Export = 0x8 };

  CSession(CSession::eMode mode);
  ~CSession();

  CSession::eMode mode(void) const;

  void addFile(const QFileInfo &file, const QString &projectName);

  int size(void) const;
  QStringList fileList(void) const;

  mediate_symbol_t* takeSymbolList(int &nSymbols);
  mediate_site_t* takeSiteList(int &nSites);

 private:
  CSession(const CSession &);
  CSession& operator=(const CSession&);

 private:
  typedef std::map<QString,CSessionItem*> sessionmap_t;

  sessionmap_t m_map;

  mediate_symbol_t *m_symbols;
  mediate_site_t *m_sites;
  int m_nSymbols;
  int m_nSites;

  eMode m_mode;
  QString m_asciiFile;

  friend class CSessionIterator;
};

class CSessionIterator
{
 public:
  CSessionIterator();
  CSessionIterator(const RefCountConstPtr<CSession> &session);
  CSessionIterator(const CSessionIterator &other);

  CSessionIterator& operator=(const CSessionIterator &rhs);

  CSessionIterator& operator++(void);
  CSessionIterator& operator--(void);

  CSessionIterator& operator()(int index);

  int index(void) const;

  bool atEnd(void) const;
  bool atBegin(void) const;

  const QFileInfo& file(void) const;
  const mediate_project_t* project(void) const;
  const mediate_analysis_window_t* analysisWindowList(int &nWindows) const;

  bool operator==(const CSessionIterator &rhs) const;
  bool operator!=(const CSessionIterator &rhs) const;

 private:
  RefCountConstPtr<CSession> m_session;
  CSession::sessionmap_t::const_iterator m_mapIt;
  int m_fileIndex;
  int m_offset;
};

#endif
