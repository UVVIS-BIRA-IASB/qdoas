/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <string.h>

#include "mediate_convolution.h"
#include "constants.h"

void initializeMediateConvolution(mediate_convolution_t *d)
{
  // Initializations

  memset(d, 0, sizeof(mediate_convolution_t));
  
  d->general.n_groundpixel=1;
}
