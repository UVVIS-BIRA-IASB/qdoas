/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <cstring>
#include <iostream>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "CWorkSpace.h"
#include "CWProjectTree.h"
#include "CWProjectExportEditor.h"
#include "CPreferences.h"
#include "CWOutputSelector.h"

#include "CHelpSystem.h"



CWProjectExportEditor::CWProjectExportEditor(CWProjectTree *projectTree, QTreeWidgetItem *items, QString projectName,mediate_project_export_t *properties, int format,
                                                   QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree),
  m_items(items),
  m_projectName(projectName),
  m_properties(properties),
  m_format(format)
{
     m_captionStr = "Export Data/Spectra";
     m_contextTag = "Export Data/Spectra";

     // main layout: VBox
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QFrame *pathFrame;

  mainLayout->addSpacing(25);

  // Output file/format widgets:
  pathFrame = new QFrame(this);
  pathFrame->setFrameStyle(QFrame::NoFrame);

  m_pathEdit = new QLineEdit(pathFrame);
  m_pathEdit->setMaxLength(1024); // sizeof(properties->path)
  QPushButton *browseBtn = new QPushButton("Browse", pathFrame);

  // Layout for the output file

  QHBoxLayout *pathLayout = new QHBoxLayout();
  pathLayout->addWidget(new QLabel("Output Path/file name", pathFrame),0);
  pathLayout->addWidget(m_pathEdit, 1);
  pathLayout->addWidget(browseBtn);

  mainLayout->addLayout(pathLayout);

  mainLayout->addSpacing(5);

  m_selector = new CWOutputSelector(&(m_properties->selection), this);
  mainLayout->addWidget(m_selector);
  mainLayout->addStretch(1);

  mainLayout->addSpacing(5);

  QHBoxLayout *checkLayout= new QHBoxLayout();;
  m_directoryCheck = new QCheckBox("Directories");
  checkLayout->addWidget(m_directoryCheck);
  mainLayout->addLayout(checkLayout);

  // initialize ...

  m_pathEdit->setText(QString(properties->path));

  m_directoryCheck->setCheckState(m_properties->directoryFlag ? Qt::Checked : Qt::Unchecked);

  m_selector->apply(&(m_properties->selection));

  slotInstrumentChanged(m_format);

  // connections

  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowsePath()));
  connect(m_pathEdit, SIGNAL(textChanged(const QString &)),this, SLOT(slotDirectoryChanged(const QString &)));
  connect(m_pathEdit, SIGNAL(returnPressed()),this, SLOT(slotReturnPressed()));

  notifyAcceptActionOk(!m_pathEdit->text().isEmpty());
 }

bool CWProjectExportEditor::actionOk(void)
 {
   strcpy(m_properties->path, m_pathEdit->text().toLocal8Bit().data());

  m_properties->directoryFlag = (m_directoryCheck->checkState() == Qt::Checked) ? 1 : 0;

  m_selector->apply(&(m_properties->selection));

  CWorkSpace::instance()->modifiedProjectProperties(m_projectName.toStdString());
  m_projectTree->ExportSpectra();

  return true;
 }

void CWProjectExportEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("project", "Export");
}

void CWProjectExportEditor::slotBrowsePath()
{
  CPreferences *pref = CPreferences::instance();

  QString fileName = QFileDialog::getSaveFileName(this, "Select export path or file name", pref->directoryName("Export"),
                          "All Files (*)");

  if (!fileName.isEmpty()) {
    // save it again
    pref->setDirectoryNameGivenFile("Export", fileName);

    m_pathEdit->setText(fileName);
  }
}

void CWProjectExportEditor::slotDirectoryChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectExportEditor::slotInstrumentChanged(int instrument)
{
  m_selector->setInstrument(instrument,TAB_SELECTOR_EXPORT);

  if ((instrument!=PRJCT_INSTR_FORMAT_OMI) &&
     (instrument!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
     (instrument!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
     (instrument!=PRJCT_INSTR_FORMAT_GOME2))

   m_directoryCheck->hide();

}

void CWProjectExportEditor::slotReturnPressed()
{
  shortcutActionOk();
}

