/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CRINGCONFIGWRITER_H_GUARD
#define _CRINGCONFIGWRITER_H_GUARD

#include <cstdio>

#include "mediate_ring.h"

class CRingConfigWriter
{
 public:
  CRingConfigWriter(const mediate_ring_t *properties);
  ~CRingConfigWriter();

  QString write(const QString &fileName);

 private:
  const mediate_ring_t *m_properties;
};

#endif
