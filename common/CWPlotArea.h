/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWPLOTAREA_H_GUARD
#define _CWPLOTAREA_H_GUARD

#include <memory>

#include <QScrollArea>
#include <QString>
#include <QSize>

#include "CPlotProperties.h"
#include "CPlotPageData.h"


class CWPlotPage;

class CWPlotArea : public QScrollArea
{
 public:
  CWPlotArea(QWidget *parent = 0);

  void setPage(std::shared_ptr<const CPlotPageData> page);

  void printPage(void);
  void exportPage(void);

  QString pageTitle(void) const;
  QString pageTag(void) const;

  const CPlotProperties& properties(void) const;
  void setProperties(const CPlotProperties &properties);

  void savePreferences(void) const;

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  CPlotProperties m_properties;
  CWPlotPage *m_plotPage;
  std::shared_ptr<const CPlotPageData> m_pageData;
  QSize m_visibleSize;
};

#endif
