/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWMAIN_H_GUARD
#define _CWMAIN_H_GUARD

#include <cstdio>

#include <QFrame>
#include <QMenuBar>
#include <QTabWidget>
#include <QAction>

#include "CWUsampTabGeneral.h"
#include "CWPlotArea.h"

class CUsampEngineController;
class CHelpSystem;

class CWMain : public QFrame
{
Q_OBJECT
 public:

  CWMain(QWidget *parent = 0);

  virtual void closeEvent(QCloseEvent *e);

 protected:
  void writeConfiguration(FILE *fp);
  bool checkStateAndConsiderSaveFile(void);
  void setConfigFileName(const QString &fileName);

  void fetchGuiProperties(void);
  bool compareProperties(void);

 public slots:
  void slotOpenFile();
  void slotNewFile();
  void slotSaveFile();
  void slotSaveAsFile();
  void slotEditPlotProperties();
  void slotPrintPlots();
  void slotExportPlots();
  void slotQdoasHelp();
  void slotAboutQdoas();
  void slotAboutQt();
  void slotErrorMessages(int highestLevel, const QString &messages);
  void slotRunUsamp();

  void slotPlotPage(const RefCountConstPtr<CPlotPageData> &page);

 private:
  QMenuBar *m_menuBar;
  QTabWidget *m_tab;

  CWUsampTabGeneral *m_generalTab;

  CWPlotArea *m_plotArea;

  QString m_configFile;

  CUsampEngineController *m_controller;

  // actions ...
  QAction *m_saveAction;
  QAction *m_saveAsAction;

  mediate_usamp_t m_properties, m_guiProperties;
};

#endif
