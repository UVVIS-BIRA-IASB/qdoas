/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>

#ifndef _CWPROJECTEXPORTEDITOR_H_GUARD
#define _CWPROJECTEXPORTEDITOR_H_GUARD

#include <QLineEdit>
#include <QCheckBox>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"
#include "mediate_project.h"

class CWProjectTree;
class CWOutputSelector;

class CWProjectExportEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectExportEditor(CWProjectTree *projectTree, QTreeWidgetItem *items, QString projectName,mediate_project_export_t *properties, int format,
                        QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

 public slots:
  void slotBrowsePath();
  void slotDirectoryChanged(const QString &text);
  void slotInstrumentChanged(int instrument);
  void slotReturnPressed();

 private:
  CWProjectTree *m_projectTree;
  QTreeWidgetItem *m_items;
  QLineEdit *m_pathEdit;
  QCheckBox *m_directoryCheck;
  CWOutputSelector *m_selector;
  mediate_project_export_t *m_properties;
  int m_format;
  QString m_projectName;
};

#endif
