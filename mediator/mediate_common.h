
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Functions of the mediator that are common to the
//                       QDOAS application and tools
//  Name of module    :  mediate_common.h
//  Creation date     :  07/05/2008
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

#ifndef _MEDIATE_COMMON_
#define _MEDIATE_COMMON_

#include "mediate_general.h"
#include "../engine/comdefs.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

void setMediateSlit(SLIT *pEngineSlit,const mediate_slit_function_t *pMediateSlit);
void setMediateFilter(PRJCT_FILTER *pEngineFilter,const mediate_filter_t *pMediateFilter,int hpFilterFlag,int convoluteFlag);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
