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

#ifndef _CPLOTPAGEDATA_H_GUARD
#define _CPLOTPAGEDATA_H_GUARD

// storage class for CPlotDataSets grouped into a single page

#include <QList>

#include "CPlotDataSet.h"  
#include "CPlotImage.h"
#include "RefCountPtr.h"      

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
  RefCountConstPtr<CPlotDataSet> dataSet(int index) const;
  RefCountConstPtr<CPlotImage> dataImage(int index) const;

  void setTitle(const QString &title);
  void setTag(const QString &tag);
  void addPlotDataSet(const CPlotDataSet *dataSet); // page takes ownership responsibility    
  void addPlotImage(const CPlotImage *dataImage);    

 private:
  int m_pageNumber;  
  int m_pageType;
  QString m_title, m_tag;
  QList< RefCountConstPtr<CPlotDataSet> > m_dataSets;  
  QList< RefCountConstPtr<CPlotImage> > m_dataImages;
};

inline bool CPlotPageData::isEmpty(void) const { return (m_pageType==PLOTPAGE_DATASET)?m_dataSets.isEmpty():m_dataImages.isEmpty(); }

#endif

