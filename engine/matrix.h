#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

#include "comdefs.h"
#include "doas.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// Structures definitions
// ----------------------

struct _matrix {
  int      nl,nc;           // resp. numbers of lines and columns of matrix
  INDEX    basel,basec;     // resp. base indexes for lines and columns in matrix
  double **matrix,          // pointer to columns in the matrix
         **deriv2;          // pointer to second derivatives
};

// Prototypes
// ----------

void MATRIX_PassCommentLines(FILE *fp);
int  MATRIX_GetColumnsNumbers(FILE *fp,double *pFirstValue);
RC   MATRIX_Allocate(MATRIX_OBJECT *pMatrix,int nl,int nc,int basel,int basec,int allocateDeriv2, const char *callingFunction);
void MATRIX_Free(MATRIX_OBJECT *pMatrix, const char *callingFunctionShort);
RC   MATRIX_Copy(MATRIX_OBJECT *pTarget,MATRIX_OBJECT *pSource, const char *callingFunction);
RC   MATRIX_Load(const char *fileName,MATRIX_OBJECT *pMatrix,int nl,int nc,double xmin,double xmax,int allocateDeriv2,int reverseFlag, const char *callingFunction);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
