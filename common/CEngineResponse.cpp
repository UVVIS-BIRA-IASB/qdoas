/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <iostream>

#include "CEngineResponse.h"
#include "CEngineController.h"

#include "debugutil.h"

using std::string;

//------------------------------------------------------------

void CEngineResponse::addErrorMessage(const string &tag, const string &msg, int errorLevel)
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
  for (auto plot_data : m_plotDataList) {
    delete plot_data.data;
  }

}

void CEngineResponseVisual::process(CEngineController *engineController)
{
  if (!hasFatalError()
      && !(m_cellList.empty() && m_plotDataList.empty() && m_plotImageList.empty() ) ) {
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

void CEngineResponseVisual::addPageTitleAndTag(int pageNumber, const string &title, const string &tag)
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
      !(m_cellList.empty() && m_plotDataList.empty() && m_plotImageList.empty()) ) {
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
