/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <iostream>

#include "CConfigHandler.h"

using std::map;

CConfigHandler::CConfigHandler() :
  xmlpp::SaxParser(),
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

void CConfigHandler::on_start_element(const Glib::ustring& name,
                                      const AttributeList& attributes)
{
  map<Glib::ustring, QString> atts;
  for (const auto& att : attributes) {
    atts[att.name] = QString::fromStdString(att.value);
  }

  element_stack.push_back(name);

  if (m_activeSubHandler) {
    // prepare for collation of character data
    collated_str.clear();
    // delegate to sub handler
    m_activeSubHandler->start(name, atts);
  } else {
    this->start_subhandler(name, atts);
  }
}

void CConfigHandler::on_end_element(const Glib::ustring& name)
{
  if (m_activeSubHandler) {
    bool status = true;
    // delegate to the sub handler

    // first any collected character data
    QString tmp(QString::fromStdString(collated_str).trimmed());
    if (!tmp.isEmpty()) {
      status = m_activeSubHandler->character(tmp);
    }

    if (status) {
      if (m_subHandlerStack.back().depth == element_stack.size()) {
        status = m_activeSubHandler->end();
        // done with this handler ... discard it
        delete m_activeSubHandler;
        m_subHandlerStack.pop_back();
        // revert back to the previous handler
        if (!m_subHandlerStack.isEmpty())
          m_activeSubHandler = m_subHandlerStack.back().handler;
        else
          m_activeSubHandler = NULL;
      } else {
        m_activeSubHandler->end(name);
      }
    }
  }

  element_stack.pop_back();
}

void CConfigHandler::on_characters(const Glib::ustring& text)
{
  if (m_activeSubHandler) {
    collated_str += text;
  }
}

QString CConfigHandler::messages(void) const
{
  return m_errorMessages;
}

QString CConfigHandler::errorString() const
{
  return m_subErrorMessage;
}

bool CConfigHandler::ignorableWhitespace(const QString &ch)
{
  return true;
}

void CConfigHandler::install_subhandler(CConfigSubHandler *newHandler,
                                        const map<Glib::ustring, QString>& attributes) {
  m_subHandlerStack.push_back(SSubHandlerItem(newHandler, element_stack.size()));
  m_activeSubHandler = newHandler;

  return m_activeSubHandler->start(attributes);

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

//------------------------------------------------------------------------

bool CConfigSubHandler::postErrorMessage(const QString &msg)
{
  master()->setSubErrorMessage(msg);
  return false;
}
