/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CIMAGEPAGEDATA_H_GUARD
#define _CIMAGEPAGEDATA_H_GUARD

// storage class for CPlotDataSets grouped into a single page

#include <QList>

#include "CPlotImage.h"
#include "RefCountPtr.h"

class CImagePageData
{
 public:
  CImagePageData(int pageNumber);
  ~CImagePageData();

  bool isEmpty(void) const;
  int pageNumber(void) const;
  int size(void) const;
  const QString& title(void) const;
  const QString& tag(void) const;
  RefCountConstPtr<CPlotImage> image(int index) const;

  void setTitle(const QString &title);
  void setTag(const QString &tag);
  void addPlotImage(const CPlotImage *dataImage); // page takes ownership responsibility 4

 private:
  int m_pageNumber;
  QString m_title, m_tag;
  QList< RefCountConstPtr<CPlotImage> > m_images;
};

inline bool CImagePageData::isEmpty(void) const { return m_images.isEmpty(); }

#endif


