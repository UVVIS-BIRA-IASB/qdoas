
//  ----------------------------------------------------------------------------
//! \addtogroup Format
//! @{
//!
//! \file      frm4doas_read.cpp
//! \brief     Routines to read data in the netCDF format used in the FRM<sub>4</sub>DOAS network.
//! \details   FRM<sub>4</sub>DOAS holds for Fiducial Reference Measurements for Ground-Based
//!            DOAS Air-Quality Observations (ESA Contract No. 4000118181/16/I-EF).  The purpose
//!            of this project is (2016-2017) to develop a centralised system providing harmonised
//!            ground-based reference data from a network of MAXDOAS instruments within a
//!            short latency period.
//! \details   For further information, see http://frm4doas.aeronomie.be/index.php
//! \authors   Caroline FAYT (qdoas@aeronomie.be)
//! \date      18/12/2017 (creation date)
//! \bug       Known issue (20/12/2017) with Windows : netCDF functions fail for compressed variables (zlib=True) when used with hdf5.dll compiled from source.  \n
//!            Work around : using the hdf5.dll of the precompiled library (4.5.0) seems to solve the problem.
//! \todo      Load reference spectrum + slit function      \n
//!            Check if the wavelength calibration exists   \n
//!            Complete with general ground-based fields
//! \copyright QDOAS is distributed under GNU General Public License
//!
//! @}
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  FRM4DOAS_Read_Metadata  load variables from the metadata group + scan index from the measurements group
//  FRM4DOAS_Set            open the netCDF file, get the number of records and load metadata variables
//  FRM4DOAS_Read           read a specified record from a file in the netCDF format implemented for the FRM4DOAS project
//  FRM4DOAS_Cleanup        close the current file and release allocated buffers
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
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or (at
//  your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string>
#include <vector>
#include <array>
#include <set>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include <cassert>
#include <cmath>

#include "frm4doas_read.h"
#include "netcdfwrapper.h"
// #include "dir_iter.h"

extern "C" {
#include "winthrd.h"
#include "comdefs.h"
#include "stdfunc.h"
#include "engine_context.h"
#include "engine.h"
#include "mediate.h"
#include "analyse.h"
#include "spline.h"
#include "vector.h"
#include "zenithal.h"
}

using std::string;
using std::vector;
using std::set;

// ======================
// STRUCTURES DEFINITIONS
// ======================

  // irradiance reference ---> !!!! CHECK AND COMPLETE

  struct refspec {
    refspec() : lambda(), irradiance(), sigma() {};
    vector<double> lambda;
    vector<double> irradiance;
    vector<double> sigma;
  };

//! \struct metadata
//! \brief This structure defined in frm4doas_read.cpp module mainly contains variables present in the metadata group of the netCDF file (format used in the FRM<sub>4</sub>DOAS network).
//! \details Vectors are automatically allocated when the file is open and released when the file is closed.  It is only locally used by functions of the module frm4doas_read.cpp.
//! \details Five groups are present in netCDF files :\n
//! \details \li <strong>ancillary</strong> for ancillary data;\n
//!          \li <strong>meteorological_data</strong> (temperature and pressures profiles)\n
//!          \li <strong>instrument_location</strong> (name of the station, latitude, longitude and altitude of the instrument)\n
//!          \li <strong>keydata</strong> (reference spectrum, measured slit function(s))\n
//!          \li <strong>measurements</strong> : radiances, instrumental errors if known, scan index and wavelength calibration\n
//!          \li <strong>metadata</strong> : currently limited to data present in ASCII files proposed for CINDI-2 reprocessing but could be extended to any data relevant for ground-based measurements (for example, temperature of the detector, �)\n
//! \details <strong>metadata</strong> group contains information useful to QDOAS to describe the records.

  struct metadata
   {
    vector<float> sza;                                                          //!< \details Solar zenith angle
    vector<float> saa;                                                          //!< \details Solar azimuth angle
    vector<float> vea;                                                          //!< \details Viewing elevation angle
    vector<float> vaa;                                                          //!< \details Viewing azimuth angle
    vector<float> mea;                                                          //!< \details Moon elevation angle
    vector<float> maa;                                                          //!< \details Moon azimuth angle
    vector<float> tat;                                                          //!< \details Total acquisition time
    vector<float> tmt;                                                          //!< \details Total measurement time
    vector<float> lon;                                                          //!< \details Longitude
    vector<float> lat;                                                          //!< \details Latitude
    vector<float> alt;                                                          //!< \details Altitude
    vector<float> tint;                                                         //!< \details Exposure time
    vector<int>   mt;                                                           //!< \details Measurement type
    vector<int>   nacc;                                                         //!< \details Number of co-added spectra
    vector<short> dt;                                                           //!< \details Datetime (date and time at half of the measurement (UT YYYY,MM,DD,hh,mm,ss,ms)
    vector<short> dts;                                                          //!< \details Datetime_start (date and time at the beginning of the measurement (UT YYYY,MM,DD,hh,mm,ss,ms)
    vector<short> dte;                                                          //!< \details Datetime_end (date and time at the end of the measurement (UT YYYY,MM,DD,hh,mm,ss,ms)
    vector<short> sci;                                                          //!< \details Scan index
    vector<short> zbi;                                                          //!< \details Zenith before index
    vector<short> zai;                                                          //!< \details Zenith after index

    float stationLon;
   };

// ================
// STATIC VARIABLES
// ================

static NetCDFFile current_file;                                                 //!< \details Pointer to the current netCDF file
static string root_name;                                                        //!< \details The name of the root (should be the basename of the file)
static metadata current_metadata;                                               //!< \details Keep the metadata as far as the netCDF file is open

static size_t det_size;                                                         //!< \details The current detector size

// -----------------------------------------------------------------------------
// FUNCTION FRM4DOAS_Read_Metadata
// -----------------------------------------------------------------------------
//!
//! \fn      static metadata FRM4DOAS_Read_Metadata(size_t number_of_records)
//! \details Load variables from the metadata group + scan index from the measurements group\n
//! \param   [in]  number_of_records the number of records
//! \return  result : a \a metadata structure with all the metadata variables and scan index
//!
// -----------------------------------------------------------------------------

static metadata FRM4DOAS_Read_Metadata(size_t number_of_records)
 {
  // Declarations

  metadata result;                                                              // metadata
  const size_t start[] = {0};                                                   // there is no reason not to start from 0
  const size_t count[] = {number_of_records};                                             // for these variables, only one dimension
  const size_t count1[]={1};                                                    // for these variables, only one element
  const size_t datetime_start[] = {0,0};                                        // there is no reason not to start from 0
  const size_t datetime_count[] = {number_of_records,7};                                  // for datetime variables, two dimensions
  vector<float> lon,lat,alt;                                                    // instrument geolocation + altitude

  // Get the instrument location and altitude

  NetCDFGroup location_group = current_file.getGroup(root_name+"/INSTRUMENT_LOCATION");

  location_group.getVar("latitude",start,count1,1,(float)-1.,lat);
  location_group.getVar("longitude",start,count1,1,(float)-1.,lon);
  location_group.getVar("altitude",start,count1,1,(float)-1.,alt);

  result.stationLon=lon[0];

  // Get metadata variables

  NetCDFGroup metadata_group;

  metadata_group = current_file.getGroup(root_name+"/RADIANCE/GEODATA");

  metadata_group.getVar("viewing_azimuth_angle",start,count,1,(float)-1.,result.vaa);
  metadata_group.getVar("viewing_elevation_angle",start,count,1,(float)-1.,result.vea);
  metadata_group.getVar("solar_zenith_angle",start,count,1,(float)-1.,result.sza);
  metadata_group.getVar("solar_azimuth_angle",start,count,1,(float)-1.,result.saa);
  metadata_group.getVar("moon_elevation_angle",start,count,1,(float)-1.,result.mea);
  metadata_group.getVar("moon_azimuth_angle",start,count,1,(float)-1.,result.maa);

  metadata_group = current_file.getGroup(root_name+"/RADIANCE/OBSERVATIONS");

  metadata_group.getVar("latitude",start,count,1,(float)lat[0],result.lat);
  metadata_group.getVar("longitude",start,count,1,(float)lon[0],result.lon);
  metadata_group.getVar("altitude",start,count,1,(float)alt[0],result.alt);
  metadata_group.getVar("exposure_time",start,count,1,(float)-1.,result.tint);
  metadata_group.getVar("total_acquisition_time",start,count,1,(float)-1.,result.tat);
  metadata_group.getVar("total_measurement_time",start,count,1,(float)-1.,result.tmt);
  metadata_group.getVar("number_of_coadded_spectra",start,count,1,(int)-1,result.nacc);
  metadata_group.getVar("measurement_type",start,count,1,(int)0,result.mt);
  metadata_group.getVar("datetime",datetime_start,datetime_count,2,(short)-1,result.dt);
  metadata_group.getVar("datetime_start",datetime_start,datetime_count,2,(short)-1,result.dts);
  metadata_group.getVar("datetime_end",datetime_start,datetime_count,2,(short)-1,result.dte);
  metadata_group.getVar("scan_index",start,count,1,(short)-1,result.sci);
  metadata_group.getVar("index_zenith_before",start,count,1,(short)-1,result.zbi);  // Check the presence of the vector !!!
  metadata_group.getVar("index_zenith_after",start,count,1,(short)-1,result.zai);

  // Return

  return result;
 }

// -----------------------------------------------------------------------------
// FUNCTION FRM4DOAS_Set
// -----------------------------------------------------------------------------
//!
//! \fn      RC FRM4DOAS_Set(ENGINE_CONTEXT *pEngineContext)
//! \details Open the netCDF file, get the number of records and load metadata variables.\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.\n
//! \return  ERROR_ID_NETCDF on run time error (opening of the file didn't succeed, missing variable...)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC FRM4DOAS_Set(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  RC rc = ERROR_ID_NO;
  ENGINE_refStartDate=1;

  // Try to open the file and load metadata

  try
   {
    current_file = NetCDFFile(pEngineContext->fileInfo.fileName,NC_NOWRITE);    // open file
    root_name = current_file.getName();                                         // get the root name (should be the file name)

    NetCDFGroup root_group = current_file.getGroup(root_name);                  // go to the root

    pEngineContext->n_alongtrack=
    pEngineContext->recordNumber=root_group.dimLen("number_of_records");                  // get the record number

    pEngineContext->n_crosstrack=1;                                             // spectra should be one dimension only
    det_size=root_group.dimLen("detector_size");                                     // get the number of pixels of the detector

    for (int i=0;i<(int)det_size;i++)
     pEngineContext->buffers.irrad[i]=(double)0.;

    // Read metadata

    current_metadata = FRM4DOAS_Read_Metadata(pEngineContext->recordNumber);

    auto dt = reinterpret_cast<const short(*)[7]>(current_metadata.dt.data());

    pEngineContext->fileInfo.startDate.da_day=(char)dt[0][2];
    pEngineContext->fileInfo.startDate.da_mon=(char)dt[0][1];
    pEngineContext->fileInfo.startDate.da_year=dt[0][0];

    THRD_localShift=current_metadata.stationLon/15.;
   }
  catch (std::runtime_error& e)
   {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());  // in case of error, capture the message
   }

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION FRM4DOAS_Read
// -----------------------------------------------------------------------------
//!
//! /fn      RC FRM4DOAS_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay)
//! /details Read a specified record from a file in the netCDF format implemented for the FRM<sub>4</sub>DOAS project\n
//! /param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.\n
//! /param   [in]  recordNo        the index of the record to read\n
//! /param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise\n
//! /param   [in]  localDay        if /a dateFlag is 1, the calendar day for the reference spectrum to search for\n
//! /return  ERROR_ID_FILE_END if the requested record number is not found\n
//!          ERROR_ID_FILE_RECORD if the requested record doesn't satisfy current criteria (for example for the selection of the reference)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC FRM4DOAS_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay)
 {
  // Declarations

  NetCDFGroup measurements_group;                                               // measurement group in the netCDF file
  RECORD_INFO *pRecordInfo;                                                     // pointer to the record structure in the engine context
  double tmLocal;                                                               // calculation of the local time (in number of seconds)
  vector<float> wve;                                                            // wavelength calibration
  vector<float> spe;                                                            // spectrum
  vector<float> err;                                                            // instrumental errors
  vector<short> qf;                                                             // quality flag
  int measurementType;
  int i;                                                                        // index for loops and arrays
  RC rc;                                                                        // return code

  // Initializations

  pRecordInfo=&pEngineContext->recordInfo;
  rc = ERROR_ID_NO;

  // The requested record is out of range

  if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    // Goto the requested record

    const size_t start[] = {(size_t)(recordNo-1), 0};
    const size_t count[] = {(size_t)1, det_size};                               // only one record to load

    // Spectra

    if (!dateFlag)
     {
      measurements_group=current_file.getGroup(root_name+"/RADIANCE/OBSERVATIONS");

      measurements_group.getVar("wavelength",start,count,2,(float)0.,wve);
      measurements_group.getVar("radiance",start,count,2,(float)0.,spe);
      measurements_group.getVar("radiance_error",start,count,2,(float)1.,err);
      measurements_group.getVar("radiance_quality_flag",start,count,2,(short)1,qf);

      for (i=0;i<det_size;i++)
       {
        pEngineContext->buffers.lambda_irrad[i]=(double)wve[i];    // Check and complete
        pEngineContext->buffers.lambda[i]=wve[i];

        pEngineContext->buffers.spectrum[i]=spe[i];
        pEngineContext->buffers.sigmaSpec[i]=err[i];
       }
     }

    // Date and time fields (UT YYYY,MM,DD,hh,mm,ss,ms)

    auto dt = reinterpret_cast<const short(*)[7]>(current_metadata.dt.data());

    pRecordInfo->present_datetime.thedate.da_day=(char)dt[recordNo-1][2];
    pRecordInfo->present_datetime.thedate.da_mon=(char)dt[recordNo-1][1];
    pRecordInfo->present_datetime.thedate.da_year=dt[recordNo-1][0];
    pRecordInfo->present_datetime.thetime.ti_hour=(char)dt[recordNo-1][3];
    pRecordInfo->present_datetime.thetime.ti_min=(char)dt[recordNo-1][4];
    pRecordInfo->present_datetime.thetime.ti_sec=(char)dt[recordNo-1][5];
    pRecordInfo->present_datetime.millis=dt[recordNo-1][6];

    auto dts = reinterpret_cast<const short(*)[7]>(current_metadata.dts.data());

    pRecordInfo->startDateTime.thedate.da_day=(char)dts[recordNo-1][2];
    pRecordInfo->startDateTime.thedate.da_mon=(char)dts[recordNo-1][1];
    pRecordInfo->startDateTime.thedate.da_year=dts[recordNo-1][0];
    pRecordInfo->startDateTime.thetime.ti_hour=(char)dts[recordNo-1][3];
    pRecordInfo->startDateTime.thetime.ti_min=(char)dts[recordNo-1][4];
    pRecordInfo->startDateTime.thetime.ti_sec=(char)dts[recordNo-1][5];
    pRecordInfo->startDateTime.millis=dts[recordNo-1][6];

    auto dte = reinterpret_cast<const short(*)[7]>(current_metadata.dte.data());

    pRecordInfo->endDateTime.thedate.da_day=(char)dte[recordNo-1][2];
    pRecordInfo->endDateTime.thedate.da_mon=(char)dte[recordNo-1][1];
    pRecordInfo->endDateTime.thedate.da_year=dte[recordNo-1][0];
    pRecordInfo->endDateTime.thetime.ti_hour=(char)dte[recordNo-1][3];
    pRecordInfo->endDateTime.thetime.ti_min=(char)dte[recordNo-1][4];
    pRecordInfo->endDateTime.thetime.ti_sec=(char)dte[recordNo-1][5];
    pRecordInfo->endDateTime.millis=dte[recordNo-1][6];

    // Other metadata

    pRecordInfo->NSomme=current_metadata.nacc[recordNo-1];
    pRecordInfo->Tint=current_metadata.tint[recordNo-1];
    pRecordInfo->Zm=current_metadata.sza[recordNo-1];
    pRecordInfo->Azimuth=current_metadata.saa[recordNo-1];
    pRecordInfo->longitude=current_metadata.lon[recordNo-1];
    pRecordInfo->latitude=current_metadata.lat[recordNo-1];
    pRecordInfo->altitude=current_metadata.alt[recordNo-1];
    pRecordInfo->elevationViewAngle=current_metadata.vea[recordNo-1];
    pRecordInfo->azimuthViewAngle=current_metadata.vaa[recordNo-1];
    pRecordInfo->maxdoas.measurementType=current_metadata.mt[recordNo-1];
    pRecordInfo->TotalExpTime=current_metadata.tmt[recordNo-1];
    pRecordInfo->TotalAcqTime=current_metadata.tat[recordNo-1];
    pRecordInfo->maxdoas.measurementType=current_metadata.mt[recordNo-1];

    pRecordInfo->maxdoas.scanIndex=current_metadata.sci[recordNo-1];
    pRecordInfo->maxdoas.zenithBeforeIndex=current_metadata.zbi[recordNo-1];
    pRecordInfo->maxdoas.zenithAfterIndex=current_metadata.zai[recordNo-1];

    if (pRecordInfo->NSomme<0)   // -1 means that the information is not available
     pRecordInfo->NSomme=1;
    
    pRecordInfo->Tm=(double)ZEN_NbSec(&pRecordInfo->present_datetime.thedate,&pRecordInfo->present_datetime.thetime,0);
    
    tmLocal=pRecordInfo->Tm+THRD_localShift*3600.;
    pRecordInfo->localCalDay=ZEN_FNCaljda(&tmLocal);

    // Recalculate solar zenith angle if necessary
    
    if (std::isnan(pRecordInfo->Zm) && !std::isnan(pRecordInfo->longitude) && !std::isnan(pRecordInfo->latitude))
     {
      double longitude;
      longitude=-pRecordInfo->longitude;
      pRecordInfo->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecordInfo->Tm),&longitude,&pRecordInfo->latitude,&pRecordInfo->Azimuth);

      pRecordInfo->Azimuth+=180.; // because the used convention is 0..360, 0° Northward and ZEN_FNTdiz is -180..180
     }

    // Selection of the reference spectrum

    measurementType=pEngineContext->project.instrumental.user;

    if (rc || (dateFlag && ((pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH) ||
             ((fabs(pRecordInfo->elevationViewAngle+1.)>EPSILON) &&
             ((pRecordInfo->elevationViewAngle<pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol) ||
              (pRecordInfo->elevationViewAngle>pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))))
     rc=ERROR_ID_FILE_RECORD;

    else if (!dateFlag && (measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_NONE))
     {
      if (((measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH)) ||
          ((measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=measurementType)))

       rc=ERROR_ID_FILE_RECORD;
     }

    // Later : add the selection of the measurement type in the instrumental page else if (!dateFlag && (measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_NONE))
    // Later : add the selection of the measurement type in the instrumental page  {
    // Later : add the selection of the measurement type in the instrumental page      if (((measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH)) ||
    // Later : add the selection of the measurement type in the instrumental page          ((measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS) && (pRecordInfo->maxdoas.measurementType!=measurementType)))
    // Later : add the selection of the measurement type in the instrumental page
    // Later : add the selection of the measurement type in the instrumental page       rc=ERROR_ID_FILE_RECORD;
    // Later : add the selection of the measurement type in the instrumental page  }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION FRM4DOAS_Cleanup
// -----------------------------------------------------------------------------
//!
//! /fn      void FRM4DOAS_Cleanup(void)
//! /details Close the current file and release allocated buffers\n
//!
// -----------------------------------------------------------------------------

void FRM4DOAS_Cleanup(void)
 {
  current_file.close();

  current_metadata = metadata();
 }

