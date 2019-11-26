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

#ifndef _CENGINETHREAD_H_GUARD
#define _CENGINETHREAD_H_GUARD

#include <QList>
#include <QEvent>

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// semi-random number for the custom event
const QEvent::Type cEngineResponseType = QEvent::Type(QEvent::User + 127);

class CEngineRequest;
class CEngineResponse;
class CQdoasEngineController;

class CEngineThread : public QThread
{
 public:
  CEngineThread(CQdoasEngineController *parent);
  virtual ~CEngineThread();

  // thread-safe interface for the controller
  void request(CEngineRequest *req); // queue for dispatch when the engine is not busy
  void takeResponses(QList<CEngineResponse*> &responses);
  void setRunState(bool setRunning);

  // interface for CEngineRequest::process

  void respond(CEngineResponse *resp);

  void* engineContext(void);

 protected:
  virtual void run();

 private:
  QMutex m_reqQueueMutex, m_respQueueMutex;
  QWaitCondition m_cv;

  QList<CEngineRequest*> m_requests;
  QList<CEngineResponse*> m_responses;
  CEngineRequest *m_activeRequest;
  bool m_terminated;

  void *m_engineContext;
};

inline void* CEngineThread::engineContext(void) { return m_engineContext; }

#endif
