/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
