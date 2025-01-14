#include <map>

#include "CPathSubHandler.h"

#include "debugutil.h"

using std::map;
using std::string;

//------------------------------------------------------------------------
//
// Handler for <paths> element (and sub elements)

void CPathSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, string>& atts)
{
  // should be a path element <path index="?">/this/is/the/path</path>

  if (element == "path") {
    try {
      m_index = stoi(value(atts, "index"));
    } catch(std::invalid_argument &e) {
      m_index = -1;
    }
    if (m_index < 0 || m_index > 9) {
      throw std::runtime_error("Invalid path index");
    }
  }
  else {
    m_index = -1;
    throw std::runtime_error("Invalid child element of paths");
  }

  m_path.clear();
}

void CPathSubHandler::character(const string &ch)
{
  // collect all path characters
  m_path += ch;
}

void CPathSubHandler::end(const Glib::ustring &element)
{
  if (m_index != -1)
    m_master->setPath(m_index, m_path);
}
