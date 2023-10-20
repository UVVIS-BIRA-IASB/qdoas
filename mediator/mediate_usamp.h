/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _MEDIATE_USAMP_H_GUARD
#define _MEDIATE_USAMP_H_GUARD

#include "mediate_limits.h"
#include "mediate_general.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif


  /****************************************************/
  /* Slit - mediate_slit_function_t */


  /****************************************************/
  /* Usamp */

  typedef struct mediate_usamp
  {
    int methodType;
    double shift;
    int noheader;
    char outputPhaseOneFile[FILENAME_BUFFER_LENGTH];
    char outputPhaseTwoFile[FILENAME_BUFFER_LENGTH];
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    mediate_slit_function_t slit;
  } mediate_usamp_t;


  void initializeMediateUsamp(mediate_usamp_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
