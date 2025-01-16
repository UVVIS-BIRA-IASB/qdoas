/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <memory>
#include <string>

#ifndef _CPROJECTCONFIGTREENODE_H_GUARD
#define _CPROJECTCONFIGTREENODE_H_GUARD

class CProjectConfigTreeNode
{
 public:
  enum Type { eFile, eFolder, eDirectory };

  CProjectConfigTreeNode(const std::string &name, bool enabled);

  std::shared_ptr<CProjectConfigTreeNode> firstChild(void) const;
  std::shared_ptr<CProjectConfigTreeNode> nextSibling(void) const;

  void addChild(std::shared_ptr<CProjectConfigTreeNode> child);
  void setNextSibling(std::shared_ptr<CProjectConfigTreeNode> sibling);

  const std::string &name(void) const;
  bool isEnabled(void) const;

  virtual Type type(void) const = 0;
  virtual std::string filter(void) const;
  virtual bool recursive(void) const;

 protected:
  std::shared_ptr<CProjectConfigTreeNode> m_firstChild, m_nextSibling;
  std::string m_name;
  bool m_enabled;
};

class CProjectConfigFile : public CProjectConfigTreeNode
{
 public:
  CProjectConfigFile(const std::string &name, bool enabled);

  virtual Type type(void) const;
};

class CProjectConfigFolder : public CProjectConfigTreeNode
{
 public:
  CProjectConfigFolder(const std::string &name, bool enabled);

  virtual Type type(void) const;
};

class CProjectConfigDirectory : public CProjectConfigTreeNode
{
 public:
  CProjectConfigDirectory(const std::string &name, const std::string &filter, bool recurse, bool enabled);

  virtual Type type(void) const;
  virtual std::string filter(void) const;
  virtual bool recursive(void) const;

 private:
  std::string m_filter;
  bool m_recurse;
};

inline std::shared_ptr<CProjectConfigTreeNode> CProjectConfigTreeNode::firstChild(void) const { return m_firstChild; }
inline std::shared_ptr<CProjectConfigTreeNode> CProjectConfigTreeNode::nextSibling(void) const { return m_nextSibling; }
inline void CProjectConfigTreeNode::setNextSibling(std::shared_ptr<CProjectConfigTreeNode> sibling) { m_nextSibling = sibling; }
inline const std::string& CProjectConfigTreeNode::name(void) const { return m_name; }
inline bool CProjectConfigTreeNode::isEnabled(void) const { return m_enabled; }

#endif
