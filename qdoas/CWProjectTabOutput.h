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


#ifndef _CWPROJECTTABOUTPUT_H_GUARD
#define _CWPROJECTTABOUTPUT_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>

#include "mediate_project.h"

class CWOutputSelector;

class CWProjectTabOutput : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabOutput(const mediate_project_output_t *properties, QWidget *parent = 0);

  void apply(mediate_project_output_t *properties) const;

  void setComponentsEnabled(bool analysisEnabled, bool calibrationEnabled);

 public slots:
  void slotBrowsePath();
  void slotInstrumentChanged(int instrument);
  void slotAnalysisCheckChanged(int state);
  void slotCalibrationCheckChanged(int state);
  void slotReferenceCheckChanged(int state);
  void slotSelectFileFormatChanged(int index);

 signals:
  void signalOutputCalibration(bool enabled);

 private:
  QFrame *m_pathFrame;
  QLineEdit *m_pathEdit;
  QCheckBox *m_analysisCheck, *m_calibrationCheck, *m_newcalibCheck,*m_referenceCheck;
  QCheckBox *m_directoryCheck;
  QCheckBox *m_useFileName;
  QCheckBox *m_successCheck;
  QGroupBox *m_editGroup;
  QLineEdit *m_fluxEdit, *m_bandWidthEdit;
  QComboBox *m_selectFileFormat;
  QLineEdit *m_groupNameEdit;
  CWOutputSelector *m_selector;
  int  m_instrument;
  bool m_successCheckEnable;
};

#endif
