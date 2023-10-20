/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _MEDIATE_CONVOLUTION_H_GUARD
#define _MEDIATE_CONVOLUTION_H_GUARD

#include "mediate_limits.h"
#include "mediate_general.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif



  /****************************************************/
  /* General */

  typedef struct mediate_conv_general
  {
    int convolutionType;
    int conversionType;
    int formatType; 
    int n_groundpixel;
    double shift;
    double conc;
    int noheader;
    char inputFile[FILENAME_BUFFER_LENGTH];
    char outputFile[FILENAME_BUFFER_LENGTH];
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char solarRefFile[FILENAME_BUFFER_LENGTH];

  } mediate_conv_general_t;


  /****************************************************/
  /* Slit - mediate_slit_function_t */


  /****************************************************/
  /* Convolution */

  typedef struct mediate_convolution
  {
    mediate_conv_general_t general;
    mediate_slit_function_t conslit;
    mediate_slit_function_t decslit;
    mediate_filter_t lowpass;
    mediate_filter_t highpass;
  } mediate_convolution_t;

  void initializeMediateConvolution(mediate_convolution_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
