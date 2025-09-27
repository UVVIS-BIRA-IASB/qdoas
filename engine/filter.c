
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  FILTERING FUNCTIONS
//  Name of module    :  FILTER.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in 97.
//
//  References        :  Routines in this module come from different sources
//
//      - numerical recipes;
//      - mathworks;
//      - ...
//
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
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  Filtering functions from different sources
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  Fourier - fast Fourier Transform algorithm
//  realft - fast Fourier Transform of a real vector
//  ModBessel - modified Bessel function of zeroth order
//  Neq_Ripple - build a Kaizer filter
//  FilterPinv - pseudoInverse function function
//  FilterSavitskyGolay - build a Savitsky-Golay filter function
//  FilterPascalTriangle - build a Pascal binomial filter function
//  FILTER_Build - build a filter function
//  FilterConv - apply a filter function on a vector by convolution on pixels
//  FILTER_Vector - apply a filter function on a vector
//
//  ----------------------------------------------------------------------------

// ===============
// INCLUDE HEADERS
// ===============

#include <string.h>
#include <math.h>

#include "filter.h"
#include "engine_context.h"
#include "linear_system.h"

#include "vector.h"
#include "spline.h"

#define   MAXNP            150
#define   EPS     (double)   2.2204e-016

// -----------------------------------------------------
// FILTER_OddEvenCorrection : Odd/Even pixels correction
// -----------------------------------------------------

RC FILTER_OddEvenCorrection(double *lambdaData,double *specData,double *output,int vectorSize)
 {
  // Declarations

  double *lambda,*spectrum,*spectrum2,*spec1,*spec2;
  INDEX i;
  RC rc;

  // Buffers allocation

  spectrum=spectrum2=spec1=spec2=NULL;

  if (((lambda=(double *)MEMORY_AllocDVector("FILTER_OddEvenCorrection ","lambda",0,vectorSize-1))==NULL) ||
      ((spectrum=(double *)MEMORY_AllocDVector("FILTER_OddEvenCorrection ","spectrum",0,vectorSize-1))==NULL) ||
      ((spectrum2=(double *)MEMORY_AllocDVector("FILTER_OddEvenCorrection ","spectrum2",0,vectorSize-1))==NULL) ||
      ((spec1=(double *)MEMORY_AllocDVector("FILTER_OddEvenCorrection ","spec1",0,vectorSize-1))==NULL) ||
      ((spec2=(double *)MEMORY_AllocDVector("FILTER_OddEvenCorrection ","spec2",0,vectorSize-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    for (i=0;i<vectorSize/2;i++)
     {
      lambda[i]=lambdaData[(i<<1)];                  // odd pixels (0 based)
      lambda[vectorSize/2+i]=lambdaData[(i<<1)+1];         // even pixels

      spectrum[i]=specData[(i<<1)];                         // odd pixels (0 based)
      spectrum[vectorSize/2+i]=specData[(i<<1)+1];                // even pixels
     }

    if (!(rc=SPLINE_Deriv2(lambda,spectrum,spectrum2,vectorSize/2,"PDA_OddEvenCorrection ")) &&
        !(rc=SPLINE_Deriv2(lambda+(vectorSize/2),spectrum+(vectorSize/2),spectrum2+(vectorSize/2),vectorSize/2,"PDA_OddEvenCorrection (2) ")))
     {
      VECTOR_Copy(spec1,specData,vectorSize);
      VECTOR_Copy(spec2,specData,vectorSize);

      for (i=0;i<vectorSize/2;i++)

       if (((rc=SPLINE_Vector(lambda+(vectorSize/2),spectrum+(vectorSize/2),spectrum2+(vectorSize/2),vectorSize/2,&lambdaData[(i<<1)],&spec1[(i<<1)],1,SPLINE_CUBIC))!=0) ||
           ((rc=SPLINE_Vector(lambda,spectrum,spectrum2,vectorSize/2,&lambdaData[(i<<1)+1],&spec2[(i<<1)+1],1,SPLINE_CUBIC))!=0))

        break;

       else
        {
         output[(i<<1)]=(double)0.5*(spec1[(i<<1)]+spec2[(i<<1)]);
         output[(i<<1)+1]=(double)0.5*(spec1[(i<<1)+1]+spec2[(i<<1)+1]);
        }
     }
   }

  // Release allocated buffers

  if (lambda!=NULL)
   MEMORY_ReleaseDVector("FILTER_OddEvenCorrection ","lambda",lambda,0);
  if (spectrum!=NULL)
   MEMORY_ReleaseDVector("FILTER_OddEvenCorrection ","spectrum",spectrum,0);
  if (spectrum2!=NULL)
   MEMORY_ReleaseDVector("FILTER_OddEvenCorrection ","spectrum2",spectrum2,0);
  if (spec1!=NULL)
   MEMORY_ReleaseDVector("FILTER_OddEvenCorrection ","spec1",spec1,0);
  if (spec2!=NULL)
   MEMORY_ReleaseDVector("FILTER_OddEvenCorrection ","spec2",spec2,0);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      fourier
// -----------------------------------------------------------------------------
// PURPOSE       Fast Fourier Transform algorithm
//
// INPUT/OUTPUT  data  complex array of length nn (ie real array of length 2*nn)
//
// INPUT         nn    the size of data (should be an integer power of 2)
//
//               is    +1 to replace data by its discrete Fourier transform
//                     -1 to replace data by its inverse discrete FT
// -----------------------------------------------------------------------------

void fourier(double *data,int nn,int is)
 {
     // Declarations

  double wtemp,theta,wr,wi,wpr,wpi,tempr,tempi;
  int    j,n,i,m,nmax,istep;
  // Bit-reversal section

  n=2*nn;
  j=1;

  for (i=1;i<=n;i+=2)
   {
    if (j>i)
     {
      tempr=data[j];
      tempi=data[j+1];
      data[j]=data[i];
      data[j+1]=data[i+1];
      data[i]=tempr;
      data[i+1]=tempi;
     }

    m=n/2;

    while ((m>=2) && (j>m))
     {
      j=j-m;
      m=m/2;
     }

    j=j+m;
   }

  nmax=2;

  while (n>nmax)
   {
    istep=2*nmax;
    theta=PI2/(is*nmax);
    wpr=-(double)2.*(sin((double)0.5*theta))*(sin((double)0.5*theta));
    wpi=(double)sin(theta);
    wr=(double)1.0;
    wi=(double)0.0;

    for (m=1;m<=nmax;m+=2)
     {
      for (i=m;i<=n;i+=istep)
       {
        j=i+nmax;
        tempr=wr*data[j]-wi*data[j+1];
        tempi=wr*data[j+1]+wi*data[j];
        data[j]=data[i]-tempr;
        data[j+1]=data[i+1]-tempi;
        data[i]=data[i]+tempr;
        data[i+1]=data[i+1]+tempi;
       }
      wtemp=wr;
      wr=wr*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
     }

    nmax=istep;
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      realft
// -----------------------------------------------------------------------------
// PURPOSE       Fast Fourier Transform of a real vector
//
// INPUT         source  input real data vector
//               nn      the size the input vector (should be a power of 2)
//               is      +1 to calculate the discrete Fourier transform
//                       -1 to calculate its inverse discrete Fourier transform
//
// OUTPUT        buffer  the result of the fourier tranform of input vector
// -----------------------------------------------------------------------------

void realft(double *source,double *buffer,int nn,int is)
 {
     // Declarations

  double wr,wi,wpr,wpi,wtemp,theta;
  double c1,c2,h1r,h1i,h2r,h2i,wrs,wis;
  int index,index1,index2,index3,index4,n2p3,ndemi;

  // Make a copy of the original data

  VECTOR_Copy(buffer+1,source+1,nn);

  // Initializations

  ndemi=nn/2;
  theta=DOAS_PI/(double)ndemi;
  c1=0.5;

  // Forward transform

  if (is==1)
   {
    c2=-0.5;
    fourier(buffer,ndemi,is);
   }

  // Set up for an inverse transform
  else
   {
    c2=0.5;
    theta=-theta;
   }

  wpr=-2.0*pow(sin(0.5*theta),(double)2);
  wpi=sin(theta);
  wr=1.0+wpr;
  wi=wpi;
  n2p3=2*ndemi+3;

  for (index=2;index<=ndemi/2;index++)                                          // case index=1 is done separately
   {
    index1=2*index-1;
    index2=index1+1;
    index3=n2p3-index2;
    index4=index3+1;
    wrs=(double)wr;
    wis=(double)wi;
    h1r=c1*(buffer[index1]+buffer[index3]);                                     // the two separate transforms are separated out of data
    h1i=c1*(buffer[index2]-buffer[index4]);
    h2r=-c2*(buffer[index2]+buffer[index4]);
    h2i=c2*(buffer[index1]-buffer[index3]);
    buffer[index1]=h1r+wrs*h2r-wis*h2i;                                         // here they are recombined to form the true transform of the original real data
    buffer[index2]=h1i+wrs*h2i+wis*h2r;
    buffer[index3]=h1r-wrs*h2r+wis*h2i;
    buffer[index4]=-h1i+wrs*h2i+wis*h2r;
    wtemp=wr;                                                                   // recurrence
    wr=wr*wpr-wi*wpi+wr;
    wi=wi*wpr+wtemp*wpi+wi;
   }

  if (is==1)
   {
    h1r=buffer[1];
    buffer[1]=h1r+buffer[2];
    buffer[2]=h1r-buffer[2];                                                    // squeeze the first and last data together to get them all within the original array
   }
  else
   {
    h1r=buffer[1];
    buffer[1]=c1*(h1r+buffer[2]);
    buffer[2]=c1*(h1r-buffer[2]);
    fourier(buffer,ndemi,is);                                                   // inverse fourier transform
    for (index=1;index<=nn;index++)
     buffer[index]/=ndemi;
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      ModBessel
// -----------------------------------------------------------------------------
// PURPOSE       Evaluates the modified Bessel function of zeroth order
//               at real values of the arguments
//
// INPUT         X       input argument
//
// RETURN        modified Bessel function calculated at X
// -----------------------------------------------------------------------------

double ModBessel(double X)
 {
     // Declarations

  double S, Ds;
  int D;

  // Initialization

  S=Ds=(double)1.;
  D=0;

  // Modified Bessel function

  do
   {
    D+=2;
    Ds*=(double)X*X/(D*D);
    S+=Ds;
   }
  while (Ds>(double)0.2e-8*S);

  // Return

  return ((double)S);
 }

// -----------------------------------------------------------------------------
// FUNCTION      Neq_Ripple
// -----------------------------------------------------------------------------
// PURPOSE       build a Kaizer filter function
//
//               Calculates the coefficients of a nearly equiripple linear
//               phase smoothing filter with an odd number of terms and
//               even symetry
//
// INPUT         Beta        cutoff frequency
//               Delta       pass band
//               dB          tolerance
//
// OUTPUT        pFilter     pointer to the buffer for the calculated filter
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,
//               ERROR_ID_BAD_ARGUMENTS in case of wrong arguments
//               0 on success
//
// Reference: Kaizer and Reed, Rev. Sci. Instrum., 48, 1447-1457, 1977.
// -----------------------------------------------------------------------------

RC FilterNeqRipple (PRJCT_FILTER *pFilter,double *Beta, double *Delta,double *dB)
 {
     // Declarations

  int i, j, k,Nterm;
  double Kf, Eta, Be1, Be2, Gk, Dk, Geta;
  double Lam21, Pow1;
  double DNp;
  double sum;
  RC rc;

  // Initializations

  sum=(double)0.;
  rc=ERROR_ID_NO;

  Kf = (double) 1.8445;
  if ( (*dB) >= (double) 21. ) Kf = (double) 0.13927 * ((*dB)-7.95);

  Lam21 = (*dB) - (double) 21.;
  Pow1 = pow ( (double) Lam21, (double) 0.4 );
  Eta = 0.58417 * Pow1 + 0.07886 * Lam21;

  if ( (*dB) < (double) 21. ) Eta = (double) 0.;
  if ( (*dB) > (double) 50. ) Eta = 0.1102 * ((*dB) - (double) 8.7 );

  Nterm = (int)(Kf/(2.*(*Delta))+0.75);
  pFilter->filterSize = Nterm+1;

  DNp = (double) Nterm;

  pFilter->filterFunction=NULL;

  // Test Np against dimension limit

  if ((Nterm<0) || (Nterm>MAXNP))
   rc=ERROR_SetLast("Neq_Ripple",ERROR_TYPE_WARNING,ERROR_ID_BAD_ARGUMENTS);
  else if ((pFilter->filterFunction=(double *)MEMORY_AllocDVector("Neq_Ripple ","pFilter->filterFunction",1,pFilter->filterSize))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    Be1 = (double) ModBessel ( Eta );

    for ( k=1; k<=Nterm; k++ )
       {
           Dk   = (double) k;
           Gk   = (double) DOAS_PI * Dk;
           Geta = (double) Eta * sqrt ( 1. - pow((double) Dk/DNp, (double) 2.) );

           Be2  = (double) ModBessel ( Geta );
           pFilter->filterFunction[k] = (double) (sin( (*Beta) * Gk )) / Gk * (Be2/Be1);
       }

    pFilter->filterFunction[Nterm] *= (double) 0.5;

    for ( i=2; i<=pFilter->filterSize; i++ )
       {
          k = pFilter->filterSize - i + 2;
          j = k-1;
          pFilter->filterFunction[k] = pFilter->filterFunction[j];
          sum+=pFilter->filterFunction[k];
       }

    pFilter->filterFunction[1] = (*Beta);

    sum=2.*sum+pFilter->filterFunction[1];

    // function normalization by its integral

    if (sum==(double)0.)
     rc=ERROR_SetLast("Neq_Ripple",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0);
    else
     {
      for (i=1;i<=pFilter->filterSize;i++)
       pFilter->filterFunction[i]/=sum;
     }

   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilterSavitskyGolay
// -----------------------------------------------------------------------------
// PURPOSE       build a Savitsky-Golay filter function
//
// INPUT         filterWidth filter width
//               filterOrder filter order
//
// OUTPUT        pFilter     pointer to the buffer for the calculated filter
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed, 0 on success
// -----------------------------------------------------------------------------

RC FilterSavitskyGolay(PRJCT_FILTER *pFilter,int filterWidth,int filterOrder) {
  int rc=ERROR_ID_NO;
  int lc=(filterWidth-1)/2;
  double sum=(double)0.;

  pFilter->filterSize=lc+1;
  // linear system of "filterwidth" equations and "1+filterOrder" unknowns:
  double **poly_matrix = MEMORY_AllocDMatrix(__func__, "pinv", 1, filterWidth, 1, filterOrder+1);
  // allocate matrix for pseudo-inverse:
  double **pinv = MEMORY_AllocDMatrix(__func__, "pinv", 1, filterOrder+1, 1, filterWidth);
  pFilter->filterFunction=MEMORY_AllocDVector(__func__,"pFilter->filterFunction",1,pFilter->filterSize);

  struct linear_system *filter_system = NULL; // allocated later on

  if (poly_matrix == NULL || pinv == NULL || pFilter->filterFunction == NULL) {
    rc = ERROR_ID_ALLOC;
    goto cleanup;
  }

  // Build the matrix of polynomial components:
  for (int j=0;j<=filterOrder;j++)
    for (int i=-lc;i<=lc;i++)
      poly_matrix[j+1][i+lc+1]=(j!=0)?pow((double)i,(double)j):(double)1.;

  filter_system = LINEAR_from_matrix((const double *const *)poly_matrix, filterWidth, filterOrder+1, DECOMP_SVD);
  if (filter_system == NULL) {
    rc = ERROR_ID_ALLOC;
    goto cleanup;
  }
  rc=LINEAR_decompose(filter_system, NULL, NULL);
  if (rc)
    goto cleanup;

  LINEAR_pinv(filter_system, pinv);

  for (int i=1; i<=pFilter->filterSize; ++i){
    pFilter->filterFunction[i] = pinv[lc+i][1];
    if (i>1)
     sum+=pFilter->filterFunction[i];
  }

  sum=2.*sum+pFilter->filterFunction[1];

      // function normalization by its integral

  if (sum==(double)0.)
   rc=ERROR_SetLast("FilterSavitskyGolay",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0);
  else
   {
    for (int i=1;i<=pFilter->filterSize;i++)
     pFilter->filterFunction[i]/=sum;
   }


 cleanup:
  LINEAR_free(filter_system);
  MEMORY_ReleaseDMatrix(__func__, "poly_matrix", poly_matrix, 1, 1);
  MEMORY_ReleaseDMatrix(__func__, "pinv", pinv, 1, 1);
  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilterPascalTriangle
// -----------------------------------------------------------------------------
// PURPOSE       build a Pascal binomial filter function
//
// INPUT         power            the filter size
// OUTPUT        coefficients     the calculated filter
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed, 0 on success
// -----------------------------------------------------------------------------

RC FilterPascalTriangle(double *coefficients,int power)
 {
  // Declarations

  double *coef;
  INDEX i;
  RC rc;

  // Initializations

  rc=0;

  coefficients[1]=1;

  if (power!=0)
   {
    coefficients[power+1]=(double)1.;

    // Buffer allocation

    if ((coef=MEMORY_AllocDVector("FilterPascalTriangle ","coef",1,power))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      // Apply function recursively

      FilterPascalTriangle(coef,power-1);

      for (i=2;i<=power;i++)
       coefficients[i]=coef[i]+coef[i-1];

      MEMORY_ReleaseDVector("FilterPascalTriangle ","coef",coef,1);
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILTER_Build
// -----------------------------------------------------------------------------
// PURPOSE       build a filter function
//
// INPUT/OUTPUT  pFilter          pointer to user filter options from project properties
// OUTPUT        fa1,fa2,fa3      filter options
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,
//               ERROR_ID_DIVISION_BY_0 if filter is 0 everywhere,
//               return code of the filtering function if any
//               0 on success
// -----------------------------------------------------------------------------

RC FILTER_Build(PRJCT_FILTER *pFilter,double fa1,double fa2,double fa3)
 {
  // Declarations

  int filterType;
  double sum;
  INDEX i;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  filterType=pFilter->type;
  sum=(double)0.;

  // Build filter

  if (filterType==PRJCT_FILTER_TYPE_KAISER)
   rc=FilterNeqRipple(pFilter,&fa1,&fa2,&fa3);
  else if (filterType==PRJCT_FILTER_TYPE_SG)
   rc=FilterSavitskyGolay(pFilter,(int)fa1,(int)fa2);
  else
   {
    switch(filterType)
     {
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_GAUSSIAN :
       pFilter->filterSize=(int)(ceil(2.*fa1)+1.);
      break;
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_BOXCAR :
      case PRJCT_FILTER_TYPE_TRIANGLE :
       pFilter->filterSize=(((int)fa1-1)>>1)+1;
      break;
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_BINOMIAL :
       pFilter->filterSize=(int)fa1;
      break;
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_ODDEVEN :
       pFilter->filterSize=2;
      break;
   // -------------------------------------------------------------------------
     }

    // Buffer allocation

    if ((pFilter->filterFunction=(double *)MEMORY_AllocDVector("FILTER_Build","pFilter->filterFunction",1,pFilter->filterSize))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      if (filterType==PRJCT_FILTER_TYPE_BINOMIAL)
       {
           if (!(rc=FilterPascalTriangle(pFilter->filterFunction,pFilter->filterSize-1)))
            {
          VECTOR_Copy(pFilter->filterFunction+1,pFilter->filterFunction+(pFilter->filterSize>>1)+1,((pFilter->filterSize+1)>>1));
          sum=(double)pow(2.,pFilter->filterSize-1);
         }

        pFilter->filterSize=(pFilter->filterSize+1)>>1;
       }
      else
       {
        for (sum=(double)0.,pFilter->filterFunction[1]=(double)1.,i=1;i<pFilter->filterSize;i++)

         if (filterType==PRJCT_FILTER_TYPE_GAUSSIAN)
          sum+=(pFilter->filterFunction[i+1]=exp((double)-4.*log(2.)*i*i/(fa1*fa1)));
         else if (filterType==PRJCT_FILTER_TYPE_TRIANGLE)
          sum+=(pFilter->filterFunction[i+1]=(double)1.-2.*i/(fa1+1.));
         else if (filterType==PRJCT_FILTER_TYPE_BOXCAR)
          sum+=(pFilter->filterFunction[i+1]=(double)1.);

        sum=2.*sum+pFilter->filterFunction[1];
       }

      // function normalization by its integral

      if (sum==(double)0.)
       rc=ERROR_SetLast("FILTER_Build",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0);
      else
       for (i=1;i<=pFilter->filterSize;i++)
        pFilter->filterFunction[i]/=sum;
     }
   }

  // Effective smoothing width

  if (!rc && (pFilter->type!=PRJCT_FILTER_TYPE_NONE) && (pFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
   {
    for (pFilter->filterEffWidth=(double)0.,i=2;i<=pFilter->filterSize;i++)     // sum of all elements of the filter normalized by the maximum
     pFilter->filterEffWidth+=pFilter->filterFunction[i];

    pFilter->filterEffWidth=(pFilter->filterFunction[1]>0)?(double)1.+2.*pFilter->filterEffWidth/pFilter->filterFunction[1]:1;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilterConv
// -----------------------------------------------------------------------------
// PURPOSE       Apply a filter function on a vector by convolution on pixels
//
// INPUT         pFilter          filter function and options
//               Input            vector to filter
//               Size             the size of input vector
//
// OUTPUT        output           the filtered vector
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,
//               0 on success
// -----------------------------------------------------------------------------

RC FilterConv(PRJCT_FILTER *pFilter,double *Input,double *Output,int Size)
 {
  // Declarations

  int     i,j,k;
  double *ftemp;
  RC      rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Temporary buffer allocation

  if ((ftemp=(double *)MEMORY_AllocDVector("FILTER_Conv ","ftemp",0,Size-1))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Initialize the temporary buffer

    VECTOR_Init(ftemp,(double)0.,Size);

    // Filtering by convolution

    for ( i=0; i<Size; i++ )
      {
        j=-(pFilter->filterSize-1);
        ftemp[i]=(i-j<Size)?Input[(i-j)]*pFilter->filterFunction[-j+1]:(double)0.;
        for (j=j+1;j<pFilter->filterSize;j++)
         {
          k=((i-j<Size)&&(i-j>=0))?i-j:i+j;

          if ((k<Size) && (k>=0))
           ftemp[i]+=Input[k]*pFilter->filterFunction[(j<=0)?-j+1:j+1];
         }
      }

    VECTOR_Copy(Output,ftemp,Size);

    // Release allocated buffer

    MEMORY_ReleaseDVector("FILTER_Conv ","ftemp",ftemp,0);
   }

  // return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILTER_Vector
// -----------------------------------------------------------------------------
// PURPOSE       Apply a filter function on a vector
//
// INPUT         pFilter          filter function and options
//               Input            vector to filter
//               Size             the size of input vector
//               outputType       type of filtering to apply (high or low)
//
// OUTPUT        output           the filtered vector
//               tmpVector       the filtered vector (for high pass vector, filtered vector before subtraction or division)
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,
//               return code of the filtering function if any
//               0 on success
// -----------------------------------------------------------------------------

RC FILTER_Vector(PRJCT_FILTER *pFilter,double *Input,double *Output,double *tmpVector,int Size,int outputType)
 {
  // Declarations

  double *tempVector;
  INDEX i,j;
  RC rc;

  // Initializations

  tempVector=NULL;
  rc=ERROR_ID_NO;

  if (Size>0)
   {
    if (tmpVector!=NULL)
     for (i=0;i<Size;i++)
      tmpVector[i]=(double)0.;

    if ((tempVector=(double *)MEMORY_AllocDVector("FILTER_vector ","tempVector",0,Size-1))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      VECTOR_Copy(tempVector,Input,Size);
      for (i=0;(i<pFilter->filterNTimes) && !rc;i++)
       rc=FilterConv(pFilter,tempVector,tempVector,Size);

      if (i==pFilter->filterNTimes)
       {
        if (tmpVector!=NULL)
         VECTOR_Copy(tmpVector,tempVector,Size);

        if (outputType==PRJCT_FILTER_OUTPUT_LOW)
         VECTOR_Copy(Output,tempVector,Size);
        else if (outputType==PRJCT_FILTER_OUTPUT_HIGH_SUB)
         for (j=0;j<Size;j++)
          Output[j]=Input[j]-tempVector[j];
        else if (outputType==PRJCT_FILTER_OUTPUT_HIGH_DIV)
         for (j=0;j<Size;j++)
          Output[j]=(tempVector[j]!=(double)0.)?Input[j]/tempVector[j]:(double)0.;
       }
     }
   }

  // Release allocated vector

  if (tempVector!=NULL)
   MEMORY_ReleaseDVector("FILTER_vector ","tempVector",tempVector,0);

  // Return

  return rc;
 }

// ------------------------------------
// FILTER_LoadFilter : Load filter data
// ------------------------------------

RC FILTER_LoadFilter(PRJCT_FILTER *pFilter)
 {
  // Declaration

  RC rc;

  // Initializations

  pFilter->filterEffWidth=(double)1.;   // effective smoothing width used for reduce number of independent terms in analysis system
  rc=ERROR_ID_NO;

  // Filter type processing

  switch((int)pFilter->type)
   {
 // ---------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_KAISER :
     rc=FILTER_Build(pFilter,(double)pFilter->kaiserCutoff,(double)pFilter->kaiserPassBand,(double)pFilter->kaiserTolerance);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_GAUSSIAN :
     rc=FILTER_Build(pFilter,(double)pFilter->fwhmWidth,(double)0.,(double)0.);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_TRIANGLE :
    case PRJCT_FILTER_TYPE_BOXCAR :
    case PRJCT_FILTER_TYPE_BINOMIAL :
     rc=FILTER_Build(pFilter,(double)pFilter->filterWidth,(double)0.,(double)0.);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_SG :
     rc=FILTER_Build(pFilter,(double)pFilter->filterWidth,(double)pFilter->filterOrder,(double)0.);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_ODDEVEN :
     pFilter->filterEffWidth=(double)1.9;
    break;
 // ---------------------------------------------------------------------------
   }

  // Return

  return rc;
 }
