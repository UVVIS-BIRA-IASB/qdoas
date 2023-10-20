/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <string.h>

#include "mediate_ring.h"
#include "constants.h"

void initializeMediateRing(mediate_ring_t *d)
{
  // Initializations

  memset(d, 0, sizeof(mediate_ring_t));

  d->normalize=1;
  d->temperature = 250.0;
  d->n_groundpixel=1;
}
