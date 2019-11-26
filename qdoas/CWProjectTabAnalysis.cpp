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

#include "CWProjectTabAnalysis.h"
#include "CValidator.h"

#include "constants.h"
#include <stdio.h>

static const int cStandardEditWidth = 75;

CWProjectTabAnalysis::CWProjectTabAnalysis(const mediate_project_analysis_t *properties,
					   QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setSpacing(5);

  int row = 0;
  mainLayout->setRowStretch(row, 1);
  ++row;

  // Method
  mainLayout->addWidget(new QLabel("Analysis Method", this), row , 1);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical density fitting",
			 QVariant(OPTICAL_DENSITY_FIT));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)",
			 QVariant(INTENSITY_FIT));
  mainLayout->addWidget(m_methodCombo, row, 2);
  ++row;

  // Fit
  mainLayout->addWidget(new QLabel("Least Squares Fit", this), row, 1);
  m_fitCombo = new QComboBox(this);
  m_fitCombo->addItem("No Weighting", QVariant(PRJCT_ANLYS_FIT_WEIGHTING_NONE));
  m_fitCombo->addItem("Instrumental", QVariant(PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL));
  //m_fitCombo->addItem("Statistical", QVariant(PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL));
  mainLayout->addWidget(m_fitCombo, row, 2);
  ++row;

  // Units - only PRJCT_ANLYS_UNITS_NANOMETERS

  // Interpolation
  mainLayout->addWidget(new QLabel("Interpolation", this), row, 1);
  m_interpCombo = new QComboBox(this);
  m_interpCombo->addItem("Linear", QVariant(PRJCT_ANLYS_INTERPOL_LINEAR));
  m_interpCombo->addItem("Spline", QVariant(PRJCT_ANLYS_INTERPOL_SPLINE));
  mainLayout->addWidget(m_interpCombo, row, 2);
  ++row;

  // gap
  mainLayout->addWidget(new QLabel("Interpolation security gap", this), row, 1);
  m_interpolationSecuritySpinBox = new QSpinBox(this);
  m_interpolationSecuritySpinBox->setRange(1, 50);
  m_interpolationSecuritySpinBox->setFixedWidth(cStandardEditWidth);
  mainLayout->addWidget(m_interpolationSecuritySpinBox, row, 2);
  ++row;

  // convergence
  mainLayout->addWidget(new QLabel("Convergence criterion", this), row, 1);
  m_convergenceCriterionEdit = new QLineEdit(this);
  m_convergenceCriterionEdit->setValidator(new CDoubleExpFmtValidator(1.0e-30, 1.0, 4, m_convergenceCriterionEdit));
  m_convergenceCriterionEdit->setFixedWidth(cStandardEditWidth);
  mainLayout->addWidget(m_convergenceCriterionEdit, row, 2);
  ++row;

  mainLayout->addWidget(new QLabel("Maximum number of iterations", this), row, 1);
  m_maxIterationsSpinBox = new QSpinBox(this);
  m_maxIterationsSpinBox->setRange(0, 50);
  m_maxIterationsSpinBox->setFixedWidth(cStandardEditWidth);
  mainLayout->addWidget(m_maxIterationsSpinBox, row, 2);
  ++row;

  // Residual spike tolerance
  mainLayout->addWidget(new QLabel("Spike tolerance factor (>3.0)", this), row, 1);
  m_spikeTolerance = new QLineEdit(this);
  m_spikeTolerance->setFixedWidth(cStandardEditWidth);
  m_spikeTolerance->setValidator(new CDoubleFixedFmtValidator(3., 999.9, 1, m_spikeTolerance));  // residuals up to 3x average are always allowed.
  mainLayout->addWidget(m_spikeTolerance, row, 2);
  ++row;

  mainLayout->setRowStretch(row, 4);
  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(3, 1);

  // set initial values

  index = m_methodCombo->findData(QVariant(properties->methodType));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  index = m_fitCombo->findData(QVariant(properties->fitType));
  if (index != -1)
    m_fitCombo->setCurrentIndex(index);

  index = m_interpCombo->findData(QVariant(properties->interpolationType));
  if (index != -1)
    m_interpCombo->setCurrentIndex(index);

  m_interpolationSecuritySpinBox->setValue(properties->interpolationSecurityGap);
  m_maxIterationsSpinBox->setValue(properties->maxIterations);

  // validator controls the initial range and format
  m_convergenceCriterionEdit->validator()->fixup(tmpStr.setNum(properties->convergenceCriterion));
  m_convergenceCriterionEdit->setText(tmpStr);

  m_spikeTolerance->validator()->fixup(tmpStr.setNum(properties->spike_tolerance));
  m_spikeTolerance->setText(tmpStr);

}

void CWProjectTabAnalysis::apply(mediate_project_analysis_t *properties) const
{
  bool ok;
  int index;
  double tmpDouble;

  // must have a current item so no need to check
  index = m_methodCombo->currentIndex();
  properties->methodType = m_methodCombo->itemData(index).toInt();

  index = m_fitCombo->currentIndex();
  properties->fitType = m_fitCombo->itemData(index).toInt();

  index = m_interpCombo->currentIndex();
  properties->interpolationType = m_interpCombo->itemData(index).toInt();

  properties->interpolationSecurityGap = m_interpolationSecuritySpinBox->value();
  properties->maxIterations = m_maxIterationsSpinBox->value();

  tmpDouble = m_convergenceCriterionEdit->text().toDouble(&ok);

  // default if not ok
  properties->convergenceCriterion = ok ? tmpDouble : 1.0e-4;

  // spikes
  tmpDouble = m_spikeTolerance->text().toDouble(&ok);
  properties->spike_tolerance = ok ? tmpDouble : 999.9;

}
