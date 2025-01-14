/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <assert.h>

#include "CSession.h"

#include "CWorkSpace.h"

CSessionItem::CSessionItem(const mediate_project_t *project) :
  m_windows(NULL),
  m_nWindows(0)
{
  // copy the project data
  m_project = *project;
}

CSessionItem::~CSessionItem()
{
  delete [] m_windows;
}

void CSessionItem::addFile(const QFileInfo &file)
{
  m_files.push_back(file);
}

void CSessionItem::giveAnalysisWindowList(mediate_analysis_window_t *windows, int nWindows)
{
  // ditch the current data ...
  delete [] m_windows;

  // takes ownership of windows (array)
  m_windows = windows;
  m_nWindows = nWindows;
}

//--------------------------------------------------------

CSession::CSession(eMode mode) :
  m_symbols(NULL),
  m_sites(NULL),
  m_nSymbols(0),
  m_nSites(0),
  m_mode(mode)
{
  // get a snap-shot of the symbol and site lists from the workspace if needed

  if (m_mode == CSession::Analyse || m_mode == CSession::Calibrate) {
    m_symbols = CWorkSpace::instance()->symbolList(m_nSymbols);
    m_sites = CWorkSpace::instance()->siteList(m_nSites);
  }
  else if ((m_mode == CSession::Browse) || (m_mode == CSession::Export))
   m_sites = CWorkSpace::instance()->siteList(m_nSites);
}

CSession::~CSession()
{
  sessionmap_t::iterator it = m_map.begin();
  while (it != m_map.end()) {
    delete it->second;
    ++it;
  }
  m_map.clear();

  delete [] m_symbols;
  delete [] m_sites;
}

CSession::eMode CSession::mode(void) const
{
  return m_mode;
}

void CSession::addFile(const QFileInfo &file, const QString &projectName)
{
  sessionmap_t::iterator it = m_map.find(projectName);

  if (it == m_map.end()) {
    // dont have this project yet ... grab it from the workspace
    mediate_project_t *proj = CWorkSpace::instance()->findProject(projectName.toStdString());
    if (proj) {
      CSessionItem *item = new CSessionItem(proj);
      item->addFile(file);

      if (m_mode == CSession::Analyse || m_mode == CSession::Calibrate) {
    int nWindows;
    mediate_analysis_window_t *d = CWorkSpace::instance()->analysisWindowList(projectName.toStdString(), nWindows);
    if (d != NULL) {
      item->giveAnalysisWindowList(d, nWindows);
    }
      }

      m_map.insert(sessionmap_t::value_type(projectName,item));
    }
    else {
      // this is very bad ...
      assert(false);
    }
  }
  else {
    // exists
    (it->second)->addFile(file);
  }
}

int CSession::size(void) const
{
  int count = 0;

  // get the number of files in the session
  sessionmap_t::const_iterator it = m_map.begin();
  while (it != m_map.end()) {
    count += (it->second)->m_files.size();
    ++it;
  }

  return count;
}

QStringList CSession::fileList(void) const
{
  // get an order list of the file in the session. The index of a file in this list corresponds
  // to the index for session iterator.

  QStringList fileList;

  sessionmap_t::const_iterator it = m_map.begin();
  while (it != m_map.end()) {
    const QList<QFileInfo> &tmp = (it->second)->m_files;
    QList<QFileInfo>::const_iterator fIt =  tmp.begin();
    while (fIt != tmp.end()) {
      fileList << fIt->fileName(); // JUST the name, not the path
      ++fIt;
    }
    ++it;
  }

  return fileList;
}

mediate_symbol_t* CSession::takeSymbolList(int &nSymbols)
{
  mediate_symbol_t *result = m_symbols;

  nSymbols = m_nSymbols;
  // released from memory management responsibility
  m_symbols = NULL;
  m_nSymbols = 0;

  return result;
}

mediate_site_t* CSession::takeSiteList(int &nSites)
{
  mediate_site_t *result = m_sites;

  nSites = m_nSites;
  // released from memory management responsibility
  m_sites = NULL;
  m_nSites = 0;

  return result;
}

//-----------------------------------------------------------------------

CSessionIterator::CSessionIterator() :
  m_fileIndex(0),
  m_offset(0)
{
}

CSessionIterator::CSessionIterator(std::shared_ptr<const CSession> session) :
  m_session(session)
{
  if (m_session) {
    m_mapIt = m_session->m_map.begin();
    m_fileIndex = 0;
    m_offset = 0;
  }
}

CSessionIterator::CSessionIterator(const CSessionIterator &other) :
  m_session(other.m_session),
  m_mapIt(other.m_mapIt),
  m_fileIndex(other.m_fileIndex),
  m_offset(other.m_offset)
{
}

CSessionIterator& CSessionIterator::operator=(const CSessionIterator &rhs)
{
  m_session = rhs.m_session;
  m_mapIt = rhs.m_mapIt;
  m_fileIndex = rhs.m_fileIndex;
  m_offset = rhs.m_offset;

  return *this;
}

CSessionIterator& CSessionIterator::operator++(void)
{
  // no range checks here - use atEnd first ...

  if (++m_fileIndex == (m_mapIt->second)->m_files.size()) {
    m_offset += m_fileIndex;
    ++m_mapIt;
    m_fileIndex = 0;
  }

  return *this;
}

CSessionIterator& CSessionIterator::operator--(void)
{
  // no range checks here - use atBegin first ...

  if (--m_fileIndex < 0) {
    --m_mapIt;
    m_fileIndex = (m_mapIt->second)->m_files.size();
    m_offset -= m_fileIndex;
    --m_fileIndex;
  }

  return *this;
}

// specific positioning of the iterator

CSessionIterator& CSessionIterator::operator()(int index)
{
  // no range checks
  m_offset = 0;
  m_fileIndex = index;
  m_mapIt = m_session->m_map.begin();
  while (m_mapIt != m_session->m_map.end() &&
     m_fileIndex >= (m_mapIt->second)->m_files.size()) {
    m_offset += (m_mapIt->second)->m_files.size();
    m_fileIndex = index - m_offset;
    ++m_mapIt;
  }

  return *this;
}

int CSessionIterator::index(void) const
{
  return m_offset + m_fileIndex;
}

bool CSessionIterator::atEnd(void) const
{
  return (!m_session || m_mapIt == m_session->m_map.end() || m_fileIndex == (m_mapIt->second)->m_files.size());
}

bool CSessionIterator::atBegin(void) const
{
  // NOTE returns false if session is null

  return (m_session && m_mapIt == m_session->m_map.begin() && m_fileIndex == 0);
}

const QFileInfo& CSessionIterator::file(void) const
{
  return (m_mapIt->second)->m_files.at(m_fileIndex);
}

const mediate_project_t* CSessionIterator::project(void) const
{
  return &((m_mapIt->second)->m_project);
}

const mediate_analysis_window_t* CSessionIterator::analysisWindowList(int &nWindows) const
{
  nWindows = (m_mapIt->second)->m_nWindows;
  return ((m_mapIt->second)->m_windows);
}

bool CSessionIterator::operator==(const CSessionIterator &rhs) const
{
  return (m_session == rhs.m_session && m_mapIt == rhs.m_mapIt && m_fileIndex == rhs.m_fileIndex);
}

bool CSessionIterator::operator!=(const CSessionIterator &rhs) const
{
  return !operator==(rhs);
}

