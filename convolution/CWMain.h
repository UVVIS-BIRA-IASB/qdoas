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

#include "CWConvTabGeneral.h"
#include "CWConvTabSlit.h"
#include "CWFilteringEditor.h"
#include "CWPlotArea.h"

class CConvEngineController;
class CHelpSystem;

class CWMain : public QFrame
{
Q_OBJECT
 public:

  CWMain(QWidget *parent = 0);
  virtual ~CWMain();

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
  void slotRunConvolution();

  void slotPlotPage(std::shared_ptr<const CPlotPageData> page);

 private:
  QMenuBar *m_menuBar;
  QTabWidget *m_tab;

  CWConvTabGeneral *m_generalTab;
  CWConvTabSlit *m_slitTab;
  CWFilteringEditor *m_filteringTab;

  CWPlotArea *m_plotArea;

  QString m_configFile;

  CConvEngineController *m_controller;

  // actions ...
  QAction *m_saveAction;
  QAction *m_saveAsAction;

  mediate_convolution_t m_properties, m_guiProperties;

};

#endif
