#include <sstream>

#include "CRingEngineController.h"

using std::vector;

CRingEngineController::CRingEngineController(QObject *parent) :
  QObject(parent),
  CEngineController()
{
}

CRingEngineController::~CRingEngineController()
{
}

void CRingEngineController::notifyPlotData(std::map<int, CPlotPageData> pageMap)
{
  for (auto& [num, page] : pageMap) {
    // put the page in a smart pointer for safe dispatch.
    emit signalPlotPage(std::make_shared<const CPlotPageData>(std::move(page)));
  }
}

void CRingEngineController::notifyErrorMessages(int highestErrorLevel, const vector<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  std::ostringstream stream;

  for (const auto& msg : errorMessages) {
    switch (msg.errorLevel()) {
    case InformationEngineError:
      stream << "INFO    (";
      break;
    case WarningEngineError:
      stream << "WARNING (";
      break;
    case FatalEngineError:
      stream << "FATAL   (";
      break;
    }
    stream << msg.tag() << ") " << msg.message() << ".\n";
  }
  emit signalErrorMessages(highestErrorLevel, QString::fromStdString(stream.str()));
}

