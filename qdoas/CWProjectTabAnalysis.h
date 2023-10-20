/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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


