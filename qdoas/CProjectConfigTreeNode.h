/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPROJECTCONFIGTREENODE_H_GUARD
#define _CPROJECTCONFIGTREENODE_H_GUARD

#include <QString>

class CProjectConfigTreeNode
{
 public:
  enum Type { eFile, eFolder, eDirectory };

  CProjectConfigTreeNode(const QString &name, bool enabled);
  virtual ~CProjectConfigTreeNode();

  CProjectConfigTreeNode *firstChild(void) const;
  CProjectConfigTreeNode *nextSibling(void) const;

  void addChild(CProjectConfigTreeNode *child);
  void setNextSibling(CProjectConfigTreeNode *sibling);

  const QString &name(void) const;
  bool isEnabled(void) const;

  virtual Type type(void) const = 0;
  virtual QString filter(void) const;
  virtual bool recursive(void) const;

 protected:
  CProjectConfigTreeNode *m_firstChild, *m_nextSibling;
  QString m_name;
  bool m_enabled;
};

class CProjectConfigFile : public CProjectConfigTreeNode
{
 public:
  CProjectConfigFile(const QString &name, bool enabled);

  virtual Type type(void) const;
};

class CProjectConfigFolder : public CProjectConfigTreeNode
{
 public:
  CProjectConfigFolder(const QString &name, bool enabled);

  virtual Type type(void) const;
};

class CProjectConfigDirectory : public CProjectConfigTreeNode
{
 public:
  CProjectConfigDirectory(const QString &name, const QString &filter, bool recurse, bool enabled);

  virtual Type type(void) const;
  virtual QString filter(void) const;
  virtual bool recursive(void) const;

 private:
  QString m_filter;
  bool m_recurse;
};

inline CProjectConfigTreeNode* CProjectConfigTreeNode::firstChild(void) const { return m_firstChild; }
inline CProjectConfigTreeNode* CProjectConfigTreeNode::nextSibling(void) const { return m_nextSibling; }
inline void CProjectConfigTreeNode::setNextSibling(CProjectConfigTreeNode *sibling) { m_nextSibling = sibling; }
inline const QString& CProjectConfigTreeNode::name(void) const { return m_name; }
inline bool CProjectConfigTreeNode::isEnabled(void) const { return m_enabled; }

#endif
