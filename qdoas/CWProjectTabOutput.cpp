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

#include "CWProjectTabOutput.h"
#include "CWOutputSelector.h"
#include "CPreferences.h"
#include "CValidator.h"

#include "constants.h"
#include "output_formats.h"

#include "debugutil.h"


CWProjectTabOutput::CWProjectTabOutput(const mediate_project_output_t *properties, const QString& project_name, QWidget *parent) :
  QFrame(parent)
{
  // main layout: VBox
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addSpacing(25);

  // Output file/format widgets:
  m_pathFrame = new QFrame(this);
  m_pathFrame->setFrameStyle(QFrame::NoFrame);

  m_pathEdit = new QLineEdit(m_pathFrame);
  m_pathEdit->setMaxLength(sizeof(properties->path)-1);

  QPushButton *browseBtn = new QPushButton("Browse", m_pathFrame);

  m_selectFileFormat = new QComboBox(m_pathFrame);
  // fill combobox with available output formats.  The index of each
  // item will correspond to the enum value of the output format.
  for(int i=0; i<=LAST_OUTPUT_FORMAT; i++) {
    m_selectFileFormat->addItem(output_file_extensions[i]);
  }

  QFrame *groupFrame = new QFrame(m_pathFrame);
  groupFrame->setFrameStyle(QFrame::NoFrame);
  m_groupNameEdit = new QLineEdit(groupFrame);
  QRegularExpression validGroupName("[^/]{1,255}"); // Swath name may not contain "/" and can be 1 to 255 characters long.
  m_groupNameEdit->setValidator(new QRegularExpressionValidator(validGroupName, m_groupNameEdit));

  // Layout for output file/format widgets:
  QHBoxLayout *pathLayout = new QHBoxLayout();
  pathLayout->addWidget(new QLabel("Output Path", m_pathFrame),0);
  pathLayout->addWidget(m_pathEdit, 1);
  pathLayout->addWidget(browseBtn);
  pathLayout->addWidget(new QLabel("File Format", m_pathFrame),0);
  pathLayout->addWidget(m_selectFileFormat, 0);

  QVBoxLayout *outputFileLayout = new QVBoxLayout(m_pathFrame);
  outputFileLayout->addLayout(pathLayout, 0);
  outputFileLayout->addWidget(groupFrame);

  QHBoxLayout *swathLayout = new QHBoxLayout(groupFrame);
  swathLayout->setContentsMargins(0, 0, 0, 0);
  swathLayout->addWidget(new QLabel("netCDF group name", groupFrame),0);
  swathLayout->addWidget(m_groupNameEdit, 1);

  mainLayout->addWidget(m_pathFrame);
  mainLayout->addSpacing(5);

  // checkboxes and edits
  QHBoxLayout *middleLayout = new QHBoxLayout;

  // check boxes
  QGroupBox *checkGroup = new QGroupBox("Options", this);
  QGridLayout *checkLayout = new QGridLayout(checkGroup);

  m_analysisCheck = new QCheckBox("Analysis");
  checkLayout->addWidget(m_analysisCheck, 0, 0);

  m_calibrationCheck = new QCheckBox("Calibration");
  checkLayout->addWidget(m_calibrationCheck, 1, 0);

  m_referenceCheck = new QCheckBox("Reference info");
  checkLayout->addWidget(m_referenceCheck, 2, 0); // !!! 3, 0 if use newcalib

  m_directoryCheck = new QCheckBox("Directories");
  checkLayout->addWidget(m_directoryCheck, 0, 1);

  m_useFileName = new QCheckBox("Use file name");
  checkLayout->addWidget(m_useFileName, 1, 1);

  m_successCheck = new QCheckBox("Successful records only");
  checkLayout->addWidget(m_successCheck, 2, 1);

  middleLayout->addWidget(checkGroup);

  // edits
  m_editGroup = new QGroupBox("Fluxes (nm)", this);
  QGridLayout *editLayout = new QGridLayout(m_editGroup);

  editLayout->addWidget(new QLabel("Central wavelengths"), 0, 0);
  m_fluxEdit = new QLineEdit(this);
  m_fluxEdit->setMaxLength(sizeof(properties->flux)-1);
  editLayout->addWidget(m_fluxEdit, 0, 1);

  editLayout->addWidget(new QLabel("Averaging bandwidth"), 1, 0);
  m_bandWidthEdit = new QLineEdit(this);
  m_bandWidthEdit->setFixedWidth(50);
  m_bandWidthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 10.0, 2, m_bandWidthEdit));

  editLayout->addWidget(m_bandWidthEdit, 1, 1);

  middleLayout->addWidget(m_editGroup);

  mainLayout->addLayout(middleLayout);

  mainLayout->addSpacing(5);

  m_selector = new CWOutputSelector(&(properties->selection), this);
  mainLayout->addWidget(m_selector);

  // initialize ...
  m_pathEdit->setText(QString(properties->path));

  m_groupNameEdit->setPlaceholderText(project_name);
  m_groupNameEdit->setText(properties->swath_name);

  m_selectFileFormat->setCurrentIndex(properties->file_format);
 
  groupFrame->setVisible( properties->file_format == NETCDF);

  m_analysisCheck->setCheckState(properties->analysisFlag ? Qt::Checked : Qt::Unchecked);
  m_calibrationCheck->setCheckState(properties->calibrationFlag ? Qt::Checked : Qt::Unchecked);
  // m_newcalibCheck->setCheckState(properties->newcalibFlag ? Qt::Checked : Qt::Unchecked);
  m_referenceCheck->setCheckState(properties->referenceFlag ? Qt::Checked : Qt::Unchecked);
  m_directoryCheck->setCheckState(properties->directoryFlag ? Qt::Checked : Qt::Unchecked);
  m_useFileName->setCheckState(properties->filenameFlag ? Qt::Checked : Qt::Unchecked);
  m_successCheck->setCheckState(properties->successFlag ? Qt::Checked : Qt::Unchecked);

  m_fluxEdit->setText(QString(properties->flux));

  // Averaging band width for calculating fluxes

  QString tmpStr;

  tmpStr.setNum(properties->bandWidth);
  m_bandWidthEdit->validator()->fixup(tmpStr);
  m_bandWidthEdit->setText(tmpStr);

  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked),
                       (m_calibrationCheck->checkState() == Qt::Checked) || (m_referenceCheck->checkState() == Qt::Checked) );

  // connections
  connect(m_analysisCheck, SIGNAL(stateChanged(int)), this, SLOT(slotAnalysisCheckChanged(int)));
  connect(m_calibrationCheck, SIGNAL(stateChanged(int)), this, SLOT(slotCalibrationCheckChanged(int)));
  connect(m_referenceCheck, SIGNAL(stateChanged(int)), this, SLOT(slotReferenceCheckChanged(int)));
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowsePath()));
  connect(m_selectFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelectFileFormatChanged(int)));
}

void CWProjectTabOutput::apply(mediate_project_output_t *properties) const
{
  properties->analysisFlag = (m_analysisCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->calibrationFlag = (m_calibrationCheck->checkState() == Qt::Checked) ? 1 : 0;
  // properties->newcalibFlag = (m_newcalibCheck->checkState() == Qt::Checked);
  properties->referenceFlag = (m_referenceCheck->checkState() == Qt::Checked);
  properties->directoryFlag = (m_directoryCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->filenameFlag = (m_useFileName->checkState() == Qt::Checked) ? 1 : 0;
  properties->successFlag = (m_successCheck->checkState() == Qt::Checked) ? 1 : 0;

  properties->file_format = static_cast<enum output_format>(m_selectFileFormat->currentIndex());

  strcpy(properties->flux, m_fluxEdit->text().toLocal8Bit().data());
  properties->bandWidth = m_bandWidthEdit->text().toDouble();
  strcpy(properties->path, m_pathEdit->text().toLocal8Bit().data());

  strcpy(properties->swath_name, m_groupNameEdit->hasAcceptableInput()
         ? m_groupNameEdit->text().toLocal8Bit().data()
         : "");

  m_selector->apply(&(properties->selection));
}

void CWProjectTabOutput::slotSelectFileFormatChanged(int index)
{
  // parent widget of m_groupNameEdit contains the label and the text
  // field 
  m_groupNameEdit->parentWidget()->setVisible( index == NETCDF);
}

void CWProjectTabOutput::slotBrowsePath()
{
  CPreferences *pref = CPreferences::instance();

  QString fileName = QFileDialog::getSaveFileName(this, "Select Output Path", pref->directoryName("Output"),
                          "All Files (*)");

  if (!fileName.isEmpty()) {
    // save it again
    pref->setDirectoryNameGivenFile("Output", fileName);

    m_pathEdit->setText(fileName);
  }
}

void CWProjectTabOutput::slotInstrumentChanged(int instrument)
{
  bool analysisEnabled=(m_analysisCheck->checkState() == Qt::Checked);
  bool calibrationEnabled=(m_calibrationCheck->checkState() == Qt::Checked);

     m_instrument=instrument;

     // Disable "successful record only" for 2D formats

     m_successCheckEnable=((instrument==PRJCT_INSTR_FORMAT_OMI) ||
                           (instrument==PRJCT_INSTR_FORMAT_TROPOMI) ||
                        (instrument==PRJCT_INSTR_FORMAT_APEX)  ||
                        (instrument==PRJCT_INSTR_FORMAT_OMPS) ||
                        (instrument==PRJCT_INSTR_FORMAT_GOME1_NETCDF) ||
                        (instrument==PRJCT_INSTR_FORMAT_GEMS))?false:true;

  m_successCheck->setEnabled((analysisEnabled || calibrationEnabled) && m_successCheckEnable);

  m_selector->setInstrument(instrument,TAB_SELECTOR_OUTPUT);
}

void CWProjectTabOutput::slotAnalysisCheckChanged(int state)
{
  setComponentsEnabled((state == Qt::Checked),
                       (m_calibrationCheck->checkState() == Qt::Checked) );
}

void CWProjectTabOutput::slotCalibrationCheckChanged(int state)
{
  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked),
                       (state == Qt::Checked) );

  emit signalOutputCalibration((state == Qt::Checked));
}

void CWProjectTabOutput::slotReferenceCheckChanged(int state)
{
  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked),
                       (m_calibrationCheck->checkState() == Qt::Checked) || (state == Qt::Checked));
}

void CWProjectTabOutput::setComponentsEnabled(bool analysisEnabled, bool calibrationEnabled)
{
  bool allEnabled = (analysisEnabled || calibrationEnabled);

  m_directoryCheck->setEnabled(allEnabled);
  m_useFileName->setEnabled(allEnabled);

  m_successCheck->setEnabled(allEnabled && m_successCheckEnable);
  //m_newcalibCheck->setEnabled(calibrationEnabled);
  m_referenceCheck->setEnabled(allEnabled);
  m_pathFrame->setEnabled(allEnabled);

  m_editGroup->setEnabled(allEnabled);
  m_selector->setEnabled(allEnabled);
}
