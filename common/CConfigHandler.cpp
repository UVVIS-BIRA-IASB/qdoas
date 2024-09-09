/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QTextStream>

#include "CConfigHandler.h"

#include "debugutil.h"

CConfigHandler::CConfigHandler() :
  QXmlDefaultHandler(),
  m_activeSubHandler(NULL),
  m_paths(10)
{
}

CConfigHandler::~CConfigHandler()
{
  // delete any sub handlers ...
  while (!m_subHandlerStack.isEmpty()) {
    delete m_subHandlerStack.back().handler;
    m_subHandlerStack.pop_back();
  }
}

QString CConfigHandler::messages(void) const
{
  return m_errorMessages;
}

bool CConfigHandler::characters(const QString &ch)
{
  // collects all character data into a single string. This is
  // passed to sub handlers IFF the trimmed result is not empty.

  if (m_activeSubHandler) {

    m_collatedStr += ch;
  }

  return true;
}

bool CConfigHandler::endDocument()
{
  return true;
}

bool CConfigHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
  bool status = true;

  if (m_activeSubHandler) {
    // delegate to the sub handler

    // first any collected character data
    QString tmp(m_collatedStr.trimmed());
    if (!tmp.isEmpty()) {
      status = m_activeSubHandler->character(tmp);
    }

    if (status) {
      if (m_subHandlerStack.back().depth == m_elementStack.count()) {
    status = m_activeSubHandler->end();
    // done with this handler ... discard it
    delete m_activeSubHandler;
    m_subHandlerStack.pop_back();
    // revert back to the previous handler
    if (!m_subHandlerStack.isEmpty())
      m_activeSubHandler = m_subHandlerStack.back().handler;
    else
      m_activeSubHandler = NULL;
      }
      else {
    status = m_activeSubHandler->end(qName);
      }
    }
  }
  else {
    // no sub handler ...
  }

  m_elementStack.pop_back();

  return status;
}

QString CConfigHandler::errorString() const
{
  return m_subErrorMessage;
}

bool CConfigHandler::ignorableWhitespace(const QString &ch)
{
  return true;
}

bool CConfigHandler::startDocument()
{
  return true;
}

bool CConfigHandler::startElement(const QString &namespaceURI, const QString &localName,
                  const QString &qName, const QXmlAttributes &atts)
{
  bool result;

  if (delegateStartElement(qName, atts, result))
    return result;

  return false;
}

bool CConfigHandler::installSubHandler(CConfigSubHandler *newHandler,
                           const QXmlAttributes &atts)
{
  m_subHandlerStack.push_back(SSubHandlerItem(newHandler, m_elementStack.count()));
  m_activeSubHandler = newHandler;

  return m_activeSubHandler->start(atts);
}

void CConfigHandler::setPath(int index, const QString &pathPrefix)
{
  // index MUST be in the range 0-9
  if (index < 0 || index > 9)
    return;

  // copy and remove any trailing '/' or '\' characters ...
  QString tmp(pathPrefix);

  while (!tmp.isEmpty() && (tmp.endsWith('/') || tmp.endsWith('\\')))
    tmp.chop(1);

  m_paths[index] = tmp;
}

QString CConfigHandler::getPath(int index) const
{
  if (index < 0 || index > 9)
    return QString();

  return m_paths[index];
}

QString CConfigHandler::pathExpand(const QString &name)
{
  // replace a '%?' prefix with a path (? must be a digit).

  int len = name.length();
  if (len > 1 && name.startsWith('%') && name.at(1).isDigit()) {

    int index = name.at(1).digitValue();

    assert(index >= 0 && index <= 9);

    QString tmp = m_paths.at(index);
    if (len > 2)
      tmp += name.right(len - 2);

    return tmp;
  }

  return name;
}

bool CConfigHandler::delegateStartElement(const QString &qName, const QXmlAttributes &atts, bool &result)
{
  // always track the stack - also provides the depth
  m_elementStack.push_back(qName);

  if (m_activeSubHandler) {
    // prepare for collation of character data
    m_collatedStr.clear();

    // delegate to the sub handler
    result = m_activeSubHandler->start(qName, atts);

    return true; // delegated
  }

  return false; // not delegated to a sub handler
}

//------------------------------------------------------------------------

bool CConfigSubHandler::postErrorMessage(const QString &msg)
{
  master()->setSubErrorMessage(msg);
  return false;
}
