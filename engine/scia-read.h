#ifndef SCIA_READ_H
#define SCIA_READ_H

void SCIA_ReleaseBuffers(char format);
RC   SCIA_SetPDS(ENGINE_CONTEXT *pEngineContext);
RC   SCIA_ReadPDS(ENGINE_CONTEXT *pEngineContext,int recordNo);
void SCIA_get_orbit_date(int *year, int *month, int *day);
RC SCIA_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);
RC SCIA_get_vza_ref(double esm_pos, int index_feno, FENO *feno);

#endif
