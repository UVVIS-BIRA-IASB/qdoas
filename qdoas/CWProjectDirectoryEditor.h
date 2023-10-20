/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWPROJECTDIRECTORYEDITOR_H_GUARD
#define _CWPROJECTDIRECTORYEDITOR_H_GUARD

#include <QLineEdit>
#include <QCheckBox>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWProjectTree;

class CWProjectDirectoryEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectDirectoryEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                           QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 public slots:
  void slotDirectoryChanged(const QString &text);
  void slotBrowseButtonClicked();

 private:
  QLineEdit *m_directoryName;
  QLineEdit *m_fileFilters;
  QCheckBox *m_recursiveCheckBox;
  CWProjectTree *m_projectTree;
  QStringList m_path;
};

#endif
