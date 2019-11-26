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

#include "CWRingTabGeneral.h"
#include "CValidator.h"
#include "CPreferences.h"

#include "constants.h"


CWRingTabGeneral::CWRingTabGeneral(const mediate_ring_t *properties, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(15);

  QGridLayout *fileLayout = new QGridLayout;
  int row = 0;

  fileLayout->setSpacing(3);

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

  m_slitEdit = new CWSlitSelector(&(properties->slit), "Slit Function", true,this);
  mainLayout->addWidget(m_slitEdit);

  QHBoxLayout *tempLayout = new QHBoxLayout;
  tempLayout->setMargin(0);
  tempLayout->addWidget(new QLabel("Temperature (K)", this));
  m_tempEdit = new QLineEdit(this);
  m_tempEdit->setValidator(new CDoubleFixedFmtValidator(0, 999, 1, m_tempEdit));
  tempLayout->addWidget(m_tempEdit);
  tempLayout->addStretch(1);

  mainLayout->addLayout(tempLayout);

  mainLayout->addStretch(1);

  m_normalizeCheck = new QCheckBox("Apply normalization", this);
  mainLayout->addWidget(m_normalizeCheck, 0, Qt::AlignLeft);

  m_headerCheck = new QCheckBox("Remove Header", this);
  mainLayout->addWidget(m_headerCheck, 0, Qt::AlignLeft);

  m_ramanCheck = new QCheckBox("Save Raman and solar spectra", this);
  mainLayout->addWidget(m_ramanCheck, 0, Qt::AlignLeft);

  // initialize
  reset(properties);

  // connections
  connect(outputBtn, SIGNAL(clicked()), this, SLOT(slotBrowseOutput()));
  connect(calibBtn, SIGNAL(clicked()), this, SLOT(slotBrowseCalibration()));
  connect(refBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarReference()));

}

CWRingTabGeneral::~CWRingTabGeneral()
{
}

void CWRingTabGeneral::reset(const mediate_ring_t *properties)
{
  // set/reset gui state from properties

  // temp
  QString tmpStr;

  tmpStr.setNum(properties->temperature);
  m_tempEdit->validator()->fixup(tmpStr);
  m_tempEdit->setText(tmpStr);

  m_normalizeCheck->setCheckState(properties->normalize ? Qt::Checked : Qt::Unchecked);
  m_headerCheck->setCheckState(properties->noheader ? Qt::Checked : Qt::Unchecked);
  m_ramanCheck->setCheckState(properties->saveraman ? Qt::Checked : Qt::Unchecked);

  // files
  m_outputFileEdit->setText(properties->outputFile);
  m_calibFileEdit->setText(properties->calibrationFile);
  m_refFileEdit->setText(properties->solarRefFile);

  // slit function
  m_slitEdit->reset(&(properties->slit));
}

void CWRingTabGeneral::apply(mediate_ring_t *properties) const
{
 	properties->normalize = (m_normalizeCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->noheader = (m_headerCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->saveraman = (m_ramanCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->temperature = m_tempEdit->text().toDouble();

  strcpy(properties->outputFile, m_outputFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->calibrationFile, m_calibFileEdit->text().toLocal8Bit().constData());
  strcpy(properties->solarRefFile, m_refFileEdit->text().toLocal8Bit().constData());

  m_slitEdit->apply(&(properties->slit));
}

void CWRingTabGeneral::slotBrowseOutput()
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

void CWRingTabGeneral::slotBrowseCalibration()
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

void CWRingTabGeneral::slotBrowseSolarReference()
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

