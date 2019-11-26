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


#ifndef _CBATCHENGINECONTROLLER_H_GUARD
#define _CBATCHENGINECONTROLLER_H_GUARD

#include <QFileInfo>
#include <QStringList>

#include "CEngineController.h"

#include "RefCountPtr.h"

#include "mediate_project.h"


class CBatchEngineController : public CEngineController
{
 public:
  CBatchEngineController();

  // query interface
  bool active(void) const;

  // notify interface is for use by response classes
  virtual void notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords);
  virtual void notifyEndOfRecords(void);

  virtual void notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages);

 private:
  bool    m_active;
};

inline bool CBatchEngineController::active(void) const { return m_active; }

#endif
