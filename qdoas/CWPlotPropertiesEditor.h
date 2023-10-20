/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWPLOTPROPERTIESEDITOR_H_GUARD
#define _CWPLOTPROPERTIESEDITOR_H_GUARD

#include <QFrame>
#include <QSpinBox>

#include "CWEditor.h"
#include "CWPlotPropertiesConfig.h"

class CWPlotRegion;

class CWPlotPropertiesEditor : public CWEditor
{
 public:
  CWPlotPropertiesEditor(CWPlotRegion *plotRegion, QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

 private:
  CWPlotRegion *m_plotRegion;
  CWPlotPropertiesConfig *m_config;
};

#endif
