
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  LOAD MATRIX FROM FILES
//  Name of module    :  MATRIX.C
//  Creation date     :  June 1997
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
//  This module allows to load data dynamically from files to matrix objects and
//  also pre-calculate second derivatives for future spline interpolation.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  MATRIX_Allocate - allocate buffers for a matrix object;
//  MATRIX_Free - release the buffers allocated for a matrix;
//  MATRIX_Copy - copy the content of a matrix in another one;
//  MATRIX_Load - load a matrix from file
//
//  ----------------------------------------------------------------------------

#include "comdefs.h"

#include "matrix.h"
#include "spline.h"
#include "winfiles.h"
#include "stdfunc.h"

#include <math.h>
#include <string.h>
#include <stdbool.h>

// format string for fscanf: read a single number and skip over any
// following characters that are not part of a number and not a
// newline:
#define NEXT_DOUBLE "%lf%*[^0-9.-+\n]"
#define COMMENT_LINE " %1[*;#]%*[^\n]\n"

int test_matrix=0;

// ==================
// BUFFERS PROCESSING
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Allocate
// -----------------------------------------------------------------------------
// PURPOSE       Allocate buffers for a matrix object
//
// INPUT         nl, nc          : resp. the number of lines and columns of the matrix to allocate
//               basel, basec    : resp. the base indexes for lines and columns of the matrix
//               allocateDeriv2  : 1 to allocate buffers for the second derivatives
//               callingFunction : the name of the calling function
//
// OUTPUT        pMatrix, structure to receive the information on the new allocated object
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MATRIX_Allocate(MATRIX_OBJECT *pMatrix,int nl,int nc,int basel,int basec,int allocateDeriv2, const char *callingFunction)
 {
  // Declarations

  char functionName[MAX_FCT_LEN+1];                                            // the name of the calling function
  RC rc;                                                                        // the return code

  // Initializations

  memset(functionName,0,MAX_FCT_LEN+1);
  rc=ERROR_ID_NO;

  // Build the complete function name

  if (strlen(callingFunction)<=MAX_FCT_LEN-strlen("MATRIX_Allocate via "))
   sprintf(functionName,"MATRIX_Allocate via %s",callingFunction);
  else
   sprintf(functionName,"MATRIX_Allocate");

  // Reset the matrix object

  MATRIX_Free(pMatrix,functionName);

  // Allocate bufferS for the matrix and the second derivatives

  if (((pMatrix->matrix=MEMORY_AllocDMatrix(functionName,"matrix",basel,nl+basel-1,basec,nc+basec-1))==NULL) ||
       (allocateDeriv2 && ((pMatrix->deriv2=MEMORY_AllocDMatrix(functionName,"deriv2",basel,nl+basel-1,basec+1,nc+basec-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  // Store the information on the new allocated matrix

  else
   {
    pMatrix->nl=nl;
    pMatrix->nc=nc;
    pMatrix->basel=basel;
    pMatrix->basec=basec;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Free
// -----------------------------------------------------------------------------
// PURPOSE       Release the buffers allocated for a matrix
//
// INPUT         pMatrix         : information on the matrix to release
//               callingFunction : the name of the calling function
// -----------------------------------------------------------------------------

void MATRIX_Free(MATRIX_OBJECT *pMatrix, const char *callingFunction)
 {
  // Declaration

  char functionName[MAX_FCT_LEN+1];

  // Initialization

  memset(functionName,0,MAX_FCT_LEN+1);

  // Build the complete function name

  if (strlen(callingFunction)<=MAX_FCT_LEN-strlen("MATRIX_Free via "))
   sprintf(functionName,"MATRIX_Free via %s",callingFunction);
  else
   sprintf(functionName,"MATRIX_Free");

  // Release the allocated buffers

  if (pMatrix->matrix!=NULL)
   MEMORY_ReleaseDMatrix(functionName,"matrix",pMatrix->matrix,pMatrix->basec,pMatrix->basel);
  if (pMatrix->deriv2!=NULL)
   MEMORY_ReleaseDMatrix(functionName,"deriv2",pMatrix->deriv2,pMatrix->basec+1,pMatrix->basel);

  // Reset pointers and indexes

  memset(pMatrix,0,sizeof(MATRIX_OBJECT));
 }

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Copy
// -----------------------------------------------------------------------------
// PURPOSE       Copy the content of a matrix in another one
//
// INPUT         pSource         : a pointer to the matrix to copy
//               callingFunction : the name of the calling function
//
// INPUT/OUTPUT  pTarget, structure to a new matrix object that is a copy of
//               the pSource one
//
// RETURN        ERROR_ID_ALLOCMATRIX if the allocation of a the target matrix failed;
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MATRIX_Copy(MATRIX_OBJECT *pTarget,MATRIX_OBJECT *pSource, const char *callingFunction)
 {
  // Declarations

  INDEX indexC;
  RC rc;

  // Initializations

  MATRIX_Free(pTarget,"MATRIX_Copy");
  rc=ERROR_ID_NO;

  // Use the dimensions of the source matrix to allocate the target matrix

  if ((pSource->nl==0) || (pSource->nc==0) || (pSource->matrix==NULL))
   ERROR_SetLast("MATRIX_Copy",ERROR_TYPE_FATAL,ERROR_ID_ALLOCMATRIX,"pSource",
                  pSource->basel,pSource->nl+pSource->basel-1,pSource->basec,pSource->nc+pSource->basec-1);

  else if ((rc=MATRIX_Allocate(pTarget,pSource->nl,pSource->nc,pSource->basel,pSource->basec,
           (pSource->deriv2!=NULL)?1:0,callingFunction))==ERROR_ID_NO)

   {
       // Make a copy of the input matrix into the new allocated one

    for (indexC=pSource->basec;indexC<pSource->basec+pSource->nc;indexC++)
     memcpy(pTarget->matrix[indexC]+pSource->basel,pSource->matrix[indexC]+pSource->basel,sizeof(double)*pSource->nl);

    // Make a copy of the second derivatives

    if (pSource->deriv2!=NULL)
     for (indexC=pSource->basec+1;indexC<pSource->basec+pSource->nc;indexC++)
      memcpy(pTarget->deriv2[indexC]+pSource->basel,pSource->deriv2[indexC]+pSource->basel,sizeof(double)*pSource->nl);
   }

  // Return

  return rc;
 }

// ================
// FILES PROCESSING
// ================

    double ScientificToDouble(char *in_String) {
        // Loop Variables
        int        Counter       = 0;
        int        Length        = strlen(in_String) + 1;
        // Flags and signs
        int        NegativeFlag  = 0;
        int        DecimalFlag   = 0;
        int        ExponentSign  = 0;  // -1 = Negative, 0 = None, 1 = Positive
        // Numerical Data
        int        Exponent      = 0;
        double     FinalDivision = 1.;
        double     Digits        = 0.;

        // Loop per each character. Ignore anything weird.
        for (;Counter < Length; Counter++) {
            // Depending on the current character
            switch (in_String[Counter]) {
                // On any digit
                case '0': case '5':
                case '1': case '6':
                case '2': case '7':
                case '3': case '8':
                case '4': case '9':
                    // If we haven't reached an exponent yet ("e")
                    if (ExponentSign == 0) {
                        // Adjust the final division if a decimal was encountered
                        if (DecimalFlag) FinalDivision *= (double)10.;
                        // Add a digit to our main number
                        Digits = (Digits * 10.) + (in_String[Counter] - '0');
                    // If we passed an "e" at some point
                    } else {
                        // Add a digit to our exponent
                        Exponent = (Exponent * 10) + (in_String[Counter] - '0');
                    }
                    break;
                // On a negative sign
                case '-':
                    // If we passed an 'e'
                    if (ExponentSign > 0)
                        // The exponent sign will be negative
                        ExponentSign = -1;
                    // Otherwise we are still dealing with the main number
                    else
                        // Set the negative flag. We will negate the main number later.
                        NegativeFlag = 1;
                    break;
                // If we encounter some kind of "e"
                case 'e': case 'E':
                    // Set the exponent flag
                    ExponentSign = 1;
                    break;
                // If we encounter a period
                case '.':
                    // Set the decimal flag. We will start tracking decimal depth.
                    DecimalFlag = 1;
                    break;
                // We gladly accept all sorts of additional garbage.
                default:
                    break;
            }
        }
        // If the negative flag is set, negate the main number
        if (NegativeFlag)
            Digits = -Digits;

        // If the exponent is supposed to be negative, negate it now
        if (ExponentSign < 0)
            Exponent = 0 - Exponent;

        // Return the calculated result of our observations
        return ((double)Digits / (double)FinalDivision) * (double)(pow((double)10.0f, (double)Exponent));
    }

int MatrixNextDouble(FILE *fp,double *dvalue)
 {
     // Declarations

     char c,oldc;
     char item[MAX_ITEM_TEXT_LEN];
     int nc;
     int isdouble;

     memset(item,0,MAX_ITEM_TEXT_LEN);
     *dvalue=(double)0.;

     for (nc=0,isdouble=0,oldc=' ';((c=fgetc(fp))!=EOF) && nc<MAX_ITEM_TEXT_LEN;)
      {
          if ((c==EOF) || (c=='\n') || (c==0x0D) || (c==0x0A) || ((c==' ') && (oldc!=' ')) || ((c=='\t') && (nc>0)))
           {
            if (nc>0)
             isdouble=((c==EOF) || (c=='\n') || (c==0x0D) || (c==0x0A))?-1:1;
            break;
           }
          else if ((c!=' ') && (c!='\t'))
           {
               if (!strchr("0123456789+-eE.",c))
             break;
            else
             item[nc++]=c;
           }

          oldc=c;
      }

  if (nc<MAX_ITEM_TEXT_LEN)
      *dvalue=(double)ScientificToDouble(item);

     // return

     return isdouble;
 }

void MATRIX_PassCommentLines(FILE *fp)
 {
      char c[2000];
      while (fscanf(fp, COMMENT_LINE, c) == 1) {
        // skip spaces and comment lines (assumed to start with character ';', '#' or '*')
      }
 }

int MATRIX_GetColumnsNumbers(FILE *fp,double *pFirstValue)
 {
  // Determine the number of columns

  int nc = 0;
  double firstCalib=(double)0.,tempValue;
  int rcNextDouble;

  while ((rcNextDouble=MatrixNextDouble(fp,&tempValue))!=0)
   {
       if (!nc)
        firstCalib=tempValue;
       nc++;
       if (rcNextDouble<0)
        break;
   }

  if (pFirstValue!=NULL)
   *pFirstValue=firstCalib;

  // Return

  return nc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Load
// -----------------------------------------------------------------------------
// PURPOSE       Load a matrix from file
//
// INPUT         fileName        : te name of the file to load
//               nl,nc           : resp. the number of lines and columns of the matrix to load
//                                 0 to retrieve the dimensions automatically from the file
//               xmin,xmax       : define a range of values to load for the first column of the matrix
//                                 usually the wavelength calibration
//               allocateDeriv2  : 1 to allocate buffers for the second derivatives
//               reverseFlag     : 1 to reverse the matrix (flip up/down)
//               callingFunction : the name of the calling function
//
// INPUT/OUTPUT  pMatrix, structure to receive the data loaded from the input file
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file can not be open;
//               ERROR_ID_FILE_EMPTY if the length of the input file is 0;
//               ERROR_ID_WAVELENGTH if the wavelength calibration of the input file
//                                   doesn't cover the analysis spectral range
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MATRIX_Load(const char *fileName,MATRIX_OBJECT *pMatrix,
               int nl,int nc,double xmin,double xmax,
               int allocateDeriv2,int reverseFlag, const char *callingFunction) {
  // Declarations

  char     fullPath[MAX_ITEM_TEXT_LEN];                                         // the complete file name to load
  int      nlMin,ncMin;                                                         // resp. the minimum numbers of lines and columns to load from the file
  INDEX    i,j;                                                                 // indexes for browsing lines and columns in matrix
  double **matrix,**deriv2,                                                     // resp. pointers to the matrix to load and to the second derivatives
           xMin,xMax,                                                           // define the range of values to load for the first column of the matrix
           tempValue;                                                           // a value of the matrix to load
  FILE    *fp;                                                                  // file pointer
  RC       rc;                                                                  // return code
  double firstCalib;

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  FILES_RebuildFileName(fullPath,fileName,1);                                   // build the complete path and file name

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Print("File to load : %s\n",fullPath);
  #endif

  nlMin=nl;
  ncMin=nc;
  firstCalib=(double)0.;
  rc=ERROR_ID_NO;

  xMin=min(xmin,xmax);
  xMax=max(xmin,xmax);

  // Reset matrix

  MATRIX_Free(pMatrix, __func__);

  // File open

  if ((fp=fopen(fullPath,"rt"))==NULL)
   rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,fullPath);
  else if (!STD_FileLength(fp) )
   rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,fullPath);
  else {
    // The function has to determine the number of lines and columns
    if (!nl || !nc) {
      MATRIX_PassCommentLines(fp);
      nc=MATRIX_GetColumnsNumbers(fp,&firstCalib);
      nl= ( (fabs(xMin-xMax)<EPSILON) || ((firstCalib>=xMin)&&(firstCalib<=xMax)) ) ? 1 : 0;

      // Determine the number of lines
      while (fscanf(fp, "%lf%*[^\n]\n", &tempValue) == 1) {

        if ((fabs(xMin-xMax)<EPSILON) || ((tempValue>=xMin)&&(tempValue<=xMax))) {
          ++nl;
        }
      }
    }

    // File read out

#if defined(__DEBUG_) && __DEBUG_
    DEBUG_Print("Size of the matrix to load : %d x %d last Value %g\n",nl,nc,tempValue);
#endif

    if (!nl || !nc || (nl<nlMin) || (nc<ncMin))
      rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fullPath);
    else if (!(rc=MATRIX_Allocate(pMatrix,nl,nc,0,0,allocateDeriv2,callingFunction)))
     {
      matrix=pMatrix->matrix;
      deriv2=pMatrix->deriv2;

      fseek(fp,0L,SEEK_SET);

      for (i=0; i<nl && !rc;) {
        char c[2000];
        while (fscanf(fp, COMMENT_LINE, c) == 1) {
          // skip spaces and comment lines (assumed to start with character ';', '#' or '*')
        }

        // read first column
        int n_read = fscanf(fp,"%lf",&tempValue);

        if (n_read != 1) {
          rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_LENGTH,fullPath);
        } else if ( (xMin==xMax) || ((tempValue>=xMin)&&(tempValue<=xMax)) ) {
          matrix[0][i] = tempValue;
          for (j=1; j<nc && !rc; ++j) {
            n_read = fscanf(fp, NEXT_DOUBLE, &matrix[j][i]);
            if (n_read != 1) {
              rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_LENGTH,fullPath);
            }
          }
          ++i;

          // check each column has same length 'nc'. we should now find a newline or the end of the file:

          int next=' ';
             while ((next==' ') || (next=='\t'))
              next = fgetc(fp);

          //int next = fgetc(fp);
          if ( next != '\n' && next != EOF) {
            rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_LENGTH,fullPath);
          }
        }
      }

      // Found no lines with a starting values in the range [xmin..xmax]

      if (i==0)
       rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_WAVELENGTH,fullPath);

      // Flip up/down the matrix

      else if (reverseFlag && (matrix[0][0]>matrix[0][1]))
       for (i=0; i<nl/2 ; ++i)
         for (j=0; j<nc; ++j)
         {
          tempValue=matrix[j][i];
          matrix[j][i]=matrix[j][nl-1-i];
          matrix[j][nl-1-i]=tempValue;
         }

      // Calculate second derivatives of the columns of the matrix for future interpolation

      if (allocateDeriv2)
        for (j=1; (j<nc) && !rc; ++j)
         rc=SPLINE_Deriv2(((double *)matrix[0]),
                          ((double *)matrix[j]),
                          ((double *)deriv2[j]),
                          pMatrix->nl,
                          callingFunction);
     }
   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Release allocated buffers

  if (rc)
   MATRIX_Free(pMatrix,__func__);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  if (!rc)
   {
       int basel=0,basec=0;
    DEBUG_PrintVar("the matrix",pMatrix->matrix,basel,basel+nl-1,basec,basec+nc-1,NULL);
    if (allocateDeriv2)
     DEBUG_PrintVar("the 2nd derivatives",pMatrix->deriv2,basel,basel+nl-1,basec+1,basec+nc-1,NULL);
   }
  DEBUG_FunctionStop(__func__, rc);
  #endif

  // Return

  return rc;
}
