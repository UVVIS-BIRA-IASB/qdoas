/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QMenu>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QTextStream>
#include <QList>

#include "CWSiteTree.h"
#include "CWSiteEditor.h"
#include "CWActiveContext.h"

#include "CPreferences.h"

#include "debugutil.h"

const int cSiteTreeGeneralMode   = 37;

CWSiteTree::CWSiteTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  CSitesObserver(),
  m_activeContext(activeContext)
{
  QStringList labelList;
  labelList << "Observation Sites" << "Details";

  setHeaderLabels(labelList);

  QList<int> widthList;
  widthList.push_back(130);
  widthList.push_back(130);

  widthList = CPreferences::instance()->columnWidthList("SiteTree", widthList);

  for (int i=0; i<2; ++i) {
    setColumnWidth(i, widthList.at(i));
  }

  // populate from the workspace
  int nSites;
  mediate_site_t *siteList = CWorkSpace::instance()->siteList(nSites);
  if (siteList != NULL) {
    for (int i=0; i<nSites; ++i) {
      createSiteItem(&siteList[i]);
    }
    delete [] siteList;
  }
}

void CWSiteTree::savePreferences(void)
{
  QList<int> widthList;

  for (int i=0; i<2; ++i)
    widthList.push_back(columnWidth(i));

  CPreferences::instance()->setColumnWidthList("SiteTree", widthList);
}

void CWSiteTree::createSiteItem(const mediate_site_t *site)
{
  QString tmpStr;
  QStringList labelList;

  // make the tree items
  QTreeWidgetItem *siteItem = new  QTreeWidgetItem(QStringList(QString(site->name)));

  // add Children for the site details

  // Abbreviation
  labelList << "Abbreviaton" << site->abbreviation;
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  // Longitude
  labelList << "Longitude" << tmpStr.setNum(site->longitude, 'f', 3);
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  // Latitude
  labelList << "Latitude" << tmpStr.setNum(site->latitude, 'f', 3);
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  // Altitude
  labelList << "Altitude" << tmpStr.setNum(site->altitude, 'f', 3);
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  addTopLevelItem(siteItem);
}

void CWSiteTree::updateNewSite(const std::string &newSiteName)
{
  const mediate_site_t *site = CWorkSpace::instance()->findSite(newSiteName);

  if (site != NULL) {
    createSiteItem(site);
  }
}

void CWSiteTree::updateModifySite(const std::string &siteName)
{
  const mediate_site_t *site = CWorkSpace::instance()->findSite(siteName);

  if (site != NULL) {

    QTreeWidgetItem *siteItem;
    int i = 0;

    while ((siteItem = topLevelItem(i)) != NULL && siteItem->text(0).toStdString() != siteName) ++i;
    if (siteItem != NULL) {
      // located the existing item - update it's children

      assert(siteItem->childCount() == 4);

      QTreeWidgetItem *child;
      QString tmpStr;

      // abbrev.
      child = siteItem->child(0);
      child->setText(1, QString(site->abbreviation));
      // long.
      child = siteItem->child(1);
      child->setText(1, tmpStr.setNum(site->longitude, 'f', 3));
      // lat.
      child = siteItem->child(2);
      child->setText(1, tmpStr.setNum(site->latitude, 'f', 3));
      // alt.
      child = siteItem->child(3);
      child->setText(1, tmpStr.setNum(site->altitude, 'f', 3));
    }
  }
}

void CWSiteTree::updateDeleteSite(const std::string &siteName)
{
  const mediate_site_t *site = CWorkSpace::instance()->findSite(siteName);

  if (site != NULL) {

    QTreeWidgetItem *siteItem;
    int i = 0;

    while ((siteItem = topLevelItem(i)) != NULL && siteItem->text(0).toStdString() != siteName) ++i;
    if (siteItem != NULL) {
      delete takeTopLevelItem(i);
    }
  }
}


void CWSiteTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(cSiteTreeGeneralMode);
}

void CWSiteTree::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  menu.addAction("Insert...", this, SLOT(slotAddNewSite()));
  if (!selectedItems().isEmpty()) {
    menu.addAction("Edit...", this, SLOT(slotEditSite()));
    menu.addSeparator();
    menu.addAction("Delete", this, SLOT(slotDeleteSite()));
  }

  menu.exec(e->globalPos()); // a slot will do the rest if appropriate
}

void CWSiteTree::slotAddNewSite()
{
  CWSiteEditor *siteEdit = new CWSiteEditor;
  m_activeContext->addEditor(siteEdit);
}

void CWSiteTree::slotEditSite()
{
  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> selection = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = selection.begin();
  while (it != selection.end()) {
    CWSiteEditor *siteEdit = new  CWSiteEditor(*it);
    m_activeContext->addEditor(siteEdit);
    ++it;
  }
}

void CWSiteTree::slotDeleteSite()
{
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

    QString siteName = item->text(0);
    ++it;

    CWorkSpace::instance()->destroySite(siteName.toStdString());
  }
}

