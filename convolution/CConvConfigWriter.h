/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CCONVCONFIGWRITER_H_GUARD
#define _CCONVCONFIGWRITER_H_GUARD

#include <cstdio>

#include "mediate_convolution.h"

class CConvConfigWriter
{
 public:
  CConvConfigWriter(const mediate_convolution_t *properties);
  ~CConvConfigWriter();

  QString write(const QString &fileName);

 private:
  void writeGeneral(FILE *fp, const mediate_conv_general_t *d);

 private:
  const mediate_convolution_t *m_properties;
};

#endif
