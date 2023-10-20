/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWSITEEDITOR_H_GUARD
#define _CWSITEEDITOR_H_GUARD

#include <QLineEdit>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWSiteEditor : public CWEditor
{
Q_OBJECT
 public:
  CWSiteEditor(QTreeWidgetItem *editItem = 0, QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 private:
  double m_long, m_lat, m_alt;
  unsigned int m_validBits;
  QLineEdit *m_siteName, *m_abbreviation;

 public slots:
   void slotLongitudeChanged(const QString &text);
   void slotLatitudeChanged(const QString &text);
   void slotAltitudeChanged(const QString &text);

};

#endif
