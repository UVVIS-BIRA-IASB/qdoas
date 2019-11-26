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

RefCountConstPtr<CPlotImage> CImagePageData::image(int index) const
{
  if (index < 0 || index > m_images.size())
    return RefCountConstPtr<CPlotImage>();

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

  m_images.push_back(RefCountConstPtr<CPlotImage>(image));
}         



