/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWPROJECTTREE_H_GUARD
#define _CWPROJECTTREE_H_GUARD

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QSize>

#include <memory>

#include "CSession.h"
#include "CViewCrossSectionData.h"
#include "CProjectConfigItem.h"

class CWActiveContext;
class CProjectTreeItem;
class CProjectItem;
class CSpectraFileItem;
class CSpectraFolderItem;
class CSpectraDirectoryItem;
class CAnalysisWindowItem;
class CProjectTreeClipboard;

const int cProjectItemType              = QTreeWidgetItem::UserType + 91;
const int cSpectraBranchItemType        = QTreeWidgetItem::UserType + 92;
const int cAnalysisWindowBranchItemType = QTreeWidgetItem::UserType + 93;
const int cSpectraFolderItemType        = QTreeWidgetItem::UserType + 94;
const int cSpectraDirectoryItemType     = QTreeWidgetItem::UserType + 95;
const int cSpectraFileItemType          = QTreeWidgetItem::UserType + 96;
const int cAnalysisWindowItemType       = QTreeWidgetItem::UserType + 97;

class CWProjectTree : public QTreeWidget
{
Q_OBJECT
 public:
  CWProjectTree(CWActiveContext *activeContext, QWidget *parent = 0);
  virtual ~CWProjectTree();

  QTreeWidgetItem *locateProjectByName(const QString &projectName);
  QTreeWidgetItem *locateByPath(const QStringList &path);

  void buildAndStartSession(CSession::eMode sessionType);
  void ExportSpectra();

  void savePreferences(void);

  void removeAllContent(void);
  QString loadConfiguration(const std::vector<CProjectConfigItem> &itemList);

  // Interface editors use to modify the tree

  QString editInsertNewProject(const QString &projectName, CProjectItem **itemCreated = NULL);
  QString editRenameProject(QTreeWidgetItem *item, const QString &projectName);
  QString editInsertNewFolder(QTreeWidgetItem *parent, const QString &folderName, CSpectraFolderItem **itemCreated = NULL);
  QString editRenameFolder(QTreeWidgetItem *item, const QString &folderName);
  QString editInsertNewAnalysisWindow(QTreeWidgetItem *parent, const QString &windowName,
                      const QString &preceedingWindowName, CAnalysisWindowItem **itemCreated = NULL);
  QString editRenameAnalysisWindow(QTreeWidgetItem *item, const QString &windowName);
  QString editInsertDirectory(QTreeWidgetItem *parent, const QString &directoryPath,
                  const QString &fileFilters, bool includeSubDirs, CSpectraDirectoryItem **itemCreated = NULL);
  QString editChangeDirectoryProperties(QTreeWidgetItem *item, const QString &fileFilters, bool includeSubDirs);

  // static methods

  static const QIcon& getIcon(int type);

  static int itemDepth(QTreeWidgetItem *item);
  static QTreeWidgetItem* ancestor(QTreeWidgetItem *item, int nth);
  static QTreeWidgetItem* projectItem(QTreeWidgetItem *item);
  static QList<QTreeWidgetItem*> normalize(QList<QTreeWidgetItem*> items);
  static QList<QTreeWidgetItem*> directoryItems(const QList<QTreeWidgetItem*> &items);
  static QTreeWidgetItem* locateChildByName(QTreeWidgetItem *parent, const QString &childName);
  static void buildSession(CSession *session, CProjectTreeItem *item);
  static void collateErrorMessage(QString &errStr, const QString &msg);

 private:
  QString buildRawSpectraTree(QTreeWidgetItem *parent, const CProjectConfigTreeNode *childConfigItem);

 protected:
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void showEvent(QShowEvent *e);
  virtual void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotEnable();
  void slotDisable();
  void slotToggleEnable();
  void slotToggleDisplayDetails();
  void slotCreateProject();
  void slotRenameProject();
  void slotEditProject();
  void slotEditAnalysisWindow();
  void slotRefreshDirectories();
  void slotCreateFolder();
  void slotRenameFolder();
  void slotInsertFile();
  void slotInsertDirectory();
  void slotEditDirectory();
  void slotCreateAnalysisWindow();
  void slotRenameAnalysisWindow();
  void slotRunAnalysis();
  void slotRunCalibration();
  void slotBrowseSpectra();
  void slotExportSpectra();
  void slotViewCrossSections();
  void slotDeleteSelection();
  void slotCutSelection();
  void slotCopySelection();
  void slotPaste();
  void slotPasteProjects();
  void slotPasteAnalysisWindows();
  void slotPasteSpectraAsSiblings();
  void slotPasteSpectraAsChildren();
  void slotShowPropertiesSelection();
  void slotSessionRunning(bool running);

 signals:
  void signalWidthModeChanged(int newMode);
  void signalStartSession(std::shared_ptr<CSession> session);
  void signalViewCrossSections(std::shared_ptr<CViewCrossSectionData> awData);
  void signalSpectraTreeChanged();

 private:
  QList<int> m_colWidthList;
  CWActiveContext *m_activeContext;
  bool m_sessionActive;
  CProjectTreeClipboard *m_clipboard;

  // icons for the tree
  static QIcon *m_windowIcon, *m_folderIcon, *m_directoryIcon, *m_fileIcon;
  // icons for the context menu - TODO
};

// ALL concrete items in CWProjectTree are derived from CProjectTreeItem

class CProjectTreeItem : public QTreeWidgetItem
{
 public:
  CProjectTreeItem(const QStringList &strings, int type);
  CProjectTreeItem(CWProjectTree *parent, int type);
  CProjectTreeItem(CWProjectTree *parent, const QStringList &strings, int type);
  CProjectTreeItem(CWProjectTree *parent, QTreeWidgetItem *preceedingSibling, const QStringList &strings, int type);
  CProjectTreeItem(QTreeWidgetItem *parent, int type);
  CProjectTreeItem(QTreeWidgetItem *parent, const QStringList &strings, int type);
  CProjectTreeItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceedingSibling, const QStringList &strings, int type);
  CProjectTreeItem(const CProjectTreeItem &other); // for cloning the family of raw spectra items

  virtual void setEnabled(bool enable);
  bool isEnabled(void) const;

 protected:
  bool m_enabled;
};

inline bool CProjectTreeItem::isEnabled(void) const { return m_enabled; }

class CProjectItem : public CProjectTreeItem
{
 public:
  CProjectItem(const QString &projectName);
  CProjectItem(CWProjectTree *parent, QTreeWidgetItem *preceedingSibling, const QString &projectName);

  virtual QVariant data(int column, int role) const;

  static void destroyItem(QTreeWidget *tree, QTreeWidgetItem *projItem);
};

class CAnalysisWindowBranchItem : public CProjectTreeItem
{
 public:
  CAnalysisWindowBranchItem(QTreeWidgetItem *parent);
};

class CSpectraBranchItem : public CProjectTreeItem
{
 public:
  CSpectraBranchItem(QTreeWidgetItem *parent);
};

// Folder can have abitrary children (files, folders, directories)

class CSpectraFolderItem : public CProjectTreeItem
{
 public:
  CSpectraFolderItem(QTreeWidgetItem *parent, const QString &folderName);
  CSpectraFolderItem(const CSpectraFolderItem &other); // used by clone

  virtual QVariant data(int column, int role) const;
  virtual QTreeWidgetItem* clone() const;
};

// Directory can have files and directories as children, but is driven by the filesystem

class CSpectraDirectoryItem : public CProjectTreeItem
{
 public:
  CSpectraDirectoryItem(QTreeWidgetItem *parent, const QDir &directory,
                        const QStringList &fileFilters, bool includesSubDirectories,
                        int *fileCount = 0);
  CSpectraDirectoryItem(const CSpectraDirectoryItem &other); // used by clone

  virtual QVariant data(int column, int role) const;
  virtual QTreeWidgetItem* clone() const;

  void refreshBranch(void);
  void changeProperties(const QStringList &fileFilters, bool includeSubDirectories);

  QString directoryName(void) const;
  QString fileFilters(void) const;
  bool isRecursive(void) const;

 protected:
  int loadBranch(void);
  void discardBranch(void);

 private:
  QDir m_directory;
  QStringList m_fileFilters;
  bool m_includeSubDirectories;
};

// File - Single reference to a file-system file

class CSpectraFileItem : public CProjectTreeItem
{
 public:
  CSpectraFileItem(QTreeWidgetItem *parent, const QFileInfo &fileInfo);
  CSpectraFileItem(const CSpectraFileItem &other);

  QString fullFileName(void) const;
  QString dateLastModified(void) const;
  QString fileSizeInBytes(void) const;

  virtual QVariant data(int column, int role) const;
  virtual QTreeWidgetItem* clone() const;

  const QFileInfo& file(void) const;

 private:
  QFileInfo m_fileInfo;
};

inline const QFileInfo& CSpectraFileItem::file(void) const { return m_fileInfo; }

class CAnalysisWindowItem : public CProjectTreeItem
{
 public:
  CAnalysisWindowItem(QTreeWidgetItem *parent, const QString &windowName);
  CAnalysisWindowItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceedingSibling, const QString &windowName);

  virtual void setEnabled(bool enable);

  virtual QVariant data(int column, int role) const;

  static void destroyItem(QTreeWidgetItem *awItem);

};

#endif
