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

#ifndef _CWUSERSYMBOLTREE_H_GUARD
#define _CWUSERSYMBOLTREE_H_GUARD

#include <QTreeWidget>

#include "CWorkSpace.h"

class CWActiveContext;

class CWUserSymbolTree : public QTreeWidget, public CSymbolObserver
{
Q_OBJECT
 public:
 CWUserSymbolTree(CWActiveContext *activeContext, QWidget *parent = 0);
  virtual ~CWUserSymbolTree();

  void savePreferences(void);

  virtual void updateNewSymbol(const QString &newSymbolName);
  virtual void updateModifySymbol(const QString &symbolName);
  virtual void updateDeleteSymbol(const QString &symbolName);
  
 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);
  virtual void showEvent(QShowEvent *e);

 private:
  void addNewUserSymbol(const QString &userSymbolName, const QString &description); 

 public slots:
   void slotAddNewSymbol();
   void slotEditSymbol();
   void slotDeleteSymbol();

 signals:
  void signalWidthModeChanged(int newMode);

 private:
  CWActiveContext *m_activeContext;
};

#endif
