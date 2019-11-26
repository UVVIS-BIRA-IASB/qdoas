
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ASCII FILES OPERATIONS
//  Name of module    :  ASCII.C
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
//  Functions to read spectra from ASCII files.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ===============
//  FILE PROCESSING
//  ===============
//
//  AsciiSkip - skip a given number of records in ASCII files;
//  ASCII_Set - set file pointers for ASCII files and get the number of records;
//  ASCII_Read - read a record from the ASCII file;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <math.h>
#include <string.h>

#include "engine_context.h"
#include "spectrum_files.h"
#include "winthrd.h"
#include "vector.h"
#include "zenithal.h"

#include "doas.h"

// ===================
// CONSTANT DEFINITION
// ===================

// format strings for fscanf
// #define NEXT_DOUBLE "%lf%*[^0-9.\n-]"
// #define NEXT_FLOAT "%f%*[^0-9.\n-]"
// #define NEXT_DATE "%d/%d/%d%*[^\n0-9.-]"
// #define COMMENT_LINE " %1[*;#]%*[^\n]"

#define NEXT_DOUBLE "%lf%*[^0-9.\n\r-]"
#define NEXT_FLOAT "%f%*[^0-9.\n\r-]"
#define NEXT_DATE "%d/%d/%d%*[^\n\r0-9.-]"
#define COMMENT_LINE " %1[*;#]%*[^\n\r]"

#define MAX_ASC_FIELDS 29

// ================
// GLOBAL VARIABLES
// ================

//PRJCT_ASCII ASCII_options;                                                      // options from the 'Export in Ascii' dialog box

// ================
// STATIC VARIABLES
// ================

static INDEX asciiLastRecord=ITEM_NONE;                                         // keep the index of the last record
static INDEX asciiLastDataSet=ITEM_NONE;
static MATRIX_OBJECT asciiMatrix;

// ===============
// FILE PROCESSING
// ===============


// Helper function to check if the next character in a file is '\n' or EOF
static inline bool line_ends(FILE *fp) {
  int next = fgetc(fp);
  if (next == '\n' || next == '\r' || next == EOF) {
    return true;
  } else {
    ungetc(next, fp);
    return false;
  }
}

// -----------------------------------------------------------------------------
// FUNCTION        AsciiSkip
// -----------------------------------------------------------------------------
// PURPOSE         skip a given number of records in ASCII files
//
// INPUT           pEngineContext : information on the file to read out
//                 specFp    : pointer to the ASCII file
//                 nSkip     : number of records/data set to skip
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_FILE_END if the end of file is reached;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC AsciiSkip(ENGINE_CONTEXT *pEngineContext,FILE *specFp,int nSkip)
 {
  // Declarations
  int itemCount,recordCount,maxCount;                                           // counters
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the pEngineContext structure
  RC rc;                                                                        // return code

  // Initializations

  pInstr=&pEngineContext->project.instrumental;
  maxCount=NDET[0]+pInstr->ascii.szaSaveFlag+pInstr->ascii.azimSaveFlag+pInstr->ascii.elevSaveFlag+pInstr->ascii.timeSaveFlag+pInstr->ascii.dateSaveFlag;
  rc=ERROR_ID_NO;

  // Buffer allocation

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if (nSkip>=pEngineContext->recordNumber)
   rc=ERROR_ID_FILE_END;
  else

  // Skip the nSkip first spectra
   {
    fseek(specFp,0L,SEEK_SET);
    recordCount=0;
    itemCount=0;

    char c[2];
    int n_scan = 0;
    while (recordCount<nSkip
           && (n_scan = fscanf(specFp, " %1[^*;#\n\r]%*[^\n\r]", c) ) != EOF ) {
      if (n_scan == 1) {
        // no * or ; at start -> line is (part of) a record
        if (pInstr->ascii.format==PRJCT_INSTR_ASCII_FORMAT_LINE) {
          // Each line of the file is a spectrum record
          ++recordCount;
        } else {
          // Spectra records are saved in successive columns
          ++itemCount;
          if (itemCount == maxCount) {
            ++recordCount;
            itemCount=0;
          }
        }
      }
    }
    // Reach the end of the file
    if (recordCount<nSkip)
      rc=ERROR_ID_FILE_END;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_Set
// -----------------------------------------------------------------------------
// PURPOSE         Set file pointers for ASCII files and get the number of records
//
// INPUT           pEngineContext : information on the file to read
//                 specFp    : pointer to the ASCII file
//
// OUTPUT          pEngineContext->recordNumber, the number of records
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC ASCII_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations
  int itemCount,startCount,maxCount;                                            // counters
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the pEngineContext structure
  double tempValue;
  int nc;
  RC rc;                                                                        // return code

  // Initializations

  asciiLastRecord=ITEM_NONE;                                                    // reset the index of the last record
  asciiLastDataSet=ITEM_NONE;                                                   // data set (for column/matrix modes)

  pEngineContext->recordNumber=0;
  pInstr=&pEngineContext->project.instrumental;
  startCount=pInstr->ascii.szaSaveFlag+pInstr->ascii.azimSaveFlag+pInstr->ascii.elevSaveFlag+pInstr->ascii.timeSaveFlag+pInstr->ascii.dateSaveFlag;
  maxCount=NDET[0];
  rc=ERROR_ID_NO;
  nc=0;

  ASCII_Free(__func__);

  // Check the file pointer

  if (specFp==NULL)
    rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else {
    // Get the number of records in the file
    fseek(specFp,0L,SEEK_SET);
    itemCount=0;

    char c[2];
    int n_scan = 0;
    while ( (n_scan = fscanf(specFp, " %1[^*;#\n\r]", c) ) != EOF) {
      if (n_scan == 0) {
        // commment, ignore and scan ahead until end of line
        fscanf(specFp, "%*[^\n\r]");
        continue;
      }
      // no * or ; at start -> line is (part of) a record
      ungetc(c[0], specFp); // put back character we read
      if (pInstr->ascii.format==PRJCT_INSTR_ASCII_FORMAT_LINE) {
        // Each line of the file is a spectrum record
        pEngineContext->recordNumber++;
        fscanf(specFp, "%*[^\n\r]");
      } else {
        // Spectra records are saved in successive columns

        // Get the number of columns
        if ((itemCount==startCount) && !pEngineContext->recordNumber) {
          bool eol = false;
          while (!eol) {
            n_scan = fscanf(specFp, NEXT_DOUBLE, &tempValue);
            if (n_scan == 1) {
              ++nc;
              eol = line_ends(specFp);
            } else {
              return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
            }
          }
          if (nc>pInstr->ascii.lambdaSaveFlag+1)
            rc=MATRIX_Allocate(&asciiMatrix,NDET[0]+startCount,nc,0,0,0,__func__);
        } else {
          // scan ahead until end of line
          fscanf(specFp, "%*[^\n\r]");
        }
        ++itemCount;

        // Matrix mode
        if (itemCount==maxCount) {
          itemCount=0;
          if (nc>pInstr->ascii.lambdaSaveFlag+1)
            pEngineContext->recordNumber+=nc-pInstr->ascii.lambdaSaveFlag;
          else
            pEngineContext->recordNumber++;
        }
      }
    }
  }

  // Return
  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_Read
// -----------------------------------------------------------------------------
// PURPOSE         Read a record from the ASCII file
//
// INPUT           pEngineContext : information on the file to read
//                 recordNo  : the index of the record to read
//                 dateFlag  : 1 to search for a reference spectrum
//                 localDay  : if dateFlag is 1, the calendar day for the
//                             reference spectrum to search for
//                 specFp    : pointer to the ASCII file
//
// OUTPUT          information on the read out record
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_FILE_END if the end of the file is reached;
//                 ERROR_ID_FILE_EMPTY if the file is not large enough;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_FILE_RECORD if the record doesn't match the spectra selection criteria
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC ASCII_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecordInfo;                                                         // pointer to the record part of the engine context
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the pEngineContext structure
  double *spectrum,*lambda,                                                     // the spectrum and the wavelength calibration to read
          tmLocal;                                                              // the measurement time in seconds
  int lambdaFlag,zmFlag,timeFlag,dateSaveFlag,azimFlag,elevFlag,                // flags to select items to read according to the format options
      day,mon,year;                                                             // decomposition of the measurement date
  INDEX i,indexColumn;                                                          // browse items to read
  int ndataSet,ndataRecord;                                                     // number of data set to bypass (column + matrix modes)
  double tempValue;
  int dateCount;
  RC rc;                                                                        // return code
  int count;

  // Initializations

  pRecordInfo=&pEngineContext->recordInfo;
  spectrum=pEngineContext->buffers.spectrum;
  lambda=pEngineContext->buffers.lambda;
  pInstr=&pEngineContext->project.instrumental;
  zmFlag=pInstr->ascii.szaSaveFlag;
  azimFlag=pInstr->ascii.azimSaveFlag;
  elevFlag=pInstr->ascii.elevSaveFlag;
  timeFlag=pInstr->ascii.timeSaveFlag;
  dateSaveFlag=pInstr->ascii.dateSaveFlag;
  lambdaFlag=pInstr->ascii.lambdaSaveFlag;
  rc=ERROR_ID_NO;

  day=mon=year=ITEM_NONE;
  pRecordInfo->Tint=(double)0.;

  dateCount=(dateSaveFlag)?zmFlag+azimFlag+elevFlag:ITEM_NONE;
  ndataSet=(asciiMatrix.nc)?(recordNo-1)/(asciiMatrix.nc-lambdaFlag):ITEM_NONE;
  ndataRecord=recordNo-ndataSet*(asciiMatrix.nc-lambdaFlag)+lambdaFlag-1;

  memset(&pRecordInfo->present_datetime,0,sizeof(pRecordInfo->present_datetime));

  const int n_wavel = NDET[0];
  VECTOR_Init(spectrum,(double)0.,n_wavel);

  // Set file pointers

  if (specFp==NULL)
    rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
    rc=ERROR_ID_FILE_END;
  else if (((ndataSet!=ITEM_NONE) && (ndataSet==asciiLastDataSet)) || (recordNo-asciiLastRecord==1) || !(rc=AsciiSkip(pEngineContext,specFp,(ndataSet!=ITEM_NONE)?ndataSet:recordNo-1)))
   {
    asciiLastRecord=recordNo;

    // ------------------------------------------
    // EACH LINE OF THE FILE IS A SPECTRUM RECORD
    // ------------------------------------------

    int n_scan = 0;
    char c[2];
    if (pInstr->ascii.format==PRJCT_INSTR_ASCII_FORMAT_LINE) {

      // skip comment lines:
      while (fscanf(specFp, COMMENT_LINE, c) == 1);

      // Read the solar zenith angle
      if (zmFlag) {
        n_scan = fscanf(specFp,NEXT_DOUBLE,&pRecordInfo->Zm);

        if (n_scan != 1)
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
      }

      if (azimFlag) {
        if (line_ends(specFp) )
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

        n_scan = fscanf(specFp,NEXT_FLOAT,&pRecordInfo->azimuthViewAngle);
        if (n_scan != 1)
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
      }

      if (elevFlag) {
        if (line_ends(specFp) )
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

        n_scan = fscanf(specFp,NEXT_FLOAT,&pRecordInfo->elevationViewAngle);
        if (n_scan != 1)
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
      }

      // Read the measurement date
      if (dateSaveFlag) {
        if (line_ends(specFp) )
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

        n_scan = fscanf(specFp,NEXT_DATE,&day,&mon,&year);

        if (n_scan != 3)
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
      }

      // Read the measurement time
      if (timeFlag) {
        if (line_ends(specFp) )
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

        n_scan = fscanf(specFp,NEXT_DOUBLE,&pRecordInfo->TimeDec);

        if (n_scan != 1)
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
      }

      // Read the spectrum
      for (i=0; i<n_wavel; ++i) {
        if (line_ends(specFp) )
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

        n_scan = fscanf(specFp,NEXT_DOUBLE,&spectrum[i]);

        if (n_scan != 1)
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
      }
    }

    // -----------------------------------------------
    // SPECTRA RECORDS ARE SAVED IN SUCCESSIVE COLUMNS
    // -----------------------------------------------

    else if ((asciiMatrix.nl>=n_wavel) && (asciiMatrix.nc>lambdaFlag)) {
      if (ndataSet!=asciiLastDataSet) {
        asciiLastDataSet=ndataSet;

        for (i=0;i<asciiMatrix.nl;i++) {
          while (fscanf(specFp, COMMENT_LINE, c) == 1);

          // Matrix mode
          for (indexColumn=0; indexColumn <asciiMatrix.nc; indexColumn++) {
            if (line_ends(specFp) )
              return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

            if (i==dateCount) {
              n_scan = fscanf(specFp,NEXT_DATE, &day, &mon, &year);
              if (n_scan != 3)
                return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
              tempValue=(double)day*1.e6+mon*1e4+year;
            } else {
              n_scan = fscanf(specFp,NEXT_DOUBLE, &tempValue);
              if (n_scan != 1)
                return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
            }
            asciiMatrix.matrix[indexColumn][i]=tempValue;
          }
          // a new line should start here now
          if (!line_ends(specFp) )
            return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
        }
      }

      count=0;
      if (zmFlag)
        pRecordInfo->Zm=asciiMatrix.matrix[ndataRecord][count++];
      if (azimFlag)
        pRecordInfo->azimuthViewAngle=asciiMatrix.matrix[ndataRecord][count++];
      if (elevFlag)
        pRecordInfo->elevationViewAngle=asciiMatrix.matrix[ndataRecord][count++];
      if (dateSaveFlag) {
        tempValue=asciiMatrix.matrix[ndataRecord][count++];

       	day=(int)floor(tempValue*1e-6);
       	mon=(int)floor((tempValue-day*1.e6)*1e-4);
       	year=(int)(tempValue-day*1.e6-mon*1.e4);
      }
      if (timeFlag)
        pRecordInfo->TimeDec=asciiMatrix.matrix[ndataRecord][count++];

      if (lambdaFlag)
        memcpy(lambda,asciiMatrix.matrix[0]+count,sizeof(double)*n_wavel);

      memcpy(spectrum,asciiMatrix.matrix[ndataRecord]+count,sizeof(double)*n_wavel);
    } else {
      // Read the solar zenith angle

      if (zmFlag) {
        while (fscanf(specFp, COMMENT_LINE, c) == 1);
        if (fscanf(specFp,NEXT_DOUBLE,&pRecordInfo->Zm) !=1
            || !line_ends(specFp) )
          return ERROR_ID_FILE_END;
      }

      if (azimFlag) {
        while (fscanf(specFp, COMMENT_LINE, c) == 1);
        if (fscanf(specFp,NEXT_FLOAT,&pRecordInfo->azimuthViewAngle)!=1
            || !line_ends(specFp) )
          return ERROR_ID_FILE_END;
      }

      if (elevFlag) {
        while (fscanf(specFp, COMMENT_LINE, c) == 1);
        if (fscanf(specFp,NEXT_FLOAT,&pRecordInfo->elevationViewAngle)!=1
            || !line_ends(specFp) )
          return ERROR_ID_FILE_END;
      }

      // Read the measurement date
      if (dateSaveFlag) {
        while (fscanf(specFp, COMMENT_LINE, c) == 1);
        if (fscanf(specFp,NEXT_DATE,&day,&mon,&year) != 3
            || !line_ends(specFp) )
          return ERROR_ID_FILE_END;
      }

      // Read the measurement time
      if (timeFlag) {
        while (fscanf(specFp, COMMENT_LINE, c) == 1);
        if (fscanf(specFp,NEXT_DOUBLE,&pRecordInfo->TimeDec) != 1
            || !line_ends(specFp) )
          return ERROR_ID_FILE_END;
      }

      // Read the spectrum and if selected, the wavelength calibration
      if (lambdaFlag) {
        // wavelength and spectrum
        for (i=0; i<n_wavel;) {
          if (fscanf(specFp, COMMENT_LINE, c) == 1)
            continue;
          // read two doubles before end of line:
          if (fscanf(specFp, NEXT_DOUBLE, &lambda[i]) != 1 || line_ends(specFp) )
            return ERROR_ID_FILE_END;
          if (fscanf(specFp, NEXT_DOUBLE, &spectrum[i]) != 1 || !line_ends(specFp) )
            return ERROR_ID_FILE_END;
          ++i;
        }
      } else {
        // just spectrum
        for (i=0; i<n_wavel; ) {
          if (fscanf(specFp, COMMENT_LINE, c) == 1)
            continue;
          if ((fscanf(specFp,NEXT_DOUBLE,&spectrum[i]) != 1)
              || (!line_ends(specFp) && !feof(specFp)) )
            return ERROR_ID_FILE_END;
          ++i;
        }
      }
    }

    pRecordInfo->maxdoas.measurementType=(elevFlag && (pRecordInfo->elevationViewAngle<(double)80.))?PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS:PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;

    // Get information on the current record
    if (timeFlag) {
      pRecordInfo->present_datetime.thetime.ti_hour=(unsigned char)pRecordInfo->TimeDec;
      pRecordInfo->present_datetime.thetime.ti_min=(unsigned char)((pRecordInfo->TimeDec-pRecordInfo->present_datetime.thetime.ti_hour)*60.);
      pRecordInfo->present_datetime.thetime.ti_sec=(unsigned char)(((pRecordInfo->TimeDec-pRecordInfo->present_datetime.thetime.ti_hour)*60.-pRecordInfo->present_datetime.thetime.ti_min)*60.);
    }

    if (dateSaveFlag) {
      pRecordInfo->present_datetime.thedate.da_day=(char)day;
      pRecordInfo->present_datetime.thedate.da_mon=(char)mon;
      pRecordInfo->present_datetime.thedate.da_year= year;

      // Daily automatic reference spectrum
      pRecordInfo->Tm=(double)ZEN_NbSec(&pRecordInfo->present_datetime.thedate,&pRecordInfo->present_datetime.thetime,0);

      tmLocal=pRecordInfo->Tm+THRD_localShift*3600.;

      pRecordInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecordInfo->localTimeDec=fmod(pRecordInfo->TimeDec+24.+THRD_localShift,(double)24.);

    } else {
      pRecordInfo->Tm=(double)0.;
    }

    // if (dateFlag && ((pRecordInfo->localCalDay!=localDay) || (elevFlag && (pRecordInfo->elevationViewAngle<80.))))                                                                               // reference spectra are zenith only

    if (rc || (dateFlag && ((pRecordInfo->localCalDay!=localDay) ||
            (( (elevFlag && (fabs(pRecordInfo->elevationViewAngle+0.1)>EPSILON)) || (azimFlag && (fabs(pRecordInfo->azimuthViewAngle+0.1)>EPSILON))) &&
             ((pRecordInfo->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
              (pRecordInfo->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))))

//        (!dateFlag && pEngineContext->analysisRef.refScan && !pEngineContext->analysisRef.refSza && (pRecordInfo->elevationViewAngle>80.)))    // zenith sky spectra are not analyzed in scan reference selection mode
      rc=ERROR_ID_FILE_RECORD;
   }

  return rc;
 }

void ASCII_Free(const char *functionStr) {
  MATRIX_Free(&asciiMatrix,functionStr);
  memset(&asciiMatrix,0,sizeof(MATRIX_OBJECT));
}
