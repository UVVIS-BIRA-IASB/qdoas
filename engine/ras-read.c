
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  RASAS read out routines
//  Name of module    :  RAS-READ.C
//  Creation date     :  7 January 2000
//  Ref               :  the format has been transmitted by INTA
//
//  Author            :  INTA, Spain
//  Adaptations       :  Caroline FAYT
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
//  This module contains routines needed to read the data measured by the RASAS
//  instrument operated by INTA.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetRAS - calculate the number of files in a directory in RAS format;
//  RasReadRecord - read a record in RAS format;
//  ReliRAS - RASAS format read out;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "winthrd.h"
#include "zenithal.h"
#include "stdfunc.h"

static int rasDirFlag;                                                          // 1 to process a directory, 0 to process only one file

// -----------------------------------------------------------------------------
// FUNCTION      SetRAS
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of files in a directory in RAS format
//
// INPUT         pEngineContext : information on the file to read out
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetRAS(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  char  fileName[DOAS_MAX_PATH_LEN+1],                                              // name of the current file
        *ptr,*ptr2;                                                             // pointers to parts in the previous string
  int    fileLength;                                                            // length of the file
  RC     rc;                                                                    // return code

  // Initializations

  memset(fileName,0,DOAS_MAX_PATH_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,DOAS_MAX_PATH_LEN);
  pEngineContext->lastRefRecord=0;
  rasDirFlag=0;
  rc=ERROR_ID_NO;

  // Check the input file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("SetRAS",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);

  // According to the file extension, process a directory or a file

  else if (((ptr=strrchr(fileName,PATH_SEP))!=NULL) &&
           ((ptr2=strrchr(ptr,'.'))!=NULL) &&                                   // get the extension
            !strcasecmp(ptr2+1,"dir"))
   {
    if ((fileLength=STD_FileLength(specFp))>0)
     {
      pEngineContext->recordNumber=fileLength/12;
      rasDirFlag=1;
     }
    else
     pEngineContext->recordNumber=0;
   }
  else
   pEngineContext->recordNumber=1;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      RasReadRecord
// -----------------------------------------------------------------------------
// PURPOSE       Read a record in RAS format
//
// INPUT         fileName          the name of the current file;
//
// OUTPUT        spe               the spectrum;
//               drk               the dark current;
//               pEngineContext         data on the current spectrum;
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file can't be found;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC RasReadRecord(char *fileName,double *spe,double *drk,ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  char fileLine[MAX_STR_SHORT_LEN+1],                                          // line of file
        keyName[MAX_STR_SHORT_LEN+1],                                           // key name for commented lines
        strBuffer[MAX_STR_SHORT_LEN+1];                                         // string buffer

  FILE *fp;                                                                     // pointer to the current file
  INDEX i;                                                                      // browse pixels in the spectrum
  int   field1,field2,field3;                                                   // fields
  RC rc;                                                                        // return code

  // Initializations

  const int n_wavel = NDET[0];
  pRecord=&pEngineContext->recordInfo;

  memset(fileLine,0,MAX_STR_SHORT_LEN+1);
  memset(keyName,0,MAX_STR_SHORT_LEN+1);
  memset(strBuffer,0,MAX_STR_SHORT_LEN+1);

  rc=ERROR_ID_NO;

  // Open file

  if ((fp=fopen(fileName,"rt"))==NULL)
   rc=ERROR_SetLast("RasReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,fileName);

  else if (!STD_FileLength(fp) ||                                               // file length is zero
           !fgets(fileLine,MAX_STR_SHORT_LEN,fp) ||                             // no file line anymore
           !sscanf(fileLine,"%lf",&pRecord->Zm))                              // no zenith angle

   rc=ERROR_SetLast("RasReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

  else
   {
    // Spectrum and dark current read out

    if (drk!=NULL)
     for (i=0;(i<n_wavel) && fgets(fileLine,MAX_STR_SHORT_LEN,fp) && sscanf(fileLine,"%lf %lf",&spe[i],&drk[i]);i++);
    else
     for (i=0;(i<n_wavel) && fgets(fileLine,MAX_STR_SHORT_LEN,fp) && sscanf(fileLine,"%lf",&spe[i]);i++);

    if (i<n_wavel)
     rc=ERROR_SetLast("RasReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
    else

     // Fill pEngineContext with data on current spectrum

     while (fgets(fileLine,MAX_STR_SHORT_LEN,fp))
      {
       sscanf(fileLine,"%[^=]=%[^\n]",keyName,strBuffer);

       // Time

       if (!strnicmp(keyName,"Hora UTM",8))
        {
         sscanf(strBuffer,"%d:%d:%d",&field1,&field2,&field3);

         pRecord->present_datetime.thetime.ti_hour=(unsigned char)field1;
         pRecord->present_datetime.thetime.ti_min=(unsigned char)field2;
         pRecord->present_datetime.thetime.ti_sec=(unsigned char)field3;
        }

       // Date

       else if (!strnicmp(keyName,"Fecha UTM",9))
        {
         sscanf(strBuffer,"%d/%d/%d",&field1,&field2,&field3);

         if (field3<30)
          field3+=(short)2000;
         else if (field3<130)
          field3+=(short)1900;
         else if (field3<1930)
          field3+=(short)100;

         pRecord->present_datetime.thedate.da_day=(char)field1;
         pRecord->present_datetime.thedate.da_mon=(char)field2;
         pRecord->present_datetime.thedate.da_year=(short)field3;
        }

       // Exposure time

       else if (!strnicmp(keyName,"Tiempo Exposicion",17))
        sscanf(strBuffer,"%lf",&pRecord->Tint);

       // Temperature of the detector

       else if (!strnicmp(keyName,"T.Detector",10))
        sscanf(strBuffer,"%lf",&pRecord->TDet);

       // Geolocation coordinates

       else if (!strnicmp(keyName,"Latitud",7))
        {
         sscanf(strBuffer,"%d° %d' %d",&field1,&field2,&field3);
         pRecord->latitude=(double)field1+1.*field2/60.+1.*field3/3600.;
        }
       else if (!strnicmp(keyName,"Longitud",8))
        {
         sscanf(strBuffer,"%d° %d' %d",&field1,&field2,&field3);
         pRecord->longitude=(double)field1+1.*field2/60.+1.*field3/3600.;
        }
       else if (!strnicmp(keyName,"Altura",6))
        sscanf(strBuffer,"%lf",&pRecord->altitude);
       else if (!strnicmp(keyName,"Barridos solicitados",20))
        sscanf(strBuffer,"%d",&pRecord->rejected);
       else if (!strnicmp(keyName,"Barridos sin saturar",20))
        sscanf(strBuffer,"%d",&pRecord->NSomme);
      }
   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliRAS
// -----------------------------------------------------------------------------
// PURPOSE       RAS format read out
//
// INPUT         pEngineContext    information on the file to read
//               recordNo     index of the record to read;
//               dateFlag     0 no date constraint; 1 a date selection is applied;
//               localDay     if dateFlag is 1, the calendar day for the
//                            reference spectrum to search for
//               specFp       pointer to the spectra file;
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliRAS(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  char                fileName[DOAS_MAX_PATH_LEN+1],                                // name of the current file (the current record)
                      *ptr;                                                     // pointers to parts in the previous string
  RC                   rc;                                                      // return code
  double               tmLocal;                                                 // the measurement time in seconds

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  memset(fileName,0,DOAS_MAX_PATH_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,DOAS_MAX_PATH_LEN);
  rc=ERROR_ID_NO;

  if ((ptr=strrchr(fileName,PATH_SEP))==NULL)
   rc=ERROR_ID_FILE_RECORD;
  else if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber))
   {
    // Build the right file name

    if (rasDirFlag)
     {
      fseek(specFp,(recordNo-1)*12,SEEK_SET);
      fread(ptr+1,12,1,specFp);
      *(ptr+13)=0;
     }

    // Record read out

    if (!(rc=RasReadRecord(fileName,pEngineContext->buffers.spectrum,pEngineContext->buffers.darkCurrent,pEngineContext)))
     {
      pRecord->rejected-=pRecord->NSomme;

      pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)pRecord->Tint*pRecord->NSomme;
      pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;
      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
//      pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&pRecord->longitude,&pRecord->latitude,&pRecord->Azimuth);

      tmLocal=pRecord->Tm+THRD_localShift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

      // User constraints

      if (dateFlag && (pRecord->localCalDay>localDay))
       rc=ERROR_ID_FILE_END;

      else if ((pRecord->NSomme<=0) ||
               (dateFlag && (pRecord->localCalDay!=localDay)))

       rc=ERROR_ID_FILE_RECORD;

      else
       {
        pRecord->longitude=-pRecord->longitude;

        if (dateFlag)
         pEngineContext->lastRefRecord=recordNo;
       }
     }
   }

  // Return

  return rc;
 }

