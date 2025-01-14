/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include "CProjectConfigTreeNode.h"

using std::string;

CProjectConfigTreeNode::CProjectConfigTreeNode(const string &name, bool enabled) :
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

string CProjectConfigTreeNode::filter(void) const
{
  return "";
}

bool CProjectConfigTreeNode::recursive(void) const
{
  return false;
}


CProjectConfigFile::CProjectConfigFile(const string &name, bool enabled) :
  CProjectConfigTreeNode(name, enabled)
{
}

CProjectConfigTreeNode::Type CProjectConfigFile::type(void) const
{
  return CProjectConfigTreeNode::eFile;
}


CProjectConfigFolder::CProjectConfigFolder(const string &name, bool enabled) :
  CProjectConfigTreeNode(name, enabled)
{
}

CProjectConfigTreeNode::Type CProjectConfigFolder::type(void) const
{
  return CProjectConfigTreeNode::eFolder;
}


CProjectConfigDirectory::CProjectConfigDirectory(const string &name, const string &filter, bool recurse, bool enabled) :
  CProjectConfigTreeNode(name, enabled),
  m_filter(filter),
  m_recurse(recurse)
{
}

CProjectConfigTreeNode::Type CProjectConfigDirectory::type(void) const
{
  return CProjectConfigTreeNode::eDirectory;
}

string CProjectConfigDirectory::filter(void) const
{
  return m_filter;
}

bool CProjectConfigDirectory::recursive(void) const
{
  return m_recurse;
}

