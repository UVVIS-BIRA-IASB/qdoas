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
