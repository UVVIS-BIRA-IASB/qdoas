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


#include <QResizeEvent>
#include <QLabel>
#include <QPixmap>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "CWPlotArea.h"
#include "CWPlotPage.h"
#include "CPreferences.h"

#include "debugutil.h"

CWPlotArea::CWPlotArea(QWidget *parent) :
  QScrollArea(parent),
  m_plotPage(NULL)
{
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void CWPlotArea::setPage(const RefCountConstPtr<CPlotPageData> &page)
{
  if (page != 0) {
    m_plotPage = new CWPlotPage(m_properties, page);
    setWidget(m_plotPage); // takes care of deleting the old widget
    m_plotPage->layoutPlots(m_visibleSize);
    m_plotPage->show();
  }
  else {
    m_plotPage = NULL;
    setWidget(m_plotPage); // takes care of deleting the old widget
  }

  m_pageData = page;
}

void CWPlotArea::printPage(void)
{
  if (m_plotPage)
    m_plotPage->slotPrintAllPlots();
}

void CWPlotArea::exportPage(void)
{
  if (m_plotPage)
    m_plotPage->slotExportAsImageAllPlots();
}

QString CWPlotArea::pageTitle(void) const
{
  if (m_pageData != 0)
    return m_pageData->title();

  return QString();
}

QString CWPlotArea::pageTag(void) const
{
  if (m_pageData != 0)
    return m_pageData->tag();

  return QString();
}

const CPlotProperties& CWPlotArea::properties(void) const
{
  return m_properties;
}

void CWPlotArea::setProperties(const CPlotProperties &properties)
{
  m_properties = properties;
  if (m_plotPage != NULL) {
    m_plotPage = new CWPlotPage(m_properties, m_pageData);
    setWidget(m_plotPage); // takes care of deleting the old widget
    m_plotPage->layoutPlots(m_visibleSize);
    m_plotPage->show();
  }
}

void CWPlotArea::resizeEvent(QResizeEvent *e)
{
  m_visibleSize = e->size();

  if (m_plotPage) {
    m_plotPage->layoutPlots(m_visibleSize);
  }
  else {
    QWidget *w = widget();
    if (w)
      w->resize(m_visibleSize);
  }
}

