/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include "CWConvTabSlit.h"
#include "CValidator.h"

#include "constants.h"


CWConvTabSlit::CWConvTabSlit(const mediate_slit_function_t *conv, const mediate_slit_function_t *deconv, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(15);

  m_convEdit = new CWSlitSelector(conv, "Convolution", false,this);
  mainLayout->addWidget(m_convEdit);

  m_deconvEdit = new CWSlitSelector(deconv, "Deconvolution", false,this);
  mainLayout->addWidget(m_deconvEdit);
}

CWConvTabSlit::~CWConvTabSlit()
{
}

void CWConvTabSlit::reset(const mediate_slit_function_t *conv, const mediate_slit_function_t *deconv)
{
  m_convEdit->reset(conv);
  m_deconvEdit->reset(deconv);
}

void CWConvTabSlit::apply(mediate_slit_function_t *conv, mediate_slit_function_t *deconv) const
{
  m_convEdit->apply(conv);
  m_deconvEdit->apply(deconv);
}

