/* Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <iostream>
#include "CBatchEngineController.h"

#include "mediate_types.h"

#include "debugutil.h"

extern int verboseMode;


CBatchEngineController::CBatchEngineController() :
  CEngineController(),
  m_active(false)
{
}

void CBatchEngineController::notifyReadyToNavigateRecords(const std::string &filename, int numberOfRecords)
{
  m_active = (numberOfRecords > 0);
}

void CBatchEngineController::notifyEndOfRecords(void)
{
  m_active = false;
}

void CBatchEngineController::notifyErrorMessages(int highestErrorLevel, const std::vector<CEngineError> &errorMessages)
{
  auto it = errorMessages.begin();
  while (it != errorMessages.end()) {
    if (it->errorLevel() == InformationEngineError && !verboseMode) {
      continue;
    }
    switch (it->errorLevel()) {
    case InformationEngineError:
      std::cout << "INFO:  ";
      break;
    case WarningEngineError:
      std::cout << "WARN:  ";
      break;
    case FatalEngineError:
      std::cout << "ERROR: ";
      break;
    default:
      std::cout << "???:   ";
    }

    std::cout << it->message() << std::endl;
    ++it;
  }

  // abort if an error occurred
  if (highestErrorLevel == FatalEngineError)
    m_active = false;
}
