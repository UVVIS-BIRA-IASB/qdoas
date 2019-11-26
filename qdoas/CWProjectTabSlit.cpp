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
#include <QLabel>
#include <QPushButton>
#include <QFontMetrics>
#include <QFileDialog>

#include "CWProjectTabSlit.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

//--------------------------------------------------------

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cStandardEditWidth         = 70;

//--------------------------------------------------------

CWProjectTabSlit::CWProjectTabSlit(const mediate_project_slit_t *slit, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(25);

  QGridLayout *topLayout = new QGridLayout;

  // solar reference
  topLayout->addWidget(new QLabel("Solar Ref. File", this), 0, 0);
  m_solarRefFileEdit = new QLineEdit(this);
  m_solarRefFileEdit->setMaxLength(sizeof(slit->solarRefFile)-1);
  topLayout->addWidget(m_solarRefFileEdit, 0, 1);
  QPushButton *refBrowseBtn = new QPushButton("Browse", this);
  topLayout->addWidget(refBrowseBtn, 0, 2);

  // slit type
  m_slitCombo = new QComboBox(this);
  m_slitStack = new QStackedWidget(this);
  // insert widgets into the stack and items into the combo in lock-step.

  m_noneEdit = new CWSlitNoneEdit(&(slit->function.file));
  m_slitStack->addWidget(m_noneEdit);
  m_slitCombo->addItem("None", QVariant(SLIT_TYPE_NONE));

  m_fileEdit = new CWSlitFileEdit(&(slit->function.file));
  m_slitStack->addWidget(m_fileEdit);
  m_slitCombo->addItem("File", QVariant(SLIT_TYPE_FILE));

  m_gaussianEdit = new CWSlitGaussianEdit(&(slit->function.gaussian));
  m_slitStack->addWidget(m_gaussianEdit);
  m_slitCombo->addItem("Gaussian", QVariant(SLIT_TYPE_GAUSS));

  m_lorentzEdit = new CWSlitLorentzEdit(&(slit->function.lorentz));
  m_slitStack->addWidget(m_lorentzEdit);
  m_slitCombo->addItem("2n-Lorentz", QVariant(SLIT_TYPE_INVPOLY));

  m_voigtEdit = new CWSlitVoigtEdit(&(slit->function.voigt));
  m_slitStack->addWidget(m_voigtEdit);
  m_slitCombo->addItem("Voigt", QVariant(SLIT_TYPE_VOIGT));

  m_errorEdit = new CWSlitErrorEdit(&(slit->function.error));
  m_slitStack->addWidget(m_errorEdit);
  m_slitCombo->addItem("Error Function", QVariant(SLIT_TYPE_ERF));

  m_agaussEdit = new CWSlitAGaussEdit(&(slit->function.agauss));
  m_slitStack->addWidget(m_agaussEdit);
  m_slitCombo->addItem("Asymmetric Gaussian", QVariant(SLIT_TYPE_AGAUSS));

  m_supergaussEdit = new CWSlitSuperGaussEdit(&(slit->function.supergauss));
  m_slitStack->addWidget(m_supergaussEdit);
  m_slitCombo->addItem("Super Gaussian", QVariant(SLIT_TYPE_SUPERGAUSS));

  m_boxcarApodEdit = new CWSlitApodEdit(&(slit->function.boxcarapod));
  m_slitStack->addWidget(m_boxcarApodEdit);
  m_slitCombo->addItem("Boxcar (FTS)", QVariant(SLIT_TYPE_APOD));

  m_nbsApodEdit = new CWSlitApodEdit(&(slit->function.nbsapod));
  m_slitStack->addWidget(m_nbsApodEdit);
  m_slitCombo->addItem("Norton Beer Strong (FTS)", QVariant(SLIT_TYPE_APODNBS));

  topLayout->addWidget(new QLabel("Slit Function Type", this), 2, 0);
  topLayout->addWidget(m_slitCombo, 2, 1, 1, 2);

  topLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  topLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  topLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(topLayout);
  mainLayout->addWidget(m_slitStack);
  mainLayout->addStretch(1);

  // connections
  connect(refBrowseBtn, SIGNAL(clicked()), this, SLOT(slotSolarRefFileBrowse()));
  connect(m_slitCombo, SIGNAL(currentIndexChanged(int)), m_slitStack, SLOT(setCurrentIndex(int)));

  // initialize
  m_solarRefFileEdit->setText(slit->solarRefFile);
  // m_fwhmCorrectionCheck->setCheckState(slit->applyFwhmCorrection ? Qt::Checked : Qt::Unchecked);
  // set the current slit - stack will follow
  int index = m_slitCombo->findData(QVariant(slit->function.type));
  if (index != -1)
    m_slitCombo->setCurrentIndex(index);

 }

void CWProjectTabSlit::apply(mediate_project_slit_t *slit) const
{
  // set values for ALL slits ... and the selected slit type

  slit->function.type = m_slitCombo->itemData(m_slitCombo->currentIndex()).toInt();
  // slit->applyFwhmCorrection = m_fwhmCorrectionCheck->isChecked() ? 1 : 0;
  strcpy(slit->solarRefFile, m_solarRefFileEdit->text().toLocal8Bit().data());

  m_noneEdit->apply(NULL);
  m_fileEdit->apply(&(slit->function.file));
  m_gaussianEdit->apply(&(slit->function.gaussian));
  m_lorentzEdit->apply(&(slit->function.lorentz));
  m_voigtEdit->apply(&(slit->function.voigt));
  m_errorEdit->apply(&(slit->function.error));
  m_agaussEdit->apply(&(slit->function.agauss));
  m_supergaussEdit->apply(&(slit->function.supergauss));
  m_boxcarApodEdit->apply(&(slit->function.boxcarapod));
  m_nbsApodEdit->apply(&(slit->function.nbsapod));
  // not used anymore : commented on 01/02/2012 m_gaussianFileEdit->apply(&(slit->function.gaussianfile));
  // not used anymore : commented on 01/02/2012 m_lorentzFileEdit->apply(&(slit->function.lorentzfile));
  // not used anymore : commented on 01/02/2012 m_errorFileEdit->apply(&(slit->function.errorfile));
  // not used anymore : commented on 12/01/2012 m_gaussianTempFileEdit->apply(&(slit->function.gaussiantempfile));
  // not used anymore : commented on 12/01/2012 m_errorTempFileEdit->apply(&(slit->function.errortempfile));
}

void CWProjectTabSlit::slotSolarRefFileBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Solar Reference File",
						  pref->directoryName("Ref"),
						  "Kurucz File (*.ktz);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);

    m_solarRefFileEdit->setText(filename);
  }
}

