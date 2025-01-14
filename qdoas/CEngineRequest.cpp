/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include "CPreferences.h"
#include "CWorkSpace.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"
#include "CEngineThread.h"

#include "mediate.h"

#include "debugutil.h"

//------------------------------------------------------------

CEngineRequestCompound::~CEngineRequestCompound()
{
  // delete all requests
  while (!m_requestList.isEmpty()) {
    delete m_requestList.takeFirst();
  }
}

bool CEngineRequestCompound::process(CEngineThread *engineThread)
{
  bool result = true; // empty is ok
  QList<CEngineRequest*>::iterator it = m_requestList.begin();

  // process sequentially until done or a failure occurred
  while (it != m_requestList.end() && (result = (*it)->process(engineThread)))
    ++it;

  return result;
}

void CEngineRequestCompound::addRequest(CEngineRequest *req)
{
  // takes owneship of req
  m_requestList.push_back(req);
}

//------------------------------------------------------------

CEngineRequestSetProject::CEngineRequestSetProject(const mediate_project_t *project, int opMode) :
  m_opMode(opMode)
{
  // deep copy the data from project.
  m_project = *project;
}

bool CEngineRequestSetProject::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseVisual;

  int rc = mediateRequestSetProject(engineThread->engineContext(),
                    &m_project, m_opMode, resp);


  // post the response
  engineThread->respond(resp);

  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestSetAnalysisWindows::CEngineRequestSetAnalysisWindows(const mediate_analysis_window_t *windowList, int nWindows,
                                   int opMode) :
  m_windowList(NULL),
  m_nWindows(0),
  m_opMode(opMode)
{
  // deep copy the data from windowList
  if (windowList != NULL && nWindows > 0) {
    m_nWindows = nWindows;
    m_windowList = new mediate_analysis_window_t[m_nWindows];
    memcpy(m_windowList, windowList, m_nWindows * sizeof(mediate_analysis_window_t));
  }
}

CEngineRequestSetAnalysisWindows::~CEngineRequestSetAnalysisWindows()
{
  delete [] m_windowList;
}

bool CEngineRequestSetAnalysisWindows::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseVisual;

  int rc = mediateRequestSetAnalysisWindows(engineThread->engineContext(),
                        m_nWindows, m_windowList, m_opMode, resp);

  // post the response
  engineThread->respond(resp);

  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestSetSymbols::CEngineRequestSetSymbols(mediate_symbol_t *symbolList, int nSymbols) :
  m_symbolList(symbolList),
  m_nSymbols(nSymbols)
{
  // takes ownership of the symbolList
}

CEngineRequestSetSymbols::~CEngineRequestSetSymbols()
{
  delete [] m_symbolList;
}

bool CEngineRequestSetSymbols::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseVisual;

  int rc = mediateRequestSetSymbols(engineThread->engineContext(),
                    m_nSymbols, m_symbolList, resp);

  // post the response
  engineThread->respond(resp);

  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestSetSites::CEngineRequestSetSites(mediate_site_t *siteList, int nSites) :
  m_siteList(siteList),
  m_nSites(nSites)
{
  // takes ownership of the siteList
}

CEngineRequestSetSites::~CEngineRequestSetSites()
{
  delete [] m_siteList;
}

bool CEngineRequestSetSites::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseVisual;

  int rc = mediateRequestSetSites(engineThread->engineContext(),
                  m_nSites, m_siteList, resp);

  // post the response
  engineThread->respond(resp);

  return (rc == 0);
}

//------------------------------------------------------------

bool CEngineRequestBeginBrowseFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginBrowseSpectra(engineThread->engineContext(),
                        m_fileName.c_str(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestBrowseNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestNextMatchingBrowseSpectrum(engineThread->engineContext(),
                            resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestBrowseSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
                      m_recordNumber, resp);

  if (rc > 0) {
    // successfully positioned .. now browse
    rc = mediateRequestNextMatchingBrowseSpectrum(engineThread->engineContext(),
                          resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestBeginExportFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginExportSpectra(engineThread->engineContext(),
                                            m_fileName.c_str(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestExportNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestNextMatchingBrowseSpectrum(engineThread->engineContext(),
                            resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestExportSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
                      m_recordNumber, resp);

  if (rc > 0) {
    // successfully positioned .. now Export
    rc = mediateRequestNextMatchingBrowseSpectrum(engineThread->engineContext(),
                          resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestBeginAnalyseFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginAnalyseSpectra(engineThread->engineContext(),
                                             CWorkSpace::instance()->getConfigFile().c_str(),
                                             m_fileName.c_str(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestAnalyseNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestNextMatchingAnalyseSpectrum(engineThread->engineContext(),
                             resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestAnalyseSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
                      m_recordNumber, resp);

  if (rc > 0) {
    // successfully positioned .. now analyse
    rc = mediateRequestNextMatchingAnalyseSpectrum(engineThread->engineContext(),
                           resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestBeginCalibrateFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginCalibrateSpectra(engineThread->engineContext(),
                        m_fileName.c_str(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestCalibrateNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestNextMatchingCalibrateSpectrum(engineThread->engineContext(),
                            resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestCalibrateSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
                      m_recordNumber, resp);

  if (rc > 0) {
    // successfully positioned .. now analyse
    rc = mediateRequestNextMatchingCalibrateSpectrum(engineThread->engineContext(),
                           resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

bool CEngineRequestStop::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseMessage *resp = new CEngineResponseMessage;

  int rc = mediateRequestStop(engineThread->engineContext(), resp);

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestViewCrossSections::CEngineRequestViewCrossSections(char *awName,double minWavelength, double maxWavelength,
                                                                 int nFiles, char **filenames) :
  m_awName(awName),
  m_minWavelength(minWavelength),
  m_maxWavelength(maxWavelength),
  m_nFiles(nFiles),
  m_filenames(filenames)
{
}

CEngineRequestViewCrossSections::~CEngineRequestViewCrossSections()
{
  for (int i=0; i<m_nFiles; ++i) {
    delete [] m_filenames[i];
  }
  delete [] m_filenames;

  delete [] m_awName;
}

bool CEngineRequestViewCrossSections::process(CEngineThread *engineThread)
{
  CEngineResponseVisual *resp = new CEngineResponseVisual;

  int rc = mediateRequestViewCrossSections(engineThread->engineContext(),
                                           m_awName, m_minWavelength, m_maxWavelength,
                                           m_nFiles, m_filenames, resp);

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}
