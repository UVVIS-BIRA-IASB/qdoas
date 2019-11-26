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


#include <QDialog>

#include "CWPlotPropertiesConfig.h"

#include "debugutil.h"

#ifndef _CWPLOTPROPERTIESDIALOG_H_GUARD
#define _CWPLOTPROPERTIESDIALOG_H_GUARD

class CWPlotPropertiesDialog : public QDialog
{
 public:
  CWPlotPropertiesDialog(CPlotProperties &prop, QWidget *parent = 0);

  virtual void accept(); 

 private:
  CPlotProperties &m_properties;
  CWPlotPropertiesConfig *m_config;
};


#endif

