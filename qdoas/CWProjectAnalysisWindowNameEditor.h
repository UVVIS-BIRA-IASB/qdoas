/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWPROJECTANALYSISWINDOWNAMEEDITOR_H_GUARD
#define _CWPROJECTANALYSISWINDOWNAMEEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWProjectTree;

class CWProjectAnalysisWindowNameEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectAnalysisWindowNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                    const QString &preceedingWindowName, bool newAnalysisWindow,
                                    QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 public slots:
  void slotNameChanged(const QString &text);
  void slotReturnPressed();

 private:
  QLineEdit *m_analysisWindowName;
  CWProjectTree *m_projectTree;
  QString m_preceedingWindowName;
  QStringList m_path;
  bool m_newAnalysisWindow;
};

#endif
