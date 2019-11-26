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
  mainLayout->setMargin(25);
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

