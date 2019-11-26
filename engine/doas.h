
//
//  Product/Project   :  QDOAS
//  Name of module    :  DOAS.H
//  Creation date     :  1997
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
//
//  MODULE DESCRIPTION
//
//  The present module contains definitions used by most of the DOAS routines
//
//  ----------------------------------------------------------------------------

/*! \file doas.h main header file */

#ifndef DOAS_H
#define DOAS_H

#include "output_formats.h"
#include "comdefs.h"
#include "constants.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define MAX_FEN                20    // maximum number of gaps in a analysis window
#define MAX_FIT                50    // maximum number of parameters to fit
#define MAX_FENO               25    // maximum number of analysis windows in a project
#define MAX_SYMB              150    // maximum number of different symbols in a project
#define MAX_SPECMAX          5000    // maximum number of items in SpecMax

// NOT USED ANYMORE ? CONFLICT WITH NETCDF #define DIM                    10    // default number of security pixels for border effects

#define MAX_KURUCZ_FWHM_PARAM   4    // maximum number of non linear parameters for fitting fwhm with Kurucz

#define STOP      ITEM_NONE

typedef struct _prjctAnlys PRJCT_ANLYS;
typedef struct _prjctKurucz PRJCT_KURUCZ;
typedef struct _prjctUsamp PRJCT_USAMP;
typedef struct _prjctSlit PRJCT_SLIT;

typedef struct _prjctAsciiResults PRJCT_RESULTS;
typedef struct _prjctExport PRJCT_EXPORT;

typedef struct _matrix MATRIX_OBJECT;

typedef struct _FFT FFT;

typedef struct _feno FENO;
typedef struct _KuruczFeno KURUCZ_FENO;
typedef struct _Kurucz KURUCZ;

typedef struct _wrkSymbol WRK_SYMBOL;
typedef struct _crossResults CROSS_RESULTS;

typedef struct _ccd CCD;
typedef struct _engineContext ENGINE_CONTEXT;

// ----------------
// GLOBAL VARIABLES
// ----------------
extern int NWorkSpace;
extern int NDET[MAX_SWATHSIZE];
extern int NFeno,SvdPDeb,SvdPFin;
extern WRK_SYMBOL   *WorkSpace;

extern PRJCT_ANLYS  *pAnalysisOptions;             // analysis options
extern PRJCT_KURUCZ *pKuruczOptions;               // Kurucz options
extern PRJCT_SLIT   *pSlitOptions;                 // slit function options
extern PRJCT_USAMP  *pUsamp;

static inline bool is_satellite(enum _prjctInstrFormat format) {
  return (format==PRJCT_INSTR_FORMAT_GDP_BIN ||
          format==PRJCT_INSTR_FORMAT_GEMS ||
          format==PRJCT_INSTR_FORMAT_GOME1_NETCDF ||
          format==PRJCT_INSTR_FORMAT_SCIA_PDS ||
          format==PRJCT_INSTR_FORMAT_OMI ||
          format==PRJCT_INSTR_FORMAT_OMPS ||
          format==PRJCT_INSTR_FORMAT_TROPOMI ||
          format==PRJCT_INSTR_FORMAT_GOME2);
}

static inline bool is_maxdoas(enum _prjctInstrFormat format) {
  return (format==PRJCT_INSTR_FORMAT_ASCII ||                                                   //  0 ASCII
          format==PRJCT_INSTR_FORMAT_BIRA_MOBILE || 
          format==PRJCT_INSTR_FORMAT_MFC ||                                                     // 12 MFC Heidelberg
          format==PRJCT_INSTR_FORMAT_MFC_STD ||                                                 // 13 MFC Heidelberg
          format==PRJCT_INSTR_FORMAT_MFC_BIRA ||                                                // 14 MFC BIRA-IASB
          format==PRJCT_INSTR_FORMAT_CCD_EEV ||                                                 // 18 CCD EEV
          format==PRJCT_INSTR_FORMAT_UOFT ||                                                    // 23 University of Toronto
          format==PRJCT_INSTR_FORMAT_NOAA ||                                                    // 24 NOAA
          format==PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF);                                          // 34 netCDF format for FRM4DOAS
}

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
