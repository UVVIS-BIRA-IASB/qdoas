/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <stdio.h>
#include <stdlib.h>
#include "CPlotPageData.h"

using std::shared_ptr;

CPlotPageData::CPlotPageData(int pageNumber,int pageType) :
  m_pageNumber(pageNumber),
  m_pageType(pageType)
{
  // default tag
  m_tag.sprintf("Tag-%d", pageNumber);
}

int CPlotPageData::pageNumber(void) const
{
  return m_pageNumber;
}

int CPlotPageData::size(void) const
{
  return (m_pageType==PLOTPAGE_DATASET)?m_dataSets.size():m_dataImages.size();
}

const QString& CPlotPageData::title(void) const
{
  return m_title;
}

const QString& CPlotPageData::tag(void) const
{
  return m_tag;
}

int CPlotPageData::type(void) const
{
      return m_pageType;
}

shared_ptr<const CPlotDataSet> CPlotPageData::dataSet(int index) const
{
  if (index < 0 || index > m_dataSets.size())
    shared_ptr<const CPlotDataSet>();

  return m_dataSets.at(index);
}

shared_ptr<const CPlotImage> CPlotPageData::dataImage(int index) const
{
  if (index < 0 || index > m_dataImages.size())
    shared_ptr<const CPlotImage>();

  return m_dataImages.at(index);
}

void CPlotPageData::setTitle(const QString &title)
{
  m_title = title;
}

void CPlotPageData::setTag(const QString &tag)
{
  m_tag = tag;
}

void CPlotPageData::addPlotDataSet(const CPlotDataSet *dataSet)
{
  // page takes ownership responsibility, which means it is safe
  // to wrap it in a reference counting pointer

  m_dataSets.push_back(shared_ptr<const CPlotDataSet>(dataSet));
}

void CPlotPageData::addPlotImage(const CPlotImage *image)
{
  // page takes ownership responsibility, which means it is safe
  // to wrap it in a reference counting pointer

  m_dataImages.push_back(shared_ptr<const CPlotImage>(image));
}


