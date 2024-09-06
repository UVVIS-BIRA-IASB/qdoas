/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CMULTIPAGETABLEMODEL_H_GUARD
#define _CMULTIPAGETABLEMODEL_H_GUARD

#include <map>
#include <memory>

#include <QAbstractTableModel>
#include <QList>

#include "CTablePageData.h"


class CMultiPageTableModel : public QAbstractTableModel
{
Q_OBJECT
 public:
  CMultiPageTableModel(QObject *parent = 0) : QAbstractTableModel(parent) {};

  void addPage(std::shared_ptr<const CTablePageData> page);
  void removeAllPages(void);
  void removePagesExcept(const QList<int> pageNumberList);

  void setActivePage(int pageNumber);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

 public slots:
  void slotTablePages(const QList<std::shared_ptr<const CTablePageData> > &pageList);

 private:
  std::map< int,std::shared_ptr<const CTablePageData> > m_pageMap;
  std::shared_ptr<const CTablePageData> m_currentPage;
};

#endif
