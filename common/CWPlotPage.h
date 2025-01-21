/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWPLOTPAGE_H_GUARD
#define _CWPLOTPAGE_H_GUARD

#include <memory>

#include <QFrame>
#include <QList>
#include <QSize>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>

#include "CPlotProperties.h"
#include "CPlotDataSet.h"
#include "CPlotImage.h"
#include "CPlotPageData.h"


// NOTE. The plotProperties references held by these classes allows them to
// poke at the internals of the CWPlotRegion (the owner of the plotProperties
// instance). In general, the properties is used to READ user preferences
// on plot style and layout, but it is NON CONST so that options selected
// when printing can be saved and restored (without needed to call on the
// CPreferences class directly).

class CWPlot : public QwtPlot
{
Q_OBJECT
 public:
  CWPlot(const CPlotDataSet& dataSet, CPlotProperties &plotProperties, QWidget *parent = 0);
  CWPlot(const CPlotImage& image, CPlotProperties &plotProperties, QWidget *parent = 0);
  virtual ~CWPlot() {};

  static bool getImageSaveNameAndFormat(QWidget *parent, QString &fileName, QString &saveFormat);
  void imageresize(QSize visibleSize,int row,int column);

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotOverlay();
  void slotSaveAs();
  void slotPrint();
  void slotExportAsImage();
  void slotToggleInteraction();

 private:
  const CPlotDataSet *m_dataSet;
  const CPlotImage *m_image;
  CPlotProperties &m_plotProperties;
  QwtPlotZoomer *m_zoomer;
  int m_type;
     QGraphicsView *m_dataView;
     QGraphicsScene *m_dataScene;
     QPixmap m_dataPixmap,m_dataPixmapScaled;
     QGraphicsPixmapItem *m_dataPixmapItem;
};

class CWPlotPage : public QFrame
{
Q_OBJECT
 public:
  CWPlotPage(CPlotProperties &plotProperties, QWidget *parent = 0);
  CWPlotPage(CPlotProperties &plotProperties,
         std::shared_ptr<const CPlotPageData> page, QWidget *parent = 0);
  virtual ~CWPlotPage() {};

  void layoutPlots(const QSize &visibleSize);

 public slots:
  void slotPrintAllPlots();
  void slotExportAsImageAllPlots();

 private:
  CPlotProperties &m_plotProperties;
  int m_pageType;

  QList<CWPlot*> m_plots;
};

#endif
