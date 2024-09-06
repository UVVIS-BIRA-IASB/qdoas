/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CPLOTPAGEDATA_H_GUARD
#define _CPLOTPAGEDATA_H_GUARD

// storage class for CPlotDataSets grouped into a single page

#include <QList>

#include <memory>

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
  const QString& title(void) const;
  const QString& tag(void) const;
  int type(void) const;
  std::shared_ptr<const CPlotDataSet> dataSet(int index) const;
  std::shared_ptr<const CPlotImage> dataImage(int index) const;

  void setTitle(const QString &title);
  void setTag(const QString &tag);
  void addPlotDataSet(const CPlotDataSet *dataSet); // page takes ownership responsibility
  void addPlotImage(const CPlotImage *dataImage);

 private:
  int m_pageNumber;
  int m_pageType;
  QString m_title, m_tag;
  QList<std::shared_ptr<const CPlotDataSet> > m_dataSets;
  QList<std::shared_ptr<const CPlotImage> > m_dataImages;
};

inline bool CPlotPageData::isEmpty(void) const { return (m_pageType==PLOTPAGE_DATASET)?m_dataSets.isEmpty():m_dataImages.isEmpty(); }

#endif

