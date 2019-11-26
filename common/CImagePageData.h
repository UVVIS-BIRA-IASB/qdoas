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


