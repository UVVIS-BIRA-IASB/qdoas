/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWPROJECTTABDISPLAY_H_GUARD
#define _CWPROJECTTABDISPLAY_H_GUARD

#include <QFrame>
#include <QCheckBox>

#include "mediate_project.h"

class CWOutputSelector;

class CWProjectTabDisplay : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabDisplay(const mediate_project_display_t *properties, QWidget *parent = 0);

  void apply(mediate_project_display_t *properties) const;

 public slots:
  void slotInstrumentChanged(int instrument);

 private:
  QCheckBox *m_reqSpectraCheck, *m_reqDataCheck, *m_reqFitsCheck, *m_reqCalibCheck;
  CWOutputSelector *m_selector;
};

#endif
