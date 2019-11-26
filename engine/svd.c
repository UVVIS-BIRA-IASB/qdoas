/* Copyright (C) 2007-2017 Royal Belgian Institute for Space Aeronomy
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

//  MODULE DESCRIPTION :
//
//  We use SVD for solving linear part of the Beer-Lambert's law.
//
//  SVD is adapted for solving sets of M equations and N unknows with M>>N.  It's
//  based on the following therorem of linear algebra :
//
//  any M x N matrix A whose number of rows M is greater than or equal to its
//  number of columns N, can be written as the product of :
//
//     . a M x N column-orthogonal matrix U,
//     . a N x N diagonal matrix W with positive or zero elements and
//     . V', the transpose of an N x N orthogonal matrix V.
//
//  Given the definition of diagonal and orthogonal matrices, A(-1) the inverse
//  of A can be written as the product of :
//
//     . the orthogonal matrix V,
//     . a N x N diagonal matrix W(-1) whose diagonal elements are the inverse
//       of diagonal elements of W and
//     . U', the transpose of the matrix U.
//
//  A set of equations :
//
//       A.x=b
//
//  can then be solved as following :
//
//       x=A(-1).b=V.W(-1).U'.
//
//  Algorithms are retrieved from the 'Numerical recipes in C', so report to this
//  reference for further information on the method.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SVD_Bksb - SVD back substitution.  This function solves A.x=b for a vector
//             x using the singular value decomposition of A;
//
//  SVD_Dcmp - given a matrix a[1..m][1..n], this routine computes its singular
//             value decomposition A=U.W.V'.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "doas.h"
#include "svd.h"

// ===========
// INLINE CODE
// ===========

#define   PYTHAG(a,b)   ( (at=fabs(a)) > (bt=fabs(b)) ? (ct=bt/at,at*sqrt(1.0+ct*ct)) : ( (bt!=0.0) ? (ct=at/bt,bt*sqrt(1.0+ct*ct)) : 0.0 ) )
#define   SMAX(a,b)     ( maxarg1=(a), maxarg2=(b), ( (maxarg1) > (maxarg2) ) ? (maxarg1) : (maxarg2) )
#define   SIGN(a,b)     ( ( (b) >= 0.0 ) ? fabs(a) : -fabs(a) )

// ====================
// CONSTANTS DEFINITION
// ====================

#define SVD_MAX_ITERATIONS       30                                             // the maximum number of iterations

// ================
// STATIC VARIABLES
// ================

static double at, bt, ct, maxarg1, maxarg2;

// -----------------------------------------------------------------------------
// FUNCTION      SVD_Bksb
// -----------------------------------------------------------------------------
// PURPOSE       SVD back substitution.  This function solves A.x=b for a vector
//               x using the singular value decomposition of A.
//
// INPUT         u,w,v : matrices resulting of the SVD of A;
//               m, n  : dimensions of previous matrices;
//                       u[1..m][1..n], w[1..n], v[1..n][1..n];
//               b     : the right-hand side vector;
//
// OUTPUT        x     : the solution of the system A.x=b.
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed;
//               ERROR_ID_SVD_ILLCONDITIONNED if the input matrix A is ill-conditionned;
//               ERROR_ID_NO otherwise;
//
// REMARKS       no input quantities are destroyed, so the function may be called
//               sequentially with different b's.
// -----------------------------------------------------------------------------

RC SVD_Bksb(const struct svd *svd, int m, int n, const double *b, double *x) {

  // Declarations

  int i, j;
  double s, *tmp;
  RC rc;

  double **u = svd->U;
  const double * const w = svd->W;
  double **v = svd->V;

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
  DEBUG_FunctionBegin("SVD_Bksb",DEBUG_FCTTYPE_MATH);
  DEBUG_PrintVar("Ax=b -> b",b,1,m,NULL);
  #endif

  // Initialization

  rc=ERROR_ID_NO;

  // Temporary buffer allocation

  if ((tmp=MEMORY_AllocDVector("SVD_Bksb","tmp",1,n))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // SVD backsubstitution

    for ( j=1; j<=n; j++ )                                 /*  Calculate u'b  */
        {
         tmp[j]=(double)0.;
         s=(double)0.;

         if ( fabs(w[j]) < 1.e-12 )
           {
            rc=ERROR_SetLast("SVD_Bksb",ERROR_TYPE_WARNING,ERROR_ID_SVD_ILLCONDITIONED);
            goto EndSVD_Bksb;
           }
          else
             {
               for ( i=1; i<=m; i++ ) s += ( u[j][i] * b[i] );
               s /= w[j];
             }

          tmp[j] = s;
        }


    for ( i=1; i<=n; i++ )            /*  Multiply matrix by v to get answer  */
        {
          s = 0.0;
          for ( j=1; j<=n; j++ ) s += ( v[j][i] * tmp[j] );
          x[i] = s;
        }
   }

  EndSVD_Bksb :

  // Release allocated buffer

  if (tmp!=NULL)
   MEMORY_ReleaseDVector("SVD_Bksb","tmp",tmp,1);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
  DEBUG_PrintVar("Ax=b -> x",x,1,n,NULL);
  DEBUG_FunctionStop("SVD_Bksb",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SVD_Dcmp
// -----------------------------------------------------------------------------
// PURPOSE       given a matrix a[1..n][1..m], this routine calculates its singular
//               value decomposition A=U.W.V'.
//
// INPUT         a     : the matrix to decompose;
//               m, n  : dimensions of the previous matrix;
//
//               the first index counts the columns of a matrix, second
//               index the rows, i.e.:
//
//               A = U*W*V' <=>
//               A[j][i] = sum(k) U[k][i] * W[k] * V[k][j]               
//
// OUTPUT        the matrix U replaces a on output;
//               the diagonal matrix of singular values is output as a vector w[1..n];
//               the matrix V (not V') is output as v[1..n][1..n]
//               SigmaSqr is the vector of variances;
//               covar is the matrix of covariances.
//
// RETURN        ERROR_ID_SVD_ARG if m<n;
//               ERROR_ID_ALLOC if buffer allocation failed;
//               ERROR_ID_CONVERGENCE if the algorigthm doesn't converge;
//               ERROR_ID_NO otherwise;
// -----------------------------------------------------------------------------

RC SVD_Dcmp (struct svd *svd, int m, int n, double *SigmaSqr, double **covar) {
  double **a = svd->U;
  double *w = svd->W;
  double **v = svd->V;

  int flag, i, its, j, jj, k, l, nm;
  double c, f, g, h, s, x, y, z, anorm, scale, *rv1, *wti;

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_MATH);
  DEBUG_PrintVar("Matrix A before the SVD decomposition",a,1,m,1,n,NULL);
  #endif

  // Initializations

  rv1=NULL;
  RC rc=ERROR_ID_NO;
  l=nm=0;

  // Check for parameters

  if ( m < n )
   rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_SVD_ARG,m,n);

  // Temporary buffer allocation

  else if ((rv1=(double *)MEMORY_AllocDVector(__func__,"rv1",1,n))==NULL)
   rc=ERROR_ID_ALLOC;

  else
   {
    anorm = g = scale = (double) 0.0;

    // Householder reduction to bidiagonal form

    for ( i=1; i<=n; i++ )
        {
          l = i + 1;
          rv1[i] = scale * g;
          g = s = scale = (double) 0.0;

          if ( i <= m )
             {
               for ( k=i; k<=m; k++ ) scale += fabs( a[i][k] );
               if ( scale != (double) 0. )
                  {
                    const double divscale = 1/scale;
                    for ( k=i; k<=m; k++ )
                        {
                          a[i][k] *= divscale;
                          s       += a[i][k] * a[i][k];
                        }

                    f       = a[i][i];
                    g       = - SIGN ( sqrt(s), f );
                    h       = f*g - s;
                    a[i][i] = f-g;

                    if ( i != n )
                       {
                         for ( j=l; j<=n; j++ )
                             {
                               for ( s=0.0, k=i; k<=m; k++ ) s += a[i][k] * a[j][k];
                               f = ( double ) s / h;
                               for ( k=i; k<=m; k++ ) a[j][k] += f * a[i][k];
                             }
                       }
                    for ( k=i; k<=m; k++ ) a[i][k] *= (double) scale;
                  }
             }

          w[i] = scale * g;
          g = s = scale = (double) 0.0;

          if ( (i<=m) && (i!=n) )
             {
               for ( k=l; k<=n; k++ ) scale += (double) fabs ( a[k][i] );

               if ( scale != (double) 0.0 )
                  {
                    const double divscale = 1/scale;
                    for ( k=l; k<=n; k++ )
                        {
                          a[k][i] *= divscale;
                          s       += a[k][i] * a[k][i];
                        }

                    f       = a[l][i];
                    g       = - SIGN ( sqrt(s), f );
                    h       = f*g - s;
                    a[l][i] = f - g;

                    for ( k=l; k<=n; k++ ) rv1[k] = a[k][i] / h;

                    if ( i != m )
                       {
                         for ( j=l; j<=m; j++ )
                             {
                               for ( s=0.0, k=l; k<=n; k++ ) s += a[k][j] * a[k][i];
                               for ( k=l; k<=n; k++ ) a[k][j] += s * rv1[k];
                             }
                       }
                    for ( k=l; k<=n; k++ ) a[k][i] *= scale;
                  }
             }
          anorm = SMAX ( anorm, (fabs(w[i])+fabs(rv1[i])) );
        }

    // Accumulation of right-hand transformations

    for ( i=n; i>=1; i-- )
        {
          if ( i < n )
             {
               if ( g != (double) 0.0 )
                  {
                    for ( j=l; j<=n; j++ ) /*  double div to avoid underflow  */
                          v[i][j] = (a[j][i]/a[l][i]) / g;

                    for ( j=l; j<=n; j++ )
                        {
                          for ( s=0.0, k=l; k<=n; k++ ) s += a[k][i] * v[j][k];
                          for ( k=l; k<=n; k++ ) v[j][k] += s * v[i][k];
                        }
                  }

               for ( j=l; j<=n; j++ ) v[j][i] = v[i][j] = (double) 0.0;
             }

          v[i][i] = (double) 1.0;
          g = rv1[i];
          l = i;
        }

    // Accumulation of left-hand transformations

    for ( i=n; i>=1; i-- )
        {
          l = i + 1;
          g = w[i];

          if ( i < n ) for ( j=l; j<=n; j++ ) a[j][i] = (double) 0.0;

          if ( g != (double) 0.0 )
             {
               g = (double) 1.0 / g;

               if ( i != n )
                  {
                    for ( j=l; j<=n; j++ )
                        {
                          for ( s=0.0, k=l; k<=m; k++ ) s += a[i][k] * a[j][k];
                          f = ( s / a[i][i] ) * g;
                          for ( k=i; k<=m; k++ ) a[j][k] += f * a[i][k];
                        }
                  }

               for ( j=i; j<=m; j++ ) a[i][j] *= g;
             }

          else { for ( j=i; j<=m; j++ ) a[i][j] = (double) 0.0; }

          a[i][i] += (double) 1.;
        }

    // Diagonalisation of the bidiagonal form

    for ( k=n;k>=1; k-- )               /*  BEGIN loop over singular values  */
        {
          for ( its=1; its<=SVD_MAX_ITERATIONS; its++ )   /*  BEGIN loop over allowed iterations */
              {
                flag = 1;

                for ( l=k; l>=1; l-- )                /*  Test for splitting  */
                    {                    /*  Note that rv1[1] is always zero  */
                      nm = l-1;
                      if ( (double) (fabs(rv1[l])+anorm) == anorm ) { flag = 0; break; }
                      if ( (double) (fabs(w [nm])+anorm) == anorm ) break;
                    }

                if ( flag )
                   {
                     c = (double) 0.0;  /*  Cancellation of rv1[l], if l > 1  */
                     s = (double) 1.0;

                     for ( i=l; i<=k; i++ )
                         {
                           f = s * rv1[i];
                           rv1[i] *= c;

                           if ( (double) (fabs(f)+anorm) == anorm ) break;

                           g    = (double) w[i];
                           h    = (double) PYTHAG ( f, g );
                           w[i] = (double) h;
                           h    = (double) 1.0 / h;
                           c    = (double) g * h;
                           s    = (double) -f * h;

                           for ( j=1; j<=m; j++ )
                               {
                                 y = a[nm][j];
                                 z = a[i][j];

                                 a[nm][j] = y*c + z*s;
                                 a[i][j]  = z*c - y*s;
                               }
                         }                            /*  END for ( i=l; ...  */
                   }                                     /*  END if ( flag )  */

                z = w[k];

                if ( l== k )         /*  Convergence                          */
                   {                 /*  Singular value is made non negative  */
                     if ( z < 0.0 )
                        {
                          w[k] = -z;
                          for ( j=1; j<=n; j++ ) v[k][j] = (-v[k][j]);
                        }
                     break;
                   }

                if ( its == SVD_MAX_ITERATIONS )
                 {
                  rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_CONVERGENCE,SVD_MAX_ITERATIONS);
                  goto EndSVD_Dcmp;
                 }

                x  = w[l];
                nm = k - 1;
                y  = w[nm];
                g  = rv1[nm];
                h  = rv1[k];
                f  = ((y-z)*(y+z)+(g-h)*(g+h)) / (2.0*h*y);
                g  = PYTHAG ( f, 1.0 );
                f  = ((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h)) / x;

                /*  Next QR transformation  */

                c = s = (double) 1.0;

                for ( j=l; j<=nm; j++ )
                    {
                      i = j + 1;
                      g = rv1[i];
                      y = w[i];
                      h = s * g;
                      g = c * g;
                      z = PYTHAG ( f, h );

                      rv1[j] = z;

                      c = f / z;
                      s = h / z;
                      f = x*c + g*s;
                      g = g*c - x*s;
                      h = y * s;
                      y = y * c;

                      for ( jj=1; jj<=n; jj++ )
                          {
                            x = v[j][jj];
                            z = v[i][jj];

                            v[j][jj] = x*c + z*s;
                            v[i][jj] = z*c - x*s;
                          }

                      w[j] = z = PYTHAG ( f, h );

                      if ( z != (double) 0.0 )
                         {
                           z = (double) 1.0 / z;
                           c = (double) f * z;
                           s = (double) h * z;
                         }

                      f = (c*g) + (s*y);
                      x = (c*y) - (s*g);

                      for ( jj=1; jj<=m; jj++ )
                          {
                            y = a[j][jj];
                            z = a[i][jj];

                            a[j][jj] = y*c + z*s;
                            a[i][jj] = z*c - y*s;
                          }
                    }                                 /*  END for ( j=l; ...  */

                rv1[l] = (double) 0.0;
                rv1[k] = f;
                w  [k] = x;
              }                             /*  END loop over allowed iteraø  */
        }                                  /*  END loop over singular values  */

    // Variance calculation

    wti=rv1;

    for ( i=1; i<=n; i++ )
          wti[i] = ( fabs(w[i]) > (double) 1.e-12 ) ? (double) 1. / ( w[i] * w[i] ) : (double) 0.;

    if (SigmaSqr!=NULL) {
      for ( j=1, SigmaSqr[0]=0.; j<=n; j++ ) {
        SigmaSqr[j] = 0.;
        for ( k=1; k<=n; k++ )
          SigmaSqr[j] += v[k][j] * v[k][j] * wti[k];
      }
    }
    
    // Covariance calculation
    if (covar!=NULL)
     {
      for (i=1;i<=n;i++)
       for (j=1;j<=i;j++)
        {
         covar[j][i]=(double)0.;
         for (k=1;k<=n;k++)
          covar[j][i]+=v[k][j]*v[k][i]*wti[k];
         covar[i][j]=covar[j][i];
        }
     }
   }

  EndSVD_Dcmp :

  // Release allocated buffer

  if (rv1!=NULL)
   MEMORY_ReleaseDVector("SVD_Dcmp ","rv1",rv1,1);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
  DEBUG_PrintVar("Matrix A after the SVD decomposition",a,1,m,1,n,NULL);
  DEBUG_PrintVar("Other matrix and vectors produced after the SVD decomposition",
                  w,1,n,          // W        (1:n)
                  v,1,n,1,n,      // V        (1:n,1:n)
                  SigmaSqr,1,n,   // SigmaSqr (1:n)
                  covar,1,n,1,n,  // Covar    (1:n,1:n)
                  NULL);
  DEBUG_FunctionStop(__func__,rc);
  #endif

  // Return

  return rc;
 }
