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
  QLineEdit *m_outputFileEdit, *m_calibFileEdit, *m_refFileEdit;
  CWSlitSelector *m_slitEdit;
  QLineEdit *m_tempEdit;
  QCheckBox *m_normalizeCheck;
  QCheckBox *m_headerCheck;
  QCheckBox *m_ramanCheck;
};

#endif
