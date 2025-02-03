
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OMI interface
//  Name of module    :  omi_read.c
//  Program Language  :  C
//  Creation date     :  4 April 2007
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
//      thomasd@aeronomie.be                        info@stcorp.nl
//
//  ----------------------------------------------------------------------------
//  LIBRARIES
//
//  This module uses HDF-EOS (Hierarchical Data Format - Earth Observing System)
//  libraries based on HDF-4
//  UPDATE: 31/1/2023:  hdf-eos removed and replaced by coda reading routines
//
//  ----------------------------------------------------------------------------

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <hdf.h>
#include <mfhdf.h>
#include <coda.h>

#include "omi_read.h"

#include "engine_context.h"
#include "spline.h"
#include "mediate.h"
#include "engine.h"
#include "kurucz.h"
#include "analyse.h"
#include "vector.h"
#include "zenithal.h"
#include "winthrd.h"
#include "output.h"
#include "visual_c_compat.h"

#if defined(WIN32) && WIN32
#define timegm _mkgmtime
#endif

#define MAX_OMI_FILES 500

// Omi field names for readout routines:
#define REFERENCE_COLUMN "WavelengthReferenceColumn"
#define WAVELENGTH_COEFFICIENT "WavelengthCoefficient"
#define OMI_NUM_COEFFICIENTS 5 // 5 coefficients in wavelength polynomial
#define OMI_XTRACK_NOTUSED 255 // XTrackQualityFlags = 255 is used to label unused rows in the detector during special zoom mode.
#define RADIANCE_MANTISSA "RadianceMantissa"
#define RADIANCE_PRECISION_MANTISSA "RadiancePrecisionMantissa"
#define RADIANCE_EXPONENT "RadianceExponent"
#define IRRADIANCE_MANTISSA "IrradianceMantissa"
#define IRRADIANCE_PRECISION_MANTISSA "IrradiancePrecisionMantissa"
#define IRRADIANCE_EXPONENT "IrradianceExponent"
#define PIXEL_QUALITY_FLAGS "PixelQualityFlags"
#define NXTRACK "nXtrack"
#define NWAVEL "nWavel"

// ========================
// DEFINITION OF STRUCTURES
// ========================

enum _omiSwathType { OMI_SWATH_UV1, OMI_SWATH_UV2, OMI_SWATH_VIS, OMI_SWATH_MAX };
enum _omiVdataType { OMI_VDATA_GEO, OMI_VDATA_DATA, OMI_VDATA_ATTR, OMI_VDATA_MAX };
enum _omiSpecType  { OMI_SPEC_IRRAD, OMI_SPEC_RAD };

struct omi_buffer {
  const char *buffername;
  void *bufferptr;
};

// Geolocation fields

struct omi_data {
  double         *time;
  float          *secondsInDay;
  float          *spacecraftLatitude;
  float          *spacecraftLongitude;
  float          *spacecraftAltitude;
  float          *latitude;
  float          *longitude;
  float          *solarZenithAngle;
  float          *solarAzimuthAngle;
  float          *viewingZenithAngle;
  float          *viewingAzimuthAngle;
  int16        *temp_RadianceMantissa;
  int16        *temp_RadiancePrecisionMantissa;
  int8_t        *temp_RadianceExponent;
  short          *terrainHeight;
  unsigned short *groundPixelQualityFlags;
  unsigned short *pixelQualityFlags;
  float *spec_wavelengthcoeff;
  uint8_t        *xtrackQualityFlags;
  short *wavelengthReferenceColumn;
  unsigned short *measurementQualityFlags;
  unsigned char *instrumentConfigurationId;
  bool have_xtrack_quality_flags;

};

struct omi_spectrum {
  unsigned short *pixelQualityFlags;
  float   wavelengthCoefficient[5];
  float   wavelengthCoefficientPrecision[5];
};

struct omi_ref {
  double       **omiRefLambda;
  double       **omiRefSpectrum;
  double       **omiRefSigma;
  double        *omiRefFact;
  long           nXtrack,nWavel;
  struct omi_spectrum spectrum;
  int            year,cday;
  char         omiRefFileName[MAX_STR_LEN+1];
};

struct omi_swath_earth {
  struct omi_spectrum spectrum;
  struct omi_data dataFields;
};

struct omi_orbit_file { // description of an orbit
  char      *omiFileName;       // the name of the file with a part of the orbit
  struct omi_swath_earth *omiSwath;          // all information about the swath
  int        specNumber;
  int32      swf_id,            // hdfeos swath file id
    sw_id;                      // hdfeos swath id
  long       nMeasurements,
    nXtrack,                    // number of detector tracks (normally 60)
    nWavel;
  coda_cursor  cursor;
  coda_product *product;

  int year, month, day;         // orbit date
  int number;                   // orbit number
  char *swathpath;          ///Earth_UV_1_Swath or /Earth_UV_2_Swath or /Earth_VIS_Swath
};

/* Before calculating the automatic reference spectrum, we build a
 * list of all spectra matching the search criteria for one of the
 * analysis windows and one of the detector rows.  This is to avoid
 * reading the same spectrum twice when it is used in the reference
 * calculation for multiple analysis windows.
 */
struct omi_ref_spectrum {
  double *wavelengths;
  double *spectrum;
  double *errors;
  struct omi_orbit_file *orbit_file; // orbit file containing this spectrum
  struct omi_ref_spectrum *next; // next in the list;
  int measurement_number;
};

/* List of spectra to be used in the automatic reference calculation
 * for a single pair (analysiswindow, detector row).
 */
struct omi_ref_list {
  struct omi_ref_spectrum *reference;
  struct omi_ref_list *next;
};

const char *OMI_EarthSwaths[OMI_SWATH_MAX]={"Earth UV-1 Swath","Earth UV-2 Swath","Earth VIS Swath"};
const char *OMI_SunSwaths[OMI_SWATH_MAX]={"Sun Volume UV-1 Swath","Sun Volume UV-2 Swath","Sun Volume VIS Swath"};

// ================
// STATIC VARIABLES
// ================

static struct omi_orbit_file current_orbit_file;
static int omiRefFilesN=0; // the total number of files to browse in one shot
static int omiTotalRecordNumber=0;
static struct omi_ref OMI_ref[MAX_FENO]; // the number of reference spectra is limited to the maximum number of analysis windows in a project

static struct omi_orbit_file* reference_orbit_files[MAX_OMI_FILES]; // List of filenames for which the current automatic reference spectrum is valid. -> all spectra from the same day/same directory.
static int num_reference_orbit_files = 0;
static bool automatic_reference_ok[OMI_TOTAL_ROWS]; // array to keep track if automatic reference creation spectrum failed for one of the detector rows

int omiSwathOld=ITEM_NONE;

static RC OmiOpen(struct omi_orbit_file *pOrbitFile, const ENGINE_CONTEXT *pEngineContext);
static void omi_free_swath_data(struct omi_swath_earth *pSwath);
static void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda);
static void omi_make_double(int16 mantissa[], int8_t exponent[], int32 n_wavel, double* result);
static void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] );
static RC omi_load_spectrum(int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, unsigned short *pixelQualityFlags,int16 * spec_mantissa,int16 * spec_precisionmantissa, int8_t * spec_exponent, float * spec_wavelengthcoeff, long dim [], int16 * refcol);
static void average_spectrum(double *average, double *errors, const struct omi_ref_list *spectra, const double *wavelength_grid);
static RC read_orbit_metadata(struct omi_orbit_file *orbit);

// ===================
// ALLOCATION ROUTINES
// ===================

void OMI_ReleaseReference(void)
{
  // Declarations

  struct omi_ref *pRef;

  // Initialization

  for (int i=0;i<omiRefFilesN;i++) {
    pRef=&OMI_ref[i];

    if (pRef->omiRefLambda!=NULL)
      MEMORY_ReleaseDMatrix(__func__,"omiRefLambda",pRef->omiRefLambda,0,0);
    if (pRef->omiRefSpectrum!=NULL)
      MEMORY_ReleaseDMatrix(__func__,"omiRefSpectrum",pRef->omiRefSpectrum,0,0);
    if (pRef->omiRefFact!=NULL)
      MEMORY_ReleaseDVector(__func__,"omiRefSpectrumK",pRef->omiRefFact,0);
    if (pRef->omiRefSigma!=NULL)
      MEMORY_ReleaseDMatrix(__func__,"omiRefSigma",pRef->omiRefSigma,0,0);

    if (pRef->spectrum.pixelQualityFlags!=NULL)                                          // pixelquality
      MEMORY_ReleaseBuffer(__func__,"pRef->spectrum.pixelQualityFlags",pRef->spectrum.pixelQualityFlags);

    memset(pRef,0,sizeof(OMI_ref[i]));
  }

  omiRefFilesN=0;
}

static RC OMI_AllocateReference(INDEX indexRef,int nSpectra,int nPoints)
{
  // Declarations

  struct omi_ref *pRef;
  RC rc;

  // Initializations

  pRef=&OMI_ref[indexRef];
  rc=ERROR_ID_NO;

  if (((pRef->omiRefLambda=(double **)MEMORY_AllocDMatrix(__func__,"omiRefLambda",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefSpectrum=(double **)MEMORY_AllocDMatrix(__func__,"omiRefSpectrum",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefFact=(double *)MEMORY_AllocDVector(__func__,"omiRefFact",0,nSpectra-1))==NULL) ||
      ((pRef->omiRefSigma=(double **)MEMORY_AllocDMatrix(__func__,"omiRefSigma",0,nPoints-1,0,nSpectra-1))==NULL) ||

      ((pRef->spectrum.pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"pixelQualityFlags",nPoints,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL))

    rc=ERROR_ID_ALLOC;

  pRef->nXtrack=nSpectra;
  pRef->nWavel=nPoints;

  // Return

  return rc;
}

/*! Use a detector row or not, based on XTrackQualityFlags and the
 *  chosen settings in the analysis configuration.
 *
 * OMI XTrackQualityFlags description from OMI IODS_Vol_2_issue8:
 *
 *  Bit 0-2:
 *    0 = ok
 *    1 = affected by anomaly, don't use pixel
 *    2 = slightly affected, use with caution
 *    3 = affected, not optimally corrected, use with caution
 *    4 = affected, optimally corrected, use with caution
 *    5 = "not used"
 *    6 = "not used"
 *    7 = error during correction, don't use
 *  Bit 3: reserved for future use
 *  Bit 4 = possibly affected by wavelength shift
 *  Bit 5 = possibly affected by blockage
 *  Bit 6 = possibly affected by stray sunlight
 *  Bit 7 = possibly affected by stray earthshine
 *
 *  We provide three possible uses of XTrackQualityFlags in the analysis:
 *
 *  IGNORE: don't use xtrackqualityflags
 *  STRICT: only use pixels with flag 0 (unaffected)
 *  NONSTRICT: also use pixels which are slightly affected or corrected (flag 2,3 or 4)
 */
bool omi_use_track(int quality_flag, enum omi_xtrack_mode mode)
{
  bool result;
  if (quality_flag == OMI_XTRACK_NOTUSED) {
    result = false;
  } else {
    quality_flag &= 7; // reduce to bits 0-2
    switch(mode)
      {
      case XTRACKQF_IGNORE:
        result = true;
        break;
      case XTRACKQF_STRICT:
        result = (quality_flag == 0);
        break;
      case XTRACKQF_NONSTRICT:
        result = (quality_flag == 0 || quality_flag == 2 || quality_flag == 3 || quality_flag == 4);
        break;
      default:
        result = true;
      }
  }
  return result;
}

/*! \brief check if automatic reference creation was successful for this row */
bool omi_has_automatic_reference(int row)
{
  return automatic_reference_ok[row];
}

/*! \brief release the allocated buffers with swath attributes. */
static void omi_free_swath_data(struct omi_swath_earth *pSwath)
{
  if(pSwath != NULL) {
    struct omi_data *data = &pSwath->dataFields;
    struct omi_buffer omi_swath_buffers[] = {
      {"pixelQualityFlags",pSwath->spectrum.pixelQualityFlags},
      {"measurementQualityFlags",data->measurementQualityFlags},
      {"wavelengthReferenceColumn",data->wavelengthReferenceColumn},
      {"instrumentConfigurationId",data->instrumentConfigurationId},
      {"secondsInDay",data->secondsInDay},
      {"spacecraftLatitude",data->spacecraftLatitude},
      {"spacecraftLongitude",data->spacecraftLongitude},
      {"spacecraftAltitude",data->spacecraftAltitude},
      {"latitude",data->latitude},
      {"longitude",data->longitude},
      {"solarZenithAngle",data->solarZenithAngle},
      {"solarAzimuthAngle",data->solarAzimuthAngle},
      {"viewingZenithAngle",data->viewingZenithAngle},
      {"viewingAzimuthAngle",data->viewingAzimuthAngle},
      {"terrainHeight",data->terrainHeight},
      {"time",data->time},
      {"groundPixelQualityFlags",data->groundPixelQualityFlags},
      {"xtrackQualityFlags",data->xtrackQualityFlags},
      {"temp_RadianceMantissa"  ,      data->temp_RadianceMantissa},
      {"temp_RadiancePrecisionMantissa" ,       data->temp_RadiancePrecisionMantissa},
      {"temp_RadianceExponent",       data->temp_RadianceExponent},
      {"pixelQualityFlags",	data->pixelQualityFlags},
      {"spec_wavelengthcoeff",	data->spec_wavelengthcoeff}

    };
    for(unsigned int i=0; i<sizeof(omi_swath_buffers)/sizeof(omi_swath_buffers[0]); i++) {
      void *ptr = omi_swath_buffers[i].bufferptr;
      if (ptr != NULL)
        MEMORY_ReleaseBuffer(__func__, omi_swath_buffers[i].buffername, ptr);
    }
    free(pSwath);
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif
}


static void omi_destroy_orbit_file(struct omi_orbit_file *pOrbitFile) {
  /* omi_close_orbit_file(pOrbitFile); */

  free(pOrbitFile->omiFileName);
  pOrbitFile->omiFileName = NULL;

  if(pOrbitFile->omiSwath != NULL) {
    omi_free_swath_data(pOrbitFile->omiSwath);
    pOrbitFile->omiSwath = NULL;
  }
  free(pOrbitFile);
}

void OMI_TrackSelection(const char *omiTrackSelection,bool *use_row)
{
  // Declarations

  char str[256];
  int number1,number2,i,n,resetFlag,rangeFlag;

  // Initializations

  resetFlag=1;
  rangeFlag=0;
  n=0;

  number1=number2=-1;

  if (!strlen(omiTrackSelection))
    for (i=0;i<MAX_SWATHSIZE;i++)
      use_row[i]=true;
  else
    {
      for (const char *ptr=omiTrackSelection;(int)(ptr-omiTrackSelection)<=256;ptr++)
        {
          if (resetFlag)
            {
              memset(str,0,256);
              n=0;
              resetFlag=0;
            }

          if ((*ptr>='0') && (*ptr<='9'))
            str[n++]=*ptr;
          else if ((*ptr==':') || (*ptr=='-'))
            {
              number1=atoi(str);
              rangeFlag=1;
              resetFlag=1;
            }
          else if ((*ptr==',') || (*ptr==';') || (*ptr=='\0'))
            {
              number2=atoi(str);

              if (!rangeFlag)
                number1=number2;

              if ((number1>0) && (number1<MAX_SWATHSIZE) && (number2>0) && (number2<MAX_SWATHSIZE))
                for (i=number1-1;i<number2;i++)
                  use_row[i]=true;

              number1=number2=-1;
              rangeFlag=0;
              resetFlag=1;

              if (*ptr=='\0')
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_AllocateSwath
// -----------------------------------------------------------------------------
// PURPOSE       Allocated buffers to load OMI data
//
// INPUT         pSwath     the structures with the buffers to allocate
//               n_alongtrack
//               n_crosstrack
//               n_wavel    the number of wavelength bands in a spectrum
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer fails
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

static RC OMI_AllocateSwath(struct omi_swath_earth **swath, const int n_alongtrack, const int n_crosstrack, const int n_wavel)
{
  // Declarations

  struct omi_swath_earth *pSwath = malloc(sizeof(*pSwath));
  *swath = pSwath;

  struct omi_spectrum *pSpectrum = &pSwath->spectrum;  // spectrum in earth swath
  struct omi_data *pData = &pSwath->dataFields; // data on earth swath
  int nRecords = n_alongtrack*n_crosstrack;            // total number of spectra
  RC rc = ERROR_ID_NO;                          // Return code

  if (
      ((pSpectrum->pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"pixelQualityFlags",n_wavel,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pData->measurementQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"measurementQualityFlags",n_alongtrack,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pData->instrumentConfigurationId=(unsigned char*)MEMORY_AllocBuffer(__func__,"instrumentConfigurationId",n_alongtrack,sizeof(unsigned short),0,MEMORY_TYPE_UNKNOWN))==NULL) ||
      ((pData->wavelengthReferenceColumn=(short *)MEMORY_AllocBuffer(__func__,"wavelengthReferenceColumn",n_alongtrack,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pData->time=MEMORY_AllocBuffer(__func__,"time",n_alongtrack,sizeof(*pData->time),0,MEMORY_TYPE_DOUBLE))==NULL) ||
      ((pData->secondsInDay=(float *)MEMORY_AllocBuffer(__func__,"secondsInDay",n_alongtrack,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->spacecraftLatitude=(float *)MEMORY_AllocBuffer(__func__,"spacecraftLatitude",n_alongtrack,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->spacecraftLongitude=(float *)MEMORY_AllocBuffer(__func__,"spacecraftLongitude",n_alongtrack,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->spacecraftAltitude=(float *)MEMORY_AllocBuffer(__func__,"spacecraftAltitude",n_alongtrack,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->latitude=(float *)MEMORY_AllocBuffer(__func__,"latitude",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->longitude=(float *)MEMORY_AllocBuffer(__func__,"longitude",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->solarZenithAngle=(float *)MEMORY_AllocBuffer(__func__,"solarZenithAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->solarAzimuthAngle=(float *)MEMORY_AllocBuffer(__func__,"solarAzimuthAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->viewingZenithAngle=(float *)MEMORY_AllocBuffer(__func__,"viewingZenithAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->viewingAzimuthAngle=(float *)MEMORY_AllocBuffer(__func__,"viewingAzimuthAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->terrainHeight=(short *)MEMORY_AllocBuffer(__func__,"terrainHeight",nRecords,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pData->groundPixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"groundPixelQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pData->xtrackQualityFlags=(uint8_t *)MEMORY_AllocBuffer(__func__,"xtrackQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_STRING))==NULL) || ((pData->temp_RadianceMantissa=(int16 *)MEMORY_AllocBuffer(__func__,"temp_RadianceMantissa",nRecords*n_wavel,sizeof(int16),0,MEMORY_TYPE_SHORT))==NULL) || ((pData->temp_RadiancePrecisionMantissa=(int16 *)MEMORY_AllocBuffer(__func__,"temp_RadiancePrecisionMantissa",nRecords*n_wavel,sizeof(int16),0,MEMORY_TYPE_SHORT))==NULL) || ((pData->temp_RadianceExponent=(int8_t *)MEMORY_AllocBuffer(__func__,"temp_RadianceExponent",nRecords*n_wavel,sizeof(int8),0,MEMORY_TYPE_STRING))==NULL) ||   ((pData->pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"pixelQualityFlags",nRecords*n_wavel,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) || ((pData->spec_wavelengthcoeff=(float *)MEMORY_AllocBuffer(__func__,"spec_wavelengthcoeff",nRecords*OMI_NUM_COEFFICIENTS,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) )
    rc=ERROR_ID_ALLOC;


  return rc;
}

// Check if the current automatic reference spectrum is still valid for the requested file name
static bool valid_reference_file(char *spectrum_file)
{
  for (int i=0; i < num_reference_orbit_files; i++ ) {
    if(!strcasecmp(spectrum_file, reference_orbit_files[i]->omiFileName))
      return true;
  }
  return false;
}

// Create the list of all orbit files in the same directory as the given file.
static RC read_reference_orbit_files(const char *spectrum_file) {
  // clear old reference_orbit_files array
  for(int i=0; i< num_reference_orbit_files; i++) {
    omi_destroy_orbit_file(reference_orbit_files[i]);
    reference_orbit_files[i] = NULL;
  }
  num_reference_orbit_files = 0;

  // get list of orbit files in same directory as requested file
  char current_dir[MAX_STR_SHORT_LEN+1];
  strcpy(current_dir, spectrum_file);

  char *directory_end = strrchr(current_dir,PATH_SEP);

  if (directory_end == NULL) { // relative path without leading './' is used
    sprintf(current_dir,".%c",PATH_SEP);
  } else {
    *(directory_end) = '\0'; // terminate current_dir string after directory separator
  }

  struct dirent *fileInfo;
  DIR *hDir = opendir(current_dir);
  if (hDir == NULL){
    return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,"Omi automatic reference selection");
  }

  while( (fileInfo=readdir(hDir)) && num_reference_orbit_files < MAX_OMI_FILES ) {
    // Only use files with filename extension ".he4":
    const char* extension = strrchr(fileInfo->d_name, '.');
    if(strcmp(extension, ".he4")) {
      continue;
    }
    reference_orbit_files[num_reference_orbit_files] = malloc(sizeof(struct omi_orbit_file));
    char *file_name = malloc(strlen(current_dir)+strlen(fileInfo->d_name) +2); //directory + path_sep + filename + trailing \0
    sprintf(file_name,"%s%c%s",current_dir,PATH_SEP,fileInfo->d_name);
    reference_orbit_files[num_reference_orbit_files]->omiFileName = file_name;
    reference_orbit_files[num_reference_orbit_files]->omiSwath = NULL;
    num_reference_orbit_files++;
  }
  closedir(hDir);

  return ERROR_ID_NO;
}

// check if a given spectrum matches the criteria to use it in the automatic reference spectrum
static bool use_as_reference(struct omi_orbit_file *orbit_file, int recordnumber, FENO *pTabFeno, enum omi_xtrack_mode xtrack_mode) {
  float lon_min = pTabFeno->refLonMin;
  float lon_max = pTabFeno->refLonMax;
  float lat_min = pTabFeno->refLatMin;
  float lat_max = pTabFeno->refLatMax;
  float sza_min = pTabFeno->refSZA - pTabFeno->refSZADelta;
  float sza_max = pTabFeno->refSZA + pTabFeno->refSZADelta;

  float lon = orbit_file->omiSwath->dataFields.longitude[recordnumber];
  float lat = orbit_file->omiSwath->dataFields.latitude[recordnumber];
  float sza = orbit_file->omiSwath->dataFields.solarZenithAngle[recordnumber];

  int xTrackQF = orbit_file->omiSwath->dataFields.xtrackQualityFlags[recordnumber];

  bool use_row = omi_use_track(xTrackQF, xtrack_mode);

  // if a range (0.0,0.0) is chosen ( < EPSILON), we don't select based on this parameter
  bool use_lon = lon_max - lon_min > EPSILON;
  bool use_lat = lat_max - lat_min > EPSILON;
  bool use_sza = sza_max - sza_min > EPSILON;

  return use_row
    && ((lon_min <= lon && lon_max >= lon) || !use_lon)
    && ((lat_min <= lat && lat_max >= lat) || !use_lat)
    && ((sza_min <= sza && sza_max >= sza) || !use_sza);
}

static char *automatic_reference_info(struct omi_ref_list *spectra) {
  char *filename = spectra->reference->orbit_file->omiFileName;
  struct omi_ref_list *current = spectra;
  int length = strlen(filename);
  while(current != NULL) {
    // spectra in the list are ordered per file.  We want to list each filename only once.
    if (current->reference->orbit_file->omiFileName != filename) {
      filename = current->reference->orbit_file->omiFileName;
      length += strlen(filename + 5); // allocate memory for "\n# <filename>: "
    }
    length+= 6; // " current->reference->measurement_number"
    current = current->next;
  }
  char *result = malloc(length);

  current = spectra;
  filename = spectra->reference->orbit_file->omiFileName;
  strcpy(result,"# ");
  strcat(result,filename);
  strcat(result,":");
  while(current != NULL) {
    if (current->reference->orbit_file->omiFileName != filename) {
      filename = current->reference->orbit_file->omiFileName;
      strcat(result,"\n# ");
      strcat(result,filename);
      strcat(result,":");
    }
    char tempstring[10];
    sprintf(tempstring, " %d",current->reference->measurement_number);
    strcat(result,tempstring);
    current = current->next;
  }

  return result;
}

static void free_ref_candidates( struct omi_ref_spectrum *reflist) {
  while(reflist != NULL) {
    struct omi_ref_spectrum *temp = reflist->next;
    free(reflist->spectrum);
    free(reflist->errors);
    free(reflist->wavelengths);
    free(reflist);
    reflist = temp;
  }
}

static void free_row_references(struct omi_ref_list *(*row_references)[OMI_TOTAL_ROWS])
{
  for(int i=0; i<NFeno; i++) {
    for(int j=0; j<OMI_TOTAL_ROWS; j++) {
      struct omi_ref_list *tempref,*omi_ref = row_references[i][j];
      while(omi_ref != NULL) {
        tempref = omi_ref->next;
        free(omi_ref);
        omi_ref = tempref;
      }
    }
  }
  free(row_references);
}

/* Read the geolocation data of all spectra in an orbit file and store
 * spectra matching the search criteria for the automatic reference
 * spectrum for one or more analysis windows in a list.
 */
static RC find_matching_spectra(const ENGINE_CONTEXT *pEngineContext, struct omi_orbit_file *orbit_file, struct omi_ref_list *(*row_references)[OMI_TOTAL_ROWS], struct omi_ref_spectrum **first)
{
  RC rc = 0;

  enum omi_xtrack_mode xtrack_mode = pEngineContext->project.instrumental.omi.xtrack_mode;

  for (int measurement=0; measurement < orbit_file->nMeasurements; measurement++) {
    for(int row = 0; row < orbit_file->nXtrack; row++) {
      if (pEngineContext->project.instrumental.use_row[row]) {

        int recordnumber = measurement*orbit_file->nXtrack + row;
        struct omi_ref_spectrum *newref = NULL; // will be allocated and used only if the spectrum is used in the automatic reference calculation for one of the analysis windows.

        // loop over all analysis windows and look if the current
        // spectrum can be used in the automatic reference for any of
        // them.
        for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {

          FENO *pTabFeno = &TabFeno[row][analysis_window];
          if (!pTabFeno->hidden
              && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
              && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
              && use_as_reference(orbit_file,recordnumber,pTabFeno, xtrack_mode)) {

            // create new spectrum structure if needed.  If the spectrum is already used for another analysis window, we can reuse the existing spectrum.
            if(newref == NULL) {
              newref = malloc(sizeof (struct omi_ref_spectrum));
              newref->measurement_number = measurement;
              newref->next = *first;
              *first = newref; // add spectrum to the list of all spectra
              newref->orbit_file = orbit_file;
              newref->spectrum = malloc(orbit_file->nWavel * sizeof(*newref->spectrum));
              newref->errors = malloc(orbit_file->nWavel * sizeof(*newref->errors));
              newref->wavelengths = malloc(orbit_file->nWavel * sizeof(*newref->wavelengths));
              long dd[3]={orbit_file->nMeasurements,orbit_file->nXtrack,orbit_file->nWavel};
              rc = omi_load_spectrum(measurement, row, orbit_file->nWavel, newref->wavelengths, newref->spectrum, newref->errors, NULL,&orbit_file->omiSwath->dataFields.temp_RadianceMantissa[0],&orbit_file->omiSwath->dataFields.temp_RadiancePrecisionMantissa[0],&orbit_file->omiSwath->dataFields.temp_RadianceExponent[0],&orbit_file->omiSwath->dataFields.spec_wavelengthcoeff[0],dd,&orbit_file->omiSwath->dataFields.wavelengthReferenceColumn[0]);
              if(rc)
                goto end_find_matching_spectra;
            }
            struct omi_ref_list *list_item = malloc(sizeof(struct omi_ref_list));
            list_item->reference = newref;
            list_item->next = row_references[analysis_window][row];
            row_references[analysis_window][row] = list_item; // add reference to the list of spectra for this analysis window/row
          }
        }
      }
    }
  }

 end_find_matching_spectra:
  return rc;
}

/* Calculate the automatic reference spectrum by averaging all spectra
   in the list.  The error on the automatic reference (for each pixel)
   is calculated as 1/n_spectra * sqrt(sum (sigma_i)^2)
*/
static void average_spectrum( double *average, double *errors, const struct omi_ref_list *spectra, const double *wavelength_grid) {
  int nWavel = spectra->reference->orbit_file->nWavel;

  double *tempspectrum = malloc(nWavel * sizeof(*tempspectrum));
  double *temperrors = malloc(nWavel * sizeof(*temperrors));
  double *derivs = malloc(nWavel * sizeof(*derivs));

  for(int i=0; i<nWavel; i++) {
    average[i] = 0.;
    errors[i] = 0.;
  }

  int n_spectra = 0;
  for(const struct omi_ref_list *cur_spectrum = spectra; cur_spectrum != NULL; cur_spectrum = cur_spectrum->next, n_spectra++) {
    struct omi_ref_spectrum* reference = cur_spectrum->reference;

    // interpolate reference on wavelength_grid
    int rc = SPLINE_Deriv2(reference->wavelengths,reference->spectrum,derivs,nWavel,__func__);
    if (rc) break;
    SPLINE_Vector(reference->wavelengths,reference->spectrum,derivs,nWavel,wavelength_grid,tempspectrum,nWavel,SPLINE_CUBIC);

    // interpolate instrumental errors on wavelength_grid
    SPLINE_Vector(reference->wavelengths,reference->errors,NULL,nWavel,wavelength_grid,temperrors,nWavel,SPLINE_LINEAR);

    for(int i=0; i<nWavel; i++) {
      average[i] += tempspectrum[i];
      errors[i] += temperrors[i] * temperrors[i];
    }
  }

  for(int i=0; i<nWavel; i++) {
    average[i] /= n_spectra;
    errors[i] = sqrt(errors[i])/n_spectra; // std deviation of the average of n_spectra independent gaussian variables
  }
  free(tempspectrum);
  free(temperrors);
  free(derivs);
}

static RC setup_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle)
{
  // keep a NFeno*OMI_TOTAL_ROWS array of matching spectra for every detector row & analysis window
  struct omi_ref_list *(*row_references)[OMI_TOTAL_ROWS] = malloc(NFeno * OMI_TOTAL_ROWS * sizeof(struct omi_ref_list*));
  for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
    for(int row = 0; row < OMI_TOTAL_ROWS; row++) {
      row_references[analysis_window][row] = NULL;
    }
  }
  // list containing the actual data of the selected spectra
  struct omi_ref_spectrum *ref_candidates = NULL;
  // strings describing the selected spectra for each row/analysis window
  for(int row = 0; row < OMI_TOTAL_ROWS; row++)
    for(int analysis_window = 0; analysis_window < NFeno; analysis_window++) {
      free(TabFeno[row][analysis_window].ref_description);
      TabFeno[row][analysis_window].ref_description = NULL;
    }

  // create the list of all orbit files used for this automatic reference
  RC rc = read_reference_orbit_files(pEngineContext->fileInfo.fileName);
  if(rc)
    goto end_setup_automatic_reference;

  // open each reference orbit file; find & read matching spectra in the file
  for(int i = 0; i < num_reference_orbit_files; i++) {
    struct omi_orbit_file *orbit_file = reference_orbit_files[i];
    rc = OmiOpen(orbit_file, pEngineContext);
    if (rc)
      goto end_setup_automatic_reference;

    // add matching spectra in this orbit file to the lists row_references & ref_candidates
    find_matching_spectra(pEngineContext, orbit_file, row_references, &ref_candidates);

    // relevant data has been copied to ref_candidates, so we can free the swath data, and close the orbit file
    omi_free_swath_data(orbit_file->omiSwath);
    orbit_file->omiSwath = NULL;
    /* omi_close_orbit_file(reference_orbit_files[i]); */
  }

  // take the average of the matching spectra for each detector row & analysis window:
  for(int row = 0; row < OMI_TOTAL_ROWS; row++) {
    if (pEngineContext->project.instrumental.use_row[row]) {
      automatic_reference_ok[row] = true; // initialize to true, set it to false if automatic reference fails for one or more analysis windows
      const int n_wavel = NDET[row];
      for(int analysis_window = 0; analysis_window < NFeno; analysis_window++) {
        FENO *pTabFeno = &TabFeno[row][analysis_window];
        if(pTabFeno->hidden || (pTabFeno->refSpectrumSelectionMode!=ANLYS_REF_SELECTION_MODE_AUTOMATIC) ) {
          continue;
        }
        struct omi_ref_list *reflist = row_references[analysis_window][row];
        if(reflist != NULL) {
          average_spectrum(pTabFeno->Sref, pTabFeno->SrefSigma, reflist, pTabFeno->LambdaRef);
          VECTOR_NormalizeVector(pTabFeno->Sref-1,n_wavel,&pTabFeno->refNormFact, __func__);
          pTabFeno->ref_description = automatic_reference_info(reflist);
        } else{
          char errormessage[250];
          sprintf( errormessage, "Can not find reference spectra for row %d and analysis window_%s", row, pTabFeno->windowName);
          mediateResponseErrorMessage(__func__, errormessage, WarningEngineError, responseHandle);
          automatic_reference_ok[row] = false;
        }
      }
    }
  }
 end_setup_automatic_reference:
  // free row_references & ref_candidates
  free_row_references(row_references);
  free_ref_candidates(ref_candidates);
  return rc;
}

// Convert TAI number of seconds since 01/01/1993 00:00:00 to a UTC datetime.  We use the
// number of UTC seconds in the current day to get the number of UTC
// leap seconds we have to subtract from the TAI value.
static void tai_to_utc(double tai, float utc_seconds_in_day, struct tm *result, int *ms) {
  static struct tm start_tai = { .tm_sec = 0,
    .tm_min = 0,
    .tm_hour = 0,
    .tm_mday = 1,
    .tm_mon = 0, // month since jan (-> 0-11)
    .tm_year = 93, // year since 1900
    .tm_isdst = 0 };

  // get seconds since epoch of 1/1/1993 00:00:00 UTC
  time_t time_epoch = timegm(&start_tai);

  // for observations after the leap second (for example the last observation
  // from 2012/06/30, 23:59:60), the number of seconds in the UTC day can be
  // higher than 24*3600.
  int extrasecs = 0;
  if(utc_seconds_in_day >= (24*3600-1) ) {
    // take the integer number of seconds past 23:59:59
    extrasecs = floor(utc_seconds_in_day - (24*3600-1)) ;
  }
  utc_seconds_in_day -= extrasecs;
  tai -= extrasecs;

  // the number of leap seconds until now can be obtained from the difference
  // of the number of TAI seconds in this day and UTC  seconds in this day:
  // difference of tai and utc_seconds_in_day modulo 24*3600
  long int leap_seconds = lround(tai -utc_seconds_in_day) % (24*3600);
  if(leap_seconds < 0)
    leap_seconds += 24*3600;

  double utc_seconds = tai - leap_seconds; // subtract leap seconds to get UTC seconds since 1/1/1993
  long int i_seconds = floor(utc_seconds);

  // add integer number of "UTC seconds" since 1/1/1993 to the epoch time
  time_epoch += i_seconds;

  // convert back to a datetime
#ifndef _WIN32
  gmtime_r(&time_epoch, result);   // use the thread-safe approach where it's available
#else
  struct tm *time = gmtime(&time_epoch);
  *result = *time;
#endif

  // add seconds past 23:59:59
  result->tm_sec += extrasecs;

  if (ms != NULL) {
    *ms = (int)(1000*(utc_seconds - i_seconds));
  }

}

// -----------------------------------------------------------------------------
// FUNCTION      OmiGetSwathData
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve data of swath
//
// INPUT         pOrbitFile   pointer to the current orbit file
// OUTPUT        rc           return code
//
// RETURN        ERROR_ID_NO if the function succeeds
//               ERROR_ID_BEAT otherwise
// -----------------------------------------------------------------------------

static RC OmiGetSwathData(struct omi_orbit_file *pOrbitFile)
{
  // Initializations
  struct omi_data *pData = &pOrbitFile->omiSwath->dataFields;
  RC rc=ERROR_ID_NO;

  char ee[300];
  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/RadianceMantissa");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_int16_array(&pOrbitFile->cursor,&pData->temp_RadianceMantissa[0],coda_array_ordering_c);
  if (rc)
    return rc;
  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/RadianceExponent");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_int8_array(&pOrbitFile->cursor,&pData->temp_RadianceExponent[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/RadiancePrecisionMantissa");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_int16_array(&pOrbitFile->cursor,&pData->temp_RadiancePrecisionMantissa[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/ViewingAzimuthAngle");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->viewingAzimuthAngle[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/TerrainHeight");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_int16_array(&pOrbitFile->cursor,&pData->terrainHeight[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/ViewingZenithAngle");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->viewingZenithAngle[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/SolarAzimuthAngle");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->solarAzimuthAngle[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/SolarZenithAngle");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->solarZenithAngle[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/Longitude");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->longitude[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/Latitude");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->latitude[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/Time/Time");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_double_array(&pOrbitFile->cursor,&pData->time[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/GroundPixelQualityFlags");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_uint16_array(&pOrbitFile->cursor,&pData->groundPixelQualityFlags[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/XTrackQualityFlags");
  if(coda_cursor_goto(&pOrbitFile->cursor,ee)==0){
    pData->have_xtrack_quality_flags = true;
    rc = coda_cursor_read_uint8_array(&pOrbitFile->cursor,&pData->xtrackQualityFlags[0],coda_array_ordering_c);
    if (rc)
      return rc;
  } else {
    pData->have_xtrack_quality_flags = false;
  }

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/MeasurementQualityFlags/MeasurementQualityFlags");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_uint16_array(&pOrbitFile->cursor,&pData->measurementQualityFlags[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/WavelengthReferenceColumn/WavelengthReferenceColumn");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_int16_array(&pOrbitFile->cursor,&pData->wavelengthReferenceColumn[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/InstrumentConfigurationId/InstrumentConfigurationId");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_uint8_array(&pOrbitFile->cursor,&pData->instrumentConfigurationId[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/PixelQualityFlags");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_uint16_array(&pOrbitFile->cursor,&pData->pixelQualityFlags[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Data_Fields/WavelengthCoefficient");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->spec_wavelengthcoeff[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/SpacecraftAltitude/SpacecraftAltitude");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->spacecraftAltitude[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/SpacecraftLongitude/SpacecraftLongitude");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->spacecraftLongitude[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/SpacecraftLatitude/SpacecraftLatitude");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->spacecraftLatitude[0],coda_array_ordering_c);
  if (rc)
    return rc;

  sprintf(ee,"%s%s",pOrbitFile->swathpath,"Geolocation_Fields/SecondsInDay/SecondsInDay");
  rc = coda_cursor_goto(&pOrbitFile->cursor,ee);
  if (rc)
    return rc;
  rc = coda_cursor_read_float_array(&pOrbitFile->cursor,&pData->secondsInDay[0],coda_array_ordering_c);
  if (rc)
    return rc;

  // normalize longitudes: should be in the range 0-360
  for (int i=0; i< (pOrbitFile->nMeasurements * pOrbitFile->nXtrack); i++) {
    if(pData->longitude[i] < 0.)
      pData->longitude[i] += 360.;
  }

  return rc;
}


static RC OmiOpen(struct omi_orbit_file *pOrbitFile, const ENGINE_CONTEXT *pEngineContext)
{

  RC rc = ERROR_ID_NO;
  if (pEngineContext->project.instrumental.omi.spectralType== PRJCT_INSTR_OMI_TYPE_UV1){
    pOrbitFile->swathpath="/Earth_UV_1_Swath/";
  }
  else if (pEngineContext->project.instrumental.omi.spectralType== PRJCT_INSTR_OMI_TYPE_UV2){
    pOrbitFile->swathpath="/Earth_UV_2_Swath/";
  }
  else if (pEngineContext->project.instrumental.omi.spectralType== PRJCT_INSTR_OMI_TYPE_VIS){
    pOrbitFile->swathpath="/Earth_VIS_Swath/";
  }
  else{
    assert(0);
  }
  coda_init();
  coda_set_option_perform_boundary_checks(0);
  rc = coda_open(pOrbitFile->omiFileName,&pOrbitFile->product);
  if (rc != 0) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS,
                         pOrbitFile->omiFileName, "coda_open() failed", "");
  }

  coda_cursor_set_product(&pOrbitFile->cursor, pOrbitFile->product);
  long int dims[3];
  char str_sw[200];
  sprintf(str_sw, "%s%s", pOrbitFile->swathpath,"Data_Fields/RadianceMantissa");
  rc =coda_cursor_goto(&pOrbitFile->cursor,str_sw);
  if (rc != 0) {
    goto cleanup;
  }
  int nd=0;
  rc = coda_cursor_get_array_dim(&pOrbitFile->cursor,&nd,dims);
  if (rc != 0) {
    goto cleanup;
  }
  pOrbitFile->nMeasurements=(long)dims[0];
  pOrbitFile->nXtrack=(long)dims[1];
  pOrbitFile->nWavel=(long)dims[2];
  pOrbitFile->specNumber=pOrbitFile->nMeasurements*pOrbitFile->nXtrack;
  if (!pOrbitFile->specNumber) {
    return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);
  }
  rc=OMI_AllocateSwath(&pOrbitFile->omiSwath,pOrbitFile->nMeasurements,pOrbitFile->nXtrack,pOrbitFile->nWavel);
  if (!rc) {
    // Retrieve information on records from Data fields and Geolocation fields
    rc=OmiGetSwathData(pOrbitFile);
  }
  if (!rc) {
    // Read orbit number and date from HDF-EOS metadata
    rc=read_orbit_metadata(pOrbitFile);
  }

 cleanup:
  coda_close(pOrbitFile->product);
  coda_done();
  return rc;
}

static RC OMI_LoadReference(int spectralType, const char *refFile, struct omi_ref **return_ref)
{
  const char * irr, * irrprec, * irrexp, * pixelq , * wave, * refstr;
  if (spectralType== PRJCT_INSTR_OMI_TYPE_UV1){
    irr="/Sun_Volume_UV_1_Swath/Data_Fields/IrradianceMantissa";
    irrprec="/Sun_Volume_UV_1_Swath/Data_Fields/IrradiancePrecisionMantissa";
    irrexp="/Sun_Volume_UV_1_Swath/Data_Fields/IrradianceExponent";
    pixelq="/Sun_Volume_UV_1_Swath/Data_Fields/PixelQualityFlags";
    wave="/Sun_Volume_UV_1_Swath/Data_Fields/WavelengthCoefficient";
    refstr="/Sun_Volume_UV_1_Swath/Data_Fields/WavelengthReferenceColumn/WavelengthReferenceColumn";
  }
  else if (spectralType== PRJCT_INSTR_OMI_TYPE_UV2){
    irr="/Sun_Volume_UV_2_Swath/Data_Fields/IrradianceMantissa";
    irrprec="/Sun_Volume_UV_2_Swath/Data_Fields/IrradiancePrecisionMantissa";
    irrexp="/Sun_Volume_UV_2_Swath/Data_Fields/IrradianceExponent";
    pixelq="/Sun_Volume_UV_2_Swath/Data_Fields/PixelQualityFlags";
    wave="/Sun_Volume_UV_2_Swath/Data_Fields/WavelengthCoefficient";
    refstr="/Sun_Volume_UV_2_Swath/Data_Fields/WavelengthReferenceColumn/WavelengthReferenceColumn";
  }
  else if (spectralType== PRJCT_INSTR_OMI_TYPE_VIS){
    irr="/Sun_Volume_VIS_Swath/Data_Fields/IrradianceMantissa";
    irrprec="/Sun_Volume_VIS_Swath/Data_Fields/IrradiancePrecisionMantissa";
    irrexp="/Sun_Volume_VIS_Swath/Data_Fields/IrradianceExponent";
    pixelq="/Sun_Volume_VIS_Swath/Data_Fields/PixelQualityFlags";
    wave="/Sun_Volume_VIS_Swath/Data_Fields/WavelengthCoefficient";
    refstr="/Sun_Volume_VIS_Swath/Data_Fields/WavelengthReferenceColumn/WavelengthReferenceColumn";
  }
  else{
    assert(0);
  }
  coda_product *product=NULL;
  coda_cursor cursor;
  coda_init();
  coda_set_option_perform_boundary_checks(0);
  RC rc = coda_open(refFile, &product);

  if (rc)
    return rc;

  coda_cursor_set_product(&cursor, product);
  rc = coda_cursor_goto(&cursor,irr);
  if (rc)
    goto cleanup2;

  long dims[3];
  int nd;
  rc = coda_cursor_get_array_dim(&cursor,&nd,dims);
  if (rc)
    goto cleanup2;

  int16 *temp_Mantissa= malloc(dims[0]*dims[1]*dims[2]*sizeof(*temp_Mantissa));
  int16 *temp_PrecisionMantissa= malloc(dims[0]*dims[1]*dims[2]*sizeof(*temp_PrecisionMantissa));
  int8_t *temp_Exponent= malloc(dims[0]*dims[1]*dims[2]*sizeof(*temp_Exponent));
  unsigned short *temp_pixelq= malloc(dims[0]*dims[1]*dims[2]*sizeof(*temp_pixelq));
  float *temp_wave= malloc(dims[0]*dims[1]*OMI_NUM_COEFFICIENTS*sizeof(*temp_wave));
  int16_t *refcol= malloc(dims[0]*sizeof(*refcol));
  rc = coda_cursor_read_int16_array(&cursor,temp_Mantissa,coda_array_ordering_c);
  if (rc)
    goto cleanup1;

  rc=coda_cursor_goto(&cursor,irrprec);
  if (rc)
    goto cleanup1;

  rc=coda_cursor_read_int16_array(&cursor,temp_PrecisionMantissa,coda_array_ordering_c);
  if (rc)
    goto cleanup1;
  rc=coda_cursor_goto(&cursor,irrexp);
  if (rc)
    goto cleanup1;

  rc=coda_cursor_read_int8_array(&cursor,temp_Exponent,coda_array_ordering_c);
  if (rc)
    goto cleanup1;

  rc=coda_cursor_read_int16_array(&cursor,temp_PrecisionMantissa,coda_array_ordering_c);
  rc=coda_cursor_goto(&cursor,pixelq);
  if (rc)
    goto cleanup1;

  rc=coda_cursor_read_uint16_array(&cursor,temp_pixelq,coda_array_ordering_c);
  if (rc)
    goto cleanup1;

  rc=coda_cursor_goto(&cursor,wave);
  rc=coda_cursor_read_float_array(&cursor,temp_wave,coda_array_ordering_c);
  if (rc)
    goto cleanup1;

  if (rc)
    goto cleanup1;
  rc=coda_cursor_goto(&cursor,refstr);
  rc=coda_cursor_read_int16_array(&cursor,refcol,coda_array_ordering_c);
  if (rc)
    goto cleanup1;

  struct omi_ref *pRef=&OMI_ref[omiRefFilesN];
  const int32 n_xtrack = dims[1];
  const int32 n_wavel = dims[2];

  OMI_AllocateReference(omiRefFilesN,n_xtrack,n_wavel);

  strcpy(pRef->omiRefFileName,refFile);
  pRef->nXtrack=n_xtrack;
  pRef->nWavel=n_wavel;

  for (int indexSpectrum=0; indexSpectrum < pRef->nXtrack; indexSpectrum++) {

    rc = omi_load_spectrum(0, indexSpectrum, n_wavel,
                           pRef->omiRefLambda[indexSpectrum],
                           pRef->omiRefSpectrum[indexSpectrum],
                           pRef->omiRefSigma[indexSpectrum],
                           pRef->spectrum.pixelQualityFlags,temp_Mantissa,temp_PrecisionMantissa,temp_Exponent,temp_wave, dims,refcol);
  }

 cleanup1: // cleanup if we've allocated memory
  free(temp_PrecisionMantissa);
  free(temp_Exponent);
  free(temp_Mantissa);
  free(temp_pixelq);
  free(temp_wave);
  free(refcol);
 cleanup2: // cleanup when no memory was allocated
  coda_close(product);
  coda_done();

  if (!rc) {
    ++omiRefFilesN;
    for(int i=0; i<n_xtrack; ++i) {
      NDET[i]=pRef->nWavel;
    }
    ANALYSE_swathSize=pRef->nXtrack;
    *return_ref = pRef;
  }

  return rc;
}


/*! read wavelengths, spectrum, and errors into the buffers lambda,
 * spectrum and sigma.  If any of these pointers is NULL, the
 * corresponding data is not read.
 */
static RC omi_load_spectrum(int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, unsigned short *pixelQualityFlags,int16 * spec_mantissa,int16 * spec_precisionmantissa, int8_t * spec_exponent, float *wavecoeff, long dim [],int16 * refcoll) {
  int16 *mantissa = malloc(n_wavel*sizeof(*mantissa));
  int16 *precisionmantissa = malloc(n_wavel*sizeof(*precisionmantissa));
  int8_t *exponent = malloc(n_wavel*sizeof(*exponent));

  int ii_rad=measurement*dim[1]*dim[2]+ track*dim[2];
  int ii_wave=measurement*dim[1]*OMI_NUM_COEFFICIENTS+ track*OMI_NUM_COEFFICIENTS;

  // read wavelengths:
  if(lambda != NULL) {
    int16 refcol;
    refcol=refcoll[measurement];
    float32 wavelength_coeff[OMI_NUM_COEFFICIENTS];
    for (int j=0; j<OMI_NUM_COEFFICIENTS; j++) {
      wavelength_coeff[j]=wavecoeff[ii_wave+j];
    }

    // store wavelength in lambda
    omi_calculate_wavelengths(wavelength_coeff, refcol, n_wavel, lambda);

  }

  // (ir-)radiance mantissae, exponents & pixel quality have dimension (nMeasurement x nXtrack x nWavel)
  if(spectrum != NULL || sigma != NULL) {
    if(spectrum != NULL) {
      for (int j=0; j<n_wavel; j++) {

        mantissa[j]=spec_mantissa[ii_rad+j];
        exponent[j]=spec_exponent[ii_rad+j];
        assert(mantissa[j]==spec_mantissa[ii_rad+j]);
      }
      omi_make_double(mantissa, exponent, n_wavel, spectrum);
      omi_interpolate_errors(mantissa,n_wavel,lambda,spectrum);
    }
  }
  if(sigma != NULL) {
    for (int j=0; j<n_wavel; j++) {
      precisionmantissa[j]=spec_precisionmantissa[ii_rad+j];
      assert(precisionmantissa[j]==spec_precisionmantissa[ii_rad+j]);
    }
    omi_make_double(precisionmantissa, exponent, n_wavel, sigma);
    omi_interpolate_errors(precisionmantissa,n_wavel,lambda,sigma);
  }

  if(pixelQualityFlags != NULL){
    for (int j=0; j<n_wavel; j++) {
      //here can the filtering on pixelqualityflag be implemented.
      // pixelQualityFlags[j]=pixelq[ii_rad+j];
    }
  }
  free(mantissa);
  free(precisionmantissa);
  free(exponent);

  return ERROR_ID_NO;
}

static void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda) {
  int i;
  // OMI wavelengths provided as a degree 4 polynomial
  // evaluate lambda = c_4*x^4 +c_3*x^3 ... + c_0,
  //                 = (((c_4*x + c_3)*x + c_2)*x + c_1)*x + c_0.
  for (i=0; i<n_wavel; i++) {
    double x = (double) i-refcol;
    lambda[i] = 0.;
    int j;
    for (j=OMI_NUM_COEFFICIENTS-1; j>=0 ; j--) {
      /* printf("%f",wavelength_coeff[j]); */
      lambda[i] = lambda[i]*x + (double)wavelength_coeff[j];
    }
  }
}

static const double pows[] = {
  1.,
  10.0,
  100.0,
  1000.0,
  10000.0,
  100000.0,
  1000000.0,
  10000000.0,
  100000000.0,
  1000000000.0,
  10000000000.0,
  100000000000.0,
  1000000000000.0,
  10000000000000.0,
  100000000000000.0,
  1000000000000000.0,
  10000000000000000.0,
  100000000000000000.0};

static void omi_make_double(int16 mantissa[], int8_t exponent[], int32 n_wavel, double* result) {
  int i;
  for (i=0; i<n_wavel; i++) {
    if (exponent[i] < 0 || exponent[i] >= sizeof(pows)/sizeof(pows[0]) ) {
      result[i] = 1;
    } else {
      result[i] = (double)mantissa[i] * pows[exponent[i]];
    }
  }
}

static void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] ){

  for (int i=1; i<n_wavel -1; i++) {
    if(mantissa[i] == -32767) {
      double lambda = wavelengths[i];
      double lambda1 = wavelengths[i-1];
      double lambda2 = wavelengths[i+1];
      double y1 = y[i-1];
      double y2 = y[i+1];
      y[i] = (y2*(lambda-lambda1) + y1*(lambda2-lambda))/(lambda2-lambda1);
    }
  }
}

RC OMI_GetReference(int spectralType, const char *refFile, INDEX indexColumn, double *lambda, double *ref, double *refSigma, int *n_wavel_ref)
{
  RC rc=ERROR_ID_NO;

  struct omi_ref *pRef= NULL;

  // Browse existing references
  for (int indexRef=0; indexRef<omiRefFilesN && (pRef == NULL); ++indexRef) {
    if (!strcasecmp(OMI_ref[indexRef].omiRefFileName,refFile))
      pRef = &OMI_ref[indexRef];
  }
  if (pRef == NULL) {
    // if not found, load the reference now
    rc = OMI_LoadReference(spectralType, refFile, &pRef);
    if (rc != ERROR_ID_NO)
      pRef = NULL;
  }

  if ((pRef != NULL) && (indexColumn>=0) && (indexColumn< pRef->nXtrack)) {
    memcpy(lambda,pRef->omiRefLambda[indexColumn],sizeof(double)*pRef->nWavel);
    memcpy(ref,pRef->omiRefSpectrum[indexColumn],sizeof(double)*pRef->nWavel);
    memcpy(refSigma,pRef->omiRefSigma[indexColumn],sizeof(double)*pRef->nWavel);
    *n_wavel_ref = pRef->nWavel;
  } else {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_OMI_REF,__func__);
  }

  return rc;
}

RC OMI_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle) {
  // if we need a new automatic reference, generate it
  if(pEngineContext->analysisRef.refAuto && !valid_reference_file(current_orbit_file.omiFileName)) {
    int rc = setup_automatic_reference(pEngineContext, responseHandle);
    if(rc) return rc;

    for(int i=0; i<ANALYSE_swathSize; i++) {
      if (pEngineContext->project.instrumental.use_row[i]) {
        // fit wavelength shift between calibrated solar irradiance
        // and automatic reference spectrum and apply this shift to
        // absorption crosssections
        rc = ANALYSE_AlignReference(pEngineContext,2,responseHandle,i);
        if (rc) return rc;
      }
    }
  }
  return ERROR_ID_NO;
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on useful data sets from the HDF file.
//
// INPUT/OUTPUT  pSpecInfo interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_HDF             if one or the access HDF functions failed;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC OMI_Set(ENGINE_CONTEXT *pEngineContext)
{
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
#endif


  // Initializations
  pEngineContext->recordNumber=0;
  omiSwathOld=ITEM_NONE;
  RC rc=ERROR_ID_NO;

  // Release old buffers and close file (if open)
  omi_free_swath_data(current_orbit_file.omiSwath);
  /* omi_close_orbit_file(&current_orbit_file); */


  current_orbit_file.omiFileName = malloc(strlen(pEngineContext->fileInfo.fileName)+1);
  strcpy(current_orbit_file.omiFileName,pEngineContext->fileInfo.fileName);
  current_orbit_file.specNumber=0;


  rc=OmiOpen(&current_orbit_file,pEngineContext);
  // Open the file


  if (!rc) {
    pEngineContext->recordNumber=current_orbit_file.specNumber;
    pEngineContext->n_alongtrack=current_orbit_file.nMeasurements;
    pEngineContext->n_crosstrack=current_orbit_file.nXtrack;

    omiTotalRecordNumber+=current_orbit_file.nMeasurements;

    for (int i=0; i<current_orbit_file.nXtrack; ++i) {
      NDET[i] = current_orbit_file.nWavel;
    }
  } else {
    omi_free_swath_data(current_orbit_file.omiSwath);
    /* omi_close_orbit_file(&current_orbit_file); */
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// transfer all OMI auxiliary data such as geolocation, time, ... to a
// RECORD_INFO structure
static void get_omi_record_data(RECORD_INFO *pRecord, const struct omi_orbit_file *orbit_file, int i_alongtrack, int i_crosstrack) {
  const struct omi_data *pData = &orbit_file->omiSwath->dataFields;

  const int i_record = i_alongtrack * orbit_file->nXtrack + i_crosstrack;

  pRecord->i_alongtrack=i_alongtrack;
  pRecord->i_crosstrack=i_crosstrack;

  pRecord->latitude=pData->latitude[i_record];
  pRecord->longitude=pData->longitude[i_record];
  pRecord->Zm=pData->solarZenithAngle[i_record];
  pRecord->Azimuth=pData->solarAzimuthAngle[i_record];
  pRecord->zenithViewAngle=pData->viewingZenithAngle[i_record];
  pRecord->azimuthViewAngle=pData->viewingAzimuthAngle[i_record];
  pRecord->useErrors=1;                                                     // Errors are available for OMI

  pRecord->xtrack_QF = pData->have_xtrack_quality_flags ? pData->xtrackQualityFlags[i_record] : QDOAS_FILL_USHORT;
  pRecord->ground_pixel_QF = pData->groundPixelQualityFlags[i_record];
  pRecord->omi.instrumentConfigurationId = pData->instrumentConfigurationId[i_alongtrack];

  pRecord->satellite.altitude = pData->spacecraftAltitude[i_alongtrack];
  pRecord->satellite.latitude = pData->spacecraftLatitude[i_alongtrack];
  pRecord->satellite.longitude = pData->spacecraftLongitude[i_alongtrack];

  pRecord->satellite.orbit_number = current_orbit_file.number;

  struct tm time_record;
  int omi_ms=0;
  // use TAI-93 "time" and UTC "secondsInDay" to get UTC date-time of current measurement:
  tai_to_utc(pData->time[i_alongtrack], pData->secondsInDay[i_alongtrack], &time_record, &omi_ms);

  struct date *pDate = &pRecord->present_datetime.thedate;
  struct time *pTime = &pRecord->present_datetime.thetime;

  pTime->ti_hour = (char)(time_record.tm_hour);
  pTime->ti_min = (char)(time_record.tm_min);
  pTime->ti_sec = (char)(time_record.tm_sec);

  pDate->da_year = time_record.tm_year + 1900;
  pDate->da_mon = (char)(time_record.tm_mon + 1);
  pDate->da_day = (char)(time_record.tm_mday);

  pRecord->present_datetime.millis = omi_ms;

  pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_read_earth
// -----------------------------------------------------------------------------
// PURPOSE       OMI level 1 earthshine read out
//
// INPUT         recordNo     index of the record to read
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC  OMI_read_earth(ENGINE_CONTEXT *pEngineContext,int recordNo)
{
  // Initializations
  const struct omi_orbit_file *pOrbitFile = &current_orbit_file; // pointer to the current orbit

  double *spectrum=pEngineContext->buffers.spectrum;
  double *sigma=pEngineContext->buffers.sigmaSpec;
  double *lambda=pEngineContext->buffers.lambda;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
    return ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
    return ERROR_ID_FILE_END;

  int i_alongtrack=(recordNo-1)/pOrbitFile->nXtrack;
  int i_crosstrack=(recordNo-1)%pOrbitFile->nXtrack;

  get_omi_record_data(&pEngineContext->recordInfo, pOrbitFile, i_alongtrack, i_crosstrack);

  if (!pEngineContext->project.instrumental.use_row[i_crosstrack]) {
    return ERROR_ID_FILE_RECORD;
  }
  long dd[3]={    pOrbitFile->nMeasurements, pOrbitFile->nXtrack,	  pOrbitFile->nWavel};

  omi_load_spectrum(i_alongtrack,
                    i_crosstrack,
                    pOrbitFile->nWavel,
                    lambda,spectrum,sigma,
                    pEngineContext->buffers.pixel_QF,&pOrbitFile->omiSwath->dataFields.temp_RadianceMantissa[0],&pOrbitFile->omiSwath->dataFields.temp_RadiancePrecisionMantissa[0],&pOrbitFile->omiSwath->dataFields.temp_RadianceExponent[0],&pOrbitFile->omiSwath->dataFields.spec_wavelengthcoeff[0],dd,&pOrbitFile->omiSwath->dataFields.wavelengthReferenceColumn[0]);

  // check L1 wavelength calibration
  // might be good to check that lambda covers the current analysis window, as well
  for (int i=1; i<pOrbitFile->nWavel; ++i) {
    // check lambda increases:
    if (lambda[i] <= lambda[i-1]) {
      return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_L1WAVELENGTH, recordNo);
    }
  }

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && omiRefFilesN) {
    if (omiSwathOld!=i_alongtrack) {
      KURUCZ_indexLine=1;
      omiSwathOld=i_alongtrack;
    }
    memcpy(pEngineContext->buffers.lambda_irrad,OMI_ref[0].omiRefLambda[i_crosstrack],sizeof(double)*pOrbitFile->nWavel);
    memcpy(pEngineContext->buffers.irrad,OMI_ref[0].omiRefSpectrum[i_crosstrack],sizeof(double)*pOrbitFile->nWavel);
  }

  return ERROR_ID_NO;
}

RC OMI_get_orbit_date(int *year, int *month, int *day) {
  *year = current_orbit_file.year;
  *month = current_orbit_file.month;
  *day = current_orbit_file.day;
  return ERROR_ID_NO;
}

/*! read the orbit number and start date from the HDF4 file attribute
  "CoreMetadata.0" */
static RC read_orbit_metadata(struct omi_orbit_file *orbit) {
  /* OMI files have metadata stored as a formatted text string in an
   * HDF SD attribute field. NASA provides the "SDP Toolkit" library
   * to read this metadata format.  However, we only need to read a
   * few fields, so we prefer to use an ad-hoc parsing method for this
   * attribute, instead of including another library in Qdoas.
   */
  orbit->year=orbit->month=orbit->day=orbit->number=0;
  RC rc = ERROR_ID_NO;

  int32 sd_id = SDstart(orbit->omiFileName, DFACC_READ);

  if(sd_id == FAIL) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS, orbit->omiFileName, "Can not open orbit file.", "");
    goto error;
  }
  const char attr_name[] = "CoreMetadata.0";
  int32 attr_index = SDfindattr(sd_id, attr_name);
  int32 data_type, n_values;
  char attr_name_out[sizeof(attr_name)]; // output parameter for SDattrinfo must be specified, even though we already know the attribute name...
  int32 status = SDattrinfo (sd_id, attr_index, attr_name_out, &data_type, &n_values);
  if(status == FAIL) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS, attr_name, "Can not read SD attribute info from file ", orbit->omiFileName);
    goto error;
  }

  char *metadata = malloc (1+n_values);
  status = SDreadattr (sd_id, attr_index, metadata);
  if(status == FAIL) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS, attr_name, "Can not read SD attribute data from file ", orbit->omiFileName);
    goto error_free;
  }
  /* metadata is a string containing all kinds of information.  We are
   * interested in the parts containing the orbit start date and orbit
   * number, which have the following format:
   *
   * OBJECT                 = RANGEBEGINNINGDATE
   *   NUM_VAL              = 1
   *   VALUE                = "2008-05-21"
   * END_OBJECT             = RANGEBEGINNINGDATE
   *
   * OBJECT                 = ORBITNUMBER
   *   CLASS                = "1"
   *   NUM_VAL              = 1
   *   VALUE                = (15945)
   *  END_OBJECT             = ORBITNUMBER
   *
   */

  int n_scan_date = 0;
  const char *str_date = strstr(metadata, "RANGEBEGINNINGDATE");
  str_date = str_date ? strstr(str_date, "VALUE") : NULL;
  if (str_date) {
    str_date +=5; // skip to end of "VALUE"
    n_scan_date = sscanf(str_date, " = \"%d-%02d-%02d\"", &orbit->year, &orbit->month, &orbit->day);
  }
  if (!str_date || n_scan_date != 3) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_HDFEOS, attr_name, "Failed to find orbit date of file ", orbit->omiFileName);
    goto error_free;
  }

  int n_scan_orbit = 0;
  const char *str_orbitnumber = strstr(metadata, "ORBITNUMBER");
  str_orbitnumber = str_orbitnumber ? strstr(str_orbitnumber, "VALUE") : NULL;
  if (str_orbitnumber) {
    str_orbitnumber += 5;  // skip to end of "VALUE"
    n_scan_orbit = sscanf(str_orbitnumber, " = (%d)", &orbit->number);
  }
  if (!str_orbitnumber || n_scan_orbit != 1) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_HDFEOS, attr_name, "Failed to find orbit number of file ", orbit->omiFileName);
    goto error_free;
  }

 error_free:
  free(metadata);
 error:
  SDend(sd_id);

  return rc;
}

void OMI_ReleaseBuffers(void) {

  for(int i=0; i< num_reference_orbit_files; i++) {

    omi_destroy_orbit_file(reference_orbit_files[i]);
    reference_orbit_files[i] = NULL;
  }

  /* omi_close_orbit_file(&current_orbit_file); */

  num_reference_orbit_files = 0;

  omiRefFilesN=0; // the total number of files to browse in one shot
  omiTotalRecordNumber=0;
  omiSwathOld=ITEM_NONE;
}
