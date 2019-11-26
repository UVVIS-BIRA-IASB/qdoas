#ifndef SPLINE_H
#define SPLINE_H

#include "comdefs.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif
  
  enum _spline
    {
      SPLINE_LINEAR,                                                             // linear interpolation
      SPLINE_CUBIC,                                                             // spline interpolation
      SPLINE_MAX
    };
  
  RC SPLINE_Deriv2(const double *X, const double *Y, double *Y2,int n, const char *callingFunction);
  RC SPLINE_Vector(const double *xa, const double *ya, const double *y2a,int na, const double *xb,double *yb,int nb,int type);
  
#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
