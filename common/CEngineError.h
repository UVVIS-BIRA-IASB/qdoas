/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CENGINEERROR_H_GUARD
#define _CENGINEERROR_H_GUARD

#include <string>

class CEngineError
{
 public:
  CEngineError(const std::string &tag, const std::string &msg, int errorLevel) : m_tag(tag),
                                                                         m_message(msg),
                                                                         m_errorLevel(errorLevel) {};

  const std::string& message(void) const;
  const std::string& tag(void) const;
  int errorLevel(void) const;

 private:
  std::string m_tag, m_message;
  int m_errorLevel;
};

inline const std::string& CEngineError::tag(void) const { return m_tag; }
inline const std::string& CEngineError::message(void) const { return m_message; }
inline int CEngineError::errorLevel(void) const { return m_errorLevel; }

#endif
