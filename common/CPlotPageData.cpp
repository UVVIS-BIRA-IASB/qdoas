/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <cstdio>
#include <cstdlib>

#include <sstream>


#include "CPlotPageData.h"

using std::shared_ptr;
using std::string;

CPlotPageData::CPlotPageData(int pageNumber,int pageType) :
  m_pageNumber(pageNumber),
  m_pageType(pageType)
{
  // default tag
  std::ostringstream stream;
  stream << "Tag-" << pageNumber;
  m_tag = stream.str();
}

int CPlotPageData::pageNumber(void) const
{
  return m_pageNumber;
}

int CPlotPageData::size(void) const
{
  return (m_pageType==PLOTPAGE_DATASET)?m_dataSets.size():m_images.size();
}

const string& CPlotPageData::title(void) const
{
  return m_title;
}

const string& CPlotPageData::tag(void) const
{
  return m_tag;
}

int CPlotPageData::type(void) const
{
  return m_pageType;
}

const CPlotDataSet& CPlotPageData::dataSet(int index) const
{
  return m_dataSets.at(index);
}

const CPlotImage& CPlotPageData::image(int index) const
{
  return m_images.at(index);
}

void CPlotPageData::setTitle(string title)
{
  m_title = title;
}

void CPlotPageData::setTag(string tag)
{
  m_tag = tag;
}

void CPlotPageData::addPlotDataSet(CPlotDataSet dataSet)
{
  m_dataSets.push_back(std::move(dataSet));
}

void CPlotPageData::addPlotImage(CPlotImage image)
{
  m_images.push_back(std::move(image));
}
