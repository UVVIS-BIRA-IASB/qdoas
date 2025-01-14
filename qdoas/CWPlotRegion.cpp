/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QResizeEvent>
#include <QLabel>
#include <QPixmap>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "CWPlotRegion.h"
#include "CWPlotPage.h"

#include "debugutil.h"

using std::shared_ptr;

CWPlotRegion::CWPlotRegion(QWidget *parent) :
  QScrollArea(parent),
  m_plotPage(NULL),
  m_activePageNumber(-1)
{
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  QLabel *splash = new QLabel;
  splash->setPixmap(QPixmap(":/images/qdoas_atmospheric_toolbox.png"));
  splash->setAlignment(Qt::AlignCenter);

  setWidget(splash);
}

void CWPlotRegion::removeAllPages()
{
  m_pageMap.clear();

  m_plotPage = NULL;
  //setWidget(m_plotPage); // deletes the current 'viewport widget'
}

void CWPlotRegion::removePagesExcept(const QList<int> pageNumberList)
{
  QList<shared_ptr<const CPlotPageData> > retained;
  std::map< int,shared_ptr<const CPlotPageData> >::iterator it;

  // create a list of the pages to be retained
  QList<int>::const_iterator pIt = pageNumberList.begin();
  while (pIt != pageNumberList.end()) {

    it = m_pageMap.find(*pIt);
    if (it != m_pageMap.end()) {
      // keep this page ...
      retained.push_back(it->second);
    }

    ++pIt;
  }

  m_pageMap.clear();

  // now put the retained pages back
  while (!retained.isEmpty()) {
   shared_ptr<const CPlotPageData> page(retained.takeFirst());

    m_pageMap.insert(std::map< int,shared_ptr<const CPlotPageData> >::value_type(page->pageNumber(), page));
  }

}

void CWPlotRegion::addPage(shared_ptr<const CPlotPageData> page)
{
  // the page must not already exist
  std::map< int,shared_ptr<const CPlotPageData> >::iterator it = m_pageMap.find(page->pageNumber());

  if (it == m_pageMap.end())
   {
    m_pageMap.insert(std::map< int,shared_ptr<const CPlotPageData> >::value_type(page->pageNumber(), page));
   }

  // else just quietly allow page to be discarded
}

// Display active page
void CWPlotRegion::displayPage(int pageNumber)
{
  std::map< int,shared_ptr<const CPlotPageData> >::iterator it = m_pageMap.find(pageNumber);

  if (it != m_pageMap.end()) {
    m_activePageNumber = pageNumber;

    m_plotPage = new CWPlotPage(m_properties, it->second);
    setWidget(m_plotPage); // takes care of deleting the old widget
    m_plotPage->layoutPlots(m_visibleSize);

    m_plotPage->show();
  }
  else {
    m_activePageNumber = -1; // invalid page number
    m_plotPage = NULL;
    setWidget(m_plotPage);
  }

}

void CWPlotRegion::printVisiblePage(void)
{
  if (m_plotPage)
    m_plotPage->slotPrintAllPlots();
}

void CWPlotRegion::exportVisiblePage(void)
{
  if (m_plotPage)
    m_plotPage->slotExportAsImageAllPlots();
}

int CWPlotRegion::pageDisplayed(void) const
{
  return m_activePageNumber;
}

QString CWPlotRegion::pageTitle(int pageNumber) const
{
  std::map<int,shared_ptr<const CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end())
    return QString::fromStdString((it->second)->title());

  return QString();
}

QString CWPlotRegion::pageTag(int pageNumber) const
{
  std::map<int,shared_ptr<const CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end())
    return QString::fromStdString((it->second)->tag());

  return QString();
}

bool CWPlotRegion::pageExists(int pageNumber, QString &tag) const
{
  std::map<int,shared_ptr<const CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end()) {
    tag = QString::fromStdString((it->second)->tag());
    return true;
  }

  tag = QString();
  return false;
}

const CPlotProperties& CWPlotRegion::properties(void) const
{
  return m_properties;
}

void CWPlotRegion::setProperties(const CPlotProperties &properties)
{
  m_properties = properties;
  if (m_plotPage != NULL) {
    displayPage(m_activePageNumber);
  }
}

void CWPlotRegion::resizeEvent(QResizeEvent *e)
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

