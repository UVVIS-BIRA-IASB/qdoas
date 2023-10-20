
//  ----------------------------------------------------------------------------
//! \addtogroup Convolution
//! @{
//!
//! \file      raman.h
//! \brief     Raman convolution - header of raman.c
//! \details   This module contains the prototype of the function used to
//!            convolve cross sections/solar spectra with Raman effect
//!
//! \authors   Original functions come from Kelly Chance's Fortran 77 program.\n
//!            See : "Ring effect studies : Rayleigh scattering, including molecular
//!            parameters for rotational Raman scattering and the Franhofer
//!            spectrum, K.Chance and R.J.D.Spurr, Applied optics, Vol. 36,
//!            Issue 21, pp. 5224-5230 (1997)"\n
//!            They have been adapted and improved for QDOAS by Caroline FAYT/Michel Van Roozendael
//! \date      1998 : initial functions provided from Kelly Chance and converted from Fortran
//! \date      2007 : separation between the engine and the user interface with QDOAS
//! \date      2010 : improve the performance for the calculation of normalised Ring cross-sections
//! \date      2018 : create a separate raman_convolution function to also convolve cross sections for molecular ring
//!
//! @}
//  ----------------------------------------------------------------------------
//
//  =========
//  FUNCTIONS
//  =========
//
//  raman_n2 - N2 Raman function
//  raman_o2 - O2 Raman function
//
//  raman_convolution : convolve a cross section with Raman effect
//  ----------------------------------------------------------------------------
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

#ifndef RAMAN_H
#define RAMAN_H

#include "constants.h"
#include "doas.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// Constants definitions

#define RING_SLIT_WIDTH (double)6.

// Prototypes

RC raman_convolution(double *xsLambda,double *xsVector,double *xsDeriv2,double *xsConv,int n,double temp,int normalizeFlag);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
