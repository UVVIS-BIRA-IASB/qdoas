/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CHELPSYSTEM_H_GUARD
#define _CHELPSYSTEM_H_GUARD

class QString;

class CHelpSystem
{
 public:
  static void showHelpTopic(const QString &chapter, const QString &key = QString());
  static QString changeDir(void);
};

#endif
