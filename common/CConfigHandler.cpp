/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <cassert>

#include <boost/algorithm/string.hpp>

#include "CConfigHandler.h"

using std::map;
using std::string;
using std::vector;

CConfigHandler::CConfigHandler() :
  xmlpp::SaxParser(),
  m_activeSubHandler(NULL),
  m_paths(10)
{
}

CConfigHandler::~CConfigHandler()
{
  // delete any sub handlers ...
  while (!m_subHandlerStack.empty()) {
    delete m_subHandlerStack.back().handler;
    m_subHandlerStack.pop_back();
  }
}

void CConfigHandler::on_start_element(const Glib::ustring& name,
                                      const AttributeList& attributes)
{
  map<Glib::ustring, string> atts;
  for (const auto& att : attributes) {
    atts[att.name] = att.value;
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
    // delegate to the sub handler

    // first any collected character data
    string tmp(boost::trim_copy(string(collated_str)));
    if (!tmp.empty()) {
      m_activeSubHandler->character(tmp);
    }

    if (m_subHandlerStack.back().depth == element_stack.size()) {
      m_activeSubHandler->end();
      // done with this handler ... discard it
      delete m_activeSubHandler;
      m_subHandlerStack.pop_back();
      // revert back to the previous handler
      if (!m_subHandlerStack.empty())
        m_activeSubHandler = m_subHandlerStack.back().handler;
      else
        m_activeSubHandler = NULL;
    } else {
      m_activeSubHandler->end(name);
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

string CConfigHandler::messages(void) const
{
  return m_errorMessages;
}

string CConfigHandler::errorString() const
{
  return m_subErrorMessage;
}

void CConfigHandler::install_subhandler(CConfigSubHandler *newHandler,
                                        const map<Glib::ustring, string>& attributes) {
  m_subHandlerStack.push_back(SSubHandlerItem(newHandler, element_stack.size()));
  m_activeSubHandler = newHandler;

  return m_activeSubHandler->start(attributes);

}

void CConfigHandler::setPath(int index, const string &pathPrefix)
{
  // index MUST be in the range 0-9
  if (index < 0 || index > 9)
    return;

  // copy and remove any trailing '/' or '\' characters ...
  string tmp(pathPrefix);

  while (!tmp.empty() && ((tmp.back() == '/') || (tmp.back() =='\\')))
    tmp.pop_back();

  m_paths[index] = tmp;
}

string CConfigHandler::getPath(int index) const
{
  if (index < 0 || index > 9)
    return string();

  return m_paths[index];
}

string CConfigHandler::pathExpand(const string &name)
{
  // replace a '%?' prefix with a path (? must be a digit).

  int len = name.length();
  if (len > 1 && name.at(0) =='%' && std::isdigit(name.at(1))) {

    int index = name.at(1) - '0';

    assert(index >= 0 && index <= 9);

    string tmp = m_paths.at(index);
    if (len > 2)
      tmp += name.substr(2);

    return tmp;
  }

  return name;
}
