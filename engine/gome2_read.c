//    This file is part of QDOAS.
//
//    QDOAS is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    QDOAS is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with QDOAS.  If not, see <http://www.gnu.org/licenses/>.
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  GOME2 interface
//  Name of module    :  Gome2Read.C
//  Creation date     :  4 April 2007
//
//  Author            :  Caroline FAYT, Thomas Danckaert
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
//
//  Please acknowledge the authors in publications on results obtained
//  with the program.
//
//  ----------------------------------------------------------------------------
//
//  This module uses readout routines written by Stefan Noel (Stefan.Noel@iup.physik.uni.bremen.de)
//  and Andreas Richter (richter@iup.physik.uni-bremen.de) from IFE/IUP Uni Bremen.  These routines
//  are based on based on the CODA library.
//
//  ----------------------------------------------------------------------------

// ========
// INCLUDES
// ========

#include "gome2_read.h"

#include <math.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>

#include "comdefs.h"
#include "spectrum_files.h"
#include "engine_context.h"
#include "spline.h"
#include "stdfunc.h"
#include "mediate.h"
#include "engine.h"
#include "output.h"
#include "kurucz.h"
#include "analyse.h"
#include "vector.h"
#include "zenithal.h"
#include "winthrd.h"
#include "ref_list.h"
#include "visual_c_compat.h"

#include "coda.h"

// ====================
// CONSTANTS DEFINITION
// ====================                                                         // these definitions come from the routines written by Stefan Noel and Andreas Richter

#define MAX_GOME2_FILES 500                                                     // usually, maximum of files per day is 480

#define NCHANNEL 6                                                              // the number of channels
#define NPIXEL_MAX 1024                                                         // the size of the detector
#define NBAND 10                                                                // number of virtual bands
#define EARTHSHINE 6                                                            // MDR subclass for earthshine measurements
#define N_TINT_MAX 6                                                            // max. no. of unique int.times
#define N_SCAN_MAX 32                                                           // maximum number of scans

// =====================
// STRUCTURES DEFINITION
// =====================

// Geolocations and angles
struct gome2_geolocation {
  // Geolocations
  double lonCorners[4],
         latCorners[4],
         lonCenter,
         latCenter;

  // Angles
  double solZen[3],
         solAzi[3],
         losZen[3],
         losAzi[3];

  // Satellite position & angles (GEO_BASIC)
  double sat_lat, sat_lon; // coordinates of sub-satellite point
  double sat_alt;
  double sat_sza, sat_saa; // solar zenith/azimuth angle at satellite height
  double sat_vza; // viewing zenith angle at satellite

  // Miscellaneous
  double cloudTopPressure,cloudFraction;                                         // information on clouds
  double earth_radius;
  int   saaFlag;
  int   sunglintDangerFlag;
  int   sunglintHighDangerFlag;
  int   rainbowFlag;
  int   scanDirection;
  int   observationMode;
};

typedef struct _gome2MdrInfo {
  uint16_t num_recs[NBAND];                                                     // number of records for the requested band
  uint16_t rec_length[NBAND];                                                   // record length for the requested band
  INDEX    indexMDR;                                                            // index of the MDR in the file
  double   startTime;                                                           // starting time of the MDR

  // GEO_BASIC data
  double sat_lon[N_SCAN_MAX], sat_lat[N_SCAN_MAX], sat_alt[N_SCAN_MAX],
         sat_sza[N_SCAN_MAX], sat_saa[N_SCAN_MAX]; // solar zenith and azimuth angle (satellite relative actual CS)

  // GEO_EARTH
  double earth_radius;

  // !!! version < 12 : NCHANNEL

  uint8_t  int_index[NCHANNEL];                                                 // index of the integration time (version <= 12
  double   unique_int[NBAND];                                                   // integration time
  double   scanner_angle[N_TINT_MAX][N_SCAN_MAX];                               // scanner angles
  double   corner_lat[N_TINT_MAX][4][N_SCAN_MAX];                               // latitudes at the four corners of the pixels
  double   corner_lon[N_TINT_MAX][4][N_SCAN_MAX];                               // longitudes at the four corners of the pixels
  double   centre_lat[N_TINT_MAX][N_SCAN_MAX];                                  // latitude at the centre of the pixels
  double   centre_lon[N_TINT_MAX][N_SCAN_MAX];                                  // longitude at the centre of the pixels
  double   sza[N_TINT_MAX][3][N_SCAN_MAX];                                      // solar zenith angles
  double   saa[N_TINT_MAX][3][N_SCAN_MAX];                                      // solar azimuth angles
  double   lza[N_TINT_MAX][3][N_SCAN_MAX];                                      // line of sight zenith angles
  double   laa[N_TINT_MAX][3][N_SCAN_MAX];                                      // line of sight azimuth angles
  //     version == 12 : NBAND

  double   integration_times[NBAND];

  double   scanner_angle12[N_SCAN_MAX];                                         // scanner angles
  double   corner_lat12[N_SCAN_MAX][4];                                         // latitudes at the four corners of the pixels
  double   corner_lon12[N_SCAN_MAX][4];                                         // longitudes at the four corners of the pixels
  double   centre_lat12[N_SCAN_MAX];                                            // latitude at the centre of the pixels
  double   centre_lon12[N_SCAN_MAX];                                            // longitude at the centre of the pixels
  double   sza12[N_SCAN_MAX][3];                                                // solar zenith angles
  double   saa12[N_SCAN_MAX][3];                                           // solar azimuth angles
  double   lza12[N_SCAN_MAX][3];                                                // line of sight zenith angles
  double   laa12[N_SCAN_MAX][3];                                                // line of sight azimuth angles

  double   earthshine_wavelength[NPIXEL_MAX];

  uint16_t geo_rec_length[NBAND];
  uint8_t  cloudFitMode[N_SCAN_MAX];
  double   cloudTopPressure[N_SCAN_MAX];
  double   cloudFraction[N_SCAN_MAX];
  uint8_t  scanDirection[N_SCAN_MAX];
  uint8_t  saaFlag[N_SCAN_MAX];
  uint8_t  sunglintDangerFlag[N_SCAN_MAX];
  uint8_t  sunglintHighDangerFlag[N_SCAN_MAX];
  uint8_t  rainbowFlag[N_SCAN_MAX];
  uint8_t  observationMode;
}
GOME2_MDR;

typedef struct _gome2Info {
  double start_lambda;
  double end_lambda;
  GOME2_MDR *mdr;

  uint32_t total_viadr;
  uint32_t total_mdr;

  int total_nadir_mdr;
  int total_nadir_obs;

  uint16_t no_of_pixels;
  uint32_t orbitStart;
  uint32_t orbitEnd;
  uint8_t channelIndex;
}
GOME2_INFO;

typedef struct _GOME2OrbitFiles {                                               // description of an orbit
  char                gome2FileName[MAX_STR_LEN+1];                             // the name of the file with a part of the orbit
  GOME2_INFO          gome2Info;                                                // all internal information about the PDS file like data offsets etc.
  double             *gome2SunRef,*gome2SunWve;                                 // the sun reference spectrum and calibration
  struct gome2_geolocation *gome2Geolocations;                                        // geolocations
  int                 specNumber;
  coda_ProductFile   *gome2Pf;                                                  // GOME2 product file pointer
  coda_Cursor         gome2Cursor;                                              // GOME2 file cursor
  coda_Cursor         gome2CursorMDR;                                           // GOME2 file cursor on MDR
  int                 version;
  int                 rc;
}
GOME2_ORBIT_FILE;

typedef struct _gome2RefSelection {
  double sza;
  double vza;
  double scanner_angle;
  double latitude;
  double longitude;
  double cloudFraction;
  int indexFile;
  int indexRecord;
}
GOME2_REF;

// ================
// GLOBAL VARIABLES
// ================

// ================
// STATIC VARIABLES
// ================

const char *gome2BandName[NBAND] = {
  "BAND_1A",
  "BAND_1B",
  "BAND_2A",
  "BAND_2B",
  "BAND_3",
  "BAND_4",
  "BAND_PP",
  "BAND_PS",
  "BAND_SWPP",
  "BAND_SWPS"
};

const char *gome2WavelengthField[NBAND] = {
  "WAVELENGTH_1A",
  "WAVELENGTH_1B",
  "WAVELENGTH_2A",
  "WAVELENGTH_2B",
  "WAVELENGTH_3",
  "WAVELENGTH_4",
  "WAVELENGTH_PP",
  "WAVELENGTH_PS",
  "WAVELENGTH_SWPP",
  "WAVELENGTH_SWPS"
};

// VZA bins for earthshine references:
static const double vza_bins[] = {0., 7., 14., 21., 28., 35., 42., 49.};

#define NUM_VZA_BINS (sizeof(vza_bins)/sizeof(vza_bins[0]))

// references for analysis window and each bin, depending on scan direction
static struct reference *(*refs_left_scan)[NUM_VZA_BINS-1]; // SCANNER_ANGLE < 0
static struct reference *(*refs_right_scan)[NUM_VZA_BINS-1]; // SCANNER_ANGLE > 0
static struct reference **ref_nadir; // center scan

#define NUM_VZA_REFS (2*NUM_VZA_BINS -1)

// array of pointers to all references, for easy iteration over all references
static struct reference (*vza_refs)[NUM_VZA_REFS];

static void free_vza_refs(void) {
  if(vza_refs != NULL) {
    for (int i=0; i<NFeno; ++i) {
      for (size_t j=0; j<NUM_VZA_REFS; ++j) {
        free(vza_refs[i][j].spectrum);
      }
    }
  }
  free(vza_refs);
  free(refs_left_scan);
  free(refs_right_scan);
  free(ref_nadir);
  vza_refs=NULL;
  refs_left_scan=refs_right_scan=NULL;
  ref_nadir=NULL;
}

static void initialize_vza_refs(void) {
  free_vza_refs();// will free previously allocated structures, if any.

  vza_refs = malloc(NFeno * sizeof(*vza_refs));
  refs_left_scan = malloc(NFeno * sizeof(*refs_left_scan));
  refs_right_scan = malloc(NFeno * sizeof(*refs_right_scan));
  ref_nadir = malloc(NFeno * sizeof(*ref_nadir));

  // Build array of pointers to the collection of VZA references:
  for (int i=0; i<NFeno; ++i) {
    const int n_wavel = TabFeno[0][i].NDET;
    for(size_t j=0; j<NUM_VZA_REFS; ++j) {
      struct reference *ref = &vza_refs[i][j];
      ref->spectrum = malloc(n_wavel*sizeof(*ref->spectrum));
      for (int i=0; i<n_wavel; ++i)
        ref->spectrum[i]=0.;
      ref->n_wavel = n_wavel;
      ref->n_spectra = 0;
    }
    ref_nadir[i] = &vza_refs[i][0];
    for (size_t j=1; j<NUM_VZA_BINS; ++j) {
      refs_left_scan[i][j-1] = &vza_refs[i][j];
      refs_right_scan[i][j-1]  = &vza_refs[i][NUM_VZA_BINS-1+j];
    }
  }
}

// find vza bin for given vza
static inline size_t find_bin(const double vza) {
  assert(vza >= 0.);

  // search backwards, starting from the last bin
  size_t bin = NUM_VZA_BINS-1;
  for (; vza < vza_bins[bin]; --bin);

  return bin;
}

static GOME2_ORBIT_FILE gome2OrbitFiles[MAX_GOME2_FILES];                       // list of files per day
static int gome2OrbitFilesN=0;                                                  // the total number of files to browse in one shot
static INDEX gome2CurrentFileIndex=ITEM_NONE;                                   // index of the current file in the list
int GOME2_beatLoaded=0;
static int gome2LoadReferenceFlag=0;
static int gome2TotalRecordNumber=0;

// =========
// FUNCTIONS
// =========

enum gome2_sort_type { SORT_LAT, SORT_LON, SORT_SZA };

// =====================================
// COMPATIBILITY WITH OLD BEAT FUNCTIONS
// =====================================

int beat_get_utc_string_from_time(double time, char *utc_string) {
  int DAY, MONTH, YEAR, HOUR, MINUTE, SECOND, MUSEC;
  static const char *monthname[12] = {
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
  };

  if (utc_string == NULL) {
    coda_set_error(CODA_ERROR_INVALID_ARGUMENT, "utc_string argument is NULL (%s:%u)", __FILE__, __LINE__);
    return -1;
  }

  if (coda_double_to_datetime(time, &YEAR, &MONTH, &DAY, &HOUR, &MINUTE, &SECOND, &MUSEC) != 0) {
    return -1;
  }
  if (YEAR < 0 || YEAR > 9999) {
    coda_set_error(CODA_ERROR_INVALID_DATETIME, "the year can not be represented using a positive four digit "
                   "number");
    return -1;
  }
  sprintf(utc_string, "%02d-%3s-%04d %02d:%02d:%02d.%06u", DAY, monthname[MONTH - 1], YEAR, HOUR, MINUTE, SECOND,
          MUSEC);

  return 0;
}

int beat_cursor_read_geolocation_double_split(const coda_cursor *cursor, double *dst_latitude,
                                              double *dst_longitude) {
  coda_cursor pair_cursor = *cursor;

  if (coda_cursor_goto_record_field_by_index(&pair_cursor, 0) != 0) {
    return -1;
  }
  if (coda_cursor_read_double(&pair_cursor, dst_latitude) != 0) {
    return -1;
  }
  if (coda_cursor_goto_next_record_field(&pair_cursor) != 0) {
    return -1;
  }
  if (coda_cursor_read_double(&pair_cursor, dst_longitude) != 0) {
    return -1;
  }

  return 0;
}

int beat_cursor_read_geolocation_double_split_array(coda_cursor *cursor, double *dst_latitude, double *dst_longitude) {
  long dim[CODA_MAX_NUM_DIMS];
  long num_elements;
  int num_dims;
  long i;

  if (coda_cursor_get_num_elements(cursor, &num_elements) != 0) {
    return -1;
  }
  if (coda_cursor_get_array_dim(cursor, &num_dims, dim) != 0) {
    return -1;
  }

  if (num_elements > 0) {
    if (coda_cursor_goto_first_array_element(cursor) != 0) {
      return -1;
    }

    for (i = 0; i < num_elements; i++) {
      long index = i;

      if (beat_cursor_read_geolocation_double_split(cursor, &dst_latitude[index], &dst_longitude[index]) != 0) {
        return -1;
      }
      if (i < num_elements - 1) {
        if (coda_cursor_goto_next_array_element(cursor) != 0) {
          return -1;
        }
      }
    }
    coda_cursor_goto_parent(cursor);
  }
  return 0;
}

// =========
// UTILITIES
// =========

/* read UTC string from product; returns both double value (= sec since 1 Jan 2000) and string */
int read_utc_string(coda_Cursor *cursor, char *utc_string, double *data) {
  int status  = coda_cursor_read_double(cursor, data);
  if (status == 0) {
    if (coda_isNaN(*data)) {
      strcpy(utc_string,"                           ");
    } else {
      status = beat_get_utc_string_from_time(*data, utc_string);
    }
  }
  return status;
}

// -----------------------------------------------------------------------------
// FUNCTION      Gome2GotoOBS
// -----------------------------------------------------------------------------
// PURPOSE       Move cursor to the requested radiance record
//
// INPUT         indexBand      index of the selected band
//               indexMDR       index of the requested MDR
//               indexObs       index of the radiance record in the MDR
// -----------------------------------------------------------------------------

void Gome2GotoOBS(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand,INDEX indexMDR,INDEX indexObs) {
  // Declarations

  int obsToBypass;

  // Initializations

  obsToBypass=indexObs*pOrbitFile->gome2Info.mdr[indexMDR].rec_length[indexBand];

  // Goto the current MDR

  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"MDR");
  coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Info.mdr[indexMDR].indexMDR);

  coda_cursor_goto_available_union_field(&pOrbitFile->gome2Cursor);                         // MDR.GOME2_MDR_L1B_EARTHSHINE_V1
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,gome2BandName[indexBand]);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.band(indexBand)

  coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,obsToBypass);
}

// -----------------------------------------------------------------------------
// FUNCTION      Gome2GetMDRIndex
// -----------------------------------------------------------------------------
// PURPOSE       Given a record number, return the MDR index.
//
// INPUT         indexBand      index of the selected band
//               recordNo       the requested record number;
//
// OUTPUT        pObs           the total number of observations covered by
//                              previous MDR
//
// RETURN        the index of the MDR
// -----------------------------------------------------------------------------

INDEX Gome2GetMDRIndex(const GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand,int recordNo,int *pObs) {
  // Declarations

  INDEX indexMDR;                                                               // browse MDR
  int sumObs;                                                                   // accumulate the number of observations in the different states

  // Search for the state

  for (indexMDR=sumObs=0; indexMDR<pOrbitFile->gome2Info.total_nadir_mdr; sumObs+=pOrbitFile->gome2Info.mdr[indexMDR].num_recs[indexBand],indexMDR++)
    if (sumObs+pOrbitFile->gome2Info.mdr[indexMDR].num_recs[indexBand]>recordNo)
      break;

  // Return

  *pObs=sumObs;

  return indexMDR; // pOrbitFile->gome2Info.mdr[indexMDR].indexMDR;
}

// ===============
// GOME2 FUNCTIONS
// ===============

// -----------------------------------------------------------------------------
// FUNCTION      Gome2Open
// -----------------------------------------------------------------------------
// PURPOSE       Open the file name and check if it is really a GOME2 one
//
// INPUT         fileName     the name of the current GOME2 orbit file
//
// OUTPUT        productFile  pointer to the product file
//
// RETURN        ERROR_ID_BEAT if the open failed
//               ERROR_ID_NO  otherwise.
// -----------------------------------------------------------------------------

static int Gome2Open(coda_ProductFile **productFile, const char *fileName, int *version) {
  RC rc=coda_open(fileName,productFile);

  if (rc!=0) {   // && (coda_errno==CODA_ERROR_FILE_OPEN))
    /* maybe not enough memory space to map the file in memory =>
     * temporarily disable memory mapping of files and try again
     */
    coda_set_option_use_mmap(0);
    rc=coda_open(fileName,productFile);
    coda_set_option_use_mmap(1);
  }

  if (rc!=0)
    rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_BEAT,"coda_open",fileName,"");    //coda_errno_to_string(coda_errno));
  else {
    // Retrieve the product class and type

    const char *productClass, *productType;
    coda_get_product_class(*productFile, &productClass);
    coda_get_product_type(*productFile, &productType);
    coda_get_product_version(*productFile, version);

    if (!productClass || strcmp(productClass,"EPS") ||
        !productType || strcmp(productType,"GOME_xxx_1B"))
      rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_BEAT,"coda_get_product_class or coda_get_product_type",fileName,"Not a GOME2 Level-1B file");
  }

  // Return

  return rc;
}

RC Gome2ReadSunRef(GOME2_ORBIT_FILE *pOrbitFile) {
  double gome2SunWve[NCHANNEL][NPIXEL_MAX];                                     // wavelength
  double gome2SunRef[NCHANNEL][NPIXEL_MAX];                                     // irradiance

  RC rc=ERROR_ID_NO;

  // Allocate buffers
  const int n_wavel = pOrbitFile->gome2Info.no_of_pixels;
  if (((pOrbitFile->gome2SunWve= (double *) MEMORY_AllocDVector(__func__,"gome2SunWve",0,n_wavel))==NULL) ||
      ((pOrbitFile->gome2SunRef= (double *) MEMORY_AllocDVector(__func__,"gome2SunRef",0,n_wavel))==NULL))

    return ERROR_ID_ALLOC;

  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"VIADR_SMR");            // VIADR_SMR (Variable Internal Auxiliary Data Record - Sun Mean Reference)
  coda_cursor_goto_first_array_element(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"LAMBDA_SMR");           // VIADR_SMR.LAMBDA_SMR

  coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&gome2SunWve[0][0],coda_array_ordering_c);

  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // VIADR_SMR
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SMR");                  // VIADR_SMR.LAMBDA_SMR

  coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&gome2SunRef[0][0],coda_array_ordering_c);

  for (int i=0; i<n_wavel; i++) {
    const size_t channel= pOrbitFile->gome2Info.channelIndex;
    pOrbitFile->gome2SunWve[i]=gome2SunWve[channel][i];
    pOrbitFile->gome2SunRef[i]=gome2SunRef[channel][i];
  }
  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      Gome2ReadOrbitInfo
// -----------------------------------------------------------------------------
// PURPOSE       Read general information on the current orbit
//
// INPUT         productFile  pointer to the current product file
//               bandIndex    the user selected band
//
// OUTPUT        pGome2Info   information on the current orbit
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC Gome2ReadOrbitInfo(GOME2_ORBIT_FILE *pOrbitFile,int bandIndex) {

  uint8_t  channel_index[NBAND];
  uint16_t no_of_pixels[NBAND];

  double start_lambda[NBAND];
  double end_lambda[NBAND];

  GOME2_INFO *pGome2Info=&pOrbitFile->gome2Info;
  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
  RC rc=ERROR_ID_NO;

  // Retrieve the MPHR
  int status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"MPHR");          // MPHR
  
  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"ORBIT_START");    // MPHR.orbitStart
  
  if (!status) status = coda_cursor_read_uint32(&pOrbitFile->gome2Cursor, &pGome2Info->orbitStart);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"ORBIT_END");     // MPHR.ORBIT_END
  if (!status) status = coda_cursor_read_uint32(&pOrbitFile->gome2Cursor, &pGome2Info->orbitEnd);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"TOTAL_VIADR");    // MPHR.TOTAL_VIADR (Variable Internal Auxiliary Data Record)
  if (!status) status = coda_cursor_read_uint32(&pOrbitFile->gome2Cursor, &pGome2Info->total_viadr);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"TOTAL_MDR");     // MPHR.TOTAL_MDR (Measurement Data Record)
  if (!status) status = coda_cursor_read_uint32(&pOrbitFile->gome2Cursor, &pGome2Info->total_mdr);
  if (!status) status = coda_cursor_goto_root(&pOrbitFile->gome2Cursor);

  // Retrieve GIADR_Bands

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"GIADR_Bands");

  if (!status) status = coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,0);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"CHANNEL_NUMBER");
  if (!status) status = coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor, channel_index, coda_array_ordering_c);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"NUMBER_OF_PIXELS");
  if (!status) status = coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor, no_of_pixels, coda_array_ordering_c);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"START_LAMBDA");
  if (!status) status = coda_cursor_read_double_array(&pOrbitFile->gome2Cursor, start_lambda, coda_array_ordering_c);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"END_LAMBDA");
  if (!status) status = coda_cursor_read_double_array(&pOrbitFile->gome2Cursor, end_lambda, coda_array_ordering_c);
  if (!status) status = coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  if (!status) status = coda_cursor_goto_root(&pOrbitFile->gome2Cursor);

  // Output
  if (status) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT, pOrbitFile->gome2FileName);
  }

  pGome2Info->channelIndex=channel_index[bandIndex]-1;
  pGome2Info->no_of_pixels = no_of_pixels[bandIndex];
  pGome2Info->start_lambda = start_lambda[bandIndex];
  pGome2Info->end_lambda = end_lambda[bandIndex];

  if ((pGome2Info->total_mdr<=0) ||
      ((pGome2Info->mdr= (GOME2_MDR *) MEMORY_AllocBuffer("Gome2ReadOrbitInfo","MDR",pGome2Info->total_mdr,sizeof(GOME2_MDR),0,MEMORY_TYPE_STRUCT)) ==NULL))

    rc=ERROR_ID_ALLOC;

  return rc;
}

int Gome2ReadMDRInfo(GOME2_ORBIT_FILE *pOrbitFile,GOME2_MDR *pMdr,int indexBand) {
  // Declarations

  uint8_t subclass;
  char start_time[40];
  double utc_start_double;
  int indexActual,i;
  char geoEarthActualString[25];

  // Generic record header

  coda_cursor_goto_available_union_field(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"RECORD_HEADER");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER

  // Subclass (determined by the instrument group - earthshine is expected)

  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"RECORD_SUBCLASS");        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER.RECORD_SUBCLASS
  coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&subclass);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER

  // Start time of the record

  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"RECORD_START_TIME");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER.RECORD_START_TIME
  read_utc_string(&pOrbitFile->gome2Cursor,start_time,&utc_start_double);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER

  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

  if (subclass==EARTHSHINE) {
    // Earthshine Wavelength grid for this band
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, gome2WavelengthField[indexBand]);
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor, pMdr->earthshine_wavelength, coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

    // Observation mode (NADIR is expected)

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"OBSERVATION_MODE");       // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.OBSERVATION_MODE
    coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->observationMode);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // number of records in each band

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"NUM_RECS");               // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.NUM_RECS
    coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor,pMdr->num_recs,coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // length of records in each band

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"REC_LENGTH");             // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.REC_LENGTH
    coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor,pMdr->rec_length,coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // flags

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"PCD_BASIC");              // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

    if (pOrbitFile->version<=11) {
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SAA");                  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SAA
      coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->saaFlag[0]);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SUNGLINT");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SUNGLINT
      coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->sunglintDangerFlag[0]);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_RAINBOW");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_RAINBOW
      coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->rainbowFlag[0]);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

      pMdr->sunglintHighDangerFlag[0]=0;

      memset(&pMdr->saaFlag[1],pMdr->saaFlag[0],N_SCAN_MAX-1);
      memset(&pMdr->sunglintDangerFlag[1],pMdr->sunglintDangerFlag[0],N_SCAN_MAX-1);
      memset(&pMdr->sunglintHighDangerFlag[1],pMdr->sunglintHighDangerFlag[0],N_SCAN_MAX-1);
      memset(&pMdr->rainbowFlag[1],pMdr->rainbowFlag[0],N_SCAN_MAX-1);
    } else {
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SAA");                  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SAA
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->saaFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SUNGLINT_RISK");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SUNGLINT_RISK
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->sunglintDangerFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SUNGLINT_HIGH_RISK");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SUNGLINT_HIGH_RISK
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->sunglintHighDangerFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_RAINBOW");              // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_RAINBOW
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->rainbowFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC
    }

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // GEO_BASIC: satellite coordinates and angles
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "GEO_BASIC");

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SUB_SATELLITE_POINT");
    beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor, pMdr->sat_lat, pMdr->sat_lon);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SATELLITE_ALTITUDE");
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor, pMdr->sat_alt, coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SOLAR_ZENITH_ANGLE");
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor, pMdr->sat_sza, coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SOLAR_AZIMUTH_ANGLE");
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor, pMdr->sat_saa, coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);  // MDR.GOME2_MDR_L1B_EARTHSHINE


    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "GEO_EARTH"); // MDR.GOME2_MDR_L1B_EARTHSHINE.GEO_EARTH
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "EARTH_RADIUS");
    coda_cursor_read_double(&pOrbitFile->gome2Cursor, &pMdr->earth_radius);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

    // Unique integration times in the scan

    if (pOrbitFile->version<=11) {
      for (unsigned int i=0; i<sizeof(pMdr->scanDirection) /sizeof(pMdr->scanDirection[0]); ++i) {
        pMdr->scanDirection[i]=255;                                                                  // Not defined
      }

      // Additional geolocation record for the actual integration time of the earthshine measurements

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "GEO_EARTH_ACTUAL");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "UNIQUE_INT");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.UNIQUE_INT
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,pMdr->unique_int,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // Unique int. time index for each channel

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "INT_INDEX");             // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.INT_INDEX
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->int_index,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // actual scanner angles

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SCANNER_ANGLE_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SCANNER_ANGLE_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->scanner_angle[0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 4 corner coordinates @ points ABCD

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"CORNER_ACTUAL");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CORNER_ACTUAL

      beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor,
          &pMdr->corner_lat[0][0][0],
          &pMdr->corner_lon[0][0][0]);

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // centre coordinate (point F)

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "CENTRE_ACTUAL");         // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CENTRE_ACTUAL

      beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor,
          &pMdr->centre_lat[0][0],
          &pMdr->centre_lon[0][0]);

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 SZAs @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SOLAR_ZENITH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->sza[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 Solar azimuth angles @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SOLAR_AZIMUTH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->saa[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 SZAs @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SAT_ZENITH_ACTUAL");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->lza[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 Solar azimuth angles @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SAT_AZIMUTH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->laa[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1
    } else {
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "UNIQUE_INT");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.UNIQUE_INT
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,pMdr->unique_int,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V5

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "INTEGRATION_TIMES");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.INTEGRATION_TIMES
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,pMdr->integration_times,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "GEO_REC_LENGTH");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.GEO_REC_LENGTH
      coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor,pMdr->geo_rec_length,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

      for (indexActual=0; indexActual<NBAND; indexActual++)
        if (fabs(pMdr->unique_int[indexActual]-pMdr->integration_times[indexBand]) < (double) 1.e-6)
          break;

      sprintf(geoEarthActualString,"GEO_EARTH_ACTUAL_%d",indexActual+1);
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,geoEarthActualString);            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.GEO_EARTH_ACTUAL

      for (i=0; i<pMdr->geo_rec_length[indexActual]; i++) {
        coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,i);

        // actual scanner angles

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SCAN_DIRECTION");        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SCAN_DIRECTION
        coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->scanDirection[i]);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // actual scanner angles

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SCANNER_ANGLE_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SCANNER_ANGLE_ACTUAL
        coda_cursor_read_double(&pOrbitFile->gome2Cursor,&pMdr->scanner_angle12[i]);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 4 corner coordinates @ points ABCD

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"CORNER_ACTUAL");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CORNER_ACTUAL

        beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor,
            &pMdr->corner_lat12[i][0],
            &pMdr->corner_lon12[i][0]);

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // centre coordinate (point F)

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "CENTRE_ACTUAL");         // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CENTRE_ACTUAL

        beat_cursor_read_geolocation_double_split(&pOrbitFile->gome2Cursor,
            &pMdr->centre_lat12[i],
            &pMdr->centre_lon12[i]);

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 SZAs @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SOLAR_ZENITH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->sza12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 Solar azimuth angles @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SOLAR_AZIMUTH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->saa12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 SZAs @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SAT_ZENITH_ACTUAL");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->lza12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 Solar azimuth angles @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SAT_AZIMUTH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->laa12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
      }

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
    }

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "CLOUD");               // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "FIT_MODE");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD.FIT_MODE
    coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,&pMdr->cloudFitMode[0],coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "FIT_1");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD.FIT_1
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->cloudTopPressure[0],coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "FIT_2");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD.FIT_2
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->cloudFraction[0],coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // Output

    pMdr->startTime=utc_start_double;
  }

  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR

  // Return

  return (subclass==EARTHSHINE) ?0:1;
}

RC Gome2BrowseMDR(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand) {

  GOME2_INFO *pGome2Info=&pOrbitFile->gome2Info;
  RC rc=ERROR_ID_NO;

  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"MDR");

  for (unsigned int i=0; i<pGome2Info->total_mdr; i++) {
    coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,i);
    GOME2_MDR *pMdr=&pGome2Info->mdr[pGome2Info->total_nadir_mdr];

    if (!(rc=Gome2ReadMDRInfo(pOrbitFile,pMdr,indexBand))) {
      pGome2Info->total_nadir_obs+=pMdr->num_recs[indexBand];
      pMdr->indexMDR=i;

      pGome2Info->total_nadir_mdr++;
    }

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
  }

  // Return

  return rc;
}

void Gome2ReadGeoloc(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand) {

  GOME2_INFO *pGome2Info=&pOrbitFile->gome2Info;

  for (int i=0; i<pGome2Info->total_nadir_obs; i++) {
    int mdrObs;
    int indexMDR=Gome2GetMDRIndex(pOrbitFile,indexBand,i,&mdrObs);
    int indexObs=i-mdrObs;

    GOME2_MDR *pMdr=&pGome2Info->mdr[indexMDR];

    struct gome2_geolocation *geolocation = &pOrbitFile->gome2Geolocations[i];

    if (pOrbitFile->version<=11) {
      int indexTint= (INDEX) pMdr->int_index[indexBand];

      // Solar zenith angles

      geolocation->solZen[0]= pMdr->sza[indexTint][0][indexObs];
      geolocation->solZen[1]= pMdr->sza[indexTint][1][indexObs];
      geolocation->solZen[2]= pMdr->sza[indexTint][2][indexObs];

      // Solar azimuth angles

      geolocation->solAzi[0]= pMdr->saa[indexTint][0][indexObs];
      geolocation->solAzi[1]= pMdr->saa[indexTint][1][indexObs];
      geolocation->solAzi[2]= pMdr->saa[indexTint][2][indexObs];

      // Line of sight viewing angles

      geolocation->losZen[0]= pMdr->lza[indexTint][0][indexObs];
      geolocation->losZen[1]= pMdr->lza[indexTint][1][indexObs];
      geolocation->losZen[2]= pMdr->lza[indexTint][2][indexObs];

      // viewing zenith angle in satellite coordinates (scanner angle)
      geolocation->sat_vza = pMdr->scanner_angle[indexTint][indexObs];

      // Line of sight azimuth angles

      geolocation->losAzi[0]= pMdr->laa[indexTint][0][indexObs];
      geolocation->losAzi[1]= pMdr->laa[indexTint][1][indexObs];
      geolocation->losAzi[2]= pMdr->laa[indexTint][2][indexObs];

      // Longitudes

      geolocation->lonCorners[0]=pMdr->corner_lon[indexTint][0][indexObs];
      geolocation->lonCorners[1]=pMdr->corner_lon[indexTint][1][indexObs];
      geolocation->lonCorners[2]=pMdr->corner_lon[indexTint][2][indexObs];
      geolocation->lonCorners[3]=pMdr->corner_lon[indexTint][3][indexObs];

      geolocation->lonCenter=pMdr->centre_lon[indexTint][indexObs];

      // Latitudes

      geolocation->latCorners[0]=pMdr->corner_lat[indexTint][0][indexObs];
      geolocation->latCorners[1]=pMdr->corner_lat[indexTint][1][indexObs];
      geolocation->latCorners[2]=pMdr->corner_lat[indexTint][2][indexObs];
      geolocation->latCorners[3]=pMdr->corner_lat[indexTint][3][indexObs];

      geolocation->latCenter=pMdr->centre_lat[indexTint][indexObs];
    } else {
      // Solar zenith angles

      geolocation->solZen[0]= pMdr->sza12[indexObs][0];
      geolocation->solZen[1]= pMdr->sza12[indexObs][1];
      geolocation->solZen[2]= pMdr->sza12[indexObs][2];

      // Solar azimuth angles

      geolocation->solAzi[0]= pMdr->saa12[indexObs][0];
      geolocation->solAzi[1]= pMdr->saa12[indexObs][1];
      geolocation->solAzi[2]= pMdr->saa12[indexObs][2];

      // Line of sight viewing angles

      geolocation->losZen[0]= pMdr->lza12[indexObs][0];
      geolocation->losZen[1]= pMdr->lza12[indexObs][1];
      geolocation->losZen[2]= pMdr->lza12[indexObs][2];

      // viewing zenith angle in satellite coordinates (scanner angle)
      geolocation->sat_vza = pMdr->scanner_angle12[indexObs];

      // Line of sight azimuth angles

      geolocation->losAzi[0]= pMdr->laa12[indexObs][0];
      geolocation->losAzi[1]= pMdr->laa12[indexObs][1];
      geolocation->losAzi[2]= pMdr->laa12[indexObs][2];

      // Longitudes

      geolocation->lonCorners[0]=pMdr->corner_lon12[indexObs][0];
      geolocation->lonCorners[1]=pMdr->corner_lon12[indexObs][1];
      geolocation->lonCorners[2]=pMdr->corner_lon12[indexObs][2];
      geolocation->lonCorners[3]=pMdr->corner_lon12[indexObs][3];

      geolocation->lonCenter=pMdr->centre_lon12[indexObs];

      // Latitudes

      geolocation->latCorners[0]=pMdr->corner_lat12[indexObs][0];
      geolocation->latCorners[1]=pMdr->corner_lat12[indexObs][1];
      geolocation->latCorners[2]=pMdr->corner_lat12[indexObs][2];
      geolocation->latCorners[3]=pMdr->corner_lat12[indexObs][3];

      geolocation->latCenter=pMdr->centre_lat12[indexObs];
    }

    geolocation->cloudTopPressure= (pMdr->cloudFitMode[indexObs]==0) ?pMdr->cloudTopPressure[indexObs]:-1.;
    geolocation->cloudFraction= (pMdr->cloudFitMode[indexObs]==0) ?pMdr->cloudFraction[indexObs]:-1.;

    geolocation->scanDirection= (int) pMdr->scanDirection[indexObs];
    geolocation->observationMode = pMdr->observationMode;
    geolocation->saaFlag= (int) pMdr->saaFlag[indexObs];
    geolocation->sunglintDangerFlag= (int) pMdr->sunglintDangerFlag[indexObs];
    geolocation->sunglintHighDangerFlag= (int) pMdr->sunglintHighDangerFlag[indexObs];
    geolocation->rainbowFlag= (int) pMdr->rainbowFlag[indexObs];

    geolocation->earth_radius = pMdr->earth_radius;

    // GEO_BASIC data are "calculated in granules of the shortest effective
    // integration time for the main channels (187.5 ms, 32 times per scan)"
    // therefore, we calculate the index corresponding to the actual integration
    // time as follows:
    const double *unique_int=pGome2Info->mdr[indexMDR].unique_int;
    const uint8_t *int_index=pGome2Info->mdr[indexMDR].int_index;
    const double tint= (pOrbitFile->version<=11) ?unique_int[int_index[indexBand]]:pGome2Info->mdr[indexMDR].integration_times[indexBand];
    const int index_actual = indexObs * floor(0.5 + tint / 0.1875);

    geolocation->sat_alt = pMdr->sat_alt[index_actual];
    geolocation->sat_lat = pMdr->sat_lat[index_actual];
    geolocation->sat_lon = pMdr->sat_lon[index_actual];
    geolocation->sat_sza = pMdr->sat_sza[index_actual];
    geolocation->sat_saa = pMdr->sat_saa[index_actual];

    if (geolocation->cloudFraction < -1.) {
      geolocation->cloudFraction=geolocation->cloudTopPressure= -1.;
    }
  }
}

// ===================
// ALLOCATION ROUTINES
// ===================

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by GOME2 readout routines
// -----------------------------------------------------------------------------

void GOME2_ReleaseBuffers(void) {
  // Declarations

  GOME2_ORBIT_FILE *pOrbitFile;
  INDEX gome2OrbitFileIndex;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE|DEBUG_FCTTYPE_MEM);
#endif

  for (gome2OrbitFileIndex=0; gome2OrbitFileIndex<gome2OrbitFilesN; gome2OrbitFileIndex++) {
    pOrbitFile=&gome2OrbitFiles[gome2OrbitFileIndex];

    if (pOrbitFile->gome2Info.mdr!=NULL)
      MEMORY_ReleaseBuffer(__func__,"MDR",pOrbitFile->gome2Info.mdr);
    if (pOrbitFile->gome2Geolocations!=NULL)
      MEMORY_ReleaseBuffer(__func__,"gome2Geolocations",pOrbitFile->gome2Geolocations);
    if (pOrbitFile->gome2SunRef!=NULL)
      MEMORY_ReleaseBuffer(__func__,"gome2SunRef",pOrbitFile->gome2SunRef);
    if (pOrbitFile->gome2SunWve!=NULL)
      MEMORY_ReleaseBuffer(__func__,"gome2SunWve",pOrbitFile->gome2SunWve);

    // Close the current file

    if (pOrbitFile->gome2Pf!=NULL) {
      coda_close(pOrbitFile->gome2Pf);
      pOrbitFile->gome2Pf=NULL;
    }
  }

  for (gome2OrbitFileIndex=0; gome2OrbitFileIndex<MAX_GOME2_FILES; gome2OrbitFileIndex++)
    memset(&gome2OrbitFiles[gome2OrbitFileIndex],0,sizeof(GOME2_ORBIT_FILE));

  gome2OrbitFilesN=0;
  gome2CurrentFileIndex=ITEM_NONE;

  free_vza_refs();

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif
}

// =======================
// GOME2 READ OUT ROUTINES
// =======================

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on useful data sets from the file and
//               load the irradiance spectrum measured at the specified channel
//
// INPUT/OUTPUT  pSpecInfo interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC GOME2_Set(ENGINE_CONTEXT *pEngineContext) {
#if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
  WIN32_FIND_DATA fileInfo,fileInfoSub;                                         // structure returned by FindFirstFile and FindNextFile APIs
  void *hDir,hDirSub;                                                           // handle to use with by FindFirstFile and FindNextFile APIs
#else
  struct dirent *fileInfo;
  DIR *hDir;
#endif

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
#endif

  RC rc=ERROR_ID_NO;

  int previous_file=gome2CurrentFileIndex;
  gome2CurrentFileIndex=ITEM_NONE;

  pEngineContext->recordNumber=0;

  if (!GOME2_beatLoaded) {
    coda_init();
    coda_set_option_perform_boundary_checks(0);
    GOME2_beatLoaded=1;
  }

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto) {

    // Close the previous file
    if (gome2OrbitFilesN && (previous_file!=ITEM_NONE) && (previous_file<gome2OrbitFilesN) &&
        (gome2OrbitFiles[previous_file].gome2Pf!=NULL)) {
      coda_close(gome2OrbitFiles[previous_file].gome2Pf);
      gome2OrbitFiles[previous_file].gome2Pf=NULL;
    }

    // Look for this file in the list of loaded files
    int indexFile=0;
    for (; indexFile<gome2OrbitFilesN; ++indexFile) {
      if (!strcasecmp(pEngineContext->fileInfo.fileName,gome2OrbitFiles[indexFile].gome2FileName)) // found it
        break;
    }

    if (indexFile<gome2OrbitFilesN) // found it
      gome2CurrentFileIndex=indexFile;
  }

  if (gome2CurrentFileIndex==ITEM_NONE) { // the file was not found amongst the previously opened files

    // Release old buffers
    GOME2_ReleaseBuffers();

    // In automatic reference mode, get the list of files to load
    if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto) {
      // if this file was not previously loaded, we are in this
      // directory for the first time, so we need to generate a new
      // reference
      gome2LoadReferenceFlag=1;

      // Get file path
      char filePath[MAX_STR_SHORT_LEN+1];
      strcpy(filePath,pEngineContext->fileInfo.fileName);

      // make "filePath" contain the directory path without the file
      // name:
      char *ptr = strrchr(filePath,PATH_SEP);
      if (ptr==NULL) {
        strcpy(filePath,".");
      } else {
        *ptr = '\0';
      }

      // Search for files of the same orbit
      for (hDir=opendir(filePath); (hDir!=NULL) && ((fileInfo=readdir(hDir)) !=NULL);) {
        sprintf(gome2OrbitFiles[gome2OrbitFilesN].gome2FileName,"%s/%s",filePath,fileInfo->d_name);
        if (STD_IsDir(gome2OrbitFiles[gome2OrbitFilesN].gome2FileName) == 0)
          gome2OrbitFilesN++;
      }
      if (hDir != NULL) closedir(hDir);

    } else { // single files:
      gome2OrbitFilesN=1;
      gome2CurrentFileIndex=0;
      strcpy(gome2OrbitFiles[0].gome2FileName,pEngineContext->fileInfo.fileName);
    }

    // Load files
    gome2TotalRecordNumber=0;
    for (int i=0; i<gome2OrbitFilesN; i++) {
      GOME2_ORBIT_FILE *pOrbitFile=&gome2OrbitFiles[i];

      pOrbitFile->gome2Pf=NULL;
      pOrbitFile->specNumber=0;

      // Open the file

      if (!(rc=Gome2Open(&pOrbitFile->gome2Pf,pOrbitFile->gome2FileName,&pOrbitFile->version))) {
        coda_cursor_set_product(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Pf);

        Gome2ReadOrbitInfo(pOrbitFile, (int) pEngineContext->project.instrumental.user);
        NDET[0] = pOrbitFile->gome2Info.no_of_pixels;
        Gome2BrowseMDR(pOrbitFile, (int) pEngineContext->project.instrumental.user);

        if ((pOrbitFile->specNumber= (THRD_browseType==THREAD_BROWSE_DARK) ?1:pOrbitFile->gome2Info.total_nadir_obs) >0) {
          if ( (pOrbitFile->gome2Geolocations= MEMORY_AllocBuffer(__func__,"geoloc",pOrbitFile->specNumber,sizeof(*pOrbitFile->gome2Geolocations),0,MEMORY_TYPE_STRUCT)) ==NULL )
            rc=ERROR_ID_ALLOC;
          else if (!(rc=Gome2ReadSunRef(pOrbitFile)))
            Gome2ReadGeoloc(pOrbitFile, (int) pEngineContext->project.instrumental.user);
        }

        if (pOrbitFile->gome2Pf!=NULL) {
          coda_close(pOrbitFile->gome2Pf);
          pOrbitFile->gome2Pf=NULL;
        }

        if (!strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->gome2FileName))
          gome2CurrentFileIndex=i;

        gome2TotalRecordNumber+=pOrbitFile->specNumber;

        pOrbitFile->rc=rc;
        rc=ERROR_ID_NO;
      }
    }
  }

  if (gome2CurrentFileIndex==ITEM_NONE)
    return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"gome2OrbitFiles");

  GOME2_ORBIT_FILE *pOrbitFile=&gome2OrbitFiles[gome2CurrentFileIndex];
  pEngineContext->recordNumber=pOrbitFile->specNumber;
  if (!pEngineContext->recordNumber)
    return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->gome2FileName);
  if (pOrbitFile->rc)
    return rc;

  if (pOrbitFile->gome2Pf==NULL)
    rc=Gome2Open(&pOrbitFile->gome2Pf,pEngineContext->fileInfo.fileName,&pOrbitFile->version);
  if (!rc) {
    coda_cursor_set_product(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Pf);
    const int n_wavel = pOrbitFile->gome2Info.no_of_pixels;
    memcpy(pEngineContext->buffers.lambda,pOrbitFile->gome2SunWve,sizeof(double) *n_wavel);
    memcpy(pEngineContext->buffers.lambda_irrad,pOrbitFile->gome2SunWve,sizeof(double) *n_wavel);
    memcpy(pEngineContext->buffers.irrad,pOrbitFile->gome2SunRef,sizeof(double) *n_wavel);
  }

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_Read
// -----------------------------------------------------------------------------
// PURPOSE       GOME2 level 1 data read out
//
// INPUT         recordNo     index of the record to read
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC GOME2_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex) {
  // Declarations

  double   *unique_int;                                                         // integration time
  uint8_t  *int_index;                                                          // index of the integration time
  double tint;
  double *spectrum,*sigma;                                                      // radiances and errors
  INDEX indexMDR;
  int mdrObs;
  INDEX indexBand;
  int year,month,day,hour,min,sec,musec;
  double utcTime;
  GOME2_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  GOME2_INFO *pGome2Info;
  RECORD_INFO *pRecord;
  RC rc;                                                                        // return code

  // Debug

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
#endif

  // Initializations

  pOrbitFile=&gome2OrbitFiles[(fileIndex==ITEM_NONE) ?gome2CurrentFileIndex:fileIndex];
  pGome2Info=&pOrbitFile->gome2Info;
  const int n_wavel = pGome2Info->no_of_pixels;
  spectrum=pEngineContext->buffers.spectrum;
  sigma=pEngineContext->buffers.sigmaSpec;
  indexBand= (INDEX) pEngineContext->project.instrumental.user;
  pRecord=&pEngineContext->recordInfo;

  rc=ERROR_ID_NO;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
    rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
    rc=ERROR_ID_FILE_END;
  else {
    for (int i=0; i<n_wavel; i++)
      spectrum[i]=sigma[i]= (double) 0.;

    if ((indexMDR=Gome2GetMDRIndex(pOrbitFile,indexBand,recordNo-1,&mdrObs)) ==ITEM_NONE)
      rc=ERROR_ID_FILE_RECORD;
    else {

      unique_int=pGome2Info->mdr[indexMDR].unique_int;
      int_index=pGome2Info->mdr[indexMDR].int_index;

      tint= (pOrbitFile->version<=11) ?unique_int[int_index[indexBand]]:pGome2Info->mdr[indexMDR].integration_times[indexBand];

      // Assign earthshine wavelength grid
      for (int i=0; i<n_wavel; ++i) {
        pEngineContext->buffers.lambda[i] = pGome2Info->mdr[indexMDR].earthshine_wavelength[i];
      }

      // read radiance & error ('RAD' and 'ERR_RAD')
      //
      // RAD and ERR_RAD fields are 'vsf_integers', consisting of a
      // 'value' and 'scale' integer pair.  CODA will automatically
      // convert these to one number value * 10^(-scale), but it's
      // slightly faster to disable this conversion and perform the
      // conversion to double ourselves
      int32_t radval;
      int16_t errval;
      int8_t radscale, errscale;
      // disable conversion of "special types" in order to access
      // value and scale integers separately:
      coda_set_option_bypass_special_types(1);

      Gome2GotoOBS(pOrbitFile, (INDEX) indexBand,indexMDR,recordNo-mdrObs-1);
      for (int i=0; i < pGome2Info->no_of_pixels && !rc; ++i) {
        coda_cursor_goto_first_record_field(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].RAD

        coda_cursor_goto_first_record_field(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].RAD.scale
        coda_cursor_read_int8(&pOrbitFile->gome2Cursor, &radscale);
        coda_cursor_goto_next_record_field(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].RAD.val
        coda_cursor_read_int32(&pOrbitFile->gome2Cursor, &radval);

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].RAD
        coda_cursor_goto_next_record_field(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].ERR_RAD
        coda_cursor_goto_first_record_field(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].ERR_RAD.scale
        coda_cursor_read_int8(&pOrbitFile->gome2Cursor, &errscale);
        coda_cursor_goto_next_record_field(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].ERR_RAD.val
        coda_cursor_read_int16(&pOrbitFile->gome2Cursor, &errval);

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].ERR_RAD
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i]

        spectrum[i] = ((double) radval) * pow(10, -radscale);
        sigma[i] = ((double) errval) * pow(10, -errscale);

        if (fabs(spectrum[i]) > (double) 1.e20)
          rc=ERROR_ID_FILE_RECORD;

        if (i < (pGome2Info->no_of_pixels - 1)) {     // without CODA bounds checking, this check is necessary to avoid memory corruption when jumping past the last element, according to S Niemeijer at S&T
          coda_cursor_goto_next_array_element(&pOrbitFile->gome2Cursor);    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i+1]
        }
      }
      // re-enable CODA handling of "special types"
      coda_set_option_bypass_special_types(0);

      utcTime=pGome2Info->mdr[indexMDR].startTime+tint* (recordNo-mdrObs-2);    // NOV 2011 : problem with integration time (FRESCO comparison)
      coda_double_to_datetime(utcTime,&year,&month,&day,&hour,&min,&sec,&musec);

      // Output information on the current record

      pRecord->present_datetime.thedate.da_day= (char) day;
      pRecord->present_datetime.thedate.da_mon= (char) month;
      pRecord->present_datetime.thedate.da_year=year;

      pRecord->present_datetime.thetime.ti_hour= (unsigned char) hour;
      pRecord->present_datetime.thetime.ti_min= (unsigned char) min;
      pRecord->present_datetime.thetime.ti_sec= (unsigned char) sec;

      pRecord->present_datetime.millis = -1;
      pRecord->present_datetime.microseconds = musec;

      // Geolocation

      struct gome2_geolocation *pGeoloc=&pOrbitFile->gome2Geolocations[recordNo-1];

      for (int i=0; i<3; ++i) {
        pRecord->gome2.solZen[i] = pGeoloc->solZen[i];
        pRecord->gome2.solAzi[i] = pGeoloc->solAzi[i];
        pRecord->gome2.losZen[i] = pGeoloc->losZen[i];
        pRecord->gome2.losAzi[i] = pGeoloc->losAzi[i];
      }

      pRecord->satellite.cloud_top_pressure=pGeoloc->cloudTopPressure;
      pRecord->satellite.cloud_fraction=pGeoloc->cloudFraction;

      pRecord->gome2.scanDirection=pGeoloc->scanDirection;
      pRecord->gome2.observationMode=pGeoloc->observationMode;
      pRecord->gome2.saaFlag=pGeoloc->saaFlag;
      pRecord->gome2.sunglintDangerFlag=pGeoloc->sunglintDangerFlag;
      pRecord->gome2.sunglintHighDangerFlag=pGeoloc->sunglintHighDangerFlag;
      pRecord->gome2.rainbowFlag=pGeoloc->rainbowFlag;

      // Miscellaneous data

      pRecord->latitude=pGeoloc->latCenter;
      pRecord->longitude=pGeoloc->lonCenter;

      pRecord->Zm=pGeoloc->solZen[1];
      pRecord->Azimuth=pGeoloc->solAzi[1];
      pRecord->zenithViewAngle=pGeoloc->losZen[1];
      pRecord->azimuthViewAngle=pGeoloc->losAzi[1];

      pRecord->satellite.earth_radius = pGeoloc->earth_radius;

      pRecord->satellite.altitude = pGeoloc->sat_alt;
      pRecord->satellite.latitude = pGeoloc->sat_lat;
      pRecord->satellite.longitude = pGeoloc->sat_lon;
      for (int i=0; i!=4; ++i) {
        pRecord->satellite.cornerlats[i] = pGeoloc->latCorners[i];
        pRecord->satellite.cornerlons[i] = pGeoloc->lonCorners[i];
      }
      pRecord->satellite.saa = pGeoloc->sat_saa;
      pRecord->satellite.sza = pGeoloc->sat_sza;
      pRecord->satellite.vza = pGeoloc->sat_vza;

      pRecord->Tint=tint;

      pRecord->TimeDec= (double) hour+min/60.+ (sec+musec*1.e-6) / (60.*60.);
      pRecord->Tm= (double) ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);

      pRecord->satellite.orbit_number= pOrbitFile->gome2Info.orbitStart;
      pRecord->gome2.mdrNumber = indexMDR;
      pRecord->gome2.observationIndex = (recordNo-mdrObs-1);

      if ((pEngineContext->project.spectra.cloudMin>=0.) &&
          (pEngineContext->project.spectra.cloudMax<=1.) &&
          fabs(pEngineContext->project.spectra.cloudMax-pEngineContext->project.spectra.cloudMin) >EPSILON &&
          fabs(pEngineContext->project.spectra.cloudMax-pEngineContext->project.spectra.cloudMin) <(1.-EPSILON) &&
          (pRecord->satellite.cloud_fraction < pEngineContext->project.spectra.cloudMin-EPSILON ||
           pRecord->satellite.cloud_fraction > pEngineContext->project.spectra.cloudMax+EPSILON) ) {
        rc=ERROR_ID_FILE_RECORD;
      }
    }
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// =============================
// AUTOMATIC REFERENCE SELECTION
// =============================

RC GOME2_get_vza_ref(double scanner_angle, int index_feno, FENO *feno) {
  const size_t bin = find_bin(fabs(scanner_angle));
  const struct reference *ref;
  if (bin == 0) {
    ref = ref_nadir[index_feno];
  } else {
    ref = (scanner_angle < 0.)
      ? refs_left_scan[index_feno][bin-1]
      : refs_right_scan[index_feno][bin-1];
  }

  if (!ref->n_spectra) { // no reference spectra were found for this bin
    const double vza_min = vza_bins[bin];
    const double vza_max = (1+bin) < NUM_VZA_BINS
      ? vza_bins[1+bin]
      : 90.0;
    // TODO: specify if it's a left or right-scan reference?
    return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_VZA_REF, vza_min, vza_max);
  }

  assert((size_t) feno->NDET == ref->n_wavel);

  feno->Shift=ref->shift;
  feno->Stretch=ref->stretch;
  feno->Stretch2=ref->stretch2;
  feno->refNormFact=ref->norm;
  feno->Decomp = 1;
  for (int i=0; i<feno->NDET; ++i) {
    feno->Sref[i] = ref->spectrum[i];
  }

  return ERROR_ID_NO;
}

static bool use_as_reference(const struct gome2_geolocation *record, const FENO *feno) {
  const double latDelta = fabs(feno->refLatMin - feno->refLatMax);
  const double lonDelta = fabs(feno->refLonMin - feno->refLonMax);
  const double cloudDelta = feno->cloudFractionMax-feno->cloudFractionMin;

   // SCIA longitudes are in range -180-180 -> convert to range 0-360.
  const double lon = (record->lonCenter > 0) ? record->lonCenter : 360.0+record->lonCenter;

  const bool match_lat = (latDelta <= EPSILON)
    || (record->latCenter >= feno->refLatMin && record->latCenter <= feno->refLatMax);
  const bool match_lon = (lonDelta <= EPSILON)
    || ( (feno->refLonMin < feno->refLonMax
        && lon >=feno->refLonMin && lon <= feno->refLonMax)
        ||
       (feno->refLonMin >= feno->refLonMax
        && (lon >= feno->refLonMin || lon <= feno->refLonMax) ) ); // if refLonMin > refLonMax, we have either lonMin < lon < 360, or 0 < lon < refLonMax
  const bool match_sza = (feno->refSZADelta <= EPSILON)
    || ( fabs(record->solZen[1] - feno->refSZA) <= feno->refSZADelta);
  const bool match_cloud = (cloudDelta <= EPSILON)
    || (record->cloudFraction >= feno->cloudFractionMin && record->cloudFraction <= feno->cloudFractionMax);

  return (record->scanDirection == 1) && match_lat && match_lon && match_sza && match_cloud;
}

// create a list of all spectra that match reference selection criteria for one or more analysis windows.
static int find_ref_spectra(struct ref_list *(*selected_spectra)[NUM_VZA_REFS], struct ref_list **list_handle) {
  // zero-initialize
  for (int i=0; i<NFeno; ++i) {
    for (size_t j=0; j<NUM_VZA_REFS; ++j) {
      selected_spectra[i][j] = NULL;
    }
  }
  *list_handle = NULL;

  // iterate over all orbit files in same directory
  for (int i=0; i<gome2OrbitFilesN; ++i) {
    GOME2_ORBIT_FILE *orbit=&gome2OrbitFiles[i];

    if (orbit->rc || !orbit->specNumber)
      continue;

    bool close_current_file = false;
    if (orbit->gome2Pf == NULL) { // open file if needed
      int rc = Gome2Open(&orbit->gome2Pf, orbit->gome2FileName, &orbit->version);
      if (rc)
        return rc;
      coda_cursor_set_product(&orbit->gome2Cursor,orbit->gome2Pf);
      close_current_file = true; // if we opened the file here, remember to close it again later.
    }

    // Browse spectra
    for(int j=0; j<orbit->specNumber; ++j) {
      // each spectrum can be used for multiple analysis windows, so
      // we use one copy, and share the pointer between the different
      // analysis windows. We initialize as NULL, it becomes not-null
      // as soon as it is used in one or more analysis windows:
      struct ref_spectrum *ref = NULL;
      const struct gome2_geolocation *record = &orbit->gome2Geolocations[j];

      // check if this spectrum satisfies constraints for one of the analysis windows:
      for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
        const FENO *pTabFeno = &TabFeno[0][analysis_window];
        const int n_wavel = pTabFeno->NDET;
        if (!pTabFeno->hidden
            && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
            && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
            && use_as_reference(record, pTabFeno) ) {

          if (ref == NULL) {
            // ref hasn't been initialized yet for another analysis window, so do that now:

            // read spectrum, exit if it cannot be used.
            int rc = GOME2_Read(&ENGINE_contextRef, j, i);
            if (rc == ERROR_ID_FILE_RECORD) {
              // ERROR_ID_FILE_RECORD is a non-fatal error to signal
              // that the current spectrum can not be used. => just
              // move to the next spectrum.
              goto next_spectrum;
            }
            if (rc != ERROR_ID_NO) {
              if (close_current_file) {
                coda_close(orbit->gome2Pf);
                orbit->gome2Pf=NULL;
              }
              return rc;
            }

            // if spectrum was read successfully, allocate structs to hold the data
            ref = malloc(sizeof(*ref));
            ref->lambda = malloc(n_wavel*sizeof(*ref->lambda));
            ref->spectrum = malloc(n_wavel*sizeof(*ref->spectrum));

            // store the new reference at the front of the linked list:
            struct ref_list *new = malloc(sizeof(*new));
            new->ref = ref;
            new->next = *list_handle;
            *list_handle = new;

            for (int i=0; i<n_wavel; ++i) {
              ref->lambda[i] = ENGINE_contextRef.buffers.lambda[i];
              ref->spectrum[i] = ENGINE_contextRef.buffers.spectrum[i];
            }
          }

          // store ref at the front of the list of selected references for this analysis window and vza bin.
          struct ref_list *list_item = malloc(sizeof(*list_item));
          list_item->ref = ref;
          const size_t bin = find_bin(fabs(record->sat_vza));

          // add the reference to the list for the right VZA bin:
          //
          // bins are identified by their offset:
          // 0 = nadir,
          // (1, NUM_VZA_BINS( = left scan
          // (NUM_VZA_BINS,2*NUM_VZA_BINS-1( = right scan
          const size_t vza_offset = (bin == 0 || record->sat_vza < 0.) ? bin : (NUM_VZA_BINS-1 + bin);
          list_item->next = selected_spectra[analysis_window][vza_offset];
          selected_spectra[analysis_window][vza_offset] = list_item;
        }
      }
    next_spectrum: ;
    }
    if (close_current_file) {
      coda_close(orbit->gome2Pf);
      orbit->gome2Pf=NULL;
    }
  }

  return ERROR_ID_NO;
}

// -----------------------------------------------------------------------------
// FUNCTION      Gome2NewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC Gome2NewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {
  // make a copy of the EngineContext structure to read reference data
  RC rc=EngineCopyContext(&ENGINE_contextRef,pEngineContext);

  if (ENGINE_contextRef.recordNumber==0)
    return ERROR_ID_ALLOC;

  // Allocate reference structures:
  initialize_vza_refs();

  // 1. look in all candidate orbit files (i.e. orbit files in same
  //    dir)

  // for each analysis window: selected spectra per VZA bin
  // the same spectrum can be used in multiple analysis windows.
  struct ref_list* (*selected_spectra)[NUM_VZA_REFS] = malloc(NFeno * sizeof(*selected_spectra));

  // list_handle: list of references to same set of spectra, used for
  // memory management.  In this list, each spectrum appears only once.
  struct ref_list *list_handle;

  rc = find_ref_spectra(selected_spectra, &list_handle);
  if (rc != ERROR_ID_NO)
    goto cleanup;

    // 2. average spectra per analysis window and per VZA bin
  for (int i=0;(i<NFeno) && (rc<THREAD_EVENT_STOP);i++) {
    FENO *pTabFeno=&TabFeno[0][i];

    if ((pTabFeno->hidden!=1) &&
        (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
        (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)) {

      for (size_t j=0; j<NUM_VZA_REFS; ++j) {
        if (selected_spectra[i][j] == NULL) {
          // We may not find references for every VZA bin/analysis
          // window.  At this point we just emit a warning (it's not a
          // problem until we *need* that during retrieval for that bin).
          const char* message = " for analysis window %s and VZA bin %zu";
          const int length = 1 + strlen(message) + strlen(pTabFeno->windowName) + strlen(TOSTRING(MAX_FENO));
          char *tmp = malloc(length);
          sprintf(tmp, message, pTabFeno->windowName, j); // TODO convert ref number back to bin for error message
          ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REFERENCE_SELECTION, tmp);
          free(tmp);
          continue;
        }
        struct reference *ref = &vza_refs[i][j];
        rc = average_ref_spectra(selected_spectra[i][j], pTabFeno->LambdaRef, pTabFeno->NDET, ref);
        if (rc != ERROR_ID_NO)
          goto cleanup;

        // align ref w.r.t irradiance reference:
        double sigma_shift, sigma_stretch, sigma_stretch2; // not used here...
        rc = ANALYSE_fit_shift_stretch(1, 0, pTabFeno->SrefEtalon, ref->spectrum,
                                       &ref->shift, &ref->stretch, &ref->stretch2, \
                                       &sigma_shift, &sigma_stretch, &sigma_stretch2);
      }
    }
  }

 cleanup:
  // 3. free lists created in step 1

  // for 'selected_spectra', we only free the 'gome_ref_list'
  // structures, the other components are pointers to spetra owned by
  // the list_handle structure:
  for (int i=0; i<NFeno; ++i) {
    for (size_t j=0; j<NUM_VZA_REFS; ++j) {
      free_ref_list(selected_spectra[i][j], FREE_LIST_ONLY);
    }
  }

  // for 'list_handle', we also free the gome_ref_spectrum* pointers,
  // and the double* pointers 'lambda' & 'spectrum':
  free_ref_list(list_handle, FREE_DATA);

  // finally, free our selected_spectra buffer itself.
  free(selected_spectra);

  return rc;
 }

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC GOME2_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {

  GOME2_ORBIT_FILE *pOrbitFile=&gome2OrbitFiles[gome2CurrentFileIndex];
  const int n_wavel = pOrbitFile->gome2Info.no_of_pixels;
  int saveFlag= pEngineContext->project.spectra.displayDataFlag;

  RC rc=pOrbitFile->rc;

  // don't continue when current file has an error, or if we are
  // working with automatic references and don't need to create a new
  // reference:
  if (rc || (pEngineContext->analysisRef.refAuto && ! gome2LoadReferenceFlag) )
    return rc;

  int useUsamp=0,useKurucz=0;

  // Browse analysis windows and load missing data

  for (int indexFeno=0; indexFeno<NFeno && !rc; indexFeno++) {
    FENO *pTabFeno=&TabFeno[0][indexFeno];
    pTabFeno->NDET=n_wavel;

    // Load calibration and reference spectra

    if (!pTabFeno->gomeRefFlag) { // use irradiance from L1B file
      memcpy(pTabFeno->LambdaRef,pOrbitFile->gome2SunWve,sizeof(double) *n_wavel);
      memcpy(pTabFeno->Sref,pOrbitFile->gome2SunRef,sizeof(double) *n_wavel);

      if (!TabFeno[0][indexFeno].hidden) {
        rc = VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"GOME2_LoadAnalysis (Reference) ");
        if (rc)
          goto EndGOME2_LoadAnalysis;

        memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double) *pTabFeno->NDET);
        pTabFeno->useEtalon=pTabFeno->displayRef=1;

        // Browse symbols

        for (int indexTabCross=0; indexTabCross<pTabFeno->NTabCross; indexTabCross++) {
          CROSS_REFERENCE *pTabCross=&pTabFeno->TabCross[indexTabCross];
          WRK_SYMBOL *pWrkSymbol=&WorkSpace[pTabCross->Comp];

          // Cross sections and predefined vectors

          if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
               ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                ((indexTabCross==pTabFeno->indexCommonResidual) ||
                 (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
              ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET)) !=ERROR_ID_NO))

            goto EndGOME2_LoadAnalysis;
        }

        // Gaps : rebuild subwindows on new wavelength scale

        doas_spectrum *new_range = spectrum_new();
        int DimL=0;
        for (int indexWindow = 0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++) {
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

        rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0);
        if (rc) goto EndGOME2_LoadAnalysis;
        if ( (!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz)) {
          rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag);
        }
        if (rc) goto EndGOME2_LoadAnalysis;
      }
    }

    memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double) *pTabFeno->NDET);
    memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double) *pTabFeno->NDET);

    useUsamp+=pTabFeno->useUsamp;
    useKurucz+=pTabFeno->useKurucz;
  }

  // Wavelength calibration alignment

  if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ)) {
    KURUCZ_Init(0,0);

    if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0)) !=ERROR_ID_NO))
      goto EndGOME2_LoadAnalysis;
  }

  // Build undersampling cross sections

  if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ)) {
    // ANALYSE_UsampLocalFree();

    if (((rc=ANALYSE_UsampLocalAlloc(0)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(0,0,0)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(1,ITEM_NONE,0)) !=ERROR_ID_NO))

      goto EndGOME2_LoadAnalysis;
  }

  // Automatic reference selection
  if (gome2LoadReferenceFlag)
    rc=Gome2NewRef(pEngineContext,responseHandle);
  if (!rc) gome2LoadReferenceFlag=0;

EndGOME2_LoadAnalysis:
  return rc;
}

// Get the date/time of the first observation in the current orbit.
void GOME2_get_orbit_date(int *year, int *month, int *day) {
  int hour, min, sec, musec;

  GOME2_MDR *curr_mdr = gome2OrbitFiles[gome2CurrentFileIndex].gome2Info.mdr;

  coda_double_to_datetime(curr_mdr->startTime,year,month,day,&hour,&min,&sec,&musec);
}
