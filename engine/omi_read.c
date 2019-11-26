
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
//  LIBRARIES
//
//  This module uses HDF-EOS (Hierarchical Data Format - Earth Observing System)
//  libraries based on HDF-4
//
//  ----------------------------------------------------------------------------

#include <dirent.h>
#include <stdio.h>

#include <hdf.h>
#include <HdfEosDef.h>
#include <mfhdf.h>

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
#include "stdfunc.h"
#include "output.h"

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
  short          *terrainHeight;
  unsigned short *groundPixelQualityFlags;
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
  int year, month, day;         // orbit date
  int number;                   // orbit number
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

static RC OmiOpen(struct omi_orbit_file *pOrbitFile,const char *swathName, const ENGINE_CONTEXT *pEngineContext);
static void omi_free_swath_data(struct omi_swath_earth *pSwath);
static void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda);
static void omi_make_double(int16 mantissa[], int8 exponent[], int32 n_wavel, double* result);
static void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] );
static RC omi_load_spectrum(int spec_type, int32 sw_id, int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, unsigned short *pixelQualityFlags);
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

static void omi_close_orbit_file(struct omi_orbit_file *pOrbitFile)
{
  if(pOrbitFile->sw_id != 0) {
    SWdetach(pOrbitFile->sw_id);
    pOrbitFile->sw_id = 0;
  }
  if(pOrbitFile->swf_id != 0) {
    SWclose(pOrbitFile->swf_id);
    pOrbitFile->swf_id = 0;
  }
}

static void omi_destroy_orbit_file(struct omi_orbit_file *pOrbitFile) {
  omi_close_orbit_file(pOrbitFile);

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
      ((pData->xtrackQualityFlags=(uint8_t *)MEMORY_AllocBuffer(__func__,"xtrackQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_STRING))==NULL))

    rc=ERROR_ID_ALLOC;

  // Return

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
  RC rc = ERROR_ID_NO;

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
  if (hDir != NULL)
    {
      while( (fileInfo=readdir(hDir)) && num_reference_orbit_files < MAX_OMI_FILES ) {
        if(fileInfo->d_name[0] !='.') // better to use 'if (fileInfo->d_type == DT_REG)' ?
          {
            reference_orbit_files[num_reference_orbit_files] = malloc(sizeof(struct omi_orbit_file));
            char *file_name = malloc(strlen(current_dir)+strlen(fileInfo->d_name) +2); //directory + path_sep + filename + trailing \0
            sprintf(file_name,"%s%c%s",current_dir,PATH_SEP,fileInfo->d_name);
            reference_orbit_files[num_reference_orbit_files]->omiFileName = file_name;
            reference_orbit_files[num_reference_orbit_files]->omiSwath = NULL;
            num_reference_orbit_files++;
          }
      }
      closedir(hDir);
    }
  else
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,"Omi automatic reference selection");

  return rc;
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

static void free_row_references(struct omi_ref_list *(*row_references)[NFeno][OMI_TOTAL_ROWS])
{
  for(int i=0; i<NFeno; i++) {
    for(int j=0; j<OMI_TOTAL_ROWS; j++) {
      struct omi_ref_list *tempref,*omi_ref = (*row_references)[i][j];
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
static RC find_matching_spectra(const ENGINE_CONTEXT *pEngineContext, struct omi_orbit_file *orbit_file, struct omi_ref_list *(*row_references)[NFeno][OMI_TOTAL_ROWS], struct omi_ref_spectrum **first)
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

              rc = omi_load_spectrum(OMI_SPEC_RAD, orbit_file->sw_id, measurement, row, orbit_file->nWavel, newref->wavelengths, newref->spectrum, newref->errors, NULL);

              if(rc)
                goto end_find_matching_spectra;
            }

            struct omi_ref_list *list_item = malloc(sizeof(struct omi_ref_list));
            list_item->reference = newref;
            list_item->next = (*row_references)[analysis_window][row];
            (*row_references)[analysis_window][row] = list_item; // add reference to the list of spectra for this analysis window/row
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

  double tempspectrum[nWavel];
  double temperrors[nWavel];
  double derivs[nWavel];

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
}

static RC setup_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle)
{
  // keep a NFeno*OMI_TOTAL_ROWS array of matching spectra for every detector row & analysis window
  struct omi_ref_list *(*row_references)[NFeno][OMI_TOTAL_ROWS] = malloc(NFeno * OMI_TOTAL_ROWS * sizeof(struct omi_ref_list*));
  for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
    for(int row = 0; row < OMI_TOTAL_ROWS; row++) {
      (*row_references)[analysis_window][row] = NULL;
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
    rc = OmiOpen(orbit_file,OMI_EarthSwaths[pEngineContext->project.instrumental.omi.spectralType], pEngineContext);
    if (rc)
      goto end_setup_automatic_reference;

    // add matching spectra in this orbit file to the lists row_references & ref_candidates
    find_matching_spectra(pEngineContext, orbit_file, row_references, &ref_candidates);

    // relevant data has been copied to ref_candidates, so we can free the swath data, and close the orbit file
    omi_free_swath_data(orbit_file->omiSwath);
    orbit_file->omiSwath = NULL;
    omi_close_orbit_file(reference_orbit_files[i]);
  }

  // take the average of the matching spectra for each detector row & analysis window:
  for(int row = 0; row < OMI_TOTAL_ROWS; row++) {
    if (pEngineContext->project.instrumental.use_row[row]) {
      automatic_reference_ok[row] = true; // initialize to true, set it to false if automatic reference fails for one or more analysis windows
      const int n_wavel = NDET[row];
      for(int analysis_window = 0; analysis_window < NFeno; analysis_window++) {
        FENO *pTabFeno = &TabFeno[row][analysis_window];
        if(pTabFeno->hidden || !pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC ) {
          continue;
        }

        struct omi_ref_list *reflist = (*row_references)[analysis_window][row];

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
  time_t time_epoch = STD_timegm(&start_tai);

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

static RC OmiGetSwathData(struct omi_orbit_file *pOrbitFile, const ENGINE_CONTEXT *pEngineContext)
{
  // Initializations
  struct omi_data *pData = &pOrbitFile->omiSwath->dataFields;
  RC rc=ERROR_ID_NO;

  struct omi_buffer swathdata[] =
    {
      {"MeasurementQualityFlags", pData->measurementQualityFlags},
      {"InstrumentConfigurationId", pData->instrumentConfigurationId},
      {"WavelengthReferenceColumn", pData->wavelengthReferenceColumn},
      {"Time",pData->time},
      {"SecondsInDay",pData->secondsInDay},
      {"SpacecraftLatitude", pData->spacecraftLatitude},
      {"SpacecraftLongitude", pData->spacecraftLongitude},
      {"SpacecraftAltitude", pData->spacecraftAltitude},
      {"Latitude", pData->latitude},
      {"Longitude", pData->longitude},
      {"SolarZenithAngle", pData->solarZenithAngle},
      {"SolarAzimuthAngle", pData->solarAzimuthAngle},
      {"ViewingZenithAngle", pData->viewingZenithAngle},
      {"ViewingAzimuthAngle", pData->viewingAzimuthAngle},
      {"TerrainHeight", pData->terrainHeight},
      {"GroundPixelQualityFlags", pData->groundPixelQualityFlags}
    };

  int32 start[] = {0,0};
  int32 edge[] =  {pOrbitFile->nMeasurements,pOrbitFile->nXtrack };
  intn swrc;
  for (unsigned int i=0; i<sizeof(swathdata)/sizeof(swathdata[0]); i++) {
    swrc = SWreadfield(pOrbitFile->sw_id, (char *) swathdata[i].buffername, start, NULL, edge, swathdata[i].bufferptr);
    if (swrc == FAIL) {
      rc = ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,swathdata[i].buffername,pOrbitFile->omiFileName,"Cannot read ", swathdata[i].buffername);
      break;
    }

    // Older OMI files do not have "XTrackQualityFlags", so reading them is optional (if the project isn't configured to use XTrackQualityFlags)
    pData->have_xtrack_quality_flags = false;
    int32 rank;
    int32 dims[2];
    int32 numbertype;
    char dimlist[520];
    swrc = SWfieldinfo(pOrbitFile->sw_id, (char *) "XTrackQualityFlags", &rank, dims, &numbertype, dimlist);
    if (swrc != FAIL) {
      swrc = SWreadfield(pOrbitFile->sw_id, (char *)"XTrackQualityFlags", start, NULL, edge, pData->xtrackQualityFlags);
      if (swrc != FAIL) {
        pData->have_xtrack_quality_flags = true;
      } else {
        rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "XTrackQualityFlags", pOrbitFile->omiFileName,
                           "Cannot read XTrackQualityFlags", "");
      }
    } else if (pEngineContext->project.instrumental.omi.xtrack_mode != XTRACKQF_IGNORE) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "XTrackQualityFlags", pOrbitFile->omiFileName,
                       "File does not contain XTrackQualityFlags", "");
    }
  }

  // normalize longitudes: should be in the range 0-360
  for (int i=0; i< (pOrbitFile->nMeasurements * pOrbitFile->nXtrack); i++) {
    if(pData->longitude[i] < 0.)
      pData->longitude[i] += 360.;
  }

  // Return

  return rc;
}

  static RC OmiOpen(struct omi_orbit_file *pOrbitFile,const char *swathName, const ENGINE_CONTEXT *pEngineContext)
{
  RC rc = ERROR_ID_NO;

  // Open the file
  int32 swf_id = SWopen(pOrbitFile->omiFileName, DFACC_READ);
  if (swf_id == FAIL) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,__func__,pOrbitFile->omiFileName,"SWopen");
    goto end_OmiOpen;
  }
  pOrbitFile->swf_id = swf_id;

  // Get a list of all swaths in the file:
  int32 strbufsize = 0;
  int nswath = SWinqswath(pOrbitFile->omiFileName, NULL, &strbufsize);
  if(nswath == FAIL) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "SWinqswath", pOrbitFile->omiFileName);
    goto end_OmiOpen;
  } else {
    char swathlist[strbufsize+1];
    SWinqswath(pOrbitFile->omiFileName, swathlist, &strbufsize);

    // Look for requested swath in the list, and extract the complete
    // name e.g. look for "Earth UV-1 Swath" and extract "Earth UV-1
    // Swath (60x159x4)"
    //
    // (the complete name is needed to open the swath with SWattach)
    char *swath_full_name = strstr(swathlist,swathName);
    if (swath_full_name == NULL) {
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,__func__,pOrbitFile->omiFileName,"find swath");
    goto end_OmiOpen;
    }
    char *end_name = strpbrk(swath_full_name,",");

    if (end_name != NULL)
      *(end_name) = '\0';

    int32 sw_id = SWattach(swf_id, swath_full_name); // attach the swath
    if (sw_id == FAIL) {
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,__func__,pOrbitFile->omiFileName,"SWattach");
      goto end_OmiOpen;
    }
    pOrbitFile->sw_id = sw_id;

    int32 dims[3];
    int32 rank;
    int32 numbertype;
    char dimlist[520]; // 520 is a safe maximum length, see HDF-EOS ref for SWfieldinfo()
    intn swrc = SWfieldinfo(sw_id, (char *) "RadianceMantissa",&rank,dims,&numbertype , dimlist);
    if(swrc == FAIL) {
      rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);
      goto end_OmiOpen;
    }

    pOrbitFile->nMeasurements=(long)dims[0];
    pOrbitFile->nXtrack=(long)dims[1];
    pOrbitFile->nWavel=(long)dims[2];

    pOrbitFile->specNumber=pOrbitFile->nMeasurements*pOrbitFile->nXtrack;
    if (!pOrbitFile->specNumber) {
      return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);
    }

    // Allocate data
    rc=OMI_AllocateSwath(&pOrbitFile->omiSwath,pOrbitFile->nMeasurements,pOrbitFile->nXtrack,pOrbitFile->nWavel);

    if (!rc) {
      // Retrieve information on records from Data fields and Geolocation fields
      rc=OmiGetSwathData(pOrbitFile, pEngineContext);
    }

    if (!rc) {
      // Read orbit number and date from HDF-EOS metadata
      rc=read_orbit_metadata(pOrbitFile);
    }
  }

 end_OmiOpen:
  return rc;
}

static RC OMI_LoadReference(int spectralType, const char *refFile, struct omi_ref **return_ref)
{
  struct omi_ref *pRef=&OMI_ref[omiRefFilesN];
  RC rc=ERROR_ID_NO;

  int32 swf_id = 0;
  int32 sw_id = 0;

  swf_id = SWopen((char *)refFile, DFACC_READ); // library header doesn't contain const modifier
  if (swf_id == FAIL) {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,refFile,"can't open file ","");
    goto end_loadreference;
  }
  sw_id = SWattach(swf_id, (char *) OMI_SunSwaths[spectralType]); // library header doesn't contain const modifier
  if (sw_id  == FAIL) {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,OMI_SunSwaths[spectralType],"swath not found in file ", refFile);
    goto end_loadreference;
  }

  const int32 n_xtrack = SWdiminfo(sw_id, (char *) NXTRACK);
  if (n_xtrack == FAIL) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, NXTRACK, "can't access dimension in file ", refFile);
    goto end_loadreference;
  }
  const int32 n_wavel = SWdiminfo(sw_id, (char *) NWAVEL);
  if (n_wavel == FAIL) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, NWAVEL, "can't access dimension in file ", refFile);
    goto end_loadreference;
  }

  OMI_AllocateReference(omiRefFilesN,n_xtrack,n_wavel);

  strcpy(pRef->omiRefFileName,refFile);
  pRef->nXtrack=n_xtrack;
  pRef->nWavel=n_wavel;

  for (int indexSpectrum=0; indexSpectrum < pRef->nXtrack; indexSpectrum++) {
    rc = omi_load_spectrum(OMI_SPEC_IRRAD, sw_id, 0, indexSpectrum, n_wavel,
                           pRef->omiRefLambda[indexSpectrum],
                           pRef->omiRefSpectrum[indexSpectrum],
                           pRef->omiRefSigma[indexSpectrum],
                           pRef->spectrum.pixelQualityFlags);
    if (rc)
      goto end_loadreference;
  }

  if (!rc) {
    ++omiRefFilesN;
    for(int i=0; i<n_xtrack; ++i) {
      NDET[i]=pRef->nWavel;
    }
    ANALYSE_swathSize=pRef->nXtrack;
    *return_ref = pRef;
  }

 end_loadreference:

  if(sw_id !=0)
    SWdetach(sw_id);
  if(swf_id !=0)
    SWclose(swf_id);

  return rc;
}

/*! read wavelengths, spectrum, and errors into the buffers lambda,
 * spectrum and sigma.  If any of these pointers is NULL, the
 * corresponding data is not read.
 */
static RC omi_load_spectrum(int spec_type, int32 sw_id, int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, unsigned short *pixelQualityFlags) {
  RC rc = ERROR_ID_NO;

  int16 *mantissa = malloc(n_wavel*sizeof(*mantissa));
  int16 *precisionmantissa = malloc(n_wavel*sizeof(*precisionmantissa));
  int8 *exponent = malloc(n_wavel*sizeof(*exponent));

  // names of the fields in omi hdf files.
  const char *s_mantissa = IRRADIANCE_MANTISSA;
  const char *s_precision_mantissa = IRRADIANCE_PRECISION_MANTISSA;
  const char *s_exponent = IRRADIANCE_EXPONENT;

  if (spec_type == OMI_SPEC_RAD) {
    s_mantissa = RADIANCE_MANTISSA;
    s_precision_mantissa = RADIANCE_PRECISION_MANTISSA;
    s_exponent = RADIANCE_EXPONENT;
  }

  int32 start[] = {measurement, track, 0};
  int32 edge[] = {1,1,0}; // read 1 measurement, 1 detector row
  intn swrc = 0;

  // read wavelengths:
  if(lambda != NULL) {
    // read reference column
    int16 refcol;
    swrc = SWreadfield(sw_id, (char *) REFERENCE_COLUMN, start, NULL, (int32[]) {1}, &refcol);
    // read 5 wavelength coefficients
    edge[2] = OMI_NUM_COEFFICIENTS;
    float32 wavelength_coeff[OMI_NUM_COEFFICIENTS];
    swrc |= SWreadfield(sw_id,(char *) WAVELENGTH_COEFFICIENT, start, NULL, edge, wavelength_coeff);

    if (swrc == FAIL) {
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "SWreadfield");
      goto end_load_spectrum;
    }
    // store wavelength in lambda
    omi_calculate_wavelengths(wavelength_coeff, refcol, n_wavel, lambda);
  }

  // (ir-)radiance mantissae, exponents & pixel quality have dimension (nMeasurement x nXtrack x nWavel)
  edge[2] = n_wavel;

  if(spectrum != NULL || sigma != NULL) {
    swrc |= SWreadfield(sw_id, (char *) s_exponent, start, NULL, edge, exponent);

    if(spectrum != NULL) {
      swrc |= SWreadfield(sw_id, (char *) s_mantissa, start, NULL, edge, mantissa);
      if(!swrc) {
        omi_make_double(mantissa, exponent, n_wavel, spectrum);
        omi_interpolate_errors(mantissa,n_wavel,lambda,spectrum);
      }
    }

    if(sigma != NULL) {
      swrc |= SWreadfield(sw_id, (char *) s_precision_mantissa, start, NULL, edge, precisionmantissa);
      if(!swrc) {
        omi_make_double(precisionmantissa, exponent, n_wavel, sigma);
        omi_interpolate_errors(precisionmantissa,n_wavel,lambda,sigma);
      }
    }
  }

  if(pixelQualityFlags != NULL)
    swrc |= SWreadfield(sw_id, (char *) PIXEL_QUALITY_FLAGS, start, NULL, edge, pixelQualityFlags);

  if(swrc) // error reading either mantissa/precision_mantissa/exponent/qualityflags:
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "SWreadfield");

 end_load_spectrum:
  free(mantissa);
  free(precisionmantissa);
  free(exponent);

  return rc;
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

static void omi_make_double(int16 mantissa[], int8 exponent[], int32 n_wavel, double* result) {
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
  omi_close_orbit_file(&current_orbit_file);

  current_orbit_file.omiFileName = malloc(strlen(pEngineContext->fileInfo.fileName)+1);
  strcpy(current_orbit_file.omiFileName,pEngineContext->fileInfo.fileName);
  current_orbit_file.specNumber=0;

  rc=OmiOpen(&current_orbit_file,OMI_EarthSwaths[pEngineContext->project.instrumental.omi.spectralType],pEngineContext);
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
    omi_close_orbit_file(&current_orbit_file);
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

  pRecord->omi.omiXtrackQF = pData->have_xtrack_quality_flags ? pData->xtrackQualityFlags[i_record] : QDOAS_FILL_USHORT;
  pRecord->omi.omiGroundPQF = pData->groundPixelQualityFlags[i_record];
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

RC OMI_read_earth(ENGINE_CONTEXT *pEngineContext,int recordNo)
{
  // Initializations
  const struct omi_orbit_file *pOrbitFile = &current_orbit_file; // pointer to the current orbit

  double *spectrum=pEngineContext->buffers.spectrum;
  double *sigma=pEngineContext->buffers.sigmaSpec;
  double *lambda=pEngineContext->buffers.lambda;
  RC rc=ERROR_ID_NO;

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
  rc= omi_load_spectrum(OMI_SPEC_RAD,
                        pOrbitFile->sw_id,
                        i_alongtrack,
                        i_crosstrack,
                        pOrbitFile->nWavel,
                        lambda,spectrum,sigma,
                        pEngineContext->recordInfo.omi.omiPixelQF);
  if (rc)
    return rc;

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

  return rc;
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

  omi_close_orbit_file(&current_orbit_file);

  num_reference_orbit_files = 0;

  omiRefFilesN=0; // the total number of files to browse in one shot
  omiTotalRecordNumber=0;
  omiSwathOld=ITEM_NONE;
}
