/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWANALYSISWINDOWPROPERTYEDITOR_H_GUARD
#define _CWANALYSISWINDOWPROPERTYEDITOR_H_GUARD

#include <QString>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include <QStackedLayout>

#include "CWEditor.h"
#include "CWorkSpace.h"
#include "CWAnalysisWindowDoasTables.h"

class PolynomialTab;

class CWAnalysisWindowPropertyEditor : public CWEditor, public CProjectObserver
{
Q_OBJECT
 public:
  CWAnalysisWindowPropertyEditor(const QString &projectName, const QString &analysisWindowName, QWidget *parent = 0);

  virtual bool actionOk();
  virtual void actionHelp();

  virtual void updateModifyProject(const std::string& projectName) override;

 private:
  void projectPropertiesChanged();

 public slots:
  void slotRefSelectionChanged(bool checked);
  void slotWavelengthCalibrationChanged(int index);
  void slotBrowseRefOne();
  void slotBrowseRefTwo();
  void slotBrowseResidual();
  void slotMaxdoasSelectionChanged(bool checked);
  void slotPageChanged(int index);

 private:
  QComboBox *m_calibrationCombo;
  QLineEdit *m_fitMinEdit, *m_fitMaxEdit,*m_resolEdit,*m_lambda0Edit;
  QCheckBox *m_spectrumCheck, *m_polyCheck, *m_fitsCheck;
  QCheckBox *m_residualCheck, *m_predefCheck, *m_ratioCheck;
  QCheckBox *m_saveResidualCheck;

  QFrame *m_refOneFrame;
  QFrame *m_refTwoEditFrame, *m_refTwoSzaFrame, *m_satelliteFrame, *m_maxdoasFrame, *m_maxdoasSzaFrame, *m_maxdoasScanFrame;
  QLineEdit *m_refOneEdit, *m_refTwoEdit, *m_residualEdit;
  QLineEdit *m_szaCenterEdit, *m_szaDeltaEdit;
  QLineEdit *m_maxdoasSzaCenterEdit, *m_maxdoasSzaDeltaEdit;

  QComboBox *m_scanCombo;
  // satellite only ...
  QLineEdit *m_refTwoLonMinEdit, *m_refTwoLonMaxEdit;
  QLineEdit *m_refTwoLatMinEdit, *m_refTwoLatMaxEdit;
  QLineEdit *m_cloudFractionMinEdit,*m_cloudFractionMaxEdit;

  QStackedLayout *m_refTwoStack;

  QTabWidget *m_tabs;
  // specialized DoasTables for each tab ...
  CWMoleculesDoasTable *m_moleculesTab;
  PolynomialTab *m_linearTab;
  CWNonLinearParametersDoasTable *m_nonLinearTab;
  CWShiftAndStretchDoasTable *m_shiftAndStretchTab;
  CWGapDoasTable *m_gapTab;
  CWOutputDoasTable *m_outputTab;


  QString m_projectName, m_analysisWindowName;
  bool m_autoSelection, m_scanSelection;
  int m_selectedPage;
};

#endif
