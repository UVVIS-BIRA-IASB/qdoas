
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ASCII FILES OPERATIONS
//  Name of module    :  ASCII_QDOAS.C
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>

#include "engine_context.h"
#include "engine.h"
#include "spectrum_files.h"
#include "winthrd.h"
#include "vector.h"
#include "zenithal.h"
#include "stdfunc.h"

#include "doas.h"
// ===================
// CONSTANT DEFINITION
// ===================

// format strings for fscanf
#define NEXT_DOUBLE "%lf%*[^0-9.\r\n-]"
#define NEXT_FLOAT "%f%*[^0-9.\r\n-]"
#define NEXT_DATE "%d/%d/%d%*[^\r\n0-9.-]"
#define COMMENT_LINE "*;#%%\n"
#define NUMBER_LINE  "0123456789+-"

#define MAX_RECORDS 3000 // for ground-based measurements, usually, a record every minute should be the maximum
                         // for satellite measurements, ASCII spectra shouldn't be used

// ================
// GLOBAL VARIABLES
// ================

// ================
// STATIC VARIABLES
// ================

static long *asciiRecordOffset=NULL;
static int   asciiRecordAllocatedSize=0;
static int   asciiFileHeaderSize=0;
static int   asciiRecordHeaderSize=0;
static int   asciiSpectraSize=0;
static int   asciiLastOffset=ITEM_NONE;

enum _ascComments
 {
  ASC_COMMENT_DETECTOR_SIZE,
  ASC_COMMENT_RECORDS_NUMBER,
  ASC_COMMENT_MAX
 };

char *asciiCommentsSelection[ASC_COMMENT_MAX]=
 {
  (char *)"size of the detector",
  (char *)"total number of records"
 };

char *ascFieldsNames[PRJCT_RESULTS_MAX]=
 {
  (char *)"Spec No",                                                            // PRJCT_RESULTS_SPECNO,
  (char *)"Name",                                                               // PRJCT_RESULTS_NAME,
  (char *)"Date & time (DD/MM/YYYY hh:mm:ss)",                                  // PRJCT_RESULTS_DATE_TIME,
  (char *)"Date",                                                               // PRJCT_RESULTS_DATE,
  (char *)"UTC time",                                                           // PRJCT_RESULTS_TIME,
  (char *)"Year",                                                               // PRJCT_RESULTS_YEAR,
  (char *)"Day Number",                                                         // PRJCT_RESULTS_JULIAN,
  (char *)"Fractional Day",                                                     // PRJCT_RESULTS_JDFRAC,
  (char *)"Fractional Time",                                                    // PRJCT_RESULTS_TIFRAC,
  (char *)"Number of scans",                                                    // PRJCT_RESULTS_SCANS,
  (char *)"Rejected",                                                           // PRJCT_RESULTS_NREJ,
  (char *)"Exposure Time",                                                      // PRJCT_RESULTS_TINT,
  (char *)"Solar Zenith Angle ",                                                // PRJCT_RESULTS_SZA,
  (char *)"",                                                                   // PRJCT_RESULTS_CHI,
  (char *)"",                                                                   // PRJCT_RESULTS_RMS,
  (char *)"Solar Azimuth Angle ",                                               // PRJCT_RESULTS_AZIM,
  (char *)"Tdet",                                                               // PRJCT_RESULTS_TDET,
  (char *)"Sky Obs",                                                            // PRJCT_RESULTS_SKY,
  (char *)"",                                                                   // PRJCT_RESULTS_BESTSHIFT,
  (char *)"",                                                                   // PRJCT_RESULTS_REFZM,
  (char *)"",                                                                   // PRJCT_RESULTS_REFNUMBER,
  (char *)"",                                                                   // PRJCT_RESULTS_REFNUMBER_BEFORE,
  (char *)"",                                                                   // PRJCT_RESULTS_REFNUMBER_AFTER,
  (char *)"",                                                                   // PRJCT_RESULTS_REFSHIFT,
  (char *)"Pixel Number",                                                       // PRJCT_RESULTS_PIXEL,
  (char *)"Pixel Type",                                                         // PRJCT_RESULTS_PIXEL_TYPE,
  (char *)"Orbit Number",                                                       // PRJCT_RESULTS_ORBIT,
  (char *)"Longitude",                                                          // PRJCT_RESULTS_LONGIT,
  (char *)"Latitude",                                                           // PRJCT_RESULTS_LATIT,
  (char *)"",                                                                   // PRJCT_RESULTS_LON_CORNERS
  (char *)"",                                                                   // PRJCT_RESULTS_LAT_CORNERS
  (char *)"Altitude",                                                           // PRJCT_RESULTS_ALTIT,
  (char *)"",                                                                   // PRJCT_RESULTS_COVAR,
  (char *)"",                                                                   // PRJCT_RESULTS_CORR,
  (char *)"Cloud fraction",                                                     // PRJCT_RESULTS_CLOUD,
  (char *)"",                                                                   // PRJCT_RESULTS_O3,
  (char *)"",                                                                   // PRJCT_RESULTS_NO2,
  (char *)"Cloud top pressure",                                                 // PRJCT_RESULTS_CLOUDTOPP,
  (char *)"Line of sight zenith angle",                                         // PRJCT_RESULTS_LOS_ZA,
  (char *)"Line of sight azimuth angle",                                        // PRJCT_RESULTS_LOS_AZIMUTH,
  (char *)"Satellite height",                                                   // PRJCT_RESULTS_SAT_HEIGHT,
  (char *)"Satellite latitude",                                                 // PRJCT_RESULTS_SAT_LAT,
  (char *)"Satellite longitude",                                                // PRJCT_RESULTS_SAT_LON,
  (char *)"Solar zenith angle at satellite",                                    // PRJCT_RESULTS_SAT_SAA,
  (char *)"Solar azimuth angle at satellite",                                   // PRJCT_RESULTS_SAT_SZA,
  (char *)"Viewing zenith angle at satellite",                                  // PRJCT_RESULTS_SAT_VZA,
  (char *)"Earth radius",                                                       // PRJCT_RESULTS_EARTH_RADIUS,
  (char *)"Viewing elevation angle ",                                           // PRJCT_RESULTS_VIEW_ELEVATION,
  (char *)"Viewing azimuth angle ",                                             // PRJCT_RESULTS_VIEW_AZIMUTH,
  (char *)"Viewing zenith angle ",                                              // PRJCT_RESULTS_VIEW_ZENITH,
  (char *)"SCIAMACHY Quality Flag",                                             // PRJCT_RESULTS_SCIA_QUALITY,
  (char *)"SCIAMACHY State Index",                                              // PRJCT_RESULTS_SCIA_STATE_INDEX,
  (char *)"SCIAMACHY State Id",                                                 // PRJCT_RESULTS_SCIA_STATE_ID,
  (char *)"Start Date",                                                         // PRJCT_RESULTS_STARTDATE,
  (char *)"End Date ",                                                          // PRJCT_RESULTS_ENDDATE,
  (char *)"UTC Start Time",                                                     // PRJCT_RESULTS_STARTTIME,
  (char *)"UTC End Time",                                                       // PRJCT_RESULTS_ENDTIME,
  (char *)"Scanning Angle",                                                     // PRJCT_RESULTS_SCANNING,
  (char *)"Filter Number",                                                      // PRJCT_RESULTS_FILTERNUMBER,
  (char *)"Measurement Type",                                                   // PRJCT_RESULTS_MEASTYPE,
  (char *)"Head Temperature",                                                   // PRJCT_RESULTS_CCD_HEADTEMPERATURE,
  (char *)"Cooler status",                                                      // PRJCT_RESULTS_COOLING_STATUS,
  (char *)"Mirror status",                                                      // PRJCT_RESULTS_MIRROR_ERROR,
  (char *)"Compass angle",                                                      // PRJCT_RESULTS_COMPASS,
  (char *)"Pitch angle",                                                        // PRJCT_RESULTS_PITCH,
  (char *)"Roll angle",                                                         // PRJCT_RESULTS_ROLL,
  (char *)"",                                                                   // PRJCT_RESULTS_ITER,
  (char *)"",                                                                   // PRJCT_RESULTS_ERROR_FLAG,
  (char *)"",                                                                   // PRJCT_RESULTS_NUM_BANDS,
  (char *)"GOME2 MDR index",                                                    // PRJCT_RESULTS_GOME2_MDR_NUMBER,
  (char *)"GOME2 observation index",                                            // PRJCT_RESULTS_GOME2_OBSERVATION_INDEX,
  (char *)"GOME2 scan direction",                                               // PRJCT_RESULTS_GOME2_SCANDIRECTION,
  (char *)"GOME2 observation mode",                                             // PRJCT_RESULTS_GOME2_OBSERVATION_MODE,
  (char *)"GOME2 SAA flag",                                                     // PRJCT_RESULTS_GOME2_SAA,
  (char *)"GOME2 sunglint risk flag",                                           // PRJCT_RESULTS_GOME2_SUNGLINT_RISK,
  (char *)"GOME2 sunglint high risk flag",                                      // PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK,
  (char *)"GOME2 rainbow flag",                                                 // PRJCT_RESULTS_GOME2_RAINBOW,
  (char *)"Diodes",                                                             // PRJCT_RESULTS_CCD_DIODES,
  (char *)"Target Azimuth",                                                     // PRJCT_RESULTS_CCD_TARGETAZIMUTH,
  (char *)"Target Elevation",                                                   // PRJCT_RESULTS_CCD_TARGETELEVATION,
  (char *)"Saturated",                                                          // PRJCT_RESULTS_SATURATED,
  (char *)"along-track index",                                                  // PRJCT_RESULTS_INDEX_CROSSTRACK,
  (char *)"cross-track index",                                                  // PRJCT_RESULTS_INDEX_ALONGTRACK,
  (char *)"OMI groundpixel quality flag",                                       // PRJCT_RESULTS_OMI_GROUNDP_QF,
  (char *)"OMI xtrack quality flag",                                            // PRJCT_RESULTS_OMI_XTRACK_QF,
  (char *)"",                                                                   // PRJCT_RESULTS_OMI_PIXELS_QF,
  (char *)"OMI instrument configuration id",                                    // PRJCT_RESULTS_OMI_CONFIGURATION_ID,
  (char *)"",                                                                   // PRJCT_RESULTS_SPIKES,
  (char *)"UAV servo sent position byte",                                       // PRJCT_RESULTS_UAV_SERVO_BYTE_SENT,
  (char *)"UAV servo received position byte",                                   // PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED,
  (char *)"Inside temperature",                                                 // PRJCT_RESULTS_UAV_INSIDE_TEMP,
  (char *)"Outside temperature",                                                // PRJCT_RESULTS_UAV_OUTSIDE_TEMP,
  (char *)"Pressure",                                                           // PRJCT_RESULTS_UAV_PRESSURE,
  (char *)"Humidity",                                                           // PRJCT_RESULTS_UAV_HUMIDITY,
  (char *)"Dewpoint",                                                           // PRJCT_RESULTS_UAV_DEWPOINT,
  (char *)"pitch angle",                                                        // PRJCT_RESULTS_UAV_PITCH,
  (char *)"roll angle",                                                         // PRJCT_RESULTS_UAV_ROLL,
  (char *)"heading",                                                            // PRJCT_RESULTS_UAV_HEADING,
  (char *)"",                                                                   // PRJCT_RESULTS_SLANT_COL,
  (char *)"",                                                                   // PRJCT_RESULTS_SLANT_ERR,
  (char *)"",                                                                   // PRJCT_RESULTS_SHIFT,
  (char *)"",                                                                   // PRJCT_RESULTS_SHIFT_ERR,
  (char *)"",                                                                   // PRJCT_RESULTS_LAMBDA_CENTER,
  (char *)"",                                                                   // PRJCT_RESULTS_STRETCH,
  (char *)"",                                                                   // PRJCT_RESULTS_STRETCH_ERR,
  (char *)"",                                                                   // PRJCT_RESULTS_SCALE,
  (char *)"",                                                                   // PRJCT_RESULTS_SCALE_ERR,
  (char *)"",                                                                   // PRJCT_RESULTS_PARAM,
  (char *)"",                                                                   // PRJCT_RESULTS_PARAM_ERR,
  (char *)"",                                                                   // PRJCT_RESULTS_AMF,
  (char *)"",                                                                   // PRJCT_RESULTS_VERT_COL,
  (char *)"",                                                                   // PRJCT_RESULTS_VERT_ERR,
  (char *)"",                                                                   // PRJCT_RESULTS_FLUX,
  (char *)"",                                                                   // PRJCT_RESULTS_CIC,
  (char *)"",                                                                   // PRJCT_RESULTS_WAVELENGTH,
  (char *)"Precalculated flux",                                                 // PRJCT_RESULTS_PRECALCULATED_FLUXES,
  (char *)"GPS Start Time (hhmmss.ms)",                                         // PRJCT_RESULTS_STARTGPSTIME,
  (char *)"GPS Stop Time (hhmmss.ms)",                                          // PRJCT_RESULTS_ENDGPSTIME,
  (char *)"Longitude End",                                                      // PRJCT_RESULTS_LONGITEND,
  (char *)"Latitude End",                                                       // PRJCT_RESULTS_LATITEND,
  (char *)"Altitude End",                                                       // PRJCT_RESULTS_ALTITEND,
  (char *)"Total Measurement Time",                                             // PRJCT_RESULTS_TOTALEXPTIME
  (char *)"Total Acquisition Time",                                             // PRJCT_RESULTS_TOTALACQTIME
  (char *)"lambda",                                                             // PRJCT_RESULTS_LAMBDA,
  (char *)"spectrum",                                                           // PRJCT_RESULTS_SPECTRA,
  (char *)"filename",                                                           // PRJCT_RESULTS_FILENAME
  (char *)"Scan index",                                                         // PRJCT_RESULTS_SCANINDEX
  (char *)"Index zenith before",                                                // PRJCT_RESULTS_ZENITH_BEFORE,
  (char *)"Index zenith after",                                                 // PRJCT_RESULTS_ZENITH_AFTER,
  (char *)"Return code"                                                         // PRJCT_RESULTS_RC
 };

enum _ascLineType
 {
  ASC_LINE_TYPE_UNKNOWN,
  ASC_LINE_TYPE_COMMENT,
  ASC_LINE_TYPE_FIELD,
  ASC_LINE_TYPE_SPECTRA
 };

// -----------------------------------------------------------------------------
// FUNCTION      ASCII_QDOAS_Reset
// -----------------------------------------------------------------------------
// PURPOSE       Release the buffers allocated by ASCII_QDOAS_Alloc
// -----------------------------------------------------------------------------

void ASCII_QDOAS_Reset(void)
 {
  if (asciiRecordOffset!=NULL)
   MEMORY_ReleaseBuffer("ASCII_QDOAS_Reset","asciiRecordOffset",asciiRecordOffset);
  asciiRecordOffset=NULL;
  asciiRecordAllocatedSize=0;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ASCII_QDOAS_Alloc
// -----------------------------------------------------------------------------
// PURPOSE       Allocate a buffer for the starting positions of records
// -----------------------------------------------------------------------------

RC ASCII_QDOAS_Alloc(int bufferSize)
 {
  if (bufferSize>asciiRecordAllocatedSize)
   {
    ASCII_QDOAS_Reset();
    asciiRecordOffset=(long *)MEMORY_AllocBuffer("ASCII_QDOAS_Alloc","asciiRecordOffset",bufferSize,sizeof(long),0,MEMORY_TYPE_LONG);
    asciiRecordAllocatedSize=bufferSize;
   }

  if (asciiRecordOffset!=NULL)
   for (int i=0;i<bufferSize;i++)
    asciiRecordOffset[i]=(long)ITEM_NONE;

  return((asciiRecordOffset==NULL)?ERROR_ID_ALLOC:ERROR_ID_NO);
 }


// ===============
// FILE PROCESSING
// ===============

int ASCII_QDOAS_GetKeynameIndex(char *keyName,char *keyNameList[],int keyNameListSize)
 {
  // Declarations

  char keyNameFromList[STRING_LENGTH+1];
  int i;

  // Search for the field in list

  for (i=0;i<keyNameListSize;i++)
   if (strlen(keyNameList[i]))
    {
     strcpy(keyNameFromList,keyNameList[i]);
     STD_Strlwr(keyNameFromList);
     if (strstr(keyName,keyNameFromList)==keyName)
      break;
    }

  // Return

  return ((i<keyNameListSize)?i:ITEM_NONE);
 }

int ASCII_QDOAS_GetLineType(char *fileLine,int *pIndexField)
 {
  // Declaration

  int rc,commentFlag;
  char *ptr;

  // Initialization

  rc=ASC_LINE_TYPE_UNKNOWN;
 *pIndexField=ITEM_NONE;
  commentFlag=0;

  // Force fileLine to be lower case

  STD_Strlwr(fileLine);

  // Remove leading blanks

  for (ptr=fileLine;*ptr!='\0';ptr++)
   if (strchr(COMMENT_LINE,*ptr)!=NULL)
    commentFlag=1;
   else if (*ptr=='\t')
    *ptr=' ';
   else if (*ptr!=' ')
    break;

  // Spectrum

  if (!commentFlag && (strchr(NUMBER_LINE,*ptr)!=NULL))
   rc=ASC_LINE_TYPE_SPECTRA;
  else
   {
    *pIndexField=(commentFlag)?ASCII_QDOAS_GetKeynameIndex(ptr,asciiCommentsSelection,ASC_COMMENT_MAX):
                               ASCII_QDOAS_GetKeynameIndex(ptr,ascFieldsNames,PRJCT_RESULTS_MAX);

    // Search for the field in list

    if (*pIndexField!=ITEM_NONE)
     rc=(commentFlag)?ASC_LINE_TYPE_COMMENT:ASC_LINE_TYPE_FIELD;
    else
     rc=ASC_LINE_TYPE_UNKNOWN;
   }

  // Return

  return(rc);
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_QDOAS_Goto
// -----------------------------------------------------------------------------
// PURPOSE         skip a given number of records in ASCII files
//
// INPUT           specFp    : pointer to the ASCII file
//                 recordNo  : the record number where to go
//
// RETURN          ERROR_ID_FILE_END if the end of file is reached;
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC ASCII_QDOAS_Goto(FILE *specFp,int recordNo)
 {
  // Declarations

  int spectraSize,indexField;
  char fileLine[STRING_LENGTH+1];
  int indexRecord;
  RC rc;

  // Initializations

  indexRecord=recordNo-1;
  rc=ERROR_ID_NO;

  if (asciiRecordOffset[indexRecord]==(long)ITEM_NONE)
   {
    while (!rc && !feof(specFp) && asciiLastOffset<indexRecord)
     {
      for (spectraSize=0;!rc && (spectraSize<asciiSpectraSize) && !feof(specFp) && fgets(fileLine,STRING_LENGTH,specFp);)
       if (ASCII_QDOAS_GetLineType(fileLine,&indexField)==ASC_LINE_TYPE_SPECTRA)
        spectraSize++;

      if (spectraSize==asciiSpectraSize)
       asciiRecordOffset[++asciiLastOffset]=(long)ftell(specFp);
     }
    if (asciiLastOffset<indexRecord)
     rc=ERROR_ID_FILE_END;
   }
  else
   fseek(specFp,asciiRecordOffset[indexRecord],SEEK_SET);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_QDOAS_Set
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

RC ASCII_QDOAS_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  char fileLine[STRING_LENGTH+1];                                               // string buffer for records
  int recordNumber;
  int headerSize,spectraSize;
  int n_wavel;
  int startDateFlag,startTimeFlag;
  int headerFlag=0;
  int indexField;
  int lineType,oldLineType;
  char keyName[STRING_LENGTH+1];
  char keyValue[STRING_LENGTH+1];
  size_t oldpos;
  long offset[MAX_RECORDS];
  RC rc;

  // Initializations

  asciiSpectraSize=
  asciiFileHeaderSize=
  asciiRecordHeaderSize=
  pEngineContext->recordNumber=0;
  asciiLastOffset=ITEM_NONE;
  recordNumber=0;
  spectraSize=0;
  startDateFlag=0;
  startTimeFlag=0;
  n_wavel = NDET[0];

  ENGINE_refStartDate=1;
  rc=ERROR_ID_NO;

  // Check the file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_NOT_FOUND),pEngineContext->fileInfo.fileName);
  else
   {
    // Get the number of records from the header ???

    fseek(specFp,0L,SEEK_SET);
    oldpos=ftell(specFp);
    spectraSize=headerSize=0;
    oldLineType=ASC_LINE_TYPE_UNKNOWN;

    for (;!rc && !feof(specFp) && fgets(fileLine,STRING_LENGTH,specFp);)
     {
      if ((lineType=ASCII_QDOAS_GetLineType(fileLine,&indexField))!=ASC_LINE_TYPE_UNKNOWN)
       {
        if (lineType!=ASC_LINE_TYPE_SPECTRA)
         sscanf(fileLine,"%[^=]=%[^\n]",keyName,keyValue);

        if ((lineType!=ASC_LINE_TYPE_COMMENT) && (asciiSpectraSize>0) && (asciiSpectraSize<=n_wavel) && (pEngineContext->recordNumber>0) &&
           ((THRD_id!=THREAD_TYPE_ANALYSIS) || !pEngineContext->analysisRef.refAuto || (startDateFlag && startTimeFlag)))
         break;
        else if (lineType==ASC_LINE_TYPE_COMMENT)
         {
          if (indexField==ASC_COMMENT_DETECTOR_SIZE)
           {
            if ((spectraSize=atoi(keyValue))>n_wavel)
             rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_BAD_LENGTH),pEngineContext->fileInfo.fileName);
            else
             asciiSpectraSize=spectraSize;

            spectraSize=0;
           }
          else if (indexField==ASC_COMMENT_RECORDS_NUMBER)
           pEngineContext->recordNumber=atoi(keyValue);
         }
        else if (lineType==ASC_LINE_TYPE_SPECTRA)
         spectraSize++;
        else if (lineType==ASC_LINE_TYPE_FIELD)
         {
          if (!startDateFlag && (indexField==PRJCT_RESULTS_DATE))
           {
            int day,mon,year;
            sscanf(fileLine,"%[^=]=%[^\n]",keyName,keyValue);

            if (sscanf(keyValue,"%d/%d/%d",&day,&mon,&year)==3)
             {
              pEngineContext->fileInfo.startDate.da_day=(char)day;
              pEngineContext->fileInfo.startDate.da_mon=(char)mon;
              pEngineContext->fileInfo.startDate.da_year=year;

              startDateFlag=1;
             }
           }

          if (!startTimeFlag && (indexField==PRJCT_RESULTS_TIME))
           {
            int hour,minute,sec,millis,n_args;
            sscanf(fileLine,"%[^=]=%[^\n]",keyName,keyValue);

            if ((n_args=sscanf(keyValue,"%d:%d:%d.%d",&hour,&minute,&sec,&millis))>=3)
             {
              pEngineContext->fileInfo.startTime.ti_hour=(unsigned char)hour;
              pEngineContext->fileInfo.startTime.ti_min=(unsigned char)minute;
              pEngineContext->fileInfo.startTime.ti_sec=(unsigned char)sec;
              pEngineContext->fileInfo.startTime.ti_hund=(unsigned char)millis/10;

              startTimeFlag=1;
             }
           }

          if (!headerSize && (recordNumber<MAX_RECORDS))
           offset[++asciiLastOffset]=oldpos;

          headerSize++;
         }
        if ((oldLineType==ASC_LINE_TYPE_FIELD) && (lineType!=ASC_LINE_TYPE_FIELD))
         {
          if (headerFlag)
           ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_BAD_FORMAT),pEngineContext->fileInfo.fileName);
          else if (!asciiRecordHeaderSize)
           {
            asciiRecordHeaderSize=headerSize;
            headerFlag=1;
           }
          else if (headerSize==asciiRecordHeaderSize)
           headerFlag=1;
          else
           ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_BAD_FORMAT),pEngineContext->fileInfo.fileName);

          headerSize=0;
         }

        if (!asciiSpectraSize && (oldLineType==ASC_LINE_TYPE_SPECTRA) && (lineType!=ASC_LINE_TYPE_SPECTRA) && spectraSize)
         asciiSpectraSize=spectraSize;

        if ((asciiSpectraSize>0) && (spectraSize==asciiSpectraSize))
         {
          if (asciiRecordHeaderSize && !headerFlag)
           ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_BAD_FORMAT),pEngineContext->fileInfo.fileName);
          else if (spectraSize!=asciiSpectraSize)
           ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_BAD_FORMAT),pEngineContext->fileInfo.fileName);
          else
           recordNumber++;

          headerFlag=0;
          spectraSize=0;
         }
       }

      oldLineType=lineType;
      oldpos=ftell(specFp);
     }

    if (!asciiSpectraSize && spectraSize)
     asciiSpectraSize=spectraSize;

    if (!recordNumber && spectraSize)
     recordNumber=1;

    if (!pEngineContext->recordNumber && recordNumber)
     pEngineContext->recordNumber=recordNumber;

    if (!pEngineContext->recordNumber || !asciiSpectraSize)
     ERROR_SetLast(__func__,ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_BAD_FORMAT),pEngineContext->fileInfo.fileName);
    else if ((rc=ASCII_QDOAS_Alloc(pEngineContext->recordNumber))!=ERROR_ID_NO)
     rc=ERROR_ID_ALLOC;
    else
     {
      if (asciiLastOffset==ITEM_NONE)
       {
        asciiRecordOffset[0]=oldpos;    // the record number and the detector size have been retrieved from the header of the file (avoid reading all the file)
        asciiLastOffset=0;
       }
      else
       memcpy(asciiRecordOffset,offset,sizeof(long)*(asciiLastOffset+1));
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_QDOAS_Read
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

RC ASCII_QDOAS_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecordInfo;                                                     // pointer to the record part of the engine context
  double *spectrum,*lambda,val1,val2,                                           // the spectrum and the wavelength calibration to read
          tmLocal;                                                              // the measurement time in seconds
  int day,mon,year,hour,minute,sec;                                             // decomposition of the measurement date and time
  INDEX i;                                                                      // browse items to read
  int spectraSize,lineType,indexField;
  char fileLine[STRING_LENGTH+1];
  char keyName[STRING_LENGTH+1];
  char keyValue[STRING_LENGTH+1];
  int useDate,useTime;
  int measurementType;
  int millis;
  RC rc;                                                                        // return code
  int n_wavel,n_args;

  // Initializations

  pRecordInfo=&pEngineContext->recordInfo;
  spectrum=pEngineContext->buffers.spectrum;
  lambda=pEngineContext->buffers.lambda;
  n_wavel=NDET[0];
  useDate=useTime=0;
  rc=ERROR_ID_NO;

  memset(&pRecordInfo->present_datetime,0,sizeof(pRecordInfo->present_datetime));

  if (specFp==NULL)
   rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber) || (ASCII_QDOAS_Goto(specFp,recordNo)!=ERROR_ID_NO))
   rc=ERROR_ID_FILE_END;
  else
   {
    for (i=0;i<n_wavel;i++)
     spectrum[i]=(double)0.;

    for (spectraSize=0;!rc && (spectraSize<asciiSpectraSize) && !feof(specFp) && fgets(fileLine,STRING_LENGTH,specFp);)
     {
      if ((lineType=ASCII_QDOAS_GetLineType(fileLine,&indexField))==ASC_LINE_TYPE_FIELD)
       {
        sscanf(fileLine,"%[^=]=%[^\n]",keyName,keyValue);
        STD_Strlwr(keyValue);

        switch(indexField)
         {
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_DATE :

           if (sscanf(keyValue,"%d/%d/%d",&day,&mon,&year)==3)
            {
             pRecordInfo->present_datetime.thedate.da_day=(char)day;
             pRecordInfo->present_datetime.thedate.da_mon=(char)mon;
             pRecordInfo->present_datetime.thedate.da_year=year;

             useDate=1;
            }
           else
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_TIME :

           if ((n_args=sscanf(keyValue,"%d:%d:%d.%d",&hour,&minute,&sec,&millis))>=3)
            {
             pRecordInfo->present_datetime.thetime.ti_hour=(char)hour;
             pRecordInfo->present_datetime.thetime.ti_min=(char)minute;
             pRecordInfo->present_datetime.thetime.ti_sec=(char)sec;
             pRecordInfo->present_datetime.millis=(n_args==4)?millis:0;

             useTime=1;
            }
           else
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_YEAR :
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_JULIAN :
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_JDFRAC :
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_TIFRAC :
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_SCANS :
           if (!sscanf(keyValue,"%d",&pRecordInfo->NSomme))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_TINT :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->Tint))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_SZA :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->Zm))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_AZIM :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->Azimuth))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_LONGIT :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->longitude))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_LATIT :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->latitude))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_ALTIT :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->altitude))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_VIEW_ELEVATION :
           if (!sscanf(keyValue,"%f",&pRecordInfo->elevationViewAngle))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_VIEW_AZIMUTH :
           if (!sscanf(keyValue,"%f",&pRecordInfo->azimuthViewAngle))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_VIEW_ZENITH :
           if (!sscanf(keyValue,"%f",&pRecordInfo->zenithViewAngle))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_STARTDATE :
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_ENDDATE :
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_STARTTIME :

           if ((n_args=sscanf(keyValue,"%d:%d:%d.%d",&hour,&minute,&sec,&millis))>=3)
            {
             pRecordInfo->startDateTime.thetime.ti_hour=(char)hour;
             pRecordInfo->startDateTime.thetime.ti_min=(char)minute;
             pRecordInfo->startDateTime.thetime.ti_sec=(char)sec;

             memcpy(&pRecordInfo->startDateTime.thedate,&pRecordInfo->present_datetime.thedate,sizeof(struct date));
             pRecordInfo->startDateTime.millis=(n_args==4)?millis:0;

            }
           else
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_ENDTIME :

           if ((n_args=sscanf(keyValue,"%d:%d:%d.%d",&hour,&minute,&sec,&millis))>=3)
            {
             pRecordInfo->endDateTime.thetime.ti_hour=(char)hour;
             pRecordInfo->endDateTime.thetime.ti_min=(char)minute;
             pRecordInfo->endDateTime.thetime.ti_sec=(char)sec;

             memcpy(&pRecordInfo->endDateTime.thedate,&pRecordInfo->present_datetime.thedate,sizeof(struct date));
             pRecordInfo->endDateTime.millis=(n_args==4)?millis:0;
            }
           else
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_MEASTYPE :

           if (strstr(keyValue,"off")!=NULL)
            pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;
           else if ((strstr(keyValue,"sun")!=NULL) || (strstr(keyValue,"ds")!=NULL))
            pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN;
           else if ((strstr(keyValue,"moon")!=NULL) || (strstr(keyValue,"dm")!=NULL))
            pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_MOON;
           else if (strstr(keyValue,"alm")!=NULL)
            pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR;
           else if (strstr(keyValue,"hor")!=NULL)
            pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_HORIZON;
           else
            pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_TOTALEXPTIME :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->TotalExpTime))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          case PRJCT_RESULTS_TOTALACQTIME :
           if (!sscanf(keyValue,"%lf",&pRecordInfo->TotalAcqTime))
            ERROR_SetLast(__func__,(rc=ERROR_TYPE_WARNING),ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);
          break;
       // ----------------------------------------------------------------------
          default :
           // TO DO : MISSING FIELDS
          break;
        // ----------------------------------------------------------------------
         }
       }
      else if (lineType==ASC_LINE_TYPE_SPECTRA)
       {
        if (sscanf(fileLine,"%lf %lf",&val1,&val2)==1)
         spectrum[spectraSize++]=val1;
        else
         {
          lambda[spectraSize]=val1;
          spectrum[spectraSize]=val2;
          spectraSize++;
         }
       }
     }

     if (!rc)
      {
       // Force zenith for viewing elevation angles higher than 80 deg

       if (pRecordInfo->elevationViewAngle>80.)
        pRecordInfo->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;

       if (useDate && useTime)
        {
         pRecordInfo->Tm=(double)ZEN_NbSec(&pRecordInfo->present_datetime.thedate,&pRecordInfo->present_datetime.thetime,0);
         tmLocal=pRecordInfo->Tm+THRD_localShift*3600.;
         pRecordInfo->localCalDay=ZEN_FNCaljda(&tmLocal);

         //  pRecordInfo->TotalExpTime = (double)pRecordInfo->NSomme*pRecordInfo->Tint;
         if (useTime)
          {
           pRecordInfo->TimeDec = (double)pRecordInfo->present_datetime.thetime.ti_hour+pRecordInfo->present_datetime.thetime.ti_min/60.+pRecordInfo->present_datetime.thetime.ti_sec/3600.+pRecordInfo->present_datetime.millis/3600000.;
           pRecordInfo->localTimeDec=fmod(pRecordInfo->TimeDec+24.+THRD_localShift,(double)24.);
          }
        }

       if (recordNo<pEngineContext->recordNumber)
        {
         asciiRecordOffset[recordNo]=(long)ftell(specFp);
         asciiLastOffset=recordNo;
        }
      }

    measurementType=pEngineContext->project.instrumental.user;

    // if (rc || (dateFlag && ((pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH) && (pRecordInfo->elevationViewAngle<80.))))

    if (rc || (dateFlag &&
            (((fabs(pRecordInfo->elevationViewAngle+0.1)>EPSILON) || (fabs(pRecordInfo->azimuthViewAngle+0.1)>EPSILON)) &&
             ((pRecordInfo->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
              (pRecordInfo->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol)))))

     rc=ERROR_ID_FILE_RECORD;

    else if (!dateFlag && (measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_NONE))
     {
     	if (((measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH)) ||
     	    ((measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=measurementType)))

     	 rc=ERROR_ID_FILE_RECORD;
     }

   }

  return rc;
 }
