/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONVENGINECONTROLLER_H_GUARD
#define _CCONVENGINECONTROLLER_H_GUARD

#include <QObject>

#include "CEngineController.h"

class CConvEngineController : public QObject, public CEngineController
{
Q_OBJECT
 public:
  CConvEngineController(QObject *parent=0);
  virtual ~CConvEngineController();

  // only need to worry about plot data and erro messages

  virtual void notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList,QList<SPlotImage> &plotDataImage);
  virtual void notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages);

 signals:
  void signalPlotPage(std::shared_ptr<const CPlotPageData> page);
  void signalErrorMessages(int highestErrorLevel, const QString &message);
};

#endif
