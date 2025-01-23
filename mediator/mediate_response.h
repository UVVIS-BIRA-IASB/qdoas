/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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

// Add C++11 default member initializers when if __cplusplus.  For C files, we use the macro below
#if defined(__cplusplus)
#define DEFAULT_INIT(VAL) = VAL
#else
#define DEFAULT_INIT(VAL)
#endif
struct curve_data {
  const char *name DEFAULT_INIT("");
  const double *x, *y;
  size_t length;
  int number DEFAULT_INIT(-1);
  enum eCurveStyleType style DEFAULT_INIT(Line);
};

// macro to "default-initialize" some fields of a plot_data struct in C
#define CURVE(...) ((struct curve_data) { .name="", .number=-1, .style=Line, ##__VA_ARGS__ })

void mediateResponseAddImage(int page,const char *imageFile,void *responseHandle);

int mediateRequestDisplaySpecInfo(void *engineContext,int page,void *responseHandle);

// mediateResponsePlotData
//
// provide the GUI with plots data. The data is contained in an array
// of curve_data of length arrayLength.
void mediateResponsePlotData(int page, const struct curve_data *plotDataArray, int arrayLength,
                             enum ePlotScaleType type, int forceAutoScaling,
                             const char *title, const char *xLabel,
                             const char *yLabel, void *responseHandle);

#define MEDIATE_PLOT_CURVES(page, type, forceAutoScaling, title, xLabel, yLabel, responseHandle, curve_args...) ({ \
      struct curve_data _curves[] = {curve_args};                                \
      mediateResponsePlotData(page, _curves, sizeof(_curves) / sizeof(*_curves), type, forceAutoScaling, title, xLabel, yLabel, responseHandle); \
    })

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
