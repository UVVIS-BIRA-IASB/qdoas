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


#ifndef _MEDIATE_RESPONSE_H_GUARD
#define _MEDIATE_RESPONSE_H_GUARD

#define MAX_PLOT_SEGMENTS

#include "mediate_types.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

//----------------------------------------------------------
// mediateResponse interface
//----------------------------------------------------------

//----------------------------------------------------------
// Plotting oriented interface
//----------------------------------------------------------

typedef struct plot_data {
  char curveName[80];
  double *x, *y;
  int length;
  int curveNumber;
  enum eCurveStyleType curveType;
} plot_data_t;


void mediateAllocateAndSetPlotData(plot_data_t *d, const char *curveName, const double *xData, const double *yData, int len, enum eCurveStyleType type);
void mediateAllocateAndSetNumberedPlotData(plot_data_t *d, const char *curveName, const double *xData, const double *yData, int len, enum eCurveStyleType type, int curveNumber);
void mediateReleasePlotData(plot_data_t *d);

void mediateResponseAddImage(int page,const char *imageFile,void *responseHandle);

// mediateResponsePlotData
//
// provide the GUI with plots data. The data is contained in an array
// of plot_data of length arrayLength.

int mediateRequestDisplaySpecInfo(void *engineContext,int page,void *responseHandle);

void mediateResponsePlotData(int page, plot_data_t *plotDataArray, int arrayLength,
			     enum ePlotScaleType type, int forceAutoScaling,
			     const char *title, const char *xLabel,
			     const char *yLabel, void *responseHandle);

void mediateResponsePlotImage(int page,const char *imageFile,const char *title,void *responseHandle);


//----------------------------------------------------------
// Data window oriented interface
//----------------------------------------------------------

// mediateResponseCellDataDouble
//
// provide the GUI with a double value for a single cell in a free-format multi-page table. There are no
// predefined limits to the number of pages, nor the number of rows/column on a page.  page can be arbitrary,
// but both row and column index from 0.

void mediateResponseCellDataDouble(int page, int row, int column, double doubleValue, void *responseHandle);


// mediateResponseCellDataDouble
//
// provide the GUI with an integer value for a single cell in a free-format multi-page table. There are no
// predefined limits to the number of pages, nor the number of rows/column on a page.  page can be arbitrary,
// but both row and column index from 0.

void mediateResponseCellDataInteger(int page, int row, int column, int integerValue, void *responseHandle);

// mediateResponseCellDataString
//
// provide the GUI with a string value for a single cell in a free-format multi-page table. There are no
// predefined limits to the number of pages, nor the number of rows/column on a page.  page can be arbitrary,
// but both row and column index from 0.

void mediateResponseCellDataString(int page, int row, int column, const char *stringValue, void *responseHandle);

// mediateResponseCellInfo
//
// provide the GUI with a label and a formatted string distributed in two in a free-format multi-page table. There are no
// predefined limits to the number of pages, nor the number of rows/column on a page.  page can be arbitrary,
// but both row and column index from 0.

void mediateResponseCellInfo(int page, int row, int column, void *responseHandle, const char *label,const char *stringFormat, ...);
void mediateResponseCellInfoNoLabel(int page,int row,int column,void *responseHandle,const char *stringFormat,...);

//----------------------------------------------------------
// Data window oriented interface
//----------------------------------------------------------

// mediateResponseLabelPage
//
// provide the GUI with descriptive information for labeling a page.

void mediateResponseLabelPage(int page, const char *title, const char *tag, void * responseHandle);


//----------------------------------------------------------
// Page retention interface
//----------------------------------------------------------

// mediateResponseRetainPage
//
// instruct the GUI to retain a page set by a previous request.

void mediateResponseRetainPage(int page, void * responseHandle);


//----------------------------------------------------------
// Error message handling
//----------------------------------------------------------

// mediateResponseErrorMessage
//
// allow the mediateRequest* functions to provide error information that might be
// meaningful to a user ...  might be. Should always be in combination with an
// error indicating return code from a mediateRequest* function.
// errorLevel is intended to support the messageString, and not be used for
// programatic error handling logic. The error code from the mediateRequest* function
// must be sufficient for that purpose.

void mediateResponseErrorMessage(const char *function, const char *messageString, enum eEngineErrorType errorType, void *responseHandle);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
