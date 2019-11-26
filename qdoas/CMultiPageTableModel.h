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

#ifndef _CMULTIPAGETABLEMODEL_H_GUARD
#define _CMULTIPAGETABLEMODEL_H_GUARD

#include <map>

#include <QAbstractTableModel>
#include <QList>

#include "CTablePageData.h"
#include "RefCountPtr.h"

class CMultiPageTableModel : public QAbstractTableModel
{
Q_OBJECT
 public:
  CMultiPageTableModel(QObject *parent = 0) : QAbstractTableModel(parent) {};

  void addPage(const RefCountConstPtr<CTablePageData> &page);
  void removeAllPages(void);
  void removePagesExcept(const QList<int> pageNumberList);

  void setActivePage(int pageNumber);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

 public slots:
  void slotTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList);

 private:
  std::map< int,RefCountConstPtr<CTablePageData> > m_pageMap;
  RefCountConstPtr<CTablePageData> m_currentPage;
};

#endif
