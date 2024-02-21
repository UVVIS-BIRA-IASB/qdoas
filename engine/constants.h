
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  CONSTANTS DEFINITIONS COMMON TO BOTH ENGINE AND GUI
//  Name of module    :  CONSTANTS.H
//  Creation date     :  14 August 2007
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
//
//  MODULE DESCRIPTION
//
//  This module contains all the definitions that are common to both the user
//  interface and the engine
//
//  ----------------------------------------------------------------------------

#if !defined(__CONSTANTS_)
#define __CONSTANTS_

// =====================
// CONSTANTS DEFINITIONS
// =====================

// Indexes

#define NO_SELECTED_FILE    (INDEX) -2
#define ORTHOGONAL_BASE     (INDEX) -3

// Standard arrays dimensions

#define STRING_LENGTH 1023                                                      // maximum size for strings
#define MAX_GB_RECORDS 4000

#define EPSILON     (double)   1.e-6

// ================
// OPERATING MODES
// ================

enum _thrdId {
  THREAD_TYPE_NONE,
  THREAD_TYPE_SPECTRA,     // for browse spectra
  THREAD_TYPE_EXPORT,      // for export spectra
  THREAD_TYPE_ANALYSIS,    // for run analysis
  THREAD_TYPE_KURUCZ       // for run calibration
};

// ================
// FILES PROCESSING
// ================

// ------------------------------------
// CONSTANTS AND STRUCTURES DEFINITIONS
// ------------------------------------

#define FILES_PATH_MAX 100

// Common dialog open mode
// -----------------------

enum _filesOpenMode
 {
  FILE_MODE_OPEN,
  FILE_MODE_SAVE
 };

// All supported types of files
// ----------------------------

enum _filesTypes
 {
  FILE_TYPE_ALL,                                  // all files (*.*)
  FILE_TYPE_ASCII,                                // ASCII files (*.asc)
  FILE_TYPE_ASCII_SPECTRA,                        // ASCII spectra files (*.asc)
  FILE_TYPE_SPECTRA,                              // spectra files (*.spe)
  FILE_TYPE_CROSS,                                // cross sections files (*.xs)
  FILE_TYPE_REF,                                  // reference spectra files (*.ref)
  FILE_TYPE_AMF_SZA,                              // SZA dependent AMF files (*.amf_sza)
  FILE_TYPE_AMF_CLI,                              // Climatology dependent AMF files (*.amf_cli)
  FILE_TYPE_AMF_WVE,                              // Wavelength dependent AMF files (*.amf_wve)
  FILE_TYPE_NAMES,                                // spectra names files (*.nms)
  FILE_TYPE_DARK,                                 // dark current files (*.drk)
  FILE_TYPE_INTERPIXEL,                           // interpixel variability files (*.vip)
  FILE_TYPE_NOT_LINEARITY,                        // detector not linearity files (*.dnl)
  FILE_TYPE_CALIB,                                // calibration files (*.clb)
  FILE_TYPE_CALIB_KURUCZ,                         // Kurucz files (*.ktz)
  FILE_TYPE_SLIT,                                 // slit function (*.slf)
  FILE_TYPE_INSTR,                                // instrumental function (*.ins)
  FILE_TYPE_FILTER,                               // filter files (*.flt)
  FILE_TYPE_FIT,                                  // fits files (*.fit*)
  FILE_TYPE_QDOAS,                                // QDOAS settings (*.xml)
  FILE_TYPE_BMP,                                  // bitmap files (*.bmp)
  FILE_TYPE_RES,                                  // residuals (*.res)
  FILE_TYPE_PATH,                                 // paths
  FILE_TYPE_CFG,                                  // config
  FILE_TYPE_NETCDF,                               // netCDF (*.nc) 
  FILE_TYPE_MAX
 };

// Individual spectra safe keeping
// -------------------------------

enum _filesTypesSpectra {
  FILE_TYPE_SPECTRA_COMMENT,
  FILE_TYPE_SPECTRA_NOCOMMENT,
  FILE_TYPE_SPECTRA_MAX
};

// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

// Analysis windows
// ----------------

// -----------
// DEFINITIONS
// -----------

enum _crossDiffOrtho {
  ANLYS_DIFFORTHO_NONE,
  ANLYS_DIFFORTHO_DIFFERENTIAL_XS,
  ANLYS_DIFFORTHO_ORTHOGONALIZATION,
  ANLYS_DIFFORTHO_SUBTRACTION,
  ANLYS_DIFFORTHO_MAX
};

enum _polynomeTypes {
  ANLYS_POLY_TYPE_NONE,
  ANLYS_POLY_TYPE_0,
  ANLYS_POLY_TYPE_1,
  ANLYS_POLY_TYPE_2,
  ANLYS_POLY_TYPE_3,
  ANLYS_POLY_TYPE_4,
  ANLYS_POLY_TYPE_5,
  ANLYS_POLY_TYPE_6,
  ANLYS_POLY_TYPE_7,
  ANLYS_POLY_TYPE_8,
  ANLYS_POLY_TYPE_MAX
};

enum _shiftTypes
 {
  ANLYS_SHIFT_TYPE_NONE,
  ANLYS_SHIFT_TYPE_NONLINEAR
  // ANLYS_SHIFT_TYPE_LINEAR
 };

enum _stretchTypes {
  ANLYS_STRETCH_TYPE_NONE,
  ANLYS_STRETCH_TYPE_FIRST_ORDER,
  ANLYS_STRETCH_TYPE_SECOND_ORDER,
  ANLYS_STRETCH_TYPE_MAX
};

enum _crossTypes {
  ANLYS_CROSS_ACTION_NOTHING,
  ANLYS_CROSS_ACTION_INTERPOLATE,
  ANLYS_CROSS_ACTION_CONVOLUTE,
  ANLYS_CROSS_ACTION_CONVOLUTE_I0,
  ANLYS_CROSS_ACTION_CONVOLUTE_RING,
  ANLYS_CROSS_ACTION_MAX
};

enum _amfTypes {
  ANLYS_AMF_TYPE_NONE,
  ANLYS_AMF_TYPE_SZA,
  ANLYS_AMF_TYPE_CLIMATOLOGY,
  ANLYS_AMF_TYPE_WAVELENGTH,
  ANLYS_AMF_TYPE_MAX
};

enum _correctionTypes {
  ANLYS_CORRECTION_TYPE_NONE,
  ANLYS_CORRECTION_TYPE_SLOPE,
  ANLYS_CORRECTION_TYPE_PUKITE,
  ANLYS_CORRECTION_TYPE_MOLECULAR_RING,
  ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE,
  ANLYS_CORRECTION_TYPE_MAX
};

enum _moleculesCombo {
  ANLYS_COMBO_NONE,
  ANLYS_COMBO_DIFF_ORTHO,
  ANLYS_COMBO_CORRECTION
};

// Analysis tab pages description
// ------------------------------

enum _refSpectrumSelectionMode {
  ANLYS_REF_SELECTION_MODE_AUTOMATIC,                      // automatic selection from spectra files
  ANLYS_REF_SELECTION_MODE_FILE                            // reference spectrum in a specific file
};

enum _maxdoasRefSelectionMode {
  ANLYS_MAXDOAS_REF_SZA,
  ANLYS_MAXDOAS_REF_SCAN
};

// Additional options for the reference of the scan selection mode

enum _maxdoasScanMode {
     ANLYS_MAXDOAS_REF_SCAN_BEFORE,                                                // use the zenith before the scan
     ANLYS_MAXDOAS_REF_SCAN_AFTER,                                                 // use the zenith after the scan
     ANLYS_MAXDOAS_REF_SCAN_AVERAGE,                                               // average the zenith spectra before and after the scan
     ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE                                            // interpolate the zenith spectra before and after the scan at the current measurement time
};

enum _kuruczMode {
  ANLYS_KURUCZ_NONE,
  ANLYS_KURUCZ_REF,
  ANLYS_KURUCZ_SPEC,
  ANLYS_KURUCZ_REF_AND_SPEC,
  ANLYS_KURUCZ_MAX
};

// ===================
// PROJECTS PROPERTIES
// ===================

// ----------------
// SPECTRA TAB PAGE
// ----------------

// Spectra tab page description
// ----------------------------

enum _prjctSpectraModes {
  PRJCT_SPECTRA_MODES_NONE,
  PRJCT_SPECTRA_MODES_CIRCLE,
  PRJCT_SPECTRA_MODES_RECTANGLE,
  PRJCT_SPECTRA_MODES_OBSLIST,
  PRJCT_SPECTRA_MODES_MAX
};

// -----------------
// ANALYSIS TAB PAGE
// -----------------

// Analysis methods
// ----------------

enum prjctAnlysMethod {
  OPTICAL_DENSITY_FIT,                              // Optical thickness fitting (SVD)
  INTENSITY_FIT,                     // Intensity fitting (Marquardt-Levenberg+SVD)
  PRJCT_ANLYS_METHOD_MAX
};

// Least-squares fit weighting
// ---------------------------

enum prjctAnlysFitWeighting {
  PRJCT_ANLYS_FIT_WEIGHTING_NONE,                      // no weighting
  PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL,              // instrumental weighting
  PRJCT_ANLYS_FIT_WEIGHTING_MAX
};

// Interpolation
// -------------

enum prjctAnlysInterpol {
  PRJCT_ANLYS_INTERPOL_LINEAR,                         // linear interpolation
  PRJCT_ANLYS_INTERPOL_SPLINE,                         // spline interpolation
  PRJCT_ANLYS_INTERPOL_MAX
};

// ---------------
// FILTER TAB PAGE
// ---------------

// Filter types
// ------------

enum prjctFilterTypes {
  PRJCT_FILTER_TYPE_NONE,                              // use no filter
  PRJCT_FILTER_TYPE_KAISER,                            // kaiser filter
  PRJCT_FILTER_TYPE_BOXCAR,                            // box car filter
  PRJCT_FILTER_TYPE_GAUSSIAN,                          // gaussian filter
  PRJCT_FILTER_TYPE_TRIANGLE,                          // triangular filter
  PRJCT_FILTER_TYPE_SG,                                // savitzky-Golay filter
  PRJCT_FILTER_TYPE_ODDEVEN,                           // odd-even pixel correction
  PRJCT_FILTER_TYPE_BINOMIAL,                          // binomial filter
  PRJCT_FILTER_TYPE_MAX
};

// Filter action
// -------------

enum prjctFilterOutput {
  PRJCT_FILTER_OUTPUT_LOW,
  PRJCT_FILTER_OUTPUT_HIGH_SUB,
  PRJCT_FILTER_OUTPUT_HIGH_DIV,
  PRJCT_FILTER_OUTPUT_MAX
};

// --------------------
// CALIBRATION TAB PAGE
// --------------------

// Calibration tab page description
// --------------------------------

#define NSFP 3
#define MAX_CALIB_WINDOWS  50

enum _prjctFwhmTypes {
  PRJCT_CALIB_FWHM_TYPE_NONE,
  PRJCT_CALIB_FWHM_TYPE_FILE,
  PRJCT_CALIB_FWHM_TYPE_GAUSS,
  PRJCT_CALIB_FWHM_TYPE_ERF,
  PRJCT_CALIB_FWHM_TYPE_INVPOLY,
  PRJCT_CALIB_FWHM_TYPE_VOIGT,
  PRJCT_CALIB_FWHM_TYPE_AGAUSS,
  PRJCT_CALIB_FWHM_TYPE_SUPERGAUSS,
  PRJCT_CALIB_FWHM_TYPE_MAX
};

enum _prjctCalibWindows {
      PRJCT_CALIB_WINDOWS_CONTIGUOUS,
      PRJCT_CALIB_WINDOWS_SLIDING,
      PRJCT_CALIB_WINDOWS_CUSTOM
};

// ----------------------
// UNDERSAMPLING TAB PAGE
// ----------------------

enum _prjctUsampMethod {
  PRJCT_USAMP_FILE,                                    // no undersampling fitting
  PRJCT_USAMP_FIXED,                                   // undersampling fitting, fixed phase
  PRJCT_USAMP_AUTOMATIC,                               // undersampling fitting, automatic phase
  PRJCT_USAMP_MAX
};

// ---------------------
// INSTRUMENTAL TAB PAGE
// ---------------------

// type of read out format
// -----------------------

enum _prjctInstrFormat {
  PRJCT_INSTR_FORMAT_ASCII,                                                     //  1 ASCII
#ifdef PRJCT_INSTR_FORMAT_OLD
  PRJCT_INSTR_FORMAT_LOGGER,                                                    //  2 Logger (PDA,CCD or HAMAMATSU)
  PRJCT_INSTR_FORMAT_ACTON,                                                     //  3 Acton (NILU)
  PRJCT_INSTR_FORMAT_PDAEGG,                                                    //  4 PDA EG&G (sept. 94 until now)
  PRJCT_INSTR_FORMAT_PDAEGG_OLD,                                                //  5 PDA EG&G (spring 94)
  PRJCT_INSTR_FORMAT_CCD_OHP_96,                                                //  6 CCD (OHP 96)
  PRJCT_INSTR_FORMAT_CCD_HA_94,                                                 //  7 CCD (HARESTUA 94)
#endif  
  PRJCT_INSTR_FORMAT_SAOZ_VIS,                                                  //  8 SAOZ visible
  PRJCT_INSTR_FORMAT_SAOZ_EFM,                                                  //  9 SAOZ EFM (1024)
  PRJCT_INSTR_FORMAT_MFC,                                                       // 10 MFC Heidelberg
  PRJCT_INSTR_FORMAT_MFC_STD,                                                   // 11 MFC Heidelberg
  PRJCT_INSTR_FORMAT_MFC_BIRA,                                                  // 12 MFC BIRA-IASB
#ifdef PRJCT_INSTR_FORMAT_OLD  
  PRJCT_INSTR_FORMAT_RASAS,                                                     // 13 RASAS (INTA)
  PRJCT_INSTR_FORMAT_PDASI_EASOE,                                               // 14 EASOE
#endif  
  PRJCT_INSTR_FORMAT_CCD_EEV,                                                   // 15 CCD EEV
  PRJCT_INSTR_FORMAT_GDP_BIN,                                                   // 16 GOME GDP BINARY format
  PRJCT_INSTR_FORMAT_SCIA_PDS,                                                  // 17 SCIAMACHY Calibrated Level 1 data in PDS format
  PRJCT_INSTR_FORMAT_UOFT,                                                      // 18 University of Toronto
  PRJCT_INSTR_FORMAT_NOAA,                                                      // 19 NOAA
  PRJCT_INSTR_FORMAT_OMI,                                                       // 20 OMI
  PRJCT_INSTR_FORMAT_TROPOMI,                                                   // 21 Tropomi
  PRJCT_INSTR_FORMAT_GOME2,                                                     // 22 GOME2
  PRJCT_INSTR_FORMAT_MKZY,                                                      // 23 MANNE Kihlman and ZHANG Yan pak format
  PRJCT_INSTR_FORMAT_BIRA_AIRBORNE,                                             // 24 ULMDOAS (BIRA-IASB)
  PRJCT_INSTR_FORMAT_BIRA_MOBILE,                                               // 25 MOBILEDOAS (BIRA-IASB)
  PRJCT_INSTR_FORMAT_APEX,                                                      // 26 APEX NetCDF files (BIRA-IASB/VITO)
  PRJCT_INSTR_FORMAT_OCEAN_OPTICS,                                              // 27 Ocean optics
  PRJCT_INSTR_FORMAT_OMPS,                                                      // 28 OMPS
  PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF,                                           // 29 netCDF format for FRM4DOAS
  PRJCT_INSTR_FORMAT_GOME1_NETCDF,                                              // 30 netCDF format for GOME1
  PRJCT_INSTR_FORMAT_GEMS,                                                      // 31 GEMS format (Geostationary Environment Monitoring Spectrometer onboard GEO-KOMPSAT-2B geostationary satellite)
  PRJCT_INSTR_FORMAT_MAX
};

enum _prjctInstrType {
  PRJCT_INSTR_TYPE_GROUND_BASED,
  PRJCT_INSTR_TYPE_SATELLITE,
  PRJCT_INSTR_TYPE_AIRBORNE,
  PRJCT_INSTR_TYPE_MAX
};

enum _prjctInstrSaaConvention {
  PRJCT_INSTR_SAA_SOUTH,
  PRJCT_INSTR_SAA_NORTH
};

enum _maxdoasSpectrumTypes {
  PRJCT_INSTR_MAXDOAS_TYPE_NONE,
  PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS,
  PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN,
  PRJCT_INSTR_MAXDOAS_TYPE_ZENITH,
  PRJCT_INSTR_MAXDOAS_TYPE_DARK,
  PRJCT_INSTR_MAXDOAS_TYPE_LAMP,
  PRJCT_INSTR_MAXDOAS_TYPE_BENTHAM,
  PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR,
  PRJCT_INSTR_MAXDOAS_TYPE_OFFSET,
  PRJCT_INSTR_MAXDOAS_TYPE_AZIMUTH,
  PRJCT_INSTR_MAXDOAS_TYPE_PRINCIPALPLANE,
  PRJCT_INSTR_MAXDOAS_TYPE_HORIZON,
  PRJCT_INSTR_MAXDOAS_TYPE_MOON,
  PRJCT_INSTR_MAXDOAS_TYPE_MAX
};

enum _ulbCurveTypes
 {
  PRJCT_INSTR_ULB_TYPE_MANUAL,
  PRJCT_INSTR_ULB_TYPE_HIGH,
  PRJCT_INSTR_ULB_TYPE_LOW,
  PRJCT_INSTR_ULB_TYPE_MAX
 };

enum _saozSpectrumRegion {
   PRJCT_INSTR_SAOZ_REGION_UV,
   PRJCT_INSTR_SAOZ_REGION_VIS
};

enum _saozSpectrumTypes {
  PRJCT_INSTR_SAOZ_TYPE_ZENITHAL,
  PRJCT_INSTR_SAOZ_TYPE_POINTED,
  PRJCT_INSTR_SAOZ_TYPE_MAX
};

enum _iasbSpectrumTypes {
  PRJCT_INSTR_IASB_TYPE_ALL,
  PRJCT_INSTR_IASB_TYPE_ZENITHAL,
  PRJCT_INSTR_IASB_TYPE_OFFAXIS,
  PRJCT_INSTR_IASB_TYPE_MAX
};

enum _niluFormatTypes {
  PRJCT_INSTR_NILU_FORMAT_OLD,
  PRJCT_INSTR_NILU_FORMAT_NEW,
  PRJCT_INSTR_NILU_FORMAT_MAX
};

enum _asciiFormat
 {
  PRJCT_INSTR_ASCII_FORMAT_LINE,
  PRJCT_INSTR_ASCII_FORMAT_COLUMN,
  PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED
 };

enum _gdpBandTypes
 {
  PRJCT_INSTR_GDP_BAND_1A,
  PRJCT_INSTR_GDP_BAND_1B,
  PRJCT_INSTR_GDP_BAND_2A,
  PRJCT_INSTR_GDP_BAND_2B,
  PRJCT_INSTR_GDP_BAND_3,
  PRJCT_INSTR_GDP_BAND_4,
  PRJCT_INSTR_GDP_BAND_MAX
 };

enum _gdpPixelTypes {
  PRJCT_INSTR_GDP_PIXEL_ALL,
  PRJCT_INSTR_GDP_PIXEL_EAST,
  PRJCT_INSTR_GDP_PIXEL_CENTER,
  PRJCT_INSTR_GDP_PIXEL_WEST,
  PRJCT_INSTR_GDP_PIXEL_BACKSCAN,
  PRJCT_INSTR_GDP_PIXEL_MAX
};

enum _gome1PixelTypes {
  PRJCT_INSTR_GOME1_PIXEL_ALL,
  PRJCT_INSTR_GOME1_PIXEL_GROUND,
  PRJCT_INSTR_GOME1_PIXEL_BACKSCAN,
  PRJCT_INSTR_GOME1_PIXEL_MAX
};

enum _sciaChannels {
  PRJCT_INSTR_SCIA_CHANNEL_1,
  PRJCT_INSTR_SCIA_CHANNEL_2,
  PRJCT_INSTR_SCIA_CHANNEL_3,
  PRJCT_INSTR_SCIA_CHANNEL_4,
  PRJCT_INSTR_SCIA_CHANNEL_MAX
};

enum _omiSpectralTypes {
  PRJCT_INSTR_OMI_TYPE_UV1,
  PRJCT_INSTR_OMI_TYPE_UV2,
  PRJCT_INSTR_OMI_TYPE_VIS,
  PRJCT_INSTR_OMI_TYPE_MAX
};

#define MAX_SWATHSIZE 2048 // maximum number of tracks per swath (for satellites)

// ----------------
// RESULTS TAB PAGE
// ----------------

// CWOutputSelector is called from display, output and export pages

enum _outputSelectorOrigin
 {
  TAB_SELECTOR_DISPLAY,
  TAB_SELECTOR_OUTPUT,
  TAB_SELECTOR_EXPORT
 };

// Possible fields to store
// ------------------------

enum _prjctResults
 {
  PRJCT_RESULTS_SPECNO,
  PRJCT_RESULTS_NAME,
  PRJCT_RESULTS_DATE_TIME,
  PRJCT_RESULTS_DATE,
  PRJCT_RESULTS_TIME,
  PRJCT_RESULTS_YEAR,
  PRJCT_RESULTS_JULIAN,
  PRJCT_RESULTS_JDFRAC,
  PRJCT_RESULTS_TIFRAC,
  PRJCT_RESULTS_SCANS,
  PRJCT_RESULTS_NREJ,
  PRJCT_RESULTS_TINT,
  PRJCT_RESULTS_SZA,
  PRJCT_RESULTS_CHI,
  PRJCT_RESULTS_RMS,
  PRJCT_RESULTS_AZIM,
  PRJCT_RESULTS_TDET,
  PRJCT_RESULTS_SKY,
  PRJCT_RESULTS_BESTSHIFT,
  PRJCT_RESULTS_REFZM,
  PRJCT_RESULTS_REFNUMBER,
  PRJCT_RESULTS_REFNUMBER_BEFORE,
  PRJCT_RESULTS_REFNUMBER_AFTER,
  PRJCT_RESULTS_REFSHIFT,
  PRJCT_RESULTS_PIXEL,
  PRJCT_RESULTS_PIXEL_TYPE,
  PRJCT_RESULTS_ORBIT,
  PRJCT_RESULTS_LONGIT,
  PRJCT_RESULTS_LATIT,
  PRJCT_RESULTS_LON_CORNERS,
  PRJCT_RESULTS_LAT_CORNERS,
  PRJCT_RESULTS_ALTIT,
  PRJCT_RESULTS_COVAR,
  PRJCT_RESULTS_CORR,
  PRJCT_RESULTS_CLOUD,
  PRJCT_RESULTS_O3,
  PRJCT_RESULTS_NO2,
  PRJCT_RESULTS_CLOUDTOPP,
  PRJCT_RESULTS_LOS_ZA,
  PRJCT_RESULTS_LOS_AZIMUTH,
  PRJCT_RESULTS_SAT_HEIGHT,
  PRJCT_RESULTS_SAT_LAT,
  PRJCT_RESULTS_SAT_LON,
  PRJCT_RESULTS_SAT_SAA,
  PRJCT_RESULTS_SAT_SZA,
  PRJCT_RESULTS_SAT_VZA,
  PRJCT_RESULTS_EARTH_RADIUS,
  PRJCT_RESULTS_VIEW_ELEVATION,
  PRJCT_RESULTS_VIEW_AZIMUTH,
  PRJCT_RESULTS_VIEW_ZENITH,
  PRJCT_RESULTS_SCIA_QUALITY,
  PRJCT_RESULTS_SCIA_STATE_INDEX,
  PRJCT_RESULTS_SCIA_STATE_ID,
  PRJCT_RESULTS_STARTDATE,
  PRJCT_RESULTS_ENDDATE,
  PRJCT_RESULTS_STARTTIME,
  PRJCT_RESULTS_ENDTIME,
  PRJCT_RESULTS_SCANNING,
  PRJCT_RESULTS_FILTERNUMBER,
  PRJCT_RESULTS_MEASTYPE,
  PRJCT_RESULTS_CCD_HEADTEMPERATURE,
  PRJCT_RESULTS_COOLING_STATUS,
  PRJCT_RESULTS_MIRROR_ERROR,
  PRJCT_RESULTS_COMPASS,
  PRJCT_RESULTS_PITCH,
  PRJCT_RESULTS_ROLL,
  PRJCT_RESULTS_ITER,
  PRJCT_RESULTS_ERROR_FLAG,
  PRJCT_RESULTS_NUM_BANDS,
  PRJCT_RESULTS_GOME2_MDR_NUMBER,
  PRJCT_RESULTS_GOME2_OBSERVATION_INDEX,
  PRJCT_RESULTS_GOME2_SCANDIRECTION,
  PRJCT_RESULTS_GOME2_OBSERVATION_MODE,
  PRJCT_RESULTS_GOME2_SAA,
  PRJCT_RESULTS_GOME2_SUNGLINT_RISK,
  PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK,
  PRJCT_RESULTS_GOME2_RAINBOW,
  PRJCT_RESULTS_CCD_DIODES,
  PRJCT_RESULTS_CCD_TARGETAZIMUTH,
  PRJCT_RESULTS_CCD_TARGETELEVATION,
  PRJCT_RESULTS_SATURATED,
  PRJCT_RESULTS_INDEX_CROSSTRACK,
  PRJCT_RESULTS_INDEX_ALONGTRACK,
  PRJCT_RESULTS_GROUNDP_QF,
  PRJCT_RESULTS_XTRACK_QF,
  PRJCT_RESULTS_PIXELS_QF,
  PRJCT_RESULTS_OMI_CONFIGURATION_ID,
  PRJCT_RESULTS_SPIKES,
  PRJCT_RESULTS_UAV_SERVO_BYTE_SENT,
  PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED,
  PRJCT_RESULTS_UAV_INSIDE_TEMP,
  PRJCT_RESULTS_UAV_OUTSIDE_TEMP,
  PRJCT_RESULTS_UAV_PRESSURE,
  PRJCT_RESULTS_UAV_HUMIDITY,
  PRJCT_RESULTS_UAV_DEWPOINT,
  PRJCT_RESULTS_UAV_PITCH,
  PRJCT_RESULTS_UAV_ROLL,
  PRJCT_RESULTS_UAV_HEADING,
  PRJCT_RESULTS_SLANT_COL,
  PRJCT_RESULTS_SLANT_ERR,
  PRJCT_RESULTS_SHIFT,
  PRJCT_RESULTS_SHIFT_ERR,
  PRJCT_RESULTS_LAMBDA_CENTER,
  PRJCT_RESULTS_STRETCH,
  PRJCT_RESULTS_STRETCH_ERR,
  PRJCT_RESULTS_SCALE,
  PRJCT_RESULTS_SCALE_ERR,
  PRJCT_RESULTS_PARAM,
  PRJCT_RESULTS_PARAM_ERR,
  PRJCT_RESULTS_AMF,
  PRJCT_RESULTS_VERT_COL,
  PRJCT_RESULTS_VERT_ERR,
  PRJCT_RESULTS_FLUX,
  PRJCT_RESULTS_CIC,
  PRJCT_RESULTS_WAVELENGTH,
  PRJCT_RESULTS_PRECALCULATED_FLUXES,
  PRJCT_RESULTS_STARTGPSTIME,
  PRJCT_RESULTS_ENDGPSTIME,
  PRJCT_RESULTS_LONGITEND,
  PRJCT_RESULTS_LATITEND,
  PRJCT_RESULTS_ALTITEND,
  PRJCT_RESULTS_TOTALEXPTIME,
  PRJCT_RESULTS_TOTALACQTIME,
  PRJCT_RESULTS_LAMBDA,
  PRJCT_RESULTS_SPECTRA,
  PRJCT_RESULTS_FILENAME,
  PRJCT_RESULTS_SCANINDEX,
  PRJCT_RESULTS_ZENITH_BEFORE,
  PRJCT_RESULTS_ZENITH_AFTER,
  PRJCT_RESULTS_RC,
  PRJCT_RESULTS_RESIDUAL_SPECTRUM,
  PRJCT_RESULTS_MAX            // addition/deletion of new fields impact changes in ascii-qdoas (ascFieldsNames)
 };

// --------------------------
// PROPERTY SHEET DESCRIPTION
// --------------------------

enum prjctTabPagesTypes
 {
  TAB_TYPE_PRJCT_SPECTRA,                              // Spectra selection tab page
  TAB_TYPE_PRJCT_ANLYS,                                // Analysis tab page
  TAB_TYPE_PRJCT_FILTER,                               // Filter tab page
  TAB_TYPE_PRJCT_CALIBRATION,                          // Calibration tab page
  TAB_TYPE_PRJCT_USAMP,                                // Undersampling tab page
  TAB_TYPE_PRJCT_INSTRUMENTAL,                         // Instrumental tab page
  TAB_TYPE_PRJCT_SLIT,                                 // Slit function tab page
  TAB_TYPE_PRJCT_ASCII_RESULTS,                        // ASCII Results tab page
  TAB_TYPE_PRJCT_MAX
 };

// ===============================
// CROSS SECTIONS CONVOLUTION TOOL
// ===============================

// ---------------------
// CONSTANTS DEFINITIONS
// ---------------------

// Supported line shapes for convolution
// -------------------------------------

enum _slitTypes {
  SLIT_TYPE_NONE,
  SLIT_TYPE_FILE,                                                            // user-defined line shape provided in a file
  SLIT_TYPE_GAUSS,                                                           // Gaussian line shape
  SLIT_TYPE_INVPOLY,                                                         // 2n-Lorentz (generalisation of the Lorentzian function
  SLIT_TYPE_VOIGT,                                                           // Voigt profile function
  SLIT_TYPE_ERF,                                                             // error function (convolution of a Gaussian and a boxcar)
  SLIT_TYPE_AGAUSS,                                                          // asymmetric gaussian line shape
  SLIT_TYPE_SUPERGAUSS,                                                      // super gaussian line shape
  SLIT_TYPE_APOD,                                                            // apodisation function (used with FTS)
  SLIT_TYPE_APODNBS,                                                         // apodisation function (Norton Beer Strong function)
  SLIT_TYPE_MAX
};

// Convolution type
// ----------------

enum _convolutionTypes {
  CONVOLUTION_TYPE_NONE,                                                     // no convolution, interpolation only
  CONVOLUTION_TYPE_STANDARD,                                                 // standard convolution
  CONVOLUTION_TYPE_I0_CORRECTION,                                            // convolution using I0 correction
  // CONVOLUTION_TYPE_RING,                                                    // creation of a ring xs using high-resoluted solar and raman spectra
  CONVOLUTION_TYPE_MAX
};

// Conversion modes
// ----------------

enum _conversionModes {
  CONVOLUTION_CONVERSION_NONE,                                                  // no conversion
  CONVOLUTION_CONVERSION_AIR2VAC,                                               // air to vacuum
  CONVOLUTION_CONVERSION_VAC2AIR,                                               // vacuum to air
  CONVOLUTION_CONVERSION_MAX
};

// Output format
// -------------

enum _convolutionFormat {
  CONVOLUTION_FORMAT_ASCII,                                                     // ASCII
  CONVOLUTION_FORMAT_NETCDF,                                                    // netCDF
  CONVOLUTION_FORMAT_MAX
};

// Pages of the dialog box
// -----------------------

enum _convolutionTabPages {
  TAB_TYPE_XSCONV_GENERAL,                                                   // general information (files names, convolution type...)
  TAB_TYPE_XSCONV_SLIT,                                                      // information on the slit function
  TAB_TYPE_XSCONV_FILTER,                                                    // information on the filter to apply
  TAB_TYPE_XSCONV_MAX
};

enum omi_xtrack_mode {
  XTRACKQF_IGNORE = 0,
  XTRACKQF_STRICT = 1,
  XTRACKQF_NONSTRICT = 2
};

extern const char *STR_IGNORE, *STR_STRICT, *STR_NONSTRICT;
extern enum omi_xtrack_mode str_to_mode(const char *configstr);
#endif
