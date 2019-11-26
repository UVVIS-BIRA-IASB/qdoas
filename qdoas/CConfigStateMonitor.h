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
