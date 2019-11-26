
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Read out spectra from the EASOE campaign
//  Name of module    :  EASOE-READ.C
//  Creation date     :  2 April 2004
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
//  This module contains routines to read data measured during the EASOE
//  campaign (Keflavik, ICELAND, 1992)
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetEASOE - Get the size and the number of records of the current file
//  ReliEASOE - EASOE record read out;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "winthrd.h"
#include "zenithal.h"

#include "doas.h"
#include "engine_context.h"

// ==========
// DEFINITION
// ==========

#pragma pack(push,1)

typedef struct easoe
 {
  int32_t t_int;                                                                   // exposure time
  short n_somm;                                                                 // number of accumulations
  SHORT_DATE present_day;                                                       // measurement date
  struct time present_time;                                                     // measurement time
  float  spectre[1025];                                                         // the measured spectrum
  char   SkyObs;                                                                // information on the sky
 }
EASOE;

#pragma pack(pop)

// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SetEASOE
// -----------------------------------------------------------------------------
// PURPOSE       Get the size and the number of records of the current file
//
// INPUT         specFp    pointer to the spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC SetEASOE(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *namesFp)
 {
  // Declarations

  char  names[20];                                                             // name of the current spectrum
  short *indexes,                                                               // size of SpecMax arrays
         curvenum;                                                              // number of spectra in the file
  uint32_t *recordIndexes;                                                         // save the position of each record in the file
  INDEX i;                                                                      // browse spectra in the file
  RC rc;                                                                        // return code

  // Initializations

  pEngineContext->recordIndexesSize=2001;
  recordIndexes=pEngineContext->buffers.recordIndexes;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if ((indexes=(short *)MEMORY_AllocBuffer("SetEASOE","indexes",pEngineContext->recordIndexesSize,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL)
   rc=ERROR_ID_ALLOC;

  // Open spectra file

  else if ((specFp==NULL) || (namesFp==NULL))
   rc=ERROR_SetLast("SetEASOE",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);
    fseek(namesFp,0L,SEEK_SET);

    if (!fread(&curvenum,sizeof(short),1,specFp) ||
        !fread(indexes,pEngineContext->recordIndexesSize*sizeof(short),1,specFp) ||
        (curvenum<=0))

     rc=ERROR_SetLast("SetEASOE",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);

    else
     {
      i=0;
      recordIndexes[0]=(int32_t)(pEngineContext->recordIndexesSize+1)*sizeof(short);    // file header : size of indexes table + curvenum

      if (namesFp!=NULL )
       {
        fseek(namesFp,0L,SEEK_SET);

        while (!feof(namesFp) && fread(names,16,1,namesFp))
         {
          recordIndexes[i]+=indexes[i];

          if (names[11]=='Z')                                                   // name of a zenith spectrum
           {
            i++;
            recordIndexes[i]=recordIndexes[i-1]+pEngineContext->recordSize;
           }
         }
       }

      pEngineContext->recordNumber=curvenum;
      pEngineContext->recordSize=(int32_t)sizeof(EASOE);

      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+pEngineContext->recordSize+indexes[i];    // take size of SpecMax arrays into account
     }
   }

  // Release local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetEASOE","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliEASOE
// -----------------------------------------------------------------------------
// PURPOSE       EASOE record read out
//
// INPUT         recordNo   index of record in file;
//               dateFlag   0 no date constraint; 1 a date selection is applied;
//               localDay   if dateFlag is 1, the calendar day for the
//                          reference spectrum to search for
//               specFp     pointer to the spectra file;
//               namesFp    pointer to the names file if any;
//               darkFp     pointer to the dark current files if any;
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliEASOE(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  EASOE             speRecord,drkRecord;                                        // resp. spectrum and dark current records
  char             names[20];                                                  // name of the current spectrum
  unsigned short           *ISpecMax;                                                   // scans number for each integration time
  double            tmLocal;                                                    // temporary data
  INDEX             i,j;                                                        // indexes for loops and arrays
  RC                rc;                                                         // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  ISpecMax=NULL;
  memset(names,0,20);

  rc=ERROR_ID_NO;

  memset(&speRecord,0,sizeof(EASOE));
  memset(&drkRecord,0,sizeof(EASOE));

  for (i=0;i<1025;i++)
   speRecord.spectre[i]=drkRecord.spectre[i]=(float)0.;

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliEASOE",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if ((ISpecMax=(unsigned short *)MEMORY_AllocBuffer("ReliEASOE","ISpecMax",2000,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Set file pointers

    i=j=0;

    if (namesFp!=NULL)
     {
      fseek(namesFp,0L,SEEK_SET);

      while (i<recordNo)
       {
        if (!fread(names,16,1,namesFp))
         break;

        if (names[11]=='O')                                                     // name of a dark current spectrum
         j++;
        if (names[11]=='Z')                                                     // name of a zenith spectrum
         i++;
       }
     }

    fseek(specFp,(int32_t)pEngineContext->buffers.recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    fread(&speRecord,pEngineContext->recordSize,1,specFp);                           // read out the zenith spectrum

    if ((darkFp!=NULL) && (j>0))
     {
      fseek(darkFp,(int32_t)sizeof(EASOE)*(j-1),SEEK_SET);
      fread(&drkRecord,pEngineContext->recordSize,1,darkFp);                         // read out the dark current
     }

    if (speRecord.n_somm>0)
     fread(ISpecMax,(speRecord.n_somm)<<1,1,specFp);

    // Invert spectra

    const int n_wavel = NDET[0];
    for (i=0;i<n_wavel;i++) {
      pEngineContext->buffers.spectrum[i]=(double)speRecord.spectre[n_wavel-i-1];
      pEngineContext->buffers.darkCurrent[i]=(double)drkRecord.spectre[n_wavel-i-1];
    }

    for (i=0;i<speRecord.n_somm;i++)
     pEngineContext->buffers.specMax[i]=(double)ISpecMax[i];

    pRecord->nSpecMax=speRecord.n_somm;

    // Data on the current record

    memcpy(pRecord->Nom,names,20);
    pRecord->present_datetime.thedate.da_day = speRecord.present_day.da_day;
    pRecord->present_datetime.thedate.da_mon = speRecord.present_day.da_mon;
    pRecord->present_datetime.thedate.da_year = speRecord.present_day.da_year;
    memcpy(&pRecord->present_datetime.thetime,&drkRecord.present_time,sizeof(struct time));

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
    pRecord->TimeDec=(double)speRecord.present_time.ti_hour+speRecord.present_time.ti_min/60.+speRecord.present_time.ti_sec/3600.;

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    pRecord->Tint     = (double)0.001*speRecord.t_int;
    pRecord->NSomme   = speRecord.n_somm;
    pRecord->SkyObs   = speRecord.SkyObs;

    pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)pRecord->NSomme*pRecord->Tint;

    if (dateFlag && (pRecord->localCalDay!=localDay))
     rc=ERROR_ID_FILE_RECORD;
   }

  // Release allocated buffers

  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliEASOE","ISpecMax",ISpecMax);

  // Return

  return rc;
 }
