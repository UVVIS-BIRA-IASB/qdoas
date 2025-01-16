#include <sstream>

#include "CConvEngineController.h"

using std::vector;

CConvEngineController::CConvEngineController(QObject *parent) :
  QObject(parent),
  CEngineController()
{
}

CConvEngineController::~CConvEngineController()
{
}

void CConvEngineController::notifyPlotData(vector<SPlotData> &plotDataList, vector<STitleTag> &titleList, vector<SPlotImage> &plotImageList)
{
  // the controller takes the data in plotDataList and titleList
  // and organises the data-sets into a single pages. The page is
  // then (safely) dispatched.

  if (!plotDataList.empty()) {

    // create a page ... give it page number 0
    CPlotPageData *plotPage = new CPlotPageData(0,PLOTPAGE_DATASET);

    for (auto& plot_data : plotDataList) {
      plotPage->addPlotDataSet(plot_data.data);
    }
    plotDataList.clear();

    if (!titleList.empty()) {
      // take the first title ...
      plotPage->setTitle(titleList.front().title);
      plotPage->setTag(titleList.front().tag);
    }
    titleList.clear();

    // put the page in a smart pointer for safe dispatch.
    std::shared_ptr<const CPlotPageData> page(plotPage);
    emit signalPlotPage(page);
  }
}

void CConvEngineController::notifyErrorMessages(int highestErrorLevel, const vector<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  std::ostringstream stream;

  for (const auto& msg : errorMessages) {
    // one message per line
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

