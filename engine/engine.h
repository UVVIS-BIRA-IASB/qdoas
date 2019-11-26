#ifndef _ENGINE_H_GUARD
#define _ENGINE_H_GUARD

#include "doas.h"

// =======================
// DEFINITION OF CONSTANTS
// =======================

// ===================
// GLOBAL DECLARATIONS
// ===================

extern int            ENGINE_refStartDate;                                      // automatic reference selection : 0 use localday of records, 1 use starting date and time of the first measurements
extern char           ENGINE_dbgFile[DOAS_MAX_PATH_LEN+1];                      // debug file
extern ENGINE_CONTEXT ENGINE_contextRef;                                        // copy of the engine context for the automatic search of the reference spectrum
extern double         ENGINE_localNoon;                                         // local noon


// ==========
// PROTOTYPES
// ==========

RC              EngineCopyContext(ENGINE_CONTEXT *pEngineContextTarget,ENGINE_CONTEXT *pEngineContextSource);
RC              EngineSetProject(ENGINE_CONTEXT *pEngineContext);
RC              EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,int dateFlag,int localCalDay);
RC              EngineRequestBeginBrowseSpectra(ENGINE_CONTEXT *pEngineContext,const char *spectraFileName,void *responseHandle);
RC              EngineRequestEndBrowseSpectra(ENGINE_CONTEXT *pEngineContext);
RC              EngineNewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle);
RC              EngineBuildScanIndex(ENGINE_CONTEXT *pEngineContext);
RC              EngineEndCurrentSession(ENGINE_CONTEXT *pEngineContext);
ENGINE_CONTEXT *EngineCreateContext(void);
RC              EngineDestroyContext(ENGINE_CONTEXT *pEngineContext);

#endif
