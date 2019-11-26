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


#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectNameEditor.h"

#include "CHelpSystem.h"


CWProjectNameEditor::CWProjectNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                         QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setMargin(50);

  if (item && item->parent() == NULL) {
    // renaming a project
    m_oldProjectName = item->text(0);  // also acts as the flag for new/rename 
    mainLayout->addWidget(new QLabel("Modify the Project name", this), 0, 1);
  }
  else
    mainLayout->addWidget(new QLabel("Enter a name for the new Project", this), 0, 1);

  m_projectName = new QLineEdit(this);
  mainLayout->addWidget(m_projectName, 0, 2);

  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(3, 1);
  mainLayout->setRowStretch(1,1);

  if (m_oldProjectName.isNull()) {
    m_captionStr = "Create new Project";
    m_contextTag = "New Project";
  }
  else {
    m_captionStr = "Rename Project ";
    m_captionStr += m_oldProjectName;

    m_contextTag = m_oldProjectName;
    m_contextTag += " Rename";

    m_projectName->setText(m_oldProjectName);
  }

  connect(m_projectName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotNameChanged(const QString &)));
  connect(m_projectName, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));

}

bool CWProjectNameEditor::actionOk(void)
{
  if (m_projectName->text().isEmpty())
    return false;

  QString msg;

  if (m_oldProjectName.isNull()) {
    // new project
    msg = m_projectTree->editInsertNewProject(m_projectName->text());
  }
  else {
    // renaming
    QTreeWidgetItem *item = m_projectTree->locateProjectByName(m_oldProjectName);
    if (item)
      msg = m_projectTree->editRenameProject(item, m_projectName->text());
    else
      msg = QString("The project no longer exists.");
  }

  if (msg.isNull())
    return true;

  // fall through failure ...
  QMessageBox::information(this, m_oldProjectName.isNull() ? "Insert Project" : "Rename Project", msg);
  return false;
}

void CWProjectNameEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("project", "ProjName");
}

void CWProjectNameEditor::takeFocus(void)
{
  // give focus to the line edit
  m_projectName->setFocus(Qt::OtherFocusReason);
}

void CWProjectNameEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectNameEditor::slotReturnPressed()
{
  shortcutActionOk();
}

