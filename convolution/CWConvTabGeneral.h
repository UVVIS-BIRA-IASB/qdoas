/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWCONVTABGENERAL_H_GUARD
#define _CWCONVTABGENERAL_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "mediate_convolution.h"

class CWConvTabGeneral : public QFrame
{
Q_OBJECT
 public:
  CWConvTabGeneral(const mediate_conv_general_t *properties, QWidget *parent = 0);
  virtual ~CWConvTabGeneral();

  void reset(const mediate_conv_general_t *properties);
  void apply(mediate_conv_general_t *properties) const;

  public slots:
    void slotBrowseInput(void);
    void slotBrowseOutput(void);
    void slotBrowseCalibration(void);
    void slotBrowseSolarReference(void);

 private:
  QComboBox *m_convolutionCombo, *m_conversionCombo, *m_formatCombo;
  QLineEdit *m_shiftEdit, *m_concEdit;
  QLineEdit *m_inputFileEdit, *m_outputFileEdit, *m_calibFileEdit, *m_refFileEdit, *m_pixelEdit;
  QCheckBox *m_headerCheck;
};

#endif
