
//  ----------------------------------------------------------------------------
//! \addtogroup Format
//! @{
//!
//! \file      mkzy-read.c
//! \brief     Routines to read data in the MKZY format implemented by Manne Kihlman and Zhang Yan.
//! \details   This module contains the routines needed to read data from PAK files written
//!            in a compressed file format created by MANNE Kihlman and ZHANG Yan, Chalmers,
//!            Goteborg, Sweden.  This file format is used within the NOVAC network.
//! \authors   Kihlman MANNE and Yan ZHANG, Chalmers, Goteborg, Sweden\n
//!            Adapted for QDOAS by Caroline FAYT
//! \date      14 January 2009
//! \copyright QDOAS is distributed under GNU General Public License
//!
//! @}
//  ----------------------------------------------------------------------------
//
//  =========
//  FUNCTIONS
//  =========
//
//  MKZY_UnPack - decompresses a spectrum record in the MKZY file format;
//  MKZY_ParseDate - decompose a date in the MKZY file format;
//  MKZY_ParseTime - decompose a time in the MKZY file format;
//
//  MKZY_ReadRecord - read a specified record from a file in MKZY format.
//  MKZY_SearchForOffset - extract the dark current spectrum and the offset (resp. called dark and offset) from the file and remove offset from dark current.
//  MKZY_SearchForSky - extract the reference spectrum (called sky) from the file and correct it by offset and dark current
//
//  MKZY_Set - calculate the number of records in a file in MKZY format;
//  MKZY_Read - call MKZY_ReadRecord to read a specified record from a file in MKZY format and check that it is a spectrum to analyze (spectrum name should be 'other').
//  MKZY_LoadAnalysis - as the reference spectrum is retrieved from spectra files, calibration has to be applied on each file
//
//  ----------------------------------------------------------------------------
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//        BIRA-IASB
//        Belgian Institute for Space Aeronomy
//        Ringlaan 3 Avenue Circulaire
//        1180     UCCLE
//        BELGIUM
//        qdoas@aeronomie.be
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

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "analyse.h"
#include "stdfunc.h"
#include "vector.h"
#include "zenithal.h"
#include "winthrd.h"
#include "kurucz.h"


// ====================
// CONSTANTS DEFINITION
// ====================

#define MAX_SPECTRUM_LENGTH 4096
#define HEADSIZ 12

// ====================
// STRUCTURE DEFINITION
// ====================

#pragma pack(push, 1)

//! \struct MKZYhdr
//! \brief MKZY file header.

typedef struct MKZY_HEADER
 {
  char           ident[4];                                                      //!< should be "MKZY"
  unsigned short hdrsize;                                                       //!< this is the size in bytes of the header
  unsigned short hdrversion;                                                    //!< version of the header
  unsigned short size;                                                          //!< the number of bytes with compressed data
 }
MKZY_HEADER;

//! \struct MKZY_RECORDINFO
//! \brief Description of a record in the MKZY file format
//! \details Further details about fields can be found in a document written by Kihlman.

typedef struct MKZY_RECORDINFO
 {
 	        //! \details
 	        //! The checksum is calculated on the uncompressed data; it is used to check that the compression worked properly. \n
 	        //! The checksum can be calculated with the following routine in C :\n\n
 	        //! \code
 	        //!      MKZY_RECORDINFO mkzy;
 	        //!      uint32_t spec[];              // containing the spectral data points
 	        //!      int speclen;                 // containing the number of datapoints stored in the previous vector
 	        //!      int i;
 	        //!      unsigned short checksum;
 	        //!      unsigned short *p;
 	        //!
 	        //!      checksum=0L;
 	        //!      for (i=0;i<speclen;i++)
 	        //!       checksum += spec[i];
 	        //!
 	        //!      p=(unsigned short *)&checksum;
 	        //!      mkzy.checksum=p[0]+p[1];
 	        //! \endcode

  unsigned short checksum;

          //! \details the name of this specific measurement
  char           name[12];
          //! \details the name of the instrument
  char           instrumentname[16];
          //! \details the startchannel for the first data-point
  unsigned short startc;
          //! \details number of pixels saved in the data-field
  unsigned short pixels;
          //! \details the viewing angle of the instrument
  short          viewangle;
          //! \details total number of scans added
  unsigned short scans;
          //! \details exposure time, negative if set automatic
  short          exptime;
          //! \details channel of the spectrometer, typically 0
  unsigned char  channel;
          //! \details for further use, currently contains the status of the solenoid(s) in bit 0 and 1
  unsigned char  flag;
          //! \details Date in \a DDMMYY format. Dates and times are given as GMT (Greenwitch Mean Time).\n
          //! Use \ref MKZY_ParseDate to decompose the date in separate year, month and day fields.
  uint32_t  date;
          //! \details time when the scanning was started
  uint32_t  starttime;
          //! \details time when the scanning was finished
  uint32_t  stoptime;
          //! \details GPS latitude in degrees
  double         lat;
          //! \details GPS longitude in degrees
  double         lon;
          //! \details new in version 2
  short          altitude;
          //! \details new in version 2, nr between 0 and measurecnt-1
  char           measureidx;
          //! \details new in version 2, number of MEAS= lines in cfg.txt
  char           measurecnt;
          //! \details new in version 3, direction of 2nd motor
  short          viewangle2;
          //! \details new in version 3, given in cfg.txt
  short          compassdir;
          //! \details new in version 3, given in cfg.txt
  short          tiltX;
          //! \details new in version 3, given in cfg.txt
  short          tiltY;
          //! \details new in version 3, given in cfg.txt
  float          temperature;
          //! \details new in version 4, given in cfg.txt
  char           coneangle;
          //! \details The voltages read by the ADC.  New in version 5.
  unsigned short ADC[8];
 }
MKZY_RECORDINFO;

#pragma pack(pop)

// -----------------------------------------------------------------------------
// FUNCTION MKZY_UnPack
// -----------------------------------------------------------------------------
//!
//! \fn      int MKZY_UnPack(unsigned char *inpek,int kvar,int *ut)
//! \details This function decompresses a spectrum record in the MKZY file format
//! \param   [in]  inpek buffer with the compressed spectrum
//! \param   [in]  kvar  the length of the uncompressed spectrum
//! \param   [out] ut    buffer to which the data will be uncompressed
//! \return  the number of bytes in the output buffer \a ut
//!
// -----------------------------------------------------------------------------

int MKZY_UnPack(unsigned char *inpek,int kvar,int *ut)
 {
 	// Declarations

  int *utpek=NULL;
  short len,curr;
  short j,jj;
  int a;
  unsigned short lentofile=0;
  int bit=0;

  // validate the input data - Added 2006.02.13 by MJ

  if (kvar>MAX_SPECTRUM_LENGTH)
   return -1;
  if((ut==NULL) || (inpek==NULL))
   return -1;

  utpek=ut;
  lentofile=0;

  while (kvar>0)
   {
    len=0;
    for(j=0;j<7;j++)
     {
      len+=len;
      len|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
      bit++;
     }
    curr=0;
    for(j=0;j<5;j++)
     {
      curr+=curr;
      curr|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
      bit++;
     }

    if(curr)
     {
      for(jj=0;jj<len;jj++)
       {
        a=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
        if(a)
         a=-1;
        bit++;
        for(j=1;j<curr;j++)
         {
          a+=a;
          a|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
          bit++;
         }
        *utpek++=a;
       }
     }
    else
     {
      for(jj=0;jj<len;jj++)
       *utpek++=0;
     }

    kvar-=len;
    lentofile+=len;
   }


  for(jj=1;jj<lentofile;jj++)
   ut[jj]+=ut[jj-1];

  // Return

  return(lentofile);
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_ParseDate
// -----------------------------------------------------------------------------
//!
//! \fn      void MKZY_ParseDate(uint32_t d, struct date *pDate)
//! \details Decompose a date in the MKZY file format.\n
//!          The date field in the header \ref MKZY_RECORDINFO::date is an unsigned long.\n
//!          Dates are represented on 6 digits in the \a DDMMYY file format.\n
//! \par     Example:
//!          To represent 29 November 2005, the date field will have the decimal value 291105.
//! \param   [in]    d  the date in DDMMYY format
//! \param   [out]   pDate pointer to the SHORT_DATE structure with separate year, month and day fields
//!
// -----------------------------------------------------------------------------

void MKZY_ParseDate(uint32_t d, struct date *pDate)
 {
  pDate->da_day=(char)(d/10000L);                                               // the day
  pDate->da_mon=(char)((d-(uint32_t)pDate->da_day*10000L)/100L);           // the month
  pDate->da_year=(short)(d%100);                                                // the year

  if(pDate->da_year<100)
   pDate->da_year+=2000;                                                        //assume the 21:st century (should be ok for another 95 years)
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_ParseTime
// -----------------------------------------------------------------------------
//!
//! \fn      void MKZY_ParseTime(uint32_t t,struct time *pTime)
//! \details Decompose a time in the MKZY file format.\n
//!          Time fields in the header are unsigned long numbers.\n
//!          They are represented on 8 digits in the \a hhmmssdd file format where \a dd are the decimal milliseconds.\n
//! \par     Example:
//!          The decimal value 09350067 represents the time 09:35:00, 670 milliseconds in the morning.
//! \param   [in]    t  the time in \a hhmmssdd (where \a dd are the decimal milliseconds)
//! \param   [out]   pTime pointer to a \a struct \a time structure with separate hour, min and sec fields
//!
// -----------------------------------------------------------------------------

void MKZY_ParseTime(uint32_t t,struct time *pTime)
 {
  pTime->ti_hour=(unsigned char)(t/1000000L);
  pTime->ti_min=(unsigned char)((t-(uint32_t)pTime->ti_hour*1000000L)/10000L);
  pTime->ti_sec=(unsigned char)((t-(uint32_t)pTime->ti_hour*1000000L-(uint32_t)pTime->ti_min*10000L)/100L);
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_ReadRecord
// -----------------------------------------------------------------------------
//!
//! \fn      RC MKZY_ReadRecord(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp)
//! \details read a specified record from a file in MKZY format.\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
//! \param   [in]  recordNo        the index of the record to read
//! \param   [in]  specFp          pointer to the spectra file to read
//! \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
//!          ERROR_ID_FILE_END if the end of the file is reached\n
//!          ERROR_ID_ALLOC if the allocation of a buffer failed\n
//!          ERROR_ID_BUFFER_FULL if the retrieved data are larger than the allocated buffers\n
//!          ERROR_ID_FILE_RECORD if the record doesn't satisfy user criteria\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC MKZY_ReadRecord(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp)
 {
  // Declarations

  BUFFERS        *pBuffers;                                                     // pointer to the buffers part of the engine context
  RECORD_INFO    *pRecord;                                                      // pointer to the record part of the engine context
  MKZY_HEADER     header;                                                       // record header
  MKZY_RECORDINFO recordInfo;                                                   // information on the record
  struct date     today;
  double          Tm1,Tm2;
  unsigned char  *buffer;                                                       // buffer for the spectrum before unpacking
  uint32_t  *lbuffer;                                                            // buffer for the spectrum after unpacking
  unsigned short  checksum,*p;                                                  // check sum
  uint32_t   chk;                                                                // for the calculation of the check sum
  double          longitude;
  int             npixels;                                                      // number of pixels returned
  double         *spectrum;                                                     // the current spectrum and its maximum value
  double          tmLocal;                                                      // measurement local time
  int             nsec,nsec1,nsec2;                                             // the total number of seconds
  INDEX           i;                                                            // browse pixels in the spectrum
  RC              rc;                                                           // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;
  pRecord=&pEngineContext->recordInfo;
  spectrum=(double *)pBuffers->spectrum;
  buffer=NULL;
  lbuffer=NULL;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Initialize the spectrum

  for (i=0;i<n_wavel;i++)
    spectrum[i]=(double)0.;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("MKZY_ReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordInfo.mkzy.recordNumber))
   rc=ERROR_ID_FILE_END;
  else if (((buffer=MEMORY_AllocBuffer("MKZY_ReadRecord","buffer",pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1],1,0,MEMORY_TYPE_STRING))==NULL) ||
           ((lbuffer=(uint32_t *)MEMORY_AllocBuffer("MKZY_ReadRecord","lbuffer",MAX_SPECTRUM_LENGTH,sizeof(uint32_t),0,MEMORY_TYPE_ULONG))==NULL))
   rc=ERROR_ID_ALLOC;
  else
   {
   	// Goto the requested record

    fseek(specFp,(int32_t)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Read the first bytes (including MKZY sequence and the size of the header)

    fread(&header,sizeof(MKZY_HEADER),1,specFp);

    if (header.hdrsize>sizeof(MKZY_HEADER))
     fread(&recordInfo,min(sizeof(MKZY_RECORDINFO),header.hdrsize-sizeof(MKZY_HEADER)),1,specFp);

    strncpy(pRecord->Nom,recordInfo.name,12);                                   // the name of this specific measurement

    pRecord->NSomme=recordInfo.scans;                                           // total number of scans added
    pRecord->Tint=(double)fabs((double)recordInfo.exptime)*0.001;               // exposure time, negative if set automatic (exposure time is given in ms)
    pRecord->longitude=(double)recordInfo.lon;                                  // GPS longitude in degrees
    pRecord->latitude=(double)recordInfo.lat;                                   // GPS latitude in degrees

    longitude=-pRecord->longitude;

    strcpy(pRecord->mkzy.instrumentname,recordInfo.instrumentname);             // the name of the instrument

    pRecord->mkzy.startc=recordInfo.startc;                                     // the startchannel for the first data-point
    pRecord->mkzy.pixels=recordInfo.pixels;                                     // number of pixels saved in the data-field
    pRecord->mkzy.channel=recordInfo.channel;                                   // channel of the spectrometer, typically 0
    pRecord->mkzy.coneangle=recordInfo.coneangle;                               // new in version 4, given in cfg.txt

    pRecord->mkzy.scanningAngle=(double)recordInfo.viewangle;                   // the viewing angle of the instrument
    pRecord->mkzy.scanningAngle2=(double)recordInfo.viewangle2;                 // the viewing angle of the 2nd instrument

    if (pRecord->mkzy.scanningAngle>180.)
     pRecord->mkzy.scanningAngle-=360.;
    if (pRecord->mkzy.scanningAngle2>180.)
     pRecord->mkzy.scanningAngle-=360.;

    MKZY_ParseDate(recordInfo.date,&today);

    MKZY_ParseTime(recordInfo.starttime,&pRecord->startDateTime.thetime);
    MKZY_ParseTime(recordInfo.stoptime,&pRecord->endDateTime.thetime);

    Tm1=(double)ZEN_NbSec(&today,&pRecord->startDateTime.thetime,0);
    Tm2=(double)ZEN_NbSec(&today,&pRecord->endDateTime.thetime,0);

    Tm1=(Tm1+Tm2)*0.5;

    pRecord->present_datetime.thedate.da_year  = (short) ZEN_FNCaljye (&Tm1);
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

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
    pRecord->TotalExpTime=(double)nsec2-nsec1;

    pRecord->TotalAcqTime=(double)pRecord->NSomme*pRecord->Tint;
    pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;
    pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longitude,&pRecord->latitude,&pRecord->Azimuth);

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    if (header.hdrversion>=2)
     pRecord->altitude=(double)recordInfo.altitude;                             // new in version 2

    // number of MEAS= lines in cfg.txt

    if (header.hdrversion>=3)
     pRecord->TDet=(double)recordInfo.temperature;                              // new in version 3, given in cfg.txt

    // Be sure to be at the beginning of the data

    fseek(specFp,(int32_t)pBuffers->recordIndexes[recordNo-1]+header.hdrsize,SEEK_SET);

    // Read compressed data and uncompress them

    fread(buffer,pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1]-header.hdrsize,1,specFp);

    if ((recordInfo.pixels>n_wavel) || ((npixels=MKZY_UnPack(buffer,recordInfo.pixels,(int *)lbuffer))<0) || (npixels>n_wavel))
     rc=ERROR_SetLast("MKZY_ReadRecord",ERROR_TYPE_WARNING,ERROR_ID_BUFFER_FULL,"spectra");
    else
     {
      // calculate the checksum

      for (chk=0L,i=0;i<npixels;i++)
       chk+=lbuffer[i];

      p=(unsigned short *)&chk;
      checksum=(unsigned short)p[0]+p[1];

      if (checksum!=recordInfo.checksum)
       rc=ERROR_ID_FILE_RECORD;
      else
       for (i=0;i<npixels;i++)
        spectrum[i]=(double)lbuffer[i];
     }
   }

  // Release allocated buffers

  if (buffer!=NULL)
   MEMORY_ReleaseBuffer("MKZY_ReadRecord","buffer",buffer);
  if (lbuffer!=NULL)
   MEMORY_ReleaseBuffer("MKZY_ReadRecord","lbuffer",lbuffer);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_SearchForOffset
// -----------------------------------------------------------------------------
//!
//! \fn      RC MKZY_SearchForOffset(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
//! \details Extract the dark current spectrum and the offset (resp. called dark and offset) from the file and remove offset from dark current.\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
//! \param   [in]  specFp pointer to the spectra file to read
//! \return  the code returned by \ref MKZY_ReadRecord \n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC MKZY_SearchForOffset(ENGINE_CONTEXT *pEngineContext,FILE *specFp) {
  // Declarations

  INDEX indexRecord;
  double *offset,*darkCurrent;
  int i;
  RC rc;

  // Initializations

  offset=pEngineContext->buffers.offset;
  darkCurrent=pEngineContext->buffers.darkCurrent;
  pEngineContext->recordInfo.mkzy.darkTint=(double)0.;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Search for the spectrum

  for (indexRecord=1;indexRecord<=pEngineContext->recordInfo.mkzy.recordNumber;indexRecord++)
    if (!(rc=MKZY_ReadRecord(pEngineContext,indexRecord,specFp))) {
      // Dark current

      if ((darkCurrent!=NULL) && !strncasecmp(pEngineContext->recordInfo.Nom,"dark",4)) {
        memcpy(darkCurrent,pEngineContext->buffers.spectrum,sizeof(double)*n_wavel);
        pEngineContext->recordInfo.mkzy.darkFlag=1;
        pEngineContext->recordInfo.mkzy.darkScans=pEngineContext->recordInfo.NSomme;
        pEngineContext->recordInfo.mkzy.darkTint=pEngineContext->recordInfo.Tint;
      }

     // Offset

     if ((offset!=NULL) && !strncasecmp(pEngineContext->recordInfo.Nom,"offset",6)) {
       memcpy(offset,pEngineContext->buffers.spectrum,sizeof(double)*n_wavel);
       pEngineContext->recordInfo.mkzy.offsetFlag=1;
       pEngineContext->recordInfo.mkzy.offsetScans=pEngineContext->recordInfo.NSomme;
     }
   }

  // Remove offset from dark current

  if (pEngineContext->recordInfo.mkzy.darkFlag && pEngineContext->recordInfo.mkzy.offsetFlag)
   for (i=0;i<n_wavel;i++)
    darkCurrent[i]-=(double)offset[i]*pEngineContext->recordInfo.mkzy.darkScans/pEngineContext->recordInfo.mkzy.offsetScans;

 	// Return

 	return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_SearchForSky
// -----------------------------------------------------------------------------
//!
//! \fn      RC MKZY_SearchForSky(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
//! \details Extract the reference spectrum (called sky) from the file and correct it by offset and dark current.\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
//! \param   [in]  specFp pointer to the spectra file to read
//! \return  the code returned by \ref MKZY_ReadRecord \n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC MKZY_SearchForSky(ENGINE_CONTEXT *pEngineContext,FILE *specFp) {
  // Declarations

  INDEX indexRecord;
  INDEX i;
  RC rc;

  // Initializations

  pEngineContext->recordInfo.mkzy.skyFlag=0;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Search for the spectrum

  for (indexRecord=1;indexRecord<=pEngineContext->recordInfo.mkzy.recordNumber;indexRecord++)
    if (!(rc=MKZY_ReadRecord(pEngineContext,indexRecord,specFp)) && !strncasecmp(pEngineContext->recordInfo.Nom,"sky",3)) {
      memcpy(pEngineContext->buffers.scanRef,pEngineContext->buffers.spectrum,sizeof(double)*n_wavel);
      pEngineContext->recordInfo.mkzy.skyFlag=1;

      // Correct by offset and dark current

      if (pEngineContext->recordInfo.mkzy.darkFlag && pEngineContext->recordInfo.mkzy.offsetFlag) // similar MFC correction
        for (i=0;i<n_wavel;i++)
          pEngineContext->buffers.scanRef[i]-=(double)(pEngineContext->buffers.offset[i]*pEngineContext->recordInfo.NSomme/pEngineContext->recordInfo.mkzy.offsetScans+
                                                       pEngineContext->buffers.darkCurrent[i]*pEngineContext->recordInfo.Tint/(pEngineContext->recordInfo.mkzy.darkTint*pEngineContext->recordInfo.mkzy.darkScans));

      // Correct by the dark current

      else if (pEngineContext->recordInfo.mkzy.darkFlag)
        for (i=0;i<n_wavel;i++)
          pEngineContext->buffers.scanRef[i]-=(double)pEngineContext->buffers.darkCurrent[i]*pEngineContext->recordInfo.NSomme/pEngineContext->recordInfo.mkzy.darkScans;

      break;
    }

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION MKZY_Set
// -----------------------------------------------------------------------------
//!
//! \fn      RC MKZY_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
//! \details Calculate the number of records in a file in MKZY format.\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
//! \param   [in]  specFp pointer to the spectra file to read
//! \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
//!          ERROR_ID_FILE_EMPTY if the file is empty\n
//!          ERROR_ID_ALLOC if the allocation of a buffer failed\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC MKZY_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  uint32_t *recordIndexes;                                                       // save the position of each record in the file
  unsigned char *buffer,*ptr;                                                   // buffer to load the file
  SZ_LEN fileLength;                                                            // the length of the file to load
  int i;                                                                        // index for loops and arrays
  RC rc;                                                                        // return code

  // Initializations

  buffer=NULL;
  const int n_wavel = NDET[0];

  for (i=0;i<n_wavel;i++)
    pEngineContext->buffers.darkCurrent[i]=pEngineContext->buffers.offset[i]=(double)0.;

  pEngineContext->recordNumber=0;
  pBuffers=&pEngineContext->buffers;
  pEngineContext->recordIndexesSize=2001;
  recordIndexes=pBuffers->recordIndexes;
  pEngineContext->recordInfo.mkzy.recordNumber=0;

  pEngineContext->recordInfo.mkzy.offsetFlag=pEngineContext->recordInfo.mkzy.darkFlag=pEngineContext->recordInfo.mkzy.skyFlag=0;

  rc=ERROR_ID_NO;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("MKZY_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!(fileLength=STD_FileLength(specFp)))
   rc=ERROR_SetLast("MKZY_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else if ((buffer=MEMORY_AllocBuffer("MKZY_Set","buffer",fileLength,1,0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
   	// load the buffer in one operation

   	fread(buffer,fileLength,1,specFp);

   	// search for the "magic" sequence of characters : MKZY

   	for (ptr=buffer;(ptr-buffer<fileLength-4) && (pEngineContext->recordInfo.mkzy.recordNumber<pEngineContext->recordIndexesSize);ptr++)
   	 if ((ptr[0]=='M') && (ptr[1]=='K') && (ptr[2]=='Z') && (ptr[3]=='Y'))
  	  	recordIndexes[pEngineContext->recordInfo.mkzy.recordNumber++]=ptr-buffer;

  	 recordIndexes[pEngineContext->recordInfo.mkzy.recordNumber]=fileLength;

  	 pEngineContext->recordNumber=pEngineContext->recordInfo.mkzy.recordNumber;  // !!!

    if (!(rc=MKZY_SearchForOffset(pEngineContext,specFp)))
  	  rc=MKZY_SearchForSky(pEngineContext,specFp);
   }

  // Release allocated buffers

  if (buffer!=NULL)
   MEMORY_ReleaseBuffer("MKZY_Set","buffer",buffer);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_Read
// -----------------------------------------------------------------------------
//!
//! \fn      RC MKZY_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
//! \details Call \ref MKZY_ReadRecord to read a specified record from a file in MKZY format and check that it is a spectrum to analyze (spectrum name should be 'other').\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
//! \param   [in]  recordNo        the index of the record to read
//! \param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise
//! \param   [in]  localDay        if \a dateFlag is 1, the calendar day for the reference spectrum to search for
//! \param   [in]  specFp          pointer to the spectra file to read
//! \return  the code returned by \ref MKZY_ReadRecord \n
//!          ERROR_ID_FILE_RECORD if the record is the spectrum is not a spectrum to analyze (sky or dark spectrum)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC MKZY_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  double *spectrum,*offset,*dark;                                               // pointer to spectrum, offset and dark current
  double *cumSpectrum;
  INDEX   i,indexRecord,nRecord;                                                // browse pixels of the detector
  RC      rc;                                                                   // return code

  // Initializations
  const int n_wavel = NDET[0];
  spectrum=pEngineContext->buffers.spectrum;
  offset=pEngineContext->buffers.offset;
  dark=pEngineContext->buffers.darkCurrent;

  if ((cumSpectrum=(double *)MEMORY_AllocDVector("MKZY_Read ","cumSpectrum",0,n_wavel-1))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
   	for (i=0;i<n_wavel;i++)
   	 cumSpectrum[i]=(double)0;
   	nRecord=0;
   	indexRecord=recordNo;

    //for (indexRecord=0;indexRecord<pEngineContext->recordInfo.mkzy.recordNumber;indexRecord++)
   	 {
      if (!(rc=MKZY_ReadRecord(pEngineContext,indexRecord,specFp)))
       {
       	if ((THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT) &&
       	   (!strncasecmp(pEngineContext->recordInfo.Nom,"dark",4) ||
       	    !strncasecmp(pEngineContext->recordInfo.Nom,"sky",3) ||
       	    !strncasecmp(pEngineContext->recordInfo.Nom,"offset",6)))

         rc=ERROR_ID_FILE_RECORD;

        else if (!strncasecmp(pEngineContext->recordInfo.Nom,"dark",4))
         memcpy(spectrum,dark,sizeof(double)*n_wavel);
        else if (!strncasecmp(pEngineContext->recordInfo.Nom,"offset",4))
         memcpy(spectrum,offset,sizeof(double)*n_wavel);

  	 	   // Correct by offset and dark current

  	     else if (pEngineContext->recordInfo.mkzy.darkFlag && pEngineContext->recordInfo.mkzy.offsetFlag) // similar MFC correction
  	     {
         for (i=0;i<n_wavel;i++)
          spectrum[i]-=(double)pEngineContext->recordInfo.NSomme*(offset[i]/pEngineContext->recordInfo.mkzy.offsetScans+
                                                      dark[i]*pEngineContext->recordInfo.Tint/(pEngineContext->recordInfo.mkzy.darkTint*pEngineContext->recordInfo.mkzy.darkScans));
        }

  	 	   // Correct by the dark current only (used as offset

  	     else if (pEngineContext->recordInfo.mkzy.darkFlag)
         for (i=0;i<n_wavel;i++)
          spectrum[i]-=(double)dark[i]*pEngineContext->recordInfo.NSomme/pEngineContext->recordInfo.mkzy.darkScans;

        if (!rc)
         {
          for (i=0;i<n_wavel;i++)
           cumSpectrum[i]+=spectrum[i];

      //    if (!nRecord)
      //     Tm1=(double)ZEN_NbSec(&pEngineContext->recordInfo.present_day,&pEngineContext->recordInfo.startTime,0);
      //    else
      //     Tm2=(double)ZEN_NbSec(&pEngineContext->recordInfo.present_day,&pEngineContext->recordInfo.endTime,0);

          nRecord++;
         }
       }
     }

    if (cumSpectrum!=NULL)
     MEMORY_ReleaseDVector("MKZY_Read ","cumSpectrum",cumSpectrum,0);
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_LoadAnalysis
// -----------------------------------------------------------------------------
//!
//! \fn      RC MKZY_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
//! \details As the reference spectrum is retrieved from spectra files, calibration
//!          has to be applied on each file\n
//! \param   [in]  pEngineContext  pointer to the engine context
//! \return  error code returned by one of the child function on error\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC MKZY_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  INDEX indexWindow,indexFeno,indexTabCross;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  int DimL,useKurucz,saveFlag;                                                  // working variables
  RC rc;                                                                        // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  rc=ERROR_ID_NO;
  const int n_wavel = NDET[0];

  if (!pEngineContext->recordInfo.mkzy.skyFlag && (THRD_id==THREAD_TYPE_ANALYSIS))
   rc=ERROR_SetLast("MKZY_LoadAnalysis",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"spectra",pEngineContext->fileInfo.fileName);
  else if (pEngineContext->refFlag)
   {
    useKurucz=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     {
      if (!TabFeno[0][indexFeno].hidden && !TabFeno[0][indexFeno].gomeRefFlag)
       {
        pTabFeno=&TabFeno[0][indexFeno];
        pTabFeno->NDET=n_wavel;

        memcpy(pTabFeno->Sref,pBuffers->scanRef,sizeof(double)*n_wavel);

        if (!rc && !(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"MKZY_LoadAnalysis (Reference) ")))
         {
          memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
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

             goto EndMKZY_LoadAnalysis;
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
          FIT_PROPERTIES_free(__func__,&pTabFeno->fit_properties);
          pTabFeno->fit_properties.DimL=DimL;
          FIT_PROPERTIES_alloc(__func__,&pTabFeno->fit_properties);
          // new spectral windows
          pTabFeno->fit_properties.specrange = new_range;

          pTabFeno->Decomp=1;

          if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
              (!pKuruczOptions->fwhmFit &&
              ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))

           goto EndMKZY_LoadAnalysis;
         }

        useKurucz+=pTabFeno->useKurucz;
       }
     }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
       goto EndMKZY_LoadAnalysis;
     }
   }

  // Return

  EndMKZY_LoadAnalysis :

  return rc;
 }
