/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CConfigStateMonitor.h"

CConfigStateMonitor::CConfigStateMonitor(QObject *parent, bool initialState) :
  QObject(parent),
  CSitesObserver(),
  CSymbolObserver(),
  CProjectObserver(),
  m_valid(initialState)
{
}

CConfigStateMonitor::~CConfigStateMonitor()
{
}

// Monitor changes to the workspace. Any change invalidates the state.
void CConfigStateMonitor::updateNewSite(const std::string &newSiteName) { slotInvalidate(); }
void CConfigStateMonitor::updateModifySite(const std::string &siteName) { slotInvalidate(); }
void CConfigStateMonitor::updateDeleteSite(const std::string &siteName) { slotInvalidate(); }
void CConfigStateMonitor::updateNewSymbol(const std::string &newSymbolName) { slotInvalidate(); }
void CConfigStateMonitor::updateModifySymbol(const std::string &symbolName) { slotInvalidate(); }
void CConfigStateMonitor::updateDeleteSymbol(const std::string &symbolName) { slotInvalidate(); }
void CConfigStateMonitor::updateNewProject(const std::string &newProjectName) { slotInvalidate(); }
void CConfigStateMonitor::updateModifyProject(const std::string &projectName) { slotInvalidate(); }
void CConfigStateMonitor::updateDeleteProject(const std::string &projectName) { slotInvalidate(); }

// validate the state.
void CConfigStateMonitor::slotValidate()
{
  if (!m_valid) {
    m_valid = true;
    emit signalStateChanged(m_valid);
  }
}

// explicitly invalidate the state.
void CConfigStateMonitor::slotInvalidate()
{
  if (m_valid) {
    m_valid = false;
    emit signalStateChanged(m_valid);
  }
}

