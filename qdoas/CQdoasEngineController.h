/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CQDOASENGINECONTROLLER_H_GUARD
#define _CQDOASENGINECONTROLLER_H_GUARD

#include <QObject>
#include <QFileInfo>
#include <QStringList>

#include <memory>

#include "CEngineController.h"
#include "CEngineThread.h"

#include "CSession.h"
#include "CViewCrossSectionData.h"

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
  virtual void notifyReadyToNavigateRecords(const std::string &filename, int numberOfRecords) override;
  virtual void notifyCurrentRecord(int recordNumber) override;
  virtual void notifyEndOfRecords(void) override;
  virtual void notifyPlotData(std::vector<SPlotData> &plotDataList, std::vector<STitleTag> &titleList, std::vector<SPlotImage> &plotDataImage) override;
  virtual void notifyTableData(std::vector<SCell> &cellList) override;

  virtual void notifyErrorMessages(int highestErrorLevel, const std::vector<CEngineError> &errorMessages) override;

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
  void slotStartSession(std::shared_ptr<CSession> session);
  void slotStopSession();

  void slotViewCrossSections(std::shared_ptr<CViewCrossSectionData> awData);

 signals:
  void signalFileListChanged(const QStringList &fileList);
  void signalCurrentFileChanged(int fileIndex, int nRecords);
  void signalCurrentFileChanged(const QString &filename);
  void signalCurrentRecordChanged(int recordNumber, int firstMiddleLast);

  void signalPlotPages(const QList<std::shared_ptr<const CPlotPageData> > &pageList);
  void signalTablePages(const QList<std::shared_ptr<const CTablePageData> > &pageList);

  void signalErrorMessages(int highestErrorLevel, const QString &messages);

  void signalSessionRunning(bool running);

 private:
  CEngineThread *m_thread;
  QList<QFileInfo> m_fileList;

  const mediate_project_t *m_currentProject;
  int m_currentRecord, m_numberOfRecords, m_numberOfFiles;

  std::shared_ptr<CSession> m_session;
  CSessionIterator m_currentIt;

  bool m_atEndOfCurrentFile;
};

#endif
