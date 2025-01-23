//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  GOME interface (calibrated level 1 data files in the modified IASB-BIRA format)
//  Name of module    :  GDP_BIN_Read.C
//  Creation date     :  First versions exist since 1998 (GWinDOAS)
//  Modified          :  5 november 2002 (possibility to read GOME binary format with WinDOAS)
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
//  FUNCTIONS
//
//  ===================
//  ALLOCATION ROUTINES
//  ===================
//
//  GDP_BIN_ReleaseBuffers - release buffers allocated by GOME readout routines;
//
//  =================
//  READ OUT ROUTINES
//  =================
//
//  GdpBinLambda - build a wavelength calibration (irradiance or
//                 earthshine spectra) using a set of coefficients of polynomial;
//
//  GDP_BIN_Set - retrieve information on data saved in the file from the header;
//  GDP_BIN_Read - GOME calibrated level 1 data read out (binary format);
//
//  =============================
//  AUTOMATIC REFERENCE SELECTION
//  =============================
//
//  GdpBinBuildRef - build a reference spectrum by averaging a set of spectra
//                   matching latitudes and SZA conditions;
//
//  GdpBinRefSelection - selection of a reference spectrum in the current orbit;
//  GdpBinNewRef - in automatic reference selection, search for reference spectra;
//
//  ========
//  ANALYSIS
//  ========
//
//  GDP_BIN_LoadAnalysis - load analysis parameters depending on the irradiance spectrum;
//
//  ----------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>

#include "gdp_bin_read.h"

#include "comdefs.h"
#include "engine_context.h"
#include "kurucz.h"
#include "stdfunc.h"
#include "mediate.h"
#include "analyse.h"
#include "engine.h"
#include "vector.h"
#include "winthrd.h"
#include "zenithal.h"
#include "output.h"
#include "ref_list.h"
#include "visual_c_compat.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define OFFSET 500.0
#define PARAMETER 5000.0
#define SCIENCE_DATA_DEFINED          10                                        // number of bands
#define SPECTRAL_FITT_ORDER            5                                        // degree of polynomial used for building wavelength scale
#define MAX_FITT_ORDER                    6
#define GDP_BIN_ERROR_ID_MASK       0x01

// reference for each pixel type: east, center, west, backscan
#define NUM_VZA_REFS 4

// references for each pixel type and for each analysis window;
static struct reference (*vza_refs)[NUM_VZA_REFS];

// =====================
// STRUCTURES DEFINITION
// =====================

#pragma pack(push,1)
// Structures read from binary files:

// Date in short format
typedef struct _SHORTDateTime
 {
  char  da_year;        /* Year - 1900      */
  char  da_day;         /* Day of the month */
  char  da_mon;         /* Month (1 = Jan)  */
  char  ti_hour;
  char  ti_min;
  char  ti_sec;
 }
SHORT_DATETIME;

// File header
typedef struct _gomeFileHeader
 {
  unsigned short nspectra;                    // total number of spectra in file
  char           version;
  char           mask;
  int            headerSize;                  // number of bytes before first record
  int            recordSize;                  // size of a record
  int            orbitNumber;                 // orbit number
  short          nbands;                      // number of available bands
  char           nSpectralParam;              // number of set of spectral parameters
  char           indexSpectralParam;          // index of set of spectral parameters for irradiance spectra
  SHORT_DATETIME dateAndTime;                 // measurement date and time in UT
 }
GDP_BIN_FILE_HEADER;

// For each available band, provide header with band and reference general info

typedef struct {
  // Band info

  char       bandType;                        // band type
  short      bandSize;                        // band size

  // Reference info

  short      startDetector;                   // index of first pixel used on detector for this ban
  float      scalingFactor;
  float      scalingError;
 } GDP_BIN_BAND_HEADER;

// Spectrum record structure

typedef struct                            // geolocation coordinates version 3
 {
  unsigned short          lonArray[5];            // longitude array
  short           latArray[5];            // latitude array
  float           szaArray[3];            // zenithal array
  unsigned short          losZa[3];               // line of sight zenithal array
  unsigned short          losAzim[3];             // line of sight azimuthal array
  float           satHeight;              // satellite geodetic height at point B
  float           radiusCurve;            // Earth radius curvatur at point B

  // From Level 2 data

  unsigned short  o3;                     // O3 VCD
  unsigned short  no2;                    // NO2 VCD
  unsigned short  cloudFraction;          // Cloud fraction
  unsigned short  cloudTopPressure;       // Cloud top pressure
  float           aziArray[3];
  unsigned short  unused[4];             // for later new data ?
 }
GEO_3;

// Cloud information : new in version 4.00, may 2009

typedef struct {
   float SurfaceHeight;
   float SurfaceAlbedo;
   float UV_Albedo;
   int Elevation;
   float CloudFraction[2]; /* Cloud Fraction and error */
   float CloudTopAlbedo[2]; /* Cloud Top Albedo and error */
   float CloudTopHeight[2]; /* Cloud Top Height and error */
   float TAU[2]; /* Cloud Optical Thickness and error */
   float CTP[2]; /* Cloud Top Pressure and error */
   short Mode; /* 0=normal, 1=snow/ice */
   short Type; /* 1=Cirrus, 2=Cirrostratus, 3=Deep convection, 4=Altocumulus, 5=Altostratus, etc */
 } GDP_BIN_CLOUD_HEADER;

typedef struct                            // geolocation coordinates version 4.00 from May 2009
 {
  unsigned short       lonArray[5];       // longitude array
  short                latArray[5];       // latitude array
  float                szaArrayTOA[3];    // solar zenithal angles, top of atmosphere
  float                aziArrayTOA[3];    // solar azimuth angles, top of atmosphere
  float                losZaTOA[3];       // line of sight zenithal angles, top of atmosphere
  float                losAzimTOA[3];     // line of sight azimuth angles, top of atmosphere
  float                szaArrayBOA[3];    // solar zenithal angles, bottom of atmosphere
  float                aziArrayBOA[3];    // solar azimuth angles, bottom of atmosphere
  float                losZaBOA[3];       // line of sight zenithal angles, bottom of atmosphere
  float                losAzimBOA[3];     // line of sight azimuth angles, bottom of atmosphere
  float                satHeight;         // satellite geodetic height at point B
  float                radiusCurve;       // Earth radius curvature at point B
  GDP_BIN_CLOUD_HEADER cloudInfo;
 }
GEO_4;

typedef struct
 {
  // From Level 1 data

  SHORT_DATETIME  dateAndTime;                // measurement date and time in UT
  short           groundPixelID;              // ground pixel order
  char            groundPixelType;            // ground pixel type
  char            indexSpectralParam;         // index of set of spectral parameters in reference record to use for building calibration
 }
SPECTRUM_RECORD;

#pragma pack(pop)

typedef struct _gome_recordInfo {
  INDEX  pixelNumber;                                                           // pixel number
  INDEX  pixelType;                                                             // pixel type
  double lat;                                                                   // latitude
  double lon;                                                                   // longitude
  double sza;                                                                   // solar zenith angle
  double cloudfrac; // cloud fraction
}
GDP_BIN_INFO;

typedef struct _GOMEOrbitFiles                                                  // description of an orbit
{
  char gdpBinFileName[MAX_STR_LEN+1];                                            // the name of the file with a part of the orbit
  GDP_BIN_INFO *gdpBinInfo;                                                     // useful information on records for fast access
  INDEX gdpBinBandIndex;                                                        // indexes of bands present in the current file
  GDP_BIN_FILE_HEADER gdpBinHeader;
  GDP_BIN_BAND_HEADER gdpBinBandInfo[SCIENCE_DATA_DEFINED];
  SPECTRUM_RECORD     gdpBinSpectrum;
  GEO_3               gdpBinGeo3;
  GEO_4               gdpBinGeo4;
  int                 gdpBinSpectraSize,                                        // total size of spectra vector GDP_BIN_coeff
                      gdpBinCoeffSize,                                          // number of polynomial coefficients in vector
                      gdpBinStartPixel[SCIENCE_DATA_DEFINED];                   // starting pixels for bands present in the file
  double             *gdpBinCoeff;                                              // coefficients for reconstructing wavelength calibrations
  float               gdpBinScalingFactor[SCIENCE_DATA_DEFINED],                // scaling factors for spectra band per band
                      gdpBinScalingError[SCIENCE_DATA_DEFINED];                 // scaling factors for errors band per band
  unsigned short             *gdpBinReference,                                          // buffer for irradiance spectra
                     *gdpBinRefError;                                           // errors on irradiance spectra
  int                 specNumber;
  RC rc;
}
GOME_ORBIT_FILE;

// ---------------------
// VARIABLES DECLARATION
// ---------------------

#define MAX_GOME_FILES 50 // maximum number of files per orbit

// ================
// GLOBAL VARIABLES
// ================

GOME_ORBIT_FILE GDP_BIN_orbitFiles[MAX_GOME_FILES];                             // list of files for an orbit
static int GDP_BIN_currentFileIndex=ITEM_NONE;                                       // index of the current file in the list
static int gdpBinOrbitFilesN=0;                                                 // the total number of files for the current day
static int gdpBinTotalRecordNumber;                                             // total number of records for an orbit
static int gdpBinLoadReferenceFlag=0;

/*----------------------------------------------------------------------------*\
**                                EvalPolynom_d
**  Input parameters:
**    X: The point to evaluate the polynom
**    Coefficient: Describe the polynom
**    Grad: grad of the polynom
**  Output parameters:
**  Other interfaces:
**  Description:
**    This function evaluates a polynom of grad Grad described by Coefficient
**    in the value x
**  References:
**  Libraries:
**  Created:    22.3.94
**  Author:     Diego Loyola, DLR/WT-DA-BS
\*----------------------------------------------------------------------------*/
static double EvalPolynom_d(double X, const double *Coefficient, short Grad) {
  double Result = 0.0, Mult = 1.0;
  short i;

  for (i=0; i<Grad; i++) {
    Result += Coefficient[i]*Mult;
    Mult *= X;
  }
  return Result;
}

// ===================
// ALLOCATION ROUTINES
// ===================

static void free_vza_refs(void) {
  if (vza_refs != NULL) {
    for (int i=0; i<NFeno; ++i) {
      for(size_t j=0; j<NUM_VZA_REFS; ++j) {
        free(vza_refs[i][j].spectrum);
      }
    }
  }
  free(vza_refs);
  vza_refs=NULL;
}

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by GOME readout routines
// -----------------------------------------------------------------------------

void GDP_BIN_ReleaseBuffers(void) {

  for (int gomeOrbitFileIndex=0;gomeOrbitFileIndex<MAX_GOME_FILES;gomeOrbitFileIndex++) {
      GOME_ORBIT_FILE *pOrbitFile=&GDP_BIN_orbitFiles[gomeOrbitFileIndex];

    if (pOrbitFile->gdpBinReference!=NULL)
      MEMORY_ReleaseBuffer(__func__,"gdpBinreference",pOrbitFile->gdpBinReference);
    if (pOrbitFile->gdpBinRefError!=NULL)
      MEMORY_ReleaseBuffer(__func__,"gdpBinrefError",pOrbitFile->gdpBinRefError);
    if (pOrbitFile->gdpBinCoeff!=NULL)
      MEMORY_ReleaseDVector(__func__,"gdpBincoeff",pOrbitFile->gdpBinCoeff,0);

    if (pOrbitFile->gdpBinInfo!=NULL)
      MEMORY_ReleaseBuffer(__func__,"gdpBinInfo",pOrbitFile->gdpBinInfo);

    memset(pOrbitFile,0,sizeof(GOME_ORBIT_FILE));
    pOrbitFile->gdpBinBandIndex=ITEM_NONE;
  }

  GDP_BIN_currentFileIndex=ITEM_NONE;                                           // index of the current file in the list
  gdpBinOrbitFilesN=0;                                                          // the total number of files for the current orbit
  gdpBinTotalRecordNumber=0;                                                    // total number of records for an orbit
  gdpBinLoadReferenceFlag=0;

  free_vza_refs();
}

// =================
// READ OUT ROUTINES
// =================

// get real year from a SHORT_DATE encoded year
static inline int get_year(int year_in) {
  if (year_in<30) {
    year_in+=(short)2000;
  } else if (year_in<130) {
    year_in+=(short)1900;
  } else if (year_in<1930) {
    year_in+=(short)100;
  }
  return year_in;
}

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinLambda
// -----------------------------------------------------------------------------
// PURPOSE       Build a wavelength calibration (irradiance or
//               earthshine spectra) using a set of coefficients of polynomial
//
// INPUT         indexParam    index of the set of parameters to use
//               fileIndex   index of the file for the current orbit
//
// OUTPUT        lambda        the wavelength calibration
// -----------------------------------------------------------------------------

static void GdpBinLambda(double *lambda, GOME_ORBIT_FILE *pOrbitFile) {

  int offset=0;                                                                   // offset in bytes

  switch(pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandType) {
  case PRJCT_INSTR_GDP_BAND_1B :
    offset=400;
    break;
  case PRJCT_INSTR_GDP_BAND_2B :
    offset=9;
    break;
  default :
    break;
  }

  int indexParam = pOrbitFile->gdpBinHeader.indexSpectralParam;
  for (int i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
       j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++) {
    lambda[i]=EvalPolynom_d((double)i+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].startDetector,
                            &pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinBandIndex+
                                                     indexParam*SPECTRAL_FITT_ORDER],SPECTRAL_FITT_ORDER);

    double lambdax=EvalPolynom_d((double)i+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].startDetector-offset,
                          &pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinBandIndex+
                                                   indexParam*SPECTRAL_FITT_ORDER],SPECTRAL_FITT_ORDER);

    lambda[i]-=EvalPolynom_d((double)(lambdax-OFFSET)/PARAMETER,&pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands+MAX_FITT_ORDER*pOrbitFile->gdpBinBandIndex],MAX_FITT_ORDER);
  }
}

RC GDP_BIN_get_orbit_date(int *year, int *month, int *day) {

  FILE *fp = NULL;
  RC rc = ERROR_ID_NO;

  const GOME_ORBIT_FILE *pOrbitFile = &GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

  if ((fp=fopen(pOrbitFile->gdpBinFileName,"rb"))==NULL) {
    return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pOrbitFile->gdpBinFileName);
  }

  GDP_BIN_FILE_HEADER tempHeader;

  // perhaps can use header from pOrbitFile
  if (!fread(&tempHeader,sizeof(GDP_BIN_FILE_HEADER),1,fp)) {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->gdpBinFileName);
  } else {
    // goto first spectrum, right after header
    fseek(fp,(int32_t)pOrbitFile->gdpBinHeader.headerSize,SEEK_SET);

    SPECTRUM_RECORD tempSpectrum;

    if (!fread(&tempSpectrum,sizeof(SPECTRUM_RECORD),1,fp)) {
      rc=ERROR_ID_FILE_END;
    } else {
      *year = get_year((int) tempSpectrum.dateAndTime.da_year);
      *month = (int) tempSpectrum.dateAndTime.da_mon;
      *day = (int) tempSpectrum.dateAndTime.da_day;
    }
  }

  if(fp != NULL) {
    fclose(fp);
  }

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on data saved in the file from the header
//
// INPUT         specFp    pointer to the current GOME orbit file
//
// INPUT/OUTPUT  pEngineContext interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------
RC GDP_BIN_Set(ENGINE_CONTEXT *pEngineContext) {
  char fileName[MAX_ITEM_TEXT_LEN];                                          // file name

  if (pEngineContext->fileInfo.specFp!=NULL) {
    fclose(pEngineContext->fileInfo.specFp);
    pEngineContext->fileInfo.specFp=NULL;
  }

  pEngineContext->recordNumber=0;
  gdpBinLoadReferenceFlag=0;

  GDP_BIN_currentFileIndex=ITEM_NONE;
  strcpy(fileName,pEngineContext->fileInfo.fileName);
  NDET[0]=1024;
  RC rc=ERROR_ID_NO;

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto) {
    int indexFile = 0;
    for (;indexFile<gdpBinOrbitFilesN;indexFile++)
      if (!strcasecmp(pEngineContext->fileInfo.fileName,GDP_BIN_orbitFiles[indexFile].gdpBinFileName) )
        break;

    if (indexFile<gdpBinOrbitFilesN)
      GDP_BIN_currentFileIndex=indexFile;
  }

  if (GDP_BIN_currentFileIndex==ITEM_NONE) { // File has not been loaded already -> load a new file.
    // Release old buffers
    GDP_BIN_ReleaseBuffers();

    // Get the number of files to load
    gdpBinOrbitFilesN = 0;
    if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto) {
      gdpBinLoadReferenceFlag=1;

      char filePath[MAX_STR_SHORT_LEN+1];
      strcpy(filePath,pEngineContext->fileInfo.fileName);

      char *ptr = strrchr(filePath, PATH_SEP);
      if (ptr == NULL) {
        strcpy(filePath,".");
      } else {
        *ptr = '\0';
      }

      DIR *hDir=opendir(filePath);
      struct dirent *fileInfo = NULL;
      while (hDir!=NULL && ((fileInfo=readdir(hDir))!=NULL) ) {
        sprintf(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName,"%s/%s",filePath,fileInfo->d_name);
        if (!STD_IsDir(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName)) {
          ++gdpBinOrbitFilesN;
        }
      }

      if (hDir != NULL)
        closedir(hDir);
    }

    if (!gdpBinOrbitFilesN) {
      gdpBinOrbitFilesN=1;
      strcpy(GDP_BIN_orbitFiles[0].gdpBinFileName,pEngineContext->fileInfo.fileName);
    }

    // Load files
    gdpBinTotalRecordNumber=0;
    for (int indexFile=0;indexFile<gdpBinOrbitFilesN;indexFile++) {
      GOME_ORBIT_FILE *pOrbitFile=&GDP_BIN_orbitFiles[indexFile];
      pOrbitFile->specNumber=0;

      // Open the current GOME orbit file
      FILE *fp=fopen(pOrbitFile->gdpBinFileName,"rb");

      if (fp==NULL) {
        pOrbitFile->rc = ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pOrbitFile->gdpBinFileName);
        continue;
      }

      // Read GOME file header
      if (!fread(&pOrbitFile->gdpBinHeader,sizeof(GDP_BIN_FILE_HEADER),1,fp)) {
        pOrbitFile->rc = ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->gdpBinFileName);
        fclose(fp);
        continue;
      }

      // In the GDP binary format, errors are saved on user request
      bool useErrors=((pOrbitFile->gdpBinHeader.mask&GDP_BIN_ERROR_ID_MASK)==GDP_BIN_ERROR_ID_MASK)?1:0;
      int errorFlag=(useErrors)?0:1;

      // Retrieve information on bands present in the file
      for (int i=0; i<pOrbitFile->gdpBinHeader.nbands; i++)
        fread(&pOrbitFile->gdpBinBandInfo[i],(sizeof(GDP_BIN_BAND_HEADER)-errorFlag*sizeof(float)),1,fp);

      // Coefficients of the polynomial for re-calculating the wavelength calibration

      pOrbitFile->gdpBinCoeffSize=pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands;
      pOrbitFile->gdpBinCoeffSize+=MAX_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands;

      // Get the size of spectra band per band

      pOrbitFile->gdpBinStartPixel[0]=0;
      pOrbitFile->gdpBinSpectraSize=pOrbitFile->gdpBinBandInfo[0].bandSize;
      for (int i=1; i<pOrbitFile->gdpBinHeader.nbands;i++) {
        pOrbitFile->gdpBinStartPixel[i]=pOrbitFile->gdpBinStartPixel[i-1]+pOrbitFile->gdpBinBandInfo[i-1].bandSize;
        pOrbitFile->gdpBinSpectraSize+=pOrbitFile->gdpBinBandInfo[i].bandSize;
      }

      for (int i=0; i<pOrbitFile->gdpBinHeader.nbands; i++)
        if (pOrbitFile->gdpBinBandInfo[i].bandType==pEngineContext->project.instrumental.gome.bandType)
          pOrbitFile->gdpBinBandIndex=i;

      // Buffers allocation

      if (pOrbitFile->gdpBinBandIndex==ITEM_NONE)
        return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_GDP_BANDINDEX,fileName);
      if (!pOrbitFile->gdpBinCoeffSize || !pOrbitFile->gdpBinSpectraSize || !pOrbitFile->gdpBinHeader.nspectra)
        return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

      if (((pOrbitFile->gdpBinCoeff=MEMORY_AllocDVector(__func__,"pOrbitFile->gdpBinCoeff",0,pOrbitFile->gdpBinCoeffSize-1))==NULL) ||
          ((pOrbitFile->gdpBinReference=MEMORY_AllocBuffer(__func__,"pOrbitFile->gdpBinReference",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((pOrbitFile->gdpBinRefError=MEMORY_AllocBuffer(__func__,"pOrbitFile->gdpBinRefError",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((pOrbitFile->gdpBinInfo=MEMORY_AllocBuffer(__func__,"pOrbitFile->gdpBinInfo",pOrbitFile->gdpBinHeader.nspectra,sizeof(GDP_BIN_INFO),0,MEMORY_TYPE_STRUCT))==NULL)) {

        fclose(fp);
        return ERROR_ID_ALLOC;
      }

      NDET[0]=pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;

      // Read the irradiance spectrum

      fread(pOrbitFile->gdpBinCoeff,sizeof(double)*pOrbitFile->gdpBinCoeffSize,1,fp);
      fread(pOrbitFile->gdpBinReference,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp);

      if (useErrors)
        fread(pOrbitFile->gdpBinRefError,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp);

      pOrbitFile->specNumber=pOrbitFile->gdpBinHeader.nspectra;

      for (int indexRecord=0; indexRecord<pOrbitFile->specNumber; indexRecord++) {
        fseek(fp,(int32_t)pOrbitFile->gdpBinHeader.headerSize+indexRecord*pOrbitFile->gdpBinHeader.recordSize,SEEK_SET);

        if (!fread(&pOrbitFile->gdpBinSpectrum,sizeof(SPECTRUM_RECORD),1,fp)) {
          rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
          continue;
        }

        if (pOrbitFile->gdpBinHeader.version<40) {
          if (fread(&pOrbitFile->gdpBinGeo3,sizeof(GEO_3),1,fp) != 1)
            return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT, pOrbitFile->gdpBinFileName);

          pOrbitFile->gdpBinInfo[indexRecord].sza=pOrbitFile->gdpBinGeo3.szaArray[1];
          pOrbitFile->gdpBinInfo[indexRecord].lat=0.01*pOrbitFile->gdpBinGeo3.latArray[4];
          pOrbitFile->gdpBinInfo[indexRecord].lon=0.01*pOrbitFile->gdpBinGeo3.lonArray[4];
          pOrbitFile->gdpBinInfo[indexRecord].cloudfrac=pOrbitFile->gdpBinGeo3.cloudFraction;
        } else { // gdpBinHeader.version >= 40:
          if (fread(&pOrbitFile->gdpBinGeo4,sizeof(GEO_4),1,fp) != 1)
            return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT, pOrbitFile->gdpBinFileName);

          pOrbitFile->gdpBinInfo[indexRecord].sza=pOrbitFile->gdpBinGeo4.szaArrayBOA[1];
          pOrbitFile->gdpBinInfo[indexRecord].lat=0.01*pOrbitFile->gdpBinGeo4.latArray[4];
          pOrbitFile->gdpBinInfo[indexRecord].lon=0.01*pOrbitFile->gdpBinGeo4.lonArray[4];
          pOrbitFile->gdpBinInfo[indexRecord].cloudfrac=pOrbitFile->gdpBinGeo4.cloudInfo.CloudFraction[0];
        }
        pOrbitFile->gdpBinInfo[indexRecord].pixelNumber=pOrbitFile->gdpBinSpectrum.groundPixelID;
        pOrbitFile->gdpBinInfo[indexRecord].pixelType=pOrbitFile->gdpBinSpectrum.groundPixelType;
      }

      fclose(fp);

      if (!strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->gdpBinFileName))
        GDP_BIN_currentFileIndex=indexFile;

      gdpBinTotalRecordNumber+=pOrbitFile->specNumber;

      if (rc!=ERROR_ID_NO)
        pOrbitFile->rc=rc;

      rc=ERROR_ID_NO;
    }
  } // end if (currentfileindex == ITEM_NONE)

  if (GDP_BIN_currentFileIndex!=ITEM_NONE
      && (pEngineContext->recordNumber=GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].specNumber)>0
      && !rc) {

      GOME_ORBIT_FILE *pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

    // Irradiance spectrum

    for (int i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
         j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++) {

      pEngineContext->buffers.irrad[i]=(double)pOrbitFile->gdpBinReference[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingFactor;
    }

    pEngineContext->recordInfo.satellite.orbit_number= 1+pOrbitFile->gdpBinHeader.orbitNumber;
    pEngineContext->recordInfo.useErrors=((pOrbitFile->gdpBinHeader.mask&GDP_BIN_ERROR_ID_MASK)==GDP_BIN_ERROR_ID_MASK)?1:0;
    NDET[0]=pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;

    GdpBinLambda(pEngineContext->buffers.lambda, &GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex]);
    for (int i=0; i<NDET[0]; ++i) {
      pEngineContext->buffers.lambda_irrad[i]=pEngineContext->buffers.lambda[i];
    }

    rc=GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].rc;
  }

  return rc;
 }

static int read_record(FILE *fp, GOME_ORBIT_FILE *pOrbitFile, int record_index,
                       double *lambda, double *spec, double *spec_error) {
  int rc = ERROR_ID_NO;

  unsigned short *temp_spec = malloc(pOrbitFile->gdpBinSpectraSize*sizeof(*temp_spec));
  unsigned short *temp_spec_error = malloc(pOrbitFile->gdpBinSpectraSize*sizeof(*temp_spec_error));
  if (temp_spec == NULL || temp_spec_error == NULL) {
    rc = ERROR_ID_ALLOC;
    goto cleanup;
  }

  fseek(fp, pOrbitFile->gdpBinHeader.headerSize+record_index*pOrbitFile->gdpBinHeader.recordSize,SEEK_SET);

  if (!fread(&pOrbitFile->gdpBinSpectrum,sizeof(SPECTRUM_RECORD),1,fp) ||
      ((pOrbitFile->gdpBinHeader.version<40) && !fread(&pOrbitFile->gdpBinGeo3,sizeof(GEO_3),1,fp)) ||
      ((pOrbitFile->gdpBinHeader.version>=40) && !fread(&pOrbitFile->gdpBinGeo4,sizeof(GEO_4),1,fp)) ||
      !fread(pOrbitFile->gdpBinScalingFactor,sizeof(float)*pOrbitFile->gdpBinHeader.nbands,1,fp) ||
      !fread(temp_spec,sizeof(*temp_spec)*pOrbitFile->gdpBinSpectraSize,1,fp) ||
      (spec_error != NULL && (!fread(pOrbitFile->gdpBinScalingError,sizeof(float)*pOrbitFile->gdpBinHeader.nbands,1,fp) ||
                              !fread(temp_spec_error,sizeof(*temp_spec_error)*pOrbitFile->gdpBinSpectraSize,1,fp))))
    rc = ERROR_ID_FILE_END;

  GdpBinLambda(lambda, pOrbitFile);

  // Convert spectrum from short integers to double

  double max=0.;
  for (int i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
       j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++) {
    spec[i]=((double)temp_spec[j])/pOrbitFile->gdpBinScalingFactor[pOrbitFile->gdpBinBandIndex];
    if (spec_error != NULL)
      spec_error[i]=((double)temp_spec_error[i])/pOrbitFile->gdpBinScalingError[pOrbitFile->gdpBinBandIndex];

    if (spec[i]>max)
      max=spec[i];
  }

  if (max==0.)
    rc=ERROR_ID_FILE_RECORD;

 cleanup:
  free(temp_spec);
  free(temp_spec_error);

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_Read
// -----------------------------------------------------------------------------
// PURPOSE       GOME calibrated level 1 data read out (binary format)
//
// INPUT         recordNo     index of the record to read
//               specFp       pointer to the current GOME orbit file
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------
RC GDP_BIN_Read(ENGINE_CONTEXT *pEngineContext,int recordNo) {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  SHORT_DATE today;                                                             // date of measurements
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
  rc=ERROR_ID_NO;
  FILE *fp=NULL;

  if (!pOrbitFile->specNumber)
    return ERROR_ID_FILE_EMPTY;
  if (recordNo<=0 || recordNo>pOrbitFile->specNumber)
    return ERROR_ID_FILE_RECORD;
  if ((fp=fopen(pOrbitFile->gdpBinFileName,"rb"))==NULL)
    return ERROR_ID_FILE_NOT_FOUND;

  // Complete record read out
  rc = read_record(fp, pOrbitFile, recordNo-1, pBuffers->lambda, pBuffers->spectrum,
                   pEngineContext->recordInfo.useErrors ? pBuffers->sigmaSpec : NULL);
  if (rc)
    goto cleanup;

  if (pEngineContext->project.instrumental.gome.pixelType!=-1 &&
      pOrbitFile->gdpBinSpectrum.groundPixelType!=pEngineContext->project.instrumental.gome.pixelType) {
    rc=ERROR_ID_FILE_RECORD;
    goto cleanup;
  }
  today.da_year=pOrbitFile->gdpBinSpectrum.dateAndTime.da_year;
  today.da_mon=pOrbitFile->gdpBinSpectrum.dateAndTime.da_mon;
  today.da_day=pOrbitFile->gdpBinSpectrum.dateAndTime.da_day;
  today.da_year = get_year((int)today.da_year);

  // Fill fields of structure

  pRecord->Tint     = 0.;
  pRecord->NSomme   = 0;

  if (pOrbitFile->gdpBinHeader.version<40) {
    pRecord->Zm       = pOrbitFile->gdpBinGeo3.szaArray[1];
    pRecord->Azimuth  = pOrbitFile->gdpBinGeo3.aziArray[1];
    pRecord->longitude=0.01*pOrbitFile->gdpBinGeo3.lonArray[4];
    pRecord->latitude =0.01*pOrbitFile->gdpBinGeo3.latArray[4];
    pRecord->zenithViewAngle=0.01*pOrbitFile->gdpBinGeo3.losZa[1];
    pRecord->azimuthViewAngle=0.01*pOrbitFile->gdpBinGeo3.losAzim[1];
    pRecord->satellite.cloud_fraction=pOrbitFile->gdpBinGeo3.cloudFraction;
    pRecord->satellite.cloud_top_pressure=pOrbitFile->gdpBinGeo3.cloudTopPressure;
    for (int i=0; i<3; ++i) {
      pRecord->gome.azim[i]=pOrbitFile->gdpBinGeo3.aziArray[i];
      pRecord->gome.sza[i]=pOrbitFile->gdpBinGeo3.szaArray[i];
      pRecord->gome.vaa[i]=0.01*pOrbitFile->gdpBinGeo3.losAzim[i];
      pRecord->gome.vza[i]=0.01*pOrbitFile->gdpBinGeo3.losZa[i];
    }
    for (int i=0; i<4; ++i) {
      pRecord->satellite.cornerlats[i] = 0.01*pOrbitFile->gdpBinGeo3.latArray[i];
      pRecord->satellite.cornerlons[i] = 0.01*pOrbitFile->gdpBinGeo3.lonArray[i];
    }
    pRecord->satellite.altitude = pOrbitFile->gdpBinGeo3.satHeight;
    pRecord->satellite.earth_radius = pOrbitFile->gdpBinGeo3.radiusCurve;
  } else {
    pRecord->Zm       = pOrbitFile->gdpBinGeo4.szaArrayBOA[1];
    pRecord->Azimuth  = pOrbitFile->gdpBinGeo4.aziArrayBOA[1];
    pRecord->longitude=0.01*pOrbitFile->gdpBinGeo4.lonArray[4];
    pRecord->latitude =0.01*pOrbitFile->gdpBinGeo4.latArray[4];
    pRecord->zenithViewAngle=pOrbitFile->gdpBinGeo4.losZaBOA[1];
    pRecord->azimuthViewAngle=pOrbitFile->gdpBinGeo4.losAzimBOA[1];
    pRecord->satellite.cloud_fraction=pOrbitFile->gdpBinGeo4.cloudInfo.CloudFraction[0];
    pRecord->satellite.cloud_top_pressure=pOrbitFile->gdpBinGeo4.cloudInfo.CTP[0];
    for (int i=0; i<3; ++i) {
      pRecord->gome.azim[i]=pOrbitFile->gdpBinGeo4.aziArrayBOA[i];
      pRecord->gome.sza[i]=pOrbitFile->gdpBinGeo4.szaArrayBOA[i];
      pRecord->gome.vaa[i]=0.01*pOrbitFile->gdpBinGeo4.losAzimBOA[i];
      pRecord->gome.vza[i]=pOrbitFile->gdpBinGeo4.losZaBOA[i];
    }
    for (int i=0; i<4; ++i) {
      pRecord->satellite.cornerlats[i] = 0.01*pOrbitFile->gdpBinGeo4.latArray[i];
      pRecord->satellite.cornerlons[i] = 0.01*pOrbitFile->gdpBinGeo4.lonArray[i];
    }
    pRecord->satellite.altitude = pOrbitFile->gdpBinGeo4.satHeight;
    pRecord->satellite.earth_radius = pOrbitFile->gdpBinGeo4.radiusCurve;
  }

  pRecord->present_datetime.thetime.ti_hour=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_hour;
  pRecord->present_datetime.thetime.ti_min=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_min;
  pRecord->present_datetime.thetime.ti_sec=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_sec;

  memset(pRecord->Nom,0,20);
  pRecord->present_datetime.thedate.da_day = today.da_day;
  pRecord->present_datetime.thedate.da_mon = today.da_mon;
  pRecord->present_datetime.thedate.da_year = today.da_year;

  pRecord->Tm=ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
  pRecord->localCalDay=ZEN_FNCaljda(&pRecord->Tm);
  pRecord->TotalExpTime=pRecord->TotalAcqTime=0.;
  pRecord->TimeDec=(double)pOrbitFile->gdpBinSpectrum.dateAndTime.ti_hour+
    pOrbitFile->gdpBinSpectrum.dateAndTime.ti_min/60.+
    pOrbitFile->gdpBinSpectrum.dateAndTime.ti_sec/3600.;

  pRecord->gome.pixelNumber=pOrbitFile->gdpBinSpectrum.groundPixelID;
  pRecord->gome.pixelType=pOrbitFile->gdpBinSpectrum.groundPixelType;

  pRecord->altitude=0.;

cleanup:

  // Close file
  if (fp!=NULL)
   fclose(fp);

  return rc;
 }

static bool use_as_reference(const GDP_BIN_INFO *record, const FENO *feno) {
  const double latDelta = fabs(feno->refLatMin - feno->refLatMax);
  const double lonDelta = fabs(feno->refLonMin - feno->refLonMax);
  const double cloudDelta = fabs(feno->cloudFractionMin - feno->cloudFractionMax);

  const bool match_lat = latDelta <= EPSILON
    || (record->lat >= feno->refLatMin && record->lat <= feno->refLatMax);
  const bool match_lon = lonDelta <= EPSILON
    || ( (feno->refLonMin < feno->refLonMax
          && record->lon >=feno->refLonMin && record->lon <= feno->refLonMax)
         ||
         (feno->refLonMin >= feno->refLonMax
          && (record->lon >= feno->refLonMin || record->lon <= feno->refLonMax) ) ); // if refLonMin > refLonMax, we have either lonMin < lon < 360, or 0 < lon < refLonMax
  const bool match_sza = feno->refSZADelta <= EPSILON
    || ( fabs(record->sza - feno->refSZA) <= feno->refSZADelta);
  const bool match_cloud = cloudDelta <= EPSILON
    || (record->cloudfrac >= feno->cloudFractionMin && record->cloudfrac <= feno->cloudFractionMax);

  return (match_lat && match_lon && match_sza && match_cloud);
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

  FILE *fp=NULL;

  // iterate over all orbit files in same directory
  for (int i=0; i<gdpBinOrbitFilesN; ++i) {
    GOME_ORBIT_FILE *orbit = &GDP_BIN_orbitFiles[i];
    size_t n_wavel = orbit->gdpBinBandInfo[orbit->gdpBinBandIndex].bandSize;
    fp = fopen(orbit->gdpBinFileName, "rb");
    if (fp == NULL) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_NOT_FOUND, orbit->gdpBinFileName);
    }
    for (int j=0; j<orbit->specNumber; ++j) {
      // each spectrum can be used for multiple analysis windows, so
      // we use one copy, and share the pointer between the different
      // analysis windows. We initialize as NULL, it becomes not-null
      // as soon as it is used in one or more analysis windows:
      struct ref_spectrum *ref = NULL;

      // check if this spectrum satisfies constraints for one of the analysis windows:
      for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
        const FENO *pTabFeno = &TabFeno[0][analysis_window];
        const int pixel_type = orbit->gdpBinInfo[j].pixelType;
        if (!pTabFeno->hidden
            && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
            && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
            && use_as_reference(&orbit->gdpBinInfo[j],pTabFeno) ) {

          if (ref == NULL) {
            // ref hasn't been initialized yet for another analysis window, so do that now:
            ref = malloc(sizeof(*ref));
            ref->lambda = malloc(n_wavel*sizeof(*ref->lambda));
            ref->spectrum = malloc(n_wavel*sizeof(*ref->spectrum));

            // store the new reference at the front of the linked list:
            struct ref_list *new = malloc(sizeof(*new));
            new->ref = ref;
            new->next = *list_handle;
            *list_handle = new;

            RC rc = read_record(fp, orbit, j, ref->lambda, ref->spectrum, NULL);
            if (rc != ERROR_ID_NO) {
              fclose(fp);
              return rc;
            }
          }

          // store ref at the front of the list of selected references for this analysis window and vza bin.
          struct ref_list *list_item = malloc(sizeof(*list_item));
          list_item->ref = ref;
          list_item->next = selected_spectra[analysis_window][pixel_type];
          selected_spectra[analysis_window][pixel_type] = list_item;
        }
      }
    }
    fclose(fp);
    fp=NULL;
  }

  return ERROR_ID_NO;
}

static void initialize_vza_refs(void) {
  free_vza_refs(); // will free previous allocated structures, if any.

  vza_refs = malloc(NFeno * sizeof(*vza_refs));

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
      ref->norm = ref->shift = ref->stretch = ref->stretch2 = 0.0;
    }
  }
}

static int show_ref_info(int i_row, const FENO *pTabFeno, const struct reference *refs, void *responseHandle) {
  int i_column=2;

  mediateResponseLabelPage(plotPageRef, "Reference", "Reference", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, i_column, "Analysis window", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 1+i_column, pTabFeno->windowName, responseHandle);
  i_row+=2;
  mediateResponseCellDataString(plotPageRef, i_row, i_column, "Earthshine reference", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 1+i_column, "# spectra", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 2+i_column, "shift", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 3+i_column, "stretch", responseHandle);
  ++i_row;
  const char *pixeltype[] = {"EAST", "CENTER", "WEST", "BACKSCAN"};
  for (int i=0; i != sizeof(pixeltype)/sizeof(pixeltype[0]); ++i) {
    mediateResponseCellDataString(plotPageRef, i_row, i_column, pixeltype[i], responseHandle);
    mediateResponseCellDataInteger(plotPageRef, i_row, 1+ i_column, refs[i].n_spectra, responseHandle);
    mediateResponseCellDataDouble(plotPageRef, i_row, 2 + i_column,refs[i].shift, responseHandle);
    mediateResponseCellDataDouble(plotPageRef, i_row, 3 + i_column,refs[i].stretch, responseHandle);
    ++i_row;
    const char* labelfmt = "Reference %s";
    char *plot_label = malloc(sizeof(labelfmt) +strlen(pixeltype[i]));
    sprintf(plot_label, labelfmt, pixeltype[i]);
    MEDIATE_PLOT_CURVES(plotPageRef,Spectrum,forceAutoScale,plot_label,"Wavelength (nm)","Intensity", responseHandle,
                        CURVE(.x=pTabFeno->LambdaRef, .y=refs[i].spectrum, .length=refs[i].n_wavel));
    free(plot_label);
  }
  return ++i_row;
}

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinNewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//               specFp       pointer to the current file
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

static RC GdpBinNewRef(const ENGINE_CONTEXT *pEngineContext,void *responseHandle) {
  mediateResponseRetainPage(plotPageRef,responseHandle);

  int i_row = 1; // row index of the GUI "spreadsheet" to print reference info.
  ANALYSE_plotRef=1;

  if (pEngineContext->recordNumber==0)
    return ERROR_ID_ALLOC;

  // Allocate references
  initialize_vza_refs();

  // 1. look in all candidate orbit files (i.e. orbit files in same
  //    dir)

  // for each analysis window: selected spectra per VZA bin
  // the same spectrum can be used in multiple analysis windows.
  struct ref_list* (*selected_spectra)[NUM_VZA_REFS] = malloc(NFeno * sizeof(*selected_spectra));

  // list_handle: list of references to same set of spectra, used for
  // memory management.  In this list, each spectrum appears only once.
  struct ref_list *list_handle;
  RC rc = find_ref_spectra(selected_spectra, &list_handle);
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
          const char* message = " for analysis window %s and pixel type %zu";
          const int length = 1 + strlen(message) + strlen(pTabFeno->windowName) + strlen(TOSTRING(MAX_FENO));
          char* tmp = malloc(length);
          sprintf(tmp, message, pTabFeno->windowName, j);
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
      if (!rc)
        i_row = show_ref_info(i_row, pTabFeno, vza_refs[i], responseHandle);
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

// pixeltype 0 = east, 1 = center, 2 = west
RC GDP_BIN_get_vza_ref(int pixel_type, int index_feno, FENO *feno) {
  const struct reference *ref = &vza_refs[index_feno][pixel_type];
  const char* message = " for analysis window %s and pixel type %d";
  char *tmp = malloc(1 + strlen(message) + strlen(feno->windowName));
  sprintf(tmp, message, feno->windowName, index_feno);
  if  (!ref->n_spectra) {
    return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REFERENCE_SELECTION, tmp);
  }
  free(tmp);

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

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//               specFp       pointer to the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC GDP_BIN_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow,i,j;                                // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double factTemp,lambdaMin,lambdaMax;                                          // working variables
  int DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

  const int n_wavel = NDET[0];

  if (!(rc=pOrbitFile->rc) && (gdpBinLoadReferenceFlag || !pEngineContext->analysisRef.refAuto)) {
    lambdaMin=9999.;
    lambdaMax=-9999.;

    rc=ERROR_ID_NO;
    useKurucz=useUsamp=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++) {
       pTabFeno=&TabFeno[0][indexFeno];
       pTabFeno->NDET=n_wavel;

       // Load calibration and reference spectra

       if (!pTabFeno->gomeRefFlag) {
         memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda,sizeof(double)*n_wavel);

         for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
              j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++) {
           pTabFeno->Sref[i]=(double)pOrbitFile->gdpBinReference[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingFactor;
           if (pRecord->useErrors && pTabFeno->SrefSigma!=NULL)
             pTabFeno->SrefSigma[i]=(double)pOrbitFile->gdpBinRefError[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingError;
         }

         if (!TabFeno[0][indexFeno].hidden) {
           if (!(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"GDP_BIN_LoadAnalysis (Reference) ")) &&
              (!pRecord->useErrors || !(rc=VECTOR_NormalizeVector(pTabFeno->SrefSigma-1,pTabFeno->NDET,&factTemp,"GDP_BIN_LoadAnalysis (RefError) ")))) {
             memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
             pTabFeno->useEtalon=pTabFeno->displayRef=1;

             // Browse symbols

             for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++) {
               pTabCross=&pTabFeno->TabCross[indexTabCross];
               pWrkSymbol=&WorkSpace[pTabCross->Comp];

               // Cross sections and predefined vectors

               if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                    ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                    ((indexTabCross==pTabFeno->indexCommonResidual) ||
                   (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
                    ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET))!=ERROR_ID_NO))

                 goto EndGOME_LoadAnalysis;
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

             if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
                 ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) &&
                 ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))
              goto EndGOME_LoadAnalysis;

             pTabFeno->Decomp=1;
            }
         }

         memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*n_wavel);
         memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*n_wavel);

         useUsamp+=pTabFeno->useUsamp;
         useKurucz+=pTabFeno->useKurucz;

         if (pTabFeno->useUsamp) {
           if (pTabFeno->LambdaRef[0]<lambdaMin)
             lambdaMin=pTabFeno->LambdaRef[0];
           if (pTabFeno->LambdaRef[n_wavel-1]>lambdaMax)
             lambdaMax=pTabFeno->LambdaRef[n_wavel-1];
         }
       }
    }

    // Kurucz

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ)) {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
        goto EndGOME_LoadAnalysis;
    }

    // Build undersampling cross sections

    if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ)) {

       if ( (rc=ANALYSE_UsampLocalAlloc(0) )!=ERROR_ID_NO ||
            (rc=ANALYSE_UsampBuild(0,0,0))!=ERROR_ID_NO ||
            (rc=ANALYSE_UsampBuild(1,ITEM_NONE,0))!=ERROR_ID_NO)

        goto EndGOME_LoadAnalysis;
    }

    // Reference

    if ((THRD_id==THREAD_TYPE_ANALYSIS) && gdpBinLoadReferenceFlag && !(rc=GdpBinNewRef(pEngineContext,responseHandle)))
      rc=ANALYSE_AlignReference(pEngineContext,2,responseHandle,0); // automatic ref selection

    if (rc==ERROR_ID_NO_REF)
      for (i=GDP_BIN_currentFileIndex+1;i<gdpBinOrbitFilesN;i++)
        GDP_BIN_orbitFiles[i].rc=rc;
  }

  EndGOME_LoadAnalysis :

  return rc;
}
