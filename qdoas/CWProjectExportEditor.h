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
