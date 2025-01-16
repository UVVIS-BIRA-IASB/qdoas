/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QMenu>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QList>
#include <QStringList>
#include <QMessageBox>

#include "CWUserSymbolTree.h"
#include "CPreferences.h"
#include "CWActiveContext.h"
#include "CWSymbolEditor.h"

#include "debugutil.h"

// NOTE: Symbols in the list MUST be unique, as defined by case-insensitive
// string comparison. Also, they can only include the AlphaNumeric characters
// dash and underscore.

const int cUserSymbolTreeGeneralMode = 47;

CWUserSymbolTree::CWUserSymbolTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  CSymbolObserver(),
  m_activeContext(activeContext)
{
  QStringList labelList;
  labelList << "Name" << "Description";
  setHeaderLabels(labelList);

  QList<int> widthList;
  widthList.push_back(130);
  widthList.push_back(130);

  widthList = CPreferences::instance()->columnWidthList("UserSymbolTree", widthList);

  for (int i=0; i<2; ++i) {
    setColumnWidth(i, widthList.at(i));
  }

  // multi-selection ....
  setSelectionMode(QAbstractItemView::ExtendedSelection);
}

CWUserSymbolTree::~CWUserSymbolTree()
{
}

void CWUserSymbolTree::savePreferences(void)
{
  QList<int> widthList;

  for (int i=0; i<2; ++i)
    widthList.push_back(columnWidth(i));

  CPreferences::instance()->setColumnWidthList("UserSymbolTree", widthList);
}

void CWUserSymbolTree::updateNewSymbol(const std::string &newSymbolName)
{
  QString description(QString::fromStdString(CWorkSpace::instance()->findSymbol(newSymbolName)));

  if (!description.isNull()) {

    QStringList labelList;
    labelList << QString::fromStdString(newSymbolName) << description;

    QTreeWidgetItem *userSymbolItem = new  QTreeWidgetItem(labelList);

    addTopLevelItem(userSymbolItem);
  }
}

void CWUserSymbolTree::updateModifySymbol(const std::string &symbolName)
{
  QString description(QString::fromStdString(CWorkSpace::instance()->findSymbol(symbolName)));

  if (!description.isNull()) {

    QTreeWidgetItem *symbolItem;
    int i = 0;

    while ((symbolItem = topLevelItem(i)) != NULL && symbolItem->text(0).toStdString() != symbolName) ++i;
    if (symbolItem != NULL)
      symbolItem->setText(1, description);
  }
}

void CWUserSymbolTree::updateDeleteSymbol(const std::string &symbolName)
{
  QString description(QString::fromStdString(CWorkSpace::instance()->findSymbol(symbolName)));

  if (!description.isNull()) {

    QTreeWidgetItem *symbolItem;
    int i = 0;

    while ((symbolItem = topLevelItem(i)) != NULL && symbolItem->text(0).toStdString() != symbolName) ++i;
    if (symbolItem != NULL) {
      delete takeTopLevelItem(i);
    }
  }
}

void CWUserSymbolTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(cUserSymbolTreeGeneralMode);
}

void CWUserSymbolTree::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;
  QAction *action;

  menu.addAction("Insert...", this, SLOT(slotAddNewSymbol()));
  action = menu.addAction("Edit...", this, SLOT(slotEditSymbol()));
  action->setEnabled(selectedItems().count() == 1);
  menu.addSeparator();
  action = menu.addAction("Delete", this, SLOT(slotDeleteSymbol()));
  action->setEnabled(!selectedItems().isEmpty());

  menu.exec(e->globalPos()); // a slot will do the rest if appropriate
}

void CWUserSymbolTree::slotAddNewSymbol()
{
  CWSymbolEditor *symbolEdit = new CWSymbolEditor;
  m_activeContext->addEditor(symbolEdit);
}

void CWUserSymbolTree::slotEditSymbol()
{
  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> selection = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = selection.begin();
  while (it != selection.end()) {
    CWSymbolEditor *symbolEdit = new CWSymbolEditor((*it)->text(0), (*it)->text(1));
    m_activeContext->addEditor(symbolEdit);
    ++it;
  }
}

void CWUserSymbolTree::slotDeleteSymbol()
{
  QStringList lockedSymbols;

  QTreeWidgetItem *item;

  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> selection = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = selection.begin();
  while (it != selection.end()) {
    item = *it;
    // get the top-level item
    while (item->parent() != NULL) {
      item = item->parent();
    }

    QString symbolName = item->text(0);
    ++it;

    if (!CWorkSpace::instance()->destroySymbol(symbolName.toStdString()))
      lockedSymbols << symbolName;
  }

  // report and failures
  if (!lockedSymbols.isEmpty()) {
    QString msg = "Cannot remove the following symbol(s) because they\nare referenced by a project or analysis window.\n";
    QStringList::const_iterator it = lockedSymbols.begin();
    msg.append(*it);
    ++it;
    while (it != lockedSymbols.end()) {
      msg.append(", ");
      msg.append(*it);
      ++it;
    }

    QMessageBox::information(this, "Delete Symbol", msg);
  }
}

