/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWSYMBOLEDITOR_H_GUARD
#define _CWSYMBOLEDITOR_H_GUARD

#include <QLineEdit>

#include "CWEditor.h"

class CWSymbolEditor : public CWEditor
{
Q_OBJECT
 public:
  CWSymbolEditor(const QString &symbolName = QString(), const QString &description = QString(), QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 public slots:
  void slotNameChanged(const QString &text);
  void slotReturnPressed();

 private:
  QLineEdit *m_symbolName, *m_description;
};

#endif
