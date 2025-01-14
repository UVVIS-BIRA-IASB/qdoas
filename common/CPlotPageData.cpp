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
  return (m_pageType==PLOTPAGE_DATASET)?m_dataSets.size():m_dataImages.size();
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

shared_ptr<const CPlotDataSet> CPlotPageData::dataSet(int index) const
{
  if (index < 0 || index > m_dataSets.size())
    return shared_ptr<const CPlotDataSet>();

  return m_dataSets.at(index);
}

shared_ptr<const CPlotImage> CPlotPageData::dataImage(int index) const
{
  if (index < 0 || index > m_dataImages.size())
    return shared_ptr<const CPlotImage>();

  return m_dataImages.at(index);
}

void CPlotPageData::setTitle(const string &title)
{
  m_title = title;
}

void CPlotPageData::setTag(const string &tag)
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


