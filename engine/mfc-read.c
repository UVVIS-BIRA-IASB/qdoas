
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  MFC read out routines
//  Name of module    :  MFC-READ.C
//  Creation date     :  24 April 99
//  Ref               :  MFC program (see IUP Heidelberg)
//
//  Authors           :  Heidelberg
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
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  This module contains routines needed for reading out data in the MFC format
//  used at IUP Heidelberg.  Two formats are described in this package :
//
//        PRJCT_INSTR_FORMAT_MFC, the binary format;
//        PRJCT_INSTR_FORMAT_MFC_STD, the ASCII format;
//
//  See MFC references for further description of the format.
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetMFC - calculate the number of files in a directory in MFC format;
//
//  MFC_ReadRecord - record read out and processing in binary format;
//
//  ReliMFC - MFC binary format read out;
//
//  ReliMFCStd - MFC ASCII format read out;
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>
#include <ctype.h>
#include <dirent.h>

#include "mfc-read.h"

#include "doas.h"
#include "winfiles.h"
#include "engine.h"
#include "engine_context.h"
#include "kurucz.h"
#include "analyse.h"
#include "vector.h"
#include "zenithal.h"
#include "winthrd.h"
#include "stdfunc.h"
#include "visual_c_compat.h"

// ================
// GLOBAL VARIABLES
// ================

TBinaryMFC MFC_headerDrk,                                                       // header of the dark current file
           MFC_headerOff,                                                       // header of the offset file
           MFC_header,                                                          // header of the spectra file
           MFC_headerInstr;                                                     // header of the instrumental function file

char MFC_fileInstr[MAX_STR_SHORT_LEN+1],
  MFC_fileSpectra[MAX_STR_SHORT_LEN+1],
  MFC_fileMin[MAX_STR_SHORT_LEN+1],
  MFC_fileDark[MAX_STR_SHORT_LEN+1],
  MFC_fileOffset[MAX_STR_SHORT_LEN+1];

int mfcLastSpectrum=0;

RC MFC_ResetFiles(ENGINE_CONTEXT *pEngineContext)
 {
     MFC_DOASIS *pMfc=&pEngineContext->recordInfo.mfcDoasis;

  if (pMfc->fileNames!=NULL)
   MEMORY_ReleaseBuffer("MFC_ResetFiles","fileNames",pMfc->fileNames);
  pMfc->fileNames=NULL;

  pMfc->nFiles=0;

  return 0;
 }

RC MFC_LoadOffset(ENGINE_CONTEXT *pEngineContext)
 {
     // Declarations

     PROJECT *pProject;                                                            // pointer to the current project
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  PRJCT_MFC *pMfc;
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RC rc;

  // Initializations

  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;
  pMfc=&pInstrumental->mfc;
  pBuffers=&pEngineContext->buffers;

  rc=ERROR_ID_NO;

  strcpy(MFC_fileOffset,pInstrumental->offsetFile);

  const int n_wavel = NDET[0];

  // Read offset

  if (strlen(pInstrumental->offsetFile) && (pBuffers->offset!=NULL)) {                  // offset
    VECTOR_Init(pBuffers->offset,0.,n_wavel);

    rc=(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)?
        MFC_ReadRecord(pInstrumental->offsetFile,&MFC_headerOff,pBuffers->offset,
       &MFC_headerDrk,NULL,&MFC_headerOff,NULL,pMfc->mfcMaskOffset,pMfc->mfcMaskSpec,pMfc->mfcRevert): // remove offset from dark current
        MFC_ReadRecordStd(pEngineContext,pInstrumental->offsetFile,&MFC_headerOff,pBuffers->offset,
       &MFC_headerDrk,NULL,&MFC_headerOff,NULL);

    if (rc==ERROR_ID_FILE_END)
     rc=0;
   }

  // Return

  return rc;
 }

RC MFC_LoadDark(ENGINE_CONTEXT *pEngineContext)
 {
     // Declarations

     PROJECT *pProject;                                                            // pointer to the current project
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  PRJCT_MFC *pMfc;
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RC rc;

  // Initializations

  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;
  pMfc=&pInstrumental->mfc;
  pBuffers=&pEngineContext->buffers;

  rc=ERROR_ID_NO;

  strcpy(MFC_fileDark,pInstrumental->vipFile);

  const int n_wavel = NDET[0];

  // Read dark current

  if (strlen(pInstrumental->vipFile) && (pBuffers->varPix!=NULL)) {                  // dark current
    VECTOR_Init(pBuffers->varPix,0.,n_wavel);

    rc=(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)?
        MFC_ReadRecord(pInstrumental->vipFile,&MFC_headerDrk,pBuffers->varPix,
       &MFC_headerDrk,NULL,&MFC_headerOff,pBuffers->offset,pMfc->mfcMaskOffset,pMfc->mfcMaskSpec,pMfc->mfcRevert): // remove offset from dark current
        MFC_ReadRecordStd(pEngineContext,pInstrumental->vipFile,&MFC_headerDrk,pBuffers->varPix,
       &MFC_headerDrk,NULL,&MFC_headerOff,pBuffers->offset);

    if (rc==ERROR_ID_FILE_END)
     rc=0;
   }

  // Return

  return rc;
 }

RC MFC_AllocFiles(ENGINE_CONTEXT *pEngineContext)
 {
     // Declarations

  char           filePath[DOAS_MAX_PATH_LEN+1];
  char           fileName[DOAS_MAX_PATH_LEN+1];
  char          *ptr,fileExt[DOAS_MAX_PATH_LEN+1];
  char          *fileList;
  struct dirent *fileInfo;
  DIR *          hDir;
  int            fileNumber,indexFile;
  RC             rc;

  // Initialization

  fileNumber=0;

  if (!(rc=MFC_ResetFiles(pEngineContext)))
   {
    // Get file path

    strcpy(filePath,pEngineContext->fileInfo.fileName);

    if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
     {
      strcpy(filePath,".");
      ptr=pEngineContext->fileInfo.fileName;
     }
    else
     *ptr++=0;

    strcpy(pEngineContext->recordInfo.mfcDoasis.filePath,filePath);

    // Retrieve file extension

    if ((ptr=strrchr(ptr,'.'))!=NULL)
     strcpy(fileExt,ptr+1);
    else
     memset(fileExt,0,MAX_STR_SHORT_LEN+1);

    // Browse files in the folder to get the file number

    for (hDir=opendir(filePath),fileNumber=0;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
     {
      sprintf(fileName,"%s%c%s",filePath,PATH_SEP,fileInfo->d_name);

      if (!STD_IsDir(fileName) &&
          ((((ptr=strrchr(fileInfo->d_name,'.'))==NULL) && !strlen(fileExt)) ||
           ((strlen(ptr+1)==strlen(fileExt)) && !strcasecmp(ptr+1,fileExt))))

       fileNumber++;
     }

    if (hDir!=NULL)
     {
      closedir(hDir);
      hDir=NULL;
     }

    if (!fileNumber || ((pEngineContext->recordInfo.mfcDoasis.fileNames=(char *)MEMORY_AllocBuffer("MFC_AllocFiles","fileNames",fileNumber*(DOAS_MAX_PATH_LEN+1),1,0,MEMORY_TYPE_STRING))==NULL))
     rc=ERROR_ID_ALLOC;
    else
     {
         memset(pEngineContext->recordInfo.mfcDoasis.fileNames,0,fileNumber*(DOAS_MAX_PATH_LEN+1));
         pEngineContext->recordInfo.mfcDoasis.nFiles=fileNumber;
         fileList=pEngineContext->recordInfo.mfcDoasis.fileNames;

         // Browse files in the folder a second time and save the file names

      for (hDir=opendir(filePath),indexFile=0;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
       {
        sprintf(fileName,"%s%c%s",filePath,PATH_SEP,fileInfo->d_name);

        // TO CHECK WITH MPI PEOPLE !!!!!!!!!!!!!

        if (!STD_IsDir(fileName) &&
            ((((ptr=strrchr(fileInfo->d_name,'.'))==NULL) && !strlen(fileExt)) ||
             ((strlen(ptr+1)==strlen(fileExt)) && !strcasecmp(ptr+1,fileExt))))
         {
          if (!indexFile || (strcmp(&fileList[0],fileInfo->d_name)>=0))
           {
            for (int i=indexFile;i>0;i--)
             memcpy(&fileList[i*(DOAS_MAX_PATH_LEN+1)],&fileList[(i-1)*(DOAS_MAX_PATH_LEN+1)],DOAS_MAX_PATH_LEN+1);
            strcpy(&fileList[0],fileInfo->d_name);
           }
          else if (strcmp(&fileList[(indexFile-1)*(DOAS_MAX_PATH_LEN+1)],fileInfo->d_name)<=0)
           strcpy(&fileList[indexFile*(DOAS_MAX_PATH_LEN+1)],fileInfo->d_name);
          else
           {
            int ilow,ihigh,icur;

            ilow=0;
            ihigh=indexFile-1;
            icur=(ilow+ihigh)>>1;

            while (ilow<ihigh)
             {
              if ((strcmp(&fileList[icur],fileInfo->d_name)<=0) && (strcmp(&fileList[icur+1],fileInfo->d_name)>=0))
               break;
              else if (strcmp(&fileList[icur],fileInfo->d_name)<0)
               ilow=icur;
              else if (strcmp(&fileList[icur+1],fileInfo->d_name)>0)
               ihigh=icur;

              icur=(ilow+ihigh)>>1;
             }

            for (int i=indexFile;i>icur;i--)
             memcpy(&fileList[i*(DOAS_MAX_PATH_LEN+1)],&fileList[(i-1)*(DOAS_MAX_PATH_LEN+1)],DOAS_MAX_PATH_LEN+1);
            strcpy(&fileList[icur],fileInfo->d_name);
           }

          indexFile++;
         }
       }

      if (hDir!=NULL)
       {
        closedir(hDir);
        hDir=NULL;
       }
     }
   }

  // Return

  return rc;
 }

INDEX MFC_SearchForCurrentFileIndex(ENGINE_CONTEXT *pEngineContext)
 {
     // Declarations

     char  fileName[DOAS_MAX_PATH_LEN+1];                                        // name of the current file
     char *ptr,*filesList;
     INDEX   indexRecord,indexRecordLow,indexRecordCur,indexRecordHigh;
     int nFiles;
     RC rcCmp;

     // Initializations

     indexRecord=ITEM_NONE;
     filesList=pEngineContext->recordInfo.mfcDoasis.fileNames;
     nFiles=pEngineContext->recordInfo.mfcDoasis.nFiles;

     //  Browse files

     if (pEngineContext->analysisRef.refScan && (filesList!=NULL) && ((ptr=strrchr(pEngineContext->fileInfo.fileName,'/'))!=NULL) && (strlen(ptr+1)>0))
      {
       strcpy(fileName,ptr+1);

    // the record is not in the list

    if ((strcasecmp(filesList,fileName)>0) || (strcasecmp(filesList+(nFiles-1)*(DOAS_MAX_PATH_LEN+1),fileName)<0))
     indexRecord=ITEM_NONE;
    else
     {
      indexRecordLow=indexRecord=0;
      indexRecordHigh=nFiles;

      while (indexRecordHigh-indexRecordLow>1)
       {
           indexRecordCur=(indexRecordLow+indexRecordHigh)>>1;

           if (!(rcCmp=strcasecmp(filesList+indexRecordCur*(DOAS_MAX_PATH_LEN+1),fileName)))
         {
             indexRecord=indexRecordCur;
             break;
            }
           else if (rcCmp<0)
            indexRecordLow=indexRecordCur;
           else
            indexRecordHigh=indexRecordCur;
       }

      indexRecord++;   // because recordNo starts at 1 !!!
     }
      }

     // Return

     return indexRecord;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SetMFC
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of files in a directory in MFC format
//
// INPUT         specFp      pointer to the spectra file
//
// OUTPUT        pEngineContext   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_NO  no error;
// -----------------------------------------------------------------------------

RC SetMFC(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  char  fileName[MAX_STR_SHORT_LEN+1];                                        // name of the current file
  RC rc;

  // Initializations

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  pEngineContext->lastRefRecord=0;
  mfcLastSpectrum=0;
  rc=ERROR_ID_NO;

   pEngineContext->recordNumber=1;

  // Return

  return rc;
 }

// =================
// MFC BINARY FORMAT
// =================

// -----------------------------------------------------------------------------
// FUNCTION      MFC_ReadRecord
// -----------------------------------------------------------------------------
// PURPOSE       record read out and processing in MFC binary format
//
// INPUT         fileName          the name of the current file;
//               pHeaderDrk, drk   dark current data if any;
//               pHeaderOff, off   offset data if any;
//               mask              mask used for spectra selection;
//
// OUTPUT        pHeaderSpe, spe   resp. data on the current record and the spectrum
//                                 to process;
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file can't be found;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints;
//               ERROR_ID_ALLOC           buffer allocation error;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC MFC_ReadRecord(char *fileName,
                  TBinaryMFC *pHeaderSpe,double *spe,
                  TBinaryMFC *pHeaderDrk,double *drk,
                  TBinaryMFC *pHeaderOff,double *off,
                  unsigned int mask,unsigned int maskSpec,unsigned int revertFlag)
 {
  // Declarations

  float *specTmp;   // the original spectrum
  FILE *fp;         // pointer to the current file
  INDEX i;          // browse pixels in the spectrum
  RC rc;            // return code

  // Initializations

  rc=ERROR_ID_NO;
  specTmp=NULL;
  const int n_wavel = NDET[0];

  // Open file

  if ((fp=fopen(fileName,"rb"))==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if (!STD_FileLength(fp))
   rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

  // Allocate a buffer for the spectrum

  else if ((specTmp=(float *)MEMORY_AllocBuffer(__func__,"specTmp",n_wavel,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    for (i=0;i<n_wavel;i++)
     specTmp[i]=0.0f;

    if (!fread(pHeaderSpe,sizeof(*pHeaderSpe),1,fp) || // header
       ((mask!=maskSpec) && ((pHeaderSpe->ty&mask)==0) && ((unsigned int)pHeaderSpe->wavelength1!=mask)) ||                    // spectrum selection
        (pHeaderSpe->no_chan==0) || (pHeaderSpe->no_chan>n_wavel) || // verify the size of the spectrum
        !fread(specTmp,sizeof(*specTmp)*pHeaderSpe->no_chan,1,fp)) { // read spectrum
      memset(pHeaderSpe,0,sizeof(TBinaryMFC));
      pHeaderSpe->int_time= 0.0f;
      rc=ERROR_ID_FILE_BAD_FORMAT;
    } else {

      // Copy original spectrum to the output buffer

      for (i=0;i<n_wavel;i++)
       spe[i]=(double)specTmp[i];

      // Offset correction if any

      if ((off!=NULL) && (pHeaderOff->noscans>0) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET))
       {
        for (i=0;i<n_wavel;i++)
         spe[i]-=(double)off[i]*pHeaderSpe->noscans/pHeaderOff->noscans;
       }

      // Dark current correction if any

      if ((drk!=NULL) && (pHeaderDrk->int_time!=(float)0.) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET) && (THRD_browseType!=THREAD_BROWSE_MFC_DARK))
       {
        for (i=0;i<n_wavel;i++)
         spe[i]-=(double)drk[i]*pHeaderSpe->int_time/pHeaderDrk->int_time;  // The int_time field should contain the number of scans * the exposure time
       }

      if (revertFlag && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET) && (THRD_browseType!=THREAD_BROWSE_MFC_DARK))
       VECTOR_Invert(spe,n_wavel);
     }
   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Release the allocated buffer

  if (specTmp!=NULL)
   MEMORY_ReleaseBuffer(__func__,"specTmp",specTmp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliMFC
// -----------------------------------------------------------------------------
// PURPOSE       MFC binary format read out
//
// INPUT         recordNo     index of record in file;
//               dateFlag     0 no date constraint; 1 a date selection is applied;
//               specFp       pointer to the spectra file;
//               mfcMask      mask for spectra selection;
//
// OUTPUT        pEngineContext  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------
RC ReliMFC(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,unsigned int mfcMask)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  int                  day,mon,year,hour,min,sec,nsec,nsec1,nsec2,              // date and time fields
                       firstFile;                                               // number of the first file in the current directory
  char                fileName[MAX_STR_SHORT_LEN+1],                           // name of the current file (the current record)
                       format[20],
                      *ptr,*ptr2;                                               // pointers to parts in the previous string
  struct date          today;                                                   // date of the current record
  PRJCT_INSTRUMENTAL  *pInstrumental;                                           // pointer to the instrumental part of the project
  PRJCT_MFC           *pMfc;
  RC                   rc;                                                      // return code
  double tmLocal,Tm1,Tm2,timeshift;

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN+1);
  pInstrumental=&pEngineContext->project.instrumental;
  pMfc=&pInstrumental->mfc;
  rc=ERROR_ID_NO;

  if ((ptr=strrchr(fileName,PATH_SEP))==NULL)
   rc=ERROR_ID_FILE_RECORD;
  else if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber))
   {
    // Build the right file name

//    if (THRD_treeCallFlag)
     {
//      ptr+=2;
//      sscanf(ptr,"%d",&firstFile);
      if ((ptr2=strrchr(ptr,'.'))==NULL)
       ptr2=strrchr(ptr,'\0');

      for (ptr=ptr2;isdigit(*(ptr-1));ptr--);

      sscanf(ptr,"%d",&firstFile);
      sprintf(format,"%%0%dd%%s",(int)(ptr2-ptr));
      sprintf(ptr,format,firstFile+recordNo-1,ptr2);
     }

    // Record read out

    if (!(rc=MFC_ReadRecord(fileName,&MFC_header,pBuffers->spectrum,&MFC_headerDrk,pBuffers->varPix,&MFC_headerOff,pBuffers->offset,mfcMask,pMfc->mfcMaskSpec,pMfc->mfcRevert)))
     {
      if ((mfcMask==pMfc->mfcMaskSpec) &&
         (((pMfc->mfcMaskSpec!=(unsigned int)0) && ((unsigned int)MFC_header.ty==mfcMask)) ||
          ((pMfc->mfcMaskSpec==(unsigned int)0) && ((unsigned int)MFC_header.ty==pMfc->mfcMaskSpec))) &&
        (((double)pMfc->wavelength>(double)100.) && ((MFC_header.wavelength1<(double)pMfc->wavelength-5.) || (MFC_header.wavelength1>(double)pMfc->wavelength+5.))))

       rc=ERROR_ID_FILE_RECORD;

      // In automatic file selection, replace instrumental functions with new ones if found

      if ((mfcMask==pMfc->mfcMaskSpec) &&
        (((mfcMask!=(unsigned int)0) && ((unsigned int)MFC_header.ty!=mfcMask)) || ((mfcMask==(unsigned int)0) && (rc==ERROR_ID_FILE_RECORD) && ((unsigned int)MFC_header.wavelength1!=mfcMask))))
       {
        if (pMfc->mfcMaskUse)
         {
          if ((((MFC_header.ty&pMfc->mfcMaskInstr)!=0) || (MFC_header.wavelength1==pMfc->mfcMaskInstr)) && (pBuffers->instrFunction!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerInstr,pBuffers->instrFunction,
                          &MFC_headerDrk,pBuffers->varPix,               // instrument function should be corrected for dark current
                          &MFC_headerOff,pBuffers->offset,                  // instrument function should be corrected for offset
                          pMfc->mfcMaskInstr,pMfc->mfcMaskSpec,pMfc->mfcRevert);

            FILES_CompactPath(MFC_fileInstr,fileName,1,1);
           }
          else if ((((MFC_header.ty&pMfc->mfcMaskDark)!=0) || (MFC_header.wavelength1==pMfc->mfcMaskDark)) && (pBuffers->varPix!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerInstr,pBuffers->varPix,
                          &MFC_headerDrk,NULL,                            // no correction for dark current
                          &MFC_headerOff,pBuffers->offset,                  // dark current should be corrected for offset
                          pMfc->mfcMaskDark,pMfc->mfcMaskSpec,0);

            FILES_CompactPath(MFC_fileDark,fileName,1,1);
           }
          else if ((((MFC_header.ty&pMfc->mfcMaskOffset)!=0) || (MFC_header.wavelength1==pMfc->mfcMaskOffset)) &&  (pBuffers->offset!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerOff,pBuffers->offset,
                          &MFC_headerDrk,NULL,                            // no correction for dark current
                          &MFC_headerOff,NULL,                            // no correction for offset
                          pMfc->mfcMaskOffset,pMfc->mfcMaskSpec,0);

            FILES_CompactPath(MFC_fileOffset,fileName,1,1);
           }
         }
        rc=ERROR_ID_FILE_RECORD;
       }
//      else if (MFC_header.noscans<=0)
//       rc=ERROR_ID_FILE_RECORD;
         // Wavelength selection

      if (!rc)
       {
        // Date and time read out

        sscanf(MFC_header.dateAndTime,"%d.%d.%d",&day,&mon,&year);
        sscanf(&MFC_header.dateAndTime[9],"%d:%d:%d",&hour,&min,&sec);

        today.da_day=(char)day;
        today.da_mon=(char)mon;
        today.da_year=year;

        if (today.da_year<30)
         today.da_year+= 2000;
        else if (today.da_year<130)
         today.da_year+= 1900;
        else if (today.da_year<1930)
         today.da_year+= 100;

        pRecord->startDateTime.thetime.ti_hour=(unsigned char)hour;
        pRecord->startDateTime.thetime.ti_min=(unsigned char)min;
        pRecord->startDateTime.thetime.ti_sec=(unsigned char)sec;

        sscanf(&MFC_header.dateAndTime[18],"%d:%d:%d",&hour,&min,&sec);

        pRecord->endDateTime.thetime.ti_hour=(unsigned char)hour;
        pRecord->endDateTime.thetime.ti_min=(unsigned char)min;
        pRecord->endDateTime.thetime.ti_sec=(unsigned char)sec;

        Tm1=(double)ZEN_NbSec(&today,&pRecord->startDateTime.thetime,0);
        Tm2=(double)ZEN_NbSec(&today,&pRecord->endDateTime.thetime,0);

        Tm1=(Tm1+Tm2)*0.5;

        pRecord->present_datetime.thedate.da_year  = ZEN_FNCaljye (&Tm1);
        pRecord->present_datetime.thedate.da_mon   = (char) ZEN_FNCaljmon (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));
        pRecord->present_datetime.thedate.da_day   = (char) ZEN_FNCaljday (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));

        memcpy(&pRecord->startDateTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));
        memcpy(&pRecord->endDateTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));

        // Data on the current spectrum

        nsec1=pRecord->startDateTime.thetime.ti_hour*3600+pRecord->startDateTime.thetime.ti_min*60+pRecord->startDateTime.thetime.ti_sec;
        nsec2=pRecord->endDateTime.thetime.ti_hour*3600+pRecord->endDateTime.thetime.ti_min*60+pRecord->endDateTime.thetime.ti_sec;

        if (nsec2<nsec1)
         nsec2+=86400;

        nsec=(nsec1+nsec2)/2;

        pRecord->present_datetime.thetime.ti_hour=(unsigned char)(nsec/3600);
        pRecord->present_datetime.thetime.ti_min=(unsigned char)((nsec%3600)/60);
        pRecord->present_datetime.thetime.ti_sec=(unsigned char)((nsec%3600)%60);

        pRecord->TDet     = 0;
        pRecord->Tint     = MFC_header.int_time/MFC_header.noscans;
        pRecord->NSomme   = MFC_header.noscans;

        pRecord->wavelength1=MFC_header.wavelength1;
        memcpy(pRecord->dispersion,MFC_header.dispersion,sizeof(float)*3);
        memcpy(pRecord->Nom,MFC_header.specname,20);

        pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
        pRecord->TotalExpTime=(double)nsec2-nsec1;
        pRecord->TotalAcqTime=(double)MFC_header.int_time;
        pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;

        pRecord->longitude=-MFC_header.longitude;  // !!!
        pRecord->latitude=MFC_header.latitude;
        pRecord->altitude=(double)0.;
        pRecord->elevationViewAngle=(float)MFC_header.elevation;
        pRecord->azimuthViewAngle=999.;
        
        if (strlen(pRecord->Nom))
         {
          if ((ptr=strchr(pRecord->Nom,','))!=NULL)
           sscanf(pRecord->Nom,"%f,%f",&pRecord->elevationViewAngle,&pRecord->azimuthViewAngle);
          else
           sscanf(pRecord->Nom,"%f",&pRecord->elevationViewAngle);
         }
        else 
         pRecord->elevationViewAngle=999.;
        
        pRecord->maxdoas.measurementType=((pRecord->elevationViewAngle>80.) && (pRecord->elevationViewAngle<100.))?PRJCT_INSTR_MAXDOAS_TYPE_ZENITH:PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;  // Not the possibility to separate almucantar, horizon and direct sun from off-axis measurements
        
        if (pRecord->elevationViewAngle>100.)
         {
          pRecord->elevationViewAngle=180.-pRecord->elevationViewAngle;
          if (pRecord->azimuthViewAngle<900.)
           pRecord->azimuthViewAngle=fmod(pRecord->azimuthViewAngle+180.,360.);
         }

        pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
        pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&pRecord->longitude,&pRecord->latitude,&pRecord->Azimuth);

        pRecord->longitude=MFC_header.longitude;  // !!!

        timeshift=(fabs(THRD_localShift)>EPSILON)?THRD_localShift:pRecord->longitude/15.;
        tmLocal=pRecord->Tm+timeshift*3600.;
        pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
        pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+timeshift,(double)24.);
        
        // User constraints

//        if (dateFlag && (pRecord->localCalDay>localDay))
//         rc=ERROR_ID_FILE_END;

        if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) ||
                                (pRecord->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
                                (pRecord->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))         // reference spectra could be a not zenith sky spectrum

           rc=ERROR_ID_FILE_RECORD;

       }
     }

    if (rc)
     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }

// ================
// MFC ASCII FORMAT
// ================

// -----------------------------------------------------------------------------
// FUNCTION      MFC_ReadRecordStd
// -----------------------------------------------------------------------------
// PURPOSE       record read out and processing in MFC binary format
//
// INPUT         fileName          the name of the current file;
//               pHeaderDrk, drk   dark current data if any;
//               pHeaderOff, off   offset data if any;
//               mask              mask used for spectra selection;
//
// OUTPUT        pHeaderSpe, spe   resp. data on the current record and the spectrum
//                                 to process;
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file can't be found;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints;
//               ERROR_ID_ALLOC           buffer allocation error;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC MFC_ReadRecordStd(ENGINE_CONTEXT *pEngineContext,char *fileName,
                     TBinaryMFC *pHeaderSpe,double *spe,
                     TBinaryMFC *pHeaderDrk,double *drk,
                     TBinaryMFC *pHeaderOff,double *off)
 {
  // Declarations

  PRJCT_INSTRUMENTAL *pInstrumental;
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  FILE *fp;         // pointer to the current file
  int  pixFin,day,mon,year,hour,min,sec,nsec,mfcDate[3],yearN,dateSize,sepN;        // date and time fields
  float                tmp;                                   // temporary variable
  char line[MAX_STR_SHORT_LEN+1],             // line of the current file
        keyWord[MAX_STR_SHORT_LEN+1],keyValue[MAX_STR_SHORT_LEN+1],ctmp;
  struct date         today;                                 // date of the current record

  INDEX i,iDay,iMon,iYear;          // browse pixels in the spectrum
  double Tm1,Tm2;
  int nsec1,nsec2;
  RC rc;            // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pInstrumental=&pEngineContext->project.instrumental;
  
  memset(mfcDate,0,sizeof(int)*3);
  iDay=iMon=iYear=-1;
  yearN=sepN=0;
  memset(line,0,MAX_STR_SHORT_LEN+1);
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Open file

  if ((fp=fopen(fileName,"rt"))==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if (!STD_FileLength(fp))
   rc=ERROR_SetLast("ReadMFCRecordStd",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
  else
   {
    for (i=0;i<n_wavel;i++)
     spe[i]=(double)0.;

    if (fgets(line,MAX_STR_SHORT_LEN,fp) &&                                       // first line
        fgets(line,MAX_STR_SHORT_LEN,fp) && // (sscanf(line,"%d",&pixDeb)>=1) &&  // get the first pixel
        fgets(line,MAX_STR_SHORT_LEN,fp) && (sscanf(line,"%d",&pixFin)>=1))       // get the last pixel

     for (i=0;i<pixFin;i++)
      {
          fgets(line,MAX_STR_SHORT_LEN,fp);
       sscanf(line,"%lf",&spe[i]);
      }

//    fgets(line,MAX_STR_SHORT_LEN,fp);

    fgets(line,MAX_STR_SHORT_LEN,fp);
    sscanf(line,"%[^'\n']",pHeaderSpe->specname);                               // Name of the spectrum

    fgets(line,MAX_STR_SHORT_LEN,fp);
    sscanf(line,"%[^'\n']",pHeaderSpe->spectroname);                            // Name of the spectrometer
    fgets(line,MAX_STR_SHORT_LEN,fp);
    sscanf(line,"%[^'\n']",pHeaderSpe->scan_dev);                               // Name of the scanning device

    memcpy(pRecord->Nom,pHeaderSpe->specname,20);

    dateSize=strlen(pInstrumental->mfc.mfcStdDate);

    for (i=0;i<dateSize;i++)
     {
      if ((pInstrumental->mfc.mfcStdDate[i]=='Y') || (pInstrumental->mfc.mfcStdDate[i]=='y'))
       {
        iYear=sepN;
        yearN++;
       }
      else if ((pInstrumental->mfc.mfcStdDate[i]=='M') ||(pInstrumental->mfc.mfcStdDate[i]=='m'))
       iMon=sepN;
      else if ((pInstrumental->mfc.mfcStdDate[i]=='D') || (pInstrumental->mfc.mfcStdDate[i]=='d'))
       iDay=sepN;
      else
       sepN++;
     }

    if (fgets(line,MAX_STR_SHORT_LEN,fp))
     sscanf(line,"%d%c%d%c%d",&mfcDate[0],&ctmp,&mfcDate[1],&ctmp,&mfcDate[2]);

    day=mfcDate[iDay];
    mon=mfcDate[iMon];
    year=mfcDate[iYear];

    fscanf(fp,"%d:%d:%d\n",&hour,&min,&sec);

    today.da_day=(char)day;
    today.da_mon=(char)mon;
    today.da_year= year;

    if (today.da_year<30)
     today.da_year+= 2000;
    else if (today.da_year<130)
     today.da_year+= 1900;
    else if (today.da_year<1930)
     today.da_year+= 100;

    pRecord->startDateTime.thetime.ti_hour=(unsigned char)hour;
    pRecord->startDateTime.thetime.ti_min=(unsigned char)min;
    pRecord->startDateTime.thetime.ti_sec=(unsigned char)sec;

    fscanf(fp,"%d:%d:%d\n",&hour,&min,&sec);

    pRecord->endDateTime.thetime.ti_hour=(unsigned char)hour;
    pRecord->endDateTime.thetime.ti_min=(unsigned char)min;
    pRecord->endDateTime.thetime.ti_sec=(unsigned char)sec;

    Tm1=(double)ZEN_NbSec(&today,&pRecord->startDateTime.thetime,0);
    Tm2=(double)ZEN_NbSec(&today,&pRecord->endDateTime.thetime,0);

    Tm1=(Tm1+Tm2)*0.5;

    pRecord->present_datetime.thedate.da_year = ZEN_FNCaljye (&Tm1);
    pRecord->present_datetime.thedate.da_mon  = ZEN_FNCaljmon (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));
    pRecord->present_datetime.thedate.da_day  = ZEN_FNCaljday (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));

    memcpy(&pRecord->startDateTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));
    memcpy(&pRecord->endDateTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));

    // Data on the current spectrum

    nsec1=pRecord->startDateTime.thetime.ti_hour*3600+pRecord->startDateTime.thetime.ti_min*60+pRecord->startDateTime.thetime.ti_sec;
    nsec2=pRecord->endDateTime.thetime.ti_hour*3600+pRecord->endDateTime.thetime.ti_min*60+pRecord->endDateTime.thetime.ti_sec;

    if (nsec2<nsec1)
     nsec2+=86400;

    pRecord->TotalExpTime=nsec2-nsec1;

    nsec=(nsec1+nsec2)/2;

    pRecord->present_datetime.thetime.ti_hour=(unsigned char)(nsec/3600);
    pRecord->present_datetime.thetime.ti_min=(unsigned char)((nsec%3600)/60);
    pRecord->present_datetime.thetime.ti_sec=(unsigned char)((nsec%3600)%60);

    pRecord->TDet     = 0;

    fscanf(fp,"%f\n",&tmp);
    fscanf(fp,"%f\n",&tmp);
    fscanf(fp,"SCANS %d\n",&pRecord->NSomme);
    fscanf(fp,"int_TIME %lf\n",&pRecord->TotalAcqTime);
    fgets(line,MAX_STR_SHORT_LEN,fp);
    fscanf(fp,"LONGITUDE %lf\n",&pRecord->longitude);
    fscanf(fp,"LATITUDE %lf\n",&pRecord->latitude);

    pRecord->TotalExpTime*=0.001;

    while (fgets(line,MAX_STR_SHORT_LEN,fp))
     {
      if (strchr(line,'=')!=NULL)
       {
        sscanf(line,"%s = %s",keyWord,keyValue);
        if (!strcasecmp(keyWord,"AzimuthAngle"))
         pRecord->azimuthViewAngle=(float)atof(keyValue);
        else if (!strcasecmp(keyWord,"ElevationAngle"))
         pRecord->elevationViewAngle=(float)atof(keyValue);
        else if (!strcasecmp(keyWord,"ExposureTime"))
         pRecord->Tint=(double)atof(keyValue)*0.001;
        else if (!strcasecmp(keyWord,"Latitude"))
         pRecord->latitude=(double)atof(keyValue);
        else if (!strcasecmp(keyWord,"Longitude"))
         pRecord->longitude=(double)atof(keyValue);
        else if (!strcasecmp(keyWord,"NumScans"))
         pRecord->NSomme=(int)atoi(keyValue);
        else if (!strcasecmp(keyWord,"Temperature"))
         pRecord->TDet=(double)atof(keyValue);
       }
     }

    pRecord->maxdoas.measurementType=((pRecord->elevationViewAngle>80.)&&(pRecord->elevationViewAngle<100.))?PRJCT_INSTR_MAXDOAS_TYPE_ZENITH:PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;  // Not the possibility to separate almucantar, horizon and direct sun from off-axis measurements
    
    if (pRecord->elevationViewAngle>100.)
     {
      pRecord->elevationViewAngle=180.-pRecord->elevationViewAngle;
      pRecord->azimuthViewAngle=fmod(pRecord->azimuthViewAngle+180,360);
     }
     
    if ((pRecord->Tint<(double)1.e-3) && (pRecord->TotalAcqTime>(double)1.e-3))
     pRecord->Tint=pRecord->TotalAcqTime;

    pHeaderSpe->int_time=(float)pRecord->Tint;
    pHeaderSpe->noscans=pRecord->NSomme;

    // Offset correction if any

    if ((off!=NULL) && (pHeaderOff->noscans>0) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET))
     {
      for (i=0;i<n_wavel;i++)
       spe[i]-=(double)off[i]*pHeaderSpe->noscans/pHeaderOff->noscans;
     }

    // Dark current correction if any

    if ((drk!=NULL) && (pHeaderDrk->int_time!=(float)0.) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET) && (THRD_browseType!=THREAD_BROWSE_MFC_DARK))
     {
      for (i=0;i<n_wavel;i++)
       spe[i]-=(double)pHeaderSpe->noscans*drk[i]*pHeaderSpe->int_time/(pHeaderDrk->int_time*pHeaderDrk->noscans);
     }
   }


  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliMFCStd
// -----------------------------------------------------------------------------
// PURPOSE       MFC ASCII format read out
//
// INPUT         recordNo     index of record in file
//               dateFlag     0 no date constraint; 1 a date selection is applied
//               specFp       pointer to the spectra file
//
// OUTPUT        pEngineContext  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_FILE_NOT_FOUND : the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------
RC ReliMFCStd(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  char                fileName[MAX_STR_SHORT_LEN+1],                           // name of the current file (the current record)
                      *ptr;                                                     // pointers to parts in the previous string

  double               longit;                                                  // longitude of the current record
  FILE                *fp;                                                      // pointer to the current file
  RC                   rc;                                                      // return code
  double               tmLocal,timeshift;

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  fp=NULL;

  const int n_wavel = NDET[0];

  rc=ERROR_ID_NO;

  if ((ptr=strrchr(fileName,PATH_SEP))==NULL)
   rc=ERROR_ID_FILE_RECORD;
  else if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber))
   {
    // open the file

    if (!(rc=MFC_ReadRecordStd(pEngineContext,fileName,&MFC_header,pBuffers->spectrum,&MFC_headerDrk,pBuffers->varPix,&MFC_headerOff,pBuffers->offset)))
     {
      pRecord->SkyObs   = 0;
      pRecord->rejected = 0;
      pRecord->ReguTemp = 0;

      pRecord->Azimuth  = 0;
      pRecord->BestShift=(double)0.;
      pRecord->NTracks=0;

      longit=-pRecord->longitude;

      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
      pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longit,&pRecord->latitude,NULL);
      pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;

      pRecord->altitude=(double)0.;

      timeshift=(fabs(THRD_localShift)>EPSILON)?THRD_localShift:pRecord->longitude/15.;

      tmLocal=pRecord->Tm+timeshift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+timeshift,(double)24.);

      // User constraints

      if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) ||
                              (pRecord->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
                              (pRecord->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))            // reference spectra could be a not zenith sky spectrum
      // if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) || (pRecord->elevationViewAngle<80.))) )                     // reference spectra are zenith only
       rc=ERROR_ID_FILE_RECORD;
      else if (pEngineContext->project.instrumental.mfc.mfcRevert)
       VECTOR_Invert(pBuffers->spectrum,n_wavel);
     }
   }
  else
   rc=ERROR_ID_FILE_RECORD;

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Return

  return rc;
 }

// ===========================
// MFC BIRA-IASB BINARY FORMAT
// ===========================

//! \struct MFC_BIRA
//! \brief Description of a record in the new MFC binary file format developed
//!        at BIRA-IASB to make the processing of MFC files by QDOAS easier

#pragma pack(push,1)

typedef struct
 {
  //! \details the measurement type
  int          measurementType;
  //! \details the measurement date
  SHORT_DATE   measurementDate;
  //! \details the starting measurement time
  struct time  startTime;
  //! \details the ending measurement time
  struct time  endTime;
  //! \details the name of the spectrometer
  char         spectroName[64];
  //! \details the name of the device
  char         deviceName[64];
  //! \details the number of scans
  int          scansNumber;
  //! \details the exposure time
  float        exposureTime;
  //! \details the total exposure time (= \ref scansNumber x \ref exposureTime)
  float        totalAcqTime;
  //! \details the name of the observation site
  char         siteName[64];
  //! \details the longitude of the observation site
  float        longitude;
  //! \details the latitude of the observation site
  float        latitude;
  //! \details the viewing azimuth angle
  float        azimuthAngle;
  //! \details the viewing elevation angle
  float        elevationAngle;
  //! \details the original file name
  char         fileName[MAX_STR_LEN+1];
  //! \details the temperature
  float        temperature;
 }
MFCBIRA_HEADER;

#pragma pack(pop)

// -----------------------------------------------------------------------------
// FUNCTION MFCBIRA_Set
// -----------------------------------------------------------------------------
/*!
   \fn      RC MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
   \details Set the number of records in a file in the MFC BIRA binary format.
            This number is the first integer read from the file.  Check also
            the size of the detector\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  specFp pointer to the spectra file to read
   \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
            ERROR_ID_FILE_EMPTY if the file is empty\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  MFCBIRA_HEADER header;
  double *offset,*darkCurrent;
  float *spectrum,drkTint;
  int detectorSize;
  int i,j,nOff,nDrk;
  RC rc;                                                                        // return code

  // Initializations

  ENGINE_refStartDate=1;
  pEngineContext->recordNumber=0;
  offset=pEngineContext->buffers.offset;
  darkCurrent=pEngineContext->buffers.varPix;
  spectrum=NULL;

  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!STD_FileLength(specFp))
   rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else {
    fread(&pEngineContext->recordNumber,sizeof(int),1,specFp);
    fread(&detectorSize,sizeof(int),1,specFp);
    fread(&header,sizeof(MFCBIRA_HEADER),1,specFp);                             // Get date and time of the first record

    memcpy(&pEngineContext->fileInfo.startDate,&header.measurementDate,sizeof(SHORT_DATE));
    memcpy(&pEngineContext->fileInfo.startTime,&header.startTime,sizeof(struct time));

    if (pEngineContext->recordNumber<=0)
      rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
    else if (detectorSize!=n_wavel)
      rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_LENGTH,pEngineContext->fileInfo.fileName);
    else if ((spectrum=MEMORY_AllocBuffer("MFCBIRA_Reli","spectrum",sizeof(float)*n_wavel,1,0,MEMORY_TYPE_FLOAT))==NULL)
      rc=ERROR_ID_ALLOC;

    else
      {
       // Load offset and dark current

      if (!rc && (offset!=NULL) && (darkCurrent!=NULL))
       {
        // Initialize vectors

        VECTOR_Init(offset,(double)0.,n_wavel);
        VECTOR_Init(darkCurrent,(double)0.,n_wavel);

        drkTint=0.;

        // Browse records

        for (i=nOff=nDrk=0,drkTint=0.;i<pEngineContext->recordNumber;i++)
         {
          fseek(specFp,2L*sizeof(int)+i*(sizeof(MFCBIRA_HEADER)+n_wavel*sizeof(float)),SEEK_SET);
          fread(&header,sizeof(MFCBIRA_HEADER),1,specFp);
          
          // Load offset

          if (header.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFSET)
           {
            fread(spectrum,sizeof(float),n_wavel,specFp);
            for (j=0;j<n_wavel;j++)
             offset[j]+=(double)spectrum[j];

            nOff+=header.scansNumber;                                             // all offset should have the same exposure time
           }

          // Load dark current

          else if (header.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_DARK)
           {
            fread(spectrum,sizeof(float),n_wavel,specFp);
            for (j=0;j<n_wavel;j++)
             darkCurrent[j]+=(double)spectrum[j];

            nDrk+=header.scansNumber;
            drkTint=header.exposureTime;                                        // all dark current should have the same exposure time
           }
         }



        // Average offset (account for the number of spectra and the number of scans)

        if (nOff)
            for (j=0;j<n_wavel;j++)
             offset[j]/=nOff;

        // Average dark current and correct by the offset

        if (nDrk)
            for (j=0;j<n_wavel;j++)                                                     // drk=drk-offset*drkScans/offScans
             darkCurrent[j]=(darkCurrent[j]-offset[j]*nDrk)/(nDrk*drkTint);       // number of scans for the dark current should be 1
       }
        }
  }

  // Release allocated buffer

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("MFCBIRA_Reli","spectrum",spectrum);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MFCBIRA_Reli
// -----------------------------------------------------------------------------
/*!
   \fn      RC MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
   \details
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  recordNo        the index of the record to read
   \param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise
   \param   [in]  localDay        if \a dateFlag is 1, the calendar day for the reference spectrum to search for
   \param   [in]  specFp          pointer to the spectra file to read
   \return  ERROR_ID_ALLOC if the allocation of the buffer for the spectrum failed \n
            ERROR_ID_FILE_RECORD if the record is the spectrum is not a spectrum to analyze (sky or dark spectrum)\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  MFCBIRA_HEADER header;                                                        // header of records
  RECORD_INFO *pRecord;                                                     // pointer to the data part of the engine context
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  float *spectrum;                                                              // pointer to spectrum and offset
  double Tm1,Tm2,tmLocal,longit;
  int nsec,nsec1,nsec2;
  struct date measurement_date;
  double timeshift;
  INDEX   i;                                                                    // browse pixels of the detector
  RC      rc;                                                                   // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  if ((spectrum=MEMORY_AllocBuffer("MFCBIRA_Reli","spectrum",sizeof(float)*n_wavel,1,0,MEMORY_TYPE_FLOAT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Go to the requested record

    fseek(specFp,2L*sizeof(int)+(recordNo-1)*(sizeof(MFCBIRA_HEADER)+n_wavel*sizeof(float)),SEEK_SET);
    fread(&header,sizeof(MFCBIRA_HEADER),1,specFp);
    fread(spectrum,sizeof(float),n_wavel,specFp);

    // Retrieve the main information from the header

    pRecord->NSomme=header.scansNumber;
    pRecord->Tint=header.exposureTime;
    pRecord->latitude=header.latitude;
    pRecord->longitude=header.longitude;
    pRecord->TotalAcqTime=header.totalAcqTime;
    pRecord->elevationViewAngle=header.elevationAngle;
    pRecord->azimuthViewAngle=header.azimuthAngle;
    pRecord->TDet=header.temperature;
    pRecord->maxdoas.measurementType=header.measurementType;

    strcpy(pRecord->mfcBira.originalFileName,header.fileName);
    
    if ((pRecord->maxdoas.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && ((pRecord->elevationViewAngle>80.)&&(pRecord->elevationViewAngle<100.)))
     pRecord->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;  
    
    if (pRecord->elevationViewAngle>100.)
     {
      pRecord->elevationViewAngle=180.-pRecord->elevationViewAngle;
      pRecord->azimuthViewAngle=fmod(pRecord->azimuthViewAngle+180,360);
     }

    // Calculate the date and time at half of the measurement

    memcpy(&pRecord->startDateTime.thetime,&header.startTime,sizeof(struct time));
    memcpy(&pRecord->endDateTime.thetime,&header.endTime,sizeof(struct time));

    measurement_date.da_year = header.measurementDate.da_year;
    measurement_date.da_mon = header.measurementDate.da_mon;
    measurement_date.da_day = header.measurementDate.da_day;
    Tm1=(double)ZEN_NbSec(&measurement_date,&pRecord->startDateTime.thetime,0);
    Tm2=(double)ZEN_NbSec(&measurement_date,&pRecord->endDateTime.thetime,0);

    pRecord->TotalExpTime=(double)Tm2-Tm1;

    Tm1=(Tm1+Tm2)*0.5;

    pRecord->present_datetime.thedate.da_year = ZEN_FNCaljye (&Tm1);
    pRecord->present_datetime.thedate.da_mon  = ZEN_FNCaljmon (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));
    pRecord->present_datetime.thedate.da_day  = ZEN_FNCaljday (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));

    memcpy(&pRecord->startDateTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));
    memcpy(&pRecord->endDateTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));

    // Data on the current spectrum

    nsec1=pRecord->startDateTime.thetime.ti_hour*3600+pRecord->startDateTime.thetime.ti_min*60+pRecord->startDateTime.thetime.ti_sec;
    nsec2=pRecord->endDateTime.thetime.ti_hour*3600+pRecord->endDateTime.thetime.ti_min*60+pRecord->endDateTime.thetime.ti_sec;

    if (nsec2<nsec1)
     nsec2+=86400;

    nsec=(nsec1+nsec2)/2;

    pRecord->present_datetime.thetime.ti_hour=(unsigned char)(nsec/3600);
    pRecord->present_datetime.thetime.ti_min=(unsigned char)((nsec%3600)/60);
    pRecord->present_datetime.thetime.ti_sec=(unsigned char)((nsec%3600)%60);

    longit=-header.longitude;

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
    pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longit,&pRecord->latitude,&pRecord->Azimuth);

    pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;

    timeshift=(fabs(THRD_localShift)>EPSILON)?THRD_localShift:header.longitude/15.;

    tmLocal=pRecord->Tm+timeshift*3600.;
    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+timeshift,(double)24.);

    for (i=0;i<n_wavel;i++)
     pBuffers->spectrum[i]=(double)spectrum[i];

    if ((header.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_DARK) && (header.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFSET))
     {
      // Offset correction

      if (pBuffers->offset!=NULL)
       for (i=0;i<n_wavel;i++)
        pBuffers->spectrum[i]-=pBuffers->offset[i]*header.scansNumber;          // offset is already divided by its number of scans

      // Dark current correction                                                // dark current is already divided by it integration time

      if (pBuffers->varPix!=NULL)
       for (i=0;i<n_wavel;i++)
        {
         pBuffers->spectrum[i]-=pBuffers->varPix[i]*header.scansNumber*header.exposureTime;
         pBuffers->spectrum[i]/=header.scansNumber;
        }

         // Average

         // for (i=0;i<NDET;i++)
         // pBuffers->spectrum[i]/=header.scansNumber;

      if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) ||
                              (pRecord->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
                              (pRecord->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))                       // reference spectra could be a not zenith sky spectrum

      //  if (rc || (dateFlag && ((pRecord->elevationViewAngle<80.) || (header.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH))) ) //||                     // reference spectra are zenith only
                //(!dateFlag && pEngineContext->analysisRef.refScan && !pEngineContext->analysisRef.refSza && (pRecord->elevationViewAngle>80.)))

       rc=ERROR_ID_FILE_RECORD;
     }
    else
     rc=ERROR_ID_FILE_RECORD;
   }

  // Release allocated buffer

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("MFCBIRA_Reli","spectrum",spectrum);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MFC_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the reference spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC MFC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  PRJCT_INSTRUMENTAL *pInstrumental;
  char fileName[MAX_STR_SHORT_LEN+1],*ptr;
  TBinaryMFC tbinaryRef;
  INDEX indexWindow,indexFeno,indexTabCross;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  int DimL,useKurucz,saveFlag;                                                       // working variables
  RC rc;                                                                        // return code

  // Initializations

  const int n_wavel = NDET[0];

  pBuffers=&pEngineContext->buffers;

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  pInstrumental=&pEngineContext->project.instrumental;
  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  rc=ERROR_ID_NO;

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->refFlag && ((ptr=strrchr(fileName,PATH_SEP))!=NULL))
   {
    useKurucz=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     if (!TabFeno[0][indexFeno].hidden && !TabFeno[0][indexFeno].gomeRefFlag)
      {
       pTabFeno=&TabFeno[0][indexFeno];
       pTabFeno->NDET=n_wavel;
       FILES_RebuildFileName(ptr+1,pTabFeno->refFile,0);

       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
        rc=MFC_ReadRecord(fileName,
                         &tbinaryRef,pTabFeno->Sref,
                         &MFC_headerDrk,pBuffers->varPix,
                         &MFC_headerOff,pBuffers->offset,
                         pInstrumental->mfc.mfcMaskSpec,pInstrumental->mfc.mfcMaskSpec,pInstrumental->mfc.mfcRevert);
             else
        rc=MFC_ReadRecordStd(pEngineContext,fileName,
                            &tbinaryRef,pTabFeno->Sref,
                            &MFC_headerDrk,pBuffers->varPix,
                            &MFC_headerOff,pBuffers->offset);

       if (!rc && !(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"MFC_LoadAnalysis (Reference) ")))
        {
         VECTOR_Copy(pTabFeno->SrefEtalon,pTabFeno->Sref,pTabFeno->NDET);
         pTabFeno->useEtalon=pTabFeno->displayRef=1;

         // Browse symbols

         for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
          {
           pTabCross=&pTabFeno->TabCross[indexTabCross];
           pWrkSymbol=&WorkSpace[pTabCross->Comp];

           // Cross sections and predefined vectors

           if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                 (indexTabCross==pTabFeno->indexCommonResidual))) &&
               ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET))!=ERROR_ID_NO))

            goto EndMFC_LoadAnalysis;
          }

         // Gaps : rebuild subwindows on new wavelength scale

         doas_spectrum *new_range = spectrum_new();
         for (indexWindow = 0, DimL=0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++)
          {
           int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
           int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

           spectrum_append(new_range, pixel_start, pixel_end);

           DimL += pixel_end - pixel_start +1;
          }

         // Buffers allocation
         FIT_PROPERTIES_free("MFC_LoadAnalysis",&pTabFeno->fit_properties);
         pTabFeno->fit_properties.DimL=DimL;
         FIT_PROPERTIES_alloc("MFC_LoadAnalysis",&pTabFeno->fit_properties);
         // new spectral windows
         pTabFeno->fit_properties.specrange = new_range;

         pTabFeno->Decomp=1;

         if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
             (!pKuruczOptions->fwhmFit &&
             ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))

          goto EndMFC_LoadAnalysis;
        }

       useKurucz+=pTabFeno->useKurucz;
      }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
       goto EndMFC_LoadAnalysis;
     }
   }

  // Return

  EndMFC_LoadAnalysis :

  return rc;
 }
