
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  INTERPOLATION FUNCTIONS
//  Name of module    :  SPLINE.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in June 97
//
//  Reference         :  Numerical recipes in C
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  This module provides functions for interpolation of tabulated functions.
//  Linear and cubic spline are supported.  In the second case, the second
//  derivatives of the function must be pre-calculated.
//
//  See Numerical Recipes for further description or comment.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SPLINE_Deriv2  calculates the second derivatives needed for cubic spline interpolation
//  SPLINE_Vector  function for linear and cubic interpolation;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "spline.h"
#include "doas.h"

// -----------------------------------------------------------------------------
// FUNCTION      SPLINE_Deriv2
// -----------------------------------------------------------------------------
// PURPOSE       This function calculates the second derivatives needed for
//               cubic spline interpolation
//
// INPUT         X,Y               vectors defining the tabulated function Y=f(X);
//               n                 the size of previous vectors;
//               callingFunction   the name of the calling function for error
//                                 message if any;
//
// OUTPUT        Y2  second derivatives needed for evaluating the spline function
//
// RETURN        ERROR_ID_ALLOC if the allocation of a temporary buffer failed;
//               ERROR_ID_SPLINE if non increasing absissae are provided;
//               ERROR_ID_NO for success.
// -----------------------------------------------------------------------------

RC SPLINE_Deriv2(const double *X, const double *Y,double *Y2,int n,const char *callingFunction)
{
  // Declarations

  int i, k;
  double qn, un, *u, yp1,ypn;
  RC rc;

  // Debugging

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
  DEBUG_FunctionBegin("SPLINE_Deriv2",DEBUG_FCTTYPE_MATH|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  yp1=ypn=(double)1.e30;                                                        // derivatives at boundaries are assumed unknown
  rc=ERROR_ID_NO;

  // Temporary buffer allocation
  if ((u=(double *)MEMORY_AllocDVector("SPLINE_Deriv2","u",0,n-1))==NULL){
   rc=ERROR_ID_ALLOC;
   }
  else
   {
    // consider lower boundary :

    if (yp1>(double)0.99e30)                                                    // The lower boundary condition is
     Y2[0]=u[0]=(double)0.;                                                     // set to be "natural" else to have
    else if (X[1]-X[0]<=(double)0.)
     rc=ERROR_SetLast(callingFunction,ERROR_TYPE_WARNING,ERROR_ID_SPLINE);
    else
     {
      Y2[0] = (double) -0.5;
      u [0] = (double) (3.0/(X[1]-X[0])) * ((Y[1]-Y[0])/(X[1]-X[0])-yp1);
     }

    // decomposition loop of the tridiagonal algorithm

    double dx_old=X[1]-X[0];
    double dydx_old = (Y[1]-Y[0])/dx_old;

    for (i=1;(i<n-1)&&!rc;i++)

     if (X[i+1]-X[i]<=0)
      rc=ERROR_SetLast(callingFunction,ERROR_TYPE_WARNING,ERROR_ID_SPLINE,i,i+1,X[i],X[i+1]);
     else {
       const double d2x = 1.0/(X[i+1]-X[i-1]);
       double sig= dx_old*d2x;
       double dp=1.0/(sig*Y2[i-1]+2.);
       Y2[i]=(sig-1.0)*dp;
       const double dx=X[i+1]-X[i];
       const double dydx = (Y[i+1]-Y[i])/dx;
       u[i]=dydx-dydx_old;
       u[i]=(6.0*u[i]*d2x-sig*u[i-1])*dp;

       if (fabs(u[i])<(double)1.e-300)
         u[i]=(double)0.;
       dx_old = dx;
       dydx_old = dydx;
     }

    // consider upper boundary :

    if (!rc)
     {
      if (ypn>0.99e30)
       qn=un=(double)0.;
      else
       {
    qn=(double)0.5;
    un=(double)(3./(X[i]-X[i-1]))*(ypn-(Y[i]-Y[i-1])/(X[i]-X[i-1]));
       }

      Y2[i]=(un-qn*u[i-1])/(qn*Y2[i-1]+1.);

      // backsubstitution loop of the tridiagonal algorithm :

      for (k=i-1;k>=0;k--)
       Y2[k]=Y2[k]*Y2[k+1]+u[k];
     }

    MEMORY_ReleaseDVector("SPLINE_Deriv2","u",u,0);
   }

  // Debugging

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
  if (rc)
   DEBUG_PrintVar("Input vectors",X,0,n-1,Y,0,n-1,NULL);
  DEBUG_FunctionStop("SPLINE_Deriv2",rc);
#endif

  // Return

  return rc;
}


/*! \brief return n such that 2^(n-1) < x <= 2^n

   The ceil of the binary log of x is given by the position of the
   first non-zero bit in the binary representation of x.*/
unsigned int log2_ceil (unsigned long x) {
  if (x <= 1) return 0;
  // number of bits in x, minus the number of leading zeroes: 
  return (8*sizeof(x))-__builtin_clzl(x-1);
}

/*! \brief linear or cubic spline interpolation

  \param[in] xa,ya x and y values of the tabulated function
  xa=f(ya). The xa must be monotonously increasing.

  \param[in] na size of input vectors xa,ya (expects na >=2)

  \param[in] xb x values for which we want to calculate the interpolated y values

  \param[out] yb output y values

  \param[in] nb number of points for which we want to interpolate.

  \param[in] type SPLINE_LINEAR or SPLINE_CUBIC

  \retval ERROR_ID_NO
*/
RC SPLINE_Vector(const double *restrict xa, const double *restrict ya, const double *restrict y2a,int na, const double *restrict xb, double *restrict yb,int nb,int type)
{
  // input vector must contain at least 2 elements for interpolation to work.
  assert(na >= 2);

  // if na is a power of 2 we need exactly log_2(na) steps to find
  // xlo.  if na is not a power of 2, the number of steps is log_2
  // of the next power of 2.
  const unsigned int num_steps = log2_ceil(na);

  // Browse new absissae
  for (int i=0; i<nb; ++i) {
    const double x=xb[i];

    // new absissae is out of boundaries
    if (x<=xa[0]) {
      yb[i]=ya[0];
      continue;
    } else if (x>=xa[na-1]) {
      yb[i]=ya[na-1];
      continue;
    }

    // binary search for xlo such that *xlo < x <= xlo[1],
    const double *xlo=xa;

    // special case for the first step, in case na is not a power of 2:

    // 1. find the largest power of 2 which is smaller than 'na'
    size_t size = (1ul << (num_steps - 1) );

    // 2. reduce the search to a search in a sequence of length 'size'
    double start = xa[na - size];
    if (start < x) // if mid < x, look in the sequence (start, start+size[
      xlo += na-size;
    // else: look in (xa, xa+size[

    // 3. we now search in a sequence of 'size' elements, where 'size'
    // is a power of 2.  'size' is halved in each iteration.
    for (unsigned j=num_steps-1; j != 0; --j) {
      size/=2;
      double mid = xlo[size];
      if (mid < x)
        xlo += size;
    }

    int k = xlo-xa;
    double xhi=xlo[1];
    double h=xhi-*xlo;
    
    // get ratios        
    double a = (xhi-x)/h;
    double b = 1.-a; // (x-(*xlo) )/h;
      
    // interpolation
       
    yb[i] = a*ya[k]+b*ya[k+1]; // assume type == SPLINE_LINEAR or SPLINE_CUBIC
    if (type==SPLINE_CUBIC)
      yb[i] += ((a*a*a-a)*y2a[k]+(b*b*b-b)*y2a[k+1])*(h*h)/6.;
  }

  return ERROR_ID_NO;
}
