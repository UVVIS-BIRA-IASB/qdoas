/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QDialog>

#include "CWPlotPropertiesConfig.h"

#include "debugutil.h"

#ifndef _CWPLOTPROPERTIESDIALOG_H_GUARD
#define _CWPLOTPROPERTIESDIALOG_H_GUARD

class CWPlotPropertiesDialog : public QDialog
{
 public:
  CWPlotPropertiesDialog(CPlotProperties &prop, QWidget *parent = 0);

  virtual void accept();

 private:
  CPlotProperties &m_properties;
  CWPlotPropertiesConfig *m_config;
};


#endif

