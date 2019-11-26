#ifndef VECTOR_H
#define VECTOR_H

#include "comdefs.h"

void   VECTOR_Init(double *vector,double value,int dim);
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

#endif
