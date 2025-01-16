/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWSITETREE_H_GUARD
#define _CWSITETREE_H_GUARD

#include <QTreeWidget>

#include "CProjectConfigItem.h"
#include "CWorkSpace.h"

class CWActiveContext;

class CWSiteTree : public QTreeWidget, public CSitesObserver
{
Q_OBJECT
 public:
  CWSiteTree(CWActiveContext *activeContext, QWidget *parent = 0);

  void savePreferences(void);

  virtual void updateNewSite(const std::string &newSiteName) override;
  virtual void updateModifySite(const std::string &siteName) override;
  virtual void updateDeleteSite(const std::string &siteName) override;

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);
  virtual void showEvent(QShowEvent *e);

 private:
  void createSiteItem(const mediate_site_t *site);

 public slots:
   void slotAddNewSite();
   void slotEditSite();
   void slotDeleteSite();

 signals:
  void signalWidthModeChanged(int newMode);

 private:
  CWActiveContext *m_activeContext;
};

#endif
