/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
