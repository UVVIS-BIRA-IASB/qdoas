
#include <QTextStream>

#include "CConvEngineController.h"

CConvEngineController::CConvEngineController(QObject *parent) :
  QObject(parent),
  CEngineController()
{
}

CConvEngineController::~CConvEngineController()
{
}

void CConvEngineController::notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList,QList<SPlotImage> &plotImageList)
{
  // the controller takes the data in plotDataList and titleList
  // and organises the data-sets into a single pages. The page is
  // then (safely) dispatched.            

  if (!plotDataList.isEmpty()) {

    // create a page ... give it page number 0
    CPlotPageData *plotPage = new CPlotPageData(0,PLOTPAGE_DATASET);
    
    while (!plotDataList.isEmpty()) {                      
      plotPage->addPlotDataSet(plotDataList.front().data);
      plotDataList.pop_front();
    }

    // built a page and emptied the plotDataList list (argument).
    
    if (!titleList.isEmpty()) {            
      // take the first title ...
      plotPage->setTitle(titleList.front().title);
      plotPage->setTag(titleList.front().tag);
      
      // discard the rest ...
      while (!titleList.isEmpty())
    titleList.pop_front();
    }

    // put the page in a smart pointer for safe dispatch.

    RefCountConstPtr<CPlotPageData> page(plotPage);

    emit signalPlotPage(page);
  }
}

void CConvEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  QString msg;
  QTextStream stream(&msg);

  QList<CEngineError>::const_iterator it = errorMessages.begin();
  while (it != errorMessages.end()) {
    // one message per line
    switch (it->errorLevel()) {
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

    stream << it->tag() << ") " << it->message() << ".\n";

    ++it;
  }

  emit signalErrorMessages(highestErrorLevel, msg);  
}

