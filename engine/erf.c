
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  CALCULATION OF THE ERROR FUNCTION
//  Name of module    :  ERF.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in June 97
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
//  This module contains the routines to calculate the error function.  In order
//  to speed up the real-time convolution of cross sections, the error function
//  can be pre-calculated on a given grid and then interpolated when needed.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ERF_GetValue - calculate the error function (if possible, interpolate the
//                 pre-calculated vector);
//
//  ERF_Alloc - allocate vectors in order to save a pre-calculated error function
//  ERF_Free - release the vectors allocated by ERF_Alloc
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <math.h>
#include <stdlib.h>

#include "doas.h"
#include "spline.h"
#include "erf.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define ITMAX         100                                                       // the maximum number of iterations
#define EPS             3.0e-7

#define ERF_N   (double)3.*100.+1.                                              // the size of the grid on which the error function is pre-calculated

// ================
// GLOBAL VARIABLES
// ================

double *ERF_x,*ERF_y,*ERF_y2;                                                   // the pre-calculated vectors (grid, error function and 2nd derivatives)

// ==========================
// ERROR FUNCTION CALCULATION
// ==========================

double gammln ( double xx )
{
    double x, tmp, ser;
    static double cof[6] = { 76.18009173, -86.50532033, 24.01409822,
          -1.231739516, 0.120858003e-2, -0.536382e-5 };
    int j;

    x = xx-1.0;
    tmp=x+5.5;
    tmp -= (x+0.5)*log(tmp);
    ser = 1.0;
    for ( j=0; j<=5; j++ )
       {
         x += 1.0;
         ser += cof[j]/x;
       }

    return ( (double) -tmp+log(2.50662827465*ser) );
}


void gser ( double *gamser, double a, double x, double *gln )
{
    int n;
    double sum, del, ap;

    *gamser=(double)0.0;

    *gln = gammln(a);
    if ( x <= 0.0 )
       {
         if ( x < 0.0 )
          ERROR_SetLast("gser",ERROR_TYPE_WARNING,ERROR_ID_ERF,"x less than 0");
          return;
       }
    else
       {
         ap = a;
         del = sum = 1.0/a;
         for ( n=1; n<=ITMAX; n++ )
            {
              ap += 1.0;
              del *= x/ap;
              sum += del;
              if ( fabs(del) < fabs(sum)*EPS )
                 {
                   *gamser = sum * exp(-x+a*log(x)-(*gln));
                   break;
                 }
            }

          if (n>ITMAX)
           ERROR_SetLast("gser",ERROR_TYPE_WARNING,ERROR_ID_ERF,"a too large, ITMAX too small");
       }
}

void gcf ( double *gammcf, double a, double x, double *gln )
{
    int n;
    double gold=0.0, g, fac=1.0, b1=1.0;
    double b0=0.0, anf, ana, an, a1, a0=1.0;

    *gln = gammln(a);
    *gammcf=(double)0.0;

    a1=x;
    for ( n=1; n<=ITMAX; n++ )
       {
         an = (double) n;
         ana=an-a;
         a0 = (a1+a0*ana)*fac;
         b0 = (b1+b0*ana)*fac;
         anf=an*fac;
         a1=x*a0+anf*a1;
         b1=x*b0+anf*b1;
         if ( a1 )
            {
               fac=1.0/a1;
               g=b1*fac;
               if ( fabs((g-gold)/g) < EPS )
                  {
                    *gammcf = exp(-x+a*log(x)-(*gln))*g;
                    break;
                  }
               gold=g;
            }
       }
          if (n>ITMAX)
           ERROR_SetLast("gcf",ERROR_TYPE_WARNING,ERROR_ID_ERF,"a too large, ITMAX too small");
}


double gammp ( double a, double x )
{
    double gamser, gammcf, gln;

    if ( x < 0.0 || a <= 0.0)
     ERROR_SetLast("gammp",ERROR_TYPE_WARNING,ERROR_ID_ERF,"invalid arguments");
    if ( x < (a+1.0) )
       {
          gser( &gamser, a, x, &gln );
          return gamser;
       }
    else
       {
         gcf (&gammcf, a, x, &gln );
         return 1.0-gammcf;
       }
}

double doas_erf ( double x )
{
    return ( x < 0.0 ) ? -gammp(0.5,x*x) : gammp(0.5,x*x);
}

// -----------------------------------------------------------------------------
// FUNCTION      ERF_GetValue
// -----------------------------------------------------------------------------
// PURPOSE       calculate the error function (if possible, interpolate the
//               pre-calculated vector)
//
// INPUT         newX : the input for the error function
//
// RETURN        the error function calculated (or interpolated) on newX
// -----------------------------------------------------------------------------

double ERF_GetValue(double newX)
 {
  // Declarations

  double absX,newY;

  // Initialization

  absX=fabs(newX);

  // Get value from vector

  if ((ERF_x!=NULL) && (ERF_y!=NULL) && (ERF_y2!=NULL) &&
      (absX>=ERF_x[0]) && (absX<=ERF_x[(int)(ERF_N)-1]) &&
      !SPLINE_Vector(ERF_x,ERF_y,ERF_y2,(int)(ERF_N),&absX,&newY,1,SPLINE_CUBIC))
   {
    if (newX<0)
     newY=-newY;
   }
  else
   newY=doas_erf(newX);

  // Return

  return newY;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ERF_Alloc
// -----------------------------------------------------------------------------
// PURPOSE       allocate vectors in order to save a pre-calculated error function
//
// RETURN        ERROR_ID_ALLOC if the allocation failed
//               the return code
// -----------------------------------------------------------------------------

RC ERF_Alloc(void)
 {
  // Declarations

  INDEX i;
  double x;
  RC rc;

  // Buffers allocation

  if (((ERF_x=MEMORY_AllocDVector("ERF_Alloc","ERF_x",0,(int)(ERF_N)-1))==NULL) ||
      ((ERF_y=MEMORY_AllocDVector("ERF_Alloc","ERF_y",0,(int)(ERF_N)-1))==NULL) ||
      ((ERF_y2=MEMORY_AllocDVector("ERF_Alloc","ERF_y2",0,(int)(ERF_N)-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
   	// Build the grid (0..3 nm by 0.01 step) and calculate the error function

    for (i=0,x=(double)0.;i<(INDEX)(ERF_N);i++,x+=(double)0.01)
     {
      ERF_x[i]=(double)x;
      ERF_y[i]=doas_erf(x);
     }

    // Calculate the second derivatives for future interpolation

    rc=SPLINE_Deriv2(ERF_x,ERF_y,ERF_y2,(int)(ERF_N),"ERF_Alloc");
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ERF_Free
// -----------------------------------------------------------------------------
// PURPOSE       release the vectors allocated by ERF_Alloc
// -----------------------------------------------------------------------------

void ERF_Free(void)
 {
  if (ERF_x!=NULL)
   MEMORY_ReleaseDVector("ERF_Free","ERF_x",ERF_x,0);
  if (ERF_y!=NULL)
   MEMORY_ReleaseDVector("ERF_Free","ERF_y",ERF_y,0);
  if (ERF_y2!=NULL)
   MEMORY_ReleaseDVector("ERF_Free","ERF_y2",ERF_y2,0);
 }
