/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFontMetrics>
#include <QRegularExpressionValidator>

#include "CWProjectTabDisplay.h"
#include "CWOutputSelector.h"

#include "constants.h"

CWProjectTabDisplay::CWProjectTabDisplay(const mediate_project_display_t *properties, QWidget *parent) :
  QFrame(parent)
{
  // construct the GUI and use properties (not NULL) to set the state of the edit widgets.
  // Each of the GUI components maintains its bit of 'properties' state (until 'apply'ed).

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(15);

  // plot (require data) group
  QGroupBox *plotGroup = new QGroupBox("Plot", this);
  QHBoxLayout *plotGroupLayout = new QHBoxLayout;

  m_reqSpectraCheck = new QCheckBox("Spectra");
  plotGroupLayout->addWidget(m_reqSpectraCheck);
  m_reqDataCheck = new QCheckBox("Information on record");
  plotGroupLayout->addWidget(m_reqDataCheck);
  m_reqCalibCheck = new QCheckBox("Calibration fits");
  plotGroupLayout->addWidget(m_reqCalibCheck);
  m_reqFitsCheck = new QCheckBox("Analysis fits");
  plotGroupLayout->addWidget(m_reqFitsCheck);
  plotGroup->setLayout(plotGroupLayout);

  mainLayout->addWidget(plotGroup);

  m_selector = new CWOutputSelector(&(properties->selection), this);
  mainLayout->addWidget(m_selector);

  // initialize
  m_reqSpectraCheck->setCheckState(properties->requireSpectra ? Qt::Checked : Qt::Unchecked);
  m_reqDataCheck->setCheckState(properties->requireData ? Qt::Checked : Qt::Unchecked);
  m_reqCalibCheck->setCheckState(properties->requireCalib ? Qt::Checked : Qt::Unchecked);
  m_reqFitsCheck->setCheckState(properties->requireFits ? Qt::Checked : Qt::Unchecked);

}

void CWProjectTabDisplay::apply(mediate_project_display_t *properties) const
{
  // extract state from the GUI and set properties

  properties->requireSpectra = (m_reqSpectraCheck->checkState() == Qt::Checked);
  properties->requireData = (m_reqDataCheck->checkState() == Qt::Checked);
  properties->requireCalib = (m_reqCalibCheck->checkState() == Qt::Checked);
  properties->requireFits = (m_reqFitsCheck->checkState() == Qt::Checked);

  m_selector->apply(&(properties->selection));
}

void CWProjectTabDisplay::slotInstrumentChanged(int instrument)
{
  m_selector->setInstrument(instrument,TAB_SELECTOR_DISPLAY);
}


