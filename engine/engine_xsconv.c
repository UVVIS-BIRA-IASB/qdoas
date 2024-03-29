

//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ENGINE CONTEXT
//  Name of module    :  ENGINE.C
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
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//
//  ----------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>

#include "engine_xsconv.h"

// ======================================================================================
// CREATE/DESTROY ENGINE CONTEXT FOR CONVOLUTION TOOLS (convolution, ring, undersampling)
// ======================================================================================

// -----------------------------------------------------------------------------
// FUNCTION      EngineXsconvCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       Create a context for the engine
// -----------------------------------------------------------------------------

ENGINE_XSCONV_CONTEXT *EngineXsconvCreateContext(void)
 {
     // Declaration

     ENGINE_XSCONV_CONTEXT *pEngineContext;                                        // pointer to the engine context

     if ((pEngineContext=(ENGINE_XSCONV_CONTEXT *)malloc(sizeof(ENGINE_XSCONV_CONTEXT)))!=NULL)
      memset(pEngineContext,0,sizeof(ENGINE_XSCONV_CONTEXT));                      // main engine context

  // Return

     return pEngineContext;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineXsconvDestroyContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the context of the current engine
//
//                -> release buffers allocated by EngineCreateXsconvContext
//
// INPUT         pEngineContext     pointer to the engine context
// -----------------------------------------------------------------------------

RC EngineXsconvDestroyContext(ENGINE_XSCONV_CONTEXT *pEngineContext)
 {
     MATRIX_Free(&pEngineContext->xsNew,"EngineXsconvDestroyContext");

  if (pEngineContext->filterVector!=NULL)
   MEMORY_ReleaseDVector("EngineXsconvDestroyContext","filterVector",pEngineContext->filterVector,0);

  if (pEngineContext!=NULL)
   free(pEngineContext);

     return 0;
 }
