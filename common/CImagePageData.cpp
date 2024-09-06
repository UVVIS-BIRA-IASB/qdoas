/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include "CImagePageData.h"

CImagePageData::CImagePageData(int pageNumber) :
  m_pageNumber(pageNumber)
{
  // default tag
  m_tag.sprintf("Tag-%d", pageNumber);
}

CImagePageData::~CImagePageData()
{
  // the list destructor and ref-counting ensure safe handling of the plot data.
}

int CImagePageData::pageNumber(void) const
{
  return m_pageNumber;
}

int CImagePageData::imagesize(void) const
{
  return m_images.size();
}

const QString& CImagePageData::title(void) const
{
  return m_title;
}

const QString& CImagePageData::tag(void) const
{
  return m_tag;
}

std::shared_ptr<const CPlotImage> CImagePageData::image(int index) const
{
  if (index < 0 || index > m_images.size())
    return std::shared_ptr<const CPlotImage>();

  return m_images.at(index);
}

void CImagePageData::setTitle(const QString &title)
{
  m_title = title;
}

void CImagePageData::setTag(const QString &tag)
{
  m_tag = tag;
}

void CImagePageData::addPlotImage(const CPlotImage *image)
{
  // page takes ownership responsibility, which means it is safe
  // to wrap it in a reference counting pointer

  m_images.push_back(std::shared_ptr<const CPlotImage>(image));
}



