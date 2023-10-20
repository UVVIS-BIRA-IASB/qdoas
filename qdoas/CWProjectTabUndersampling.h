/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWPROJECTTABUNDERSAMPLING_H_GUARD
#define _CWPROJECTTABUNDERSAMPLING_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

#include "mediate_project.h"


class CWProjectTabUndersampling : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabUndersampling(const mediate_project_undersampling_t *properties, QWidget *parent = 0);

  void apply(mediate_project_undersampling_t *properties) const;

 public slots:
  void slotBrowseSolarRefFile();

 private:
  QLineEdit *m_refFileEdit;
  QComboBox *m_methodCombo;
  QLineEdit *m_shiftEdit;
};

#endif
