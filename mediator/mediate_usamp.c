/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <string.h>

#include "mediate_usamp.h"
#include "constants.h"

void initializeMediateUsamp(mediate_usamp_t *d)
{
  // Initializations
  
  memset(d, 0, sizeof(mediate_usamp_t));
}
