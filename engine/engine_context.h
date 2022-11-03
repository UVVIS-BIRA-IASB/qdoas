#ifndef ENGINE_CONTEXT_H
#define ENGINE_CONTEXT_H

// ==============
// ENGINE CONTEXT
// ==============

#include <stdio.h>
#include <stdint.h>

#include "doas.h"
#include "mediate_general.h"
#include "matrix.h"

#include "tropomi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _prjctSpectra {
  int    noMin,noMax;                                                           // spectra numbers range
  float  SZAMin,SZAMax,SZADelta;                                                // SZA range
  float  elevMin,elevMax,elevTol;
  float  refAngle,refTol;                                                       // for maxdoas measurements, angle to use for reference (usually zenith but could be another elevation angle)
  float  cloudMin,cloudMax;                                                     // cloud fraction range (for satellite measurements)
  float  longMin,longMax,latMin,latMax,                                         // resp. longitude and latitude ranges
         radius;                                                                // radius if circle mode is used
  int    namesFlag,                                                             // use names
         darkFlag,                                                              // use dark current
         displaySpectraFlag,                                                    // display complete spectra
         displayDataFlag,                                                       // display data on spectra
         displayCalibFlag,
         displayFitFlag,                                                        // display fits
         displayPause,                                                          // QDOAS obsolete field !!! : force pause between two graph display
         displayDelay,                                                          // QDOAS obsolete field !!! : calculated delay for display pause
         maxGraphV,                                                             // QDOAS obsolete field !!! : maximum number of graphs in height a graphic page can hold
         maxGraphH,                                                             // QDOAS obsolete field !!! : maximum number of graphs in width a graphic page can hold
         mode;
  int    fieldsNumber;                                                          // number of ascii flags set in the next list
  int    fieldsFlag[PRJCT_RESULTS_MAX];                                         // fields used in ascii format
} PRJCT_SPECTRA;

// -----------------
// ANALYSIS TAB PAGE
// -----------------

// Analysis tab page description
// -----------------------------

struct _prjctAnlys {
    int method;                                        // analysis method
    int fitWeighting;                                  // least-squares fit weighting
    int interpol;                                      // interpolation
    double convergence;                                // convergence criterion
    double spike_tolerance;                            // max ratio of (pixel residual)/(average residual)

    int securityGap;
    int maxIterations;                                 // maximum number of iterations
};

// --------------------
// CALIBRATION TAB PAGE
// --------------------

// Calibration tab page description
// --------------------------------

struct _prjctKurucz {
  int              windowsNumber;                      // number of windows
  int              fwhmPolynomial;                     // security gap in pixels numbers
  int              shiftPolynomial;                    // degree of polynomial to use
  char           file[MAX_ITEM_TEXT_LEN];          // kurucz file
  char           slfFile[MAX_ITEM_TEXT_LEN];       // slit function file
  int              displayFit;                         // display fit flag
  int              displayResidual;                    // display new calibration flag
  int              displayShift;                       // display shift/Fwhm in each pixel
  int              displaySpectra;                     // display fwhm in each pixel
  int              fwhmFit;                            // force fit of fwhm while applying Kurucz
  int              fwhmType;                           // type of slit function to fit
  double           lambdaLeft;
  double           lambdaRight;
  double           windowSize;
  double           customLambdaMin[MAX_CALIB_WINDOWS];
  double           customLambdaMax[MAX_CALIB_WINDOWS];
  int              divisionMode;
  int              invPolyDegree;
  int              analysisMethod;
  int              preshiftFlag;
  double           preshiftMin;
  double           preshiftMax;
};

// ----------------------
// UNDERSAMPLING TAB PAGE
// ----------------------

struct _prjctUsamp {
  char  kuruczFile[MAX_STR_LEN+1];
  int    method;
  double phase;
};


// ---------------------
// INSTRUMENTAL TAB PAGE
// ---------------------

// Instrumental tab page description
// ---------------------------------

typedef struct _prjctAsciiFormat
 {
  int   format,szaSaveFlag,azimSaveFlag,elevSaveFlag,timeSaveFlag,dateSaveFlag,lambdaSaveFlag;
 }
PRJCT_ASCII;

typedef struct _prjctSaozFormat
 {
     int spectralRegion;
     int spectralType;
 }
PRJCT_SAOZ;

typedef struct _prjctSciaFormat
 {
  int         sciaChannel;
  int         sciaCluster[6];
  char       sciaReference[4];
 }
PRJCT_SCIA;

typedef struct _prjctGomeFormat
 {
     int bandType;
     int pixelType;
 }
PRJCT_GOME;

typedef struct _prjctOmiFormat
 {
   char refPath[MAX_STR_LEN+1];
   int spectralType;
   int averageFlag;
   int   pixelQFRejectionFlag,pixelQFMaxGaps,pixelQFMask;                       // pixel quality flags rejection
   enum omi_xtrack_mode xtrack_mode;                                            // how to use XTrackQualityFlags
 }
PRJCT_OMI;

typedef struct _prjctMfcFormat
 {
  unsigned int        mfcMaskOffset;
  unsigned int        mfcMaskDark;
  unsigned int        mfcMaskInstr;
  unsigned int        mfcMaskSpec;
  int         mfcMaskUse;
  int         mfcMaxSpectra;
  int         mfcRevert;
  char       mfcStdDate[24];
  int     wavelength;
 }
PRJCT_MFC;

typedef struct _prjctGemsFormat
 {
  int binning;
 }
PRJCT_GEMS;

typedef struct _prjctInstrumental
 {
  char        observationSite[MAX_ITEM_NAME_LEN+1];                            // index of observation site in list
  char        readOutFormat;                                                   // spectra read out format
   int        user;                                                            // user defined
  char        calibrationFile[MAX_ITEM_TEXT_LEN];                              // calibration file
  char        instrFunction[MAX_ITEM_TEXT_LEN];                                // instrumental function
  char        vipFile[MAX_ITEM_TEXT_LEN];                                      // interpixel variability correction
  char        dnlFile[MAX_ITEM_TEXT_LEN];                                      // detector not linearity correction
  char        imagePath[MAX_ITEM_TEXT_LEN];                                    // root path for camera pictures
  bool        use_row[MAX_SWATHSIZE];                                          // to skip rows of imager instruments
  INDEX       use_row_index[MAX_SWATHSIZE];                                    // to skip rows of imager instruments
  int         detectorSize;                                                    // size of detector in pixels
  int         azimuthFlag;
  int         averageFlag;
  PRJCT_ASCII ascii;
  PRJCT_SAOZ  saoz;
  PRJCT_GOME  gome;
  PRJCT_GOME  gomenetcdf;
  PRJCT_SCIA  scia;
  PRJCT_OMI   omi;
  PRJCT_MFC   mfc;
  PRJCT_GEMS  gems;
  char       offsetFile[MAX_ITEM_TEXT_LEN];                                 // offset file
  struct instrumental_tropomi tropomi;
  int         offsetFlag;
  double      lambdaMin,lambdaMax;
  float       opusTimeShift;
  int         saaConvention;
 }
PRJCT_INSTRUMENTAL;

// ----------------
// RESULTS TAB PAGE
// ----------------

// Description of available fields to output
// -----------------------------------------

typedef struct _prjctResultsFields
 {
  char   fieldName[2*(MAX_ITEM_NAME_LEN+1)];
  int     fieldType;
  int     fieldSize;
  int     fieldDim1,fieldDim2;
  char   fieldFormat[MAX_ITEM_NAME_LEN+1];
 }
PRJCT_RESULTS_FIELDS;

// ASCII results tab page description
// ----------------------------------

struct _prjctAsciiResults {
  char path[MAX_ITEM_TEXT_LEN];                                                 // path for results and fits files
  char newCalibPath[MAX_ITEM_TEXT_LEN];                                         // path for calibrated irradiances
  int   analysisFlag,calibFlag,newcalibFlag,
        referenceFlag,dirFlag,fileNameFlag,successFlag;                         // store results in ascii format
  char fluxes[MAX_ITEM_TEXT_LEN];                                               // fluxes
  double bandWidth;                                                             // averaging bandwidth for fluxes
  // char cic[MAX_ITEM_TEXT_LEN];                                               // color indexes
  int fieldsNumber;                                                             // number of ascii flags set in the next list
  int fieldsFlag[PRJCT_RESULTS_MAX];                                            // fields used in output
  enum output_format file_format;
  char swath_name[HDFEOS_OBJ_LEN_MAX];
};

// Export spectra
// --------------

struct _prjctExport {
  char path[MAX_ITEM_TEXT_LEN];                                                 // path for ASCII files
  int   lambdaFlag,spectraFlag,titlesFlag,directoryFlag;                        // additional options
  int fieldsNumber;                                                             // number of ascii flags set in the next list
  int fieldsFlag[PRJCT_RESULTS_MAX];                                           // fields used in output
} ;

// Buffers specific to CCD

struct _ccd {
  MATRIX_OBJECT drk;
  MATRIX_OBJECT vip;
  MATRIX_OBJECT dnl;
  int           filterNumber;
  double        headTemperature;
  int           measureType;
  float         diodes[4];
  float         targetElevation,targetAzimuth;
  int           saturatedFlag;
  INDEX         indexImage;
  float         wve1,wve2,flux1,flux2;
};

// Record information specific to MAXDOAS format

typedef struct _maxdoasformat
 {
  int scanIndex;           // for FRM4DOAS netCDF file
  int zenithBeforeIndex;   // for FRM4DOAS netCDF file
  int zenithAfterIndex;    // for FRM4DOAS netCDF file
  int measurementType;
 }
MAXDOAS;

// common location data for satellite instruments
struct satellite_location {
  double cornerlats[4], cornerlons[4]; // pixel corner coordinates
  double latitude, longitude; // coordinates of sub-satellite point
  double altitude;
  double earth_radius;
  double sza, saa; // solar zenith/azimuth angles at satellite height
  double vza, vaa;
  double cloud_top_pressure;
  double cloud_fraction;
  int orbit_number;
};

// Record information specific to the GOME format

typedef struct _gomeData                                                        // data on the current GOME pixel
{
  int   pixelNumber;                                                            // pixel number
  int   pixelType;                                                              // pixel type

  SHORT_DATE irradDate;                                                         // date of measurement for the irradiance spectrum
  struct time irradTime;                                                        // time of measurement for the irradiance spectrum

  int     nRef;                                                                 // size of irradiance vectors

  float sza[3];                                                                 // solar zenith angles (East, center and west points of the GOME pixel)
  float azim[3];                                                                // solar azimuth angles (East, center and west points of the GOME pixel)
  float vza[3];
  float vaa[3];
}
GOME_DATA;

// Record information specific to SCIAMACHY

typedef struct _sciamachy
 {
  float  solZen[3],solAzi[3],losZen[3],losAzi[3];                               // resp. solar and line of sight zenith and azimuth angles
  INDEX  stateIndex,stateId;                                                    // information on the state
  int    qualityFlag;
 }
SCIA_DATA;

// Record information specific to GOME2

typedef struct _gome2
{
  double solZen[3],solAzi[3],losZen[3],losAzi[3];                               // resp. solar and line of sight zenith and azimuth angles

  int    saaFlag;
  int    sunglintDangerFlag;
  int    sunglintHighDangerFlag;
  int    rainbowFlag;
  int    scanDirection;
  int    observationMode;
  int    mdrNumber;
  int    observationIndex;
}
GOME2_DATA;

// Record information specific to OMI

typedef struct _omi {
  unsigned short  instrumentConfigurationId;
}
OMI_DATA;

typedef struct _als
 {
  short       alsFlag;
  short       scanIndex;
  double      scanningAngle;
  float       compassAngle;
  float       pitchAngle;
  float       rollAngle;
  char      atrString[1024];
 }
ALS_DATA;

typedef struct _mkzy
 {
  double         scanningAngle;
  double         scanningAngle2;
  char           instrumentname[16];                                            // the name of the instrument
  unsigned short startc;                                                        // the startchannel for the first data-point
  unsigned short pixels;                                                        // number of pixels saved in the data-field
  unsigned char  channel;                                                       // channel of the spectrometer, typically 0
  char           coneangle;                                                     // new in version 4, given in cfg.txt
  int            darkFlag,offsetFlag,skyFlag;                                   // flags indicating the presence resp. of dark current, offset and sky spectra in the file
  int            darkScans,offsetScans;                                         // number of scans of the dark current and the offset
  double         darkTint;
  int            recordNumber;
 }
MKZY_DATA;

typedef struct _mfc
 {
     char    filePath[DOAS_MAX_PATH_LEN+1];
     char   *fileNames;                                                            // in automatic selection of the reference spectrum, maxdoas measurements in MFC format (DOASIS binary or STD), it is important to save
     int     nFiles;                                                               // the names of all files in the directory
     int     resetFlag;
 }
MFC_DOASIS;

typedef struct _mfcBira
 {
     char originalFileName[1024];
     int  measurementType;
 }
MFC_BIRA;

typedef struct uoft_format
 {
  struct date meanDate;                                                         // averaged date
  struct date startDate;                                                        // start date
  struct date endDate;                                                          // end date
  struct time meanTime;                                                         // averaged time
  struct time startTime;                                                        // start time
  struct time endTime;                                                          // end time
  float solarElevAngle;                                                         // averaged solar elevation angle
  float startSolarElevAngle;                                                    // solar elevation angle at the beginning of the measurement
  float endSolarElevAngle;                                                      // solar elevation angle at the end of the measurement
  float shutter;                                                                // shutter
  float numCounts;                                                              // ideal Num of Counts
  float slitWidth;                                                              // slit Width
  float groove;                                                                 // groove Density
  float turret;                                                                 // turret Position
  float blazeWve;                                                               // blaze wavelength
  float centerWve;                                                              // centre wavelength
  float intTime;                                                                // integration Time
  float numAcc;                                                                 // num Accumulations
  float meanCCDT;                                                               // mean CCD temperature
  float minCCDT;                                                                // min CCD temperature
  float maxCCDT;                                                                // max TCCD temperature
  float meanBoxT;                                                               // mean box temperature
  float measType;                                                               // measurement type
  float viewElev;
  float viewAzim;
  float filterId;
  float longitude;
  float latitude;
 }
UOFT_DATA;

typedef struct _airborneBira
 {
     unsigned char servoSentPosition;
     unsigned char servoReceivedPosition;
     float outsideTemp;                                                            // outside temperature
     float insideTemp;                                                             // inside temperature
     float dewPoint;
     float humidity;
     float altitudeP;
     float pressure;
     float longitudeEnd,latitudeEnd,altitudeEnd;
     struct datetime startTime;
     struct datetime endTime;
     // struct time gpsStartTime;    // to delete -> to confirm by Alexis
     // struct time gpsEndTime;

     float  pitch,roll,heading;                                                    // airborne
 }
UAVBIRA_DATA;

// Buffers needed to load spectra

typedef struct _engineBuffers
 {
  // spectra buffers

  double *lambda,                                                               // wavelengths
         *spectrum,                                                             // raw spectrum
         *sigmaSpec,                                                            // error on raw spectrum if any
         *lambda_irrad,                                                         // irradiance spectrum wavelength (for satellite measurements)
         *irrad,                                                                // irradiance spectrum
         *darkCurrent,                                                          // dark current
         *offset,                                                               // offset
         *specMaxx,                                                             // scans numbers to use as absissae for the plot of specMax
         *specMax,                                                              // maxima of signal over scans
         *instrFunction,                                                        // instrumental function
         *varPix,                                                               // variability interpixel
         *scanRef;                                                              // reference spectrum for the scan (MAXDOAS measurements, MKZY format)

  INDEX  *scanIndexes;
  INDEX  *zenithBeforeIndexes;
  INDEX  *zenithAfterIndexes;
  unsigned int *recordIndexes;                                                     // indexes of records for direct access (specific to BIRA-IASB spectra file format)
  unsigned short *pixel_QF;
  MATRIX_OBJECT dnl;                                                            // correction for the non linearity of the detector
 }
BUFFERS;

// Information related to the file

typedef struct _engineFileInfo {
  char   fileName[MAX_STR_LEN+1];                                               // the name of the file
  FILE   *specFp,*darkFp,*namesFp;                                              // file pointers for the engine
  int nScanRef;                                                                 // number of reference spectra in the scanRefIndexes buffer
  SHORT_DATE  startDate;                                                        // date and time of the first record (useful for automatic reference selection when the file covers two days)
  struct time startTime;
}
  FILE_INFO;

// Information on the record

typedef struct _engineRecordInfo
 {
  // Measurement data common to any format

  char   Nom[21];                                                               // name of the spectrum
  int    NSomme;                                                                // total number of scans
  int    rejected;                                                              // number of rejected scans
  double Tint,                                                                  // integration time
         Zm,                                                                    // solar zenith angle
         oldZm,
         Azimuth,                                                               // solar azimut angle
         Tm;                                                                    // date and time in seconds since ...
  char   SkyObs;                                                                // observation of the sky (obsolete)
  float  ReguTemp;                                                              // temperature regulation
  double TotalExpTime;                                                          // total experiment time
  double TotalAcqTime;                                                          // total acquisition time

  struct datetime present_datetime;                                             // measurement date and time

  double TimeDec;                                                               // decimal time
  double localTimeDec;                                                          // local decimal time
  int    localCalDay;                                                           // local calendar day
  double bandWidth;                                                             // color index

  double aMoon,hMoon,fracMoon;                                                  // moon information

  float  zenithViewAngle;                                                       // zenith viewing angle
  float  elevationViewAngle;                                                    // elevation viewing angle
  float  azimuthViewAngle;                                                      // azimuth viewing angle
  float  scanningAngle;                                                         // scanning angle

  // Data related to specific formats
                                                                                // SATELLITES MEASUREMENTS

  int    useErrors;                                                             // 1 if errors are present in the files (GOME)
  int    coolingStatus,mirrorError;                                             // only for OHP measurements (September 2008)
  int    i_alongtrack, i_crosstrack;                                            // for imagers (OMI/APEX/Tropomi/...)
  INDEX  indexBand;                                                             // index of the band

  struct satellite_location satellite;                                          // satellite location data

  // Specific formats

  GOME_DATA gome;                                                               // GOME format
  SCIA_DATA scia;                                                               // SCIAMACHY format
  GOME2_DATA gome2;                                                             // GOME2 format
  OMI_DATA omi;
  ALS_DATA als;
  MKZY_DATA mkzy;
  MFC_DOASIS mfcDoasis;
  MFC_BIRA mfcBira;

  UOFT_DATA uoft;
  UAVBIRA_DATA uavBira;
  CCD    ccd;    // !!! This field should always be the last one -> cfr. ENGINE_CopyContext
  MAXDOAS maxdoas;

  double longitude;                                                             // longitude
  double latitude;                                                              // latitude
  double altitude;                                                              // altitude

                                                                                // SAOZ

  double TDet,                                                                  // temperature of the detector
         BestShift;                                                             // best shift

  int NTracks;                                                                  // Nbre de tracks retenus
  int nSpecMax;                                                                 // number of elements in the SpecMax buffer if any

                                                                                // MFC format

  struct datetime startDateTime;
  struct datetime endDateTime;

  float wavelength1;                                                            // first wavelength
  float dispersion[3];                                                          // dispersion parameters

  // Reference data

  char  refFileName[DOAS_MAX_PATH_LEN+1];
  int    refRecord;
  RC rc;
  
  // quality flag

  unsigned short  ground_pixel_QF;                                            // ground pixel quality flags
  unsigned short  xtrack_QF;                                                  // xtrack quality flags
 }
RECORD_INFO;

typedef struct _engineCalibFeno
 {
  cross_section_list_t crossSectionList;
  struct anlyswin_linear linear;
  struct calibration_sfp sfp[4]; // SFP1 .. SFP4
  shift_stretch_list_t shiftStretchList;
  output_list_t outputList;
 }
CALIB_FENO;

typedef struct _analysisRef
 {
     int    *refIndexes;
     double *zmList;
     double *timeDec;

     int nRef;
     int zmMinIndex;
     int zmMaxIndex;
     int zenBefIndex;
     int zenAftIndex;

     int indexScanBefore;
     int indexScanAfter;

     int refAuto;
     int refScan;
     int refSza;
     int refLon;
 }
ANALYSIS_REF;

struct _slit {
  int    slitType;                                                              // type of line shape (see above)
  int    slitWveDptFlag;                                                        // 1 if the slit function is wavelength dependent (in this case, use files)
  char   slitFile[MAX_STR_LEN+1];                                               // for line shapes provided in file, name of the file (first parameter)
  char   slitFile2[MAX_STR_LEN+1];                                              // for line shapes provided in file, name of the file (second parameter)
  char   slitFile3[MAX_STR_LEN+1];                                              // for line shapes provided in file, name of the file (third parameter)
  double slitParam,slitParam2,slitParam3,slitParam4;                                                       // up to 4 parameters can be provided for the line shape
 };

typedef struct _slitParam
 {
  MATRIX_OBJECT slitMatrix[NSFP];                                               // for wavelength dependent slit functions, we need vectors
  double slitParam[NSFP];                                                       // for non wavelength dependent slit functions, we need constants
  int nslit;                                                                    // the number of parameters used (matrix or constants)
 }
SLIT_MATRIX;

  struct _prjctSlit {
  SLIT  slitFunction;                                  // slit function
  int   fwhmCorrectionFlag;                            // flag set if fwhm correction is to be applied
  char kuruczFile[MAX_STR_LEN+1];
  };

  struct _filter {
  int     type;                                          // type of filter
  float   fwhmWidth;                                     // fwhm width for gaussian
  float   kaiserCutoff;                                  // cutoff frequency for kaiser filter type
  float   kaiserPassBand;                                // pass band for kaiser filter type
  float   kaiserTolerance;                               // tolerance for kaiser filter type
  int     filterOrder;                                   // filter order
  int     filterWidth;                                   // filter width for boxcar, triangle or Savitsky-Golay filters
  int     filterNTimes;                                  // the number of times to apply the filter
  int     filterAction;
  double *filterFunction;
  int     filterSize;
  double  filterEffWidth;
  int     hpFilterCalib;
  int     hpFilterAnalysis;
  };

  typedef struct _project
 {
   char window_name[MAX_ITEM_NAME_LEN];                      // name of window
   char config_file[FILENAME_BUFFER_LENGTH];
   char project_name[PROJECT_NAME_BUFFER_LENGTH];
  PRJCT_SPECTRA spectra;                               // spectra selection tab page
  PRJCT_ANLYS analysis;                                // analysis tab page
  PRJCT_FILTER lfilter;                                // filter (low pass options) tab page
  PRJCT_FILTER hfilter;                                // filter (high pass options) tab page
  PRJCT_KURUCZ kurucz;                                 // Kurucz tab page
  PRJCT_USAMP usamp;                                   // undersampling tab page
  PRJCT_INSTRUMENTAL instrumental;                     // instrumental tab page
  PRJCT_RESULTS asciiResults;                          // ASCII results tab page
  PRJCT_SLIT slit;                                     // slit function tab page
  PRJCT_EXPORT exportSpectra;                          // export spectra
 } PROJECT;

struct _engineContext
 {
  BUFFERS           buffers;                                                    // buffers needed to load spectra
  FILE_INFO         fileInfo;                                                   // the name of the file to load and file pointers
  RECORD_INFO       recordInfo;                                                 // data on the current record
  PROJECT           project;                                                    // data from the current project
  ANALYSIS_REF      analysisRef;

  // record information

  int     recordNumber;                                                         // total number of record in file
  int     recordIndexesSize;                                                    // size of 'recordIndexes' buffer
  int     recordSize;                                                           // size of record if length fixed
  INDEX   indexRecord,indexFile;
  INDEX   currentRecord;
  INDEX   lastRefRecord;
  int     lastSavedRecord;
  int     satelliteFlag;
  int     maxdoasFlag;
  int     maxdoasScanIndexFlag;                                                 // determine the scan index takes time; could be disabled if not requested
  int     mfcDoasisFlag;                                                        // MFC original format generated by DOASIS is very specific with individual files per spectrum
  int     n_alongtrack, n_crosstrack;

  int     refFlag;
  int     radAsRefFlag;

  CALIB_FENO        calibFeno;                                                  // transfer of wavelength calibration options from the project mediator to the analysis mediator
   const char   *outputPath;                                                           // pointer to the output path (from export or output part of the project)
};

#ifdef __cplusplus
}
#endif

#endif
