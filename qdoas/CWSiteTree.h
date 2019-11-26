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

  virtual void updateNewSite(const QString &newSiteName);
  virtual void updateModifySite(const QString &siteName);
  virtual void updateDeleteSite(const QString &siteName);
  
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
