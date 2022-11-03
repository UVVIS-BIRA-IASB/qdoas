#ifndef MATRIX_NETCDF_H
#define MATRIX_NETCDF_H

#include <stdio.h>

#include "matrix.h"
#include "doas.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// Prototypes
// ----------

RC   MATRIX_netcdf_LoadXS(const char *fileName,MATRIX_OBJECT *pMatrix,int nl,int nc,double xmin,double xmax,int allocateDeriv2,int reverseFlag,bool *use_row,const char *callingFunction);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
