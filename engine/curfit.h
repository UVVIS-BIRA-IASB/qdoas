#ifndef CURFIT_H
#define CURFIT_H

#include "comdefs.h"

double Fchisq(int mode,int nFree,double *Y,double *Yfit,double *sigmay,int nY);

RC Curfit(int     mode,                                                         // I   method of weighting least-squares fit
          int niter,                                                            // current number of iterations
          int     nFree,                                                        // I   the number of degrees of freedom
          double *specX,                                                        // I   the spectrum to evaluate
          double *srefX,                                                        // I   the control spectrum (also called reference spectrum)
          double *Y,                                                            // I   the data to fit (vector of zeros to fit the residual)
          double *sigmaY,                                                       // I   standard deviations for Y data points
          int     nY,                                                           // I   number of data points in Y
          double *P,                                                            // I/O parameters fitted linearly in the fitting function (ANALYSE_Function)
          double *A,                                                            // I/O parameters to fit non linearly in this function
          double *deltaA,                                                       // I   increments for non linear parameters in A
          double *sigmaA,                                                       // O   standard deviations for non linear parameters in A
          double *minA,                                                         // I   lower limit for fitted parameters in A
          double *maxA,                                                         // I   upper limit for fitted parameters in A
          int     nA,                                                           // I   number of parameters in A
          double *Yfit,                                                         // O   vector of calculated values of Y
          double *pLambda,                                                      // O   proportion of gradient search included
          double *pChisqr,                                                      // O   reduced Chi square for fit ( output )
          INDEX   indexFenoColumn,
          struct fit_properties *fitprops);

#endif
