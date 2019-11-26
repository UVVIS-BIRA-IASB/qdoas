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


#ifndef _CWPROJECTTABSELECTION_H_GUARD
#define _CWPROJECTTABSELECTION_H_GUARD

#include <QFrame>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>
#include <QGroupBox>

#include "mediate_project.h"

class CWGeolocation;

class CWProjectTabSelection : public QFrame
{
	Q_OBJECT
 public:
  CWProjectTabSelection(const mediate_project_selection_t *properties, QWidget *parent = 0);

  void apply(mediate_project_selection_t *properties) const;

 public slots:
  void slotInstrumentChanged(int instrument);
  void slotInstrumentTypeChanged(int instrumentType);

 private:
  QLineEdit *m_szaMinEdit, *m_szaMaxEdit, *m_szaDeltaEdit;
  QLineEdit *m_refAngleEdit, *m_refTolEdit;
  QLineEdit *m_recordMinEdit, *m_recordMaxEdit;
  QLineEdit *m_elevationMinEdit, *m_elevationMaxEdit, *m_elevationTolEdit;
  QLineEdit *m_cloudFractionMinEdit, *m_cloudFractionMaxEdit;
  CWGeolocation *m_geolocationEdit;
  QGroupBox *m_cloudFractionGroup,*m_geolocationGroup,*m_elevationGroup,*m_refGroup;
};

class CWGeolocation : public QFrame
{
 public:
  CWGeolocation(const struct geolocation *geo, QWidget *parent = 0);

  void apply(struct geolocation *geo) const;

 private:
  QLineEdit *m_westEdit, *m_eastEdit, *m_southEdit, *m_northEdit;
  QLineEdit *m_cenLongEdit, *m_cenLatEdit, *m_radiusEdit, *m_sitesRadiusEdit;
  QComboBox *m_modeCombo;
  QStackedWidget *m_modeStack;
};

#endif
