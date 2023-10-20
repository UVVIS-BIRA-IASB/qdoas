/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
