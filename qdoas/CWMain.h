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


#ifndef _CWMAIN_H_GUARD
#define _CWMAIN_H_GUARD

#include <cstdio>

#include <QFrame>
#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QAction>

class CWProjectTree;
class CWSiteTree;
class CWUserSymbolTree;
class CWActiveContext;
class CQdoasEngineController;
class CWTableRegion;
class CWSplitter;
class CConfigStateMonitor;
class CHelpSystem;

class CWMain : public QFrame
{
Q_OBJECT
 public:

  CWMain(QWidget *parent = 0);

  virtual void closeEvent(QCloseEvent *e);
 
  bool isMessageFileLogging(void) const;

 protected:
  void writeConfiguration(FILE *fp);
  bool checkStateAndConsiderSaveFile(void);
  void setProjectFileName(const QString &fileName);

 public slots:
  void slotOpenFile();
  void slotNewFile();
  void slotSaveFile();
  void slotSaveAsFile();
  void slotCutButtonClicked();
  void slotCopyButtonClicked();
  void slotPasteButtonClicked();
  void slotDeleteButtonClicked();
  void slotStateMonitorChanged(bool valid);
  void slotConvolutionTool();
  void slotRingTool();
  void slotUndersamplingTool();
  void slotQdoasHelp();
  void slotAboutQdoas();
  void slotAboutQt();
  void slotSetMessageFileLogging(bool logToFile);
  void slotErrorMessages(int highestLevel, const QString &messages);
  void slotOpenRecent();
  
 private:
  QMenuBar *m_menuBar;
  QToolBar *m_toolBar;
  QTabWidget *m_projEnvTab;

  CWProjectTree *m_projTree;
  CWSiteTree *m_siteTree;
  CWUserSymbolTree *m_userSymbolTree;

  CWSplitter *m_subSplitter;

  CWTableRegion *m_tableRegion;

  CWActiveContext *m_activeContext;

  QStatusBar *m_statusBar;

  CQdoasEngineController *m_controller;
  
  CConfigStateMonitor *m_stateMonitor;

  QString m_projectFile;
  
  // actions ...
  QAction *m_saveAction;
  QAction *m_saveAsAction;

  QList <QAction *>  m_recentFileActs;
  QMenu *m_openRecentMenu;

  bool m_logToFile;

  void openFile(const QString& fileName);
  void updateRecentFiles(const QString &fileName);
  void updateRecentFileMenu();
};

inline bool CWMain::isMessageFileLogging(void) const { return m_logToFile; }

#endif
