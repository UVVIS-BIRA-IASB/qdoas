
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


#include <QTextStream>

#include "CPathMgr.h"

#include "debugutil.h"

CPathMgr *CPathMgr::m_instance = NULL;

CPathMgr* CPathMgr::instance(void)
{
  if (m_instance == NULL)
    m_instance = new CPathMgr;

  return m_instance;
}

CPathMgr::~CPathMgr()
{
  m_instance = NULL;
}

void CPathMgr::removeAll(void)
{
  m_pathSet.clear();
}

void CPathMgr::removePath(int index)
{
  // locate by index - walk the set

  std::set<SPathBucket>::iterator it = m_pathSet.begin();
  while (it != m_pathSet.end()) {
    if (it->index == index) {
      m_pathSet.erase(it);
      break;
    }
    ++it;
  }
}

void CPathMgr::addPath(int index, const QString &path)
{
  // DO NOT allow empty paths to be stored.
  // DO NOT allow duplicate indexes

  // check for this index - just remove it if it exists
  std::set<SPathBucket>::iterator it = m_pathSet.begin();
  while (it != m_pathSet.end()) {
    if (it->index == index) {
      m_pathSet.erase(it);
      break;
    }
    ++it;
  }

  if (path.isEmpty())
    return;

  m_pathSet.insert(SPathBucket(index, path));
}

QString CPathMgr::simplifyPath(const QString &name) const
{
  // walk in search of a matching path. set is sorted so that
  // a longer path will match first.

  std::set<SPathBucket>::const_iterator it = m_pathSet.begin();
  while (it != m_pathSet.end()) {
    
    if (name.startsWith(it->path)) {
      // matches ... but must match either perfectly or on a path separator boundary
      int len = it->path.length();  // certain that name.length() > path.length()
      if (name.length() == len) {
	// one-to-one match
	QString tmp;
	QTextStream stream(&tmp);
	
	stream << '%' << it->index;

	return tmp;
      }
      else if (name.at(len) == '/' || name.at(len) == '\\') {
	// matches on separator boundary
	QString tmp;
	QTextStream stream(&tmp);
	
	stream << '%' << it->index << name.right(name.length() - len);

	return tmp;
	
      }
    }
    ++it;
  }
  // no matches
  return name;
}

QString CPathMgr::path(int index) const
{
  std::set<SPathBucket>::const_iterator it = m_pathSet.begin();
  while (it != m_pathSet.end() && it->index != index) ++it;
  if (it != m_pathSet.end())
    return it->path;

  return QString();
}

