
//  ----------------------------------------------------------------------------
/*!
    \file    bira-airborne-read.C
    \brief   Format developed at BIRA-IASB for airborne measurements
    \details This module contains the routines needed to read data measured
             from aircraft and ULM
    \authors Caroline FAYT (caroline.fayt@aeronomie.be)
    \date    13 March 2009
*/
//  ----------------------------------------------------------------------------
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
//  =========
//  FUNCTIONS
//  =========
//
//
//  AIRBORNE_Set - calculate the number of records in a file in the format
//                 developed at BIRA-IASB for airborne measurements;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "winthrd.h"

#include "doas.h"
#include "engine_context.h"
#include "zenithal.h"
#include "stdfunc.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define IGNORED_BYTES 73

// ====================
// STRUCTURE DEFINITION
// ====================

#pragma pack(push,1)

typedef struct _airborneData
 {
  SHORT_DATE    today;                                                          // measurement date and time
  struct time   now;
  int           averagedSpectra;                                                // number of averaged spectra
  int           totalTime;                                                      // the total measurement time
  int           nrejMeas,naccMeas;                                              // resp. number of rejected spectra and number of accumulation
  float         longitude,latitude,altitude;                                    // GPS data
  float         exposureTime;                                                   // exposure time (in milliseconds)
  struct time   gpsTime;
  unsigned char floatflag;
  unsigned char sentPosition;
  unsigned char receivedPosition;
  unsigned char instrumentType;                                                 // 0 for MobileDOAS, 1 for airborne measurements;
  float         outsideTemp,
	               humidity,
	               dewPoint,
	               pressure,
	               altitudeP,
	               insideTemp;

  struct time   endMeas;

  float         longitudeEnd,latitudeEnd,altitudeEnd;
  struct time   gpsTimeEnd;
  float         pitch,roll,heading;                                             // airborne
  short         msBegin,msEnd;                                                  // airborne
  float         viewAzim,viewElev;
  unsigned char useNexstarFlag;
  unsigned char useTempFlag;                                                    // control of temperature (JFJ)
  float         sza,saa;                                                        // solar zenith angle, solar azimuth angle
  char          measType;                                                       // measurement type
  char        ignoredBytes[IGNORED_BYTES];
 }
AIRBORNE_DATA;

#pragma pack(pop)


// -----------------------------------------------------------------------------
// FUNCTION AIRBORNE_Set
// -----------------------------------------------------------------------------
/*!
   \fn      RC AIRBORNE_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
   \details calculate the number of records in a file in the format developed at BIRA-IASB for airborne measurements.\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  specFp pointer to the spectra file to read
   \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
            ERROR_ID_FILE_EMPTY if the file is empty\n
            ERROR_ID_ALLOC if the allocation of a buffer failed\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC AIRBORNE_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  SZ_LEN fileLength;                                                            // the length of the file to load
  RC rc;                                                                        // return code

  // Initializations

  pEngineContext->recordNumber=0;
  rc=ERROR_ID_NO;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("AIRBORNE_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!(fileLength=STD_FileLength(specFp)))
   rc=ERROR_SetLast("AIRBORNE_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else
   pEngineContext->recordNumber=fileLength/(sizeof(AIRBORNE_DATA)+sizeof(double)*NDET[0]);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION AIRBORNE_Reli
// -----------------------------------------------------------------------------
/*!
   \fn      RC AIRBORNE_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  recordNo        the index of the record to read
   \param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise
   \param   [in]  localDay        if \a dateFlag is 1, the calendar day for the reference spectrum to search for
   \param   [in]  specFp          pointer to the spectra file to read
   \return  the code returned by \ref AIRBORNE_ReadRecord \n
            ERROR_ID_FILE_RECORD if the record is the spectrum is not a spectrum to analyze (sky or dark spectrum)\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC AIRBORNE_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO   *pRecord;                                                       // pointer to the record part of the engine context
  AIRBORNE_DATA  header;                                                        // record header
  double        *spectrum;                                                      // the current spectrum
  double         tmLocal;                                                       // measurement local time
  double         timeDec;
  int            nsec1,nsec2;
  RC             rc;                                                            // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  spectrum=(double *)pEngineContext->buffers.spectrum;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("AIRBORNE_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else {
    // Complete the reading of the record
    const int n_wavel = NDET[0];

    fseek(specFp,(recordNo-1)*(sizeof(AIRBORNE_DATA)+sizeof(double)*n_wavel),SEEK_SET);
    fread(&header,sizeof(AIRBORNE_DATA),1,specFp);
    fread(spectrum,sizeof(double)*n_wavel,1,specFp);

    pRecord->present_datetime.thedate.da_day = header.today.da_day;
    pRecord->present_datetime.thedate.da_mon = header.today.da_mon;
    pRecord->present_datetime.thedate.da_year = header.today.da_year;

    memcpy(&pRecord->uavBira.startTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));
    memcpy(&pRecord->uavBira.endTime.thedate,&pRecord->present_datetime.thedate,sizeof(struct date));

    pRecord->TDet=(double)-1.;
    pRecord->rejected=header.nrejMeas;
    pRecord->Tint = (double)header.exposureTime*0.001;
    pRecord->NSomme = header.naccMeas;

    pRecord->longitude=(double)header.longitude;
    pRecord->latitude=(double)header.latitude;
    pRecord->altitude=(double)header.altitude;

    pRecord->uavBira.servoSentPosition=(unsigned char)header.sentPosition;                     // UAV servo control : position byte sent to the PIC
    pRecord->uavBira.servoReceivedPosition=(unsigned char)header.receivedPosition;             // UAV servo control : position byte received by the PIC
    pRecord->uavBira.insideTemp=(float)header.insideTemp;
    pRecord->uavBira.outsideTemp=(float)header.outsideTemp;

    pRecord->uavBira.pressure=(float)header.pressure;
 	  pRecord->uavBira.dewPoint=(float)header.dewPoint;
 	  pRecord->uavBira.humidity=(float)header.humidity;
 	  pRecord->uavBira.altitudeP=(float)header.altitudeP;
 	  pRecord->uavBira.longitudeEnd=(float)header.longitudeEnd;
 	  pRecord->uavBira.latitudeEnd=(float)header.latitudeEnd;
 	  pRecord->uavBira.altitudeEnd=(float)header.altitudeEnd;
 	  pRecord->uavBira.pitch=(float)header.pitch;
 	  pRecord->uavBira.roll=(float)header.roll;
 	  pRecord->uavBira.heading=(float)header.heading;
	  
 	  if (header.useNexstarFlag)
 	   {
 	    pRecord->elevationViewAngle=(double)header.viewElev;
 	    pRecord->azimuthViewAngle=(double)header.viewAzim;
 	   }

 	  memcpy(&pRecord->uavBira.startTime.thetime,&header.now,sizeof(struct time));
 	  memcpy(&pRecord->uavBira.endTime.thetime,&header.endMeas,sizeof(struct time));

 	  pRecord->uavBira.startTime.millis=(int)header.msBegin;
 	  pRecord->uavBira.endTime.millis=(int)header.msEnd;
 	  pRecord->uavBira.startTime.microseconds=0;
    pRecord->uavBira.endTime.microseconds=0;

    nsec1=pRecord->uavBira.startTime.thetime.ti_hour*3600+pRecord->uavBira.startTime.thetime.ti_min*60+pRecord->uavBira.startTime.thetime.ti_sec;
    nsec2=pRecord->uavBira.endTime.thetime.ti_hour*3600+pRecord->uavBira.endTime.thetime.ti_min*60+pRecord->uavBira.endTime.thetime.ti_sec;

    if (nsec2<nsec1)
     nsec2+=86400;

 	  // memcpy(&pRecord->uavBira.gpsStartTime,&header.gpsTime,sizeof(struct time));           // to delete -> to confirm by Alexis
 	  // memcpy(&pRecord->uavBira.gpsEndTime,&header.gpsTimeEnd,sizeof(struct time));

	   pRecord->Tm=(double)(ZEN_NbSec(&pRecord->present_datetime.thedate,&header.now,0)+0.001*header.msBegin+ZEN_NbSec(&pRecord->present_datetime.thedate,&header.endMeas,0)+0.001*header.msEnd)*0.5;
    pRecord->Zm=(double)ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&pRecord->longitude,&pRecord->latitude,&pRecord->Azimuth);
    pRecord->TotalAcqTime=(double)header.totalTime;
    pRecord->TotalExpTime=(double)nsec2-nsec1;
	pRecord->maxdoas.measurementType=(header.measType!=0)?(char)header.measType:PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;

    if (header.endMeas.ti_hour || header.endMeas.ti_min || header.endMeas.ti_sec)
     pRecord->TimeDec=(double)(header.now.ti_hour+header.now.ti_min/60.+(header.now.ti_sec+0.001*header.msBegin)/3600.+header.endMeas.ti_hour+header.endMeas.ti_min/60.+(header.endMeas.ti_sec+0.001*header.msEnd)/3600.)*0.5;
    else
     pRecord->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+(header.now.ti_sec+0.001*header.msBegin)/3600.;

    timeDec=pRecord->TimeDec;

    if (!header.endMeas.ti_hour && !header.endMeas.ti_min && !header.endMeas.ti_sec)
     {
 	    memcpy(&header.endMeas,&header.now,sizeof(struct time));
 	    memcpy(&pRecord->present_datetime.thetime,&header.now,sizeof(struct time));
 	    pRecord->present_datetime.millis=0;
 	   }
 	  else
 	   {
      pRecord->present_datetime.thetime.ti_hour=(int)floor(timeDec);
      timeDec=(timeDec-(double)pRecord->present_datetime.thetime.ti_hour)*60.;
      pRecord->present_datetime.thetime.ti_min=(int)floor(timeDec);
      timeDec=(timeDec-(double)pRecord->present_datetime.thetime.ti_min)*60.;
      pRecord->present_datetime.thetime.ti_sec=(int)floor(timeDec);
      timeDec=(timeDec-(double)pRecord->present_datetime.thetime.ti_sec)*1000.;
      pRecord->present_datetime.millis=(int)floor(timeDec);
     }

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

//    if (dateFlag && (pRecord->localCalDay!=localDay))

    if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) ||
            (((fabs(pRecord->elevationViewAngle+1.)>EPSILON) || (fabs(pRecord->azimuthViewAngle+1.)>EPSILON)) &&
             ((pRecord->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
              (pRecord->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))))

     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }
