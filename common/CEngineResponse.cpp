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

#include <QFile>
#include <QTextStream>

#include <iostream>

#include "CEngineResponse.h"
#include "CEngineController.h"

#include "debugutil.h"

//------------------------------------------------------------

void CEngineResponse::addErrorMessage(const QString &tag, const QString &msg, int errorLevel)
{
  m_errorMessages.push_back(CEngineError(tag, msg, errorLevel));

  if (errorLevel > m_highestErrorLevel)
    m_highestErrorLevel = errorLevel;
}

bool CEngineResponse::processErrors(CEngineController *engineController)
{
  if (hasErrors()) {
    // send the set of messages to the controller
    engineController->notifyErrorMessages(m_highestErrorLevel, m_errorMessages);

    return hasFatalError();
  }

  return false;
}

//------------------------------------------------------------

void CEngineResponseMessage::process(CEngineController *engineController)
{
  processErrors(engineController);
}

//------------------------------------------------------------

CEngineResponseVisual::~CEngineResponseVisual()
{
  while (!m_plotDataList.isEmpty()) {
    delete m_plotDataList.front().data;
    m_plotDataList.pop_front();
  }
  
  while (!m_plotImageList.isEmpty()) {
    m_plotImageList.pop_front();
  }  
}

void CEngineResponseVisual::process(CEngineController *engineController)
{
  if (!hasFatalError() 
      && !(m_cellList.isEmpty() && m_plotDataList.isEmpty() && m_plotImageList.isEmpty() ) ) {
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(m_plotDataList, m_titleList, m_plotImageList);
  }

  processErrors(engineController);
}

void CEngineResponseVisual::addDataSet(int pageNumber, const CPlotDataSet *dataSet)
{
  m_plotDataList.push_back(SPlotData(pageNumber, dataSet));
}

void CEngineResponseVisual::addImage(int pageNumber, const CPlotImage *plotImage)
{ 
  m_plotImageList.push_back(SPlotImage(pageNumber, plotImage));
}

void CEngineResponseVisual::addPageTitleAndTag(int pageNumber, const QString &title, const QString &tag)
{
  m_titleList.push_back(STitleTag(pageNumber, title, tag));
}

void CEngineResponseVisual::addCell(int pageNumber, int row, int col, const QVariant &data)
{
  m_cellList.push_back(SCell(pageNumber, row, col, data));
}

//------------------------------------------------------------

void CEngineResponseBeginAccessFile::process(CEngineController *engineController)
{
  if (!hasFatalError() &&
      !(m_cellList.isEmpty() && m_plotDataList.isEmpty() && m_plotImageList.isEmpty()) ) {
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(m_plotDataList, m_titleList, m_plotImageList);
  }

  if (processErrors(engineController)) {
    return;
  }

  if (m_numberOfRecords > 0) {
    // calibration data ... TODO ...

    engineController->notifyReadyToNavigateRecords(m_fileName, m_numberOfRecords);
    // wait for the request to process a record ...
  }

 else                                                   // Added by Caroline on 7 December  2008 (otherwise, the program was waiting indefinitely when
  engineController->notifyEndOfRecords();               // the file was empty

}

void CEngineResponseBeginAccessFile::setNumberOfRecords(int numberOfRecords)
{
  m_numberOfRecords = numberOfRecords;
}

//------------------------------------------------------------

void CEngineResponseSpecificRecord::process(CEngineController *engineController)
{
  if (!hasFatalError()) {
    if (m_recordNumber <= 0) {    // 20090113 Caroline : If the record number is -1, do not stop the automatic process
      // EOF
      engineController->notifyEndOfRecords();
    }
    else if (m_recordNumber > 0) {
      // display ... table data MUST be before plot data
      engineController->notifyTableData(m_cellList);
      engineController->notifyPlotData(m_plotDataList, m_titleList, m_plotImageList);
      
      engineController->notifyCurrentRecord(m_recordNumber);
    }
  }

  processErrors(engineController);
}

void CEngineResponseSpecificRecord::setRecordNumber(int recordNumber)
{
  m_recordNumber = recordNumber;
}
