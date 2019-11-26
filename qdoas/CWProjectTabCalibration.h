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


#ifndef _CWPROJECTTABCALIBRATION_H_GUARD
#define _CWPROJECTTABCALIBRATION_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>

#include "mediate_project.h"

#include "CWAnalysisWindowDoasTables.h"

class PolynomialTab;

class CWProjectTabCalibrationCustomWindowsDialog : public QDialog
 {
  public:
   CWProjectTabCalibrationCustomWindowsDialog(const double *lambdaMin,const double *lambdaMax,int nWindows, QWidget *parent = 0);
   void GetLambdaMin(double *lambdaMin);
   void GetLambdaMax(double *lambdaMax);

  private:
   QLineEdit *m_customLambdaMinEdit[MAX_CALIB_WINDOWS],*m_customLambdaMaxEdit[MAX_CALIB_WINDOWS];
   int m_nWindows;
 };

class CWProjectTabCalibration : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabCalibration(const mediate_project_calibration_t *properties, QWidget *parent = 0);

  void apply(mediate_project_calibration_t *properties) const;

 public slots:
  void slotLineShapeSelectionChanged(int index);
  void slotDivisionModeChanged(int index);
  void slotDefineCustomWindows(void);
  void slotBrowseSolarRefFile();
  void slotBrowseSlfFile();
  void slotOutputCalibration(bool enabled);

 private:

  void CheckCustomWindows(double *customLambdaMin,double *customLambdaMax,int nWindows) const;

  QLineEdit *m_refFileEdit,*m_slfFileEdit;
  QPushButton *m_refBrowseBtn;
  QComboBox *m_methodCombo;
  QComboBox *m_subwindowsCombo;
  QComboBox *m_lineShapeCombo;
  QPushButton *m_subWindowsCustomButton;
  QFrame *m_orderWidget,*m_fileWidget;
  QCheckBox *m_spectraCheck, *m_fitsCheck, *m_residualCheck, *m_shiftSfpCheck;
  QSpinBox *m_orderSpinBox, *m_shiftDegreeSpinBox, *m_sfpDegreeSpinBox;
  QLineEdit *m_lambdaMinEdit, *m_lambdaMaxEdit,*m_subWindowsSizeEdit;
  double m_customLambdaMin[MAX_CALIB_WINDOWS],m_customLambdaMax[MAX_CALIB_WINDOWS];
  QSpinBox *m_subWindowsSpinBox;

  QTabWidget *m_tabs;

  CWMoleculesDoasTable *m_moleculesTab;
  PolynomialTab *m_linearTab;
  CWSfpParametersDoasTable *m_sfpTab;
  CWShiftAndStretchDoasTable *m_shiftAndStretchTab;
  CWGapDoasTable *m_gapTab;
  CWOutputDoasTable *m_outputTab;

  QCheckBox *m_preshiftCheck;
  QLabel *m_preshiftMinLabel,*m_preshiftMaxLabel,*m_subWindowsSizeLabel;
  QLineEdit *m_preshiftMinEdit,*m_preshiftMaxEdit;
};

#endif
