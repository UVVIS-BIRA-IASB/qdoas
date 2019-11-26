/* Qdoas is a cross-platform application for spectral analysis with the DOAS
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

#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include "CQdoasEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"
#include "constants.h"

#include "debugutil.h"

CQdoasEngineController::CQdoasEngineController(QObject *parent) :
  QObject(parent),
  CEngineController(),
  m_currentProject(NULL),
  m_currentRecord(-1),
  m_numberOfRecords(0),
  m_numberOfFiles(0),
  m_atEndOfCurrentFile(false)
{
  m_engineCurrentRecord=m_currentRecord;
  m_engineCurrentFile="";

  // create the engine thread
  m_thread = new CEngineThread(this);

  m_thread->setRunState(true);
}

void CQdoasEngineController::notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords)
{
  // sanity check - currentIt and filename must be consistent
  assert(m_currentIt.file().filePath() == filename);

  m_numberOfRecords = numberOfRecords;
  m_currentRecord = 0;

  m_atEndOfCurrentFile = false;

  // signals for navigation control
  emit signalCurrentFileChanged(m_currentIt.index(), m_numberOfRecords);
  // files
  emit signalCurrentFileChanged(filename);

  // session is up and running
  emit signalSessionRunning(true);

  slotNextRecord(); // goto the first spectrum ...
}

void CQdoasEngineController::notifyCurrentRecord(int recordNumber)
{
  int firstMiddleLast = 0;

  m_currentRecord = recordNumber;

  m_atEndOfCurrentFile = false;

  if (m_currentRecord <= 1)
    firstMiddleLast = -1; // lower limit
  else if (m_currentRecord >= m_numberOfRecords)
    firstMiddleLast = 1;

  emit signalCurrentRecordChanged(m_currentRecord, firstMiddleLast);
}

void CQdoasEngineController::notifyEndOfRecords(void)
{
  // This means no more matching records were found in the current file. The
  // current record will not have changed, but we signal it again (so play
  // can send another request).

  m_atEndOfCurrentFile = true;

  emit signalCurrentRecordChanged(m_currentRecord, 1);
}

void CQdoasEngineController::notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList, QList<SPlotImage> &plotImageList)
{
  // the controller takes the data in plotDataList and titleList
  // and organises the data-sets into a set of pages. Each page is
  // then (safely) dispatched.

  std::map<int,CPlotPageData*> pageMap;
  std::map<int,CPlotPageData*>::iterator mIt;
  int pageNo;

  while (!plotDataList.isEmpty()) {
    // existing page?
    pageNo = plotDataList.front().page;

    mIt = pageMap.find(pageNo);

    if (mIt == pageMap.end()) {
      // need a new page
      CPlotPageData *newPage = new CPlotPageData(pageNo,PLOTPAGE_DATASET);

      mIt = pageMap.insert(std::map<int,CPlotPageData*>::value_type(pageNo, newPage)).first;
    }
    if (plotDataList.front().data != NULL) {
      (mIt->second)->addPlotDataSet(plotDataList.front().data);
    }
    plotDataList.pop_front();
  }

  // built a map of pages and emptied the plotDataList list (argument).

  while (!plotImageList.isEmpty())
   {
    // existing page?
    pageNo = plotImageList.front().page;

    mIt = pageMap.find(pageNo);
    if (mIt == pageMap.end())
     {
      // need a new page
      CPlotPageData *newPage = new CPlotPageData(pageNo,PLOTPAGE_IMAGE);
      CPlotImage myImage=*plotImageList.front().plotImage;
      QString	 str=myImage.GetFile();
      QString titleStr=myImage.GetTitle();

      if (plotImageList.front().plotImage != NULL)
       {
        newPage->addPlotImage(plotImageList.front().plotImage);
        newPage->setTitle(titleStr);
        newPage->setTag(titleStr);
       }
          pageMap.insert(std::map<int,CPlotPageData*>::value_type(pageNo, newPage));
     }
    else
     {
     // exists
     if (plotImageList.front().plotImage != NULL)
      (mIt->second)->addPlotImage(plotImageList.front().plotImage);
     }
    plotImageList.pop_front();
  }

  // set page titles and tags if specified ... emptying the list as we go
  while (!titleList.isEmpty()) {
    mIt = pageMap.find(titleList.front().page);
    if (mIt != pageMap.end()) {
      (mIt->second)->setTitle(titleList.front().title);
      (mIt->second)->setTag(titleList.front().tag);
    }
    titleList.pop_front();

  }

  // shift the items in the pageMap to a QList for cheap and safe dispatch.

  QList< RefCountConstPtr<CPlotPageData> > pageList;

  mIt = pageMap.begin();
  while (mIt != pageMap.end()) {
    pageList.push_back(RefCountConstPtr<CPlotPageData>(mIt->second));
    ++mIt;
  }
  pageMap.clear();

  // send the pages to any connected slots
  emit signalPlotPages(pageList);
}

void CQdoasEngineController::notifyTableData(QList<SCell> &cellList)
{
  // the controller takes the cells and organises the data into
  // pages. Each page is then (safely) dispatched.

  std::map<int,CTablePageData*> pageMap;
  std::map<int,CTablePageData*>::iterator mIt;
  int pageNo;

  while (!cellList.isEmpty()) {
    // existing page?
    pageNo = cellList.front().page;
    mIt = pageMap.find(pageNo);
    if (mIt == pageMap.end()) {
      // need a new page
      CTablePageData *newPage = new CTablePageData(pageNo);
      newPage->addCell(cellList.front().row, cellList.front().col, cellList.front().data);
      pageMap.insert(std::map<int,CTablePageData*>::value_type(pageNo, newPage));
    }
    else {
      // exists
      (mIt->second)->addCell(cellList.front().row, cellList.front().col, cellList.front().data);
    }
    cellList.pop_front();
  }

  // built a map of pages and emptied cellList (argument).
  // shift them to a QList for cheap and safe dispatch.

  QList< RefCountConstPtr<CTablePageData> > pageList;

  mIt = pageMap.begin();
  while (mIt != pageMap.end()) {
    pageList.push_back(RefCountConstPtr<CTablePageData>(mIt->second));
    ++mIt;
  }
  pageMap.clear();

  // send the pages to any connected slots
  emit signalTablePages(pageList);
}

void CQdoasEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  QString msg;
  QTextStream stream(&msg);

  if (highestErrorLevel == FatalEngineError) {
    // abort the session
    emit signalSessionRunning(false);
  }

  for (QList<CEngineError>::const_iterator it = errorMessages.begin();
       it != errorMessages.end(); ++it) {
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
  }
  emit signalErrorMessages(highestErrorLevel, msg);
}

bool CQdoasEngineController::event(QEvent *e)
{
  if (e->type() == cEngineResponseType) {

    // one or more responses are ready for processing ...
    QList<CEngineResponse*> responses;

    m_thread->takeResponses(responses);

    // work through the responses ...
    while (!responses.isEmpty()) {
      CEngineResponse *tmp = responses.takeFirst();

      tmp->process(this);

      delete tmp;
    }

    e->accept();
    return true;
  }

  // let the base class handle it
  return QObject::event(e);
}

void CQdoasEngineController::slotNextFile()
{
  CEngineRequestCompound *req = new CEngineRequestCompound;

  if (!m_currentIt.atEnd()) {
    ++m_currentIt;
    if (!m_currentIt.atEnd()) {
      // check for a change in project
      if (m_currentProject != m_currentIt.project()) {

	int opMode = THREAD_TYPE_NONE;
	switch (m_session->mode()) {
	case CSession::Browse: opMode = THREAD_TYPE_SPECTRA; break;
	case CSession::Export: opMode = THREAD_TYPE_EXPORT; break;
	case CSession::Calibrate: opMode = THREAD_TYPE_KURUCZ; break;
	case CSession::Analyse: opMode = THREAD_TYPE_ANALYSIS; break;
	}
	m_currentProject = m_currentIt.project();
	req->addRequest(new CEngineRequestSetProject(m_currentProject, opMode));
	// might also need to replace the analysis windows
	if (m_session->mode() == CSession::Calibrate || m_session->mode() == CSession::Analyse) {
	  int nWindows;
	  const mediate_analysis_window_t *anlysWinList = m_currentIt.analysisWindowList(nWindows);
	  req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, opMode));
	}
      }

      switch (m_session->mode()) {
      case CSession::Browse:
	req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
	break;
      case CSession::Export:
	req->addRequest(new CEngineRequestBeginExportFile(m_currentIt.file().filePath()));
	break;
      case CSession::Calibrate:
	req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
	break;
      case CSession::Analyse:
	req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
	break;
      }
    }

  }

  m_thread->request(req);

}

void CQdoasEngineController::slotGotoFile(int number)
{
  CEngineRequestCompound *req = new CEngineRequestCompound;

  if (number >= 0 && number < m_numberOfFiles) {
    // implicitly checks that that m_numberOfFiles > 0
    m_currentIt(number);
    // check for a change in project
    if (m_currentProject != m_currentIt.project()) {
      int opMode = THREAD_TYPE_NONE;
      switch (m_session->mode()) {
      case CSession::Browse: opMode = THREAD_TYPE_SPECTRA; break;
      case CSession::Export: opMode = THREAD_TYPE_EXPORT; break;
      case CSession::Calibrate: opMode = THREAD_TYPE_KURUCZ; break;
      case CSession::Analyse: opMode = THREAD_TYPE_ANALYSIS; break;
      }
      m_currentProject = m_currentIt.project();
      req->addRequest(new CEngineRequestSetProject(m_currentProject, opMode));
      // might also need to replace the analysis windows
      if (m_session->mode() == CSession::Calibrate || m_session->mode() == CSession::Analyse) {
	int nWindows;
	const mediate_analysis_window_t *anlysWinList = m_currentIt.analysisWindowList(nWindows);
	req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, opMode));
      }
    }

    switch (m_session->mode()) {
    case CSession::Browse:
      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
      break;
    case CSession::Export:
      req->addRequest(new CEngineRequestBeginExportFile(m_currentIt.file().filePath()));
      break;
    case CSession::Calibrate:
      req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
      break;
    case CSession::Analyse:
      req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
      break;
    }
  }

  m_thread->request(req);
}


void CQdoasEngineController::slotFirstRecord()
{
  slotGotoRecord(1);
}

void CQdoasEngineController::slotPreviousRecord()
{
  slotGotoRecord(m_currentRecord - 1);
}

void CQdoasEngineController::slotNextRecord()
{
  if (m_currentRecord != -1) {

    switch (m_session->mode()) {
    case CSession::Browse:
      m_thread->request(new CEngineRequestBrowseNextRecord);
      break;
    case CSession::Export:
      m_thread->request(new CEngineRequestExportNextRecord);
      break;
    case CSession::Calibrate:
      m_thread->request(new CEngineRequestCalibrateNextRecord);
      break;
    case CSession::Analyse:
      m_thread->request(new CEngineRequestAnalyseNextRecord);
      break;
    }
  }
}

void CQdoasEngineController::slotLastRecord()
{
  slotGotoRecord(m_numberOfRecords);
}

void CQdoasEngineController::slotGotoRecord(int recordNumber)
{
  if (m_currentRecord != -1 && recordNumber > 0 && recordNumber <= m_numberOfRecords) {

    switch (m_session->mode()) {
    case CSession::Browse:
      m_thread->request(new CEngineRequestBrowseSpecificRecord(recordNumber));
      break;
    case CSession::Export:
      m_thread->request(new CEngineRequestExportSpecificRecord(recordNumber));
      break;
    case CSession::Calibrate:
      m_thread->request(new CEngineRequestCalibrateSpecificRecord(recordNumber));
      break;
    case CSession::Analyse:
      m_thread->request(new CEngineRequestAnalyseSpecificRecord(recordNumber));
      break;
    }
  }

}

void CQdoasEngineController::slotStep()
{
  m_engineCurrentRecord=m_currentRecord;
  m_engineCurrentFile=m_currentIt.file().filePath();

  // If the m_atEndOfCurrentFile flag is set, then the previous request indicated that no
  // more records matched. Advance to the next file if possible

  if (m_atEndOfCurrentFile) {

      bool endOfSession = true;

      if (!m_currentIt.atEnd()) {

          CSessionIterator tmpIt = m_currentIt;

          ++tmpIt;
          if (!tmpIt.atEnd()) {
              // there are more files
              m_currentIt = tmpIt;
              m_currentRecord = -1;
              m_numberOfRecords = 0;

              endOfSession = false;

              // move to the next file
              CEngineRequestCompound *req = new CEngineRequestCompound;

              // check for a change in project
              if (m_currentProject != m_currentIt.project()) {
                  int opMode = THREAD_TYPE_NONE;
                  switch (m_session->mode()) {
                      case CSession::Browse: opMode = THREAD_TYPE_SPECTRA; break;
                      case CSession::Export: opMode = THREAD_TYPE_EXPORT; break;
                      case CSession::Calibrate: opMode = THREAD_TYPE_KURUCZ; break;
                      case CSession::Analyse: opMode = THREAD_TYPE_ANALYSIS; break;
                  }
                  m_currentProject = m_currentIt.project();
                  req->addRequest(new CEngineRequestSetProject(m_currentProject, opMode));
              }

              switch (m_session->mode()) {
                  case CSession::Browse:
                      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
                      break;
                  case CSession::Export:
                      req->addRequest(new CEngineRequestBeginExportFile(m_currentIt.file().filePath()));
                      break;
                  case CSession::Calibrate:
                      req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
                      break;
                  case CSession::Analyse:
                      req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
                      break;
              }

              m_thread->request(req);
          }
      }

      if (endOfSession) {
        // Close current session, save output files
        slotStopSession();
        // Post end of .... notificatons
        if (m_session->mode()==CSession::Analyse)
          QMessageBox::information((QWidget *)this->parent(),"QDOAS : Run Analysis","End of analysis");
        else if (m_session->mode()==CSession::Calibrate)
          QMessageBox::information((QWidget *)this->parent(),"QDOAS : Run Calibrate","End of calibration");
      }

  }
  else if (m_currentRecord >= 0) {

      // step record
      switch (m_session->mode()) {
          case CSession::Browse:
              m_thread->request(new CEngineRequestBrowseNextRecord);
              break;
          case CSession::Export:
              m_thread->request(new CEngineRequestExportNextRecord);
              break;
          case CSession::Calibrate:
              m_thread->request(new CEngineRequestCalibrateNextRecord);
              break;
          case CSession::Analyse:
              m_thread->request(new CEngineRequestAnalyseNextRecord);
              break;
      }
  }
}

void CQdoasEngineController::slotStartSession(const RefCountPtr<CSession> &session)
{
  // need a compound request
  CEngineRequestCompound *req = new CEngineRequestCompound;

  // change session and reset current markers
  m_session = session;
  m_currentIt = CSessionIterator(m_session);

  QStringList sessionFileList = m_session->fileList();

  m_numberOfFiles = sessionFileList.count();
  m_currentRecord = -1;
  m_currentProject = NULL;

  if (!m_currentIt.atEnd()) {
    m_currentProject = m_currentIt.project();

    // mode dependent parts of the request
    if ((m_session->mode() == CSession::Browse) || (m_session->mode() == CSession::Export)) {

    	 int nSites;
    	 mediate_site_t *sites = m_session->takeSiteList(nSites);                  // Added by Caroline : need observation sites for spectra selection (overpasses)

    	 if (sites)
	      req->addRequest(new CEngineRequestSetSites(sites, nSites));

      if (m_session->mode() == CSession::Browse)
       {
        req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_SPECTRA));
        req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
       }
      else
       {
       	req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_EXPORT));
        req->addRequest(new CEngineRequestBeginExportFile(m_currentIt.file().filePath()));
       }
    }
    else {
      // take the site and symbol lists from the session ... and hand responsibility over to request objects.
      int nSymbols, nSites, nWindows;
      mediate_symbol_t *symbols = m_session->takeSymbolList(nSymbols);
      mediate_site_t *sites = m_session->takeSiteList(nSites);
      const mediate_analysis_window_t *anlysWinList = m_currentIt.analysisWindowList(nWindows);

      if (symbols)
	req->addRequest(new CEngineRequestSetSymbols(symbols, nSymbols));
      if (sites)
	req->addRequest(new CEngineRequestSetSites(sites, nSites));


      if (m_session->mode() == CSession::Analyse) {
	req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_ANALYSIS));
	req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, THREAD_TYPE_ANALYSIS));
	req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
      }
      else if (m_session->mode() == CSession::Calibrate) {
	req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_KURUCZ));
	req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, THREAD_TYPE_KURUCZ));
	req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
      }
    }

  }

  emit signalFileListChanged(sessionFileList);

  m_thread->request(req);

}

void CQdoasEngineController::slotStopSession()
{
  // session is stop(ping)
  emit signalSessionRunning(false);

  m_thread->request(new CEngineRequestStop);

}

void CQdoasEngineController::slotViewCrossSections(const RefCountPtr<CViewCrossSectionData> &awData)
{
  const mediate_analysis_window_t *d = awData->analysisWindow();

  int nFiles = d->crossSectionList.nCrossSection;

  char **filenames = new char*[nFiles];
  char *awName=new char[1+strlen(d->name)];

  strcpy(awName,d->name);

  for (int i=0; i<nFiles; ++i) {
    int len = strlen(d->crossSectionList.crossSection[i].crossSectionFile);
    char *tmp = new char[len + 1];
    strcpy(tmp, d->crossSectionList.crossSection[i].crossSectionFile);
    filenames[i] = tmp;
  }

  // request takes responsibility for the char** and char* memory.

  CEngineRequestViewCrossSections *req = new CEngineRequestViewCrossSections(awName,
                                                                             d->fitMinWavelength,
                                                                             d->fitMaxWavelength,
                                                                             nFiles, filenames);


  // send the request
  m_thread->request(req);
}
