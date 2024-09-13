#include <map>

#include "CPathSubHandler.h"

#include "debugutil.h"

using std::map;

//------------------------------------------------------------------------
//
// Handler for <paths> element (and sub elements)

void CPathSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, QString>& atts)
{
  // should be a path element <path index="?">/this/is/the/path</path>

  if (element == "path") {
    bool ok;
    m_index = value(atts, "index").toInt(&ok);
    if (!ok || m_index < 0 || m_index > 9) {
      m_index = -1;
      throw std::runtime_error("Invalid path index");
    }
  }
  else {
    m_index = -1;
    throw std::runtime_error("Invalid child element of paths");
  }

  m_path.clear();
}

void CPathSubHandler::character(const QString &ch)
{
  // collect all path characters
  m_path += ch;
}

void CPathSubHandler::end(const Glib::ustring &element)
{
  if (m_index != -1)
    m_master->setPath(m_index, m_path);
}
