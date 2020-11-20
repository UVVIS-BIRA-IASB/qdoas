/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _MEDIATE_GENERAL_H_GUARD
#define _MEDIATE_GENERAL_H_GUARD

#include "mediate_limits.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif


  /*************************************************************************/
  /* Symbol */

  typedef struct mediate_symbol
  {
    char name[SYMBOL_NAME_BUFFER_LENGTH];
    char description[SYMBOL_DESCR_BUFFER_LENGTH];
  } mediate_symbol_t;


  /*************************************************************************/
  /* Site */

  typedef struct mediate_site
  {
    char name[SITE_NAME_BUFFER_LENGTH];
    char abbreviation[SITE_ABBREV_BUFFER_LENGTH];
    double longitude;
    double latitude;
    double altitude;
  } mediate_site_t;


  /*************************************************************************/
  /* Slits */

  struct slit_file {
       int wveDptFlag;
    char filename[FILENAME_BUFFER_LENGTH];
    char filename2[FILENAME_BUFFER_LENGTH];
  };

  struct slit_gaussian {
       int wveDptFlag;
    double fwhm;
    char filename[FILENAME_BUFFER_LENGTH];
  };

  struct slit_lorentz {
    int wveDptFlag;
    double width;
    int order;
    char filename[FILENAME_BUFFER_LENGTH];
  };

  struct slit_voigt {
       int wveDptFlag;
    double fwhmL, fwhmR;
    double glRatioL, glRatioR;
    char filename[FILENAME_BUFFER_LENGTH];
    char filename2[FILENAME_BUFFER_LENGTH];
  };

  struct slit_error {
       int wveDptFlag;
    double fwhm;
    double width;
    char filename[FILENAME_BUFFER_LENGTH];
    char filename2[FILENAME_BUFFER_LENGTH];
  };

  struct slit_agauss {
       int wveDptFlag;
       double fwhm;
       double asym;
       char filename[FILENAME_BUFFER_LENGTH];
       char filename2[FILENAME_BUFFER_LENGTH];
  };

  struct slit_supergauss {
       int wveDptFlag;
       double fwhm;
       double exponential;
       double asym;
       char filename[FILENAME_BUFFER_LENGTH];
       char filename2[FILENAME_BUFFER_LENGTH];
       char filename3[FILENAME_BUFFER_LENGTH];
  };

  struct slit_apod {
    double resolution;
    double phase;
  };

  struct slit_lorentz_file {
    char filename[FILENAME_BUFFER_LENGTH];
    int degree;
  };

  struct slit_error_file {
    char filename[FILENAME_BUFFER_LENGTH];
    char filename2[FILENAME_BUFFER_LENGTH];
    double width;
  };

  typedef struct mediate_slit_function
  {
    int type;
    struct slit_file file;
    struct slit_gaussian gaussian;
    struct slit_lorentz lorentz;
    struct slit_voigt voigt;
    struct slit_error error;
    struct slit_agauss agauss;
    struct slit_supergauss supergauss;
    struct slit_apod boxcarapod;
    struct slit_apod nbsapod;
    struct slit_file gaussianfile;
    struct slit_lorentz_file lorentzfile;
    struct slit_error_file errorfile;
    // not useful anymore : commented on 12/01/2012 struct slit_file gaussiantempfile;
    // not useful anymore : commented on 12/01/2012 struct slit_error_file errortempfile;
  } mediate_slit_function_t;

  /*************************************************************************/
  /* Filters */

  struct filter_usage
  {
    int calibrationFlag;
    int fittingFlag;
    int divide;
  };

  struct filter_kaiser
  {
    double cutoffFrequency;
    double tolerance;
    double passband;
    int iterations;
    struct filter_usage usage;
  };

  struct filter_boxcar
  {
    int width;            /* odd number of pixels */
    int iterations;
    struct filter_usage usage;
  };

  struct filter_gaussian
  {
    double fwhm;          /* pixels */
    int iterations;
    struct filter_usage usage;
  };

  struct filter_triangular
  {
    int width;            /* odd number of pixels */
    int iterations;
    struct filter_usage usage;
  };

  struct filter_savitzky_golay
  {
    int width;            /* odd number of pixels */
    int order;            /* even number */
    int iterations;
    struct filter_usage usage;
  };

  struct filter_binomial
  {
    int width;            /* odd number of pixels */
    int iterations;
    struct filter_usage usage;
  };

  typedef struct mediate_filter
  {
    int mode;
    struct filter_kaiser kaiser;
    struct filter_boxcar boxcar;
    struct filter_gaussian gaussian;
    struct filter_triangular triangular;
    struct filter_savitzky_golay savitzky;
    struct filter_binomial binomial;
  } mediate_filter_t;


  /*************************************************************************/
  /* Components shared by project and analysis window */

  struct anlyswin_cross_section
  {
    char symbol[SYMBOL_NAME_BUFFER_LENGTH];
    char crossSectionFile[FILENAME_BUFFER_LENGTH];    /* the cross section filename */
    char orthogonal[SYMBOL_NAME_BUFFER_LENGTH];       /* a symbol or predefined constant; add 20 characters for othogonal to/subtract from */
    char subtract[SYMBOL_NAME_BUFFER_LENGTH];
    int subtractFlag;
    int crossType;
    int amfType;
    int correctionType;
    char molecularRing[SYMBOL_NAME_BUFFER_LENGTH];
    char amfFile[FILENAME_BUFFER_LENGTH];
    int requireFit;
    int requireFilter;
    int constrainedCc;
    int requireCcFit;
    double initialCc;
    double deltaCc;
    double ccIo;
    double ccMin;
    double ccMax;
  };


struct anlyswin_linear
  {
    int xPolyOrder;
    int xBaseOrder;
    int xFlagFitStore;
    int xFlagErrStore;

    int offsetPolyOrder;
    int offsetFlagFitStore;
    int offsetFlagErrStore;

    int offsetI0;
  };

// Caro : would be nice to replace struct calibration_sfp with an array of structures of this type
//        more flexible and could be processed as struct calibration_sfp

// typedef struct _AnalyseNonLinearParameters
//  {
//      char symbolName[MAX_ITEM_TEXT_LEN+1];
//      char crossFileName[MAX_ITEM_TEXT_LEN+1];
//      int fitFlag;
//      double initialValue;
//      double deltaValue;
//      double minValue;
//      double maxValue;
//      int storeFit;
//      int storeError;
//  }
// ANALYSE_NON_LINEAR_PARAMETERS;

  struct anlyswin_nonlinear
  {
    int solFlagFit;
    double solInitial;
    double solDelta;
    int solFlagFitStore;
    int solFlagErrStore;

    int off0FlagFit;
    double off0Initial;
    double off0Delta;
    int off0FlagFitStore;
    int off0FlagErrStore;

    int off1FlagFit;
    double off1Initial;
    double off1Delta;
    int off1FlagFitStore;
    int off1FlagErrStore;

    int off2FlagFit;
    double off2Initial;
    double off2Delta;
    int off2FlagFitStore;
    int off2FlagErrStore;

    int comFlagFit;
    double comInitial;
    double comDelta;
    int comFlagFitStore;
    int comFlagErrStore;

    int usamp1FlagFit;
    double usamp1Initial;
    double usamp1Delta;
    int usamp1FlagFitStore;
    int usamp1FlagErrStore;

    int usamp2FlagFit;
    double usamp2Initial;
    double usamp2Delta;
    int usamp2FlagFitStore;
    int usamp2FlagErrStore;

    int resolFlagFit;
    double resolInitial;
    double resolDelta;
    int resolFlagFitStore;
    int resolFlagErrStore;

    char comFile[FILENAME_BUFFER_LENGTH];
    char usamp1File[FILENAME_BUFFER_LENGTH];
    char usamp2File[FILENAME_BUFFER_LENGTH];
    char ramanFile[FILENAME_BUFFER_LENGTH];
  };

  struct anlyswin_shift_stretch
  {
    int nSymbol;
    char symbol[MAX_AW_SHIFT_STRETCH][SYMBOL_NAME_BUFFER_LENGTH];
    int shFit;
    int stFit;
    int shStore;
    int stStore;
    int errStore;
    double shInit;
    double stInit;
    double stInit2;
    double shDelta;
    double stDelta;
    double stDelta2;
    double shMin;
    double shMax;
  };

  struct anlyswin_gap
  {
    double minimum;
    double maximum;
  };

  struct anlyswin_output
  {
    char symbol[SYMBOL_NAME_BUFFER_LENGTH];
    int amf;
    double resCol;
    // residual
    int slantCol;
    int slantErr;
    double slantFactor;
    int vertCol;
    int vertErr;
    double vertFactor;
  };

// Caro : would be nice to replace struct calibration_sfp with an array of structures of this type
//        more flexible and could be processed as struct anlyswin_nonlinear

// typedef struct _AnalyseNonLinearParameters
//  {
//      char symbolName[MAX_ITEM_TEXT_LEN+1];
//      char crossFileName[MAX_ITEM_TEXT_LEN+1];
//      int fitFlag;
//      double initialValue;
//      double deltaValue;
//      double minValue;
//      double maxValue;
//      int storeFit;
//      int storeError;
//  }
// ANALYSE_NON_LINEAR_PARAMETERS;

  struct calibration_sfp
  {
    int fitFlag;
    double initialValue;
    double deltaValue;
    int fitStore;
    int errStore;
  };


  /*************************************************************************/
  /* struct array wrappers ... */

  typedef struct cross_section_list
  {
    int nCrossSection;
    struct anlyswin_cross_section crossSection[MAX_AW_CROSS_SECTION];
  } cross_section_list_t;

  typedef struct shift_stretch_list
  {
    int nShiftStretch;
    struct anlyswin_shift_stretch shiftStretch[MAX_AW_SHIFT_STRETCH];
  } shift_stretch_list_t;

  typedef struct gap_list
  {
    int nGap;
    struct anlyswin_gap gap[MAX_AW_GAP];
  } gap_list_t;

  typedef struct output_list
  {
    int nOutput;
    struct anlyswin_output output[MAX_AW_CROSS_SECTION];
  } output_list_t;

  typedef struct data_select_list
  {
    int nSelected;
    int selected[256];
  } data_select_list_t;

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
