/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CENGINECONTROLLER_H_GUARD
#define _CENGINECONTROLLER_H_GUARD

#include <QList>
#include <memory>

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
