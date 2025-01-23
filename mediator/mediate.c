/*
  Qdoas is a cross-platform application for spectral analysis with the DOAS
  algorithm.  Copyright (C) 2007  S[&]T and BIRA
*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "mediate.h"

#include "mediate_common.h"
#include "engine_context.h"
#include "winsites.h"
#include "winsymb.h"
#include "winfiles.h"
#include "engine.h"
#include "analyse.h"
#include "spectrum_files.h"
#include "output.h"
#include "kurucz.h"
#include "svd.h"
#include "winthrd.h"

#include "radiance_ref.h"
#include "omi_read.h"
#include "omiv4_read.h"
#include "tropomi_read.h"
#include "gome1netcdf_read.h"
#include "apex_read.h"
#include "gems_read.h"
#include "mfc-read.h"

#include "matrix_netcdf_read.h"

int mediateRequestDisplaySpecInfo(void *engineContext,int page,void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext=(ENGINE_CONTEXT *)engineContext;
   PROJECT *pProject;                                                            // pointer to the project part of the engine context
   PRJCT_SPECTRA *pSpectra;                                                      // pointer to the spectra part of the project
   PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   struct date *pDay;                                                            // pointer to measurement date
   struct time *pTime;                                                           // pointer to measurement date
   struct datetime *pdateTime;
   int indexLine,indexColumn;
   char blankString[256];

   // Initializations

   pRecord=&pEngineContext->recordInfo;
   pProject=&pEngineContext->project;
   pSpectra=&pProject->spectra;
   pInstrumental=&pProject->instrumental;
   pDay=&pRecord->present_datetime.thedate;
   pTime=&pRecord->present_datetime.thetime;

   memset(blankString,' ',256);
   blankString[255]='\0';

   indexLine=1;
   indexColumn=2;

   mediateResponseCellInfo(page,0,3,responseHandle,blankString,blankString);

   if (strlen(pInstrumental->instrFunction))
    {
     if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Transmission file","%s",pInstrumental->instrFunction);
     else if (((pInstrumental->mfc.mfcMaskSpec!=0) && ((unsigned int)MFC_header.ty==pInstrumental->mfc.mfcMaskSpec)) ||
              ((pInstrumental->mfc.mfcMaskSpec==0) &&
               ((MFC_header.wavelength1==pInstrumental->mfc.mfcMaskInstr) ||
                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->mfc.wavelength))<(double)5.))))
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Transmission file","%s",pInstrumental->instrFunction);
    }

   if (strlen(pInstrumental->vipFile))
    {
     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dark current","%s",MFC_fileDark);
     else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
              (((pInstrumental->mfc.mfcMaskSpec!=0) && (((unsigned int)MFC_header.ty==pInstrumental->mfc.mfcMaskSpec) || ((unsigned int)MFC_header.ty==pInstrumental->mfc.mfcMaskInstr))) ||
               ((pInstrumental->mfc.mfcMaskSpec==0) &&
                ((MFC_header.wavelength1==pInstrumental->mfc.mfcMaskInstr) ||
                 (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->mfc.wavelength))<(double)5.)))))
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dark current","%s",MFC_fileDark);
     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Straylight correction","%s",pInstrumental->vipFile);
     else
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Interpixel","%s",pInstrumental->vipFile);
    }

  if (strlen(pInstrumental->offsetFile))
    {
      if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Offset","%s",MFC_fileOffset);
      else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
               (((MFC_header.ty!=0) &&
                 (((unsigned int)MFC_header.ty==pInstrumental->mfc.mfcMaskSpec) ||
                  ((unsigned int)MFC_header.ty==pInstrumental->mfc.mfcMaskDark) ||
                  ((unsigned int)MFC_header.ty==pInstrumental->mfc.mfcMaskInstr))) ||
                ((pInstrumental->mfc.mfcMaskSpec==0) &&
                 ((MFC_header.wavelength1==pInstrumental->mfc.mfcMaskDark) ||
                  (MFC_header.wavelength1==pInstrumental->mfc.mfcMaskInstr)||
                  (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->mfc.wavelength))<(double)5.)))))
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Offset","%s",MFC_fileOffset);
    }

  if (strlen(pInstrumental->dnlFile))
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Non linearity","%s",pInstrumental->dnlFile);

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d.%06d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,pRecord->present_datetime.microseconds);
  else if (pRecord->present_datetime.millis>0)
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d.%03d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,pRecord->present_datetime.millis);
  else
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);


  pDay=&pRecord->startDateTime.thedate;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_STARTDATE])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Start date","%02d/%02d/%d",pDay->da_day,pDay->da_mon,pDay->da_year);
  pDay=&pRecord->endDateTime.thedate;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ENDDATE])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"End date","%02d/%02d/%d",pDay->da_day,pDay->da_mon,pDay->da_year);


  pTime=&pRecord->startDateTime.thetime;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_STARTTIME])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Start time","%02d:%02d:%02d.%03d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec,pRecord->startDateTime.millis);
  pTime=&pRecord->endDateTime.thetime;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ENDTIME])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"End time","%02d:%02d:%02d.%03d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec,pRecord->endDateTime.millis);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_TOTALEXPTIME])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Total Experiment Time (sec)","%.6f",pRecord->TotalExpTime);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_TOTALACQTIME])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Total Acquisition Time (sec)","%.6f",pRecord->TotalAcqTime);

  if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_AIRBORNE) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_MOBILE))
   {
    pdateTime=&pRecord->uavBira.startTime;
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_STARTGPSTIME])
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Start time","%02d:%02d:%02d.%03d",pdateTime->thetime.ti_hour,pdateTime->thetime.ti_min,pdateTime->thetime.ti_sec,pdateTime->millis);
    pdateTime=&pRecord->uavBira.endTime;
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_ENDGPSTIME])
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"End time","%02d:%02d:%02d.%03d",pdateTime->thetime.ti_hour,pdateTime->thetime.ti_min,pdateTime->thetime.ti_sec,pdateTime->millis);
   }

  if (ANALYSE_swathSize>1)
    {
      if (pInstrumental->averageFlag)
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d (%d spectra averaged)",
                                pEngineContext->indexRecord,pEngineContext->recordNumber,pEngineContext->n_alongtrack);
      else
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d (measurement %d/%d, row %d/%d)",
                               pEngineContext->indexRecord,pEngineContext->recordNumber,
                               1+pRecord->i_alongtrack,pEngineContext->n_alongtrack,
                               1+pRecord->i_crosstrack,pEngineContext->n_crosstrack);
    }
  else
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord,pEngineContext->recordNumber);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_INDEX_ALONGTRACK])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Along-track index","%d",pRecord->i_alongtrack);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_INDEX_CROSSTRACK])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cross-track index","%d",pRecord->i_crosstrack);

  if (strlen(pRecord->Nom) && (pSpectra->fieldsFlag[PRJCT_RESULTS_NAME]))
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record name","%s",pRecord->Nom);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_GROUNDP_QF])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"GroundPixel QF","%d",pRecord->ground_pixel_QF);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_XTRACK_QF])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"XTrack QF","%d",pRecord->xtrack_QF);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_PIXEL])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Pixel number","%d",pRecord->gome.pixelNumber);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_PIXEL_TYPE])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Pixel type","%d",pRecord->gome.pixelType);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ORBIT])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->satellite.orbit_number);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_TINT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Exposure time",(pRecord->Tint>=0.)?"%.3f sec":"N/A",pRecord->Tint);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANS])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scans taken into account","%d",pRecord->NSomme);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_NREJ])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Rejected scans","%d",pRecord->rejected);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SZA])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Solar Zenith angle","%-.3f",pRecord->Zm);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_AZIM])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Solar Azimuth angle","%.3f",pRecord->Azimuth);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_VIEW_ELEVATION])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Elevation viewing angle","%.3f",pRecord->elevationViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_VIEW_ZENITH])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Zenith viewing angle","%.3f",pRecord->zenithViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_VIEW_AZIMUTH])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Azimuth viewing angle","%.3f",pRecord->azimuthViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LOS_ZA])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Viewing Zenith angle","%.3f",pRecord->zenithViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LOS_AZIMUTH])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Viewing Azimuth angle","%.3f",pRecord->azimuthViewAngle);

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANNING])
     {
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning angle","%.3f",pRecord->mkzy.scanningAngle);
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning angle 2","%.3f",pRecord->mkzy.scanningAngle2);
     }
   }

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANNING])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Telescope Scanning angle","%.3f",pRecord->als.scanningAngle);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_COMPASS])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning compass angle","%.3f",pRecord->als.compassAngle);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_PITCH])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning compass angle","%.3f",pRecord->als.pitchAngle);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_ROLL])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning compass angle","%.3f",pRecord->als.rollAngle);

    if (pSpectra->fieldsFlag[PRJCT_RESULTS_FILTERNUMBER])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Filter number","%d",pRecord->ccd.filterNumber);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_HEADTEMPERATURE])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Temperature in optic head","%.3f deg",pRecord->ccd.headTemperature);

    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_DIODES])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Diodes","%.3f %.3f %.3f %.3f",pRecord->ccd.diodes[0],pRecord->ccd.diodes[1],pRecord->ccd.diodes[2],pRecord->ccd.diodes[3]);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_TARGETAZIMUTH])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Target azimuth","%.3f deg",pRecord->ccd.targetAzimuth);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_TARGETELEVATION])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Target elevation","%.3f deg",pRecord->ccd.targetElevation);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_SATURATED])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Saturated","%s",(pRecord->ccd.saturatedFlag)?"yes":"no");

    if (pSpectra->fieldsFlag[PRJCT_RESULTS_PRECALCULATED_FLUXES])
     {
         char str1[80],str2[80];

         sprintf(str1,"Flux (%g)",pRecord->ccd.wve1);
         sprintf(str2,"Flux (%g)",pRecord->ccd.wve2);

      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,str1,"%.3f",pRecord->ccd.flux1);
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,str2,"%.3f",pRecord->ccd.flux2);
     }
   }

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_MEASTYPE] && is_maxdoas(pInstrumental->readOutFormat))
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Measurement type","%s",MAXDOAS_measureTypes[pRecord->maxdoas.measurementType]);   // TEST FORMAT ASCII

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_TDET])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Detector temperature","%.3f",pRecord->TDet);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_MIRROR_ERROR])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Mirror status","%.3f",(pRecord->mirrorError==1)?"!!! PROBLEM !!!":"OK");
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_COOLING_STATUS])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cooler status","%.3f",(pRecord->coolingStatus==0)?"!!! UNLOCKED !!!":"Locked");

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LONGIT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Longitude","%.6f",pRecord->longitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LATIT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Latitude","%.6f",pRecord->latitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ALTIT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Altitude","%.6f",pRecord->altitude);

  if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_AIRBORNE) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_MOBILE))
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_LONGITEND])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Longitude End","%.6f",pRecord->uavBira.longitudeEnd);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_LATITEND])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Latitude End","%.6f",pRecord->uavBira.latitudeEnd);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_ALTITEND])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Altitude End","%.6f",pRecord->uavBira.altitudeEnd);
   }

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_CLOUD])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cloud fraction","%.3f",pRecord->satellite.cloud_fraction);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_CLOUDTOPP])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cloud top pressure","%.3f",pRecord->satellite.cloud_top_pressure);

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SCANDIRECTION])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scan direction","%d",pRecord->gome2.scanDirection);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SAA])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"SAA flag","%d",pRecord->gome2.saaFlag);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SUNGLINT_RISK])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Sunglint risk flag","%d",pRecord->gome2.sunglintDangerFlag);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Sunglint high risk flag","%d",pRecord->gome2.sunglintHighDangerFlag);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_RAINBOW])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Rainbow flag","%d",pRecord->gome2.rainbowFlag);
   }

  if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_AIRBORNE) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_MOBILE))
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_SERVO_BYTE_SENT])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Servo position byte sent","%d",(int)pRecord->uavBira.servoSentPosition);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Servo position byte received","%d",(int)pRecord->uavBira.servoReceivedPosition);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_INSIDE_TEMP])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Inside temperature","%.3f",(float)pRecord->uavBira.insideTemp);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_OUTSIDE_TEMP])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Outside temperature","%.3f",(float)pRecord->uavBira.outsideTemp);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_PRESSURE])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Pressure","%.3f",(float)pRecord->uavBira.pressure);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_HUMIDITY])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Humidity","%.3f",(float)pRecord->uavBira.humidity);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_DEWPOINT])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dewpoint","%.3f",(float)pRecord->uavBira.dewPoint);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_PITCH])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"pitch","%.3f",(float)pRecord->uavBira.pitch);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_ROLL])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Roll","%.3f",(float)pRecord->uavBira.roll);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_HEADING])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Heading","%.3f",(float)pRecord->uavBira.heading);
   }

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANINDEX])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scan index","%d",pRecord->maxdoas.scanIndex);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ZENITH_BEFORE])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Zenith before index","%d",1+pRecord->maxdoas.zenithBeforeIndex);   // Add one because record indexes are 1 based
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ZENITH_AFTER])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Zenith after index","%d",1+pRecord->maxdoas.zenithAfterIndex);    // Add one because record indexes are 1 based
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SAT_LON])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Satellite longitude","%g",pRecord->satellite.longitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SAT_LAT])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Satellite latitude","%g",pRecord->satellite.latitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SAT_HEIGHT])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Satellite height","%g",pRecord->satellite.altitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_EARTH_RADIUS])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Earth radius","%g",pRecord->satellite.earth_radius);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SAT_SZA])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Satellite solar zenith angle","%g",pRecord->satellite.sza);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SAT_SAA])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Satellite solar azimuth angle","%g",pRecord->satellite.saa);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SAT_VZA])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Satellite viewing zenith angle","%g",pRecord->satellite.vza);

  // Return

  return indexLine;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MediateRequestPlotSpectra
// -----------------------------------------------------------------------------
// PURPOSE       Plot all the spectra related to a record
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

void mediateRequestPlotSpectra(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
{
   // Declarations

   PROJECT *pProject;                                                           // pointer to the project part of the engine context
   PRJCT_SPECTRA *pSpectra;                                                     // pointer to the spectra part of the project
   PRJCT_INSTRUMENTAL *pInstrumental;                                           // pointer to the instrumental part of the project
   BUFFERS *pBuffers;                                                           // pointer to the buffers part of the engine context
   RECORD_INFO *pRecord;                                                        // pointer to the record part of the engine context
   char tmpString[80];                                                          // buffer for formatted strings
   char *fileName;                                                              // the name of the current file

   // Initializations

   pRecord=&pEngineContext->recordInfo;
   pBuffers=&pEngineContext->buffers;
   pProject=&pEngineContext->project;
   pSpectra=&pProject->spectra;
   pInstrumental=&pProject->instrumental;

   const int n_wavel = NDET[pRecord->i_crosstrack];

   fileName=pEngineContext->fileInfo.fileName;

   if (ANALYSE_plotRef && (ANALYSE_swathSize>1)) {
    mediateResponseRetainPage(plotPageRef,responseHandle);
   }
   if (ANALYSE_plotKurucz) {
    mediateResponseRetainPage(plotPageCalib,responseHandle);
   }

   sprintf(tmpString,"Spectrum (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

   if (pProject->spectra.displaySpectraFlag) {
     double *tempSpectrum;
     int i;

     const char* y_units;
     switch (pInstrumental->readOutFormat) {
       // satellite formats have calibrated radiances:
     case PRJCT_INSTR_FORMAT_GDP_BIN:
     case PRJCT_INSTR_FORMAT_GOME1_NETCDF :
     case PRJCT_INSTR_FORMAT_SCIA_PDS:
     case PRJCT_INSTR_FORMAT_GOME2:
     case PRJCT_INSTR_FORMAT_OMI:
     case PRJCT_INSTR_FORMAT_GEMS:
       y_units = "photons / (nm s cm<sup>2</sup>)";
       break;
     case PRJCT_INSTR_FORMAT_OMIV4:
     case PRJCT_INSTR_FORMAT_TROPOMI:
       y_units = "mol photons / (nm s m<sup>2</sup>)";
       break;
     default:
       y_units = "Counts";
       break;
     }

     if ((tempSpectrum=(double *)MEMORY_AllocDVector("mediateRequestPlotSpectra","tempSpectrum",0,n_wavel-1))!=NULL) {
       memcpy(tempSpectrum,pBuffers->spectrum,sizeof(double)*n_wavel);

        if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC_BIRA) &&
            (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC) &&
            (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC_STD) &&
            (pEngineContext->buffers.instrFunction!=NULL)) {
         for (i=0;i<n_wavel;i++)
           if (pBuffers->instrFunction[i]==(double)0.)
             tempSpectrum[i]=(double)0.;
           else
             tempSpectrum[i]/=pBuffers->instrFunction[i];
       }

       const char *tmpTitle;
       if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC_BIRA) || ((pEngineContext->recordInfo.mfcBira.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_DARK) && (pEngineContext->recordInfo.mfcBira.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFSET)))
         tmpTitle = "Spectrum";
       else if (pEngineContext->recordInfo.mfcBira.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_DARK)
         tmpTitle = "Dark Current";
       else if (pEngineContext->recordInfo.mfcBira.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFSET)
         tmpTitle = "Offset";

       MEDIATE_PLOT_CURVES(plotPageSpectrum, Spectrum, allowFixedScale, tmpTitle, "Wavelength (nm)", y_units, responseHandle,
                           CURVE(.name=tmpTitle, .x=pBuffers->lambda, .y=tempSpectrum, .length=n_wavel));
       mediateResponseLabelPage(plotPageSpectrum, fileName, tmpString, responseHandle);

       MEMORY_ReleaseDVector(__func__,"spectrum",tempSpectrum,0);
     }

     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY) {
       if ((pBuffers->offset!=NULL) && pEngineContext->recordInfo.mkzy.offsetFlag) {
         sprintf(tmpString,"Offset");
         MEDIATE_PLOT_CURVES(plotPageOffset, Spectrum, forceAutoScale, tmpString, "Wavelength (nm)", "Counts", responseHandle,
                             CURVE(.name=tmpString, .x=pBuffers->lambda, .y=pBuffers->offset, .length=n_wavel));
         mediateResponseLabelPage(plotPageOffset, fileName, tmpString, responseHandle);
       }

       if ((pBuffers->darkCurrent!=NULL) && pEngineContext->recordInfo.mkzy.darkFlag) {
         sprintf(tmpString,(pEngineContext->recordInfo.mkzy.offsetFlag)?"Dark current":"Offset");
         MEDIATE_PLOT_CURVES(plotPageDarkCurrent, Spectrum, forceAutoScale,tmpString, "Wavelength (nm)", "Counts", responseHandle,
                             CURVE(.name=tmpString, .x=pBuffers->lambda, .y=pBuffers->darkCurrent, .length=n_wavel));
         mediateResponseLabelPage(plotPageDarkCurrent, fileName, tmpString, responseHandle);
       }

       if ((pBuffers->scanRef!=NULL) && pEngineContext->recordInfo.mkzy.skyFlag) {
         sprintf(tmpString,"Sky spectrum");

         MEDIATE_PLOT_CURVES(plotPageIrrad, Spectrum, forceAutoScale, "Sky spectrum", "Wavelength (nm)", "Counts", responseHandle,
                             CURVE(.name="Sky spectrum", .x=pBuffers->lambda, .y=pBuffers->scanRef, .length=n_wavel));
         mediateResponseLabelPage(plotPageIrrad, fileName, tmpString, responseHandle);
       }
      }

     if ((
         #ifdef PRJCT_INSTR_FORMAT_OLD
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
         #endif 
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)) &&
         (pEngineContext->fileInfo.darkFp!=NULL) && (pBuffers->darkCurrent!=NULL)) {
       sprintf(tmpString,"Dark current (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       MEDIATE_PLOT_CURVES(plotPageDarkCurrent, Spectrum, forceAutoScale, "Dark current", "Wavelength (nm)", "Counts", responseHandle,
                           CURVE(.name="Dark current", .x=pBuffers->lambda, .y=pBuffers->darkCurrent, .length=n_wavel));
       mediateResponseLabelPage(plotPageDarkCurrent, fileName, tmpString, responseHandle);
     }

     if (pBuffers->sigmaSpec!=NULL) {
       sprintf(tmpString,"Error (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       MEDIATE_PLOT_CURVES(plotPageErrors, Spectrum, forceAutoScale, "Error", "Wavelength (nm)", "Counts", responseHandle,
                           CURVE(.name="Error", .x=pBuffers->lambda, .y=pBuffers->sigmaSpec, .length=n_wavel));
       mediateResponseLabelPage(plotPageErrors, fileName, tmpString, responseHandle);
      }

     // satellite formats have an irradiance spectrum
     if (pBuffers->irrad!=NULL
         && ( !(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI || // for OMI and Tropomi, irradiance is stored in separate file, which is only read during analysis
                pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMIV4 ||
                pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI ||
                pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GEMS ||
                pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF)
              || THRD_id==THREAD_TYPE_ANALYSIS) ) {

       MEDIATE_PLOT_CURVES(plotPageIrrad, Spectrum, forceAutoScale, "Irradiance spectrum", "Wavelength/(nm)", y_units, responseHandle,
                           CURVE(.name="Irradiance spectrum", .x=pBuffers->lambda_irrad, .y=pBuffers->irrad, .length=n_wavel));
       mediateResponseLabelPage(plotPageIrrad, fileName, "Irradiance", responseHandle);
     }

     if ((
         #ifdef PRJCT_INSTR_FORMAT_OLD
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
         #endif 
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)) &&
         (pBuffers->specMax!=NULL) &&
         (pRecord->NSomme>1)) {
       sprintf(tmpString,"SpecMax (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       MEDIATE_PLOT_CURVES(plotPageSpecMax, SpecMax, allowFixedScale, "SpecMax", "Scans number", "Signal Maximum", responseHandle,
                           CURVE(.name="SpecMax", .x=pBuffers->specMaxx, .y=pBuffers->specMax, .length=pRecord->rejected+pRecord->NSomme));
       mediateResponseLabelPage(plotPageSpecMax, fileName, tmpString, responseHandle);
      }

    if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) && (strlen(pInstrumental->imagePath)>0)) {
      sprintf(tmpString,"Sky picture (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);
      mediateResponsePlotImage(plotPageImage,(pRecord->ccd.indexImage!=ITEM_NONE)?CCD_GetImageFile(pRecord->ccd.indexImage):"./no-image-available.jpg","Sky picture",responseHandle);
      mediateResponseLabelPage(plotPageImage, fileName, tmpString, responseHandle);
     }
    }

   if (pSpectra->displaySpectraFlag && pSpectra->displayDataFlag)
    mediateRequestDisplaySpecInfo(pEngineContext,plotPageSpectrum,responseHandle);
 }

// =================================
// CREATE/DESTROY THE ENGINE CONTEXT
// =================================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestCreateEngineContext
// -----------------------------------------------------------------------------
// PURPOSE       This function is called on program start.  It creates a single
//               context for safely accessing its features through the mediator
//               layer.  The engine context is never destroyed before the user
//               exits the program.
//
// RETURN        On success 0 is returned and the value of handleEngine is set,
//               otherwise -1 is retured and the value of handleEngine is undefined.
// -----------------------------------------------------------------------------

int mediateRequestCreateEngineContext(void **engineContext, void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext;

   if ( (pEngineContext=(ENGINE_CONTEXT *)(*engineContext=EngineCreateContext() ) ) ==NULL )
    ERROR_DisplayMessage(responseHandle);

   return (pEngineContext!=NULL)?0:-1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestDestroyEngineContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the engine context when the user exits the program.
//
// RETURN        Zero is returned on success, -1 otherwise.
// -----------------------------------------------------------------------------

int mediateRequestDestroyEngineContext(void *engineContext, void *responseHandle)
 {
   return (!EngineDestroyContext((ENGINE_CONTEXT *)engineContext))?0:-1;
 }

// ==============================================================
// TRANSFER OF PROJECT PROPERTIES FROM THE MEDIATOR TO THE ENGINE
// ==============================================================

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectDisplay
// -----------------------------------------------------------------------------
// PURPOSE       Display part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectDisplay(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_display_t *pMediateSpectra)
 {
   // Declaration

   INDEX i;

   // Control of what to display

   pEngineSpectra->displaySpectraFlag=pMediateSpectra->requireSpectra;
   pEngineSpectra->displayDataFlag=pMediateSpectra->requireData;
   pEngineSpectra->displayCalibFlag=pMediateSpectra->requireCalib;
   pEngineSpectra->displayFitFlag=pMediateSpectra->requireFits;

   memset(pEngineSpectra->fieldsFlag,0,PRJCT_RESULTS_MAX*sizeof(int));

   for (i=0;i<pMediateSpectra->selection.nSelected;i++)
    pEngineSpectra->fieldsFlag[(int)pMediateSpectra->selection.selected[i]]=(char)1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSelection
// -----------------------------------------------------------------------------
// PURPOSE       Selection part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSelection(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_selection_t *pMediateSpectra)
 {
   // Declarations

   float tmp;

   // Spectral record range

   pEngineSpectra->noMin=pMediateSpectra->recordNumberMinimum;
   pEngineSpectra->noMax=pMediateSpectra->recordNumberMaximum;

   // SZA (Solar Zenith Angle) range of interest

   pEngineSpectra->SZAMin=(float)pMediateSpectra->szaMinimum;
   pEngineSpectra->SZAMax=(float)pMediateSpectra->szaMaximum;
   pEngineSpectra->SZADelta=(float)pMediateSpectra->szaDelta;

   // Elevation range of interest

   pEngineSpectra->elevMin=(float)pMediateSpectra->elevationMinimum;
   pEngineSpectra->elevMax=(float)pMediateSpectra->elevationMaximum;
   pEngineSpectra->elevTol=(float)pMediateSpectra->elevationTolerance+1.e-4;

   // Angle to use for the reference spectrum

   pEngineSpectra->refAngle=(float)pMediateSpectra->refAngle;
   pEngineSpectra->refTol=(float)pMediateSpectra->refTolerance+1.e-4;;

   // Cloud fraction

   pEngineSpectra->cloudMin=(float)pMediateSpectra->cloudFractionMinimum;
   pEngineSpectra->cloudMax=(float)pMediateSpectra->cloudFractionMaximum;

   if (pEngineSpectra->cloudMin>=pEngineSpectra->cloudMax+EPSILON)
    {
     tmp=pEngineSpectra->cloudMin;
     pEngineSpectra->cloudMin=pEngineSpectra->cloudMax;
     pEngineSpectra->cloudMax=tmp;
    }

   // QDOAS ??? to move to the instrumental page

   pEngineSpectra->namesFlag=pMediateSpectra->useNameFile;
   pEngineSpectra->darkFlag=pMediateSpectra->useDarkFile;

   // Geolocation

   pEngineSpectra->mode=pMediateSpectra->geo.mode;
   pEngineSpectra->radius=
     pEngineSpectra->longMin=
     pEngineSpectra->longMax=
     pEngineSpectra->latMin=
     pEngineSpectra->latMax=(double)0.;


   switch (pEngineSpectra->mode)
    {
     // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :

      pEngineSpectra->radius=pMediateSpectra->geo.circle.radius;
      pEngineSpectra->longMin=(float)pMediateSpectra->geo.circle.centerLongitude;
      pEngineSpectra->latMin=(float)pMediateSpectra->geo.circle.centerLatitude;

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :

      pEngineSpectra->longMin=pMediateSpectra->geo.rectangle.westernLongitude;
      pEngineSpectra->longMax=pMediateSpectra->geo.rectangle.easternLongitude;
      pEngineSpectra->latMin=pMediateSpectra->geo.rectangle.southernLatitude;
      pEngineSpectra->latMax=pMediateSpectra->geo.rectangle.northernLatitude;

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
      pEngineSpectra->radius=pMediateSpectra->geo.sites.radius;
      break;
      // ----------------------------------------------------------------------------
    }

 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Analysis part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectAnalysis(PRJCT_ANLYS *pEngineAnalysis,const mediate_project_analysis_t *pMediateAnalysis)
 {
   pEngineAnalysis->method=pMediateAnalysis->methodType;                         // analysis method
   pEngineAnalysis->fitWeighting=pMediateAnalysis->fitType;                      // least-squares fit weighting
   pEngineAnalysis->interpol=pMediateAnalysis->interpolationType;                // interpolation
   pEngineAnalysis->convergence=pMediateAnalysis->convergenceCriterion;          // convergence criterion
   pEngineAnalysis->spike_tolerance=pMediateAnalysis->spike_tolerance;
   pEngineAnalysis->securityGap=pMediateAnalysis->interpolationSecurityGap;      // security pixels to take in order to avoid interpolation problems at the edge of the spectral window
   pEngineAnalysis->maxIterations=pMediateAnalysis->maxIterations;               // maximum number of iterations

 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectCalibration
// -----------------------------------------------------------------------------
// PURPOSE       Calibration part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectCalibration(PRJCT_KURUCZ *pEngineCalibration,CALIB_FENO *pEngineCalibFeno,const mediate_project_calibration_t *pMediateCalibration,int displayFitFlag)
 {
   strcpy(pEngineCalibration->file,pMediateCalibration->solarRefFile);          // kurucz file
   strcpy(pEngineCalibration->slfFile,pMediateCalibration->slfFile);            // slit function file

   pEngineCalibration->analysisMethod=pMediateCalibration->methodType;          // analysis method type
   pEngineCalibration->windowsNumber=pMediateCalibration->subWindows;           // number of windows
   pEngineCalibration->divisionMode=pMediateCalibration->divisionMode;          // number of windows
   pEngineCalibration->fwhmPolynomial=pMediateCalibration->sfpDegree;           // security gap in pixels numbers
   pEngineCalibration->shiftPolynomial=pMediateCalibration->shiftDegree;        // degree of polynomial to use

   if (displayFitFlag)
    {
     pEngineCalibration->displayFit=pMediateCalibration->requireFits;           // display fit flag
     pEngineCalibration->displayResidual=pMediateCalibration->requireResidual;  // display new calibration flag
     pEngineCalibration->displayShift=pMediateCalibration->requireShiftSfp;     // display shift/Fwhm in each pixel
     pEngineCalibration->displaySpectra=pMediateCalibration->requireSpectra;    // display fwhm in each pixel
    }
   else
    pEngineCalibration->displayFit=pEngineCalibration->displayResidual=pEngineCalibration->displayShift=pEngineCalibration->displaySpectra=0;

   pEngineCalibration->fwhmFit=(pMediateCalibration->lineShape>0)?1:0;          // force fit of fwhm while applying Kurucz
   pEngineCalibration->lambdaLeft=pMediateCalibration->wavelengthMin;           // minimum wavelength for the spectral interval
   pEngineCalibration->lambdaRight=pMediateCalibration->wavelengthMax;          // maximum wavelength for the spectral interval
   pEngineCalibration->windowSize=pMediateCalibration->windowSize;              // size of subwindows (sliding case)

   if (pEngineCalibration->divisionMode==PRJCT_CALIB_WINDOWS_CUSTOM)
    for (int i=0;i<pEngineCalibration->windowsNumber;i++)
     {
         pEngineCalibration->customLambdaMin[i]=pMediateCalibration->customLambdaMin[i];
         pEngineCalibration->customLambdaMax[i]=pMediateCalibration->customLambdaMax[i];
     }

   pEngineCalibration->invPolyDegree=2*pMediateCalibration->lorentzOrder;       // degree of the lorentzian

   pEngineCalibration->preshiftFlag=pMediateCalibration->preshiftFlag;          // calculate the preshift
   pEngineCalibration->preshiftMin=pMediateCalibration->preshiftMin;            // minimum value for the preshift
   pEngineCalibration->preshiftMax=pMediateCalibration->preshiftMax;            // maximum value for the preshift

   switch(pMediateCalibration->lineShape)
    {
     // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_FILE :
      pEngineCalibration->fwhmType=SLIT_TYPE_FILE;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_ERF :
      pEngineCalibration->fwhmType=SLIT_TYPE_ERF;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_INVPOLY :
      pEngineCalibration->fwhmType=SLIT_TYPE_INVPOLY;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_VOIGT :
      pEngineCalibration->fwhmType=SLIT_TYPE_VOIGT;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_AGAUSS :
      pEngineCalibration->fwhmType=SLIT_TYPE_AGAUSS;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_SUPERGAUSS :
      pEngineCalibration->fwhmType=SLIT_TYPE_SUPERGAUSS;
      break;
      // ---------------------------------------------------------------------------
    default :
      pEngineCalibration->fwhmType=SLIT_TYPE_GAUSS;
      break;
      // ---------------------------------------------------------------------------
    }

   // !!! tables will be loaded by mediateRequestSetAnalysisWindows !!!

   memcpy(&pEngineCalibFeno->crossSectionList,&pMediateCalibration->crossSectionList,sizeof(cross_section_list_t));
   memcpy(&pEngineCalibFeno->linear,&pMediateCalibration->linear,sizeof(struct anlyswin_linear));
   memcpy(pEngineCalibFeno->sfp,pMediateCalibration->sfp,sizeof(struct calibration_sfp)*4); // SFP1 .. SFP4
   memcpy(&pEngineCalibFeno->shiftStretchList,&pMediateCalibration->shiftStretchList,sizeof(shift_stretch_list_t));
   memcpy(&pEngineCalibFeno->outputList,&pMediateCalibration->outputList,sizeof(output_list_t));
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectUndersampling
// -----------------------------------------------------------------------------
// PURPOSE       Undersampling part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectUndersampling(PRJCT_USAMP *pEngineUsamp,const mediate_project_undersampling_t *pMediateUsamp)
 {
   strcpy(pEngineUsamp->kuruczFile,pMediateUsamp->solarRefFile);

   pEngineUsamp->method=pMediateUsamp->method;
   pEngineUsamp->phase=pMediateUsamp->shift;
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Instrumental part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectInstrumental(PRJCT_INSTRUMENTAL *pEngineInstrumental,const mediate_project_instrumental_t *pMediateInstrumental)
 {
   INDEX indexCluster;

   pEngineInstrumental->readOutFormat=(char)pMediateInstrumental->format;       // File format
   pEngineInstrumental->saaConvention=pMediateInstrumental->saaConvention;      // Solar azimuth convention
   strcpy(pEngineInstrumental->observationSite,pMediateInstrumental->siteName); // Observation site

   switch (pEngineInstrumental->readOutFormat)
    {
     // ----------------------------------------------------------------------------
    #ifdef PRJCT_INSTR_FORMAT_OLD
    case PRJCT_INSTR_FORMAT_ACTON :                                                                 // Acton (NILU)

      NDET[0]=1024;                                                                     // size of the detector

      pEngineInstrumental->user=pMediateInstrumental->acton.niluType;                                // old or new format

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->acton.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->acton.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER :                                                                // Logger (PDA,CCD or HAMAMATSU)

      NDET[0]=1024;                                                                                     // size of the detector
      pEngineInstrumental->azimuthFlag=(int)pMediateInstrumental->logger.flagAzimuthAngle;           // format including or not the azimuth angle
      pEngineInstrumental->user=pMediateInstrumental->logger.spectralType;                           // spectrum type (offaxis or zenith)

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->logger.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->logger.transmissionFunctionFile);     // instrumental function file

      break;
    #endif  
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ASCII :                                                                 // Format ASCII

      NDET[0]=pMediateInstrumental->ascii.detectorSize;                                                 // size of the detector

      pEngineInstrumental->ascii.format=pMediateInstrumental->ascii.format;                          // format line or column
      pEngineInstrumental->ascii.szaSaveFlag=pMediateInstrumental->ascii.flagZenithAngle;            // 1 if the solar zenith angle information is saved in the file
      pEngineInstrumental->ascii.azimSaveFlag=pMediateInstrumental->ascii.flagAzimuthAngle;          // 1 if the solar azimuth angle information is saved in the file
      pEngineInstrumental->ascii.elevSaveFlag=pMediateInstrumental->ascii.flagElevationAngle;        // 1 if the viewing elevation angle information is saved in the file
      pEngineInstrumental->ascii.timeSaveFlag=pMediateInstrumental->ascii.flagTime;                  // 1 if the time information is saved in the file
      pEngineInstrumental->ascii.dateSaveFlag=pMediateInstrumental->ascii.flagDate;                  // 1 if the date information is saved in the file
      pEngineInstrumental->ascii.lambdaSaveFlag=pMediateInstrumental->ascii.flagWavelength;          // 1 if the wavelength calibration is saved with spectra in the file

      pEngineInstrumental->offsetFlag=pMediateInstrumental->ascii.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->ascii.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->ascii.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->ascii.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->ascii.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    #ifdef PRJCT_INSTR_FORMAT_OLD  
    case PRJCT_INSTR_FORMAT_PDAEGG_OLD :                                                            // PDA EG&G (spring 94)

      NDET[0]=1024;                                                                                     // size of the detector

      pEngineInstrumental->azimuthFlag=(int)0;                                                       // format including or not the azimuth angle
      pEngineInstrumental->user=PRJCT_INSTR_IASB_TYPE_ALL;                                           // spectrum type (offaxis or zenith)

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdaeggold.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdaeggold.transmissionFunctionFile);     // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG :                                                                // PDA EG&G (sept. 94 until now)

      NDET[0]=1024;                                                                                     // size of the detector

      pEngineInstrumental->azimuthFlag=(int)pMediateInstrumental->pdaegg.flagAzimuthAngle;           // format including or not the azimuth angle
      pEngineInstrumental->user=pMediateInstrumental->pdaegg.spectralType;                           // spectrum type (offaxis or zenith)

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdaegg.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdaegg.transmissionFunctionFile);     // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :                                                           // PDA SI (IASB)

      NDET[0]=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->pdasieasoe.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->pdasieasoe.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->pdasieasoe.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdasieasoe.calibrationFile); // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdasieasoe.transmissionFunctionFile); // instrumental function file

      break;
    #endif  
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OCEAN_OPTICS :                                                                 // Format OCEAN OPTICS

      NDET[0]=pMediateInstrumental->oceanoptics.detectorSize;                                           // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->oceanoptics.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->oceanoptics.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->oceanoptics.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->oceanoptics.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->oceanoptics.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_VIS :                                                              // SAOZ PCD/NMOS 512

      NDET[0]=512;                                                                                      // size of the detector

      pEngineInstrumental->saoz.spectralRegion=pMediateInstrumental->saozvis.spectralRegion;         // spectral region (UV or visible)
      pEngineInstrumental->saoz.spectralType=pMediateInstrumental->saozvis.spectralType;             // spectral type (zenith sky or pointed measuremets

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->saozvis.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->saozvis.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :                                                              // SAOZ EFM 1024

      NDET[0]=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->saozefm.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->saozefm.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->saozefm.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->saozefm.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->saozefm.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_MOBILE :                                                              // BIRA MOBILE

      NDET[0]=(pMediateInstrumental->biramobile.detectorSize)?pMediateInstrumental->biramobile.detectorSize:2048;  // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->biramobile.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->biramobile.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->biramobile.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->biramobile.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->biramobile.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :                                                              // BIRA AIRBORNE

      NDET[0]=(pMediateInstrumental->biraairborne.detectorSize)?pMediateInstrumental->biraairborne.detectorSize:2048;  // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->biraairborne.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->biraairborne.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->biraairborne.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->biraairborne.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->biraairborne.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_APEX:
      for (int i=0; i<MAX_SWATHSIZE; ++i) {
        NDET[i] = APEX_INIT_LENGTH;
        pEngineInstrumental->use_row[i]=false;
      }

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->apex.calibrationFile);
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->apex.transmissionFunctionFile);

      OMI_TrackSelection(pMediateInstrumental->apex.trackSelection,pEngineInstrumental->use_row);

      break;
    #ifdef PRJCT_INSTR_FORMAT_OLD  
    case PRJCT_INSTR_FORMAT_RASAS :                                                                 // Format RASAS (INTA)

      NDET[0]=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->rasas.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->rasas.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->rasas.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->rasas.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->rasas.transmissionFunctionFile);      // instrumental function file

      break;
    #endif  
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :                                                                  // NOAA

      NDET[0]=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->noaa.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->noaa.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->noaa.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->noaa.calibrationFile);       // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->noaa.transmissionFunctionFile);       // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SCIA_PDS :                                                              // SCIAMACHY calibrated Level 1 data in PDS format

      NDET[0]=1024;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->sciapds.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->sciapds.transmissionFunctionFile);    // instrumental function file
      strcpy(pEngineInstrumental->dnlFile,pMediateInstrumental->sciapds.detectorNonLinearityFile);   // correction for the non linearity of the detector

      pEngineInstrumental->scia.sciaChannel=pMediateInstrumental->sciapds.channel;

      for (indexCluster=SCIA_clusters[pEngineInstrumental->scia.sciaChannel][0];
           indexCluster<=SCIA_clusters[pEngineInstrumental->scia.sciaChannel][1];
           indexCluster++)

       pEngineInstrumental->scia.sciaCluster[indexCluster-SCIA_clusters[pEngineInstrumental->scia.sciaChannel][0]]=
         (pMediateInstrumental->sciapds.clusters[indexCluster])?1:0;

      memcpy(pEngineInstrumental->scia.sciaReference,pMediateInstrumental->sciapds.sunReference,4);

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_BIN :                                                               // GOME WinDOAS BINARY format

      NDET[0]=1024;                                                                                     // Could be reduced by Set function

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gdpbin.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gdpbin.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->gome.bandType=pMediateInstrumental->gdpbin.bandType;
      pEngineInstrumental->gome.pixelType=pMediateInstrumental->gdpbin.pixelType-1;

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GOME1_NETCDF :                                                               // GOME ERS2 netCDF format

      for(unsigned int i=0; i<MAX_SWATHSIZE; ++i) {
        NDET[i]=1024;
        pEngineInstrumental->use_row[i]=false;
      }                                                                                    // Could be reduced by Set function

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gdpnetcdf.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gdpnetcdf.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->gomenetcdf.bandType=pMediateInstrumental->gdpnetcdf.bandType;
      pEngineInstrumental->gomenetcdf.pixelType=pMediateInstrumental->gdpnetcdf.pixelType;

      if (pEngineInstrumental->gomenetcdf.pixelType==PRJCT_INSTR_GOME1_PIXEL_BACKSCAN)
       pEngineInstrumental->use_row[3]=true;
      else if (pEngineInstrumental->gomenetcdf.pixelType==PRJCT_INSTR_GOME1_PIXEL_GROUND)
       pEngineInstrumental->use_row[0]=
       pEngineInstrumental->use_row[1]=
       pEngineInstrumental->use_row[2]=true;
      else
       pEngineInstrumental->use_row[0]=
       pEngineInstrumental->use_row[1]=
       pEngineInstrumental->use_row[2]=
       pEngineInstrumental->use_row[3]=true;

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :                                                               // CCD EEV 1340x400

      NDET[0]=(pMediateInstrumental->ccdeev.detectorSize)?pMediateInstrumental->ccdeev.detectorSize:1340;
      pEngineInstrumental->user=pMediateInstrumental->ccdeev.spectralType;                           // spectrum type (offaxis or zenith)

      pEngineInstrumental->offsetFlag=pMediateInstrumental->ccdeev.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->ccdeev.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->ccdeev.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->ccdeev.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->ccdeev.transmissionFunctionFile);     // instrumental function file
      strcpy(pEngineInstrumental->imagePath,pMediateInstrumental->ccdeev.imagePath);     // instrumental function file

      // ---> not used for the moment : pMediateInstrumental->ccdeev.straylightCorrectionFile;
      // ---> not used for the moment : pMediateInstrumental->ccdeev.detectorNonLinearityFile;

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GOME2 :                            // GOME2

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gome2.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gome2.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->user=pMediateInstrumental->gome2.bandType;

      NDET[0]=1024;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :                             // University of Toronto

      pEngineInstrumental->offsetFlag=pMediateInstrumental->uoft.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->uoft.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->uoft.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->uoft.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->uoft.transmissionFunctionFile);     // instrumental function file

      NDET[0]=2048;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :                              // MFC Heidelberg

      NDET[0]=pMediateInstrumental->mfc.detectorSize;

      pEngineInstrumental->offsetFlag=pMediateInstrumental->mfc.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mfc.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mfc.lambdaMax;

      pEngineInstrumental->mfc.mfcRevert=pMediateInstrumental->mfc.revert;

      pEngineInstrumental->mfc.mfcMaskOffset=pMediateInstrumental->mfc.offsetMask;
      pEngineInstrumental->mfc.mfcMaskInstr=pMediateInstrumental->mfc.instrFctnMask;
      pEngineInstrumental->mfc.mfcMaskSpec=pMediateInstrumental->mfc.spectraMask;
      pEngineInstrumental->mfc.mfcMaskDark=pMediateInstrumental->mfc.darkCurrentMask;

      pEngineInstrumental->mfc.wavelength=pMediateInstrumental->mfc.firstWavelength;
      pEngineInstrumental->mfc.mfcMaskUse=pMediateInstrumental->mfc.autoFileSelect;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mfc.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mfc.transmissionFunctionFile);     // instrumental function file
      strcpy(pEngineInstrumental->vipFile,pMediateInstrumental->mfc.darkCurrentFile);             // dark current file
      strcpy(pEngineInstrumental->offsetFile,pMediateInstrumental->mfc.offsetFile);               // offset file

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_STD :                          // MFC Heidelberg (ASCII)

      NDET[0]=pMediateInstrumental->mfcstd.detectorSize;

      pEngineInstrumental->mfc.mfcRevert=pMediateInstrumental->mfcstd.revert;
      pEngineInstrumental->offsetFlag=pMediateInstrumental->mfcstd.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mfcstd.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mfcstd.lambdaMax;

      strcpy(pEngineInstrumental->mfc.mfcStdDate,pMediateInstrumental->mfcstd.dateFormat);

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mfcstd.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mfcstd.transmissionFunctionFile);     // instrumental function file
      strcpy(pEngineInstrumental->vipFile,pMediateInstrumental->mfcstd.darkCurrentFile);             // dark current file
      strcpy(pEngineInstrumental->offsetFile,pMediateInstrumental->mfcstd.offsetFile);               // offset file

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_BIRA :                          // MFC BIRA (binary)

      pEngineInstrumental->offsetFlag=pMediateInstrumental->mfcbira.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mfcbira.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mfcbira.lambdaMax;

      NDET[0]=pMediateInstrumental->mfcbira.detectorSize;
      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mfcbira.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mfcbira.transmissionFunctionFile);       // instrumental function file

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MKZY :                                                                  // MKZY

      NDET[0]=2048;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->mkzy.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mkzy.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mkzy.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mkzy.calibrationFile);       // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mkzy.transmissionFunctionFile);       // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMI :

      for(unsigned int i=0; i<MAX_SWATHSIZE; ++i) {
        NDET[i]=1024;
        pEngineInstrumental->use_row[i]=false;
      }

      pEngineInstrumental->omi.spectralType=pMediateInstrumental->omi.spectralType;
      pEngineInstrumental->omi.averageFlag=pMediateInstrumental->omi.flagAverage;
      pEngineInstrumental->omi.pixelQFRejectionFlag=pMediateInstrumental->omi.pixelQFRejectionFlag;
      pEngineInstrumental->omi.pixelQFMaxGaps=pMediateInstrumental->omi.pixelQFMaxGaps;
      pEngineInstrumental->omi.pixelQFMask=pMediateInstrumental->omi.pixelQFMask;
      pEngineInstrumental->omi.xtrack_mode=pMediateInstrumental->omi.xtrack_mode;

      OMI_TrackSelection(pMediateInstrumental->omi.trackSelection,pEngineInstrumental->use_row);

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMIV4:

      for(unsigned int i=0; i<60; ++i) {  // TODO UV1 band (sometimes?) has only 30 rows
        NDET[i]=1024;
        pEngineInstrumental->use_row[i]=true;
      }
      for(unsigned int i=60; i<MAX_SWATHSIZE; ++i) {
        NDET[i]=0;
        pEngineInstrumental->use_row[i]=false;
      }
      pEngineInstrumental->omi.spectralType=pMediateInstrumental->omi.spectralType;
      pEngineInstrumental->omi.xtrack_mode=pMediateInstrumental->omi.xtrack_mode;
      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_TROPOMI:

      for (int i=0; i<MAX_SWATHSIZE; ++i) {
        NDET[i]=1024;
        pEngineInstrumental->use_row[i]=false;
      }

      pEngineInstrumental->tropomi.spectralBand = pMediateInstrumental->tropomi.spectralBand;
      strcpy(pEngineInstrumental->tropomi.reference_orbit_dir, pMediateInstrumental->tropomi.reference_orbit_dir);

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->tropomi.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->tropomi.instrFunctionFile);     // instrumental function file

      OMI_TrackSelection(pMediateInstrumental->tropomi.trackSelection,pEngineInstrumental->use_row);

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMPS:

      for (int i=0; i<MAX_SWATHSIZE; ++i) {
        NDET[i]=512;
      }

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF:

      NDET[0]=pMediateInstrumental->frm4doas.detectorSize;
      pEngineInstrumental->user=pMediateInstrumental->frm4doas.spectralType;

      pEngineInstrumental->frm4doas.averageRows=pMediateInstrumental->frm4doas.averageRows;

      pEngineInstrumental->offsetFlag=pMediateInstrumental->frm4doas.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->frm4doas.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->frm4doas.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->frm4doas.calibrationFile); // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->frm4doas.transmissionFunctionFile); // instrumental function file

      break;      
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GEMS :

      for (int i=0; i<MAX_SWATHSIZE; ++i)
       {
        NDET[i] = GEMS_INIT_LENGTH;
        pEngineInstrumental->use_row[i]=false;
       }                                                                                    // Could be reduced by Set function
      OMI_TrackSelection(pMediateInstrumental->gems.trackSelection,pEngineInstrumental->use_row);

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gems.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gems.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->gems.binning=pMediateInstrumental->gems.binning;

      break;
      // ----------------------------------------------------------------------------
    }
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSlit
// -----------------------------------------------------------------------------
// PURPOSE       Slit part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSlit(PRJCT_SLIT *pEngineSlit,const mediate_project_slit_t *pMediateSlit)
 {
   // Fields

   pEngineSlit->fwhmCorrectionFlag=pMediateSlit->applyFwhmCorrection;
   strcpy(pEngineSlit->kuruczFile,pMediateSlit->solarRefFile);

   setMediateSlit(&pEngineSlit->slitFunction,&pMediateSlit->function);
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectOutput
// -----------------------------------------------------------------------------
// PURPOSE       Output (binary/ascii format) part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectOutput(PRJCT_RESULTS *pEngineOutput, const char *project_name, const mediate_project_output_t *pMediateOutput)
 {
   // Declarations

   strcpy(pEngineOutput->path,pMediateOutput->path);                            // path for results and fits files
   strcpy(pEngineOutput->newCalibPath,pMediateOutput->newCalibPath);            // path for calibrated irradiances (run calib for satellites)
   strcpy(pEngineOutput->fluxes,pMediateOutput->flux);                          // fluxes

   pEngineOutput->bandWidth=pMediateOutput->bandWidth;

   // use configured netCDF group name if provided, project name otherwise.
   if (pMediateOutput->swath_name[0] != '\0') {
     strcpy(pEngineOutput->swath_name, pMediateOutput->swath_name);
   } else {
     strcpy(pEngineOutput->swath_name, project_name);
   }

   pEngineOutput->file_format=pMediateOutput->file_format;

   pEngineOutput->analysisFlag=pMediateOutput->analysisFlag;
   pEngineOutput->calibFlag=pMediateOutput->calibrationFlag;
   pEngineOutput->newcalibFlag=pMediateOutput->newcalibFlag;
   pEngineOutput->referenceFlag=pMediateOutput->referenceFlag;
   pEngineOutput->dirFlag=pMediateOutput->directoryFlag;
   pEngineOutput->fileNameFlag=pMediateOutput->filenameFlag;
   pEngineOutput->successFlag=pMediateOutput->successFlag;

   if (!(pEngineOutput->fieldsNumber=pMediateOutput->selection.nSelected))
    memset(pEngineOutput->fieldsFlag,0,PRJCT_RESULTS_MAX*sizeof(int));
   else
    memcpy(pEngineOutput->fieldsFlag,pMediateOutput->selection.selected,pEngineOutput->fieldsNumber*sizeof(int));
 }

void setMediateProjectExport(PRJCT_EXPORT *pEngineExport,const mediate_project_export_t *pMediateExport)
 {
     strcpy(pEngineExport->path,pMediateExport->path);

     pEngineExport->titlesFlag=pMediateExport->titlesFlag;
     pEngineExport->directoryFlag=pMediateExport->directoryFlag;

  if (!(pEngineExport->fieldsNumber=pMediateExport->selection.nSelected))
   memset(pEngineExport->fieldsFlag,0,PRJCT_RESULTS_MAX*sizeof(int));
  else
   memcpy(pEngineExport->fieldsFlag,pMediateExport->selection.selected,pEngineExport->fieldsNumber*sizeof(int));
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetProject
// -----------------------------------------------------------------------------
// PURPOSE       Interface between the mediator and the engine for project properties
//
//               Project will be defined by the GUI. The engine is responsible for copying
//               any required data from project. The project details will remain valid
//               for the engine until the next call to mediateRequestSetProject.
//               project may be the null pointer, in which case the engine may free any
//               resources DIRECTLY associated with the project.
//
//               The operatingMode indicates the intended usage (Browsing, Analysis or Calibration).
//               It is a workaround that may ultimately be removed.
//
// RETURN        Zero is returned if the operation succeeded, -1 otherwise.
//               On success, project becomes the 'current project'.
// -----------------------------------------------------------------------------

int mediateRequestSetProject(void *engineContext,
                 const mediate_project_t *project,
                 int operatingMode,
                 void *responseHandle)
 {
   // Initializations

   ENGINE_CONTEXT *pEngineContext=(ENGINE_CONTEXT *)engineContext;
   PROJECT * pEngineProject= &pEngineContext->project;

   RC rc=ERROR_ID_NO;

   // Release buffers allocated at the previous session

   EngineEndCurrentSession(pEngineContext);

   THRD_id=operatingMode;
   ANALYSE_swathSize=1;

   // Transfer projects options from the mediator to the engine

   strncpy(pEngineProject->project_name, project->project_name, PROJECT_NAME_BUFFER_LENGTH-1);
   for(unsigned int i=0; i<MAX_SWATHSIZE; ++i)
     pEngineContext->project.instrumental.use_row[i]=true;

   setMediateProjectDisplay(&pEngineProject->spectra,&project->display);
   setMediateProjectSelection(&pEngineProject->spectra,&project->selection);
   setMediateProjectAnalysis(&pEngineProject->analysis,&project->analysis);
   setMediateFilter(&pEngineProject->lfilter,&project->lowpass,0,0);
   setMediateFilter(&pEngineProject->hfilter,&project->highpass,1,0);
   setMediateProjectCalibration(&pEngineProject->kurucz,&pEngineContext->calibFeno,&project->calibration,pEngineContext->project.spectra.displayCalibFlag);
   setMediateProjectInstrumental(&pEngineProject->instrumental,&project->instrumental);
   setMediateProjectUndersampling(&pEngineProject->usamp,&project->undersampling);
   setMediateProjectSlit(&pEngineProject->slit,&project->slit);
   setMediateProjectOutput(&pEngineProject->asciiResults, project->project_name, &project->output);
   setMediateProjectExport(&pEngineProject->exportSpectra,&project->export_spectra);

   if (is_maxdoas(pEngineProject->instrumental.readOutFormat))
    {
     int fieldsFlag[PRJCT_RESULTS_MAX];

     memset(fieldsFlag,0,PRJCT_RESULTS_MAX*sizeof(int));

     // fieldsFlag not processed the same time in display page and output/export ??? -> to check later

     if (operatingMode==THREAD_TYPE_ANALYSIS)
      for (int i=0;i<project->output.selection.nSelected;i++)
       fieldsFlag[project->output.selection.selected[i]]=1;
     else if (operatingMode==THREAD_TYPE_EXPORT)
      for (int i=0;i<project->export_spectra.selection.nSelected;i++)
       fieldsFlag[project->export_spectra.selection.selected[i]]=1;
    }

   // Allocate buffers requested by the project

   if (EngineSetProject(pEngineContext)!=ERROR_ID_NO)
    rc=ERROR_DisplayMessage(responseHandle);
   else if ((THRD_id==THREAD_TYPE_EXPORT) && !(rc=OUTPUT_CheckPath(pEngineContext,pEngineContext->project.exportSpectra.path,ASCII)))
    rc=OUTPUT_RegisterSpectra(pEngineContext);

   return (rc!=ERROR_ID_NO)?-1:0;    // supposed that an error at the level of the load of projects stops the current session
 }

// =======================================================================
// TRANSFER OF ANALYSIS WINDOWS PROPERTIES FROM THE MEDIATOR TO THE ENGINE
// =======================================================================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisLinear
// -----------------------------------------------------------------------------
// PURPOSE       Load linear parameters
// -----------------------------------------------------------------------------

RC mediateRequestSetAnalysisLinear(const struct anlyswin_linear *pLinear,INDEX indexFenoColumn)
 {
   // Declarations

   ANALYSE_LINEAR_PARAMETERS linear[2];
   RC rc;

   // Polynomial (x)

   strcpy(linear[0].symbolName,"Polynomial (x)");

   linear[0].polyOrder=pLinear->xPolyOrder-1;
   linear[0].baseOrder=pLinear->xBaseOrder-1;
   linear[0].storeFit=pLinear->xFlagFitStore;
   linear[0].storeError=pLinear->xFlagErrStore;

   // Linear offset

   strcpy(linear[1].symbolName, pLinear->offsetI0 ? "Offset (ref)" : "Offset (rad)");

   linear[1].polyOrder=pLinear->offsetPolyOrder-1;
   linear[1].baseOrder= -1;
   linear[1].storeFit=pLinear->offsetFlagFitStore;
   linear[1].storeError=pLinear->offsetFlagErrStore;

   rc=ANALYSE_LoadLinear(linear, 2, indexFenoColumn);

   // Return

   return rc;
 }

// Caro : in the future, replace structures anlyswin_nonlinear and calibration_sfp with the following one more flexible

// typedef struct _AnalyseNonLinearParameters
//  {
//      char symbolName[MAX_ITEM_TEXT_LEN];
//      char crossFileName[MAX_ITEM_TEXT_LEN];
//      int fitFlag;
//      double initialValue;
//      double deltaValue;
//      double minValue;
//      double maxValue;
//      int storeFit;
//      int storeError;
//  }
// ANALYSE_NON_LINEAR_PARAMETERS;

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisNonLinearCalib
// -----------------------------------------------------------------------------
// PURPOSE       Load non linear parameters for the calibration
// -----------------------------------------------------------------------------

RC mediateRequestSetAnalysisNonLinearCalib(ENGINE_CONTEXT *pEngineContext,struct calibration_sfp *nonLinearCalib,double *lambda,INDEX indexFenoColumn)
 {
   // Declarations

   ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NSFP];
   INDEX indexNonLinear;
   RC rc;

   // Initialization

   memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NSFP);

   for (indexNonLinear=0;indexNonLinear<NSFP;indexNonLinear++)
    {
     nonLinear[indexNonLinear].minValue=nonLinear[indexNonLinear].maxValue=(double)0.;
     sprintf(nonLinear[indexNonLinear].symbolName,"SFP %d",indexNonLinear+1);

     nonLinear[indexNonLinear].fitFlag=nonLinearCalib[indexNonLinear].fitFlag;
     nonLinear[indexNonLinear].initialValue=nonLinearCalib[indexNonLinear].initialValue;
     nonLinear[indexNonLinear].deltaValue=nonLinearCalib[indexNonLinear].deltaValue;
     nonLinear[indexNonLinear].storeFit=nonLinearCalib[indexNonLinear].fitStore;
     nonLinear[indexNonLinear].storeError=nonLinearCalib[indexNonLinear].errStore;
    }

   rc=ANALYSE_LoadNonLinear(pEngineContext,nonLinear,NSFP,lambda,indexFenoColumn );

   // Return

   return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisNonLinearDoas
// -----------------------------------------------------------------------------
// PURPOSE       Load non linear parameters
// -----------------------------------------------------------------------------

#define NNONLINEAR_DOAS 8

RC mediateRequestSetAnalysisNonLinearDoas(ENGINE_CONTEXT *pEngineContext, const struct anlyswin_nonlinear *pNonLinear,double *lambda, INDEX indexFenoColumn)
 {
   // Declarations

   ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NNONLINEAR_DOAS];
   INDEX indexNonLinear;
   RC rc;

   // Initialization

   memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NNONLINEAR_DOAS);                 // this reset cross section file names
   // QDOAS MISSING FIELD !!!
   for (indexNonLinear=0;indexNonLinear<NNONLINEAR_DOAS;indexNonLinear++)
    nonLinear[indexNonLinear].minValue=nonLinear[indexNonLinear].maxValue=(double)0.;

   // Sol

   strcpy(nonLinear[0].symbolName,"Sol");

   nonLinear[0].fitFlag=pNonLinear->solFlagFit;
   nonLinear[0].initialValue=pNonLinear->solInitial;
   nonLinear[0].deltaValue=pNonLinear->solDelta;
   nonLinear[0].storeFit=pNonLinear->solFlagFitStore;
   nonLinear[0].storeError=pNonLinear->solFlagErrStore;

   // Offset (Constant)

   strcpy(nonLinear[1].symbolName,"Offset (Constant)");

   nonLinear[1].fitFlag=pNonLinear->off0FlagFit;
   nonLinear[1].initialValue=pNonLinear->off0Initial;
   nonLinear[1].deltaValue=pNonLinear->off0Delta;
   nonLinear[1].storeFit=pNonLinear->off0FlagFitStore;
   nonLinear[1].storeError=pNonLinear->off0FlagErrStore;

   // Offset (Order 1)

   strcpy(nonLinear[2].symbolName,"Offset (Order 1)");

   nonLinear[2].fitFlag=pNonLinear->off1FlagFit;
   nonLinear[2].initialValue=pNonLinear->off1Initial;
   nonLinear[2].deltaValue=pNonLinear->off1Delta;
   nonLinear[2].storeFit=pNonLinear->off1FlagFitStore;
   nonLinear[2].storeError=pNonLinear->off1FlagErrStore;

   // Offset (Order 2)

   strcpy(nonLinear[3].symbolName,"Offset (Order 2)");

   nonLinear[3].fitFlag=pNonLinear->off2FlagFit;
   nonLinear[3].initialValue=pNonLinear->off2Initial;
   nonLinear[3].deltaValue=pNonLinear->off2Delta;
   nonLinear[3].storeFit=pNonLinear->off2FlagFitStore;
   nonLinear[3].storeError=pNonLinear->off2FlagErrStore;

   // Com

   strcpy(nonLinear[4].symbolName,"Com");
   strcpy(nonLinear[4].crossFileName,pNonLinear->comFile);

   nonLinear[4].fitFlag=pNonLinear->comFlagFit;
   nonLinear[4].initialValue=pNonLinear->comInitial;
   nonLinear[4].deltaValue=pNonLinear->comDelta;
   nonLinear[4].storeFit=pNonLinear->comFlagFitStore;
   nonLinear[4].storeError=pNonLinear->comFlagErrStore;

   // Usamp1

   strcpy(nonLinear[5].symbolName,"Usamp1");
   strcpy(nonLinear[5].crossFileName,pNonLinear->usamp1File);

   nonLinear[5].fitFlag=pNonLinear->usamp1FlagFit;
   nonLinear[5].initialValue=pNonLinear->usamp1Initial;
   nonLinear[5].deltaValue=pNonLinear->usamp1Delta;
   nonLinear[5].storeFit=pNonLinear->usamp1FlagFitStore;
   nonLinear[5].storeError=pNonLinear->usamp1FlagErrStore;

   // Usamp2

   strcpy(nonLinear[6].symbolName,"Usamp2");
   strcpy(nonLinear[6].crossFileName,pNonLinear->usamp2File);

   nonLinear[6].fitFlag=pNonLinear->usamp2FlagFit;
   nonLinear[6].initialValue=pNonLinear->usamp2Initial;
   nonLinear[6].deltaValue=pNonLinear->usamp2Delta;
   nonLinear[6].storeFit=pNonLinear->usamp2FlagFitStore;
   nonLinear[6].storeError=pNonLinear->usamp2FlagErrStore;

   // Resol

   strcpy(nonLinear[7].symbolName,"Resol");

   nonLinear[7].fitFlag=pNonLinear->resolFlagFit;
   nonLinear[7].initialValue=pNonLinear->resolInitial;
   nonLinear[7].deltaValue=pNonLinear->resolDelta;
   nonLinear[7].storeFit=pNonLinear->resolFlagFitStore;
   nonLinear[7].storeError=pNonLinear->resolFlagErrStore;

   rc=ANALYSE_LoadNonLinear(pEngineContext,nonLinear,NNONLINEAR_DOAS,lambda,indexFenoColumn);

   // Return

   return rc;
 }

int mediateRequestSetAnalysisWindows(void *engineContext,
                     int numberOfWindows,
                     const mediate_analysis_window_t *analysisWindows,
                     int operatingMode,
                     void *responseHandle)
 {
   // Declarations

   double lambdaMin,lambdaMax;
   int useKurucz,                                                               // flag set if Kurucz is to be used in at least one analysis window
       useUsamp,                                                                // flag set if undersampling correction is requested in at least one analysis window
       xsToConvolute,                                                           // flag set if at least one cross section has to be convolved in at least one analysis window
       xsToConvoluteI0,                                                         // flag set if at least one cross section has to be I0-convolved in at least one analysis window
       saveFlag;
   INDEX indexKurucz,indexWindow;
   ENGINE_CONTEXT *pEngineContext;                                               // engine context
   PRJCT_INSTRUMENTAL *pInstrumental;
   const mediate_analysis_window_t *pAnalysisWindows;                            // pointer to the current analysis window from the user interface
   mediate_analysis_window_t calibWindows;                                       // pointer to the calibration parameters
   FENO *pTabFeno;                                                               // pointer to the description of an analysis window
   int indexFeno,indexFenoColumn;                                                // browse analysis windows
   int n_wavel_temp1, n_wavel_temp2;                                             // temporary spectral channel
   MATRIX_OBJECT hr_solar_temp,  slit_matrix_temp;                               // to preload high res solar spectrum and slit function matrix
   RC rc;                                                                        // return code

   // Initializations

   #if defined(__DEBUG_) && __DEBUG_
   DEBUG_Start(ENGINE_dbgFile, __func__, DEBUG_FCTTYPE_MEM, 15, DEBUG_DVAR_YES, 0);
   #endif

   lambdaMin=1000;
   lambdaMax=0;
   pEngineContext=(ENGINE_CONTEXT *)engineContext;
   pInstrumental=&pEngineContext->project.instrumental;
   saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
   useKurucz=useUsamp=xsToConvolute=xsToConvoluteI0=0;
   indexKurucz=ITEM_NONE;

   // for imagers, it is possible that errors occur for only some of
   // the rows.  In that case, analysis can continue for the other
   // rows, but we still want to display a warning message.
   bool imager_err = false;

   memset(&calibWindows,0,sizeof(mediate_analysis_window_t));
   memset(&hr_solar_temp, 0, sizeof(hr_solar_temp));
   memset(&slit_matrix_temp, 0, sizeof(slit_matrix_temp));

   memcpy(&calibWindows.crossSectionList,&pEngineContext->calibFeno.crossSectionList,sizeof(cross_section_list_t));
   memcpy(&calibWindows.linear,&pEngineContext->calibFeno.linear,sizeof(struct anlyswin_linear));
   memcpy(&calibWindows.shiftStretchList,&pEngineContext->calibFeno.shiftStretchList,sizeof(shift_stretch_list_t));
   memcpy(&calibWindows.outputList,&pEngineContext->calibFeno.outputList,sizeof(output_list_t));

   // Reinitialize all global variables used for the analysis, release old buffers and allocate new ones

   KURUCZ_indexLine=1;
   rc=ANALYSE_SetInit(pEngineContext);

   if (numberOfWindows == 0) { // We need at least one active analysis window.
     rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NOANALYSIS);
     goto handle_errors;
   }

   n_wavel_temp1 = 0;
   n_wavel_temp2 = 0;
   // if the user wants to write output to a file, check if the path is valid before starting analysis
   if ( (THRD_id==THREAD_TYPE_ANALYSIS && pEngineContext->project.asciiResults.analysisFlag) ||
        (THRD_id==THREAD_TYPE_KURUCZ && pEngineContext->project.asciiResults.calibFlag) ) {
     rc = OUTPUT_CheckPath(pEngineContext,pEngineContext->project.asciiResults.path,pEngineContext->project.asciiResults.file_format);
     if (rc != ERROR_ID_NO)
       goto handle_errors;
   }

   switch(pInstrumental->readOutFormat) {
   case PRJCT_INSTR_FORMAT_OMI:
     ANALYSE_swathSize = 60;
     break;
   case PRJCT_INSTR_FORMAT_OMIV4:
     rc = OMIV4_init_irradiances(analysisWindows, numberOfWindows, pEngineContext);
     break;
   case PRJCT_INSTR_FORMAT_GOME1_NETCDF:
     ANALYSE_swathSize = 4;   // the number of pixel types
     if (strlen(analysisWindows[0].refOneFile))
       rc = GOME1NETCDF_InitRef(analysisWindows[0].refOneFile,&n_wavel_temp1,pEngineContext);
     if (strlen(analysisWindows[0].refTwoFile))
       rc = GOME1NETCDF_InitRef(analysisWindows[0].refTwoFile,&n_wavel_temp2,pEngineContext);
     break;
   case PRJCT_INSTR_FORMAT_TROPOMI:
     rc = tropomi_init_irradiances(analysisWindows, numberOfWindows,
                                  pEngineContext->project.instrumental.tropomi.spectralBand,
                                  &n_wavel_temp1);
     if (!rc) {
       rc = tropomi_init_radref(analysisWindows, numberOfWindows, &n_wavel_temp2);
     }
     break;
   case PRJCT_INSTR_FORMAT_OMPS:
     ANALYSE_swathSize = 36;
     for (int i =0; i< ANALYSE_swathSize; ++i)
       pInstrumental->use_row[i] = true;
     break;
   case PRJCT_INSTR_FORMAT_GEMS:
     if (strlen(analysisWindows[0].refOneFile)) {
       rc = GEMS_init_irradiance(analysisWindows[0].refOneFile, pEngineContext->buffers.lambda, pEngineContext->buffers.spectrum, &n_wavel_temp1);
     }
     if (strlen(analysisWindows[0].refTwoFile)) {
       rc = GEMS_init_radref(analysisWindows[0].refTwoFile, &n_wavel_temp2);
     }
     break;
     // TO SEE LATER WHAT IS NECESSARY FOR THIS FORMAT rc = gems_init(analysisWindows[0].refOneFile,pEngineContext);              // !!! GEMS : if fixed format, just initialize the ANALYSE_swathSize
     break;
   case PRJCT_INSTR_FORMAT_APEX:
     // TODO: generalize for different analysis windows APEX
     rc = apex_init(analysisWindows[0].refOneFile,pEngineContext);
     break;
   default:
     // for all non-imager instruments
     pInstrumental->use_row[0]=true;
   }
   if (ANALYSE_swathSize > MAX_SWATHSIZE) {
     rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_SWATHSIZE, ANALYSE_swathSize, MAX_SWATHSIZE);
     goto handle_errors;
   }

   // Load analysis windows

   for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++) {

     // check if we have to skip this column
     if (!pEngineContext->project.instrumental.use_row[indexFenoColumn])
       continue;

     NFeno=0;

     for (indexFeno=0;(indexFeno<numberOfWindows+1) && !rc;indexFeno++) {
       // Pointers initialization

       pTabFeno=&TabFeno[indexFenoColumn][NFeno];

       pTabFeno->hidden=!indexFeno;
       pAnalysisWindows=(!pTabFeno->hidden)? &analysisWindows[indexFeno-1]: &calibWindows;

       pTabFeno->NDET=NDET[indexFenoColumn];
       pTabFeno->n_wavel_ref1=n_wavel_temp1;
       pTabFeno->n_wavel_ref2=n_wavel_temp2;
       const int n_wavel = pTabFeno->NDET;

       if ((pTabFeno->hidden<2) && ((THRD_id==THREAD_TYPE_ANALYSIS) || (pTabFeno->hidden==1))) {
         // QDOAS : avoid the load of disabled analysis windows with hidden==2

         if (pTabFeno->hidden) {                                                     // if indexFeno==0, load calibration parameters
           strcpy(pTabFeno->windowName,"Calibration description");                   // like WinDOAS
           pTabFeno->analysisMethod=pKuruczOptions->analysisMethod;
         } else {                                                                    // otherwise, load analysis windows from analysisWindows[indexFeno-1]
           // Load data from analysis windows panels

           strcpy(pTabFeno->windowName,pAnalysisWindows->name);
           strcpy(pTabFeno->residualsFile,pAnalysisWindows->residualFile);
           strcpy(pTabFeno->ref1,pAnalysisWindows->refOneFile);
           strcpy(pTabFeno->ref2,pAnalysisWindows->refTwoFile);

           pTabFeno->resolFwhm=pAnalysisWindows->resolFwhm;
           pTabFeno->lambda0=pAnalysisWindows->lambda0;

           pTabFeno->saveResidualsFlag=(!pTabFeno->hidden)?pAnalysisWindows->saveResidualsFlag:0;

           if (fabs(pTabFeno->lambda0)<EPSILON)
            pTabFeno->lambda0=(double)0.5*(pAnalysisWindows->fitMinWavelength+pAnalysisWindows->fitMaxWavelength);

           if ((pTabFeno->refSpectrumSelectionMode=pAnalysisWindows->refSpectrumSelection)==ANLYS_REF_SELECTION_MODE_AUTOMATIC) {
               if (((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_ASCII) && is_maxdoas(pEngineContext->project.instrumental.readOutFormat)) ||
                   ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) && (pEngineContext->project.instrumental.ascii.elevSaveFlag || (pEngineContext->project.instrumental.ascii.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED)))) {

             //if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) || (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) || (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) || (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA) ||
             //    (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_BIRA_MOBILE) || (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_BIRA_AIRBORNE) || (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF) ||
             //   ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) && (pEngineContext->project.instrumental.ascii.elevSaveFlag || (pEngineContext->project.instrumental.ascii.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED)))) {

               if ((pTabFeno->refMaxdoasSelectionMode=pAnalysisWindows->refMaxdoasSelection)==ANLYS_MAXDOAS_REF_SCAN)
                 pEngineContext->analysisRef.refScan++;
               else if (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA)
                 pEngineContext->analysisRef.refSza++;
             }

             pTabFeno->refSpectrumSelectionScanMode=pAnalysisWindows->refSpectrumSelectionScanMode;

             pTabFeno->refSZA=(double)pAnalysisWindows->refSzaCenter;
             pTabFeno->refSZADelta=(double)pAnalysisWindows->refSzaDelta;

             pTabFeno->refLatMin=pAnalysisWindows->refMinLatitude;
             pTabFeno->refLatMax=pAnalysisWindows->refMaxLatitude;
             pTabFeno->refLonMin=pAnalysisWindows->refMinLongitude;
             pTabFeno->refLonMax=pAnalysisWindows->refMaxLongitude;

             pTabFeno->cloudFractionMin=pAnalysisWindows->cloudFractionMin;
             pTabFeno->cloudFractionMax=pAnalysisWindows->cloudFractionMax;

             pEngineContext->analysisRef.refAuto++;

             if ((fabs(pTabFeno->refLonMax-pTabFeno->refLonMin)>1.e-5) ) // && (fabs(pTabFeno->refLonMax-pTabFeno->refLonMin)<359.))
               pEngineContext->analysisRef.refLon++;
           }

           if (pEngineContext->project.spectra.displayFitFlag) {

             pTabFeno->displaySpectrum=pAnalysisWindows->requireSpectrum;
             pTabFeno->displayResidue=pAnalysisWindows->requireResidual;
             pTabFeno->displayTrend=pAnalysisWindows->requirePolynomial;
             pTabFeno->displayRefEtalon=pAnalysisWindows->requireRefRatio;
             pTabFeno->displayFits=pAnalysisWindows->requireFit;
             pTabFeno->displayPredefined=pAnalysisWindows->requirePredefined;

             pTabFeno->displayFlag=pTabFeno->displaySpectrum+
               pTabFeno->displayResidue+
               pTabFeno->displayTrend+
               pTabFeno->displayRefEtalon+
               pTabFeno->displayFits+
               pTabFeno->displayPredefined;
           }

           pTabFeno->useKurucz=pAnalysisWindows->kuruczMode;

           pTabFeno->analysisMethod=pAnalysisOptions->method;
           useKurucz+=pAnalysisWindows->kuruczMode;
         }  // if (pTabFeno->hidden)

         pTabFeno->Decomp=1;

         // spikes buffer
         if ((pTabFeno->spikes == NULL) &&
             ((pTabFeno->spikes=(bool *)MEMORY_AllocBuffer(__func__,"spikes",n_wavel,sizeof(int),0,MEMORY_TYPE_INT))==NULL)) {
           rc = ERROR_ID_ALLOC;
           break;
         }

         // Wavelength scales read out

         if (((pTabFeno->Lambda==NULL) && ((pTabFeno->Lambda=MEMORY_AllocDVector(__func__,"Lambda",0,n_wavel-1))==NULL)) ||
             ((pTabFeno->LambdaK==NULL) && ((pTabFeno->LambdaK=MEMORY_AllocDVector(__func__,"LambdaK",0,n_wavel-1))==NULL)) ||
             ((pTabFeno->LambdaRef==NULL) && ((pTabFeno->LambdaRef=MEMORY_AllocDVector(__func__,"LambdaRef",0,n_wavel-1))==NULL)) ||

             // omi rejected pixels

             ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) && pEngineContext->project.instrumental.omi.pixelQFRejectionFlag &&
              (pTabFeno->omiRejPixelsQF == NULL) && ((pTabFeno->omiRejPixelsQF=(bool *)MEMORY_AllocBuffer(__func__,"omiRejPixelsQF",n_wavel,sizeof(int),0,MEMORY_TYPE_INT))==NULL))) {
           rc=ERROR_ID_ALLOC;
           break;
         }

         if ( pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI
              && strlen(pInstrumental->calibrationFile) ) {
           int n_wavel_ref;
           rc=OMI_GetReference(pInstrumental->omi.spectralType,pInstrumental->calibrationFile,indexFenoColumn,pEngineContext->buffers.lambda,pEngineContext->buffers.spectrum,pEngineContext->buffers.sigmaSpec, &n_wavel_ref);

           if (rc != 0) {
             break;
           }

         }

         memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda,sizeof(double)*n_wavel);
         memcpy(pTabFeno->Lambda,pEngineContext->buffers.lambda,sizeof(double)*n_wavel);

         // TODO: ANALYSE_LoadRef can change NDET[] -> should n_wavel be updated here?
         if (!(rc=ANALYSE_LoadRef(pEngineContext,indexFenoColumn)) &&   // eventually, modify LambdaRef for continuous functions
             !(rc=ANALYSE_LoadCross(pEngineContext,pAnalysisWindows->crossSectionList.crossSection,pAnalysisWindows->crossSectionList.nCrossSection,pTabFeno->LambdaRef,indexFenoColumn)) &&
             !(rc=mediateRequestSetAnalysisLinear(&pAnalysisWindows->linear,indexFenoColumn)) &&

             // Caro : int the future, replace structures anlyswin_nonlinear and calibration_sfp with the following one more flexible
             //        mediateRequestSetAnalysisNonLinearDoas and mediateRequestSetAnalysisNonLinearCalib would be replaced by only one call to ANALYSE_LoadNonLinear

             ((!pTabFeno->hidden && !(rc=mediateRequestSetAnalysisNonLinearDoas(pEngineContext,&pAnalysisWindows->nonlinear,pTabFeno->LambdaRef,indexFenoColumn))) ||
              (pTabFeno->hidden && !(rc=mediateRequestSetAnalysisNonLinearCalib(pEngineContext,pEngineContext->calibFeno.sfp,pTabFeno->LambdaRef,indexFenoColumn)))) &&

             !(rc=ANALYSE_LoadShiftStretch(pAnalysisWindows->shiftStretchList.shiftStretch,pAnalysisWindows->shiftStretchList.nShiftStretch,indexFenoColumn)) &&
             !(rc=ANALYSE_LoadOutput(pAnalysisWindows->outputList.output,pAnalysisWindows->outputList.nOutput,indexFenoColumn)) &&
             (pTabFeno->hidden ||
              (!(rc=ANALYSE_LoadGaps(pEngineContext,pAnalysisWindows->gapList.gap,pAnalysisWindows->gapList.nGap,pTabFeno->LambdaRef,pAnalysisWindows->fitMinWavelength,pAnalysisWindows->fitMaxWavelength,indexFenoColumn)) &&

               (!pTabFeno->gomeRefFlag || !(rc=FIT_PROPERTIES_alloc(__func__,&pTabFeno->fit_properties)))
               ))) {
           if (pTabFeno->hidden==1) {
             indexKurucz=NFeno;
           } else {
             useUsamp+=pTabFeno->useUsamp;
             xsToConvolute+=pTabFeno->xsToConvolute;
             xsToConvoluteI0+=pTabFeno->xsToConvoluteI0;

             if (pTabFeno->gomeRefFlag || pEngineContext->refFlag) {
               memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*n_wavel);
               memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*n_wavel);

               rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,indexFenoColumn);
             }
           }

           ANALYSE_SetAnalysisType(indexFenoColumn);
           if (!pTabFeno->hidden) {
             lambdaMin=min(lambdaMin,pTabFeno->LambdaRef[0]);
             lambdaMax=max(lambdaMax,pTabFeno->LambdaRef[n_wavel-1]);
           }

           NFeno++;
         }
       } // if ((pTabFeno->hidden<2) && ((THRD_id==THREAD_TYPE_ANALYSIS) || (pTabFeno->hidden==1)))
     }  // for (indexFeno=0;(indexFeno<numberOfWindows+1) && !rc;indexFeno++)
   } // for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)

   if (rc)
     goto handle_errors;

   int max_ndet = 0;
   for (int i=0; i<ANALYSE_swathSize; ++i) {
     if (NDET[i] > max_ndet)
       max_ndet = NDET[i];
   }

   if (lambdaMin>=lambdaMax) {
     lambdaMin=pEngineContext->buffers.lambda[0];
     lambdaMax=pEngineContext->buffers.lambda[max_ndet-1];
   }
   
   if (!useKurucz && (THRD_id!=THREAD_TYPE_KURUCZ))
     pEngineContext->project.kurucz.fwhmFit=0;

   // load slit function from project properties -> slit page?
   // calibration procedure with FWHM fit -> Kurucz (and xs) are convolved with the fitted slit function
   // no calibration procedure and no xs to convolve -> nothing to do with the slit function in the slit page
   // other cases:
   if ( ( (useKurucz || THRD_id==THREAD_TYPE_KURUCZ) && !pKuruczOptions->fwhmFit) // calibration procedure but FWHM not fitted
        || (!useKurucz  && xsToConvolute) ) {   // no calibration procedure and xs to convolve
     // -> use the slit function in the slit page of project properties to convolve
     //    solar spectrum and xs
     rc=ANALYSE_LoadSlit(pSlitOptions,useKurucz||xsToConvoluteI0);
   }
   if (rc)
     goto handle_errors;
   
   if ((THRD_id==THREAD_TYPE_KURUCZ) || useKurucz) {
     // pre-load multi-row Kurucz reference spectrum one time, reuse it for each indexFenoColumn in KURUCZ_Alloc
     char kurucz_file[MAX_ITEM_TEXT_LEN];
     FILES_RebuildFileName(kurucz_file,(pKuruczOptions->fwhmFit)?pKuruczOptions->file:pSlitOptions->kuruczFile,1);

     if ( !strlen(kurucz_file) ) {
       rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_MSGBOX_FIELDEMPTY, "Solar Ref. File");
     } else {
       const char *ext = strrchr(kurucz_file, '.');
       if (ext == NULL) {
         ext = "";
       }
       if (!strcmp(ext, ".nc")) { // netCDF file
         rc=MATRIX_netcdf_Load(kurucz_file, &hr_solar_temp, 0, 0, lambdaMin, lambdaMax, 1, 0, NULL, __func__);
       } else {
         rc=MATRIX_Load(kurucz_file, &hr_solar_temp, 0, 0, lambdaMin, lambdaMax, 1, 0, __func__);
       }
     }

     if (pKuruczOptions->fwhmType==SLIT_TYPE_FILE) {
       const char *slitFile = pEngineContext->project.kurucz.slfFile;

       if (!strlen(slitFile)) {
         rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit File");
       } else {
         rc = MATRIX_Load(slitFile,&slit_matrix_temp, 0, 0, -9999., 9999., 1, 0, __func__);
       }
     }
   }

   if (rc)
     goto handle_errors;

   for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++) {

     if (!pEngineContext->project.instrumental.use_row[indexFenoColumn])
       continue;

     for (indexWindow=0;(indexWindow<NFeno) && !rc;indexWindow++)
      {
       pTabFeno=&TabFeno[indexFenoColumn][indexWindow];

       if (pTabFeno->saveResidualsFlag && !pTabFeno->hidden &&
         ((pTabFeno->residualSpectrum=(double *)MEMORY_AllocDVector(__func__,"residualSpectrum",0,pTabFeno->fit_properties.DimL))==NULL))
          break;

       if ((xsToConvolute && !useKurucz) || !pKuruczOptions->fwhmFit)
        {
         if ((pSlitOptions->slitFunction.slitType==SLIT_TYPE_NONE) && pTabFeno->xsToConvolute)
           rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_CONVOLUTION);
         else if ((pTabFeno->gomeRefFlag || pEngineContext->refFlag) &&         // test on pTabFeno->xsToConvolute done in ANALYSE_XsConvolution (molecular ring done in this function for both convolution and interpolation)
                 ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,indexFenoColumn,pSlitOptions->slitFunction.slitWveDptFlag))!=0))
           break;
        }
      }

     if (!rc) {
       // Allocate Kurucz buffers on Run Calibration or
       //                            Run Analysis and wavelength calibration is different from None at least for one spectral window
       //
       // Apply the calibration procedure on the reference spectrum if the wavelength calibration is different from None at least for one spectral window

       if ((THRD_id==THREAD_TYPE_KURUCZ) || useKurucz) {

         rc=KURUCZ_Alloc(&pEngineContext->project,pEngineContext->buffers.lambda,indexKurucz,lambdaMin,lambdaMax,indexFenoColumn, &hr_solar_temp, &slit_matrix_temp);
         if (rc) {
           goto handle_errors; // If KURUCZ_Alloc fails, there is a fatal configuration error.
         }

         // the reference spectrum is corrected by the instrument function if any
         if (useKurucz) {
           rc=KURUCZ_Reference(pEngineContext->buffers.instrFunction,0,saveFlag,1,responseHandle,indexFenoColumn);
         }
       }

       if (!rc && (THRD_id!=THREAD_TYPE_KURUCZ)) {
         rc=ANALYSE_AlignReference(pEngineContext,0,responseHandle,indexFenoColumn);
       }
     }

     if ( (ANALYSE_swathSize > 1) && rc) {
       // Error on one irradiance spectrum shouldn't stop the analysis of other spectra
       ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_IMAGER_CALIB, 1+indexFenoColumn);
       imager_err = true;
       for (indexWindow=0;indexWindow<NFeno;indexWindow++)
         TabFeno[indexFenoColumn][indexWindow].rcKurucz=rc;
       rc=ERROR_ID_NO;
     }
   }  // for (indexFenoColumn=0;(

   // OMI SEE LATER

   if (!rc && !(rc=OUTPUT_RegisterData(pEngineContext)) &&
       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) && useUsamp &&
       !(rc=ANALYSE_UsampGlobalAlloc(lambdaMin,lambdaMax,max_ndet)) &&
       !(rc=ANALYSE_UsampLocalAlloc(1)))
    rc=ANALYSE_UsampBuild(0,1,0);   // !!! ACCOUNT FOR UNDERSAMPLING ???

 handle_errors:

   GEMS_CloseReferences();
   radiance_ref_clear_cache();
   MATRIX_Free(&hr_solar_temp, __func__);
   MATRIX_Free(&slit_matrix_temp, __func__);

   if (rc!=ERROR_ID_NO) {
     ERROR_DisplayMessage(responseHandle);
   } else if (imager_err) {
     ERROR_DisplayMessage(responseHandle);
   }

   #if defined(__DEBUG_) && __DEBUG_
   DEBUG_Stop(__func__);
   #endif

   return (rc!=ERROR_ID_NO)?-1:0;    // supposed that an error at the level of the load of projects stops the current session
 }

// ===============================================================
// TRANSFER OF THE LIST OF SYMBOLS FROM THE MEDIATOR TO THE ENGINE
// ===============================================================

int mediateRequestSetSymbols(void *engineContext,
                 int numberOfSymbols,
                 const mediate_symbol_t *symbols,
                 void *responseHandle)
 {
   // Declarations

   int indexSymbol;
   RC rc;

   // Initializations

   SYMB_itemCrossN=SYMBOL_PREDEFINED_MAX;
   rc=ERROR_ID_NO;

   // Add symbols in the list

   for (indexSymbol=0;(indexSymbol<numberOfSymbols) && !rc;indexSymbol++)
    rc=SYMB_Add(symbols[indexSymbol].name,symbols[indexSymbol].description);

   // Check for error

   if (rc)
    ERROR_DisplayMessage(responseHandle);

   // Return

   return rc;
 }

// =========================================================================
// TRANSFER OF THE LIST OF OBSERVATION SITES FROM THE MEDIATOR TO THE ENGINE
// =========================================================================

int mediateRequestSetSites(void *engineContext,
               int numberOfSites,
               const mediate_site_t *sites,
               void *responseHandle)
 {
   // Declarations

   int indexSite;
   RC rc;

   // Initializations

   SITES_itemN=0;
   rc=ERROR_ID_NO;

   // Add the observation site in the list

   for (indexSite=0;(indexSite<numberOfSites) && !rc;indexSite++)
    rc=SITES_Add((OBSERVATION_SITE *)&sites[indexSite]);

   // Check for error

   if (rc)
    ERROR_DisplayMessage(responseHandle);

   // Return

   return rc;
 }

// ==================
// BROWSING FUNCTIONS
// ==================

int mediateRequestGotoSpectrum(void *engineContext,
                   int recordNumber,
                   void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

   if (recordNumber > 0 && recordNumber <= pEngineContext->recordNumber) {
    pEngineContext->currentRecord=recordNumber;

    return recordNumber;
   }
   else {
    pEngineContext->currentRecord = 0;
    return 0;
   }
 }

int mediateRequestNextMatchingSpectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
   // Declarations

   PROJECT *pProject;                                                            // pointer to the project part of the engine context
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   int orec=pEngineContext->indexRecord;
   int rec;
   int upperLimit=pEngineContext->recordNumber;
   int inc,geoFlag;
   int outputFlag;
   double longit,latit;
   INDEX indexSite;
   OBSERVATION_SITE *pSite;
   RC rc;

   // Initializations

   pProject=&pEngineContext->project;
   pRecord=&pEngineContext->recordInfo;
   outputFlag=
    (((THRD_id==THREAD_TYPE_KURUCZ) && pProject->asciiResults.calibFlag) ||
     ((THRD_id==THREAD_TYPE_ANALYSIS) && pProject->asciiResults.analysisFlag))?1:0;

   inc=1;
   geoFlag=1;

   rc=ERROR_ID_NO;

   if (!pEngineContext->recordNumber)                                            // file is empty
    return 0;

   // IAP 200812 - set 'starting point' - next or goto
   if (pEngineContext->currentRecord) {
    // use this as the starting point rather than indexRecord
    rec = pEngineContext->currentRecord;
    // reset the currentRecord to 0 now that the GOTO request has been serviced.
    pEngineContext->currentRecord = 0;
   }
   else {
    rec = pEngineContext->indexRecord + 1; // start at the Next record
   }

   // consider increasing the starting point
   if (pProject->spectra.noMin && rec < pProject->spectra.noMin)
    rec = pProject->spectra.noMin;

   // consider reducing the upper boundary (inclusive)
   if (pProject->spectra.noMax && pProject->spectra.noMax < upperLimit)
    upperLimit = pProject->spectra.noMax;

   // Loop in search of a matching record - respect the min and max limits
   // in general 'break' to exit the loop

   while (rc == ERROR_ID_NO && rec <= upperLimit) {

     // read the 'next' record
     if ((rc=EngineReadFile(pEngineContext,rec,0,0))!=ERROR_ID_NO) {

       // reset the rc based on the severity of the failure - for non fatal errors keep searching
       rc = ERROR_DisplayMessage(responseHandle);
     } else if ( (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI ||
                  pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMIV4) &&
                 !omi_use_track(pRecord->xtrack_QF, pProject->instrumental.omi.xtrack_mode) ) {
       // skip this spectrum
     } else {
       longit=pRecord->longitude;
       latit=pRecord->latitude;
       geoFlag=1;

       if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_CIRCLE) && (pProject->spectra.radius>1.) &&
           (THRD_GetDist(longit,latit,pProject->spectra.longMin,pProject->spectra.latMin)>(double)pProject->spectra.radius))
        geoFlag=0;
       else if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_OBSLIST) && (pProject->spectra.radius>1.)) {

         for (indexSite=0;indexSite<SITES_itemN;indexSite++) {
           pSite=&SITES_itemList[indexSite];

           if (THRD_GetDist(longit,latit,pSite->longitude,pSite->latitude)<=(double)pProject->spectra.radius)
             break;
         }
         if (indexSite==SITES_itemN) {
          geoFlag=0;
         }
       } else if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_RECTANGLE) &&

                  (((pProject->spectra.longMin!=pProject->spectra.longMax) &&
                    ((longit>max(pProject->spectra.longMin,pProject->spectra.longMax)) ||
                     (longit<min(pProject->spectra.longMin,pProject->spectra.longMax)))) ||

                   ((pProject->spectra.latMin!=pProject->spectra.latMax) &&
                    ((latit>max(pProject->spectra.latMin,pProject->spectra.latMax)) ||
                     (latit<min(pProject->spectra.latMin,pProject->spectra.latMax)))))) {
         geoFlag=0;
       }
       
       // Check SZA

       if (geoFlag &&
       (((fabs(pProject->spectra.SZAMin-pProject->spectra.SZAMax)>(double)1.e-4) && ((pRecord->Zm>max(pProject->spectra.SZAMin,pProject->spectra.SZAMax)) || (pRecord->Zm<min(pProject->spectra.SZAMin,pProject->spectra.SZAMax)))) ||
        ((fabs(pProject->spectra.SZADelta)>(double)1.e-4) && (fabs(pRecord->Zm-pRecord->oldZm)<pProject->spectra.SZADelta))))

        geoFlag=0;

       // Check elevation angle

       if (geoFlag &&
        (((fabs(pProject->spectra.elevMin-pProject->spectra.elevMax)>(double)1.e-4) || (fabs(pProject->spectra.elevMax)>0.)) &&
         ((pRecord->elevationViewAngle>pProject->spectra.elevMax+pProject->spectra.elevTol) || (pRecord->elevationViewAngle<pProject->spectra.elevMin-pProject->spectra.elevTol))))

        geoFlag=0;

       if (geoFlag) // this record matches - exit the search loop
        break;

     }

     if (outputFlag && (THRD_id!=THREAD_TYPE_EXPORT) && !pEngineContext->project.asciiResults.successFlag) {   // analysis : bad record but save all spectra

       int indexFenoColumn=(pEngineContext->recordNumber - 1) % ANALYSE_swathSize;

       for (int indexFeno=0;indexFeno<NFeno;indexFeno++)
         if (!TabFeno[indexFenoColumn][indexFeno].hidden)
           TabFeno[indexFenoColumn][indexFeno].rc=ERROR_ID_FILE_RECORD;             // force the output to default values

       OUTPUT_SaveResults(pEngineContext,indexFenoColumn);
     }

     // try the next record
     rec+=inc;
    }  // end while

   if (rc != ERROR_ID_NO) {
    // search loop terminated due to fatal error - a message was already logged
    return -1;
   }
   else if (rec > upperLimit) {
    // did not find a matching record ... reread the last matching index (if there was one)
    // and return 0 to indicate the end of records.

    if (orec != 0
        && (rc=EngineReadFile(pEngineContext,orec,0,0))!=ERROR_ID_NO) {

      if (outputFlag && (THRD_id!=THREAD_TYPE_EXPORT) && !pEngineContext->project.asciiResults.successFlag) {
        int indexFenoColumn=(pEngineContext->recordNumber - 1) % ANALYSE_swathSize;

        for (int indexFeno=0;indexFeno<NFeno;indexFeno++)
          if (!TabFeno[indexFenoColumn][indexFeno].hidden)
            TabFeno[indexFenoColumn][indexFeno].rc=ERROR_ID_FILE_RECORD;             // force the output to default values

        OUTPUT_SaveResults(pEngineContext,indexFenoColumn);
      }

      ERROR_DisplayMessage(responseHandle);
      return -1; // error
    }
    return 0; // No more matching records
   }


   else if (THRD_id==THREAD_TYPE_EXPORT)
    {
        int indexFenoColumn=(pEngineContext->recordNumber - 1) % ANALYSE_swathSize;
        
        OUTPUT_SaveResults(pEngineContext,indexFenoColumn);
    }

   return pEngineContext->indexRecord;
 }

int mediateRequestBeginBrowseSpectra(void *engineContext,
                     const char *spectraFileName,
                     void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   int rc;

   rc=ERROR_ID_NO;

   if (EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle)!=0)
    rc=ERROR_DisplayMessage(responseHandle);

   return ((rc==ERROR_ID_NO)?pEngineContext->recordNumber:-1);
 }

// mediateRequestNextMatchingBrowseSpectrum
//
// attempt to locate and extract the data for the next spectral record in the current
// spectra file that matches the filter conditions of the current project. The search
// begins with the current spectral record. On success the data is extracted and
// pre-processed based on the settings of the current project. The spectrum data is
// returned with a call to
//    mediateResponsePlotData(page, plotDataArray, arrayLength, title, xLabel, yLabel, responseHandle);
//
// On success, the actual record number of the matching spectrum is returned. Zero is returned
// if a matching spectrum is not found. -1 is returned for all other errors and an error message
// should be posted with
//    mediateResponseErrorMessage(functionName, messageString, errorLevel, responseHandle);

int mediateRequestNextMatchingBrowseSpectrum(void *engineContext,
                         void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

   int rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);

   if (rec > 0 && (pEngineContext->indexRecord<=pEngineContext->recordNumber)) {

    mediateRequestPlotSpectra(pEngineContext,responseHandle);
   }

   return rec;
 }

int mediateRequestBeginExportSpectra(void *engineContext,
                     const char *spectraFileName,
                     void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   int rc;

   rc=ERROR_ID_NO;

   if (EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle)!=0)
    rc=ERROR_DisplayMessage(responseHandle);

   return ((rc==ERROR_ID_NO)?pEngineContext->recordNumber:-1);
 }

// mediateRequestNextMatchingBrowseSpectrum
//
// attempt to locate and extract the data for the next spectral record in the current
// spectra file that matches the filter conditions of the current project. The search
// begins with the current spectral record. On success the data is extracted and
// pre-processed based on the settings of the current project. The spectrum data is
// returned with a call to
//    mediateResponsePlotData(page, plotDataArray, arrayLength, title, xLabel, yLabel, responseHandle);
//
// On success, the actual record number of the matching spectrum is returned. Zero is returned
// if a matching spectrum is not found. -1 is returned for all other errors and an error message
// should be posted with
//    mediateResponseErrorMessage(functionName, messageString, errorLevel, responseHandle);

int mediateRequestBeginAnalyseSpectra(void *engineContext,
                                      const char *configFileName,
                      const char *spectraFileName,
                      void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc = ERROR_ID_NO;

   strncpy(pEngineContext->project.config_file, configFileName, FILENAME_BUFFER_LENGTH -1);

   rc=EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle);

   if (rc != ERROR_ID_NO) {
     ERROR_DisplayMessage(responseHandle);
     return -1;
   } else {
     return pEngineContext->recordNumber;
   }
 }

int mediateRequestNextMatchingAnalyseSpectrum(void *engineContext,
                          void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rcOutput = ERROR_ID_NO;

   int rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);

   if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI &&
       pEngineContext->analysisRef.refAuto)
    {
     // for omi, when using automatic reference selection for one or
     // more analysis windows, check if automatic reference spectrum is
     // ok for this detector row, if not: get next spectrum.
     while( !omi_has_automatic_reference(pEngineContext->recordInfo.i_crosstrack)
            && rec > 0 )
      {
       rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);
      }
    }

   if (rec > 0 && (pEngineContext->indexRecord<=pEngineContext->recordNumber))
    {
     mediateRequestPlotSpectra(pEngineContext,responseHandle);
     ANALYSE_InitResults();

     if (!pEngineContext->analysisRef.refAuto || pEngineContext->satelliteFlag || ((pEngineContext->recordInfo.rc=EngineNewRef(pEngineContext,responseHandle))==ERROR_ID_NO))
      pEngineContext->recordInfo.rc=ANALYSE_Spectrum(pEngineContext,responseHandle);

    if ((pEngineContext->mfcDoasisFlag || (pEngineContext->lastSavedRecord!=pEngineContext->indexRecord)) &&
        (   ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->project.asciiResults.analysisFlag && (!pEngineContext->project.asciiResults.successFlag || !pEngineContext->recordInfo.rc )) // (!pEngineContext->project.asciiResults.successFlag /* || nrc */))
            || ((THRD_id==THREAD_TYPE_KURUCZ) && pEngineContext->project.asciiResults.calibFlag) ) )

      pEngineContext->recordInfo.rc=OUTPUT_SaveResults(pEngineContext,pEngineContext->recordInfo.i_crosstrack);

//    if (!rc)
//      rc=rcOutput;

     if (pEngineContext->recordInfo.rc!=ERROR_ID_NO)
      ERROR_DisplayMessage(responseHandle);
    }

   // NB if the function returns -1, the problem is that it is not possible to process
   // next records.

   return ((pEngineContext->recordInfo.rc != ERROR_ID_REF_ALIGNMENT) || pEngineContext->analysisRef.refScan) ? rec : -1;
 }

int mediateRequestPrevMatchingAnalyseSpectrum(void *engineContext,
                          void *responseHandle)
 {
   return 0;
 }

int mediateRequestBeginCalibrateSpectra(void *engineContext,
                    const char *spectraFileName,
                    void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc;

   if ((rc=EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle))!=ERROR_ID_NO)
    ERROR_DisplayMessage(responseHandle);

   return (rc==ERROR_ID_NO)?((ENGINE_CONTEXT *)engineContext)->recordNumber:-1;
 }

int mediateRequestNextMatchingCalibrateSpectrum(void *engineContext,
                        void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc = ERROR_ID_NO;

   int rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);

   if (rec > 0 && (pEngineContext->indexRecord<=pEngineContext->recordNumber))
    {
     mediateRequestPlotSpectra(pEngineContext,responseHandle);
     if ((rc = ANALYSE_Spectrum(pEngineContext,responseHandle))!=ERROR_ID_NO)
      ERROR_DisplayMessage(responseHandle);
    }


   if ((pEngineContext->mfcDoasisFlag || (pEngineContext->lastSavedRecord!=pEngineContext->indexRecord)) &&
       (THRD_id==THREAD_TYPE_KURUCZ) && pEngineContext->project.asciiResults.calibFlag)

     pEngineContext->recordInfo.rc=OUTPUT_SaveResults(pEngineContext,pEngineContext->recordInfo.i_crosstrack);

//   if (!rc)
//     rc=rcOutput;

    if (pEngineContext->recordInfo.rc!=ERROR_ID_NO)
     ERROR_DisplayMessage(responseHandle);





   // NB if the function returns -1, the problem is that it is not possible to process
   // next records.
   return rec; // (rc == ERROR_ID_NO) ? rec : -1;
 }

int mediateRequestPrevMatchingCalibrateSpectrum(void *engineContext,
                        void *responseHandle)
 {
   return 0;
 }

int mediateRequestStop(void *engineContext,
               void *responseHandle)
 {
   // Close open files, write "automatic" output, and release
   // allocated buffers to reset the engine context
   int rc = EngineRequestEndBrowseSpectra((ENGINE_CONTEXT *)engineContext);

   if (rc != 0) {
    ERROR_DisplayMessage(responseHandle);
    return -1;
   }

   return 0;
 }


int mediateRequestViewCrossSections(void *engineContext, char *awName,double minWavelength, double maxWavelength,
                                    int nFiles, char **filenames, void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   char symbolName[MAX_ITEM_NAME_LEN+1],*ptr,                                   // the symbol name
        windowTitle[MAX_ITEM_TEXT_LEN],                                         // title to display at the top of the page
        tabTitle[MAX_ITEM_TEXT_LEN],                                            // title to display on the tab of the page
       *ext;                                                                    // pointer to the extension of the file
   MATRIX_OBJECT xs;                                                            // matrix to load the cross section
   INDEX indexFile;                                                             // browse files
   int   indexLine,indexColumn;                                                 // browse lines and column in the data page
   bool use_rows[MAX_SWATHSIZE];                                                // only select the first row

   // Initializations

   sprintf(windowTitle,"Cross sections used in %s analysis window of project %s",awName,pEngineContext->project.project_name);
   sprintf(tabTitle,"%s.%s (XS)",pEngineContext->project.project_name,awName);
   indexLine=indexColumn=2;
   memset(use_rows,false,sizeof(bool)*MAX_SWATHSIZE);
   use_rows[0]=true;

   // Get index of selected analysis window in list

   for (indexFile=0;indexFile<nFiles;indexFile++,indexLine++)
    {
     // Reinitialize the matrix object

     memset(&xs,0,sizeof(MATRIX_OBJECT));

     // Retrieve the symbol name from the file

     if ((ptr=strrchr(filenames[indexFile],'/'))!=NULL)
      {
       strcpy(symbolName,ptr+1);
       if ((ptr=strchr(symbolName,'_'))!=NULL)
        *ptr=0;
      }
     else
      symbolName[0]='\0';

     if ((ext=strrchr(filenames[indexFile],'.'))!=NULL)
      ext++;
     else
      ext="xs";


     // Load the file

     if ((strcmp(ext,"nc") && !MATRIX_Load(filenames[indexFile],&xs,0,0,
                      minWavelength,maxWavelength,
                      0,   // no derivatives
                      1,   // reverse vectors if needed
                      "mediateRequestViewCrossSections") && (xs.nl>1) && (xs.nc>1)) ||
         (!strcmp(ext,"nc") && !MATRIX_netcdf_Load(filenames[indexFile],&xs,0,0,
                      minWavelength,maxWavelength,
                      0,   // no derivatives
                      1,   // reverse vectors if needed
                      use_rows,
                      "mediateRequestViewCrossSections") && (xs.nl>1) && (xs.nc>1))) {
       // Plot the cross section
        MEDIATE_PLOT_CURVES(plotPageCross, Spectrum, allowFixedScale, symbolName, "Wavelength", "cm**2 / molec", responseHandle,
                            CURVE(.name = symbolName, .x = xs.matrix[0], .y = xs.matrix[1], .length = xs.nl));
        mediateResponseLabelPage(plotPageCross,windowTitle,tabTitle, responseHandle);
        mediateResponseCellInfo(plotPageCross,indexLine,indexColumn,responseHandle,filenames[indexFile],"%s","Loaded");
     }
     else
      mediateResponseCellInfo(plotPageCross,indexLine,indexColumn,responseHandle,filenames[indexFile],"%s","Not found !!!");

     // Release the allocated buffers

     MATRIX_Free(&xs,"mediateRequestViewCrossSections");
    }

   // Return

   return 0;
 }
