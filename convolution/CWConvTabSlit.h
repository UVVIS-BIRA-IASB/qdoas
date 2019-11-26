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
