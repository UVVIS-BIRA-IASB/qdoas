/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWUSAMPTABGENERAL_H_GUARD
#define _CWUSAMPTABGENERAL_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "mediate_usamp.h"

#include "CWSlitEditors.h"

class CWUsampTabGeneral : public QFrame
{
Q_OBJECT
 public:
  CWUsampTabGeneral(const mediate_usamp_t *properties, QWidget *parent = 0);
  virtual ~CWUsampTabGeneral();

  void reset(const mediate_usamp_t *properties);
  void apply(mediate_usamp_t *properties) const;

  public slots:
    void slotBrowseOutputPhaseOne(void);
    void slotBrowseOutputPhaseTwo(void);
    void slotBrowseCalibration(void);
    void slotBrowseSolarReference(void);

 private:
  QLineEdit *m_outputPhaseOneFileEdit, *m_outputPhaseTwoFileEdit, *m_calibFileEdit, *m_refFileEdit;
  CWSlitSelector *m_slitEdit;
  QComboBox *m_analysisCombo;
  QLineEdit *m_shiftEdit;
  QCheckBox *m_headerCheck;
};

#endif
