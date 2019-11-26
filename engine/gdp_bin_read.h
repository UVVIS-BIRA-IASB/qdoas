#ifndef GDP_BIN_READ_H
#define GDP_BIN_READ_H

#include <stdio.h>

#include "doas.h"

RC               GDP_BIN_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);
RC               GDP_BIN_get_orbit_date(int *year, int *month, int *day);
RC GDP_BIN_get_vza_ref(int pixel_type, int index_feno, FENO *feno);
#endif
