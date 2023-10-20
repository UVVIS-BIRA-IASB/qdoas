/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWPROJECTFOLDERNAMEEDITOR_H_GUARD
#define _CWPROJECTFOLDERNAMEEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWProjectTree;

class CWProjectFolderNameEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectFolderNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item, bool newFolder,
                            QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 public slots:
  void slotNameChanged(const QString &text);
  void slotReturnPressed();

 private:
  QLineEdit *m_folderName;
  CWProjectTree *m_projectTree;
  QStringList m_path;
  bool m_newFolder;
};

#endif
