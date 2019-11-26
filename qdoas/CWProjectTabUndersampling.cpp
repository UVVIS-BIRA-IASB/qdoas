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


#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include "CWProjectTabUndersampling.h"
#include "CValidator.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabUndersampling::CWProjectTabUndersampling(const mediate_project_undersampling_t *properties, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QGridLayout *topLayout = new QGridLayout;

  mainLayout->addSpacing(25);

  // ref file
  topLayout->addWidget(new QLabel("Solar Ref. File", this), 0, 0);
  m_refFileEdit  = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile) - 1); // limit test length to buffer size

  topLayout->addWidget(m_refFileEdit, 0, 1);
  QPushButton *browseBtn = new QPushButton("Browse...", this);
  topLayout->addWidget(browseBtn, 0, 2);

  // method
  topLayout->addWidget(new QLabel("Method", this), 1, 0);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("From File", QVariant(PRJCT_USAMP_FILE));
  m_methodCombo->addItem("Fixed Phase", QVariant(PRJCT_USAMP_FIXED));
  m_methodCombo->addItem("Automatic Phase", QVariant(PRJCT_USAMP_AUTOMATIC));
  topLayout->addWidget(m_methodCombo, 1, 1);

  // shift
  topLayout->addWidget(new QLabel("Shift", this), 2, 0);
  m_shiftEdit = new QLineEdit(this);
  m_shiftEdit->setFixedWidth(70);
  m_shiftEdit->setValidator(new CDoubleFixedFmtValidator(-50.0, 50.0, 4, m_shiftEdit));
  topLayout->addWidget(m_shiftEdit, 2, 1);

  mainLayout->addLayout(topLayout);
  
  mainLayout->addStretch(1);

  // set initial values

  m_refFileEdit->setText(QString(properties->solarRefFile));

  index = m_methodCombo->findData(QVariant(properties->method));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  m_shiftEdit->validator()->fixup(tmpStr.setNum(properties->shift));
  m_shiftEdit->setText(tmpStr);

  // connections
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarRefFile()));
  
}

void CWProjectTabUndersampling::apply(mediate_project_undersampling_t *properties) const
{
  // a safe text length is assured.
  strcpy(properties->solarRefFile, m_refFileEdit->text().toLocal8Bit().data());

  properties->method = m_methodCombo->itemData(m_methodCombo->currentIndex()).toInt();
  
  properties->shift = m_shiftEdit->text().toDouble();
}

void CWProjectTabUndersampling::slotBrowseSolarRefFile()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Open Solar Reference File",
						  pref->directoryName("Ref"),
                                                  "Kurucz File (*.ktz);;All Files (*)");
  
  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);
    
    m_refFileEdit->setText(filename);
  }
}

