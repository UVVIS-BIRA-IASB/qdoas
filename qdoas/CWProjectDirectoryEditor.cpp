/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectDirectoryEditor.h"
#include "CPreferences.h"

#include "CHelpSystem.h"

CWProjectDirectoryEditor::CWProjectDirectoryEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                                   QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  CSpectraDirectoryItem *dirItem = NULL;
  if (item->type() == cSpectraDirectoryItemType) {
    // editing an existing item ...
    dirItem = dynamic_cast<CSpectraDirectoryItem*>(item);
  }

  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0 - directory name
  mainLayout->addWidget(new QLabel("Directory", this), 0, 1);

  m_directoryName = new QLineEdit(this);
  mainLayout->addWidget(m_directoryName, 0, 2);

  QPushButton *browseButton = new QPushButton("Browse", this);
  mainLayout->addWidget(browseButton, 0, 3);

  // row 1 - file filters
  mainLayout->addWidget(new QLabel("File filters", this), 1, 1);

  m_fileFilters = new QLineEdit(this);
  mainLayout->addWidget(m_fileFilters, 1, 2);

  // row 3 - include sub-dirs
  m_recursiveCheckBox = new QCheckBox("Include Sub-Directories", this);
  mainLayout->addWidget(m_recursiveCheckBox, 2, 2);

  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(2, 1);
  mainLayout->setColumnStretch(4, 1);
  mainLayout->setRowStretch(3, 1);

  if (dirItem != NULL) {
    // editing an existing item ...
    m_directoryName->setText(dirItem->directoryName());
    m_directoryName->setEnabled(false); // enabled state is the indactor for edit (as opposed to add) mode.
    browseButton->setEnabled(false);
    m_fileFilters->setText(dirItem->fileFilters());
    m_recursiveCheckBox->setCheckState(dirItem->isRecursive() ? Qt::Checked : Qt::Unchecked);

    m_captionStr = "Edit directory ";
  }
  else
    m_captionStr = "Insert new directory in ";

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
  m_contextTag += " Dir";

  connect(browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseButtonClicked()));

  connect(m_directoryName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotDirectoryChanged(const QString &)));


  notifyAcceptActionOk(dirItem != NULL);
}

bool CWProjectDirectoryEditor::actionOk(void)
{
  // use the path to locate the correct place in the tree

  if (m_directoryName->text().isEmpty())
    return false;

  QString msg;

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree
    if (m_directoryName->isEnabled()) {
      msg = m_projectTree->editInsertDirectory(item, m_directoryName->text(),
                           m_fileFilters->text(),
                           (m_recursiveCheckBox->checkState() == Qt::Checked));
    }
    else {
      msg = m_projectTree->editChangeDirectoryProperties(item, m_fileFilters->text(),
                             (m_recursiveCheckBox->checkState() == Qt::Checked));
    }

    if (msg.isNull())
      return true;

  }
  else {
    // no longer exists ...
    msg = "Parent folder no longer exists.";
  }

  // all errors fall through to here
  QMessageBox::information(this, "Insert Directory Failed", msg);
  return false;
}

void CWProjectDirectoryEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("project", "DirName");
}

void CWProjectDirectoryEditor::takeFocus(void)
{
  m_directoryName->setFocus(Qt::OtherFocusReason);
}

void CWProjectDirectoryEditor::slotDirectoryChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectDirectoryEditor::slotBrowseButtonClicked()
{
  QString dir = CPreferences::instance()->directoryName("RawSpecDir", ".");

  // modal dialog
  dir = QFileDialog::getExistingDirectory(0, "Select a directory containing spectra files", dir);

  if (!dir.isEmpty()) {
    CPreferences::instance()->setDirectoryName("RawSpecDir", dir);
    m_directoryName->setText(dir);
  }
}

