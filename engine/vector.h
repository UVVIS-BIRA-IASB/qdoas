#ifndef VECTOR_H
#define VECTOR_H

#include "comdefs.h"

void   VECTOR_Copy(double *target, const double *source, int dim);
int    VECTOR_Equal(const double *vector1, const double *vector2,int dim,double error);
double VECTOR_Max(double *vector,int dim);
double VECTOR_Min(double *vector,int dim);
RC     VECTOR_Log(double *out,double *in,int dim,const char *callingFunction);
int    VECTOR_LocGt(double *vector,double value,int dim);
void   VECTOR_Invert(double *vector,int dim);
double VECTOR_Table2_Index1(double **Table,int Nx,int Ny,double X,double Y);
double VECTOR_Table2(double **Table,int Nx,int Ny,double X,double Y);
double VECTOR_Norm(const double *v,int dim);
RC     VECTOR_NormalizeVector(double *v,int dim,double *fact,const char *function);


// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Init
// -----------------------------------------------------------------------------
// PURPOSE       initialization of all vector components to one constant value
//
// INPUT         value  : the constant value to use for initializing the vector;
//               dim    : the size of vector to initialize;
//
// OUTPUT        vector : the content of this vector is modified by the function.
// -----------------------------------------------------------------------------

inline void VECTOR_Init (double *vector, double value, int dim ) {
  for (int i=0; i<dim; vector[i++]=value );
}



#endif
