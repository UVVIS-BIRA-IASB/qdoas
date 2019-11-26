/* Copyright (C) 2017 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "fit_properties.h"

#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------
// FIT_PROPERTIES_free : Release allocated buffers
// ----------------------------------------------------------------------

void FIT_PROPERTIES_free(const char *callingFunctionShort, struct fit_properties *fitprops) {
  char functionNameShort[MAX_STR_SHORT_LEN+1] = {0};

  // Build complete function name

  if (strlen(callingFunctionShort)<=MAX_STR_SHORT_LEN-strlen("FIT_PROPERTIES_free via  "))
   sprintf(functionNameShort,"FIT_PROPERTIES_free via %s ",callingFunctionShort);
  else
    sprintf(functionNameShort,__func__);

  // Release allocated buffers

  LINEAR_free(fitprops->linfit);
  fitprops->linfit=NULL;

  if (fitprops->A!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"A",fitprops->A,0,1);
  if (fitprops->P!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"P",fitprops->P,0,1);
  if (fitprops->SigmaSqr!=NULL)
   MEMORY_ReleaseDVector(functionNameShort,"SigmaSqr",fitprops->SigmaSqr,0);
  if (fitprops->covar!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"covar",fitprops->covar,1,1);
  if (fitprops->specrange !=NULL)
   spectrum_destroy(fitprops->specrange);

  fitprops->A=fitprops->P=fitprops->covar=NULL;
  fitprops->SigmaSqr=NULL;
  fitprops->specrange=NULL;
}

RC FIT_PROPERTIES_alloc(const char *callingFunctionShort,struct fit_properties *fitprops) {

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_MEM);
  #endif

  RC rc=ERROR_ID_NO;

  fitprops->linfit=NULL;

  // Allocation

  if (fitprops->DimC && fitprops->DimL) {
    if (((fitprops->A=(double **)MEMORY_AllocDMatrix(__func__,"A",1,fitprops->DimL,0,fitprops->DimC))==NULL) ||
        ((fitprops->covar=(double **)MEMORY_AllocDMatrix(__func__,"covar",1,fitprops->DimC,1,fitprops->DimC))==NULL) ||
        ((fitprops->SigmaSqr=(double *)MEMORY_AllocDVector(__func__,"SigmaSqr",0,fitprops->DimC))==NULL) ||
        ((fitprops->DimP>0) && ((fitprops->P=(double **)MEMORY_AllocDMatrix(__func__,"P",1,fitprops->DimL,0,fitprops->DimP))==NULL))) {

      rc=ERROR_ID_ALLOC;

    } else {
      for (int i=1;i<=fitprops->DimC;i++) {
        for (int j=1;j<=fitprops->DimC;j++) {
          fitprops->covar[i][j]=0.;
        }
        fitprops->SigmaSqr[i]=0.;
      }

      for (int i=1;i<=fitprops->DimL;i++)
       fitprops->A[0][i]=0.;

      if (fitprops->P!=NULL)
       for (int i=1;i<=fitprops->DimL;i++)
        fitprops->P[0][i]=0.;

      fitprops->SigmaSqr[0]=0.;
     }
   } else {
    // Build complete function name
    char functionNameShort[MAX_STR_SHORT_LEN+1];
    if (strlen(callingFunctionShort)<=MAX_STR_SHORT_LEN-strlen("FIT_PROPERTIES_alloc via  "))
      sprintf(functionNameShort,"FIT_PROPERTIES_alloc via %s ",callingFunctionShort);
    else
      sprintf(functionNameShort,__func__);
    
   rc=ERROR_SetLast(functionNameShort,ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"DimC or DimL is zero !");
  }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  return rc;
}
