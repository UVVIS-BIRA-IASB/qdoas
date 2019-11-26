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


#ifndef _CWPLOTREGION_H_GUARD
#define _CWPLOTREGION_H_GUARD

#include <QList>
#include <QScrollArea>
#include <QString>
#include <QSize>

#include "CPlotProperties.h"
#include "CPlotPageData.h"  
#include "CImagePageData.h"
#include "RefCountPtr.h"

class CWPlotPage;

class CWPlotRegion : public QScrollArea
{
 public:
  CWPlotRegion(QWidget *parent = 0);

  void removeAllPages();
  void removePagesExcept(const QList<int> pageNumberList);
  void addPage(const RefCountConstPtr<CPlotPageData> &page); 
   
  void displayPage(int pageNumber);
  void printVisiblePage(void);
  void exportVisiblePage(void);

  int pageDisplayed(void) const;
  QString pageTitle(int pageNumber) const;
  QString pageTag(int pageNumber) const;
  bool pageExists(int pageNumber, QString &tag) const;

  const CPlotProperties& properties(void) const;
  void setProperties(const CPlotProperties &properties);

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  CPlotProperties m_properties;
  CWPlotPage *m_plotPage;
  std::map< int,RefCountConstPtr<CPlotPageData> > m_pageMap;
  int m_activePageNumber;
  QSize m_visibleSize; 
};

#endif
