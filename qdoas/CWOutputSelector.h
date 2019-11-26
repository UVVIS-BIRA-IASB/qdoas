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


#ifndef _CWOUTPUTSELECTOR_H_GUARD
#define _CWOUTPUTSELECTOR_H_GUARD

#include <QFrame>
#include <QListWidget>

#include "mediate_general.h"

class CWOutputSelector : public QFrame
{
Q_OBJECT
 public:
  CWOutputSelector(const data_select_list_t *d, QWidget *parent = 0);

  void apply(data_select_list_t *d);
  void setInstrument(int instrument,int selectorOrigin);

  static QListWidgetItem* locateItemByKey(QListWidget *listWidget, int key);

 public slots:
  void slotToChosenList();
  void slotToAvailableList();

 private:
  QListWidget *m_availableList, *m_chosenList;
};

class CWOutputFieldItem : public QListWidgetItem
{
 public:
  CWOutputFieldItem(int key, const QString &text, QListWidget *parent = 0);

  virtual QVariant data(int role) const; // implement Qt::UserRole to get key

 private:
  int m_key;
};

#endif
