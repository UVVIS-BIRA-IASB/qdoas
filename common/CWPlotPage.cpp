/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <cstdio>

#include <QColor>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFont>

#include <QGraphicsView>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_zoneitem.h>
#include <qwt_text.h>

#include "CWPlotPage.h"
#include "CPreferences.h"

#include "debugutil.h"

// static helper function

bool CWPlot::getImageSaveNameAndFormat(QWidget *parent, QString &fileName, QString &saveFormat)
{
  CPreferences *pref = CPreferences::instance();

  QFileDialog dialog(parent);

  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setWindowTitle("Export as Image");;
  dialog.setDirectory(pref->directoryName("ExportPlot"));
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setOption(QFileDialog::DontConfirmOverwrite, false);
  QStringList filters;
  filters << "PNG (*.png)" << "BMP (*.bmp)" << "JPEG (*.jpg)";
  dialog.setNameFilters(filters);

  if (dialog.exec()) {

    QStringList fileList = dialog.selectedFiles();
    if (!fileList.isEmpty()) {

      QString format = dialog.selectedNameFilter();
      fileName = fileList.first();

      // update the preference
      pref->setDirectoryNameGivenFile("ExportPlot", fileName);

      // attempt to determine the format from the tail of the filename - otherwise the filter selected.
      if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
    saveFormat = "JPEG";
      }
      else if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
    saveFormat = "BMP";
      }
      else if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
    saveFormat = "PNG";
      }
      else {
    // not a known extension ... append based on the filter
    if (format.startsWith("BMP")) {
      saveFormat = "BMP";
      fileName += ".bmp";
    }
    else if (format.startsWith("JPEG")) {
      saveFormat = "JPEG";
      fileName += ".jpeg";
    }
    else { // default to PNG
      saveFormat = "PNG";
      fileName += ".png";
    }
      }

      return true;
    }
  }

  return false;
}

CWPlot::CWPlot(std::shared_ptr<const CPlotDataSet> dataSet,
           CPlotProperties &plotProperties, QWidget *parent) :
  QwtPlot(parent),
  m_dataSet(dataSet),
//  m_dataImage(""),
  m_plotProperties(plotProperties),
  m_zoomer(NULL),
  m_type(PLOTPAGE_DATASET)
{
  // Example code for font changes ... TODO

  QwtText tmpTitle = title();
  QFont tmpFont = tmpTitle.font();
  tmpFont.setPointSize(tmpFont.pointSize());
  tmpTitle.setFont(tmpFont);
  tmpTitle.setText(QString::fromStdString(m_dataSet->plotTitle()));
  setTitle(tmpTitle);

  QString str=tmpTitle.text();

  setFocusPolicy(Qt::ClickFocus); // TODO - prevents keyPressEvent

  //setTitle(m_dataSet->plotTitle());
  //setAxisTitle(QwtPlot::xBottom, m_dataSet->xAxisLabel());

  setAxisTitle(QwtPlot::yLeft, QString::fromStdString(m_dataSet->yAxisLabel()));

    // curves ...

  int n = m_dataSet->count();
  int i = 0;

  while (i < n) {

    const CXYPlotData &curveData = m_dataSet->rawData(i);

    if (curveData.size() > 0) {
      QwtPlotCurve *curve = new QwtPlotCurve();
      double *xraw=(double *)curveData.xRawData();
      int xsize=curveData.size();
      curve->setRenderHint(QwtPlotItem::RenderAntialiased);

      // the data is guaranteed to be valid for the life of this object
      curve->setRawSamples(curveData.xRawData(), curveData.yRawData(), curveData.size());

      // configure curve's pen color based on index

      if (curveData.curveType() == Line)
       curve->setPen(m_plotProperties.pen((curveData.curveNumber()%4)+1));
      else if (curveData.curveType() == DashLine)
       {
           curve->setPen(m_plotProperties.pen((curveData.curveNumber()%4)+1));
           QPen newpen=curve->pen();
           QBrush newBrush=QBrush(Qt::gray,Qt::Dense4Pattern);
           newpen.setStyle(Qt::DashLine);
           curve->setPen(newpen);

        QwtPlotZoneItem *zoneItem=new QwtPlotZoneItem;

        zoneItem->setZ( 11 );
        zoneItem->setBrush(newBrush);
        zoneItem->setOrientation( Qt::Vertical );
        zoneItem->setItemAttribute( QwtPlotItem::Legend, true );

        zoneItem->setInterval(xraw[0],xraw[xsize-1]);
        zoneItem->setVisible( true );
        zoneItem->attach( this );
       }
      else if (curveData.curveType() == Point) {
       curve->setStyle(QwtPlotCurve::NoCurve);

       QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse);
              sym->setSize(4);
       curve->setSymbol(sym);
      }

      curve->attach(this);
    }

    ++i;
  }

  // grid
  QwtPlotGrid *grid = new QwtPlotGrid;
  QPen pen(Qt::DotLine);
  pen.setColor((m_plotProperties.backgroundColour().value() < 128) ? Qt::white : Qt::black);
  grid->setPen(pen);
  grid->attach(this);

  // possibly apply fixed scaling
  if (!m_dataSet->forceAutoScaling()) {

    if (m_plotProperties.scaleControl(m_dataSet->scaleType()).isFixedScale()) {
      setAxisScale(QwtPlot::yLeft,
           m_plotProperties.scaleControl(m_dataSet->scaleType()).minimum(),
           m_plotProperties.scaleControl(m_dataSet->scaleType()).maximum());

    }
  }

  setCanvasBackground(plotProperties.backgroundColour());

  replot();
}

CWPlot::CWPlot(std::shared_ptr<const CPlotImage> dataImage,
           CPlotProperties &plotProperties, QWidget *parent) :
  QwtPlot(parent),
  m_dataImage(dataImage),
  m_plotProperties(plotProperties),
  m_zoomer(NULL),
  m_type(PLOTPAGE_IMAGE)
 {
  QString filename(QString::fromStdString(m_dataImage->GetFile()));
  QwtText tmpTitle=title();
  const QByteArray fname=filename.toLocal8Bit();
  const char *ptr=strrchr(fname.constData(),'/')+1;

  // Example code for font changes ... TODO

  QFont tmpFont = tmpTitle.font();
  tmpFont.setPointSize(tmpFont.pointSize());
  tmpTitle.setFont(tmpFont);
  tmpTitle.setText(ptr);
  setTitle(tmpTitle);

  enableAxis(QwtPlot::xBottom, false);
  enableAxis(QwtPlot::yLeft, false);

  setFocusPolicy(Qt::ClickFocus); // TODO - prevents keyPressEvent

  m_dataPixmap=QPixmap(filename);

  m_dataView = new QGraphicsView(this);
  m_dataView->setObjectName(QString::fromUtf8(ptr));
  m_dataView->setGeometry(QRect(0,0,m_dataPixmap.width(),m_dataPixmap.height()));
  m_dataView->setStyleSheet("background: transparent; border: transparent;");

  m_dataPixmapScaled=m_dataPixmap.scaled(m_dataView->width()-5,m_dataView->height()-5,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

  m_dataScene = new QGraphicsScene;
  m_dataPixmapItem=m_dataScene->addPixmap(m_dataPixmapScaled);

  m_dataView->setScene(m_dataScene);
}

void CWPlot::imageresize(QSize visibleSize,int row,int column)
 {
     int cBorderSize=15;

  m_dataView->setGeometry(QRect(row-cBorderSize,column+5,visibleSize.width(), visibleSize.height()));
  m_dataView->setSceneRect (QRect(0,0,m_dataView->width()-5, m_dataView->height()-5));
  m_dataPixmapScaled=m_dataPixmap.scaled(m_dataView->width()-5,m_dataView->height()-5,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
  m_dataPixmapItem->setPixmap(m_dataPixmapScaled);
 }

void CWPlot::contextMenuEvent(QContextMenuEvent *e)
{
  // position dependent
  if (childAt(e->pos()) != canvas()) {

    QMenu menu;

    if (m_type!=PLOTPAGE_IMAGE)
     {
      if (m_zoomer)
        menu.addAction("Non-Interactive", this, SLOT(slotToggleInteraction()));
      else
        menu.addAction("Interactive", this, SLOT(slotToggleInteraction()));
      menu.addSeparator();
      menu.addAction("Overlay...", this, SLOT(slotOverlay()));
     }

    menu.addAction("Save As...", this, SLOT(slotSaveAs()));
    menu.addAction("Export As Image...", this, SLOT(slotExportAsImage()));
    menu.addAction("Print...", this, SLOT(slotPrint()));

    menu.exec(e->globalPos()); // slot will do the rest

    e->accept();
  }
  else
    e->ignore();
}

void CWPlot::slotOverlay()
{
  if (m_type==PLOTPAGE_DATASET) {
    CPreferences *pref = CPreferences::instance();

    QString dirName = pref->directoryName("ASCII_Plot");

    QString filename = QFileDialog::getOpenFileName(this, "Overlay Plot(s)", dirName, "*.asc");

    if (!filename.isEmpty()) {
      pref->setDirectoryNameGivenFile("ASCII_Plot", filename);

      bool failed = false;

      FILE *fp = fopen(filename.toLocal8Bit().constData(), "r");
      if (fp != NULL) {
        char buffer[32];
        int nCurves, nPoints, i, j;

        int curveCount = m_dataSet->count(); // number of 'original' curves ...

        double *xData = NULL;
        double *yData = NULL;

        // skip the header
        fgets(buffer, sizeof(buffer), fp);

        if (fscanf(fp, "%d", &nCurves) == 1 && nCurves > 0) {
          i = 0;
          while (!failed && i < nCurves) {
            if (fscanf(fp, "%d", &nPoints) == 1) {
              xData = new double[nPoints];
              yData = new double[nPoints];
              j = 0;
              while (j<nPoints && fscanf(fp, "%lf %lf", (xData+j), (yData+j)) == 2)
                ++j;
              if (j == nPoints) {
                QwtPlotCurve *curve = new QwtPlotCurve();
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                curve->setSamples(xData, yData, nPoints);
                // configure curve's pen color based on index
                curve->setPen(m_plotProperties.pen((curveCount % 4) + 1));
                curve->attach(this);
                ++curveCount;
              } else {
                failed = true;
              }

              delete [] xData;
              delete [] yData;
              xData = yData = NULL;
            }
            else
              failed  = true;

            ++i;
          }
        }
        else
          failed = true;

        fclose(fp);
      }
      else
        failed  = true;

      if (failed) {
        QString msg = "Failed (correctly) open or parse ASCII data file ";
        msg += filename;

        QMessageBox::warning(this, "Failed file read", msg);
      }
      else
        replot();

    }
  }
}

void CWPlot::slotSaveAs() {
  if (m_type==PLOTPAGE_DATASET) {
    CPreferences *pref = CPreferences::instance();
    QString dirName = pref->directoryName("ASCII_Plot")+"/undefined.asc";
    QString filename = QFileDialog::getSaveFileName(this, "Save Plot", dirName, "All files (*)");

    if (!filename.isEmpty())
     {
      if (!filename.contains('.'))
       filename += ".asc";

      pref->setDirectoryNameGivenFile("ASCII_Plot", filename);

      FILE *fp = fopen(filename.toLocal8Bit().constData(), "w");
      if (fp != NULL)
       {
        int nCurves, nPoints, i, j, n, maxPoints;

        nCurves = m_dataSet->count();
        fprintf(fp,";\n");
        fprintf(fp, "; Plot %s (%d %s)\n;\n", m_dataSet->plotTitle().c_str(),nCurves,(nCurves>1)?"curves":"curve");
        for (i=0,maxPoints=0;i<nCurves;i++) {
          n=m_dataSet->rawData(i).size();
          fprintf(fp,";      Curve %d : %s (%d data points)\n",i+1,m_dataSet->rawData(i).curveName().c_str(),n);
          if (n>maxPoints)
            maxPoints=n;
        }
        fprintf(fp,";\n");

        for (j=0;j<maxPoints;j++)
         {
          for (i=0;i<nCurves;i++)
           {
               const CXYPlotData &curveData = m_dataSet->rawData(i);
               nPoints = curveData.size();
               if (nPoints > j)
                fprintf(fp, "%-22.14le %-22.14le ", *(curveData.xRawData() + j), *(curveData.yRawData() + j));
               else
                fprintf(fp,"%-22.14le %-22.14le ",(double)0.,(double)0.);
              }
             fprintf(fp,"\n");
            }

           fclose(fp);
          }
      else
       {
        QString msg = "Failed to create ASCII plot file ";
        msg += filename;

        QMessageBox::warning(this, "Failed file write", msg);
       }
        }
      }
    }

void CWPlot::slotPrint()
{
  QPrinter printer(QPrinter::HighResolution);

  QPageSize pageSize(m_plotProperties.printPaperSize());
  if (!pageSize.isValid())
    pageSize = QPageSize(QPageSize::A4);
  printer.setPageSize(pageSize);
  printer.setPageOrientation(QPageLayout::Landscape); // single plot ALWAYS defaults to landscape
  printer.setCopyCount(1);
  printer.setPageMargins(QMarginsF(1,1,1,1), QPageLayout::Inch);

  QPrintDialog dialog(&printer, this);

  if (dialog.exec() == QDialog::Accepted) {

    // store printer preference
    m_plotProperties.setPrintPaperSize(printer.pageLayout().pageSize().id());

    QPainter p(&printer);
    p.setPen(QPen(QColor(Qt::black)));

    QRect page = printer.pageLayout().paintRectPixels(printer.resolution());

    const int cPageBorder = 150;

    QRect tmp(cPageBorder, cPageBorder, page.width() - 2 * cPageBorder, page.height() - 2 * cPageBorder);

    p.drawRect(tmp);

    tmp.adjust(20, 20, -20, -20);

    QwtPlotRenderer renderer;

    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);

    renderer.render(this, &p,tmp);
  }
}

void CWPlot::slotExportAsImage()
{
  QStringList filter;
  QString fileName;

  fileName="";
  filter += "PNG Documents (*.png)";

  fileName = QFileDialog::getSaveFileName(this, "Export File Name", fileName, filter.join(";;"));

  if (!fileName.isEmpty() ) {
    QwtPlotRenderer renderer;
    renderer.renderDocument(this, fileName, QSizeF(100, 75), 300);
  }
}

void CWPlot::slotToggleInteraction()
{
  QwtPlotCanvas *c = qobject_cast<QwtPlotCanvas *>(canvas());

  if (m_zoomer) {
    c->setCursor(Qt::CrossCursor);
    delete m_zoomer;
    m_zoomer = NULL;
  }
  else {
    m_zoomer = new QwtPlotZoomer(c);
    c->setCursor(Qt::PointingHandCursor); // change the cursor to indicate that zooming is active
    // contrasting colour ...
    m_zoomer->setRubberBandPen(QPen((canvasBackground().color().value() < 128) ? Qt::white : Qt::black));
  }
}

CWPlotPage::CWPlotPage(CPlotProperties &plotProperties, QWidget *parent) :
  QFrame(parent),
  m_plotProperties(plotProperties)
{
}

CWPlotPage::CWPlotPage(CPlotProperties &plotProperties,
               std::shared_ptr<const CPlotPageData> page, QWidget *parent) :
  QFrame(parent),
  m_plotProperties(plotProperties),
  m_pageType(page->type())
{
  if (page) {
    int nplots=page->size();

    if (page->type()==PLOTPAGE_DATASET) {
      int i = 0;
      while (i < nplots ) {
        CWPlot *tmp = new CWPlot(page->dataSet(i), m_plotProperties, this);
        tmp->hide();
        m_plots.push_back(tmp);
        ++i;
      }
    } else {
      int i = 0;

      while (i < nplots) {
        CWPlot *tmp = new CWPlot(page->dataImage(i), m_plotProperties, this);
        tmp->hide();
        m_plots.push_back(tmp);
        ++i;
      }
    }
  }
}

void CWPlotPage::layoutPlots(const QSize &visibleSize)
{
  const int cBorderSize = 15;

  // MUST have at least one plot for this to be meaningful
  if (m_plots.size() == 0)
    return;

  // work out the layout ... and size ...

  int columns;
  QSize unitSize, minUnitSize;

  QList<CWPlot*>::iterator it = m_plots.begin();
  while (it != m_plots.end()) {
    minUnitSize = minUnitSize.expandedTo((*it)->minimumSizeHint());
    ++it;
  }

  if (m_plots.size() == 1) {
    // only one plot ... fit to the visible area
    unitSize = visibleSize;
    unitSize -= QSize(2 * cBorderSize, 2 * cBorderSize);
    columns = 1;
  }
  else {
    // calculate the size that fits nicely to the full width
    // make a local change to columns if too few plots to fill them...
    columns = (m_plots.size() < m_plotProperties.columns()) ? m_plots.size() : m_plotProperties.columns();


    unitSize.setWidth((visibleSize.width() - cBorderSize * (columns+1)) / columns);
    // want 3:4 ratio
    unitSize.setHeight(3 * unitSize.width() / 4);
  }

  // position and resize
  int fitWidth = unitSize.width() + cBorderSize;
  int fitHeight = unitSize.height() + cBorderSize;

  int col = 0;
  int row = 0;
  it = m_plots.begin();
  while (it != m_plots.end()) {

    if (m_pageType==PLOTPAGE_IMAGE)
     {
      (*it)->imageresize(unitSize,col * fitWidth + cBorderSize,row * fitHeight + cBorderSize);
     }
    (*it)->move(col * fitWidth + cBorderSize, row * fitHeight + cBorderSize);
    (*it)->resize(unitSize);
    (*it)->show();

    // (*it)->m_pixmap->scaled(unitSize.width(),unitSize.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    if (++col == m_plotProperties.columns()) {
      col = 0;
      ++row;
    }
    ++it;
  }
  // resize the plot page
  resize(columns*fitWidth + cBorderSize, (row + (col?1:0)) * fitHeight + cBorderSize);

}

void CWPlotPage::slotPrintAllPlots()
{
  // MUST have at least one plot for this to be meaningful
  if (m_plots.size() == 0)
    return;

  QPrinter printer(QPrinter::HighResolution);
  QwtPlotRenderer renderer;

  renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);

  QPageSize pageSize(m_plotProperties.printPaperSize());
  if (!pageSize.isValid())
    pageSize = QPageSize(QPageSize::A4);
  printer.setPageSize(pageSize);
  printer.setPageOrientation(m_plotProperties.printPaperOrientation());
  printer.setCopyCount(1);

  QPrintDialog dialog(&printer, this);

  if (dialog.exec() == QDialog::Accepted) {

    // store print preferences
    m_plotProperties.setPrintPaperSize(printer.pageLayout().pageSize().id());
    m_plotProperties.setPrintPaperOrientation(printer.pageLayout().orientation());

    QPainter p(&printer);
    p.setPen(QPen(QColor(Qt::black)));

    QRect page = printer.pageLayout().paintRectPixels(printer.resolution());

    // configure the plot rectangle
    const int cPageBorder = 150; // offset inside the drawable region of the page.
    const int cPlotBorder = 20;  // space between page border and plots, and between two plots.

    QRect tmp(cPageBorder, cPageBorder, page.width() - 2 * cPageBorder, page.height() - 2 * cPageBorder);

    p.drawRect(tmp);


    // work out the layout ... and size ...

    if (m_plots.size() == 1) {
      // only one plot ...
      tmp.adjust(cPlotBorder, cPlotBorder, -cPlotBorder, -cPlotBorder);
    }
    else {

      // calculate the size that fits nicely to the full width
      // make a local change to columns if too few plots to fill them...
      int columns = (m_plots.size() < m_plotProperties.columns()) ? m_plots.size() : m_plotProperties.columns();
      int rows = m_plots.size() / columns + ((m_plots.size() % columns) ? 1 : 0);

      int unitWidth = (tmp.width() - cPlotBorder * (columns+1)) / columns;
      int unitHeight = (tmp.height() - cPlotBorder * (rows+1)) / rows;

      int col = 0;
      int row = 0;
      QList<CWPlot*>::iterator it = m_plots.begin();
      while (it != m_plots.end()) {
    tmp = QRect(cPageBorder + cPlotBorder + col * (cPlotBorder + unitWidth),
            cPageBorder + cPlotBorder + row * (cPlotBorder + unitHeight),
            unitWidth, unitHeight);

        renderer.render(*it,&p,tmp);

        if (++col == columns) {
          col = 0;
          ++row;
        }
        ++it;
      }

    }
  }
}

void CWPlotPage::slotExportAsImageAllPlots()
{
  if (m_plots.size() == 0)
    return;

  QString fileName;
  QString format;
  QwtPlotRenderer renderer;

  // flags to make the document look like the widget
  renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);

  if (CWPlot::getImageSaveNameAndFormat(this, fileName, format)) {

    // determine the required image size ...

    const int cPlotBorder = 20;

    QSize plotSize;

    int columns = (m_plots.size() < m_plotProperties.columns()) ? m_plots.size() : m_plotProperties.columns();
    int rows = m_plots.size() / columns + ((m_plots.size() % columns) ? 1 : 0);

    QList<CWPlot*>::iterator it = m_plots.begin();
    while (it != m_plots.end()) {
      plotSize = plotSize.expandedTo((*it)->size());
      ++it;
    }

    // the size of the final image
    QSize imgSize(plotSize.width() * columns + (columns+1) * cPlotBorder,
                  plotSize.height() * rows + (rows+1) * cPlotBorder);

    QImage img(imgSize, QImage::Format_RGB32); // image the same size as the plot widget.
    img.fill(0xffffffff);

    QPainter p(&img);

    int col = 0;
    int row = 0;
    it = m_plots.begin();
    while (it != m_plots.end()) {
      QRect tmp(cPlotBorder + col * (cPlotBorder + plotSize.width()),
                cPlotBorder + row * (cPlotBorder + plotSize.height()),
                plotSize.width(), plotSize.height());

      renderer.render(*it,&p,tmp);

      if (++col == columns) {
        col = 0;
        ++row;
      }
      ++it;
    }

    img.save(fileName, format.toLocal8Bit().constData());
  }
}
