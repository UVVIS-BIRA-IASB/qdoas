/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectAnalysisWindowNameEditor.h"

#include "CHelpSystem.h"

CWProjectAnalysisWindowNameEditor::CWProjectAnalysisWindowNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                     const QString &preceedingWindowName,
                                                                     bool newAnalysisWindow, QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree),
  m_preceedingWindowName(preceedingWindowName),
  m_newAnalysisWindow(newAnalysisWindow)
{
  // item will be either an analysis window item, or a branch.
  // preceeding item is null if the item is a branch or if
  // newAnalysisWindow is false (rename).

  QGridLayout *mainLayout = new QGridLayout(this);

  if (newAnalysisWindow)
    mainLayout->addWidget(new QLabel("Enter a name for the new Analysis Window", this), 0, 1);
  else
    mainLayout->addWidget(new QLabel("Modify the Analysis Window name", this), 0, 1);

  m_analysisWindowName = new QLineEdit(this);
  mainLayout->addWidget(m_analysisWindowName, 0, 2);

  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(3, 1);
  mainLayout->setRowStretch(1,1);

  if (m_newAnalysisWindow)
    m_captionStr = "Create new Analysis Window in ";
  else
    m_captionStr = "Rename Analysis Window ";

  m_contextTag.clear();

  // build a path from item - will use this to locate the target point in the tree on 'Ok'
  while (item != NULL) {
    QString tmpStr(item->text(0));

    m_path.push_front(tmpStr);
    m_contextTag.prepend(tmpStr);
    m_contextTag.prepend(':');

    item = item->parent();
  }
  m_captionStr += m_contextTag;
  m_contextTag += " Anlys Win";

  // if renaming
  if (!m_newAnalysisWindow && !m_path.isEmpty())
    m_analysisWindowName->setText(m_path.back());

  connect(m_analysisWindowName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotNameChanged(const QString &)));
  connect(m_analysisWindowName, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));

}

bool CWProjectAnalysisWindowNameEditor::actionOk()
{
  // use the path to locate the correct place in the tree

  if (m_analysisWindowName->text().isEmpty())
    return false;

  QString msg;

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree
    if (m_newAnalysisWindow)
      msg = m_projectTree->editInsertNewAnalysisWindow(item, m_analysisWindowName->text(), m_preceedingWindowName);
    else
      msg = m_projectTree->editRenameAnalysisWindow(item, m_analysisWindowName->text());

    if (msg.isNull())
      return true;

  }
  else {
    // no longer exists ...
    msg = QString("Could not locate the parent in the project tree.");
  }

  // fall through failure ...
  QMessageBox::information(this,
                           m_newAnalysisWindow ? "Insert Analysis Window" : "Rename Analysis Window",
                           msg);
  return false;
}

void CWProjectAnalysisWindowNameEditor::actionHelp(void)
{
}

void CWProjectAnalysisWindowNameEditor::takeFocus(void)
{
  m_analysisWindowName->setFocus(Qt::OtherFocusReason);
}

void CWProjectAnalysisWindowNameEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectAnalysisWindowNameEditor::slotReturnPressed()
{
  shortcutActionOk();
}

