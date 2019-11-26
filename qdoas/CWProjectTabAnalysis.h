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

#ifndef _CWPROJECTTABANALYSIS_H_GUARD
#define _CWPROJECTTABANALYSIS_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#include "mediate_project.h"

class CWProjectTabAnalysis : public QFrame
{
 public:
  CWProjectTabAnalysis(const mediate_project_analysis_t *properties, QWidget *parent = 0);

  void apply(mediate_project_analysis_t *properties) const;

 private:
  QComboBox *m_methodCombo, *m_fitCombo, *m_interpCombo;
  QSpinBox *m_interpolationSecuritySpinBox;
  QLineEdit *m_convergenceCriterionEdit, *m_spikeTolerance;
  QSpinBox *m_maxIterationsSpinBox;
};

#endif


