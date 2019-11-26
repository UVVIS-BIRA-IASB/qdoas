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


#include <assert.h>

#include <QCoreApplication>
#include <QLocale>
#include <clocale>

#include "CEngineThread.h"
#include "CQdoasEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"

#include "mediate.h"

CEngineThread::CEngineThread(CQdoasEngineController *parent) :
  QThread(parent),
  m_activeRequest(NULL),
  m_terminated(false)
{
  // get an engine context
  CEngineResponse *resp = new CEngineResponseMessage;

  assert(mediateRequestCreateEngineContext(&m_engineContext, resp) == 0);
  delete resp;
}

CEngineThread::~CEngineThread()
{
  m_reqQueueMutex.lock();
  m_terminated = true;
  m_cv.wakeOne();
  m_reqQueueMutex.unlock();

  // wait until the run thread has stopped ...
  while (isRunning())
    QThread::msleep(50);

  CEngineResponse *resp = new CEngineResponseMessage;

  assert(mediateRequestDestroyEngineContext(m_engineContext, resp) == 0);
  delete resp;
}

void CEngineThread::setRunState(bool setRunning)
{
  if (setRunning) {
    m_reqQueueMutex.lock();
    m_terminated = false;
    m_reqQueueMutex.unlock();
    start();
  }
  else {
    m_reqQueueMutex.lock();
    m_terminated = true;
    m_cv.wakeOne();
    m_reqQueueMutex.unlock();
    // let the run loop exit cleanly
  }
}

void CEngineThread::request(CEngineRequest *req)
{
  m_reqQueueMutex.lock();

  m_requests.push_back(req); // add the request
  // wake the engine thread if it is sitting idle (harmless if it is not idle)
  m_cv.wakeOne();

  m_reqQueueMutex.unlock();
}

void CEngineThread::takeResponses(QList<CEngineResponse*> &responses)
{
  m_respQueueMutex.lock();

  responses = m_responses;
  m_responses.clear();

  m_respQueueMutex.unlock();
}

void CEngineThread::respond(CEngineResponse *resp)
{
  // used by the engine thread to post data for collection by the GUI thread
  m_respQueueMutex.lock();

  m_responses.push_back(resp);
  // post the notify event to the parent
  QCoreApplication::postEvent(parent(), new QEvent(cEngineResponseType));

  m_respQueueMutex.unlock();
}

void CEngineThread::run()
{
  // engine thread - loop until terminated ...
  //
  // m_terminated, m_requests and are protected by
  // m_reqQueueMutex

  // set the locale to "C" to avoid parsing problems when the system locale uses a different decimal separator
	// to avoid that a thousands comma separator (QT 4.7.3)

	   QLocale qlocale=QLocale::system();
	   qlocale.setNumberOptions(QLocale::OmitGroupSeparator);
	   QLocale::setDefault(qlocale);

  setlocale(LC_NUMERIC, "C");

  CEngineRequest *activeRequest;

  m_reqQueueMutex.lock();
  while (!m_terminated) {

    // will wake the thread if a request is lodged or terminated is set true.
    while (m_requests.isEmpty() && !m_terminated) {
      m_cv.wait(&m_reqQueueMutex);
    }

    if (!m_requests.isEmpty() && !m_terminated) {
      // queue is not empty - process the first item
      activeRequest = m_requests.takeFirst();

      m_reqQueueMutex.unlock();
      // process the request then discard it
      activeRequest->process(this);           // what about the return code ???? TODO
      delete activeRequest;

      // lock before modifying the queue and checking m_terminated
      m_reqQueueMutex.lock();
    }
  }
  m_reqQueueMutex.unlock();
}
