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


#ifndef _CPLOTDATASET_H_GUARD
#define _CPLOTDATASET_H_GUARD

#include <QString>
#include <QList>
// #include <QImage>

#include "mediate_types.h"


// a set of (increasing x ordered) x,y points that define a single 'curve' on a plot.

class CXYPlotData
{
 public:
  CXYPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType type);
  CXYPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType type, int curveNumber);
  ~CXYPlotData();

  enum eCurveStyleType curveType(void) const;
  const QString& curveName(void) const;
  const double* xRawData(void) const;
  const double* yRawData(void) const;
  int curveNumber(void) const;
  int size(void) const;

 private:
  QString m_curveName;
  double *m_xData, *m_yData;
  int m_nSamples;
  int m_curveNumber;
  enum eCurveStyleType m_curveType;
};

inline enum eCurveStyleType CXYPlotData::curveType(void) const { return m_curveType; }
inline const QString& CXYPlotData::curveName(void) const { return m_curveName; }
inline const double* CXYPlotData::xRawData(void) const { return m_xData; }
inline const double* CXYPlotData::yRawData(void) const { return m_yData; }
inline int CXYPlotData::size(void) const { return m_nSamples; }
inline int CXYPlotData::curveNumber(void) const { return m_curveNumber; }



// A collection of curves for one plot and plot labels/titles

class CPlotDataSet
{
 public:
  CPlotDataSet(enum ePlotScaleType type, bool forceAutoScaling, const char *title, const char *xlabel, const char *ylabel);
  ~CPlotDataSet();

  void addPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType type);
  void addPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType curveType, int curveNumber);

  int count(void) const;
  const CXYPlotData& rawData(int index) const;
  enum eCurveStyleType curveType(int index) const;

  enum ePlotScaleType scaleType(void) const;
  bool forceAutoScaling(void) const;
  const QString& plotTitle(void) const;
  const QString& xAxisLabel(void) const;
  const QString& yAxisLabel(void) const;

 private:
  QList<CXYPlotData*> m_dataList;
  enum ePlotScaleType m_scaleType;
  bool m_forceAutoScaling;
  QString m_file,m_title, m_xLabel, m_yLabel;
};

inline int CPlotDataSet::count(void) const { return m_dataList.count(); }
inline const CXYPlotData& CPlotDataSet::rawData(int index) const { return *(m_dataList.at(index)); }
inline enum eCurveStyleType CPlotDataSet::curveType(int index) const { return m_dataList.at(index)->curveType(); }
inline enum ePlotScaleType CPlotDataSet::scaleType(void) const { return m_scaleType; }
inline bool CPlotDataSet::forceAutoScaling(void) const { return m_forceAutoScaling; }
inline const QString& CPlotDataSet::plotTitle(void) const { return m_title; }
inline const QString& CPlotDataSet::xAxisLabel(void) const { return m_xLabel; }
inline const QString& CPlotDataSet::yAxisLabel(void) const { return m_yLabel; }


// structures to assist in the collation of data bundled in a response from the engine.
// specifically for control of the page distribution of plot.

struct SPlotData
{
  int page;
  const CPlotDataSet *data;

  SPlotData(int p, const CPlotDataSet *d) : page(p), data(d) {}
};

struct STitleTag
{
  int page;
  QString title;
  QString tag;

  STitleTag(int p, const QString &ti, const QString &ta) : page(p), title(ti), tag(ta) {}
};

struct SImage
{
//  int page;
//  QString imageFilename;
//  QImage image;
//
//  SImage(int p,const QString &fn,const QImage &img) : page(p),imageFilename(fn),image(img) {}

};

#endif
