/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
 
  virtual void notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList,QList<SPlotImage> &plotDataImage);
  virtual void notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages);

 signals:
  void signalPlotPage(const RefCountConstPtr<CPlotPageData> &page);
  void signalErrorMessages(int highestErrorLevel, const QString &message);
};

#endif
