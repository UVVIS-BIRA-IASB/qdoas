/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWCONVTABSLIT_H_GUARD
#define _CWCONVTABSLIT_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>

#include "mediate_convolution.h"

#include "CWSlitEditors.h"

class CWConvTabSlit : public QFrame
{
Q_OBJECT
 public:
  CWConvTabSlit(const mediate_slit_function_t *conv, const mediate_slit_function_t *deconv, QWidget *parent = 0);
  virtual ~CWConvTabSlit();

  void reset(const mediate_slit_function_t *conv, const mediate_slit_function_t *deconv);
  void apply(mediate_slit_function_t *conv, mediate_slit_function_t *deconv) const;

 private:
  CWSlitSelector *m_convEdit, *m_deconvEdit;
};

#endif
