/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _MEDIATE_PROJECT_H_GUARD
#define _MEDIATE_PROJECT_H_GUARD

#include "mediate_limits.h"
#include "mediate_general.h"
#include "constants.h"
#include "output_formats.h"

#include "tropomi.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif



  /****************************************************/
  /* Geolocation */

  /* cGeolocationModeCircle */
  struct geolocation_circle
  {
    double radius;
    double centerLongitude;
    double centerLatitude;
  };

  /* cGeolocationModeRectangle */
  struct geolocation_rectangle
  {
    double westernLongitude;
    double easternLongitude;
    double southernLatitude;
    double northernLatitude;
  };

  /* cGeolocationModeSites */
  struct geolocation_sites
  {
    double radius;
  };

  /* all gelocations and a selection (mode) */
  struct geolocation
  {
    int mode;
    struct geolocation_circle circle;
    struct geolocation_rectangle rectangle;
    struct geolocation_sites sites;
  };

  /****************************************************/
  /* Project Spectra */

  typedef struct mediate_project_spectra
  {
    /* Data required from engine for Display purposes */
    int requireSpectra;
    int requireData;
    int requireCalib;
    int requireFits;

    /* SZA (Solar Zenith Angle) range of interest */
    double szaMinimum;
    double szaMaximum;
    double szaDelta;

    /* Spectral record range */
    int recordNumberMinimum;
    int recordNumberMaximum;

    /* boolean flags for separate dark and name files */
    int useDarkFile;
    int useNameFile;

    /* geolocation limits */
    struct geolocation geo;

  } mediate_project_spectra_t;


  /****************************************************/
  /* Project Display */

  typedef struct mediate_project_display
  {
    /* Data required from engine for Display purposes */
    int requireSpectra;
    int requireCalib;
    int requireData;
    int requireFits;
    data_select_list_t selection;
  } mediate_project_display_t;


  /****************************************************/
  /* Project Selection */

  typedef struct mediate_project_selection
  {
    /* SZA (Solar Zenith Angle) range of interest */
    double szaMinimum;
    double szaMaximum;
    double szaDelta;

    /* Spectral record range */
    int recordNumberMinimum;
    int recordNumberMaximum;

    /* ref angle */
    double refAngle;
    double refTolerance;

    /* Viewing elevation angle range of interest */
    double elevationMinimum;
    double elevationMaximum;
    double elevationTolerance;

    // Cloud fraction

    double cloudFractionMinimum;
    double cloudFractionMaximum;

    /* boolean flags for separate dark and name files */
    int useDarkFile;
    int useNameFile;

    /* geolocation limits */
    struct geolocation geo;

  } mediate_project_selection_t;


  /****************************************************/
  /* Project Analysis */

  typedef struct mediate_project_analysis
  {
    int methodType;
    int fitType;
    int interpolationType;
    int interpolationSecurityGap;
    double convergenceCriterion;
    double spike_tolerance;
    int maxIterations;
  } mediate_project_analysis_t;


  /****************************************************/
  /* Project Filtering - mediate_filter_t */


  /****************************************************/
  /* Project Calibration */

  typedef struct mediate_project_calibration
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    char slfFile[FILENAME_BUFFER_LENGTH];
    int methodType;
    int subWindows;
    int lineShape;
    int lorentzOrder;
    int shiftDegree;
    int sfpDegree;
    double wavelengthMin;
    double wavelengthMax;
    double customLambdaMin[MAX_CALIB_WINDOWS];
    double customLambdaMax[MAX_CALIB_WINDOWS];
    int divisionMode;
    int requireSpectra;
    int requireFits;
    int requireResidual;
    int requireShiftSfp;
    int preshiftFlag;
    double windowSize;
    double preshiftMin;
    double preshiftMax;
    /* table data ... */
    cross_section_list_t crossSectionList;
    struct anlyswin_linear linear;
    struct calibration_sfp sfp[4]; // SFP1 .. SFP4
    shift_stretch_list_t shiftStretchList;
    gap_list_t gapList;
    output_list_t outputList;

  } mediate_project_calibration_t;


  /****************************************************/
  /* Project Undersampling */

  typedef struct mediate_project_undersampling
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    int method;
    double shift;
  } mediate_project_undersampling_t;


  /****************************************************/
  /* Project Instrumental */

  struct instrumental_ascii
  {
    int detectorSize;
    int format;
    int flagZenithAngle;
    int flagAzimuthAngle;
    int flagElevationAngle;
    int flagDate;
    int flagTime;
    int flagWavelength;
    int  straylight;
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_logger {
    int spectralType;
    int flagAzimuthAngle;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_acton {
    int niluType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_saoz {
    int spectralRegion;
    int spectralType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_mfc {
    int detectorSize;
    int revert;
    int autoFileSelect;
    int firstWavelength;
    unsigned int offsetMask;
    unsigned int instrFctnMask;
    unsigned int spectraMask;
    unsigned int darkCurrentMask;
    int  straylight;
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char darkCurrentFile[FILENAME_BUFFER_LENGTH];
    char offsetFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_mfcstd {
    int  detectorSize;
    int  revert;
    int  straylight;
    char dateFormat[24];
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char darkCurrentFile[FILENAME_BUFFER_LENGTH];
    char offsetFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_mfcbira {
    int  detectorSize;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    int  straylight;
    double lambdaMin,lambdaMax;
  };

  struct instrumental_minimum {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    int  straylight;
    double lambdaMin,lambdaMax;
  };

  struct instrumental_ccd {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char interPixelVariabilityFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_ccdeev {
    int  detectorSize;
    int  spectralType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char imagePath[FILENAME_BUFFER_LENGTH];
    char straylightCorrectionFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
    int  straylight;
    double lambdaMin,lambdaMax;
  };

  struct instrumental_gdp {
    int bandType;
    int pixelType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_gome2 {
    int bandType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_scia {
    int  channel;
    char clusters[32];                      // flags with cluster number as index
    char sunReference[4];                            // 2 characters plus terminator
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_omi {
    int spectralType;
    double minimumWavelength;
    double maximumWavelength;
    int flagAverage;
    int  pixelQFRejectionFlag;
    int  pixelQFMaxGaps;
    int  pixelQFMask;
    enum omi_xtrack_mode xtrack_mode;
    char trackSelection[TRACK_SELECTION_LENGTH];
  };

  struct instrumental_apex {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char trackSelection[TRACK_SELECTION_LENGTH];
  };

  struct instrumental_oceanoptics {
    int  detectorSize;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    int  straylight;
    double lambdaMin,lambdaMax;
  };

  struct instrumental_bira_airborne
  {
    int  straylight;
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_bira_mobile
  {
    int  straylight;
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_frm4doas
  {
    int  averageRows;
    int  detectorSize;
    int  spectralType;
    int  straylight;
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_avantes
  {
    int  detectorSize;
    int  straylight;
    double lambdaMin,lambdaMax;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_gems {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char transmissionFunctionFile[FILENAME_BUFFER_LENGTH];
    char trackSelection[TRACK_SELECTION_LENGTH];
    int  binning;
  };

  typedef struct mediate_project_instrumental
   {
    int format;
    char siteName[SITE_NAME_BUFFER_LENGTH];
    int  saaConvention;
    struct instrumental_ascii ascii;
    struct instrumental_logger logger;
    struct instrumental_acton acton;
    struct instrumental_logger pdaegg;
    struct instrumental_logger pdaeggold;
    struct instrumental_ccd ccdohp96;
    struct instrumental_ccd ccdha94;
    struct instrumental_saoz saozvis;
    struct instrumental_minimum saozefm;
    struct instrumental_mfc mfc;
    struct instrumental_mfcstd mfcstd;
    struct instrumental_mfcbira mfcbira;
    struct instrumental_minimum rasas;
    struct instrumental_minimum pdasieasoe;
    struct instrumental_ccdeev ccdeev;
    struct instrumental_gdp gdpbin;
    struct instrumental_gdp gdpnetcdf;
    struct instrumental_scia sciapds;
    struct instrumental_minimum uoft;
    struct instrumental_minimum noaa;
    struct instrumental_omi omi;
    struct instrumental_tropomi tropomi;
    struct instrumental_gome2 gome2;
    struct instrumental_minimum mkzy;
    struct instrumental_avantes biraairborne;
    struct instrumental_avantes biramobile;
    struct instrumental_apex apex;
    struct instrumental_oceanoptics oceanoptics;
    struct instrumental_frm4doas frm4doas;
    struct instrumental_gems gems;
   }
  mediate_project_instrumental_t;


  /****************************************************/
  /* Project Slit */

  typedef struct mediate_project_slit
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    int applyFwhmCorrection;
    mediate_slit_function_t function;      /* the selected slit and properties for all slit types */
  } mediate_project_slit_t;


  /****************************************************/
  /* Project Output */

  typedef struct mediate_project_output
  {
    int analysisFlag;
    int calibrationFlag;
    int newcalibFlag;
    int referenceFlag; // write components of automatic reference to output
    int configurationFlag;
    enum output_format file_format;
   char swath_name[SWATH_NAME_LEN_MAX];  // for HDF-EOS5 output
    int directoryFlag;
    int filenameFlag;
    int successFlag;      // write only successful records (default)
    char flux[FLUX_BUFFER_LENGTH];
    double bandWidth;
    char colourIndex[COLOUR_INDEX_BUFFER_LENGTH]; // colour index is the ratio of two fluxes
   char path[FILENAME_BUFFER_LENGTH];
    char newCalibPath[FILENAME_BUFFER_LENGTH];  // path for calibrated irradiances (satellites)
    /* result field flags. A list of PRJCT_RESULTS_ASCII_*** ... */
    data_select_list_t selection;
  } mediate_project_output_t;

  typedef struct mediate_project_export
   {
    int titlesFlag; // write components of automatic reference to output
    int directoryFlag;
    char path[FILENAME_BUFFER_LENGTH];
    data_select_list_t selection;
   } mediate_project_export_t;

  /* mediate_project_t
   *
   * Contains all user-specified information about a project. It allows the GUI to
   * provide information to the engine.
   */

  typedef struct mediate_project
  {
    /* Coupled to the control offered by the GUI Project Tabs. */
    char project_name[PROJECT_NAME_BUFFER_LENGTH];
    mediate_project_spectra_t spectra;
    mediate_project_display_t display;
    mediate_project_selection_t selection;
    mediate_project_analysis_t analysis;
    mediate_filter_t lowpass;
    mediate_filter_t highpass;
    mediate_project_calibration_t calibration;
    mediate_project_undersampling_t undersampling;
    mediate_project_instrumental_t instrumental;
    mediate_project_slit_t slit;
    mediate_project_output_t output;
    mediate_project_export_t export_spectra;
  } mediate_project_t;


  /****************************************************/
  /* Helper functions */

  void initializeMediateProject(mediate_project_t *d, const char *config_file, const char *project_name);
  void initializeMediateProjectDisplay(mediate_project_display_t *d);
  void initializeMediateProjectSelection(mediate_project_selection_t *d);
  void initializeMediateProjectAnalysis(mediate_project_analysis_t *d);
  void initializeMediateProjectFiltering(mediate_filter_t *d);
  void initializeMediateProjectCalibration(mediate_project_calibration_t *d);
  void initializeMediateProjectUndersampling(mediate_project_undersampling_t *d);
  void initializeMediateProjectInstrumental(mediate_project_instrumental_t *d);
  void initializeMediateProjectSlit(mediate_project_slit_t *d);
  void initializeMediateProjectOutput(mediate_project_output_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
