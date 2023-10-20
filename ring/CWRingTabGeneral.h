/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWRINGTABGENERAL_H_GUARD
#define _CWRINGTABGENERAL_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "mediate_ring.h"

#include "CWSlitEditors.h"

class CWRingTabGeneral : public QFrame
{
Q_OBJECT
 public:
  CWRingTabGeneral(const mediate_ring_t *properties, QWidget *parent = 0);
  virtual ~CWRingTabGeneral();

  void reset(const mediate_ring_t *properties);
  void apply(mediate_ring_t *properties) const;

  public slots:
    void slotBrowseOutput(void);
    void slotBrowseCalibration(void);
    void slotBrowseSolarReference(void);

 private:
  QComboBox *m_formatCombo;
  QLineEdit *m_outputFileEdit, *m_calibFileEdit, *m_refFileEdit, *m_pixelEdit;
  CWSlitSelector *m_slitEdit;
  QLineEdit *m_tempEdit;
  QCheckBox *m_normalizeCheck;
  QCheckBox *m_headerCheck;
  QCheckBox *m_ramanCheck;
};

#endif
