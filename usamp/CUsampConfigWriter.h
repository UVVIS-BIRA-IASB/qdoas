/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CUSAMPCONFIGWRITER_H_GUARD
#define _CUSAMPCONFIGWRITER_H_GUARD

#include <cstdio>

#include "mediate_usamp.h"

class CUsampConfigWriter
{
 public:
  CUsampConfigWriter(const mediate_usamp_t *properties);
  ~CUsampConfigWriter();

  QString write(const QString &fileName);

 private:
  const mediate_usamp_t *m_properties;
};

#endif
