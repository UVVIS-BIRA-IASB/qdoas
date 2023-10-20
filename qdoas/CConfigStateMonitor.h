/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONFIGSTATEMONITOR_H_GUARD
#define _CCONFIGSTATEMONITOR_H_GUARD

#include <QObject>

#include "CWorkSpace.h"

#include "debugutil.h"

// this call tracks changes to the configuration.

class CConfigStateMonitor : public QObject, public CSitesObserver, public CSymbolObserver, public CProjectObserver
{
Q_OBJECT
 public:
  CConfigStateMonitor(QObject *parent = 0, bool initialState = true);
  virtual ~CConfigStateMonitor();

  bool isValid(void) const;

  virtual void updateNewSite(const QString &newSiteName);
  virtual void updateModifySite(const QString &siteName);
  virtual void updateDeleteSite(const QString &siteName);
  virtual void updateNewSymbol(const QString &newSymbolName);
  virtual void updateModifySymbol(const QString &symbolName);
  virtual void updateDeleteSymbol(const QString &symbolName);
  virtual void updateNewProject(const QString &newProjectName);
  virtual void updateModifyProject(const QString &projectName);
  virtual void updateDeleteProject(const QString &projectName);

 public slots:
  void slotValidate();
  void slotInvalidate();

 signals:
  void signalStateChanged(bool valid);

 private:
  bool m_valid;
};

inline bool CConfigStateMonitor::isValid(void) const { return m_valid; }

#endif
