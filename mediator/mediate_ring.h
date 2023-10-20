/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _MEDIATE_RING_H_GUARD
#define _MEDIATE_RING_H_GUARD

#include "mediate_limits.h"
#include "mediate_general.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif


  /****************************************************/
  /* Slit - mediate_slit_function_t */


  /****************************************************/
  /* Ring */

  typedef struct mediate_ring
  {
    double temperature;
    int normalize;
    int noheader;
    int saveraman;
    char outputFile[FILENAME_BUFFER_LENGTH];
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    mediate_slit_function_t slit;
    int formatType;
    int n_groundpixel;
  } mediate_ring_t;


  void initializeMediateRing(mediate_ring_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
