#ifndef NELMIN_H
#define NELMIN_H

void nelmin ( double fn (double *,double *,double *,double *,double *,double *,int ,int ,int ,double *,int * ),
              double *lambda,double *ref,double *spec,double *spec2,double *lambdas,double *specInt,int ndet,int imin,int imax,double *shift,int *pRc,
              int n, double start[], double xmin[],double *ynewlo, double reqmin, double step[], int konvge, int kcount,int *icount, int *numres, int *ifault );

#endif
