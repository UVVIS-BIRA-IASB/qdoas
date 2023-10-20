/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _MEDIATE_LIMITS_H_GUARD
#define _MEDIATE_LIMITS_H_GUARD

#define SITE_NAME_BUFFER_LENGTH     128
#define SITE_ABBREV_BUFFER_LENGTH     8

#define FILENAME_BUFFER_LENGTH      256
#define SYMBOL_NAME_BUFFER_LENGTH    32
#define SYMBOL_DESCR_BUFFER_LENGTH  256
#define FLUX_BUFFER_LENGTH          256
#define COLOUR_INDEX_BUFFER_LENGTH  256
#define ANLYSWIN_NAME_BUFFER_LENGTH  64
#define PROJECT_NAME_BUFFER_LENGTH 64
#define TRACK_SELECTION_LENGTH      256

#define MAX_AW_CROSS_SECTION         16
#define MAX_AW_SHIFT_STRETCH         MAX_AW_CROSS_SECTION+2                     // the total number of cross sections + spectrum + ref (note that practically, it will never possible to shift all cross sections separately !)
#define MAX_AW_GAP                   8                                          // usually max 1 or 2 gaps

#endif
