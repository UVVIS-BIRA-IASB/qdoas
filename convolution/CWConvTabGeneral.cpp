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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include "CWConvTabGeneral.h"
#include "CValidator.h"
#include "CPreferences.h"

#include "constants.h"


CWConvTabGeneral::CWConvTabGeneral(const mediate_conv_general_t *properties, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(15);

  QHBoxLayout *topLayout = new QHBoxLayout;

  // type group
  QGroupBox *typeGroup = new QGroupBox(this);
  QGridLayout *typeLayout = new QGridLayout(typeGroup);

  typeLayout->addWidget(new QLabel("Convolution", this), 0, 0);
  m_convolutionCombo = new QComboBox(this);
  m_convolutionCombo->addItem("None", QVariant(CONVOLUTION_TYPE_NONE));
  m_convolutionCombo->addItem("Standard", QVariant(CONVOLUTION_TYPE_STANDARD));
  m_convolutionCombo->addItem("Io Correction", QVariant(CONVOLUTION_TYPE_I0_CORRECTION));
  typeLayout->addWidget(m_convolutionCombo, 0, 1);

  typeLayout->addWidget(new QLabel("Conversion", this), 1, 0);
  m_conversionCombo = new QComboBox(this);
  m_conversionCombo->addItem("None", QVariant(CONVOLUTION_CONVERSION_NONE));
  m_conversionCombo->addItem("Air -> Vacuum", QVariant(CONVOLUTION_CONVERSION_AIR2VAC));
  m_conversionCombo->addItem("Vacuum -> Air", QVariant(CONVOLUTION_CONVERSION_VAC2AIR));
  typeLayout->addWidget(m_conversionCombo, 1, 1);
  
  typeLayout->setColumnStretch(2, 1);
  topLayout->addWidget(typeGroup);

  // valueGroup
  QGroupBox *valueGroup = new QGroupBox(this);
  QGridLayout *valueLayout = new QGridLayout(valueGroup);

  valueLayout->addWidget(new QLabel("Shift", this), 0, 0);
  m_shiftEdit = new QLineEdit(this);
  m_shiftEdit->setValidator(new CDoubleFixedFmtValidator(-100.0, 100.0, 3, m_shiftEdit));
  valueLayout->addWidget(m_shiftEdit, 0, 1);

  valueLayout->addWidget(new QLabel("Conc.", this), 1, 0);
  m_concEdit = new QLineEdit(this);
  m_concEdit->setValidator(new CDoubleExpFmtValidator(0.0, 1.0e30, 3, m_concEdit));
  valueLayout->addWidget(m_concEdit, 1, 1);

  valueLayout->setColumnStretch(2, 1);
  topLayout->addWidget(valueGroup);

  mainLayout->addLayout(topLayout);

  QGridLayout *fileLayout = new QGridLayout;
  int row = 0;

  fileLayout->setSpacing(3);

  // input
  fileLayout->addWidget(new QLabel("Input", this), row, 0);
  m_inputFileEdit = new QLineEdit(this);
  m_inputFileEdit->setMaxLength(sizeof(properties->inputFile)-1);
  fileLayout->addWidget(m_inputFileEdit, row, 1);
  QPushButton *inputBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(inputBtn, row, 2);
  ++row;

  // output
  fileLayout->addWidget(new QLabel("Output", this), row, 0);
  m_outputFileEdit = new QLineEdit(this);
  m_outputFileEdit->setMaxLength(sizeof(properties->outputFile)-1);
  fileLayout->addWidget(m_outputFileEdit, row, 1);
  QPushButton *outputBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(outputBtn, row, 2);
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

  mainLayout->addStretch(1);

  m_headerCheck = new QCheckBox("Remove Header", this);
  mainLayout->addWidget(m_headerCheck, 0, Qt::AlignLeft);

  // initialize
  reset(properties);

  // connections
  connect(inputBtn, SIGNAL(clicked()), this, SLOT(slotBrowseInput()));
  connect(outputBtn, SIGNAL(clicked()), this, SLOT(slotBrowseOutput()));
  connect(calibBtn, SIGNAL(clicked()), this, SLOT(slotBrowseCalibration()));
  connect(refBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarReference()));

}

CWConvTabGeneral::~CWConvTabGeneral()
{
}

void CWConvTabGeneral::reset(const mediate_conv_general_t *properties)
{
  // set/reset gui state from properties

  int index;
  QString tmpStr;

  m_inputFileEdit->setText(properties->inputFile);
  m_outputFileEdit->setText(properties->outputFile);
  m_calibFileEdit->setText(properties->calibrationFile);
  m_refFileEdit->setText(properties->solarRefFile);

  // convolution type
  index = m_convolutionCombo->findData(QVariant(properties->convolutionType));
  if (index != -1)
    m_convolutionCombo->setCurrentIndex(index);

  // conversion type
  index = m_conversionCombo->findData(QVariant(properties->conversionType));
  if (index != -1)
    m_conversionCombo->setCurrentIndex(index);

  // shift
  tmpStr.setNum(properties->shift);
  m_shiftEdit->validator()->fixup(tmpStr);
  m_shiftEdit->setText(tmpStr);
  // conc
  tmpStr.setNum(properties->conc);
  m_concEdit->validator()->fixup(tmpStr);
  m_concEdit->setText(tmpStr);

  m_headerCheck->setCheckState(properties->noheader ? Qt::Checked : Qt::Unchecked);
}

void CWConvTabGeneral::apply(mediate_conv_general_t *properties) const
{
  properties->convolutionType = m_convolutionCombo->itemData(m_convolutionCombo->currentIndex()).toInt();
  properties->conversionType = m_conversionCombo->itemData(m_conversionCombo->currentIndex()).toInt();

  properties->shift = m_shiftEdit->text().toDouble();
  properties->conc = m_concEdit->text().toDouble();

  strcpy(properties->inputFile, m_inputFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->outputFile, m_outputFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->calibrationFile, m_calibFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->solarRefFile, m_refFileEdit->text().toLocal8Bit().constData());

  properties->noheader = (m_headerCheck->checkState() == Qt::Checked) ? 1 : 0;
}

void CWConvTabGeneral::slotBrowseInput()
{
  CPreferences *pref = CPreferences::instance();
  
  QString filename = QFileDialog::getOpenFileName(this, "Input File",
						  pref->directoryName("Input"),
						  "All files (*)");
  
  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Input", filename);
    
    m_inputFileEdit->setText(filename);
  }
}

void CWConvTabGeneral::slotBrowseOutput()
{
  CPreferences *pref = CPreferences::instance();
  
  QString filename = QFileDialog::getSaveFileName(this, "Output File",
						  pref->directoryName("Output"),
						  "All files (*)");
  
  
  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Output", filename);
    
    m_outputFileEdit->setText(filename);
  }
}

void CWConvTabGeneral::slotBrowseCalibration()
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

void CWConvTabGeneral::slotBrowseSolarReference()
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

