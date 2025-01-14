/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CPROJECTTREECLIPBOARD_H_GUARD
#define _CPROJECTTREECLIPBOARD_H_GUARD

#include <vector>

#include <QString>
#include <QList>
#include <QTreeWidgetItem>

#include <mediate_project.h>
#include <mediate_analysis_window.h>

class CProjectTreeClipboard;

class CProjClipBucket
{
 private:
  CProjClipBucket(const QString &projectName, mediate_project_t *properties,
          QList<mediate_analysis_window_t*> &analysisWindows,
          QList<QTreeWidgetItem*> &rawSpectraItems);

  ~CProjClipBucket();

 private:
  QString name;
  mediate_project_t *properties;
  QList<mediate_analysis_window_t*> windows;
  QList<QTreeWidgetItem*> spectra; // folders, directories, files

  friend class CProjectTreeClipboard;  // give direct access to this classes members
};

class CProjectTreeClipboard
{
 public:
  CProjectTreeClipboard();
  ~CProjectTreeClipboard();

  void beginInsertItems(void); // Call ONCE before a series of insert*
  void endInsertItems(void);   // Call ONCE after the series of insert*

  bool projectGroupIsEmpty(void) const;
  bool analysisWindowGroupIsEmpty(void) const;
  bool spectraGroupIsEmpty(void) const;

  // a complete project ... takes ownershift responsibility of data referenced
  // by pointer (ie. properties, AWs, tree items)
  void insertProject(const QString &projectName, mediate_project_t *properties,
                     std::vector<mediate_analysis_window_t*> &analysisWindows,
                     QList<QTreeWidgetItem*> &rawSpectraItems);

  // a single analysis window ... takes ownership of the analysisWindow
  void insertAnalysisWindow(mediate_analysis_window_t *analysisWindow);

  void insertRawSpectraItem(QTreeWidgetItem *rawSpectraItem);
  void insertRawSpectraItems(QList<QTreeWidgetItem*> &rawSpectraItems);

  // retrieve data for project items - NOTE the 'const mediate_..._t* ...Properties(...) const'
  // methods provide a handle to internal data that the caller MUST copy. The pointer MUST NOT
  // be copied.
  int projectGroupSize(void) const;
  QString projectGroupItemName(int projIndex) const;
  const mediate_project_t* projectGroupItemProperties(int projIndex) const;
  int projectGroupItemAnalysisWindowSize(int projIndex) const;
  const mediate_analysis_window_t* projectGroupItemAnalysisWindowProperties(int projIndex, int anlysWinIndex) const;
  QList<QTreeWidgetItem*> projectGroupItemSpectraList(int projIndex) const;

  int analysisWindowGroupSize(void) const;
  const mediate_analysis_window_t* analysisWindowGroupItemProperties(int anlysWinIndex) const;

  // cloned contents of the clipboard spectra group.
  QList<QTreeWidgetItem*> spectraGroupList(void) const;

 private:
  void clearProjectGroup(void);
  void clearAnlysWinGroup(void);
  void clearSpectraGroup(void);

 private:
  QList<CProjClipBucket*> m_projectGroup;
  QList<mediate_analysis_window_t*> m_anlysWinGroup;
  QList<QTreeWidgetItem*> m_spectraGroup;

  bool m_markedProjectGroup;
  bool m_markedAnlysWinGroup;
  bool m_markedSpectraGroup;
};

inline bool CProjectTreeClipboard::projectGroupIsEmpty(void) const { return m_projectGroup.isEmpty(); }
inline bool CProjectTreeClipboard::analysisWindowGroupIsEmpty(void) const { return m_anlysWinGroup.isEmpty(); }
inline bool CProjectTreeClipboard::spectraGroupIsEmpty(void) const { return m_spectraGroup.isEmpty(); }

#endif
