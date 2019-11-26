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


#ifndef _CENGINERESPONSE_H_GUARD
#define _CENGINERESPONSE_H_GUARD

#include <QList>
#include <QString>

#include "CPlotDataSet.h"
#include "CPlotImage.h"
#include "CTablePageData.h"
#include "CEngineError.h"
#include "mediate_types.h"

class CEngineController;

//------------------------------------------------------------

class CEngineResponse
{
 public:
  CEngineResponse() : m_highestErrorLevel(0) {};
  virtual ~CEngineResponse() {};

  void addErrorMessage(const QString &tag, const QString &msg, int errorLevel);

  virtual void process(CEngineController *engineController) = 0;

  bool processErrors(CEngineController *engineController);

  bool hasErrors(void) const;
  bool hasFatalError(void) const;

 protected:
  int m_highestErrorLevel;
  QList<CEngineError> m_errorMessages;
};

inline bool CEngineResponse::hasErrors(void) const { return !m_errorMessages.isEmpty(); }
inline bool CEngineResponse::hasFatalError(void) const { return (m_highestErrorLevel == FatalEngineError); }

//------------------------------------------------------------

class CEngineResponseMessage : public CEngineResponse
{
 public:
  virtual void process(CEngineController *engineController);
};

//------------------------------------------------------------

class CEngineResponseVisual : public CEngineResponse
{
 public:
  virtual ~CEngineResponseVisual();

  virtual void process(CEngineController *engineController);

  void addDataSet(int pageNumber, const CPlotDataSet *dataSet);
  void addImage(int pageNumber,const CPlotImage *plotImage);
  void addPageTitleAndTag(int pageNumber, const QString &title, const QString &tag);
  void addCell(int pageNumber, int row, int col, const QVariant &data);

 protected:
  QList<SPlotData> m_plotDataList;
  QList<STitleTag> m_titleList;
  QList<SCell> m_cellList;
  QList<SPlotImage> m_plotImageList;
};

//------------------------------------------------------------

class CEngineResponseBeginAccessFile : public CEngineResponseVisual
{
 public:
  CEngineResponseBeginAccessFile(const QString &fileName) : m_fileName(fileName),
                                                            m_numberOfRecords(-1) {};

  virtual void process(CEngineController *engineController);

  void setNumberOfRecords(int numberOfRecords);

 private:
  QString m_fileName;
  int m_numberOfRecords;
};

//------------------------------------------------------------

class CEngineResponseSpecificRecord : public CEngineResponseVisual
{
 public:
  CEngineResponseSpecificRecord() : m_recordNumber(-1) {};

  virtual void process(CEngineController *engineController);

  void setRecordNumber(int recordNumber);

 private:
  int m_recordNumber;
};

#endif

