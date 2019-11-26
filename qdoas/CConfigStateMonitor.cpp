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
void CConfigStateMonitor::updateNewSite(const QString &newSiteName) { slotInvalidate(); }
void CConfigStateMonitor::updateModifySite(const QString &siteName) { slotInvalidate(); }
void CConfigStateMonitor::updateDeleteSite(const QString &siteName) { slotInvalidate(); }
void CConfigStateMonitor::updateNewSymbol(const QString &newSymbolName) { slotInvalidate(); }
void CConfigStateMonitor::updateModifySymbol(const QString &symbolName) { slotInvalidate(); }
void CConfigStateMonitor::updateDeleteSymbol(const QString &symbolName) { slotInvalidate(); }
void CConfigStateMonitor::updateNewProject(const QString &newProjectName) { slotInvalidate(); }
void CConfigStateMonitor::updateModifyProject(const QString &projectName) { slotInvalidate(); }
void CConfigStateMonitor::updateDeleteProject(const QString &projectName) { slotInvalidate(); }

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

