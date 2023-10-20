/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include "doas.h"
#include "mediate_convolution.h"
#include "mediate_ring.h"
#include "mediate_usamp.h"

#ifndef _MEDIATE_XSCONV_
#define _MEDIATE_XSCONV_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

extern const char *mediateConvolutionTypesStr[CONVOLUTION_TYPE_MAX];
extern const char *mediateConvolutionFileExt[CONVOLUTION_TYPE_MAX];
extern const char *mediateConvolutionFilterTypes[PRJCT_FILTER_TYPE_MAX];
extern const char *mediateUsampAnalysisMethod[PRJCT_ANLYS_METHOD_MAX];

RC   mediateRequestConvolution(void *engineContext,mediate_convolution_t *pMediateConvolution,void *responseHandle);
RC   mediateConvolutionCalculate(void *engineContext,void *responseHandle);

RC   mediateRequestRing(void *engineContext,mediate_ring_t *pMediateRing,void *responseHandle);
RC   mediateRingCalculate(void *engineContext,void *responseHandle);

RC   mediateRequestUsamp(void *engineContext,mediate_usamp_t *pMediateUsamp,void *responseHandle);
RC   mediateUsampCalculate(void *engineContext,void *responseHandle);

int  mediateXsconvCreateContext(void **engineContext, void *responseHandle);
int  mediateXsconvDestroyContext(void *engineContext, void *responseHandle);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
