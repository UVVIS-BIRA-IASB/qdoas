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

#ifndef _CENGINECONTROLLER_H_GUARD
#define _CENGINECONTROLLER_H_GUARD

#include <QList>

#include "CEngineError.h"

#include "CPlotDataSet.h"  
#include "CPlotImage.h"
#include "CPlotPageData.h" 
#include "CImagePageData.h"
#include "CTablePageData.h"

// CEngineController is a mediator. It is the interface for the GUI
// to control the activities of the engine, and to access any data
// provided by the engine.


class CEngineController
{
 public:
  CEngineController() {};
  virtual ~CEngineController() {};

  // notify interface is for use by response classes
  virtual void notifyNumberOfFiles(int nFiles) {};
  virtual void notifyCurrentFile(int fileNumber) {};
  virtual void notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords) {};
  virtual void notifyCurrentRecord(int recordNumber) {};
  virtual void notifyEndOfRecords(void) {};
  virtual void notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList,QList<SPlotImage> &plotImageList) {};
  virtual void notifyTableData(QList<SCell> &cellList) {};

  virtual void notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages) {};
};

#endif
