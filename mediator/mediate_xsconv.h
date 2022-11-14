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

#include "mediate_convolution.h"
#include "mediate_ring.h"
#include "mediate_usamp.h"
#include "../engine/engine_xsconv.h"

#ifndef _MEDIATE_XSCONV_
#define _MEDIATE_XSCONV_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

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
