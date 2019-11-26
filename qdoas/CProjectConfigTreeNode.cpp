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

#include "CProjectConfigTreeNode.h"

CProjectConfigTreeNode::CProjectConfigTreeNode(const QString &name, bool enabled) :
  m_firstChild(NULL),
  m_nextSibling(NULL),
  m_name(name),
  m_enabled(enabled)
{
}

CProjectConfigTreeNode::~CProjectConfigTreeNode()
{
  delete m_nextSibling;
  delete m_firstChild;
}

void CProjectConfigTreeNode::addChild(CProjectConfigTreeNode *child)
{
  if (m_firstChild)
    child->setNextSibling(m_firstChild);
  m_firstChild = child;
}

QString CProjectConfigTreeNode::filter(void) const
{
  return QString();
}

bool CProjectConfigTreeNode::recursive(void) const
{
  return false;
}


CProjectConfigFile::CProjectConfigFile(const QString &name, bool enabled) :
  CProjectConfigTreeNode(name, enabled)
{
}

CProjectConfigTreeNode::Type CProjectConfigFile::type(void) const
{
  return CProjectConfigTreeNode::eFile;
}

  
CProjectConfigFolder::CProjectConfigFolder(const QString &name, bool enabled) :
  CProjectConfigTreeNode(name, enabled)
{
}

CProjectConfigTreeNode::Type CProjectConfigFolder::type(void) const
{
  return CProjectConfigTreeNode::eFolder;
}

  
CProjectConfigDirectory::CProjectConfigDirectory(const QString &name, const QString &filter, bool recurse, bool enabled) :
  CProjectConfigTreeNode(name, enabled),
  m_filter(filter),
  m_recurse(recurse)
{
}

CProjectConfigTreeNode::Type CProjectConfigDirectory::type(void) const
{
  return CProjectConfigTreeNode::eDirectory;
}

QString CProjectConfigDirectory::filter(void) const
{
  return m_filter;
}

bool CProjectConfigDirectory::recursive(void) const
{
  return m_recurse;
}

