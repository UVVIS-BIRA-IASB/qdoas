/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CProjectTreeClipboard.h"


CProjClipBucket::CProjClipBucket(const QString &projectName, mediate_project_t *projectProperties,
                 QList<mediate_analysis_window_t*> &analysisWindows,
                 QList<QTreeWidgetItem*> &rawSpectraItems) :
  name(projectName),
  properties(projectProperties),
  windows(analysisWindows),
  spectra(rawSpectraItems)
{
}

CProjClipBucket::~CProjClipBucket()
{
  delete properties;

  while (!windows.isEmpty())
    delete windows.takeFirst();

  while (!spectra.isEmpty())
    delete spectra.takeFirst();
}

//----------------------------------------------------------------

CProjectTreeClipboard::CProjectTreeClipboard() :
  m_markedProjectGroup(false),
  m_markedAnlysWinGroup(false),
  m_markedSpectraGroup(false)
{
}

CProjectTreeClipboard::~CProjectTreeClipboard()
{
  // delete ALL groups ...
  clearProjectGroup();
  clearAnlysWinGroup();
  clearSpectraGroup();
}

void CProjectTreeClipboard::beginInsertItems(void)
{
  // Call ONCE before a series of insert*. Marks each group for 'clear on first insert'

  m_markedProjectGroup = true;
  m_markedAnlysWinGroup = true;
  m_markedSpectraGroup = true;
}

void CProjectTreeClipboard::endInsertItems(void)
{
  // Call ONCE after the series of insert*. Removes the mark

  m_markedProjectGroup = false;
  m_markedAnlysWinGroup = false;
  m_markedSpectraGroup = false;
}

// a complete project ... takes ownershift responsibility of data referenced
// by pointer (ie. properties, AWs, tree items)
void CProjectTreeClipboard::insertProject(const QString &projectName, mediate_project_t *properties,
                                          std::vector<mediate_analysis_window_t*> &analysisWindows,
                                          QList<QTreeWidgetItem*> &rawSpectraItems)
{
  // first consider the mark ... clear the group on first touch
  if (m_markedProjectGroup) {
    clearProjectGroup();
    m_markedProjectGroup = false;
  }

  // Need to convert
  QList<mediate_analysis_window_t*> analysisWindowsList;
  for (auto aw : analysisWindows) {
    analysisWindowsList.push_back(aw);
  }

  m_projectGroup.push_back(new CProjClipBucket(projectName, properties, analysisWindowsList, rawSpectraItems));
}

// a single analysis window ... takes ownership of the analysisWindow
void CProjectTreeClipboard::insertAnalysisWindow(mediate_analysis_window_t *analysisWindow)
{
  // first consider the mark ... clear the group on first touch
  if (m_markedAnlysWinGroup) {
    clearAnlysWinGroup();
    m_markedAnlysWinGroup = false;
  }

  m_anlysWinGroup.push_back(analysisWindow);
}

void CProjectTreeClipboard::insertRawSpectraItem(QTreeWidgetItem *rawSpectraItem)
{
  // first consider the mark ... clear the group on first touch
  if (m_markedSpectraGroup) {
    clearSpectraGroup();
    m_markedSpectraGroup = false;
  }

  m_spectraGroup.push_back(rawSpectraItem);
}

void CProjectTreeClipboard::insertRawSpectraItems(QList<QTreeWidgetItem*> &rawSpectraItems)
{
  // first consider the mark ... clear the group on first touch
  if (m_markedSpectraGroup) {
    clearSpectraGroup();
    m_markedSpectraGroup = false;
  }

  m_spectraGroup += rawSpectraItems;
}

void CProjectTreeClipboard::clearProjectGroup(void)
{
  while (!m_projectGroup.isEmpty())
    delete m_projectGroup.takeFirst();
}

void CProjectTreeClipboard::clearAnlysWinGroup(void)
{
  while (!m_anlysWinGroup.isEmpty())
    delete m_anlysWinGroup.takeFirst();
}

void CProjectTreeClipboard::clearSpectraGroup(void)
{
  while (!m_spectraGroup.isEmpty())
    delete m_spectraGroup.takeFirst();
}

// extracting items

int CProjectTreeClipboard::projectGroupSize(void) const
{
  return m_projectGroup.size();
}

// index based access to parts of the project bucket data
QString CProjectTreeClipboard::projectGroupItemName(int projIndex) const
{
  if (projIndex >= 0 && projIndex < m_projectGroup.size())
    return m_projectGroup.at(projIndex)->name;

  return QString();
}

const mediate_project_t* CProjectTreeClipboard::projectGroupItemProperties(int projIndex) const
{
  if (projIndex >= 0 && projIndex < m_projectGroup.size())
    return m_projectGroup.at(projIndex)->properties;

  return NULL;
}

int CProjectTreeClipboard::projectGroupItemAnalysisWindowSize(int projIndex) const
{
  if (projIndex >= 0 && projIndex < m_projectGroup.size())
    return m_projectGroup.at(projIndex)->windows.size();

  return 0;
}

const mediate_analysis_window_t* CProjectTreeClipboard::projectGroupItemAnalysisWindowProperties(int projIndex, int anlysWinIndex) const
{
  if (projIndex >= 0 && projIndex < m_projectGroup.size()) {
    const CProjClipBucket *tmp = m_projectGroup.at(projIndex);
    if (anlysWinIndex >= 0 && anlysWinIndex < tmp->windows.size())
      return tmp->windows.at(anlysWinIndex);
  }

  return NULL;
}

QList<QTreeWidgetItem*> CProjectTreeClipboard::projectGroupItemSpectraList(int projIndex) const
{
  QList<QTreeWidgetItem*> result;

  if (projIndex >= 0 && projIndex < m_projectGroup.size()) {
    const QList<QTreeWidgetItem*> &tmp = m_projectGroup.at(projIndex)->spectra;

    // clone the current list
    QList<QTreeWidgetItem*>::const_iterator it = tmp.begin();
    while (it != tmp.end()) {
      result.push_back((*it)->clone());
      ++it;
    }
  }

  return result;
}

int CProjectTreeClipboard::analysisWindowGroupSize(void) const
{
  return m_anlysWinGroup.size();
}

const mediate_analysis_window_t* CProjectTreeClipboard::analysisWindowGroupItemProperties(int anlysWinIndex) const
{
  if (anlysWinIndex >= 0 && anlysWinIndex < m_anlysWinGroup.size())
    return m_anlysWinGroup.at(anlysWinIndex);

  return NULL;
}

QList<QTreeWidgetItem*> CProjectTreeClipboard::spectraGroupList(void) const
{
  QList<QTreeWidgetItem*> result;

  // clone the current list
  QList<QTreeWidgetItem*>::const_iterator it = m_spectraGroup.begin();
  while (it != m_spectraGroup.end()) {
    result.push_back((*it)->clone());
    ++it;
  }

  return result;
}
