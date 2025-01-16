/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CUSAMPENGINECONTROLLER_H_GUARD
#define _CUSAMPENGINECONTROLLER_H_GUARD

#include <QObject>

#include "CEngineController.h"

class CUsampEngineController : public QObject, public CEngineController
{
Q_OBJECT
 public:
  CUsampEngineController(QObject *parent);
  virtual ~CUsampEngineController();

  // only need to worry about plot data and erro messages

  virtual void notifyPlotData(std::vector<SPlotData> &plotDataList, std::vector<STitleTag> &titleList,std::vector<SPlotImage> &plotDataImage) override;
  virtual void notifyErrorMessages(int highestErrorLevel, const std::vector<CEngineError> &errorMessages) override;

 signals:
  void signalPlotPage(std::shared_ptr<const CPlotPageData> page);
  void signalErrorMessages(int highestErrorLevel, const QString &message);
};

#endif
