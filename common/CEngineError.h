/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CENGINEERROR_H_GUARD
#define _CENGINEERROR_H_GUARD

#include <QString>

class CEngineError
{
 public:
  CEngineError(const QString &tag, const QString &msg, int errorLevel) : m_tag(tag),
                                                                         m_message(msg),
                                                                         m_errorLevel(errorLevel) {};

  const QString& message(void) const;
  const QString& tag(void) const;
  int errorLevel(void) const;

 private:
  QString m_tag, m_message;
  int m_errorLevel;
};

inline const QString& CEngineError::tag(void) const { return m_tag; }
inline const QString& CEngineError::message(void) const { return m_message; }
inline int CEngineError::errorLevel(void) const { return m_errorLevel; }

#endif
