
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Read spectra from measurements performed by the University of Toronto
//  Name of module    :  UofT-read.c
//  Creation date     :  12 November 2003
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
//  This module has been written according to the format description provided by
//  Elham FARAHANI, Department of Physics, University of Toronto.  Two formats
//  are available : CSV with several spectra in the same file and another one
//  with the individual spectra distributed in several files.  In the latter
//  case, the following rules are applied :
//
//  - spectra numbers for one day go from 00 01 02 ... 09 0A 0B ... 0Z  to
//    Z0 Z1 ... Z9 ZA ZB ... ZZ e.g. 36x36 records maximum for one day;
//
//  - all individual spectra files of a same day are considered as a one large
//    file (in order to make a daily reference search easier);
//
//  - so, to insert files in the project tree, only one file per day is enough;
//  - for the use of the option "insert path" from the raw spectrum node of the
//    project tree, only the first number of daily records is listed;
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetUofT - calculate the number of spectra measured the current day;
//  UofTReadRecord - read a record in the case of individual spectra in several files;
//  ReliUofT - read a record in the UofT format.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "zenithal.h"
#include "winthrd.h"

// ========================
// DEFINITION OF STRUCTURES
// ========================

// ===================
// STATIC DECLARATIONS
// ===================

static INDEX UofT_lastRecord=ITEM_NONE;                                         // index of last record in the CSV format

// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SetUofT
// -----------------------------------------------------------------------------
// PURPOSE       Get the number of spectra in the current file
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetUofT(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declaration

  RC rc;

  // Initializations

  UofT_lastRecord=ITEM_NONE;

  pEngineContext->recordNumber=0;

  rc=ERROR_ID_NO;

  // Calculate the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("SetUofT",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    fseek(specFp,0L,SEEK_SET);
    fscanf(specFp,"%d",&pEngineContext->recordNumber);

    if (pEngineContext->recordNumber<=0)
     rc=ERROR_SetLast("SetUofT",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UofTGotoRecord
// -----------------------------------------------------------------------------
// PURPOSE       skip records in a file created by University of Toronto
//
// INPUT         specFp    pointer to the current file
//               recordNo  the record to reach
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_FILE_END if the end of the file is reached
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC UofTGotoRecord(FILE *specFp,int recordNo)
 {
  // Declarations

  char fileLine[STRING_LENGTH+1];                                               // string buffer for records
  RC rc;                                                                        // return code

  // Initialization

  rc=ERROR_ID_NO;

  // Buffer allocation

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else
   {
    // Goto back to the beginning of the file

    if (UofT_lastRecord>=recordNo)
     fseek(specFp,0L,SEEK_SET);

    // Browse lines

    for (;!feof(specFp) && fgets(fileLine,STRING_LENGTH,specFp);)

     // Search for spectra divider

     if ((fileLine[0]=='*') &&
         (fileLine[1]=='*') &&
         (fileLine[2]=='*'))
      {
      	// Retrieve the record index

       fgets(fileLine,STRING_LENGTH,specFp);
      	sscanf(fileLine,"%d",&UofT_lastRecord);

      	// Check if it is the searched record

      	if (UofT_lastRecord==recordNo)
      	 break;
      }
   }

  if (UofT_lastRecord!=recordNo)
   rc=ERROR_ID_FILE_END;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UofTReadRecord
// -----------------------------------------------------------------------------
// PURPOSE       read the current record
//
// INPUT         pUofTData    pointer to the structure with information on the current record
//               specFp       pointer to the current file
//
// OUTPUT        spectrum     the current spectrum
//
// RETURN        ERROR_ID_FILE_RECORD     problem while reading the record
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC UofTReadRecord(UOFT_DATA *pUofTData,double *spectrum,FILE *specFp,char *fileName)
 {
  // Declarations

  int    Dm,Mm,Ym,Ds,Ms,Ys,Df,Mf,Yf,                                            // fields of resp. mean, starting and ending dates
         hm,mm,sm,hs,ms,ss,hf,mf,sf;                                            // fields of resp. mean, starting and ending times
  char   fileLine[MAX_STR_LEN+1];                                               // line of the file
  int    pixMin,pixMax,i;                                                       // pixels range
  RC     rc;                                                                    // return code

  // Initializations

  const int n_wavel = NDET[0];
  rc=ERROR_ID_NO;

  // Get the range of pixels of the detector

  if (!fgets(fileLine,MAX_STR_LEN,specFp) ||                                        // the size of the detector
      (sscanf(fileLine,"%d %d",&pixMin,&pixMax)!=2) ||
      (pixMax-pixMin+1!=n_wavel))

   rc=ERROR_SetLast("UofTReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

  // Read all information on the current measurement

  else if (!fgets(fileLine,MAX_STR_LEN,specFp) ||
          (sscanf(fileLine,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                            &Dm,&Mm,&Ym,&hm,&mm,&sm,&Ds,&Ms,&Ys,&hs,&ms,&ss,&Df,&Mf,&Yf,&hf,&mf,&sf)!=18) ||

           !fgets(fileLine,MAX_STR_LEN,specFp) || (sscanf(fileLine,"%f %f %f",&pUofTData->solarElevAngle,&pUofTData->startSolarElevAngle,&pUofTData->endSolarElevAngle)!=3) ||
           !fgets(fileLine,MAX_STR_LEN,specFp) ||
          (sscanf(fileLine,"%f %f %f %f %f %f %f %f %f %f %f %f %f ",
                 &pUofTData->shutter,                                           // Shutter
                 &pUofTData->numCounts,                                         // Ideal Num of Counts
                 &pUofTData->slitWidth,                                         // Slit Width
                 &pUofTData->groove,                                            // Groove Density
                 &pUofTData->turret,                                            // Turret Position
                 &pUofTData->blazeWve,                                          // Blaze wavelength
                 &pUofTData->centerWve,                                         // Centre wavelength
                 &pUofTData->intTime,                                           // Integration Time
                 &pUofTData->numAcc,                                            // Num Accumulations
                 &pUofTData->meanCCDT,                                          // Mean CCD temperature
                 &pUofTData->minCCDT,                                           // Min CCD temperature
                 &pUofTData->maxCCDT,                                           // Max TCCD temperature
                 &pUofTData->meanBoxT)!=13) ||                                  // Mean box temperature

           !fgets(fileLine,MAX_STR_LEN,specFp) || (sscanf(fileLine,"%f %f %f %f",&pUofTData->measType,&pUofTData->viewElev,&pUofTData->viewAzim,&pUofTData->filterId)!=4) ||
           !fgets(fileLine,MAX_STR_LEN,specFp) || (sscanf(fileLine,"%f %f",&pUofTData->longitude,&pUofTData->latitude)!=2))

   rc=ERROR_SetLast("UofTReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
  else
   {
    // Spectrum read out

    for (i=0;(i<n_wavel) && fgets(fileLine,MAX_STR_LEN,specFp) && sscanf(fileLine,"%lf",&spectrum[i]);i++);

    if (i<n_wavel)
     rc=ERROR_SetLast("UofTReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
    else
     {
      // Get date and time information (avoid problem of type conversion)

      pUofTData->startDate.da_day=(char)Ds;                                     // start Date (day)
      pUofTData->startDate.da_mon=(char)Ms;                                     // start Date (month)
      pUofTData->startDate.da_year=(short)Ys;                                   // start Date (year)

      pUofTData->startTime.ti_hour=(char)hs;                                    // start Time (hour)
      pUofTData->startTime.ti_min=(char)ms;                                     // start Time (min)
      pUofTData->startTime.ti_sec=(char)ss;                                     // start Time (sec)

      pUofTData->endDate.da_day=(char)Df;                                       // end Date (day)
      pUofTData->endDate.da_mon=(char)Mf;                                       // end Date (month)
      pUofTData->endDate.da_year=(short)Yf;                                     // end Date (year)

      pUofTData->endTime.ti_hour=(char)hf;                                      // end Time (hour)
      pUofTData->endTime.ti_min=(char)mf;                                       // end Time (min)
      pUofTData->endTime.ti_sec=(char)sf;                                       // end Time (sec)

      pUofTData->meanDate.da_day=(char)Dm;                                      // mean Date (day)
      pUofTData->meanDate.da_mon=(char)Mm;                                      // mean Date (month)
      pUofTData->meanDate.da_year=(short)Ym;                                    // mean Date (year)

      pUofTData->meanTime.ti_hour=(char)hm;                                     // mean Time (hour)
      pUofTData->meanTime.ti_min=(char)mm;                                      // mean Time (min)
      pUofTData->meanTime.ti_sec=(char)sm;                                      // mean Time (sec)
     }
   }

  // Return

  if (rc)
   rc=ERROR_ID_FILE_RECORD;

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliUofT
// -----------------------------------------------------------------------------
// PURPOSE       Read a record in the UofT format
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_FILE_END if the end of the file is reached;
//               ERROR_ID_FILE_RECORD if the record doesn't satisfy criteria
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC ReliUofT(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  UOFT_DATA *pUofT;                                                             // pointer to the uoft part of the record
  RC      rc;                                                                   // return code
  double  tmLocal;
  int hourDiff;                                                             // local time

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pUofT=&pRecord->uoft;
  rc=ERROR_ID_NO;

  // Read the record

  if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber) &&
     !(rc=UofTGotoRecord(specFp,recordNo)) &&
     (!(rc=UofTReadRecord(pUofT,pEngineContext->buffers.spectrum,specFp,pEngineContext->fileInfo.fileName)) || (rc==ERROR_ID_FILE_RECORD)))
   {
    memcpy(&pRecord->present_datetime.thedate,&pUofT->meanDate,sizeof(struct date));
    memcpy(&pRecord->present_datetime.thetime,&pUofT->meanTime,sizeof(struct time));
    memcpy(&pRecord->startDateTime.thetime,&pUofT->startTime,sizeof(struct time));
    memcpy(&pRecord->endDateTime.thetime,&pUofT->endTime,sizeof(struct time));
    memcpy(&pRecord->startDateTime.thedate,&pUofT->startDate,sizeof(struct date));
    memcpy(&pRecord->endDateTime.thedate,&pUofT->endDate,sizeof(struct date));



    // Get information on the current record

    pRecord->NSomme=(int)pUofT->numAcc;                                         // number of accumulations
    pRecord->Tint=(double)pUofT->intTime*0.001;                                 // integration time
    pRecord->Zm=(double)90.-pUofT->solarElevAngle;                              // solar zenith angle
    pRecord->ReguTemp=pUofT->meanBoxT;                                          // box temperature
    pRecord->TDet=(double)pUofT->meanCCDT;                                      // detector temperature

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
    pRecord->TotalAcqTime=(double)pRecord->NSomme*pRecord->Tint;

    hourDiff=(pRecord->endDateTime.thetime.ti_hour>=pRecord->startDateTime.thetime.ti_hour)?
          pRecord->endDateTime.thetime.ti_hour-pRecord->startDateTime.thetime.ti_hour:
          pRecord->endDateTime.thetime.ti_hour+24-pRecord->startDateTime.thetime.ti_hour;

    pRecord->TotalExpTime=(double)hourDiff*3600.+
                                  (pRecord->endDateTime.thetime.ti_min-pRecord->startDateTime.thetime.ti_min)*60.+
                                  (pRecord->endDateTime.thetime.ti_sec-pRecord->startDateTime.thetime.ti_sec);

    pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;

    pRecord->elevationViewAngle=pUofT->viewElev;
    pRecord->azimuthViewAngle=pUofT->viewAzim;
    pRecord->longitude=pUofT->longitude;
    pRecord->latitude=pUofT->latitude;

    pRecord->maxdoas.measurementType=(pRecord->elevationViewAngle<(double)80.)?PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS:PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;

    // Determine the local time

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    // Search for a reference spectrum
    // For UofT format, one spectra file per day

    // if (dateFlag && (pRecord->localCalDay>localDay))
    //  rc=ERROR_ID_FILE_END;
    //
    // else if ((pRecord->NSomme<=0) ||
    //          (dateFlag && (pRecord->localCalDay!=localDay)))
    //
    //  rc=ERROR_ID_FILE_RECORD;
    //
    // else

    if (dateFlag)
     pEngineContext->lastRefRecord=recordNo;
   }

  // Return

  return rc;
 }

