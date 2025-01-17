/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA
*/


#ifndef _CPLOTDATASET_H_GUARD
#define _CPLOTDATASET_H_GUARD

#include "mediate_types.h"

#include <string>
#include <vector>

// a set of (increasing x ordered) x,y points that define a single 'curve' on a plot.

class CXYPlotData
{
 public:
  CXYPlotData(const char *curveName,const double *x, const double *y, int n, enum eCurveStyleType type, int curveNumber);

  enum eCurveStyleType curveType(void) const;
  const std::string& curveName(void) const;
  const double* xRawData(void) const;
  const double* yRawData(void) const;
  int curveNumber(void) const;
  int size(void) const;

 private:
  std::string m_curveName;
  std::vector<double> m_xData, m_yData;
  int m_curveNumber;
  enum eCurveStyleType m_curveType;
};

inline enum eCurveStyleType CXYPlotData::curveType(void) const { return m_curveType; }
inline const std::string& CXYPlotData::curveName(void) const { return m_curveName; }
inline const double* CXYPlotData::xRawData(void) const { return m_xData.data(); }
inline const double* CXYPlotData::yRawData(void) const { return m_yData.data(); }
inline int CXYPlotData::size(void) const { return m_xData.size(); }
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
  const std::string& plotTitle(void) const;
  const std::string& xAxisLabel(void) const;
  const std::string& yAxisLabel(void) const;

 private:
  std::vector<CXYPlotData*> m_dataList;
  enum ePlotScaleType m_scaleType;
  bool m_forceAutoScaling;
  std::string m_file,m_title, m_xLabel, m_yLabel;
};

inline int CPlotDataSet::count(void) const { return m_dataList.size(); }
inline const CXYPlotData& CPlotDataSet::rawData(int index) const { return *(m_dataList.at(index)); }
inline enum eCurveStyleType CPlotDataSet::curveType(int index) const { return m_dataList.at(index)->curveType(); }
inline enum ePlotScaleType CPlotDataSet::scaleType(void) const { return m_scaleType; }
inline bool CPlotDataSet::forceAutoScaling(void) const { return m_forceAutoScaling; }
inline const std::string& CPlotDataSet::plotTitle(void) const { return m_title; }
inline const std::string& CPlotDataSet::xAxisLabel(void) const { return m_xLabel; }
inline const std::string& CPlotDataSet::yAxisLabel(void) const { return m_yLabel; }


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
  std::string title;
  std::string tag;

  STitleTag(int p, const std::string &ti, const std::string &ta) : page(p), title(ti), tag(ta) {}
};

struct SImage
{
//  int page;
//  std::string imageFilename;
//  QImage image;
//
//  SImage(int p,const std::string &fn,const QImage &img) : page(p),imageFilename(fn),image(img) {}

};

#endif
