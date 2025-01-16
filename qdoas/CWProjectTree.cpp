/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QMenu>
#include <QDateTime>
#include <QKeyEvent>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QRegularExpression>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

#include <QFile>

#include "CWorkSpace.h"
#include "CWProjectTree.h"
#include "CProjectTreeClipboard.h"
#include "CWProjectNameEditor.h"
#include "CWProjectFolderNameEditor.h"
#include "CWProjectDirectoryEditor.h"
#include "CWProjectAnalysisWindowNameEditor.h"
#include "CWProjectPropertyEditor.h"
#include "CWAnalysisWindowPropertyEditor.h"
#include "CWActiveContext.h"
#include "CSession.h"

#include "CPreferences.h"
#include "CWProjectExportEditor.h"

#include "debugutil.h"

using std::shared_ptr;
using std::vector;

// Somewhat arbitrary constants that need to be unique for all widgets coupled to the
// CWSplitter widget.
const int cProjectTreeHideDetailMode   = 27;
const int cProjectTreeShowDetailMode   = 28;

// Title bar (Plot/Edit) colours (0xAARRGGBB)
const QRgb cDisabledTextColour         = 0xFFAAAAAA;
const QRgb cProjectTextColour          = 0xFFA93F26;

QIcon *CWProjectTree::m_windowIcon = NULL;
QIcon *CWProjectTree::m_folderIcon = NULL;
QIcon *CWProjectTree::m_directoryIcon = NULL;
QIcon *CWProjectTree::m_fileIcon = NULL;

CWProjectTree::CWProjectTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  m_activeContext(activeContext),
  m_sessionActive(false)
{
  QStringList labelList;
  labelList << "Name" << "Size" << "Modified";
  setHeaderLabels(labelList);

  QList<int> widthList;
  widthList.push_back(260);
  widthList.push_back(90);
  widthList.push_back(160);

  widthList = CPreferences::instance()->columnWidthList("ProjectTree", widthList);

  for (int i=0; i<3; ++i) {
    setColumnWidth(i, widthList.at(i));
  }

  setSelectionMode(QAbstractItemView::ExtendedSelection);

  slotToggleDisplayDetails();

  m_clipboard = new CProjectTreeClipboard;
}

CWProjectTree::~CWProjectTree()
{
  delete m_clipboard;

  if (m_windowIcon) {
    delete m_windowIcon;
    m_windowIcon = NULL;
  }
  if (m_folderIcon) {
    delete m_folderIcon;
    m_folderIcon = NULL;
  }
  if (m_directoryIcon) {
    delete m_directoryIcon;
    m_directoryIcon = NULL;
  }
  if (m_fileIcon) {
    delete m_fileIcon;
    m_fileIcon = NULL;
  }
}

void CWProjectTree::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_D) {
    slotToggleDisplayDetails();
    e->accept();
  }
  else
    QTreeWidget::keyPressEvent(e);
}

void CWProjectTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(m_colWidthList.empty() ? cProjectTreeShowDetailMode : cProjectTreeHideDetailMode);
}

void CWProjectTree::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  // always have at least 1 item selected ...
  QList<QTreeWidgetItem*> items = selectedItems();

  // Try and keep the order the same for all cases. Use a consistent layout for each
  // selected item type, and disable unavailble options.

  //------------------------------
  // Enable/Disable
  // New ...
  // Insert ...
  // Refresh
  //------------------------------
  // Run Analysis + Browse Spectra + Calibration (disabled if session is active)
  //------------------------------
  // Cut/Copy/Paste/Delete
  //------------------------------
  // properties
  //------------------------------
  // Hide/Show Details
  //------------------------------

  if (items.count() > 1) {
    // multiple selection - run analysis - enable/disable/toggle - cut/copy/delete
    menu.addAction("Enable", this, SLOT(slotEnable()));
    menu.addAction("Disable", this, SLOT(slotDisable()));
    menu.addAction("Enable/Disable", this, SLOT(slotToggleEnable()));
    menu.addSeparator();
    menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()))->setEnabled(!m_sessionActive);
    menu.addAction("Run Calibration", this, SLOT(slotRunCalibration()))->setEnabled(!m_sessionActive);
    menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()))->setEnabled(!m_sessionActive);
    menu.addAction("Export Data/Spectra", this, SLOT(slotExportSpectra()))->setEnabled(!m_sessionActive);
    menu.addSeparator();
    menu.addAction("Cut", this, SLOT(slotCutSelection()));
    menu.addAction("Copy", this, SLOT(slotCopySelection()));
    // Can paste when multiple items are selected.
    menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
  }
  else if (items.count()) {
    // one item selected - The type determines the menu content
    QTreeWidgetItem *item = items.front();
    int itemType = item->type();

    if (itemType == cSpectraDirectoryItemType) {
      // A Directory item
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(item);

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Edit...", this, SLOT(slotEditDirectory()));
      menu.addAction("Refresh", this, SLOT(slotRefreshDirectories()));

      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()))->setEnabled(!m_sessionActive);
      menu.addAction("Run Calibration", this, SLOT(slotRunCalibration()))->setEnabled(!m_sessionActive);
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()))->setEnabled(!m_sessionActive);
      menu.addAction("Export Data/Spectra", this, SLOT(slotExportSpectra()))->setEnabled(!m_sessionActive);
      menu.addSeparator();

      // Can't cut, delete or paste below an item that is a child of a directory item
      bool enableCutDel = (projItem->parent() && projItem->parent()->type() != cSpectraDirectoryItemType);
      bool enablePaste = enableCutDel && !m_clipboard->spectraGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()))->setEnabled(enableCutDel);
      menu.addAction("Copy", this, SLOT(slotCopySelection()));
      menu.addAction("Paste Below", this, SLOT(slotPasteSpectraAsSiblings()))->setEnabled(enablePaste);
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()))->setEnabled(enableCutDel);
      menu.addSeparator();
      menu.addAction("Properties", this, SLOT(slotShowPropertiesSelection()));
    }
    else if (itemType == cSpectraFolderItemType) {
      // A Folder Item
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(item);

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Rename...", this, SLOT(slotRenameFolder()));
      menu.addAction("New Sub-Folder...", this, SLOT(slotCreateFolder()));
      menu.addAction("Insert Directory...", this, SLOT(slotInsertDirectory()));
      menu.addAction("Insert File...", this, SLOT(slotInsertFile()));

      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()))->setEnabled(!m_sessionActive);
      menu.addAction("Run Calibration", this, SLOT(slotRunCalibration()))->setEnabled(!m_sessionActive);
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()))->setEnabled(!m_sessionActive);
      menu.addAction("Export Data/Spectra", this, SLOT(slotExportSpectra()))->setEnabled(!m_sessionActive);
      menu.addSeparator();

      bool enablePaste = !m_clipboard->spectraGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()));
      menu.addAction("Copy", this, SLOT(slotCopySelection()));
      menu.addAction("Paste In", this, SLOT(slotPasteSpectraAsChildren()))->setEnabled(enablePaste);
      menu.addAction("Paste Below", this, SLOT(slotPasteSpectraAsSiblings()))->setEnabled(enablePaste);
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cSpectraFileItemType) {
      // A File Item
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(item);

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));

      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()))->setEnabled(!m_sessionActive);
      menu.addAction("Run Calibration", this, SLOT(slotRunCalibration()))->setEnabled(!m_sessionActive);
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()))->setEnabled(!m_sessionActive);
      menu.addAction("Export Data/Spectra", this, SLOT(slotExportSpectra()))->setEnabled(!m_sessionActive);
      menu.addSeparator();

      // Can't cut, delete or paste below an item that is a child of a directory item
      bool enableCutDel = (projItem->parent() && projItem->parent()->type() != cSpectraDirectoryItemType);
      bool enablePaste = enableCutDel && !m_clipboard->spectraGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()))->setEnabled(enableCutDel);
      menu.addAction("Copy", this, SLOT(slotCopySelection()));
      menu.addAction("Paste Below", this, SLOT(slotPasteSpectraAsSiblings()))->setEnabled(enablePaste);
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()))->setEnabled(enableCutDel);
      menu.addSeparator();
      menu.addAction("Properties", this, SLOT(slotShowPropertiesSelection()));
    }
    else if (itemType == cSpectraBranchItemType) {
      // A Spectra Branch (Raw Spectra)
      menu.addAction("New Folder...", this, SLOT(slotCreateFolder()));
      menu.addAction("Insert Directory...", this, SLOT(slotInsertDirectory()));
      menu.addAction("Insert File...", this, SLOT(slotInsertFile()));

      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()))->setEnabled(!m_sessionActive);
      menu.addAction("Run Calibration", this, SLOT(slotRunCalibration()))->setEnabled(!m_sessionActive);
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()))->setEnabled(!m_sessionActive);
      menu.addAction("Export Data/Spectra", this, SLOT(slotExportSpectra()))->setEnabled(!m_sessionActive);
      menu.addSeparator();

      // cant remove this item - refers to all children
      bool enableCutCopyDel = item->childCount();
      bool enablePaste = !m_clipboard->spectraGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()))->setEnabled(enableCutCopyDel);
      menu.addAction("Copy", this, SLOT(slotCopySelection()))->setEnabled(enableCutCopyDel);
      menu.addAction("Paste In", this, SLOT(slotPasteSpectraAsChildren()))->setEnabled(enablePaste);
    }
    else if (itemType == cAnalysisWindowBranchItemType) {
      // Analysis Window Branch
      menu.addAction("New Analysis Window...", this, SLOT(slotCreateAnalysisWindow()));
      menu.addSeparator();

      // cant remove this item - refers to all children
      bool enableCutCopyDel = item->childCount();
      bool enablePaste = !m_clipboard->analysisWindowGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()))->setEnabled(enableCutCopyDel);
      menu.addAction("Copy", this, SLOT(slotCopySelection()))->setEnabled(enableCutCopyDel);
      menu.addAction("Paste In", this, SLOT(slotPasteAnalysisWindows()))->setEnabled(enablePaste);
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()))->setEnabled(enableCutCopyDel);
    }
    else if (itemType == cAnalysisWindowItemType) {
      // Analysis Window
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(item);

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("New Analysis Window...", this, SLOT(slotCreateAnalysisWindow()));
      menu.addAction("Rename...", this, SLOT(slotRenameAnalysisWindow()));
      menu.addAction("View Cross Sections", this, SLOT(slotViewCrossSections()))->setEnabled(!m_sessionActive);
      menu.addAction("Properties...", this, SLOT(slotEditAnalysisWindow()));
      menu.addSeparator();

      bool enablePaste = !m_clipboard->analysisWindowGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()));
      menu.addAction("Copy", this, SLOT(slotCopySelection()));
      menu.addAction("Paste", this, SLOT(slotPasteAnalysisWindows()))->setEnabled(enablePaste);
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cProjectItemType) {
      // Project
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(item);

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Rename...", this, SLOT(slotRenameProject()));
      menu.addAction("New Project...", this, SLOT(slotCreateProject()));
      menu.addAction("Properties...", this, SLOT(slotEditProject()));

      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()))->setEnabled(!m_sessionActive);
      menu.addAction("Run Calibration", this, SLOT(slotRunCalibration()))->setEnabled(!m_sessionActive);
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()))->setEnabled(!m_sessionActive);
      menu.addAction("Export Data/Spectra", this, SLOT(slotExportSpectra()))->setEnabled(!m_sessionActive);
      menu.addSeparator();

      bool enablePaste = !m_clipboard->projectGroupIsEmpty();

      menu.addAction("Cut", this, SLOT(slotCutSelection()));
      menu.addAction("Copy", this, SLOT(slotCopySelection()));
      menu.addAction("Paste", this, SLOT(slotPasteProjects()))->setEnabled(enablePaste);
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }

  }
  else {
    // must be an empty tree
    bool enablePaste = !m_clipboard->projectGroupIsEmpty();

    menu.addAction("New Project...", this, SLOT(slotCreateProject()));
    menu.addSeparator();
    menu.addAction("Paste", this, SLOT(slotPasteProjects()))->setEnabled(enablePaste);
  }


  // last item - hide/show details
  menu.addSeparator();
  menu.addAction(m_colWidthList.isEmpty() ? "Hide Details" : "Show Details",
                 this, SLOT(slotToggleDisplayDetails()));

  menu.exec(e->globalPos()); // a slot will do the rest if appropriate
}

QTreeWidgetItem *CWProjectTree::locateByPath(const QStringList &path)
{
  QTreeWidgetItem *item = NULL;
  QTreeWidgetItem *p = NULL;
  int i;
  QList<QString>::const_iterator it = path.begin();
  if (it == path.end())
    return NULL;

  // find the top-level item
  i = 0;
  while (i < topLevelItemCount() && (p = topLevelItem(i))->text(0) != *it) ++i;
  if (i >= topLevelItemCount())
    return NULL;

  // found top level item ... continue
  ++it;
  while (it != path.end()) {
    i = 0;
    while (i < p->childCount() && (item = p->child(i))->text(0) != *it) ++i;
    if (i >= p->childCount())
      return NULL;

    p = item;
    ++it;
  }
  return p;
}

QTreeWidgetItem *CWProjectTree::locateProjectByName(const QString &projectName)
{
  QTreeWidgetItem *p = NULL;
  int i = 0;

  // must be a top-level item - try and find it
  while (i < topLevelItemCount() && (p = topLevelItem(i))->text(0) != projectName) ++i;
  if (i < topLevelItemCount())
    return p;

  return NULL;
}

void CWProjectTree::savePreferences(void)
{
  if (m_colWidthList.empty()) {
    QList<int> widthList;
    for (int i=0; i<3; ++i)
      widthList.push_back(columnWidth(i));

    CPreferences::instance()->setColumnWidthList("ProjectTree", widthList);
  }
  else
    CPreferences::instance()->setColumnWidthList("ProjectTree", m_colWidthList);
}

void CWProjectTree::buildAndStartSession(CSession::eMode sessionType)
{
  CSession *session = new CSession(sessionType);

  // normalise the selection and then traverse to build session
  QList<QTreeWidgetItem*> items = CWProjectTree::normalize(selectedItems());

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    // only consider enabled items
    CProjectTreeItem *tmp = dynamic_cast<CProjectTreeItem*>(*it);
    if (tmp && tmp->isEnabled())
      CWProjectTree::buildSession(session, tmp);
    ++it;
  }

  // safely dispatch the session to attached slots by wrapping in a reference
  // counting pointer.

  shared_ptr<CSession> ptr(session);

  emit signalStartSession(ptr);

}

//------------------------------------------------------------------------------
// Interface for editors
//------------------------------------------------------------------------------

QString CWProjectTree::editInsertNewProject(const QString &projectName, CProjectItem **itemCreated)
{
  if (CWorkSpace::instance()->createProject(projectName.toStdString())) {
    // created the project
    CProjectItem *item = new CProjectItem(projectName);
    addTopLevelItem(item);
    item->setExpanded(true);

    if (itemCreated != NULL) *itemCreated = item;
  } else {
    return QString("A project with that name already exists.");
  }

  // success falls through
  return QString();
}

QString CWProjectTree::editRenameProject(QTreeWidgetItem *item, const QString &projectName)
{
  if (item && item->type() == cProjectItemType) {

    if (CWorkSpace::instance()->renameProject(item->text(0).toStdString(), projectName.toStdString())) {
      item->setText(0, projectName);
    }
    else {
      // explain why it failed ...
      if (CWorkSpace::instance()->findProject(item->text(0).toStdString()))
    return QString("A project with that name already exists.");
      else
    return QString("The project no longer exists.");
    }
  }
  else
    return QString("The item is not a project.");

  return QString();
}

QString CWProjectTree::editInsertNewFolder(QTreeWidgetItem *parent, const QString &folderName, CSpectraFolderItem **itemCreated)
{
  if (parent && (parent->type() == cSpectraBranchItemType || parent->type() == cSpectraFolderItemType)) {

    // first make sure that the parent does not already have a child with this name
    QTreeWidgetItem *item = CWProjectTree::locateChildByName(parent, folderName);
    if (!item) {
      CSpectraFolderItem *tmp = new CSpectraFolderItem(parent, folderName);

      if (itemCreated != NULL) *itemCreated = tmp;
      emit signalSpectraTreeChanged();
    }
    else
      return QString("The parent already contains a folder or file with that name.");
  }
  else
    return QString("The parent cannot have a folder as a child.");

  // success falls through
  return QString();
}

QString CWProjectTree::editRenameFolder(QTreeWidgetItem *item, const QString &newFolderName)
{
  if (item && item->type() == cSpectraFolderItemType) {

    // first make sure that the parent does not already have a child with this name
    QTreeWidgetItem *sibling = CWProjectTree::locateChildByName(item->parent(), newFolderName);
    if (sibling) {
      if (sibling != item)
    return QString("The parent already has a folder, file or directory with that name.");
      // do nothing if nothing changed (and consider it a successful rename)
    }
    else {
      item->setText(0, newFolderName);
      emit signalSpectraTreeChanged();
    }
  }
  else
    return QString("The item is not a folder.");

  return QString();
}

QString CWProjectTree::editInsertNewAnalysisWindow(QTreeWidgetItem *parent, const QString &windowName,
                           const QString &preceedingWindowName,
                           CAnalysisWindowItem **itemCreated)
{
  // Window Order is important

  if (parent && parent->type() == cAnalysisWindowBranchItemType) {

    QTreeWidgetItem *projItem = CWProjectTree::projectItem(parent);
    if (!projItem) {
      // corrupt system
      return QString("The project tree is corrupt.");
    }
    // locate the preceeding window
    QTreeWidgetItem *preceeding = NULL;
    int index = 0;
    if (!preceedingWindowName.isEmpty()) {
      // try and locate the preceeding item
      while (index < parent->childCount()) {
    preceeding = parent->child(index);
    if (preceeding->text(0) == preceedingWindowName)
      break;
    ++index;
      }
    }
    if (CWorkSpace::instance()->createAnalysisWindow(projItem->text(0).toStdString(), windowName.toStdString(), preceedingWindowName.toStdString())) {

      CAnalysisWindowItem *tmp = new CAnalysisWindowItem(parent, preceeding, windowName);

      if (itemCreated != NULL) *itemCreated = tmp;
    }
    else
      return QString("The project already contains an analysis window with that name.");
  }
  else
    return QString("The parent cannot have an analysis window as a child.");

  // success falls through
  return QString();
}

QString CWProjectTree::editRenameAnalysisWindow(QTreeWidgetItem *item, const QString &newWindowName)
{
  if (item && item->type() == cAnalysisWindowItemType) {

    QTreeWidgetItem *projItem = CWProjectTree::projectItem(item);
    if (!projItem) {
      // corrupt system
      return QString("The project tree is corrupt.");
    }
    if (CWorkSpace::instance()->renameAnalysisWindow(projItem->text(0).toStdString(), item->text(0).toStdString(), newWindowName.toStdString())) {
      item->setText(0, newWindowName);
    }
    else {
      // see why it failed
      if (CWorkSpace::instance()->findAnalysisWindow(projItem->text(0).toStdString(), item->text(0).toStdString()))
    return QString("The project already has an analysis window with that name.");
      else
    return QString("The project or analysis window no longer exists.");
    }
  }
  else
    return QString("The item is not an analysis window.");

  return QString();
}

QString CWProjectTree::editInsertDirectory(QTreeWidgetItem *parent, const QString &directoryPath,
                       const QString &fileFilters, bool includeSubDirs,
                       CSpectraDirectoryItem **itemCreated)
{

  if (parent && (parent->type() == cSpectraBranchItemType || parent->type() == cSpectraFolderItemType)) {
    // split the filter text into a list of file filter strings - an empty list means the filter is '*'
    QStringList filters;

    if (!fileFilters.isEmpty()) {
      if (fileFilters.contains(';')) {
    // split on ';' - NOTE whitespace is significant
    filters = fileFilters.split(';', Qt::SkipEmptyParts);
      }
      else {
    // split on whitespace
    filters = fileFilters.split(QRegularExpression("\\s+"));
      }
    }

    // the directory must exist
    QDir directory(directoryPath);

    if (directory.exists()) {
      // create a new directory item
      int fileCount;
      CSpectraDirectoryItem *dirItem = new CSpectraDirectoryItem(0, directory, filters,
                                 includeSubDirs,
                                 &fileCount);

      if (fileCount) {
    parent->addChild(dirItem);

    if (itemCreated != NULL) *itemCreated = dirItem;
    emit signalSpectraTreeChanged();
      }
      else {
    // empty file count ...
    delete dirItem;
    return QString("No files matched the file filters specified.");
      }
    }
    else {
      QString msg = "The directory ";
      msg += directoryPath;
      msg += " does not exist.";
      return msg;
    }
  }
  else {
    return QString("The parent cannot have a directory as a child.");
  }

  // success falls through to return the null string
  return QString();
}

QString CWProjectTree::editChangeDirectoryProperties(QTreeWidgetItem *item,
                             const QString &fileFilters, bool includeSubDirs)
{
  if (item && item->type() == cSpectraDirectoryItemType) {
    // split the filter text into a list of file filter strings - an empty list means the filter is '*'
    QStringList filters;

    if (!fileFilters.isEmpty()) {
      if (fileFilters.contains(';')) {
    // split on ';' - NOTE whitespace is significant
    filters = fileFilters.split(';', Qt::SkipEmptyParts);
      }
      else {
    // split on whitespace
    filters = fileFilters.split(QRegularExpression("\\s+"));
      }
    }

    CSpectraDirectoryItem *dirItem = dynamic_cast<CSpectraDirectoryItem*>(item);
    if (dirItem != NULL) {
      dirItem->changeProperties(filters, includeSubDirs);
      emit signalSpectraTreeChanged();
      return QString(); // success
    }

  }

  return QString("Not a directory item");
}

const QIcon& CWProjectTree::getIcon(int type)
{
  switch (type) {
  case cAnalysisWindowItemType:
    {
      if (!m_windowIcon)
        m_windowIcon = new QIcon(":/icons/project_window_16.png");
      return *m_windowIcon;
    }
    break;
  case cSpectraFolderItemType:
    {
      if (!m_folderIcon)
        m_folderIcon = new QIcon(":/icons/project_folder_16.png");
      return *m_folderIcon;
    }
    break;
  case cSpectraDirectoryItemType:
    {
      if (!m_directoryIcon)
        m_directoryIcon = new QIcon(":/icons/project_directory_16.png");
      return *m_directoryIcon;
    }
    break;
  default:
  case cSpectraFileItemType:
    {
      if (!m_fileIcon)
        m_fileIcon = new QIcon(":/icons/project_file_16.png");
      return *m_fileIcon;
    }
    break;
  }
}

//------------------------------------------------------------------------------
// Loading from configuration file
//------------------------------------------------------------------------------

void CWProjectTree::removeAllContent(void)
{
  // clear the current projects - bottom up - this delete all projects from the workspace
  int i = topLevelItemCount();
  while (i > 0) {
    delete takeTopLevelItem(--i);
  }

  emit signalSpectraTreeChanged();
}

QString CWProjectTree::loadConfiguration(const vector<CProjectConfigItem>& itemList)
{
  // walk the list and create ...
  QString errStrPartial,errStrTotal=QString();
  QTreeWidgetItem *item;
  CProjectItem *projItem;
  CAnalysisWindowItem *awItem;
  mediate_project_t *projProp;
  mediate_analysis_window_t *awProp;

  CWorkSpace *ws = CWorkSpace::instance();

  // first make sure it is clear ...
  removeAllContent();

  // use the edit* interface to get reasonable error messages.

  for (const auto& config_item : itemList) {
    QString projName(QString::fromStdString(config_item.name()));

    // create the project item
    projItem = NULL;
    errStrPartial = editInsertNewProject(projName, &projItem);

    if (!errStrPartial.isNull())
      return errStrPartial;

    assert(projItem && projItem->childCount() == 2); // sanity check

    // enable (or disable) the project
    projItem->setEnabled(config_item.isEnabled());

    // locate the properties in the workspace then copy
    projProp = CWorkSpace::instance()->findProject(projName.toStdString());
    assert(projProp != NULL);
    *projProp = *(config_item.properties()); // blot copy

    // update useCount for count the symbols used in the calibration
    for (int i=0; i < projProp->calibration.crossSectionList.nCrossSection; ++i)
      ws->incrementUseCount(projProp->calibration.crossSectionList.crossSection[i].symbol);

    ws->modifiedProjectProperties(projName.toStdString()); // notification to any observers

    item = projItem->child(0); // raw spectra node for the project

    // recursive construction of the project tree from the config tree ...
    const CProjectConfigTreeNode *firstChild = config_item.rootNode()->firstChild().get();
    // top-down construction of the tree ...
    errStrPartial = CWProjectTree::buildRawSpectraTree(item, firstChild);

    if (!errStrPartial.isNull())
      collateErrorMessage(errStrTotal,errStrPartial);

    // add any analysis windows
    item = projItem->child(1); // the analysis window branch node

    QString precedingWindowName;
    auto awList = config_item.analysisWindowItems();
    auto awIt = awList.begin();
    while (awIt != awList.end()) {
      QString awName(QString::fromStdString(awIt->name()));

      // create the item with the edit iterface
      errStrPartial = editInsertNewAnalysisWindow(item, awName, precedingWindowName, &awItem);
      if (!errStrPartial.isNull())
       collateErrorMessage(errStrTotal,errStrPartial);

      // locate the properties in the workspace and copy
      awProp = CWorkSpace::instance()->findAnalysisWindow(projName.toStdString(), awName.toStdString());
      assert(awProp != NULL);
      *awProp = *(awIt->properties()); // blot copy
      // update useCount for the symbols used in the molecules
      for (int i=0; i < awProp->crossSectionList.nCrossSection; ++i)
    ws->incrementUseCount(awProp->crossSectionList.crossSection[i].symbol);

      // enable or disable the analyis window
      awItem->setEnabled(awIt->isEnabled());

      precedingWindowName = awName;
      ++awIt;
    }
  }

  return errStrTotal;
}

QString CWProjectTree::buildRawSpectraTree(QTreeWidgetItem *parent, const CProjectConfigTreeNode *childConfigItem)
{
  // recursive construction ...
  QString errStr;

  // create items for childConfigItem and its siblings
  while (childConfigItem != NULL) {

    switch (childConfigItem->type()) {
    case CProjectConfigTreeNode::eFile:
      {
        QFileInfo fileInfo(QString::fromStdString(childConfigItem->name()));
        // make sure it exists ...
        if (fileInfo.exists()) {
          CSpectraFileItem *fileItem = new CSpectraFileItem(parent, fileInfo);
          fileItem->setEnabled(childConfigItem->isEnabled());
          // should not have children ...
        }
        else {
          QString msg("File ");
          msg += fileInfo.filePath();
          msg += " does not exist.";
          collateErrorMessage(errStr, msg);
        }
      }
      break;
    case CProjectConfigTreeNode::eFolder:
      {
    CSpectraFolderItem *folderItem = NULL;

    collateErrorMessage(errStr, editInsertNewFolder(parent,
                                                    QString::fromStdString(childConfigItem->name()), &folderItem));

    if (folderItem != NULL) {
      folderItem->setEnabled(childConfigItem->isEnabled());

      // can have children ...
      const CProjectConfigTreeNode *firstChild = childConfigItem->firstChild().get();
      if (firstChild != NULL) {
        collateErrorMessage(errStr, CWProjectTree::buildRawSpectraTree(folderItem, firstChild));
      }
    }
      }
      break;
    case CProjectConfigTreeNode::eDirectory:
      {
    CSpectraDirectoryItem *dirItem = NULL;

    collateErrorMessage(errStr, editInsertDirectory(parent,
                                                    QString::fromStdString(childConfigItem->name()),
                                                    QString::fromStdString(childConfigItem->filter()),
                            childConfigItem->recursive(), &dirItem));
    if (dirItem != NULL)
      dirItem->setEnabled(childConfigItem->isEnabled());

    // should not have children ...
      }
      break;
    }

    childConfigItem = childConfigItem->nextSibling().get();
  }

  return errStr;
}

//------------------------------------------------------------------------------
// SLOTS
//------------------------------------------------------------------------------

void CWProjectTree::slotEnable()
{
  // setEnabled(true) the entire selection

  QList<QTreeWidgetItem*> items = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    CProjectTreeItem *p = static_cast<CProjectTreeItem*>(*it);
    p->setEnabled(true);

    ++it;
  }

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotDisable()
{
  // setEnabled(false) the entire selection

  QList<QTreeWidgetItem*> items = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    CProjectTreeItem *p = static_cast<CProjectTreeItem*>(*it);
    p->setEnabled(false);

    ++it;
  }

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotToggleEnable()
{
  // invert the enable status of the entire selection

  QList<QTreeWidgetItem*> items = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    CProjectTreeItem *p = static_cast<CProjectTreeItem*>(*it);
    p->setEnabled(!p->isEnabled());

    ++it;
  }

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotToggleDisplayDetails()
{
  if (m_colWidthList.empty()) {
    // hide the details
    for (int i=0; i<3; ++i)
      m_colWidthList.push_back(columnWidth(i));

    hideColumn(1);
    hideColumn(2);
    //    hideColumn(3);

    emit signalWidthModeChanged(cProjectTreeHideDetailMode);
  }
  else {
    // show the details
    showColumn(1);
    showColumn(2);
    //    showColumn(3);

    // restore the sizes
    for (int i=0; i<3; ++i)
      setColumnWidth(i, m_colWidthList.at(i));
    m_colWidthList.clear();

    emit signalWidthModeChanged(cProjectTreeShowDetailMode);
  }
}

void CWProjectTree::slotCreateProject()
{
  CWEditor *nameEditor = new  CWProjectNameEditor(this);
  m_activeContext->addEditor(nameEditor);
}

void CWProjectTree::slotRenameProject()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cProjectItemType) {

      CWEditor *nameEditor = new  CWProjectNameEditor(this, item);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotEditProject()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cProjectItemType) {

      CWEditor *propEditor = new  CWProjectPropertyEditor(item->text(0), this);
      m_activeContext->addEditor(propEditor);
    }
  }
}

void CWProjectTree::slotEditAnalysisWindow()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cAnalysisWindowItemType) {
      QTreeWidgetItem *projectItem = CWProjectTree::projectItem(item);
      if (projectItem) {

    CWEditor *awEditor = new  CWAnalysisWindowPropertyEditor(projectItem->text(0), item->text(0), this);
    m_activeContext->addEditor(awEditor);
      }
    }
  }
}

void CWProjectTree::slotRefreshDirectories()
{
  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> items = CWProjectTree::directoryItems(selectedItems());

  if (items.count() > 1)
    items = CWProjectTree::normalize(items);

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    // only for directory entries - aleady selected by type()
    CSpectraDirectoryItem *dirItem = dynamic_cast<CSpectraDirectoryItem*>(*it);
    if (dirItem)
      dirItem->refreshBranch();

    ++it;
  }
}

void CWProjectTree::slotCreateFolder()
{
  // expect selection has one item and it is either
  // a spectra branch or folder item

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraBranchItemType || parent->type() == cSpectraFolderItemType) {

      CWEditor *nameEditor = new  CWProjectFolderNameEditor(this, parent, true);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotRenameFolder()
{
  // expect selection has one item and it is a
  // a spectra folder item

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType) {

      CWEditor *dirEditor = new  CWProjectFolderNameEditor(this, parent, false);
      m_activeContext->addEditor(dirEditor);
    }
  }
}

void CWProjectTree::slotInsertFile()
{
  // expect selection has one item and it is a
  // a spectra folder item or the raw spectra branch
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType || parent->type() == cSpectraBranchItemType) {

      QTreeWidgetItem *projItem = CWProjectTree::projectItem(parent);
      if (projItem == NULL)
    return;

      // access to the project data
      const mediate_project_t *projData = CWorkSpace::instance()->findProject(projItem->text(0).toStdString());
      if (projData == NULL)
    return;

      CPreferences *prefs = CPreferences::instance();

      QString extension = prefs->fileExtension("Instrument", projData->instrumental.format, "spe");
      QString filter;
      QTextStream stream(&filter);

      stream << "Spectra (*." << extension << ");;All files(*)";

      QStringList files = QFileDialog::getOpenFileNames(0, "Select one or more spectra files",
                            prefs->directoryName("Spectra"), filter);

      // Qt Documentation says copy ??
      if (!files.isEmpty()) {
    QStringList copy = files;
    QList<QString>::iterator it = copy.begin();
    if (it != copy.end()) {
      // store the preferences
      prefs->setFileExtensionGivenFile("Instrument", projData->instrumental.format, *it);
      prefs->setDirectoryNameGivenFile("Spectra", *it);
    }
    while (it != copy.end()) {
      // create the items
      QFileInfo fileInfo(*it);
      if (fileInfo.exists())
        new CSpectraFileItem(parent, fileInfo);
      ++it;
    }

    emit signalSpectraTreeChanged();
      }
    }
  }
}

void CWProjectTree::slotInsertDirectory()
{
  // expect selection has one item and it is a
  // a CSpectraFolderItem or a CSpectraBranchItem
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType || parent->type() == cSpectraBranchItemType) {

      CWEditor *nameEditor = new  CWProjectDirectoryEditor(this, parent);
      m_activeContext->addEditor(nameEditor);

    }
  }
}

void CWProjectTree::slotEditDirectory()
{
  // expect selection has one item and it is a
  // a CSpectraDirectoryItem
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *dirItem = items.front();
    if (dirItem->type() == cSpectraDirectoryItemType) {

      CWEditor *nameEditor = new  CWProjectDirectoryEditor(this, dirItem);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotCreateAnalysisWindow()
{
  // expect selection has one item and is an Anaylsis Window Branch or An analysis Window Item

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QString preceedingWindowName;

    QTreeWidgetItem *item = items.front();
    if (item && item->type() == cAnalysisWindowItemType) {
      preceedingWindowName = item->text(0);
      item = item->parent();
    }
    // item MUST be of type Analysis Window Branch
    if (item && item->type() == cAnalysisWindowBranchItemType) {

      CWEditor *nameEditor = new  CWProjectAnalysisWindowNameEditor(this, item, preceedingWindowName, true);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotRenameAnalysisWindow()
{
  // expect selection has one item and is an Anaylsis Window

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cAnalysisWindowItemType) {

      CWEditor *nameEditor = new  CWProjectAnalysisWindowNameEditor(this, item, QString(), false);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotRunAnalysis()
{
  if (m_sessionActive)
    QMessageBox::information(this, "Run Analysis", "A session is currently active.");
  else
    buildAndStartSession(CSession::Analyse);
}

void CWProjectTree::slotRunCalibration()
{
  if (m_sessionActive)
    QMessageBox::information(this, "Run Calibration", "A session is currently active.");
  else
    buildAndStartSession(CSession::Calibrate);
}

void CWProjectTree::slotBrowseSpectra()
{
  if (m_sessionActive)
    QMessageBox::information(this, "Browse Spectra", "A session is currently active.");
  else
    buildAndStartSession(CSession::Browse);
}

void CWProjectTree::ExportSpectra()
 {
    if (m_sessionActive)
      QMessageBox::information(this, "Export Data/Spectra", "A session is currently active.");
    else
      buildAndStartSession(CSession::Export);
 }

void CWProjectTree::slotExportSpectra()
 {
  // expect selection has one item and it is a
  // a CSpectraFolderItem or a CSpectraBranchItem

  QList<QTreeWidgetItem*> items = selectedItems();
  QTreeWidgetItem *parent = items.front();
  QTreeWidgetItem *projItem = CWProjectTree::projectItem(parent);

  if (parent->type()!=cAnalysisWindowBranchItemType)
   {
     mediate_project_t *projProp;

    // locate the properties in the workspace then copy
    projProp = CWorkSpace::instance()->findProject(projItem->text(0).toStdString());
    assert(projProp != NULL);

    CWEditor *exportEditor = new  CWProjectExportEditor(this,parent,projItem->text(0),&projProp->export_spectra,projProp->instrumental.format);
    m_activeContext->addEditor(exportEditor);
   }
}

void CWProjectTree::slotViewCrossSections()
{
  if (m_sessionActive)
    QMessageBox::information(this, "View Cross Sections", "A session is currently active.");
  else {

    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.count() == 1) {
      QTreeWidgetItem *item = items.front();
      if (item->type() == cAnalysisWindowItemType) {

    QTreeWidgetItem *projItem = CWProjectTree::projectItem(item);
    if (projItem != NULL) {
      const mediate_analysis_window_t *aw = CWorkSpace::instance()->findAnalysisWindow(projItem->text(0).toStdString(), item->text(0).toStdString());

      if (aw != NULL) {
        shared_ptr<CViewCrossSectionData> ptr(new CViewCrossSectionData(aw));
        emit signalViewCrossSections(ptr);
      }
    }
      }
    }

  }
}

void CWProjectTree::slotDeleteSelection()
{
  // normalize the selection
  QList<QTreeWidgetItem*> items = CWProjectTree::normalize(selectedItems());

  int type;
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    // cant delete the Raw Spectra or Analysis Window branches (instead delete all of their children)
    type = (*it)->type();
    if (type == cSpectraBranchItemType) {
      // delete all children
      QList<QTreeWidgetItem*> kids = (*it)->takeChildren();
      QList<QTreeWidgetItem*>::iterator kIt = kids.begin();
      while (kIt != kids.end()) {
    delete *kIt;
    ++kIt;
      }
    }
    else if (type == cAnalysisWindowBranchItemType) {
      // delete all children - while still in the tree ... delete in reverse order
      int index;
      while ((index = (*it)->childCount()) > 0) {
    QTreeWidgetItem *awItem = (*it)->child(--index);
    CAnalysisWindowItem::destroyItem(awItem);
      }
    }
    else if (type == cAnalysisWindowItemType) {

      CAnalysisWindowItem::destroyItem(*it);
    }
    else if ((*it)->parent() == NULL) {
      // a project item
      CProjectItem::destroyItem(this, *it);
    }
    else {
      QTreeWidgetItem *p = (*it)->parent();
      // cant remove the children of directory items
      if (p->type() !=  cSpectraDirectoryItemType)
        delete p->takeChild(p->indexOfChild(*it));
    }

    ++it;
  }

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotCutSelection()
{
  // normalize the selection
  QList<QTreeWidgetItem*> items = CWProjectTree::normalize(selectedItems());

  //mark the clipboard, ready for adding items
  m_clipboard->beginInsertItems();

  CWorkSpace *ws = CWorkSpace::instance();

  int type;
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    type = (*it)->type();

    if (type == cSpectraBranchItemType) {
      // cant cut the Raw Spectra - cut its children instead.
      QList<QTreeWidgetItem*> kids = (*it)->takeChildren();
      m_clipboard->insertRawSpectraItems(kids);
    }
    else if (type == cAnalysisWindowBranchItemType) {
      // cant cut the Analysis Window branches - cut its children instead.
      // CAREFULL - the items MUST destroyed while in the tree ...
      QString projectName = (*it)->parent()->text(0);

      mediate_analysis_window_t *awProp;

      while ((*it)->childCount()) {
    QTreeWidgetItem *awItem = (*it)->child(0);
    awProp = ws->findAnalysisWindow(projectName.toStdString(), awItem->text(0).toStdString());
    assert(awProp);
    // make a deep copy of the data to hand over to the clipboard
    mediate_analysis_window_t *awData = new mediate_analysis_window_t;
    *awData = *awProp; // blot copy
    m_clipboard->insertAnalysisWindow(awData); // hand over the copy

    CAnalysisWindowItem::destroyItem(awItem); // destroy the item (and removes the data from the workspace)
      }
    }
    else if (type == cAnalysisWindowItemType) {

      QTreeWidgetItem *projItem = CWProjectTree::projectItem(*it);
      assert(projItem);
      mediate_analysis_window_t *awProp = ws->findAnalysisWindow(projItem->text(0).toStdString(), (*it)->text(0).toStdString());
      assert(awProp);
      // make a deep copy of the data to hand over to the clipboard
      mediate_analysis_window_t *awData = new mediate_analysis_window_t;
      *awData = *awProp; // blot copy
      m_clipboard->insertAnalysisWindow(awData); // hand over the copy

      CAnalysisWindowItem::destroyItem(*it);
    }
    else if ((*it)->parent() == NULL) {
      // a project item ... Collect all the bits and pieces for the project
      QString projectName = (*it)->text(0);
      mediate_project_t *projProp = ws->findProject(projectName.toStdString());
      assert(projProp);
      // make a deep copy of the data to hand over to the clipboard
      mediate_project_t *projData = new mediate_project_t;
      *projData = *projProp;
      // get a deep copy of the analysis windows for this project
      auto awList = ws->analysisWindowList(projectName.toStdString());
      // steal the raw spectra items from the tree
      QTreeWidgetItem *rawSpectraItem = (*it)->child(0);
      assert(rawSpectraItem);
      QList<QTreeWidgetItem*> rawSpectraList = rawSpectraItem->takeChildren();

      // now have all of the components required for a the clipboard
      m_clipboard->insertProject(projectName, projData, awList, rawSpectraList);

      CProjectItem::destroyItem(this, *it);
    }
    else {
      // must be a raw spectra item (directory, folder or file)
      QTreeWidgetItem *p = (*it)->parent();
      // cant remove the children of directory items .. just ignore them
      if (p->type() !=  cSpectraDirectoryItemType) {
        m_clipboard->insertRawSpectraItem(p->takeChild(p->indexOfChild(*it)));
      }
    }

    ++it;
  }

  m_clipboard->endInsertItems();

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotCopySelection()
{
  // normalize the selection
  QList<QTreeWidgetItem*> items = CWProjectTree::normalize(selectedItems());

  //mark the clipboard, ready for adding items
  m_clipboard->beginInsertItems();

  CWorkSpace *ws = CWorkSpace::instance();

  int type;
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    type = (*it)->type();

    if (type == cSpectraBranchItemType) {
      // cant copy the Raw Spectra - copy (clone) its children instead.
      int index = 0;
      while (index < (*it)->childCount()) {
    m_clipboard->insertRawSpectraItem((*it)->child(index)->clone());
    ++index;
      }
    }
    else if (type == cAnalysisWindowBranchItemType) {
      // cant copy the Analysis Window branch - copy its children instead.

      QString projectName = (*it)->parent()->text(0);

      mediate_analysis_window_t *awProp;

      while ((*it)->childCount()) {
        QTreeWidgetItem *awItem = (*it)->child(0);
        awProp = ws->findAnalysisWindow(projectName.toStdString(), awItem->text(0).toStdString());
        assert(awProp);
        // make a deep copy of the data to hand over to the clipboard
        mediate_analysis_window_t *awData = new mediate_analysis_window_t;
        *awData = *awProp; // blot copy
        m_clipboard->insertAnalysisWindow(awData); // hand over the copy
      }
    }
    else if (type == cAnalysisWindowItemType) {

      QTreeWidgetItem *projItem = CWProjectTree::projectItem(*it);
      assert(projItem);
      mediate_analysis_window_t *awProp = ws->findAnalysisWindow(projItem->text(0).toStdString(), (*it)->text(0).toStdString());
      assert(awProp);
      // make a deep copy of the data to hand over to the clipboard
      mediate_analysis_window_t *awData = new mediate_analysis_window_t;
      *awData = *awProp; // blot copy
      m_clipboard->insertAnalysisWindow(awData); // hand over the copy
    }
    else if ((*it)->parent() == NULL) {
      // a project item ... Collect all the bits and pieces for the project
      QString projectName = (*it)->text(0);
      mediate_project_t *projProp = ws->findProject(projectName.toStdString());
      assert(projProp);
      // make a deep copy of the data to hand over to the clipboard
      mediate_project_t *projData = new mediate_project_t;
      *projData = *projProp;
      // get a deep copy of the analysis windows for this project
      auto awList = ws->analysisWindowList(projectName.toStdString());
      // steal the raw spectra items from the tree
      QTreeWidgetItem *rawSpectraItem = (*it)->child(0);
      assert(rawSpectraItem);

      QList<QTreeWidgetItem*> rawSpectraList;
      int index = 0;
      while (index < rawSpectraItem->childCount()) {
    rawSpectraList.push_back(rawSpectraItem->child(index)->clone());
    ++index;
      }

      // now have all of the components required for a the clipboard
      m_clipboard->insertProject(projectName, projData, awList, rawSpectraList);
    }
    else {
      // must be a raw spectra item (directory, folder or file) - just copy (clone) it
      m_clipboard->insertRawSpectraItem((*it)->clone());
    }

    ++it;
  }

  m_clipboard->endInsertItems();
}

void CWProjectTree::slotPaste()
{
  // non-specialized paste ... MUST have a single item selected ...
  // redirects to a specialized paste slot. Chooses PasteAsSiblings if both
  // AsSiblings and AsChildren options are valid...

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() != 1) return;

  int itemType = items.front()->type();

  if (itemType == cSpectraDirectoryItemType || itemType == cSpectraFileItemType || itemType == cSpectraFolderItemType) {
    slotPasteSpectraAsSiblings();
  }
  else if (itemType == cSpectraBranchItemType) {
    slotPasteSpectraAsChildren();
  }
  else if (itemType == cAnalysisWindowItemType || itemType == cAnalysisWindowBranchItemType) {
    slotPasteAnalysisWindows();
  }
  else if (itemType == cProjectItemType) {
    slotPasteProjects();
  }
}

void CWProjectTree::slotPasteProjects()
{
  int nProjects = m_clipboard->projectGroupSize();

  if (nProjects == 0) return; // nothing to do

  // A single project item selected or no selection
  QList<QTreeWidgetItem*> items = selectedItems();

  QTreeWidgetItem *preceedingProj = NULL;

  if (!items.isEmpty()) {
    // selection MUST be a project item, and will be the preceeding item
    preceedingProj = items.front();
    if (preceedingProj->type() != cProjectItemType)
      return; // silent bailout ...
  }

  CWorkSpace *ws = CWorkSpace::instance();

  // walk the list of projects, check the name is OK, then create items. NOTE, there is
  // no need to notify observers of the changes made to properties after the creation.
  // (BUT this is ONLY because no current observers distinguish these updates).
  // A potential ToDo ...

  int projIndex = 0;
  while (projIndex < nProjects) {
    QString projName = m_clipboard->projectGroupItemName(projIndex);
    const mediate_project_t *existingProj = ws->findProject(projName.toStdString());
    while (existingProj != NULL) {
      projName += ".Copy";
      existingProj = ws->findProject(projName.toStdString());
    }
    // safe to create a new project in the workspace
    mediate_project_t *projData = ws->createProject(projName.toStdString());
    if (projData) {
      // copy the property data from the clipboard
      *projData = *(m_clipboard->projectGroupItemProperties(projIndex)); // blot copy
      // create a tree item for this
      preceedingProj = new CProjectItem(this, preceedingProj, projName);

      QTreeWidgetItem *branchItem = preceedingProj->child(0);

      // the spectra
      QList<QTreeWidgetItem*> spectra = m_clipboard->projectGroupItemSpectraList(projIndex);
      branchItem->addChildren(spectra); // the responsibility for the items is transferred to branchItem

      // now create analysis windows ... (no checks required)
      branchItem = preceedingProj->child(1);
      QString preceedingWindowName;
      QTreeWidgetItem *preceedingWindow = NULL;

      int nWindows = m_clipboard->projectGroupItemAnalysisWindowSize(projIndex);
      int awIndex = 0;
      while (awIndex < nWindows) {
    const mediate_analysis_window_t *awDataHandle = m_clipboard->projectGroupItemAnalysisWindowProperties(projIndex, awIndex);
    QString awName(awDataHandle->name);

    mediate_analysis_window_t *awData = ws->createAnalysisWindow(projName.toStdString(), awName.toStdString(), preceedingWindowName.toStdString());
    // could assert that awData != NULL because this creation MUST work
    if (awData) {
      // copy the properties data
      *awData = *awDataHandle;
      // create the tree item ...
      preceedingWindow = new CAnalysisWindowItem(branchItem, preceedingWindow, awName);

      preceedingWindowName = awName; // ensure ordered insertion
    }

    ++awIndex;
      }

    }

    ++projIndex;
  }
}

void CWProjectTree::slotPasteAnalysisWindows()
{
  int nWindows = m_clipboard->analysisWindowGroupSize();

  if (nWindows == 0) return; // nothing to do

  // A single item must be selected (either an Analysis Window or an Analysis Window Branch)

  QList<QTreeWidgetItem*> items = selectedItems();

  if (items.isEmpty()) return;

  QString preceedingWindowName;
  QTreeWidgetItem *preceedingWindow = NULL;
  QTreeWidgetItem *parent = items.front();
  if (parent->type() == cAnalysisWindowItemType) {
    preceedingWindow = parent;
    preceedingWindowName = preceedingWindow->text(0);
    parent = parent->parent();
  }
  else if (parent->type() != cAnalysisWindowBranchItemType) {
    // invalid item type selected ... silent bailout
    return;
  }

  CWorkSpace *ws = CWorkSpace::instance();

  QString projName = CWProjectTree::projectItem(parent)->text(0);

  int awIndex = 0;
  while (awIndex < nWindows) {
    const mediate_analysis_window_t *awDataHandle = m_clipboard->analysisWindowGroupItemProperties(awIndex);
    QString awName(awDataHandle->name);

    // must be uniquely named ... because window names are size limited ... only make one ".Copy" attempt...
    mediate_analysis_window_t *awData = ws->createAnalysisWindow(projName.toStdString(), awName.toStdString(), preceedingWindowName.toStdString());
    if (awData == NULL) {
      awName += ".Copy";
      awData = ws->createAnalysisWindow(projName.toStdString(), awName.toStdString(), preceedingWindowName.toStdString());
    }
    // Is it OK to proceed ... silently skip this window if not OK ...
    if (awData) {
      // copy the properties data
      *awData = *awDataHandle;
      // NOTE, because the window name could have been changed, it MUST be reset. (no length check required)
      strcpy(awData->name, awName.toLocal8Bit().data());
      // create the tree item ...
      preceedingWindow = new CAnalysisWindowItem(parent, preceedingWindow, awName);
      preceedingWindowName = awName; // ensure ordered insertion
    }

    ++awIndex;
  }
}

void CWProjectTree::slotPasteSpectraAsSiblings()
{
  if (m_clipboard->spectraGroupIsEmpty()) return;

  // A single item must be selected and it MUST have a SpectraFolderItem or SpectraBranchItem as parent.

  QList<QTreeWidgetItem*> items = selectedItems();

  if (items.isEmpty()) return;

  QTreeWidgetItem *preceedingItem = items.front();
  QTreeWidgetItem *parent = preceedingItem->parent();

  // check the parent type is appropriate
  if (parent->type() != cSpectraFolderItemType && parent->type() != cSpectraBranchItemType) return;

  int index = parent->indexOfChild(preceedingItem);
  QList<QTreeWidgetItem*> spectra = m_clipboard->spectraGroupList();
  parent->insertChildren(++index, spectra); // the responsibility for the items is transferred to parent

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotPasteSpectraAsChildren()
{
  if (m_clipboard->spectraGroupIsEmpty()) return;

  // A single item must be selected and it MUST be SpectraFolderItem or SpectraBranchItem.

  QList<QTreeWidgetItem*> items = selectedItems();

  if (items.isEmpty()) return;

  QTreeWidgetItem *parent = items.front();

  // check the parent type is appropriate
  if (parent->type() != cSpectraFolderItemType && parent->type() != cSpectraBranchItemType) return;

  // insert at the beginning
  QList<QTreeWidgetItem*> spectra = m_clipboard->spectraGroupList();
  parent->insertChildren(0, spectra); // the responsibility for the items is transferred to parent

  emit signalSpectraTreeChanged();
}

void CWProjectTree::slotShowPropertiesSelection()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() != 1) return;

  int itemType = items.front()->type();

  if (itemType == cSpectraDirectoryItemType) {
    CSpectraDirectoryItem *dirItem = dynamic_cast<CSpectraDirectoryItem*>(items.front());
    if (dirItem) {
      // format the information into a message string
      QString msg = dirItem->directoryName();
      msg += "\nFile Filters\t: ";
      msg += dirItem->fileFilters();
      msg += "\nIncl. SubDirs\t: ";
      if (dirItem->isRecursive())
    msg += "Yes";
      else
    msg += "No";

      QMessageBox::information(this, "Directory Properties", msg);
    }
  }
  else if (itemType == cSpectraFileItemType) {
    CSpectraFileItem *fileItem = dynamic_cast<CSpectraFileItem*>(items.front());
    if (fileItem) {
      QString msg = fileItem->fullFileName();
      msg +=      "\nSize\t: ";
      msg += fileItem->fileSizeInBytes();
      msg +=      " bytes\nModified\t: ";
      msg += fileItem->dateLastModified();

      QMessageBox::information(this, "File Properties", msg);
    }
  }
}

void CWProjectTree::slotSessionRunning(bool running)
{
  m_sessionActive = running;
}

//------------------------------------------------------------------------------
// Tree Items
//------------------------------------------------------------------------------

CProjectTreeItem::CProjectTreeItem(const QStringList &strings, int type) :
  QTreeWidgetItem(strings, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(CWProjectTree *parent, int type) :
  QTreeWidgetItem(parent, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(CWProjectTree *parent, const QStringList &strings, int type) :
  QTreeWidgetItem(parent, strings, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(CWProjectTree *parent, QTreeWidgetItem *preceedingSibling, const QStringList &strings, int type) :
  QTreeWidgetItem(parent, preceedingSibling, type),
  m_enabled(true)
{
  // set the labels  ... there isn't a contructor with a preceeding item & string labels!!

  int index = 0;
  while (index < strings.size()) {
    setText(index, strings.at(index));
    ++index;
  }
}

CProjectTreeItem::CProjectTreeItem(QTreeWidgetItem *parent, int type) :
  QTreeWidgetItem(parent, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(QTreeWidgetItem *parent, const QStringList &strings, int type) :
  QTreeWidgetItem(parent, strings, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceedingSibling, const QStringList &strings, int type) :
  QTreeWidgetItem(parent, preceedingSibling, type),
  m_enabled(true)
{
  // set the labels  ... there isn't a contructor with a preceeding item & string labels!!

  int index = 0;
  while (index < strings.size()) {
    setText(index, strings.at(index));
    ++index;
  }
}

// creates an isolated item (no parent, no children)
CProjectTreeItem::CProjectTreeItem(const CProjectTreeItem &other) :
  QTreeWidgetItem(other.type()),
  m_enabled(other.m_enabled)
{
}

void CProjectTreeItem::setEnabled(bool enable)
{
  m_enabled = enable;
}

//------------------------------------------------------------------------------

CProjectItem::CProjectItem(const QString &projectName) :
  CProjectTreeItem(QStringList(projectName), cProjectItemType)
{
  // add Children for Raw Spectra and Analysis Windows
  new CSpectraBranchItem(this);
  new CAnalysisWindowBranchItem(this);
}

CProjectItem::CProjectItem(CWProjectTree *parent, QTreeWidgetItem *preceedingSibling, const QString &projectName) :
  CProjectTreeItem(parent, preceedingSibling, QStringList(projectName), cProjectItemType)
{
  // add Children for Raw Spectra and Analysis Windows
  new CSpectraBranchItem(this);
  new CAnalysisWindowBranchItem(this);
}

void CProjectItem::destroyItem(QTreeWidget *tree, QTreeWidgetItem *projItem)
{
  assert(tree && projItem && !projItem->parent());

  // there should be a corresponding project in the workspace ... this implicitly
  // destroys the analysis associated windows
  CWorkSpace::instance()->destroyProject(projItem->text(0).toStdString());

  delete tree->takeTopLevelItem(tree->indexOfTopLevelItem(projItem));
}

//CProjectItem::~CProjectItem()
//{
//  // This will recursively destroy all child items, including any analysis window items.
//  // Since destroyProject has removed the items in the workspace, it is OK that
//  // the CAnalysisWindowItem destructor is called, and not the destroyItem function.
//}

QVariant CProjectItem::data(int column, int role) const
{
  if (role == Qt::ForegroundRole) {
    return QVariant(QBrush(QColor(m_enabled ? cProjectTextColour : cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

//------------------------------------------------------------------------------

CAnalysisWindowBranchItem::CAnalysisWindowBranchItem(QTreeWidgetItem *parent) :
  CProjectTreeItem(parent, QStringList("Analysis Windows"), cAnalysisWindowBranchItemType)
{
}

//------------------------------------------------------------------------------

CSpectraBranchItem::CSpectraBranchItem(QTreeWidgetItem *parent) :
  CProjectTreeItem(parent, QStringList("Raw Spectra"), cSpectraBranchItemType)
{
}

//------------------------------------------------------------------------------

CSpectraFolderItem::CSpectraFolderItem(QTreeWidgetItem *parent, const QString &folderName) :
  CProjectTreeItem(parent, QStringList(folderName), cSpectraFolderItemType)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraFolderItemType));
}

CSpectraFolderItem::CSpectraFolderItem(const CSpectraFolderItem &other) :
  CProjectTreeItem(other)
{
  setText(0, other.text(0)); // copy the folder name
  setIcon(0, CWProjectTree::getIcon(cSpectraFolderItemType));
}

QVariant CSpectraFolderItem::data(int column, int role) const
{
  if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

QTreeWidgetItem* CSpectraFolderItem::clone() const
{
  // copy constructor makes a childless and parentless item
  CSpectraFolderItem *tmp = new CSpectraFolderItem(*this);

  // clone and insert all children
  int index = 0;
  while (index < childCount()) {
    tmp->addChild(child(index)->clone());
    ++index;
  }

  return tmp;
}

//------------------------------------------------------------------------------

CSpectraDirectoryItem::CSpectraDirectoryItem(QTreeWidgetItem *parent, const QDir &directory,
                                               const QStringList &fileFilters, bool includeSubDirectories,
                                               int *fileCount) :
  CProjectTreeItem(parent, cSpectraDirectoryItemType),
  m_directory(directory),
  m_fileFilters(fileFilters),
  m_includeSubDirectories(includeSubDirectories)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraDirectoryItemType));

  // build the file and directory tree for this directory (recursive)
  int nFiles = loadBranch();

  // set the fileCount for the entire branch if requested
  if (fileCount)
    *fileCount = nFiles;
}

CSpectraDirectoryItem::CSpectraDirectoryItem(const CSpectraDirectoryItem &other) :
  CProjectTreeItem(other),
  m_directory(other.m_directory),
  m_fileFilters(other.m_fileFilters),
  m_includeSubDirectories(other.m_includeSubDirectories)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraDirectoryItemType));

  // build the file and directory tree for this directory (recursive)
  loadBranch();
}

QVariant CSpectraDirectoryItem::data(int column, int role) const
{
  if (role == Qt::DisplayRole) {

    if (column == 0)
      return QVariant(m_directory.dirName());
    //    if (column == 1)
    //      return QVariant(m_directory.nameFilters().join("; "));
    //if (column == 3)
    //  return QVariant(m_directory.absolutePath());

    return QVariant();
  }
  else if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

QTreeWidgetItem* CSpectraDirectoryItem::clone() const
{
  CSpectraDirectoryItem *tmp = new CSpectraDirectoryItem(*this);
  // the copy contrstuctor has already recursively build the sub tree ...
  return tmp;
}

void CSpectraDirectoryItem::refreshBranch(void)
{
  discardBranch();
  loadBranch();
}

void CSpectraDirectoryItem::changeProperties(const QStringList &fileFilters, bool includeSubDirectories)
{
  // change the properties
  m_fileFilters = fileFilters;
  m_includeSubDirectories = includeSubDirectories;

  // reload ...
  discardBranch();
  loadBranch();
}

QString CSpectraDirectoryItem::directoryName(void) const
{
  return m_directory.absolutePath();
}

QString CSpectraDirectoryItem::fileFilters(void) const
{
  return m_fileFilters.join(";");
}

bool CSpectraDirectoryItem::isRecursive(void) const
{
  return m_includeSubDirectories;
}

int CSpectraDirectoryItem::loadBranch(void)
{
  int totalFileCount = 0;

  QFileInfoList entries;
  QFileInfoList::iterator it;

  // first consder sub directories ...
  if (m_includeSubDirectories) {
    entries = m_directory.entryInfoList(); // all entries ... but only take directories on this pass

    it = entries.begin();
    while (it != entries.end()) {
      if (it->isDir() && !it->fileName().startsWith('.')) {
        int tmpFileCount;

        CSpectraDirectoryItem *dItem = new CSpectraDirectoryItem(NULL, it->filePath(),
                                 m_fileFilters, true, &tmpFileCount);
        // were there any matching files in this branch?
        if (tmpFileCount) {
          addChild(dItem);
          totalFileCount += tmpFileCount;
        }
        else
          delete dItem; // discard
      }
      ++it;
    }
  }

  // now the files that match the filters
  if (m_fileFilters.isEmpty())
    entries = m_directory.entryInfoList();
  else
    entries = m_directory.entryInfoList(m_fileFilters);

  it = entries.begin();
  while (it != entries.end()) {
    if (it->isFile()) {
      new CSpectraFileItem(this, *it);
      ++totalFileCount;
    }
    ++it;
  }

  return totalFileCount;
}

void CSpectraDirectoryItem::discardBranch(void)
{
  // remove all child items (recursive destruction)

  QList<QTreeWidgetItem*> children = takeChildren();
  QList<QTreeWidgetItem*>::iterator it = children.begin();
  while (it != children.end()) {
    delete *it;
    ++it;
  }
}

//------------------------------------------------------------------------------

CSpectraFileItem::CSpectraFileItem(QTreeWidgetItem *parent, const QFileInfo &fileInfo) :
  CProjectTreeItem(parent, cSpectraFileItemType),
  m_fileInfo(fileInfo)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraFileItemType));
}

CSpectraFileItem::CSpectraFileItem(const CSpectraFileItem &other) :
  CProjectTreeItem(other),
  m_fileInfo(other.m_fileInfo)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraFileItemType));
}

QString CSpectraFileItem::fullFileName(void) const
{
  return m_fileInfo.absoluteFilePath();
}

QString CSpectraFileItem::dateLastModified(void) const
{
  return m_fileInfo.lastModified().toString();
}

QString CSpectraFileItem::fileSizeInBytes(void) const
{
  QString tmp;
  tmp.setNum(m_fileInfo.size());
  return tmp;
}

QVariant CSpectraFileItem::data(int column, int role) const
{
  if (role == Qt::DisplayRole) {

    if (column == 0)
      return QVariant(m_fileInfo.fileName());
    if (column == 1) {
      QString tmpStr;
      QTextStream tmpStream(&tmpStr);
      qint64 fs = m_fileInfo.size();
      if (fs >= 1024*100) {
        fs >>= 10; // now in KBytes
        if (fs >= 1024*100) {
          fs >>= 10; // now in MBytes
          if (fs >= 1024*100) {
            fs >>= 10; // now in GBytes
            tmpStream << fs << " GB";
          }
          else
            tmpStream << fs << " MB";
        }
        else
          tmpStream << fs << " KB";
      }
      else
        tmpStream << fs << "  B";

      return QVariant(tmpStr);
    }
    if (column == 2)
      return QVariant(m_fileInfo.lastModified().toString());
    //if (column == 3)
    //  return QVariant(m_fileInfo.absolutePath());

    return QVariant();

  }
  else if (role == Qt::TextAlignmentRole) {

    if (column == 1)
      return QVariant(Qt::AlignRight);
    if (column == 2)
      return QVariant(Qt::AlignCenter);
  }
  else if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

QTreeWidgetItem* CSpectraFileItem::clone() const
{
  CSpectraFileItem *tmp = new CSpectraFileItem(*this);

  return tmp;
}

//------------------------------------------------------------------------------

CAnalysisWindowItem::CAnalysisWindowItem(QTreeWidgetItem *parent, const QString &windowName) :
  CProjectTreeItem(parent, QStringList(windowName), cAnalysisWindowItemType)
{
  setIcon(0, CWProjectTree::getIcon(cAnalysisWindowItemType));
}

CAnalysisWindowItem::CAnalysisWindowItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceedingSibling, const QString &windowName) :
  CProjectTreeItem(parent, preceedingSibling, QStringList(windowName), cAnalysisWindowItemType)
{
  setIcon(0, CWProjectTree::getIcon(cAnalysisWindowItemType));
}

void CAnalysisWindowItem::destroyItem(QTreeWidgetItem *awItem)
{
  // this must be used to delete items WHILE THEY ARE STILL IN THE TREE.
  // This is essential, because the tree must be used to determine the
  // parent project name.

  QTreeWidgetItem *projItem = CWProjectTree::projectItem(awItem);

  assert(projItem && projItem != awItem);

  CWorkSpace::instance()->destroyAnalysisWindow(projItem->text(0).toStdString(), awItem->text(0).toStdString());
  // remove the item from the tree and delete it.
  QTreeWidgetItem *p = awItem->parent();
  delete p->takeChild(p->indexOfChild(awItem));
}

void CAnalysisWindowItem::setEnabled(bool enable)
{
  QTreeWidgetItem *projItem = CWProjectTree::projectItem(this);

  assert(projItem);

  // this is NOT state that is saved to the configuration file ...
  if (CWorkSpace::instance()->setAnalysisWindowEnabled(projItem->text(0).toStdString(), text(0).toStdString(), enable))
    m_enabled = enable;
}

QVariant CAnalysisWindowItem::data(int column, int role) const
{
  if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}


//------------------------------------------------------------------------------
// Static methods
//------------------------------------------------------------------------------

int CWProjectTree::itemDepth(QTreeWidgetItem *item)
{
  // item MUST be a valid item (ie. NOT null) - A toplevel item has depth = 0
  int n = 0;

  while ((item = item->parent()) != NULL) ++n;

  return n;
}

QTreeWidgetItem* CWProjectTree::ancestor(QTreeWidgetItem *item, int nth)
{
  // move back n generations. nth MUST not be larger the items depth
  while (nth--)
    item = item->parent();

  return item;
}

QTreeWidgetItem* CWProjectTree::projectItem(QTreeWidgetItem *item)
{
  // move back to the top level of the branch containing item.
  if (!item)
    return NULL;

  QTreeWidgetItem *p;

  while ((p = item->parent()) != NULL) {
    item = p;
  }

  return item;
}

QList<QTreeWidgetItem*> CWProjectTree::normalize(QList<QTreeWidgetItem*> items)
{
  QList<int> depthList;
  QList<QTreeWidgetItem*> normList;
  int iDepth, delta;

  while (true) {
    bool repeat = false;

    // for each item
    QList<QTreeWidgetItem*>::iterator iIt = items.begin();
    while (iIt != items.end()) {
      bool discardItem = false;
      iDepth = CWProjectTree::itemDepth(*iIt);

      QList<QTreeWidgetItem*>::iterator nIt = normList.begin();
      QList<int>::iterator dIt = depthList.begin();
      while (nIt != normList.end()) {

        delta = iDepth - *dIt;
        if (delta > 0) {
          // *iIt could be in the tree of *nIt
          if (*nIt == CWProjectTree::ancestor(*iIt, delta)) {
            // yes it is - can discard this item
            discardItem = true;
            break;
          }
        }
        else if (delta < 0) {
          // *nIt could be in the tree of *iIt
          if (*iIt == CWProjectTree::ancestor(*nIt, -delta)) {
            // yes it is - delete nIt
            // no other normList item is an ancestor of iIt, but it could be an
            // ancestor of items in normList => repeat
            normList.erase(nIt);
            depthList.erase(dIt);
            repeat = true;
            break;
          }
        }
        // else at the same level and connot be the same (no duplicates in items)
        ++nIt;
        ++dIt;
      }
      // checked against all items in normList
      if (!discardItem) {
        normList.push_front(*iIt);
        depthList.push_front(iDepth);
      }
      ++iIt;
    }

    // try again with a reduced set of items or return ...
    if (repeat) {
      items = normList;
      normList.clear();
      depthList.clear();
    }
    else {
      return normList;
    }
  }
}

QList<QTreeWidgetItem*> CWProjectTree::directoryItems(const QList<QTreeWidgetItem*> &items)
{
  QList<QTreeWidgetItem*> dirItems;

  QList<QTreeWidgetItem*>::const_iterator it = items.begin();
  while (it != items.end()) {
    if ((*it)->type() == cSpectraDirectoryItemType)
      dirItems.push_back(*it);
    ++it;
  }

  return dirItems;
}

QTreeWidgetItem* CWProjectTree::locateChildByName(QTreeWidgetItem *parent, const QString &childName)
{
  if (!parent)
    return NULL;

  QTreeWidgetItem *item = NULL;
  int i = 0;

  while (i < parent->childCount() && (item = parent->child(i))->text(0) != childName) ++i;
  if (i < parent->childCount())
    return item;

  return NULL;
}

void CWProjectTree::buildSession(CSession *session, CProjectTreeItem *item)
{
  // recursive construction of the session ...

  if (item->type() == cSpectraFileItemType) {
    // individual file
    QTreeWidgetItem *proj = CWProjectTree::projectItem(item);
    if (proj)
      session->addFile(static_cast<CSpectraFileItem*>(item)->file(), proj->text(0));
  }
  else if (item->type() == cSpectraFolderItemType ||
       item->type() == cSpectraDirectoryItemType ||
       item->type() == cSpectraBranchItemType ||
       item->type() == cProjectItemType) {
    // all enabled sub items ...
    CProjectTreeItem *child;
    int nChildren = item->childCount();
    int i = 0;
    while (i < nChildren) {
      child = static_cast<CProjectTreeItem*>(item->child(i));
      if (child->isEnabled())
    CWProjectTree::buildSession(session, child);
      ++i;
    }
  }
}

void CWProjectTree::collateErrorMessage(QString &errStr, const QString &msg)
{
  if (msg.isNull()) return;

  if (errStr.isEmpty()) {
    errStr = msg;
  }
  else {
    errStr += '\n';
    errStr += msg;
  }
}
