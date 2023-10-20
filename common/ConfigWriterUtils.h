/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CONFIGWRITERUTILS_H_GUARD
#define _CONFIGWRITERUTILS_H_GUARD

#include <cstdio>

#include "mediate_general.h"

void writePaths(FILE *fp);
void writeFilter(FILE *fp, size_t nIndent, const char *passband, const mediate_filter_t *d);
void writeSlitFunction(FILE *fp, size_t nIndex, const mediate_slit_function_t *d);

#endif
