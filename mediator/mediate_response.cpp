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

void mediateResponsePlotData(int page,
                             const struct curve_data *plotDataArray,
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
    if(plotDataArray[i].number >= 0)
      dataSet.addPlotData(plotDataArray[i].name,plotDataArray[i].x, plotDataArray[i].y, plotDataArray[i].length, plotDataArray[i].style, plotDataArray[i].number);
    else
      dataSet.addPlotData(plotDataArray[i].name,plotDataArray[i].x, plotDataArray[i].y, plotDataArray[i].length, plotDataArray[i].style);
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
