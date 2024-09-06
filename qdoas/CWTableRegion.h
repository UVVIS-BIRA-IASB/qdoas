/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWTABLEREGION_H_GUARD
#define _CWTABLEREGION_H_GUARD

#include <QTableView>
#include <QList>

#include "CMultiPageTableModel.h"

class CWTableRegion : public QTableView
{
Q_OBJECT
 public:
  CWTableRegion(QWidget *parent = 0);

  void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotTablePages(const QList<std::shared_ptr<const CTablePageData> > &pageList);
  void slotDisplayPage(int pageNumber);
  void slotSaveAs();

 private:
  CMultiPageTableModel *m_model;

};

#endif
