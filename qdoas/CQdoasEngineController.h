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

#ifndef _CQDOASENGINECONTROLLER_H_GUARD
#define _CQDOASENGINECONTROLLER_H_GUARD

#include <QObject>
#include <QFileInfo>
#include <QStringList>

#include "CEngineController.h"
#include "CEngineThread.h"

#include "CSession.h"
#include "CViewCrossSectionData.h"
#include "RefCountPtr.h"

#include "mediate_project.h"


class CQdoasEngineController : public QObject, public CEngineController
{
Q_OBJECT
 public:
// why are these needed??? - IAP 200812
  int     m_engineCurrentRecord;
  QString m_engineCurrentFile;

  CQdoasEngineController(QObject *parent = 0);

  // query interface
  bool isSessionRunning(void) const;

  // notify interface is for use by response classes
  virtual void notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords);
  virtual void notifyCurrentRecord(int recordNumber);
  virtual void notifyEndOfRecords(void);
  virtual void notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList,QList<SPlotImage> &plotDataImage);
  virtual void notifyTableData(QList<SCell> &cellList);

  virtual void notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages);

 protected:
  virtual bool event(QEvent *e);

 public slots:
  // toolbar file navigation interface
  void slotNextFile();
  void slotGotoFile(int number);
  // toolbar record navigation interface
  void slotFirstRecord();
  void slotPreviousRecord();
  void slotNextRecord();
  void slotLastRecord();
  void slotGotoRecord(int recNumber);
  // toolbar auto-stepping navigation interface
  void slotStep();

  // session control
  void slotStartSession(const RefCountPtr<CSession> &session);
  void slotStopSession();

  void slotViewCrossSections(const RefCountPtr<CViewCrossSectionData> &awData);

 signals:
  void signalFileListChanged(const QStringList &fileList);
  void signalCurrentFileChanged(int fileIndex, int nRecords);
  void signalCurrentFileChanged(const QString &filename);
  void signalCurrentRecordChanged(int recordNumber, int firstMiddleLast);

  void signalPlotPages(const QList< RefCountConstPtr<CPlotPageData> > &pageList);  
  void signalTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList);

  void signalErrorMessages(int highestErrorLevel, const QString &messages);

  void signalSessionRunning(bool running);

 private:
  CEngineThread *m_thread;
  QList<QFileInfo> m_fileList;

  const mediate_project_t *m_currentProject;
  int m_currentRecord, m_numberOfRecords, m_numberOfFiles;

  RefCountPtr<CSession> m_session;
  CSessionIterator m_currentIt;

  bool m_atEndOfCurrentFile;
};

#endif
