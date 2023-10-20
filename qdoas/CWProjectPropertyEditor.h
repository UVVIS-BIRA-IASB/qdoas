/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWPROJECTPROPERTYEDITOR_H_GUARD
#define _CWPROJECTPROPERTYEDITOR_H_GUARD

#include <QString>
#include <QComboBox>

#include "CWEditor.h"
#include "CWProjectTabDisplay.h"
#include "CWProjectTabSelection.h"
#include "CWProjectTabAnalysis.h"
#include "CWFilteringEditor.h"
#include "CWProjectTabCalibration.h"
#include "CWProjectTabUndersampling.h"
#include "CWProjectTabInstrumental.h"
#include "CWProjectTabSlit.h"
#include "CWProjectTabOutput.h"

#include "constants.h"

class CWProjectPropertyEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectPropertyEditor(const QString &projectName, QWidget *parent = 0);

  virtual bool actionOk();
  virtual void actionHelp();

 private slots:
   void slotInstrumentTypeChanged(int index);
   void slotGroundInstrumentChanged(int index);
   void slotSatelliteInstrumentChanged(int index);
   void slotAirborneInstrumentChanged(int index);
   void slotPageChanged(int index);

 signals:
   void signalInstrumentChanged(int);
   void signalInstrumentTypeChanged(int);

 private:
  QComboBox *m_instrTypeCombo, *m_groundFormatCombo, *m_satelliteFormatCombo, *m_airborneFormatCombo;
  QTabWidget *m_tabs;
  CWProjectTabDisplay *m_displayTab;
  CWProjectTabSelection *m_selectionTab;
  CWProjectTabAnalysis *m_analysisTab;
  CWFilteringEditor *m_filteringTab;
  CWProjectTabCalibration *m_calibrationTab;
  CWProjectTabUndersampling *m_undersamplingTab;
  CWProjectTabInstrumental *m_instrumentalTab;
  CWProjectTabSlit *m_slitTab;
  CWProjectTabOutput *m_outputTab;

  QString m_projectName;
  int m_selectedInstrument,m_selectedPage;
};

#endif
