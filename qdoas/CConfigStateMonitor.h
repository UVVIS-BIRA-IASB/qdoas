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

  virtual void updateNewSite(const std::string &newSiteName) override;
  virtual void updateModifySite(const std::string &siteName) override;
  virtual void updateDeleteSite(const std::string &siteName) override;
  virtual void updateNewSymbol(const std::string &newSymbolName) override;
  virtual void updateModifySymbol(const std::string &symbolName) override;
  virtual void updateDeleteSymbol(const std::string &symbolName) override;
  virtual void updateNewProject(const std::string &newProjectName) override;
  virtual void updateModifyProject(const std::string &projectName) override;
  virtual void updateDeleteProject(const std::string &projectName) override;

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
