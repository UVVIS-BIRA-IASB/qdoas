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
