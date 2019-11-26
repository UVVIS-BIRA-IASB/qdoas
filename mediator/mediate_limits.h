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
