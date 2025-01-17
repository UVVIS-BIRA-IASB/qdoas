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

void CEngineResponseVisual::process(CEngineController *engineController)
{
  if (!hasFatalError()
      && !(m_cellList.empty() && m_plotPages.empty())) {
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(std::move(m_plotPages));
  }

  processErrors(engineController);
}

decltype(CEngineResponseVisual::m_plotPages)::iterator CEngineResponseVisual::getOrAddPage(int pageNumber, int pageType, string title="", string tag="") {
  // check if we already have that page, and insert it if not
  auto i_plotPage = m_plotPages.find(pageNumber);

  if (i_plotPage == m_plotPages.end()) {
    i_plotPage =  m_plotPages.insert(decltype(m_plotPages)::value_type(pageNumber, CPlotPageData(pageNumber, pageType))).first;
    i_plotPage->second.setTitle(title);
    i_plotPage->second.setTag(tag);
  }
  return i_plotPage;
}

void CEngineResponseVisual::addDataSet(int pageNumber, CPlotDataSet dataSet)
{
  auto i_plotPage = getOrAddPage(pageNumber, PLOTPAGE_DATASET);
  if (dataSet.count() != 0) {
    (i_plotPage->second).addPlotDataSet(std::move(dataSet));
  }
}

void CEngineResponseVisual::addImage(int pageNumber, CPlotImage plotImage)
{
  auto i_plotPage = getOrAddPage(pageNumber, PLOTPAGE_IMAGE, plotImage.getTitle(), plotImage.getTitle());
  i_plotPage->second.addPlotImage(std::move(plotImage));
}

void CEngineResponseVisual::addPageTitleAndTag(int pageNumber, string title, string tag)
{
  auto i_plotPage = getOrAddPage(pageNumber, PLOTPAGE_DATASET);
  i_plotPage->second.setTitle(title);
  i_plotPage->second.setTag(tag);
}

void CEngineResponseVisual::addCell(int pageNumber, int row, int col, const cell_data &data)
{
  m_cellList.push_back(SCell(pageNumber, row, col, data));
}

//------------------------------------------------------------

void CEngineResponseBeginAccessFile::process(CEngineController *engineController)
{
  if (!hasFatalError() &&
      !(m_cellList.empty() && m_plotPages.empty())) {
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(std::move(m_plotPages));
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
      engineController->notifyPlotData(std::move(m_plotPages));

      engineController->notifyCurrentRecord(m_recordNumber);
    }
  }

  processErrors(engineController);
}

void CEngineResponseSpecificRecord::setRecordNumber(int recordNumber)
{
  m_recordNumber = recordNumber;
}
