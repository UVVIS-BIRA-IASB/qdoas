/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CPlotDataSet.h"

#include <cstring>

CXYPlotData::CXYPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType curveType,int curveNumber) :
  m_curveName(curveName),
  m_xData(NULL),
  m_yData(NULL),
  m_nSamples(0),
  m_curveNumber(curveNumber),
  m_curveType(curveType)
{
  if (n > 0) {

    // deep copy the data
    m_xData = new double [n];
    m_yData = new double [n];

    memcpy(m_xData, x, n * sizeof(double));
    memcpy(m_yData, y, n * sizeof(double));

    m_nSamples = n;
  }
}

CXYPlotData::~CXYPlotData()
{
  delete [] m_xData;
  delete [] m_yData;

  m_xData=NULL;
  m_yData=NULL;
}

//----------------------------------------------------

CPlotDataSet::CPlotDataSet(enum ePlotScaleType scaleType, bool forceAutoScaling,
               const char *title, const char *xlabel, const char *ylabel) :
  m_scaleType(scaleType),
  m_forceAutoScaling(forceAutoScaling),
  m_title(title),
  m_xLabel(xlabel),
  m_yLabel(ylabel)
{
}

void CPlotDataSet::addPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType curveType)
{
  m_dataList.push_back(new CXYPlotData(curveName, x, y, n, curveType, m_dataList.size()));
}

void CPlotDataSet::addPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType curveType,int curveNumber)
{
  m_dataList.push_back(new CXYPlotData(curveName, x, y, n, curveType, curveNumber));
}


CPlotDataSet::~CPlotDataSet()
{
  for (auto ds : m_dataList) {
    delete ds;
  }
}
