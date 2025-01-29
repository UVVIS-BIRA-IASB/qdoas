/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CENGINECONTROLLER_H_GUARD
#define _CENGINECONTROLLER_H_GUARD

#include <memory>

#include "CEngineError.h"
#include "CPlotDataSet.h"
#include "CPlotImage.h"
#include "CPlotPageData.h"
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
  virtual void notifyReadyToNavigateRecords([[maybe_unused]] const std::string &filename, [[maybe_unused]] int numberOfRecords) {};
  virtual void notifyCurrentRecord([[maybe_unused]] int recordNumber) {};
  virtual void notifyEndOfRecords(void) {};
  virtual void notifyPlotData([[maybe_unused]] std::map<int, CPlotPageData> pageData) {};
  virtual void notifyTableData([[maybe_unused]] std::vector<SCell> &cellList) {};

  virtual void notifyErrorMessages(int highestErrorLevel, const std::vector<CEngineError> &errorMessages) =0;
};

#endif
