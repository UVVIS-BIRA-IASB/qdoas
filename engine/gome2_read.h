#ifndef GOME2_READ_H
#define GOME2_READ_H

#include "comdefs.h"
#include "doas.h"

extern int GOME2_beatLoaded;

void GOME2_ReleaseBuffers(void);

RC GOME2_Set(ENGINE_CONTEXT *pEngineContext);
RC GOME2_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex);
RC GOME2_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);
void GOME2_get_orbit_date(int *year, int *month, int *day);
RC GOME2_get_vza_ref(double scanner_angle, int index_feno, FENO *feno);

#endif
