/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
