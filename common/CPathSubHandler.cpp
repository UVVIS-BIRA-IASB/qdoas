
#include "CPathSubHandler.h"

#include "debugutil.h"


//------------------------------------------------------------------------
//
// Handler for <paths> element (and sub elements)

bool CPathSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // should be a path element <path index="?">/this/is/the/path</path>

  if (element == "path") {
    bool ok;
    m_index = atts.value("index").toInt(&ok);
    if (!ok || m_index < 0 || m_index > 9) {
      m_index = -1;
      return postErrorMessage("Invalid path index");
    }
  }
  else {
    m_index = -1;
    return postErrorMessage("Invalid child element of paths");
  }

  m_path.clear();

  return true;
}

bool CPathSubHandler::character(const QString &ch)
{
  // collect all path characters
  m_path += ch;

  return true;
}

bool CPathSubHandler::end(const QString &element)
{
  if (m_index != -1)
    m_master->setPath(m_index, m_path);

  return true;
}
