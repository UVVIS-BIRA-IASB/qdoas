/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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

void CWPlotArea::setPage(std::shared_ptr<const CPlotPageData> page)
{
  if (page) {
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
  if (m_pageData)
    return QString::fromStdString(m_pageData->title());

  return QString();
}

QString CWPlotArea::pageTag(void) const
{
  if (m_pageData)
    return QString::fromStdString(m_pageData->tag());

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

