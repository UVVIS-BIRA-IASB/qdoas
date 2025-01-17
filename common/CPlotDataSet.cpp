/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CPlotDataSet.h"

#include <cstring>

CXYPlotData::CXYPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType curveType,int curveNumber) :
  m_curveName(curveName),
  m_curveNumber(curveNumber),
  m_curveType(curveType)
{
  m_xData.reserve(n);
  m_yData.reserve(n);
  for (int i=0; i!=n; ++i) {
    m_xData.push_back(x[i]);
    m_yData.push_back(y[i]);
  }
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
