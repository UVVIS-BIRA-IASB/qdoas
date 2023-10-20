/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPATHMGR_H_GUARD
#define _CPATHMGR_H_GUARD

#include <set>

#include <QString>

struct SPathBucket
{
  int index;
  QString path;

  SPathBucket(int i, const QString &p) : index(i), path(p) {}
  // sort with longest path first ... if equal then string compare with operator<()
  bool operator<(const SPathBucket &rhs) const { return ((path.length() > rhs.path.length()) || (path.length() == rhs.path.length() && path < rhs.path)); }
};

class CPathMgr
{
 public:
  static CPathMgr* instance(void);

  ~CPathMgr();

  void removeAll(void);

  void removePath(int index);
  void addPath(int index, const QString &path);
  QString simplifyPath(const QString &name) const;
  QString path(int index) const;

 private:
  // singleton => no copies permitted
  CPathMgr() {}
  CPathMgr(const CPathMgr &) {}
  CPathMgr& operator=(const CPathMgr &) { return *this; }

 private:
  static CPathMgr *m_instance;

  std::set<SPathBucket> m_pathSet;
};

#endif

