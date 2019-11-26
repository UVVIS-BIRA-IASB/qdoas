/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
