/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include "CWUsampTabGeneral.h"
#include "CValidator.h"
#include "CPreferences.h"

#include "constants.h"


CWUsampTabGeneral::CWUsampTabGeneral(const mediate_usamp_t *properties, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(15);

  QGridLayout *fileLayout = new QGridLayout;
  int row = 0;

  fileLayout->setSpacing(3);

  // output phase 1
  fileLayout->addWidget(new QLabel("Output (Phase 1)", this), row, 0);
  m_outputPhaseOneFileEdit = new QLineEdit(this);
  m_outputPhaseOneFileEdit->setMaxLength(sizeof(properties->outputPhaseOneFile)-1);
  fileLayout->addWidget(m_outputPhaseOneFileEdit, row, 1);
  QPushButton *outputPhaseOneBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(outputPhaseOneBtn, row, 2);
  ++row;

  // output phase 2
  fileLayout->addWidget(new QLabel("Output (Phase 2)", this), row, 0);
  m_outputPhaseTwoFileEdit = new QLineEdit(this);
  m_outputPhaseTwoFileEdit->setMaxLength(sizeof(properties->outputPhaseTwoFile)-1);
  fileLayout->addWidget(m_outputPhaseTwoFileEdit, row, 1);
  QPushButton *outputPhaseTwoBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(outputPhaseTwoBtn, row, 2);
  ++row;

  // calibration
  fileLayout->addWidget(new QLabel("Calibration", this), row, 0);
  m_calibFileEdit = new QLineEdit(this);
  m_calibFileEdit->setMaxLength(sizeof(properties->calibrationFile)-1);
  fileLayout->addWidget(m_calibFileEdit, row, 1);
  QPushButton *calibBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(calibBtn, row, 2);
  ++row;

  // solar ref
  fileLayout->addWidget(new QLabel("Solar Ref.", this), row, 0);
  m_refFileEdit = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile)-1);
  fileLayout->addWidget(m_refFileEdit, row, 1);
  QPushButton *refBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(refBtn, row, 2);
  ++row;

  mainLayout->addLayout(fileLayout);

  m_slitEdit = new CWSlitSelector(&(properties->slit), "Slit Function", false,this);
  mainLayout->addWidget(m_slitEdit);

  // analysis
  QGroupBox *analysisGroup = new QGroupBox("Analysis", this);
  QGridLayout *analysisLayout = new QGridLayout(analysisGroup);
  analysisLayout->addWidget(new QLabel("Anylsis Method", analysisGroup), 0, 0);
  m_analysisCombo = new QComboBox(analysisGroup);
  m_analysisCombo->addItem("Optical density fitting", QVariant(OPTICAL_DENSITY_FIT));
  m_analysisCombo->addItem("Intensity fitting (Marquardt-Levenberg)", QVariant(INTENSITY_FIT));
  analysisLayout->addWidget(m_analysisCombo, 0, 1);

  analysisLayout->addWidget(new QLabel("Shift (nm)", this), 1, 0);
  m_shiftEdit = new QLineEdit(this);
  m_shiftEdit->setFixedWidth(100);
  m_shiftEdit->setValidator(new CDoubleFixedFmtValidator(-100.0, 100.0, 3, m_shiftEdit));
  analysisLayout->addWidget(m_shiftEdit, 1, 1);


  mainLayout->addWidget(analysisGroup);

  mainLayout->addStretch(1);

  m_headerCheck = new QCheckBox("Remove Header", this);
  mainLayout->addWidget(m_headerCheck, 0, Qt::AlignLeft);

  // initialize
  reset(properties);

  // connections
  connect(outputPhaseOneBtn, SIGNAL(clicked()), this, SLOT(slotBrowseOutputPhaseOne()));
  connect(outputPhaseTwoBtn, SIGNAL(clicked()), this, SLOT(slotBrowseOutputPhaseTwo()));
  connect(calibBtn, SIGNAL(clicked()), this, SLOT(slotBrowseCalibration()));
  connect(refBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarReference()));

}

CWUsampTabGeneral::~CWUsampTabGeneral()
{
}

void CWUsampTabGeneral::reset(const mediate_usamp_t *properties)
{
  // set/reset gui state from properties

  int index;
  QString tmpStr;

  index = m_analysisCombo->findData(QVariant(properties->methodType));
  if (index != -1)
    m_analysisCombo->setCurrentIndex(index);

  tmpStr.setNum(properties->shift);
  m_shiftEdit->validator()->fixup(tmpStr);
  m_shiftEdit->setText(tmpStr);

  m_headerCheck->setCheckState(properties->noheader ? Qt::Checked : Qt::Unchecked);

  // files
  m_outputPhaseOneFileEdit->setText(properties->outputPhaseOneFile);
  m_outputPhaseTwoFileEdit->setText(properties->outputPhaseTwoFile);
  m_calibFileEdit->setText(properties->calibrationFile);
  m_refFileEdit->setText(properties->solarRefFile);

  // slit function
  m_slitEdit->reset(&(properties->slit));
}

void CWUsampTabGeneral::apply(mediate_usamp_t *properties) const
{
  properties->noheader = (m_headerCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->methodType = m_analysisCombo->itemData(m_analysisCombo->currentIndex()).toInt();
  properties->shift = m_shiftEdit->text().toDouble();

  strcpy(properties->outputPhaseOneFile, m_outputPhaseOneFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->outputPhaseTwoFile, m_outputPhaseTwoFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->calibrationFile, m_calibFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->solarRefFile, m_refFileEdit->text().toLocal8Bit().constData());

  m_slitEdit->apply(&(properties->slit));
}

void CWUsampTabGeneral::slotBrowseOutputPhaseOne()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getSaveFileName(this, "Output File (Phase 1)",
                          pref->directoryName("Output"),
                          "All files (*)");


  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Output", filename);

    m_outputPhaseOneFileEdit->setText(filename);
  }
}

void CWUsampTabGeneral::slotBrowseOutputPhaseTwo()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getSaveFileName(this, "Output File (Phase 2)",
                          pref->directoryName("Output"),
                          "All files (*)");


  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Output", filename);

    m_outputPhaseTwoFileEdit->setText(filename);
  }
}

void CWUsampTabGeneral::slotBrowseCalibration()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Calibration File",
                          pref->directoryName("Calib"),
                          "Calibration File (*.clb);;All files (*)");
  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Calib", filename);

    m_calibFileEdit->setText(filename);
  }
}

void CWUsampTabGeneral::slotBrowseSolarReference()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "reference File",
                          pref->directoryName("Ref"),
                          "Kurucz File (*.ktz);;All files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);

    m_refFileEdit->setText(filename);
  }
}

