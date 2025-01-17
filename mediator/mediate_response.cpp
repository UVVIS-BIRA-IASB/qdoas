/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "mediate_response.h"

#include "CEngineResponse.h"
#include "CPlotDataSet.h"

using std::string;

void mediateAllocateAndSetPlotData(plot_data_t *d, const char *curveName, const double *xData, const double *yData, int len, enum eCurveStyleType type)
{
  d->curveNumber = -1;

  int byteLen = len * sizeof(double);

  d->x = (double*)malloc(byteLen);
  d->y = (double*)malloc(byteLen);

  strcpy(d->curveName,curveName);

  if (d->x == NULL || d->y == NULL) {
    // bail out ...
    mediateReleasePlotData(d);
    d->x = d->y = NULL;
    d->length = 0;
    d->curveType = type;
  }
  else {
    memcpy(d->x, xData, byteLen);
    memcpy(d->y, yData, byteLen);
    d->length = len;
    d->curveType = type;
  }
}

void mediateAllocateAndSetNumberedPlotData(plot_data_t *d, const char *curveName, const double *xData, const double *yData, int len, enum eCurveStyleType type, int curveNumber)
{
  d->curveNumber = curveNumber;
  int byteLen = len * sizeof(double);

  d->x = (double*)malloc(byteLen);
  d->y = (double*)malloc(byteLen);

  strcpy(d->curveName,curveName);

  if (d->x == NULL || d->y == NULL) {
    // bail out ...
    mediateReleasePlotData(d);
    d->x = d->y = NULL;
    d->length = 0;
    d->curveType = type;
  }
  else {
    memcpy(d->x, xData, byteLen);
    memcpy(d->y, yData, byteLen);
    d->length = len;
    d->curveType = type;
  }
}

void mediateReleasePlotData(plot_data_t *d)
{
  d->length=0;
  strcpy(d->curveName,"");

  free(d->x);
  free(d->y);
}


void mediateResponsePlotData(int page,
                 plot_data_t *plotDataArray,
                 int arrayLength,
                 enum ePlotScaleType type,
                 int forceAutoScaling,
                 const char *title,
                 const char *xLabel,
                 const char *yLabel,
                 void *responseHandle)
{
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);

  CPlotDataSet dataSet(type, forceAutoScaling, title, xLabel, yLabel);

  int i = 0;
  while (i < arrayLength) {
    if(plotDataArray[i].curveNumber >= 0)
      dataSet.addPlotData(plotDataArray[i].curveName,plotDataArray[i].x, plotDataArray[i].y, plotDataArray[i].length, plotDataArray[i].curveType, plotDataArray[i].curveNumber);
    else
      dataSet.addPlotData(plotDataArray[i].curveName,plotDataArray[i].x, plotDataArray[i].y, plotDataArray[i].length, plotDataArray[i].curveType);
    ++i;
  }

  resp->addDataSet(page, std::move(dataSet));
}

void mediateResponsePlotImage(int page,const char *imageFile,const char *title,void *responseHandle)
 {
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);

  resp->addImage(page, CPlotImage(imageFile,title));
 }

void mediateResponseCellDataDouble(int page,
                   int row,
                   int column,
                   double doubleValue,
                   void *responseHandle)
{
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);
  resp->addCell(page, row, column, cell_data(doubleValue));
}

void mediateResponseCellDataInteger(int page,
                    int row,
                    int column,
                    int integerValue,
                    void *responseHandle)
{
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);
  resp->addCell(page, row, column, cell_data(integerValue));
}

void mediateResponseCellDataString(int page,
                   int row,
                   int column,
                   const char *stringValue,
                   void *responseHandle)
{
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);
  resp->addCell(page, row, column, cell_data(string(stringValue)));
}

void mediateResponseCellInfo(int page,
                   int row,
                   int column,
                   void *responseHandle,
                   const char *label,
                   const char *stringFormat,...)
 {
   va_list argList;
   char stringValue[1024];

  va_start(argList,stringFormat);
  vsprintf(stringValue,stringFormat,argList);
  va_end(argList);

  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);

  resp->addCell(page,row,column,cell_data(string(label)));
  resp->addCell(page,row,column+1,cell_data(string(stringValue)));
 }

void mediateResponseCellInfoNoLabel(int page,
                   int row,
                   int column,
                   void *responseHandle,
                   const char *stringFormat,...)
 {
   va_list argList;
   char stringValue[1024];

   va_start(argList,stringFormat);
   vsprintf(stringValue,stringFormat,argList);
   va_end(argList);

   CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);
   resp->addCell(page,row,column,cell_data(string(stringValue)));
 }

void mediateResponseLabelPage(int page,
                  const char *title,
                  const char *tag,
                  void *responseHandle)
{
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);
  resp->addPageTitleAndTag(page, title, tag);
}

void mediateResponseRetainPage(int page, void * responseHandle)
{
  CEngineResponseVisual *resp = static_cast<CEngineResponseVisual*>(responseHandle);
  // an invalid cell position
  resp->addCell(page, -1, -1, cell_data(nullptr));
  // an empty data set
  resp->addDataSet(page, CPlotDataSet(Spectrum, forceAutoScale, "", "", ""));
}

void mediateResponseErrorMessage(const char *function,
                 const char *messageString,
                 enum eEngineErrorType errorType,
                 void *responseHandle)
{
  CEngineResponse *resp = static_cast<CEngineResponse*>(responseHandle);
  resp->addErrorMessage(function, messageString, errorType);
}
