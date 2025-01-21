/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CPLOTPAGEDATA_H_GUARD
#define _CPLOTPAGEDATA_H_GUARD

// storage class for CPlotDataSets grouped into a single page
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "CPlotDataSet.h"
#include "CPlotImage.h"


#define PLOTPAGE_DATASET 0
#define PLOTPAGE_IMAGE   1

class CPlotPageData
{
 public:
  CPlotPageData(int pageNumber,int pageType);

  bool isEmpty(void) const;
  int pageNumber(void) const;
  int size(void) const;
  const std::string& title(void) const;
  const std::string& tag(void) const;
  int type(void) const;
  const CPlotDataSet& dataSet(int index) const;
  const CPlotImage& image(int index) const;

  void setTitle(std::string title);
  void setTag(std::string tag);
  void addPlotDataSet(CPlotDataSet dataSet);
  void addPlotImage(CPlotImage image);

 private:
  int m_pageNumber;
  int m_pageType;
  std::string m_title, m_tag;
  std::vector<CPlotDataSet> m_dataSets;
  std::vector<CPlotImage> m_images;
};

inline bool CPlotPageData::isEmpty(void) const { return (m_pageType==PLOTPAGE_DATASET)?m_dataSets.empty():m_images.empty(); }

#endif
