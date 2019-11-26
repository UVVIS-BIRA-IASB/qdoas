
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  PDA detectors read out routines
//  Name of module    :  PDA-READ.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in 97
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
//  This module contains routines needed for reading out data from PDA detectors.
//  Three formats are described in this package :
//
//        PRJCT_INSTR_FORMAT_LOGGER        ASCII  format used for logger data by IASB;
//        PRJCT_INSTR_FORMAT_PDAEGG[_OLD]  binary format used for PDA EG&G by IASB;
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetPDA_EGG_Logger - calculate the size and the number of records for a new file
//                      in logger format;
//
//  GotoPDA_EGG_Logger - goto the requested record number;
//
//  ReliPDA_EGG_Logger - logger format read out;
//
//  SetPDA_EGG - calculate the size and the number of records for a new file
//               in PDA EG&G binary format format;
//
//  ReliPDA_EGG - IASB PDA EG&G binary format read out;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "winsites.h"
#include "vector.h"
#include "zenithal.h"
#include "stdfunc.h"
#include "winthrd.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define LOG_LENGTH 6307

// =============
// LOGGER FORMAT
// =============

INDEX pdaLastRecord=ITEM_NONE;   // Record number of the last record read out.
                                 // The logger format is an ASCII one, so in order to speed up the
                                 // spectra read out, the file pointer is not moved for successive
                                 // spectra.

// -----------------------------------------------------------------------------
// FUNCTION      SetPDA_EGG_Logger
// -----------------------------------------------------------------------------
// PURPOSE       calculate the size and the number of records for a new file
//               in logger format
//
// INPUT         specFp      pointer to the spectra file
//
// OUTPUT        pEngineContext   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------
RC SetPDA_EGG_Logger(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  char *record;                         // string buffer
  RC rc;                                // return code

  // Initializations

  pdaLastRecord=ITEM_NONE;
  pEngineContext->recordNumber=0;
  rc=ERROR_ID_NO;

  // Buffer allocation

  if ((record=(char *)MEMORY_AllocBuffer("SetPDA_EGG_Logger ","record",8001,sizeof(char),0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if (specFp==NULL)
   rc=ERROR_SetLast("SetPDA_EGG_Logger",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Count the number of records in the file

//    fseek(specFp,0L,SEEK_SET);

//    while ((fgets(record,8000,specFp)!=0))
//     pEngineContext->recordNumber++;

    pEngineContext->recordNumber=STD_FileLength(specFp)/LOG_LENGTH;
   }

  // Release the allocated buffer

  if (record!=NULL)
   MEMORY_ReleaseBuffer("SetPDA_EGG_Logger ","record",record);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GotoPDA_EGG_Logger
// -----------------------------------------------------------------------------
// PURPOSE       goto the requested record number
//
// INPUT         specFp      pointer to the spectra file
//               recordNo    the record number (0 base indexed)
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC GotoPDA_EGG_Logger(FILE *specFp,int recordNo)
 {
  // Declarations

  char *record;
  int recordSize;
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Buffer allocation

  if ((record=(char *)MEMORY_AllocBuffer("GotoPDA_EGG_Logger ","record",8001,sizeof(char),0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else
   {
    // Goto back to the beginning of the file

    fseek(specFp,0L,SEEK_SET);

    // Skip the requested number of record

    fgets(record,8000,specFp);                             // read out the first record for calculating its size
    recordSize=strlen(record);                             // each record has the same size in despite this is an ASCII format
    fseek(specFp,(int32_t)recordSize*recordNo,SEEK_SET);      // move the file pointer to the requested record

//    fseek(specFp,(int32_t)LOG_LENGTH*recordNo,SEEK_SET);      // move the file pointer to the requested record
   }

  // Release the allocated buffer

  if (record!=NULL)
   MEMORY_ReleaseBuffer("GotoPDA_EGG_Logger ","record",record);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliPDA_EGG_Logger
// -----------------------------------------------------------------------------
// PURPOSE       logger format read out
//
// INPUT         recordNo     index of record in file
//               dateFlag     0 no date constraint; 1 a date selection is applied
//               specFp       pointer to the spectra file
//
// OUTPUT        pEngineContext  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------
RC ReliPDA_EGG_Logger(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  char       *record,                                                           // string buffer for record read out
             *p;                                                                // pointer to characters in the previous string buffer
  double     *spectrum,tmLocal,                                                 // current spectrum
              Max;                                                              // maximum value used for normalizing spectra during safe keeping
  struct date day;                                                              // date of the current spectrum
  int         iday,imonth,iyear,ihour,imin,isec,                                // substitution variable for current measurement date and time read out
              ccdFlag;                                                          // 0 for RETICON detector, 1 for CCD detector
  INDEX       i;                                                                // index for browsing pixels in spectrum
  SZ_LEN      nameLen;
  RC          rc;                                                               // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  spectrum=pEngineContext->buffers.spectrum;
  rc=ERROR_ID_NO;
  const int n_wavel = NDET[0];

  // Buffer allocation

  if ((record=(char *)MEMORY_AllocBuffer("ReliPDA_EGG_Logger ","record",8001,sizeof(char),0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Set file pointer

  else if ((recordNo-pdaLastRecord==1) ||               // for successive spectra, don't move file pointer
          !(rc=GotoPDA_EGG_Logger(specFp,recordNo-1)))
   {
    pdaLastRecord=recordNo;

    // Record read out

    if (!fgets(record,8001,specFp) ||
//    if (!fread(record,LOG_LENGTH,1,specFp) ||
      (((p=strstr(record,"R "))==NULL) &&      // R holds for RETICON detectors
       ((p=strstr(record,"H "))==NULL) &&      // H holds for HAMAMATSU detectors
       ((p=strstr(record,"C "))==NULL)))       // C holds for CCD detectors

     rc=ERROR_ID_FILE_END;

    else
     {
      ccdFlag=(p[0]=='C')?1:0;

      // record read out

      p+=2;

      sscanf(p," %[^' '] %d/%d/%d %d:%d:%d %d %d %lf %lf %lf %f %f %d %d",

                 pRecord->Nom,                              // record name
                &iday,&imonth,&iyear,&ihour,&imin,&isec,      // date and time
                &pRecord->NSomme,                           // number of saved scans
                &pRecord->rejected,                         // number of rejected scans
                &pRecord->Tint,                             // integration time
                &pRecord->Zm,                               // zenith angle
                &Max,                                         // scaling factor
                &pRecord->azimuthViewAngle,
                &pRecord->elevationViewAngle,
                &pRecord->coolingStatus,
                &pRecord->mirrorError);

      if (!pEngineContext->project.instrumental.azimuthFlag || (pRecord->Azimuth==99999.))
       {
        pRecord->Azimuth=(double)-1.;
        pRecord->azimuthViewAngle=-1.;
        pRecord->elevationViewAngle=-1.;
       }
      else
       {
        if (pRecord->elevationViewAngle==99999.)
         pRecord->elevationViewAngle=-1.;
        if (pRecord->azimuthViewAngle==99999.)
         pRecord->azimuthViewAngle=-1.;
        else
         pRecord->azimuthViewAngle-=90.;                                        // mirror is perpendicular to the direction of the sun
       }

      // Build date and time of the current measurement

      day.da_day=(char)iday;
      day.da_mon=(char)imonth;
      day.da_year= iyear;

      if (day.da_year<30)
       day.da_year+= 2000;
      else if (day.da_year<130)
       day.da_year+= 1900;
      else if (day.da_year<1930)
       day.da_year+= 100;

      pRecord->present_datetime.thetime.ti_hour=(unsigned char)ihour;
      pRecord->present_datetime.thetime.ti_min=(unsigned char)imin;
      pRecord->present_datetime.thetime.ti_sec=(unsigned char)isec;

      memcpy(&pRecord->present_datetime.thedate,&day,sizeof(day));

      // Available data on the current spectrum

      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
      pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)pRecord->NSomme*pRecord->Tint;
      pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;

      tmLocal=pRecord->Tm+THRD_localShift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

      nameLen=strlen(pRecord->Nom);

      if (!pRecord->NSomme || // (pRecord->rejected>=pRecord->NSomme) ||
         ((pEngineContext->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_ZENITHAL) && (pRecord->Nom[nameLen-4]!='Z')) ||
         ((pEngineContext->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_OFFAXIS) && (pRecord->Nom[nameLen-4]!='o')) ||
         (dateFlag && (pRecord->localCalDay!=localDay)))

       rc=ERROR_ID_FILE_RECORD;

      else
       {
        // Spectrum read out

        p += 142;
        Max*=(double)pRecord->NSomme/65000.;

        for (i=0;i<n_wavel;i++)
         {
          sscanf(p,"%lf",&spectrum[i]);
          spectrum[i]*=(double)Max/pRecord->NSomme;  // test pRecord->NSomme==0 is made before
          p+=6;
         }

        if (ccdFlag)
         VECTOR_Invert(spectrum,n_wavel);
       }
     }
   }

  // Release allocated buffer

  if (record!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG_Logger ","record",record);

  // Return

  return rc;
 }

// =======================================
// BINARY FORMAT USED FOR PDA EG&G BY IASB
// =======================================

// ------------------
// Record description
// ------------------

#pragma pack(push,1)

#define PDA1453A struct PDA1453A
PDA1453A
 {
  float       ReguTemp;
  short       Detector_Temp;
  short       ScansNumber;
  short       Rejected;
  double      Exposure_time;
  double      Zm;
  char        SkyObs;
  SHORT_DATE  today;
  struct time now;
  double      azimuth;
  float       mirrorElv;
 };

#pragma pack(pop)

// ---------------------------------------------------------
// Arrays of integration times used by the real time program
// ---------------------------------------------------------

#define MAXTPS1 30 // Harestua
#define MAXTPS2 36 // OHP

float Tps1[MAXTPS1] = { (float)   0.1, (float)   0.2, (float)   0.3, (float)   0.4, (float)   0.55, (float)   0.75, (float)   1.,  (float)  1.5, (float)   2.,
                        (float)   3. , (float)   4. , (float)   5.5, (float)   7.5, (float)  10.  , (float)  15.  , (float)  20.,  (float) 30. , (float)  40.,
                        (float)  55. , (float)  75. , (float) 100. , (float) 125. , (float) 150.  , (float) 175.  , (float) 200.,  (float) 225., (float) 250.,
                        (float) 275. , (float) 300.,  (float) 600. };
float Tps2[MAXTPS2] =
 { (float)  1.00, (float)  1.20, (float)  1.50, (float)  1.75, (float)  2.00, (float)  2.50, (float)  3.00, (float)  3.60, (float)   4.30, (float)   5.20, (float)   6.20, (float)   7.50, (float)   8.90, (float)  11.00, (float)  13.00, (float)  16.00, (float)  19.00, (float)  22.00,
   (float) 27.00, (float) 32.00, (float) 38.00, (float) 46.00, (float) 55.00, (float) 66.00, (float) 80.00, (float) 95.00, (float) 115.00, (float) 140.00, (float) 160.00, (float) 200.00, (float) 240.00, (float) 280.00, (float) 340.00, (float) 410.00, (float) 490.00, (float) 590.00 };

// -----------------------------------------------------------------------------
// FUNCTION      SetPDA_EGG
// -----------------------------------------------------------------------------
// PURPOSE       calculate the size and the number of records for a new file
//               in PDA EG&G binary format
//
// INPUT         specFp      pointer to the spectra file
//               newFlag     0 format used during intercomparison campaign at Camborne, ENGLAND, 94
//                           1 format used from spring 94 until now
//
// OUTPUT        pEngineContext   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------
RC SetPDA_EGG(ENGINE_CONTEXT *pEngineContext,FILE *specFp,int newFlag)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  PDA1453A header;                                                              // record header
  short *indexes,                                                               // size of SpecMax arrays
         curvenum;                                                              // number of spectra in the file
  uint32_t  recordSize,                                                            // size of a record without SpecMax
        *recordIndexes;                                                         // save the position of each record in the file
  INDEX i;                                                                      // browse spectra in the file
  RC rc;                                                                        // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;

  pEngineContext->recordIndexesSize=2001;
  recordIndexes=pBuffers->recordIndexes;
  rc=ERROR_ID_NO;
  const int n_wavel = NDET[0];

  // Buffers allocation

  if ((indexes=(short *)MEMORY_AllocBuffer("SetPDA_EGG ","indexes",pEngineContext->recordIndexesSize,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL)
   rc=ERROR_ID_ALLOC;

  // Open spectra file

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetPDA_EGG",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
   	// Initializations

// !!! */    char *ptr,fileout[MAX_ITEM_TEXT_LEN];
// !!! */    FILE *fp;

// !!! */    ptr=strrchr(fileName,PATH_SEP);
// !!! */    sprintf(fileout,"e:%s",ptr);
// !!! */    fp=fopen(fileout,"a+b");

    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (!fread(&curvenum,sizeof(short),1,specFp) ||
        !fread(indexes,pEngineContext->recordIndexesSize*sizeof(short),1,specFp) ||
        (curvenum<=0))

     rc=ERROR_SetLast("SetPDA_EGG",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);

    else
     {
      // Save the position of each record in the file

// !!! */      fwrite(&curvenum,sizeof(short),1,fp);
// !!! */      fwrite(indexes,pEngineContext->recordIndexesSize*sizeof(short),1,fp);
// !!! */      fclose(fp);

      pEngineContext->recordNumber=curvenum;

      pEngineContext->recordSize=recordSize=(int32_t)sizeof(PDA1453A)-
                                             sizeof(double)-                    // azimuth (only since dec 2000)
                                             sizeof(float);                     // elevation (only since 2003)

      fread(&header,recordSize,1,specFp);

      if (pEngineContext->project.instrumental.azimuthFlag)
       {
        pEngineContext->recordSize+=sizeof(double);
        if ((header.today.da_year>=2003) || (header.today.da_year==1993))
         pEngineContext->recordSize+=sizeof(float);
       }

      fseek(specFp,-((int32_t)recordSize),SEEK_CUR);

      recordSize=pEngineContext->recordSize+(int32_t)sizeof(unsigned short)*n_wavel+(300L*(sizeof(short)+sizeof(float))+8L)*newFlag;

//      recordSize=(int32_t)sizeof(PDA1453A)-((!pEngineContext->project.instrumental.azimuthFlag)?
//                       sizeof(double):0)+(int32_t)sizeof(unsigned short)*n_wavel+(300L*(sizeof(short)+sizeof(float))+8L)*newFlag;

      recordIndexes[0]=(int32_t)(pEngineContext->recordIndexesSize+1)*sizeof(short);      // file header : size of indexes table + curvenum

      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+recordSize+indexes[i]*sizeof(unsigned short);  // take size of SpecMax arrays into account
     }
   }

  // Release local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetPDA_EGG ","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliPDA_EGG
// -----------------------------------------------------------------------------
// PURPOSE       IASB PDA EG&G binary format read out
//
// INPUT         recordNo     index of record in file;
//               dateFlag     0 no date constraint; 1 a date selection is applied;
//               specFp       pointer to the spectra file;
//               namesFp      pointer to the names file if any;
//               darkFp       pointer to the dark current files if any;
//               newFlag      0 format used during intercomparison campaign at Camborne, ENGLAND, 94
//                            1 format used from spring 94 until now;
//
// OUTPUT        pEngineContext  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------
RC ReliPDA_EGG(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp,int newFlag)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  PDA1453A          header;                                                     // file header
  OBSERVATION_SITE *pSite;                                                      // pointer to the observation sites data
  char             fileNameShort[MAX_STR_SHORT_LEN+1],                         // temporary file name
                   *ptr,                                                        // pointer to part of the previous string
                    names[20];                                                  // name of the current spectrum
  unsigned short           *ISpectre,                                                   // spectrum in the original format
                   *ISpecMax,                                                   // maximum values for each scan
                   *TabNSomme;                                                  // scans number for each integration time
  double            SMax,                                                       // maximum value of the current not normalized spectrum
                    Max,                                                        // scaling factor used for normalizing spectrum before safe keeping in the file
                   *ObsScan,                                                    // dark current spectrum
                    longit,latit,azimuth,Tm,tmLocal;                            // temporary data
  int               azimuthFlag,                                                // 1 if format with azimuth angle; 0 otherwise
                    MAXTPS,                                                     // size of the integration time array to use
                    indexSite;                                                  // index of the observation site in the sites list
  float            *TabTint,*Tps;                                               // integration time arrays
  INDEX             i,j,k;                                                      // indexes for loops and arrays
  FILE             *fp;                                                         // file pointer
  SZ_LEN            nameLen;
  RC                rc;                                                         // return code

// !!! */    FILE *gp;
// !!! */    char fileout[MAX_ITEM_TEXT_LEN];

  // Initializations

  const int n_wavel = NDET[0];
  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  memset(fileNameShort,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileNameShort,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);

  ISpectre=ISpecMax=TabNSomme=NULL;
  azimuthFlag=pEngineContext->project.instrumental.azimuthFlag;
  memset(names,0,20);
  TabTint=NULL;
  ObsScan=NULL;
  fp=NULL;

  Tps=(azimuthFlag)?Tps2:Tps1;
  MAXTPS=(azimuthFlag)?MAXTPS2:MAXTPS1;

// !!! */    ptr=strrchr(fileName,PATH_SEP);
// !!! */    sprintf(fileout,"e:%s",ptr);
// !!! */    gp=fopen(fileout,"a+b");

  rc=ERROR_ID_NO;

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(unsigned short *)MEMORY_AllocBuffer("ReliPDA_EGG ","ISpectre",n_wavel,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ObsScan=(double *)MEMORY_AllocBuffer("ReliPDA_EGG ","ObsScan",n_wavel,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((ISpecMax=(unsigned short *)MEMORY_AllocBuffer("ReliPDA_EGG ","ISpecMax",2000,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((TabTint=(float *)MEMORY_AllocBuffer("ReliPDA_EGG ","TabTint",300,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
           ((TabNSomme=(unsigned short *)MEMORY_AllocBuffer("ReliPDA_EGG ","TabNSomme",300,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(int32_t)20L*((recordNo-1)*(1+!newFlag)+!newFlag),SEEK_SET);

    fseek(specFp,(int32_t)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||
         !fread(&header,pEngineContext->recordSize,1,specFp) ||
         !fread(ISpectre,sizeof(unsigned short)*n_wavel,1,specFp) ||
         (newFlag &&
        (!fread(TabTint,sizeof(float)*300,1,specFp) ||
         !fread(TabNSomme,sizeof(unsigned short)*300,1,specFp) ||
         !fread(&Max,sizeof(double),1,specFp))) ||
         !fread(ISpecMax,sizeof(unsigned short)*(header.Rejected+header.ScansNumber),1,specFp))

     rc=ERROR_ID_FILE_END;

    else
     {
      nameLen=strlen(names);

      if (((pEngineContext->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_ZENITHAL) && (names[nameLen-4]!='Z')) ||
          ((pEngineContext->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_OFFAXIS) && (names[nameLen-4]!='o')))

       rc=ERROR_ID_FILE_RECORD;

      else
       {
        // Get the maximum value of the read out spectrum

        for (i=1,SMax=(double)ISpectre[0];i<n_wavel;i++)
         if ((double)ISpectre[i]>SMax)
          SMax=(double)ISpectre[i];

        // Time deviation correction

        {
         int Heure, Min, Sec, Np, Jday;
         double  DHour, DMin, DifUT;
         FILE *fp;

         if ((ptr=strrchr(fileNameShort,PATH_SEP))!=NULL)
          ptr++;
         else
          ptr=fileNameShort;

         *ptr=0;
         if ((int)(&fileNameShort[MAX_STR_SHORT_LEN]-ptr)>=(int)strlen("Drift.ret"))
          strcpy(ptr,"Drift.ret");

         DHour = (double) header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;

         struct date today;
         today.da_year = header.today.da_year;
         today.da_mon = header.today.da_mon;
         today.da_day = header.today.da_day;
         Tm=ZEN_NbSec(&today,&header.now,0);

         fp = fopen ( fileNameShort, "rt" );

         if ( fp != NULL )
           {
             fscanf ( fp, "%d", &Np );
             for ( i=0; i<Np; i++ )
               {
                 fscanf ( fp, "%d %lf", &Jday, &DifUT );
                 if ( Jday < 1 )  Jday += 365;
                 if ( Jday == (int) ZEN_FNCaljda(&Tm) ) break;
               }

             if ( Jday == ZEN_FNCaljda(&Tm) )
               {
                 DHour -= (DifUT / 60.);  /* DifUT donne en minutes  */
                 Heure = (int) DHour;
                 DMin  = (double) (DHour-Heure) * 60.;
                 Min   = (int) DMin;
                 Sec   = (int)((double)(DMin-Min) * 60.);

                 header.now.ti_hour = (unsigned char)Heure;
                 header.now.ti_min  = (unsigned char)Min;
                 header.now.ti_sec  = (unsigned char)Sec;

                 Tm = ZEN_NbSec (&today, &header.now, 0 );

                 if ((indexSite=SITES_GetIndex(pEngineContext->project.instrumental.observationSite))!=ITEM_NONE)
                  {
                   pSite=&SITES_itemList[indexSite];

                   longit=-pSite->longitude;  // !!! sign is inverted
                   latit=pSite->latitude;

                   header.Zm = ZEN_FNTdiz(ZEN_FNCrtjul(&Tm),&longit,&latit,&azimuth);
                  }
               }

             fclose ( fp );
           }
        }

// ! ***/           fwrite(&header,sizeof(PDA1453A),1,gp);
// ! ***/           fwrite(ISpectre,sizeof(unsigned short)*n_wavel,1,gp);
// ! ***/           fwrite(TabTint,sizeof(float)*300,1,gp);
// ! ***/           fwrite(TabNSomme,sizeof(unsigned short)*300,1,gp);
// ! ***/           fwrite(&Max,sizeof(double),1,gp);
// ! ***/           fwrite(ISpecMax,sizeof(unsigned short)*(header.Rejected+header.ScansNumber),1,gp);
// ! ***/           fclose(gp);

        pRecord->present_datetime.thedate.da_day = header.today.da_day;
        pRecord->present_datetime.thedate.da_mon = header.today.da_mon;
        pRecord->present_datetime.thedate.da_year = header.today.da_year;
        memcpy(&pRecord->present_datetime.thetime, &header.now, sizeof(struct time));

        pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
        pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)0.;
        pRecord->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;

        tmLocal=pRecord->Tm+THRD_localShift*3600.;

        pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
        pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);
        pRecord->elevationViewAngle=(pRecord->present_datetime.thedate.da_year>=2003)?header.mirrorElv:(float)-1.;

        // User constraints

        if ((SMax==(double)0.) || (header.ScansNumber<=0) ||
            (dateFlag && (pRecord->localCalDay!=localDay)))

         rc=ERROR_ID_FILE_RECORD;

        else
         {
          // Build the original spectrum

          if (newFlag)
           {
            for (i=0;i<(header.Rejected+header.ScansNumber);i++)
             pBuffers->specMax[i]=(double)ISpecMax[i]; // *0.5;

            for (i=0;i<n_wavel;i++)
             pBuffers->spectrum[i]=(double)ISpectre[i]*Max/65000.; //SMax;
           }
          else
           {
            if (pBuffers->specMax!=NULL)
             for (i=0,Max=(double)0.;i<(header.Rejected+header.ScansNumber);i++)
              {
               Max+=(double)ISpecMax[i];
               pBuffers->specMax[i]=(double)ISpecMax[i];
              }

            for (i=0,Max/=(double)header.ScansNumber;i<n_wavel;i++ )
             pBuffers->spectrum[i]=(double)ISpectre[i]*Max/(SMax*2);
           }

          // Data on the current record

          pRecord->nSpecMax = header.Rejected+header.ScansNumber;
          pRecord->TDet     = header.Detector_Temp;
          pRecord->Tint     = header.Exposure_time;
          pRecord->NSomme   = header.ScansNumber;
          pRecord->Zm       = header.Zm;
          pRecord->SkyObs   = header.SkyObs;
          pRecord->rejected = header.Rejected;
          pRecord->ReguTemp = header.ReguTemp;

          pRecord->azimuthViewAngle  = (azimuthFlag)?(float)header.azimuth:(float)-1.;
          pRecord->TotalAcqTime=pRecord->TotalExpTime=(double)pRecord->Tint*header.ScansNumber;

          memcpy(pRecord->Nom,names,20);

          // Dark current correction

          if (pBuffers->darkCurrent!=NULL)
           for (i=0;i<n_wavel;i++)
            pBuffers->darkCurrent[i]=(double)0.;

          if (newFlag)
           {
            k=0;
            pRecord->TotalExpTime=(double)0.;

            while (TabNSomme[k]!=0)
             {
              pRecord->TotalExpTime+=(double)TabNSomme[k]*TabTint[k];

              for (j=0;j<MAXTPS;j++)
               if (TabTint[k]==Tps[j])
                break;

              if ((j!=MAXTPS) && (pBuffers->darkCurrent!=NULL) && (darkFp!=NULL) &&
                  ((int)STD_FileLength(darkFp)>=(int)((j+1)*sizeof(double)*n_wavel)))
               {
                fseek(darkFp,(int32_t)j*n_wavel*sizeof(double),SEEK_SET);
                fread(ObsScan,sizeof(double)*n_wavel,1,darkFp);

                for (i=0;i<n_wavel;i++)
                 pBuffers->darkCurrent[i]+=(double)ObsScan[i]*TabNSomme[k];
               }

              k++;
             }

            pRecord->TotalAcqTime=pRecord->TotalExpTime;
           }
          else if (pBuffers->darkCurrent!=NULL)
           {
            fseek(darkFp,(int32_t)(pEngineContext->recordIndexesSize+1)*sizeof(short)+(pEngineContext->recordSize+(int32_t)sizeof(unsigned short)*n_wavel)*(recordNo-1),SEEK_SET);

            fread(&header,pEngineContext->recordSize,1,darkFp);
            fread(ISpectre,sizeof(unsigned short)*n_wavel,1,darkFp);

            for (i=0;i<n_wavel;i++)
             pBuffers->darkCurrent[i]=(double)ISpectre[i];
           }

          // Dark current subtraction

          if (pRecord->NSomme!=0)
           for (i=0;i<n_wavel;i++)
            {
             pBuffers->spectrum[i]/=(double)pRecord->NSomme;
             if (pBuffers->darkCurrent!=NULL)
              pBuffers->darkCurrent[i]/=(double)pRecord->NSomme;
             pBuffers->spectrum[i]-=pBuffers->darkCurrent[i];
            }
         }
       }
     }
   }

//  **********************************
//  CONVERSION intO DATA LOGGER FORMAT
//  **********************************

  if (!rc)
   {
    FILE *logFp;
    char fileName[40];
                                // U for Hamamatsu
                                // V for Reticon
    sprintf(fileName,"ZARD%03d.%02dU",ZEN_FNCaljda(&pRecord->Tm),pRecord->present_datetime.thedate.da_year%100);

    if ((logFp=fopen(fileName,"a+t"))!=NULL)
     {
      Max=(double)pBuffers->spectrum[0];

      for (i=0;i<n_wavel;i++)
       if ((double)pBuffers->spectrum[i]>Max)
        Max=(double)pBuffers->spectrum[i];
                                 // H for Hamamatsu
                                 // R for Reticon
      sprintf(pRecord->Nom,"%-3d%02dHA%02d%02dZ%03d    ",ZEN_FNCaljda(&pRecord->Tm),pRecord->present_datetime.thedate.da_year%100,
                      pRecord->present_datetime.thetime.ti_hour,
                      pRecord->present_datetime.thetime.ti_min,
                (int)(pRecord->Zm*10.));
                                           // H for Hamamatsu
                                           // R for Reticon
      fprintf(logFp,"%04d/%02d/%02d %02d:%02d H %-20s %02d/%02d/%02d %02d:%02d:%02d %05d %05d %07.3f %4.1f %05d ",
                      pRecord->present_datetime.thedate.da_year,
                      pRecord->present_datetime.thedate.da_mon,
                      pRecord->present_datetime.thedate.da_day,
                      pRecord->present_datetime.thetime.ti_hour,
                      pRecord->present_datetime.thetime.ti_min,
                      pRecord->Nom,
                      pRecord->present_datetime.thedate.da_day,
                      pRecord->present_datetime.thedate.da_mon,
                      pRecord->present_datetime.thedate.da_year%100,
                      pRecord->present_datetime.thetime.ti_hour,
                      pRecord->present_datetime.thetime.ti_min,
                      pRecord->present_datetime.thetime.ti_sec,
                      pRecord->NSomme,                           // number of saved scans
                      pRecord->rejected,                         // number of rejected scans
                      pRecord->Tint,                             // integration time
                      pRecord->Zm,                               // zenith angle
                      (int)(Max+0.5));                             // scaling factor

      fprintf(logFp,"99999 99999 99999 99999 99999 99999 99999 99999 99999 99999 99999 99999 " );

      for (i=0;i<n_wavel;i++)
       fprintf(logFp,"%05d ",(pBuffers->spectrum[i]<=(double)0.)?0:
                        (int)(pBuffers->spectrum[i]*65000./Max));

      fprintf(logFp,"\n");
     }

    // Release the allocated buffer

    if (logFp!=NULL)
     fclose(logFp);
   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Release allocated buffers

  if (ISpectre!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","ISpectre",ISpectre);
  if (ObsScan!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","ObsScan",ObsScan);
  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","ISpecMax",ISpecMax);
  if (TabNSomme!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","TabNSomme",TabNSomme);
  if (TabTint!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","TabTint",TabTint);

  // Return

  return rc;
 }

