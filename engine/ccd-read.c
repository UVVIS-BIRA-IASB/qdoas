
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  CCD read out routines
//  Name of module    :  CCD-READ.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in the WinDOAS package in 97
//
//  Author            :  Caroline FAYT
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
//  This module contains the routines needed to read the data from the CCD detectors
//  operated by the BIRA-IASB institute andthe Universite Libre of Brussels.
//  The following formats are supported :
//
//  - PRJCT_INSTR_FORMAT_CCD_HA_94 for a 1024x512 detector used at Harestua in 94;
//  - PRJCT_INSTR_FORMAT_CCD_OHP_96 for a 1024x512 detector used at OHP in summer 96;
//  - PRJCT_INSTR_FORMAT_CCD_EEV for a 1340x400 CCD detector built in 99 for the
//                               total solar eclipse (11 Augustus) and still in use;
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetCCD_EEV - set file pointers and get the number of records (1340x400/1340x100);
//  ReliCCD_EEV - 1340x400/1340x100 format read out;
//
//  SetCCD - set file pointers and get the number of records (1024x512,94-96);
//  ReliCCD - 1024x512 format read out (1024x512 detectors, Harestua 94);
//  ReliCCDTrack - 1024x512 format read out (1024x512 detectors, OHP 96);
//
//  CCD_LoadInstrumental - load instrumental functions needed to correct measurements;
//  CCD_ResetInstrumental - release the buffers allocated by CCD_LoadInstrumental;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <dirent.h>
#include <string.h>
#include <math.h>

#include "doas.h"
#include "winfiles.h"
#include "engine.h"
#include "engine_context.h"
#include "stdfunc.h"
#include "vector.h"
#include "zenithal.h"
#include "winthrd.h"

#define NCURVE      20
#define MAX_CAMERA_PICTURES 2000                                                // a picture should be captured every minute.  So 60x24 = 1440 pictures max per day

// ===================================
// 1340x400 DETECTORS (1999 until now)
// ===================================

// -------------------------------------
// Description of the header of a record
// -------------------------------------

#pragma pack(push,1)

typedef struct _ccdData
 {
  double      exposureTime;                                                     // exposure time
  int         totalTime;                                                        // the total measurement time
  int         lockedTemperature;                                                // 1 if temperature is locked, 0 otherwise
  double      targetTemperature;                                                // target temperature
  double      currentTemperature;                                               // current temperature

  int         roiWveStart,                                                      // region of interest
              roiWveEnd,
              roiWveGroup,
              roiSlitStart,
              roiSlitEnd,
              roiSlitGroup;


  SHORT_DATE  today;                                                            // measurement date and time
  struct time now;
  double      Tm;                                                               // measurement date and time in seconds
  double      Zm;                                                               // solar zenith angle
  double      Azimuth;                                                          // solar azimuth angle
  int         nrej,nacc;                                                        // resp. number of rejected spectra and number of accumulation
  int         saveTracks;                                                       // 1 if all tracks are saved, 0 if only the mean is saved
  int         vipCorrection;                                                    // 1 if a interpixel variability correction has been applied; 0 otherwise
  int         nlCorrection;                                                     // 1 if a nonlinearity correction has been applied; 0 otherwise
  short       nTint;                                                            // number of integration times
  double      trackerElevation;                                                  // elevation angle for off axis measurements
  int         filterUnused;
  int         filterNumber;
  double      headTemperature;
  short       alsFlag;                                                          // 1 for asl measurents (cfr Alexis)
  short       scanIndex;                                                        // scan index for asl measurements
  double      scanningAngle;
  char      doubleFlag;                                                       // for Alexis
  char      brusagVersion;
  float       diodes[4];
  float       targetElevation,targetAzimuth;
  char      saturatedFlag,ignoredFlag;
  float       wve1,wve2,flux1,flux2;
  char      ignored[910];                                                     // if completed with new data in the future, authorizes compatibility with previous versions
  float       compassAngle;
  float       pitchAngle;
  float       rollAngle;
  struct time startTime;
  struct time endTime;
  double      trackerAzimuth;
  int         measureType;                                                      // if completed with new data in the future, authorizes compatibility with previous versions
 }
CCD_DATA;

typedef struct cameraPicture
 {
     int timestamp;
     char fileName[MAX_STR_LEN];
 }
CAMERA_PICTURE;

const char *MAXDOAS_measureTypes[PRJCT_INSTR_MAXDOAS_TYPE_MAX]=
                                     { "None","Off axis","Direct sun","Zenith","Dark","Lamp","Bentham","Almucantar","Offset","Azimuth", "Principal plane", "Horizon", "Moon" };


// ------------------------------------------------------------------
// Different predefined integration times for dark current correction
// ------------------------------------------------------------------

#define MAXTPS 41

static double predTint[MAXTPS] =
 { 0.10,   0.20,   0.40,   0.50,   0.80,   1.00,   1.20,   1.50,   1.75,   2.00,   2.50,   3.00,   3.60,   4.30,   5.20,
   6.20,   7.50,   8.90,  11.00,  13.00,  16.00,  19.00,  22.00,  27.00,  32.00,
  38.00,  46.00,  55.00,  66.00,  80.00,  95.00, 115.00, 140.00, 160.00, 200.00,
  240.00, 280.00, 340.00, 410.00, 490.00, 590.00 };

static int ccdDarkCurrentOffset[MAXTPS];                                      // offset of measured dark currents for each integration time

CAMERA_PICTURE ccdImageFilesList[MAX_CAMERA_PICTURES];
char ccdCurrentImagePath[MAX_ITEM_TEXT_LEN];
int ccdImageFilesN=0;

void CCD_GetImageFilesList(SHORT_DATE *pFileDate,char *rootPath)
 {
     // Declarations

     char imageFileName[MAX_ITEM_TEXT_LEN];
     char dateStr[9],*ptr;
  struct dirent *fileInfo;
  int hour,minute,sec,newtimestamp;
  INDEX i;
  DIR *hDir;

  // Initializations

  sprintf(dateStr,"%d%02d%02d",pFileDate->da_year,pFileDate->da_mon,pFileDate->da_day);
  ccdImageFilesN=0;

  // Build the full path

  sprintf(ccdCurrentImagePath,"%s/%d/%02d/%02d",rootPath,pFileDate->da_year,pFileDate->da_mon,pFileDate->da_day);

  if (strlen(rootPath) && STD_IsDir(ccdCurrentImagePath))

   // Browse the directory

   for (hDir=opendir(ccdCurrentImagePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
    {
        sprintf(imageFileName,"%s/%s",ccdCurrentImagePath,fileInfo->d_name);

     if (!STD_IsDir(imageFileName) && ((ptr=strstr(fileInfo->d_name,dateStr))!=NULL))
      {
          sscanf(ptr+9,"%02d%02d%02d",&hour,&minute,&sec);
          newtimestamp=hour*3600+minute*60+sec;

       for (i=ccdImageFilesN;(i>0) && (newtimestamp<ccdImageFilesList[i-1].timestamp);i--)
        memcpy(&ccdImageFilesList[i],&ccdImageFilesList[i-1],sizeof(CAMERA_PICTURE));

          ccdImageFilesList[i].timestamp=newtimestamp;
       strcpy(ccdImageFilesList[i].fileName,imageFileName);

       ccdImageFilesN++;
      }
    }
 }

char *CCD_GetImageFile(INDEX indexImage)
 {
     return (indexImage==ITEM_NONE)?NULL:ccdImageFilesList[indexImage].fileName;
 }


INDEX CCD_SearchForImage(int timestampMin,int timestampMax)
 {
  // Declarations

  INDEX i,imin,imax;
  int timestamp;

  timestamp=(timestampMin+timestampMax)>>1;

  // Initializations

  i=imin=0;
  imax=ccdImageFilesN-1;

  // Dichotomic search

  while (imin+1<imax)
   {
       i=(imax+imin)>>1;

       if (timestamp<=ccdImageFilesList[imin].timestamp)
        i=imax=imin;
       else if (timestamp>=ccdImageFilesList[imax].timestamp)
        i=imin=imax;
       else if (timestamp<ccdImageFilesList[i].timestamp)
        imax=i;
       else if (timestamp>=ccdImageFilesList[i].timestamp)
        imin=i;
   }

  if ((imax>=imin) && (i>=imin) && (i<=imax))
   {
    if ((ccdImageFilesList[imin].timestamp>=timestampMin) && (ccdImageFilesList[imin].timestamp<=timestampMax))
     i=imin;
    else if ((ccdImageFilesList[imax].timestamp>=timestampMin) && (ccdImageFilesList[imax].timestamp<=timestampMax))
     i=imax;
    else
     i=ITEM_NONE;
   }
  else
   i=ITEM_NONE;

  // Return

  return i;
 }

// ------------------
// Read out functions
// ------------------



// -----------------------------------------------------------------------------
// FUNCTION      SetCCD_EEV
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records (1340x400/1340x100)
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD_EEV(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *darkFp)
 {
  // Declarations

  CCD_DATA header;                                                              // header of a record
  unsigned int  *recordIndexes;                                                 // indexes of records for direct access
  int      ccdX,ccdY,dataSize;                                                  // size of the detector
  INDEX    indexTps;                                                            // browse the predefined integration time
  int    offset;                                                              // offset to remove from the spectrum
  RC       rc;                                                                  // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SetCCD_EEV",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  recordIndexes=pEngineContext->buffers.recordIndexes;

  memset(ccdDarkCurrentOffset,-1,sizeof(int)*MAXTPS);
  pEngineContext->recordIndexesSize=2001;
  pEngineContext->recordNumber=0;
  ENGINE_refStartDate=1;

  rc=ERROR_ID_NO;

  // Check the input file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Read the header of the first record

    fseek(specFp,0L,SEEK_SET);
    memset(recordIndexes,0L,sizeof(int)*pEngineContext->recordIndexesSize);

    fread(&header,sizeof(CCD_DATA),1,specFp);                             // Get date and time of the first record

       memcpy(&pEngineContext->fileInfo.startDate,&header.today,sizeof(SHORT_DATE));
       memcpy(&pEngineContext->fileInfo.startTime,&header.now,sizeof(struct time));

       fseek(specFp,0L,SEEK_SET);

    while (!feof(specFp) && fread(&header,sizeof(CCD_DATA),1,specFp))
     {
      ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
      dataSize=(header.doubleFlag==(char)1)?sizeof(double):sizeof(unsigned short);

      if (!pEngineContext->recordNumber)
       CCD_GetImageFilesList(&header.today,pEngineContext->project.instrumental.imagePath);

      pEngineContext->recordNumber++;

      recordIndexes[pEngineContext->recordNumber]=(int)
        recordIndexes[pEngineContext->recordNumber-1]+
        sizeof(CCD_DATA)+((header.saveTracks)?ccdX*ccdY*dataSize:ccdX*dataSize);

      // 15/02/2003 : dark current acquisition

      if ((header.nTint>0) &&
         ((header.today.da_year>2003) ||
         ((header.today.da_year==2003) && (header.today.da_mon>2)) ||
         ((header.today.da_year==2003) && (header.today.da_mon==2) && (header.today.da_day>14))))

       recordIndexes[pEngineContext->recordNumber]+=
        (sizeof(double)+sizeof(short))*header.nTint+sizeof(double)*(header.nrej+header.nacc);


      fseek(specFp,recordIndexes[pEngineContext->recordNumber],SEEK_SET);
     }

    // Dark currents

    if ((darkFp!=NULL) && (pEngineContext->buffers.darkCurrent!=NULL))
     {
      fseek(darkFp,0L,SEEK_SET);
      offset=0L;

      while (!feof(darkFp) && fread(&header,sizeof(CCD_DATA),1,darkFp))
       {
        ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
        ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
        dataSize=(header.doubleFlag==(char)1)?sizeof(double):sizeof(unsigned short);

        if (header.measureType==PRJCT_INSTR_MAXDOAS_TYPE_DARK)
         {
          for (indexTps=0;indexTps<MAXTPS;indexTps++)
           if (header.exposureTime==predTint[indexTps])
            break;

          if (indexTps<MAXTPS)
           ccdDarkCurrentOffset[indexTps]=offset;

         }

        offset+=sizeof(CCD_DATA)+((header.saveTracks)?ccdX*ccdY*dataSize:ccdX*dataSize);
        fseek(darkFp,offset,SEEK_SET);
       }
     }

    // Eclipse 99

/*  if (!(fileLength=filelength(fileno(specFp))) || !fread(&header,sizeof(CCD_DATA),1,specFp))
     THRD_Error(ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_EMPTY),"SetCCD_EVV1340x400 ",pEngineContext->fileInfo.fileName);
    else
     {
      // Retrieve the really used size of the detector

      NDET=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdSizeY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
      pEngineContext->recordSize=sizeof(CCD_DATA)+sizeof(unsigned short)*NDET*ccdSizeY;
      pEngineContext->recordNumber=fileLength/pEngineContext->recordSize;
     }  */
   }

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SetCCD_EEV",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCD_EEV
// -----------------------------------------------------------------------------
// PURPOSE       1340x400/1340x100 format read out
//
// INPUT         pEngineContext : information on the current file
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               darkFp    : pointer to the dark currents file if any
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : if the input file pointer is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_EMPTY     : if the file is not large enough;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCD_EEV(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CCD_DATA header;                                                              // header of the current record
  double *dspectrum,*tmpSpectrum;                                               // pointer to the vector in 'pEngineContext' to fill with the current spectrum
  unsigned short *spectrum;                                                             // spectrum to retrieve from the current record in the original format
  int ccdX,ccdY,spSize;                                                // dimensions of the CCD detector
  unsigned short *ccdTabNTint;                                                          // number of scans per different integration times
  double *ccdTabTint;                                                           // the different integration times used for the current measurement
  unsigned short *darkCurrent;                                                          // the dark current
  int nTint;                                                                    // the number of different integration times
//  double offset;                                                                // offset correction
  double tmLocal;
  INDEX i,j,k;                                                                  // indexes to browse vectors
  int nsec1,nsec2;
  int measurementType;
  RC rc,rcFread;                                                                // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ReliCCD_EEV",DEBUG_FCTTYPE_FILE);
  #endif
  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  dspectrum=pBuffers->spectrum;
  tmpSpectrum=NULL;
  ccdTabNTint=NULL;
  ccdTabTint=NULL;
  spectrum=NULL;
  darkCurrent=NULL;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    for(i=0;i<n_wavel;i++)
     dspectrum[i]=(double)0.;

    // Set file pointers

    fseek(specFp,(int32_t)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (!fread(&header,sizeof(CCD_DATA),1,specFp))
     rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
    else
     {
      // get the size of the CCD

      ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

      spSize=(header.saveTracks)?ccdX*ccdY:ccdX;
      nTint=header.nTint;

      // Buffers allocation

      if (((spectrum=(unsigned short *)MEMORY_AllocBuffer("ReliCCD_EEV","spectrum",spSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((tmpSpectrum=(double *)MEMORY_AllocBuffer("ReliCCD_EEV","tmpSpectrum",spSize,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
          ((darkCurrent=(unsigned short *)MEMORY_AllocBuffer("ReliCCD_EEV","darkCurrent",spSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((ccdTabNTint=(unsigned short *)MEMORY_AllocBuffer("ReliCCD_EEV","ccdTabNTint",300,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((ccdTabTint=(double *)MEMORY_AllocBuffer("ReliCCD_EEV","ccdTabTint",300,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL))

       rc=ERROR_ID_ALLOC;

       else
        {
         memset(spectrum,0,sizeof(unsigned short)*spSize);
         memset(darkCurrent,0,sizeof(unsigned short)*spSize);

         for(i=0;i<n_wavel;i++)
          tmpSpectrum[i]=(double)0.;

         rcFread=(header.doubleFlag==(char)1)?
                  fread(tmpSpectrum,sizeof(double)*spSize,1,specFp):
                  fread(spectrum,sizeof(unsigned short)*spSize,1,specFp);

         if (!rcFread)
          rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
         else if ((header.nTint>0) &&
                 (!fread(ccdTabTint,sizeof(double)*header.nTint,1,specFp) ||
                  !fread(ccdTabNTint,sizeof(short)*header.nTint,1,specFp) ||
                  !fread(pBuffers->specMax,sizeof(double)*(header.nacc+header.nrej),1,specFp)))
          rc=ERROR_ID_FILE_END;
         else if (!header.saveTracks)
          {
              if (header.doubleFlag==(char)1)
               memcpy(dspectrum,tmpSpectrum,sizeof(double)*spSize);
              else
            for (i=0;i<spSize;i++)
             dspectrum[i]=(double)spectrum[i];
          }
         else
          {
              if (header.doubleFlag==(char)1)
               {
             // Accumulate spectra

             for (i=0;i<n_wavel;i++)
              {
               dspectrum[i]=(double)0.;
               for (j=0;j<ccdY;j++)
                dspectrum[i]+=tmpSpectrum[n_wavel*j+i];
               dspectrum[i]/=ccdY;
              }
               }
              else
               {
             // Accumulate spectra

             for (i=0;i<n_wavel;i++)
              {
               dspectrum[i]=(double)0.;
               for (j=0;j<ccdY;j++)
                dspectrum[i]+=spectrum[n_wavel*j+i];
               dspectrum[i]/=ccdY;
              }
            }
          }

         if (!rc)
          {
           // Data on the current spectrum

           pRecord->nSpecMax  = header.nacc+header.nrej;
           pRecord->Tint      = (double)header.exposureTime;
           pRecord->NSomme    = header.nacc;
           pRecord->Zm        = header.Zm;
           pRecord->Tm        = header.Tm;
           pRecord->NTracks   = (header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
           pRecord->rejected  = header.nrej;
           pRecord->SkyObs    = 8;
           pRecord->TDet      = (double)header.currentTemperature;
           pRecord->ReguTemp  = (double)0.;

           pRecord->als.alsFlag=0;
           pRecord->als.scanIndex=-1;
           pRecord->als.scanningAngle=
           pRecord->als.compassAngle=
           pRecord->als.pitchAngle=
           pRecord->als.rollAngle=(float)0.;

           pRecord->ccd.diodes[0]=pRecord->ccd.diodes[1]=pRecord->ccd.diodes[2]=pRecord->ccd.diodes[3]=(float)0.;
           pRecord->ccd.targetElevation=header.targetElevation;
           pRecord->ccd.targetAzimuth=header.targetAzimuth;

           pRecord->ccd.saturatedFlag=0;

           if ((header.wve1>=200.) && (header.wve1<=1000.))
            {
             pRecord->ccd.wve1=header.wve1;
             pRecord->ccd.wve2=header.wve2;
             pRecord->ccd.flux1=header.flux1;
             pRecord->ccd.flux2=header.flux2;
            }
           else
            {
             pRecord->ccd.wve1=
             pRecord->ccd.wve2=
             pRecord->ccd.flux1=
             pRecord->ccd.flux2=0.;
            }

           if (header.alsFlag)
            {
                pRecord->als.alsFlag=header.alsFlag;
             pRecord->als.scanIndex=header.scanIndex;
             pRecord->als.scanningAngle=header.scanningAngle;                                          // total number of spectra in tracks
             pRecord->als.compassAngle=header.compassAngle;
             pRecord->als.pitchAngle=header.pitchAngle;
             pRecord->als.rollAngle=header.rollAngle;

             strcpy(pRecord->als.atrString,header.ignored);

             pRecord->ccd.filterNumber=0;
             pRecord->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;
             pRecord->ccd.headTemperature=(double)0.;
            }
           else
            {
              pRecord->ccd.filterNumber=header.filterNumber;
              pRecord->ccd.headTemperature=header.headTemperature;

              if ((pRecord->maxdoas.measurementType=header.measureType)==PRJCT_INSTR_MAXDOAS_TYPE_AZIMUTH)
                pRecord->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;


              // This part is not elegant at all but the code should be more consistent in the acquisition program according to the different situations with the pointing mode

              if (pRecord->maxdoas.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFSET)
               {
                   pRecord->ccd.targetElevation=header.targetElevation=-1.;
                   pRecord->ccd.targetAzimuth=header.targetAzimuth=-1.;
               }
              else if ((fabs(header.trackerElevation+1.)<EPSILON) && (fabs(header.trackerAzimuth+1.)<EPSILON))  // NO sun tracker
               {
                   pRecord->ccd.targetElevation=header.targetElevation=-1.;
                   pRecord->ccd.targetAzimuth=header.targetAzimuth=-1.;

                   pRecord->maxdoas.measurementType=header.measureType=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;
               }
              else
               {
                if (fabs(header.targetElevation-header.targetAzimuth)<EPSILON)
                 {
                  header.targetElevation=floor(header.trackerElevation+0.5);
                  header.targetAzimuth=floor(header.trackerAzimuth+0.5);
                 }
//                 else
//                  {
//                      if (fabs(header.targetElevation+1.)<EPSILON)
//                          header.targetElevation=floor(header.trackerElevation+0.5);
//                      if (fabs(header.targetAzimuth+1.)<EPSILON)
//                          header.targetAzimuth=floor(header.trackerAzimuth+0.5);
//                     }

                   pRecord->ccd.targetElevation=header.targetElevation;
                   pRecord->ccd.targetAzimuth=header.targetAzimuth;
               }
            }

           pRecord->present_datetime.thedate.da_day = header.today.da_day;
           pRecord->present_datetime.thedate.da_mon = header.today.da_mon;
           pRecord->present_datetime.thedate.da_year = header.today.da_year;

           memcpy(&pRecord->present_datetime.thetime,&header.now,sizeof(struct time));

           pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);


           memcpy(&pRecord->startDateTime.thetime,&header.startTime,sizeof(struct time));
           memcpy(&pRecord->endDateTime.thetime,&header.endTime,sizeof(struct time));

           nsec1=pRecord->startDateTime.thetime.ti_hour*3600+pRecord->startDateTime.thetime.ti_min*60+pRecord->startDateTime.thetime.ti_sec;
           nsec2=pRecord->endDateTime.thetime.ti_hour*3600+pRecord->endDateTime.thetime.ti_min*60+pRecord->endDateTime.thetime.ti_sec;

           if (nsec2<nsec1)
            nsec2+=86400;

           pRecord->TotalExpTime=(double)nsec2-nsec1;
           pRecord->TotalAcqTime = (double)pRecord->NSomme*pRecord->Tint;

           pRecord->ccd.indexImage=CCD_SearchForImage((int)header.startTime.ti_hour*3600+header.startTime.ti_min*60+header.startTime.ti_sec,(int)header.endTime.ti_hour*3600+header.endTime.ti_min*60+header.endTime.ti_sec);

           pRecord->elevationViewAngle=
                ((pRecord->present_datetime.thedate.da_year>2003) ||
                ((pRecord->present_datetime.thedate.da_year==2003) && (pRecord->present_datetime.thedate.da_mon>2)) ||
                ((pRecord->present_datetime.thedate.da_year==2003) && (pRecord->present_datetime.thedate.da_mon==2) && (pRecord->present_datetime.thedate.da_day>20)))?
                 (float)header.targetElevation:(float)-1.;

           pRecord->Azimuth=header.Azimuth;

           if (!header.alsFlag)
            pRecord->azimuthViewAngle=
                 ((pRecord->present_datetime.thedate.da_year>2008) ||
                 ((pRecord->present_datetime.thedate.da_year==2008) && (pRecord->present_datetime.thedate.da_mon>4)))?
                  (float)header.targetAzimuth:(float)-1.;                       // trackerAzimuth

           pRecord->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;
           pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);
           memset(pRecord->Nom,0,20);

           // Build dark current

           if (pRecord->NSomme==0)
            rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"number of accumulations");
           else if ((pBuffers->darkCurrent!=NULL) && (darkFp!=NULL))
            {
             for (i=0;i<n_wavel;i++)
              pBuffers->darkCurrent[i]=(double)0.;

             pRecord->TotalAcqTime=0.;

             for (k=0;k<nTint;k++)
              {
               pRecord->TotalAcqTime+=(double)ccdTabNTint[k]*ccdTabTint[k];

               for (j=0;j<MAXTPS;j++)
                if (ccdTabTint[k]<=predTint[j])
                 break;

               if (j<MAXTPS)
                {
                 if ((j>0) && (predTint[j]!=ccdTabTint[k]) && (predTint[j]-ccdTabTint[k]>ccdTabTint[k]-predTint[j-1]))
                  j--;

                 if (ccdDarkCurrentOffset[j]!=-1)
                  {
                   fseek(darkFp,ccdDarkCurrentOffset[j],SEEK_SET);

                   if (!fread(&header,sizeof(CCD_DATA),1,darkFp) && (!header.roiWveGroup || !header.roiSlitGroup))
                    rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"header is corrupted");
                   else
                    {
                     ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
                     ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

                     spSize=(header.saveTracks)?ccdX*(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup:ccdX;

                        fread(darkCurrent,sizeof(unsigned short)*spSize,1,darkFp);

                     for (i=0;i<n_wavel;i++)
                      {
                       if (!header.saveTracks)
                        pBuffers->darkCurrent[i]+=darkCurrent[i]*ccdTabNTint[k];
                       else
                        {
                         for (j=0;j<ccdY;j++)
                          pBuffers->darkCurrent[i]+=(double)darkCurrent[n_wavel*j+i]*ccdTabNTint[k];
                         pBuffers->darkCurrent[i]/=ccdY;
                        }
                      }
                    }
                  }
                }
              }

             if (pRecord->NSomme)
              for (i=0;i<n_wavel;i++)
               {
                   pBuffers->darkCurrent[i]/=(double)pRecord->NSomme;              // if we have different integration times for the spectrum
                pBuffers->spectrum[i]-=pBuffers->darkCurrent[i];
               }
            }

           tmLocal=pRecord->Tm+THRD_localShift*3600.;
           pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
           measurementType=pEngineContext->project.instrumental.user;

           // before 02/02/2016 if (rc ||
           // before 02/02/2016    (dateFlag && (((pRecord->elevationViewAngle>0.) && (pRecord->elevationViewAngle<80.)) ||
           // before 02/02/2016                  ((pRecord->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_NONE) && (pRecord->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH)))) ||                    // reference spectra are zenith only
           // before 02/02/2016    (!dateFlag && pEngineContext->analysisRef.refScan && !pEngineContext->analysisRef.refSza && (pRecord->elevationViewAngle>80.)))    // zenith sky spectra are not analyzed in scan reference selection mode
           
           if (rc || 
            (((measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecord->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecord->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH)) ||
             ((measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_NONE) && (pRecord->maxdoas.measurementType!=measurementType))) ||
              (dateFlag &&
            (((fabs(pRecord->elevationViewAngle+1.)>EPSILON) || (fabs(pRecord->azimuthViewAngle+1.)>EPSILON)) &&
             ((pRecord->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
              (pRecord->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol)))))

            rc=ERROR_ID_FILE_RECORD;

          }
       }
     }
   }

  // Straylight correction

  if (!rc && (pRecord->ccd.vip.matrix!=NULL) && (THRD_browseType==THREAD_BROWSE_SPECTRA))
   {
       memcpy(tmpSpectrum,dspectrum,sizeof(double)*n_wavel);

       for (i=0;i<n_wavel;i++)
        {
            dspectrum[i]=(double)0.;

            for (j=0;j<n_wavel;j++)
             dspectrum[i]+=pRecord->ccd.vip.matrix[j%NCURVE][(j/NCURVE)*n_wavel+i]*tmpSpectrum[j];
        }
   }

  // Release the allocated buffers

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","spectrum",spectrum);
  if (tmpSpectrum!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","tmpSpectrum",tmpSpectrum);
  if (darkCurrent!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","darkCurrent",darkCurrent);
  if (ccdTabNTint!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","ccdTabNTint",ccdTabNTint);
  if (ccdTabTint!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","ccdTabTint",ccdTabTint);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ReliCCD_EEV",rc);
  #endif

  // Return
  
  return rc;
 }

// ==========================
// 1024x512 DETECTORS (94-96)
// ==========================

// -----------------------------
// Description of records header
// -----------------------------

typedef struct _ccd_1024
 {
  short        Scans;                                                           //  equivalent to the value for I in the custom scan programs
  short        Memories;                                                        //  equivalent to the value for J in the custom scan programs
  short        Ignored_Scans;                                                   //  equivalent to the value for K in the custom scan programs
  short        Prep_Frames;                                                     //  equivalent to the value for H in the custom scan programs
  short        Detector_Temp;                                                   //  detector temperature in degrees Celsius
  short        Cooler_Locked;                                                   //  cooler locked flag
  short        Shutter_Open_Sync_Index;                                         //  TRUE if wait for sync pulse to open shutter
  short        Shutter_Close_Sync_Index;                                        //  TRUE if wait for sync pulse to close shutter
  short        Shutter_Forced_Mode;                                             //  0 = normal.  1 = closed.  2 = open.
  float        Exposure_Time;                                                   //  current exposure time
  short        Tracks;                                                          //  number of tracks per memory
  short        Number_of_Points;                                                //  number of points per tracks
  short        X0;                                                              //  start column of X axis
  short        Y0;                                                              //  start row of Y axis
  short        ActiveX;                                                         //  array size, number of channels in the X axis
  short        ActiveY;                                                         //  array size, number of channels in the Y axis
  float        ReguTemp;                                                        //  bin size for uniform columns
  short        Slicemode;                                                       //  0 = uniform   1 = non uniform.
  short        Trackmode;                                                       //  0 = contiguous   1 = Random.
  short        Shiftmode;                                                       //  0 = CCD   1 = diode array   2 = streak camera
  short        rejected;                                                        //  number of rejected spectra ( too low or too high signal )
  SHORT_DATE   Day;                                                             //  date of the measurement
  struct time  Hour;                                                            //  hour of the measurement
  double       Zm;                                                              //  zenith angle
  char         SkyObs;                                                          //  sky observation
 }
CCD_1024;

#pragma pack(pop)

// -----------------------------------------------------------------------------
// FUNCTION      SetCCD
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records (1024x512 detectors,94-96)
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//               flag      : 0 for OHP 96, 1 for Harestua 94
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD (ENGINE_CONTEXT *pEngineContext,FILE *specFp,int flag)
 {
  // Declarations

  int  recordSize;                                                              // the size of one record in bytes (without SpecMax)
  unsigned int *recordIndexes;                                                  // the position of spectra in bytes from the beginning of the file
  short *indexes,                                                               // the number of accumulations per spectrum (give by the same way,
                                                                                // the size of the SpecMax vectors)
         curvenum;                                                              // the number of spectra in the file
  INDEX  i;                                                                     // index for browsing spectra
  RC     rc;                                                                    // return code

  // Initializations

  pEngineContext->recordIndexesSize=2001;
  pEngineContext->recordNumber=0;
  recordSize=0L;
  rc=ERROR_ID_NO;
  const int n_wavel = NDET[0];

  // Buffers allocation

  recordIndexes=pEngineContext->buffers.recordIndexes;

  if ((indexes=(short *)MEMORY_AllocBuffer("SetCCD","indexes",pEngineContext->recordIndexesSize,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL)
   rc=ERROR_ID_ALLOC;

  // Check the input file pointer

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (fread(&curvenum,sizeof(short),1,specFp) &&                              // number of spectra in the file
        fread(indexes,pEngineContext->recordIndexesSize*sizeof(short),1,specFp) &&   // number of accumulations (size of SpecMax vectors) per measurement
        (curvenum>0))
     {
      pEngineContext->recordNumber=curvenum;

      // Calculate the size of a record (without SpecMax)

      if (flag==0)
       recordSize=(int32_t)sizeof(CCD_1024)+44L*(sizeof(float)+sizeof(unsigned short)*n_wavel) + (int32_t)(sizeof(float)+sizeof(short))*300;
      else if (flag==1)
       recordSize=(int32_t)sizeof(CCD_1024)+(int32_t)sizeof(short)*n_wavel;

      // Calculate the position in bytes from the beginning of the file for each spectra

      recordIndexes[0]=(int32_t)(pEngineContext->recordIndexesSize+1)*sizeof(short);    // file header : size of indexes table + curvenum


      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+                                     // position of the previous record
                        recordSize+                                             // size of the record (without SpecMax)
                        indexes[i]*sizeof(short);                               // take SpecMax of the previous record into account
     }
   }

  // Release the allocated local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetCCD","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCD
// -----------------------------------------------------------------------------
// PURPOSE       1024x512 format read out (Harestua 94)
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               namesFp   : pointer to the names file if any
//               darkFp    : pointer to the dark currents file if any
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : if the input file pointer is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy input criteria
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCD(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CCD_1024 DetInfo;                                                             // data on the current record retrieved from the spectra file
  char names[20];                                                              // name of the current spectrum
  unsigned short *ISpectre,*ISpecMax;                                                   // resp. spectrum and SpecMax retrieved from the file
  double Max,tmLocal;                                                           // the maximum value measured for the current spectrum
  INDEX i;                                                                      // index for browsing pixels in the spectrum
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  ISpectre=ISpecMax=NULL;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(unsigned short *)MEMORY_AllocBuffer("ReliCCD","ISpectre",n_wavel,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ISpecMax=(unsigned short *)MEMORY_AllocBuffer("ReliCCD","ISpecMax",n_wavel,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL))

     rc=ERROR_ID_ALLOC;
  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(int32_t)20L*(recordNo-1),SEEK_SET);

    fseek(specFp,(int32_t)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||
         !fread(&DetInfo,sizeof(CCD_1024),1,specFp) ||
         !fread(ISpectre,sizeof(unsigned short)*n_wavel,1,specFp) ||
         !fread(ISpecMax,sizeof(unsigned short)*(DetInfo.Scans+DetInfo.rejected),1,specFp))

     rc=ERROR_ID_FILE_END;

    else
     {
      // Data on the current spectrum

      pRecord->Tint      = (double)DetInfo.Exposure_Time;
      pRecord->NSomme    = DetInfo.Scans;
      pRecord->Zm        = DetInfo.Zm;
      pRecord->NTracks   = DetInfo.Tracks;
      pRecord->rejected  = DetInfo.rejected;
      pRecord->SkyObs    = DetInfo.SkyObs;
      pRecord->TDet      = (double)-40.;
      pRecord->ReguTemp  = (float)DetInfo.ReguTemp;

      pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)pRecord->NSomme*pRecord->Tint;
      pRecord->TimeDec=(double)DetInfo.Hour.ti_hour+DetInfo.Hour.ti_min/60.;

      pRecord->present_datetime.thedate.da_year  = DetInfo.Day.da_year;
      pRecord->present_datetime.thedate.da_mon   = DetInfo.Day.da_mon;
      pRecord->present_datetime.thedate.da_day   = DetInfo.Day.da_day;

      pRecord->present_datetime.thetime.ti_hour = DetInfo.Hour.ti_hour;
      pRecord->present_datetime.thetime.ti_min  = DetInfo.Hour.ti_min;
      pRecord->present_datetime.thetime.ti_sec  = DetInfo.Hour.ti_sec;

      pRecord->Tm = (double) ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);

      tmLocal=pRecord->Tm+THRD_localShift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

      memcpy(pRecord->Nom,names,20);

      // SpecMax (maxima per accumulation)

      for (i=0,Max=(double)ISpecMax[0];i<(DetInfo.Scans+DetInfo.rejected);i++)
       if ((pBuffers->specMax[i]=(double)ISpecMax[i])>Max)
        Max=(double)pBuffers->specMax[i];

      // Rebuild spectrum

      for (i=0;i<n_wavel;i++)
       pBuffers->spectrum[n_wavel-i-1]=(double)ISpectre[i]*Max/65000.*176.-908.25*44.;

      if ((DetInfo.Scans<=0) || (dateFlag && (pRecord->localCalDay!=localDay)))
       rc=ERROR_ID_FILE_RECORD;
     }
   }

  // Release the allocated buffers

  if (ISpectre!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD","ISpectre",ISpectre);
  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD","ISpecMax",ISpecMax);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCDTrack
// -----------------------------------------------------------------------------
// PURPOSE       1024x512 format read out (OHP 96)
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               namesFp   : pointer to the names file if any
//               darkFp    : pointer to the dark currents file if any
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : if the input file pointer is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCDTrack(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CCD_1024 DetInfo;                                                             // data on the current spectra retrieved from the spectra file
  char names[20],                                                              // name of the current spectrum
        fileName[DOAS_MAX_PATH_LEN+1];                                               // the complete file name (including path)
  unsigned short *ISpectre,*ISpecMax;                                                   // resp. spectrum and SpecMax retrieved from the file
  float *TabTint,*MaxTrk;                                                       // resp. integration time and track data
  int *TabNSomme;                                                               // scans data
  double *varPix,tmLocal;                                                       // instrumental function for interpixel variability correction
  INDEX i,j;                                                                    // indexes for browsing vectors
  FILE *varFp;                                                                  // pointer to the file for interpixel variability correction
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  ISpectre=ISpecMax=NULL;
  TabTint=MaxTrk=NULL;
  varPix=NULL;
  TabNSomme=NULL;
  varFp=NULL;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(unsigned short *)MEMORY_AllocBuffer("ReliCCDTrack","ISpectre",n_wavel,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ISpecMax=(unsigned short *)MEMORY_AllocBuffer("ReliCCDTrack","ISpecMax",2000,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((MaxTrk=(float *)MEMORY_AllocBuffer("ReliCCDTrack","MaxTrk",44,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
           ((varPix=(double *)MEMORY_AllocBuffer("ReliCCDTrack","varPix",n_wavel,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((TabTint=(float *)MEMORY_AllocBuffer("ReliCCDTrack","TabTint",300,sizeof(float),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((TabNSomme=(int *)MEMORY_AllocBuffer("ReliCCDTrack","TabNSomme",300,sizeof(int),0,MEMORY_TYPE_INT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(int32_t)20L*(recordNo-1),SEEK_SET);

    fseek(specFp,(int32_t)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||                      // name of the current record
         !fread(&DetInfo,sizeof(CCD_1024),1,specFp) ||                          // data on the current measurement
         !fread(MaxTrk,sizeof(float)*DetInfo.Tracks,1,specFp))                  // maximum value per track

     rc=ERROR_ID_FILE_END;

    else
     {
      // Read out the instrumental function for the interpixel variability correction

      if (strlen(pEngineContext->project.instrumental.vipFile)>0)
       {
        FILES_RebuildFileName(fileName,pEngineContext->project.instrumental.vipFile,1);
        varFp=fopen(fileName,"rb");
       }

      for (j=0;j<n_wavel;j++)
       {
        pBuffers->spectrum[j]=(double)0.;
        varPix[j]=(double)1.;
       }

      // Tracks accumulation and interpixel variability correction

      for (i=0;(i<DetInfo.Tracks) && !rc;i++)
       {
        if (!fread(ISpectre,sizeof(unsigned short)*n_wavel,1,specFp))
         rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
        else
         {
          if (varFp!=NULL)
           fread(varPix,sizeof(double)*n_wavel,1,varFp);

          for (j=0;j<n_wavel;j++)
           pBuffers->spectrum[j]+=(double)ISpectre[j]*MaxTrk[i]/varPix[j];
         }
       }

      if (!rc)
       {
        if (!fread(TabTint,sizeof(float)*300,1,specFp) || !fread(TabNSomme,sizeof(short)*300,1,specFp) ||
            !fread(ISpecMax,sizeof(unsigned short)*(DetInfo.Scans+DetInfo.rejected),1,specFp))

         rc=ERROR_ID_FILE_END;

        else
         {
          // Data on the current spectrum

          pRecord->TotalAcqTime=(double)0.;

          for (i=0;TabNSomme[i]>0;i++)
           pRecord->TotalAcqTime+=(double)TabNSomme[i]*TabTint[i];

          pRecord->TotalExpTime=pRecord->TotalAcqTime;

          for (i=0;i<(DetInfo.Scans+DetInfo.rejected);i++)
           pBuffers->specMax[i]=(double)ISpecMax[i]*4.;

          pRecord->nSpecMax  = DetInfo.Scans+DetInfo.rejected;

          pRecord->Tint      = (double)DetInfo.Exposure_Time;
          pRecord->NSomme    = DetInfo.Scans;
          pRecord->Zm        = DetInfo.Zm;
          pRecord->NTracks   = DetInfo.Tracks;
          pRecord->rejected  = DetInfo.rejected;
          pRecord->SkyObs    = DetInfo.SkyObs;
          pRecord->TDet      = (double)-40.;
          pRecord->ReguTemp  = (float)DetInfo.ReguTemp;

          pRecord->TimeDec=(double)DetInfo.Hour.ti_hour+DetInfo.Hour.ti_min/60.;

          pRecord->present_datetime.thedate.da_year  = DetInfo.Day.da_year;
          pRecord->present_datetime.thedate.da_mon   = DetInfo.Day.da_mon;
          pRecord->present_datetime.thedate.da_day   = DetInfo.Day.da_day;

          pRecord->present_datetime.thetime.ti_hour = DetInfo.Hour.ti_hour;
          pRecord->present_datetime.thetime.ti_min  = DetInfo.Hour.ti_min;
          pRecord->present_datetime.thetime.ti_sec  = DetInfo.Hour.ti_sec;

          pRecord->Tm = (double) ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);

          tmLocal=pRecord->Tm+THRD_localShift*3600.;

          pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
          pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

          memcpy(pRecord->Nom,names,20);

          VECTOR_Invert(pBuffers->spectrum,n_wavel);

          // spectra mean calculation

          if (pRecord->NSomme)
           for (i=0;i<n_wavel;i++)
            pBuffers->spectrum[i]/=(double)(44.*pRecord->NSomme);

          if ((DetInfo.Scans<=0) || (dateFlag && (pRecord->localCalDay!=localDay)))
           rc=ERROR_ID_FILE_RECORD;
         }
       }
     }
   }

  // Release the allocated buffers

  if (ISpectre!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","ISpectre",ISpectre);
  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","ISpecMax",ISpecMax);
  if (TabTint!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","TabTint",TabTint);
  if (MaxTrk!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","MaxTrk",MaxTrk);
  if (varPix!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","varPix",varPix);
  if (TabNSomme!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","TabNSomme",TabNSomme);

  // Close file

  if (varFp!=NULL)
   fclose(varFp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CCD_LoadInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Load instrumental functions needed to correct measurements
//
// INPUT         pEngineContext   information on the current file
//
// RETURN        the code returned by MATRIX_Load if the load of a file failed
//               ERROR_ID_NO, otherwise.
// -----------------------------------------------------------------------------

RC CCD_LoadInstrumental(ENGINE_CONTEXT *pEngineContext)
{
  char fileName[MAX_STR_LEN+1];                                                // the complete name (including path) of the file to load

  // Initializations

  const int n_wavel = NDET[0];
  RECORD_INFO *pRecord=&pEngineContext->recordInfo;
  PROJECT *pProject=&pEngineContext->project;
  PRJCT_INSTRUMENTAL *pInstrumental=&pProject->instrumental;
  RC rc=ERROR_ID_NO;

  // Offset

  if ((strlen(pInstrumental->instrFunction)>0) && (pRecord->ccd.drk.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pInstrumental->instrFunction,1),
                     &pRecord->ccd.drk,
                      n_wavel,NCURVE,
                      0.,0.,
                      0,0,"CCD_LoadInstrumental (offset)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Interpixel variability

  if ((strlen(pInstrumental->vipFile)>0) && (pRecord->ccd.vip.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pInstrumental->vipFile,1),
                     &pRecord->ccd.vip,
                      0,NCURVE,
                      0.,0.,
                      0,0,"CCD_LoadInstrumental (vip)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Non linearity of the detector

  if ((strlen(pInstrumental->dnlFile)>0) && (pRecord->ccd.dnl.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pInstrumental->dnlFile,1),
                     &pRecord->ccd.dnl,
                      0,0,
                      0.,0.,
                      1,0,"CCD_LoadInstrumental (dnl)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Return

  End_CCD_LoadInstrumental :

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CCD_ResetInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Release the buffers allocated by CCD_LoadInstrumental
//
// INPUT         pUlb : pointer to the instrumental correction matrices
// -----------------------------------------------------------------------------

void CCD_ResetInstrumental(CCD *pCCD)
 {
  if (pCCD->vip.matrix!=NULL)
   MATRIX_Free(&pCCD->vip,"CCD_ResetInstrumental (vip)");
  if (pCCD->dnl.matrix!=NULL)
   MATRIX_Free(&pCCD->dnl,"CCD_ResetInstrumental (dnl)");
  if (pCCD->drk.matrix!=NULL)
   MATRIX_Free(&pCCD->drk,"CCD_ResetInstrumental (offset)");
 }
