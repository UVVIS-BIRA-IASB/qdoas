/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QGridLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QColorDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QTextStream>

#include "CWPlotPropertiesConfig.h"
#include "CPreferences.h"

#include "debugutil.h"

CWPlotPropertySample::CWPlotPropertySample(const QPen &pen, const QColor &bgColour, QWidget *parent) :
  QFrame(parent),
  m_pen(pen)
{
  setLineWidth(2);
  setFrameStyle(QFrame::Sunken | QFrame::Panel);

  // background
  QPalette p(palette());
  p.setColor(backgroundRole(), bgColour);
  setPalette(p);

  setAutoFillBackground(true);

  setMinimumSize(QSize(128,32));
}

void CWPlotPropertySample::setBackgroundColour(const QColor &c)
{
  QPalette p(palette());
  p.setColor(backgroundRole(), c);
  setPalette(p);

  update();
}

const QPen& CWPlotPropertySample::pen(void) const
{
  return m_pen;
}

void CWPlotPropertySample::paintEvent(QPaintEvent *e)
{
  QFrame::paintEvent(e);

  QPainter p(this);
  p.setPen(m_pen);

  int y = height() / 2;
  p.drawLine(10, y, width() - 10, y);
}

void CWPlotPropertySample::mousePressEvent(QMouseEvent *e)
{
  // popup a color selection dialog to set the colour of m_pen
  QColor result = QColorDialog::getColor(m_pen.color(), this);

  if (result.isValid()) {
    m_pen.setColor(result);
    update();
  }
  e->accept();
}

void CWPlotPropertySample::slotSetPenWidth(int penWidth)
{
  m_pen.setWidth(penWidth);
  update();
}

//--------------------------------------------------------------------

CWPlotPropertyScale::CWPlotPropertyScale(const CScaleControl &scaleControl, const QString &name, QWidget *parent) :
  QFrame(parent),
  m_scale(scaleControl)
{
  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setSpacing(3);

  m_fixedCheck = new QCheckBox(name, this);
  m_fixedCheck->setCheckState(m_scale.isFixedScale() ? Qt::Checked : Qt::Unchecked);
  mainLayout->addWidget(m_fixedCheck, 0, 0);

  mainLayout->addWidget(new QLabel("Min", this), 0, 1, Qt::AlignRight);

  m_minEdit = new QLineEdit(this);
  m_minEdit->setFixedWidth(70);
  mainLayout->addWidget(m_minEdit, 0, 2);

  mainLayout->addWidget(new QLabel("Max", this), 0, 3, Qt::AlignRight);

  m_maxEdit = new QLineEdit(this);
  m_maxEdit->setFixedWidth(70);
  mainLayout->addWidget(m_maxEdit, 0, 4);

  mainLayout->setColumnStretch(0, 1);

  // initialize
  QString tmpStr;
  m_minEdit->setText(tmpStr.setNum(scaleControl.minimum()));
  m_maxEdit->setText(tmpStr.setNum(scaleControl.maximum()));
  slotFixedCheckChanged(m_fixedCheck->checkState());

  // connections
  connect(m_fixedCheck, SIGNAL(stateChanged(int)), this, SLOT(slotFixedCheckChanged(int)));
}

const CScaleControl& CWPlotPropertyScale::scaleControl(void) const
{
  bool okMin, okMax, fixed;
  double tmpMin, tmpMax;

  tmpMin = m_minEdit->text().toDouble(&okMin);
  tmpMax = m_maxEdit->text().toDouble(&okMax);

  fixed = (m_fixedCheck->checkState() == Qt::Checked);

  if (!fixed || (okMin && okMax && tmpMin != tmpMax)) {
    m_scale = CScaleControl(fixed, tmpMin, tmpMax);
  }

  return m_scale;
}

void CWPlotPropertyScale::slotFixedCheckChanged(int checkState)
{
  bool fixed = (checkState == Qt::Checked);

  m_minEdit->setEnabled(fixed);
  m_maxEdit->setEnabled(fixed);
}

//--------------------------------------------------------------------

void CWPlotPropertiesConfig::loadFromPreferences(CPlotProperties &prop)
{
  // restore the plot properties from preferences
  QPen pen(Qt::black);
  QColor colour(Qt::white);

  CPreferences *pref = CPreferences::instance();

  prop.setPen(1, pref->plotPen("Curve1", pen));
  prop.setPen(2, pref->plotPen("Curve2", pen));
  prop.setPen(3, pref->plotPen("Curve3", pen));
  prop.setPen(4, pref->plotPen("Curve4", pen));

  prop.setScaleControl(Spectrum, pref->plotScale("SpectrumScale"));
  prop.setScaleControl(SpecMax, pref->plotScale("SpecMaxScale"));
  prop.setScaleControl(Residual, pref->plotScale("ResidualScale"));

  prop.setBackgroundColour(pref->plotColour("Background", colour));

  prop.setColumns(pref->plotLayout("Columns", 1));
  prop.setPrintPaperSize(static_cast<QPageSize::PageSizeId>(pref->plotLayout("PaperSize", 7)));
  prop.setPrintPaperOrientation(QPageLayout::Orientation(pref->plotLayout("PaperOrientation", 0)));
}


void CWPlotPropertiesConfig::saveToPreferences(const CPlotProperties &prop)
{
  CPreferences *pref = CPreferences::instance();

  pref->setPlotPen("Curve1", prop.pen(1));
  pref->setPlotPen("Curve2", prop.pen(2));
  pref->setPlotPen("Curve3", prop.pen(3));
  pref->setPlotPen("Curve4", prop.pen(4));

  pref->setPlotScale("SpectrumScale", prop.scaleControl(Spectrum));
  pref->setPlotScale("SpecMaxScale", prop.scaleControl(SpecMax));
  pref->setPlotScale("ResidualScale", prop.scaleControl(Residual));

  pref->setPlotColour("Background", prop.backgroundColour());

  pref->setPlotLayout("Columns", prop.columns());
  pref->setPlotLayout("PaperSize", static_cast<int>(prop.printPaperSize()));
  pref->setPlotLayout("PaperOrientation", prop.printPaperOrientation());

}

CWPlotPropertiesConfig::CWPlotPropertiesConfig(const CPlotProperties &prop, QWidget *parent) :
  QFrame(parent)
{
  int row;

  QGridLayout *mainLayout = new QGridLayout(this);

  QGroupBox *colourGroup = new QGroupBox("Colour/Line Width", this);

  QGridLayout *colourLayout = new QGridLayout(colourGroup);

  row = 0;

  m_bgColour = prop.backgroundColour();

  // 4 curves
  for (int i=0; i<4; ++i) {
    QString name;
    QTextStream stream(&name);

    stream << "Curve " << i+1;

    colourLayout->addWidget(new QLabel(name), row, 0);
    m_curveSample[i] = new CWPlotPropertySample(prop.pen(i+1), m_bgColour);
    colourLayout->addWidget(m_curveSample[i], row, 1);
    QSpinBox *spinBox = new QSpinBox;
    spinBox->setRange(0, 5);
    spinBox->setValue(prop.pen(i+1).width());
    colourLayout->addWidget(spinBox, row, 2);
    connect(spinBox, SIGNAL(valueChanged(int)), m_curveSample[i], SLOT(slotSetPenWidth(int)));
    ++row;
  }

  // background colour
  QPushButton *bgColourBtn = new QPushButton("Background Colour");
  colourLayout->addWidget(bgColourBtn, row, 0, 1, 3);

  colourLayout->setRowStretch(row, 1);

  mainLayout->addWidget(colourGroup, 0, 0, 2, 1);

  // scale group ...

  QGroupBox *scaleGroup = new QGroupBox("Fixed Scale", this);

  QVBoxLayout *scaleLayout = new QVBoxLayout(scaleGroup);
  m_scaleEdit[0] = new CWPlotPropertyScale(prop.scaleControl(Spectrum), "Spectrum", scaleGroup);
  scaleLayout->addWidget(m_scaleEdit[0]);
  m_scaleEdit[1] = new CWPlotPropertyScale(prop.scaleControl(SpecMax), "SpecMax", scaleGroup);
  scaleLayout->addWidget(m_scaleEdit[1]);
  m_scaleEdit[2] = new CWPlotPropertyScale(prop.scaleControl(Residual), "Residual", scaleGroup);
  scaleLayout->addWidget(m_scaleEdit[2]);

  mainLayout->addWidget(scaleGroup, 0, 1);


  // Layout group ...

  QGroupBox *layoutGroup = new QGroupBox("Layout", this);

  QGridLayout *layoutLayout = new QGridLayout(layoutGroup);

  row = 0;

  layoutLayout->addWidget(new QLabel("Plot Columns\n(Max. per page)"), row, 0);
  m_plotColumnsSpin = new QSpinBox;
  m_plotColumnsSpin->setRange(1, 6);
  m_plotColumnsSpin->setValue(prop.columns());
  m_plotColumnsSpin->setFixedWidth(50);
  layoutLayout->addWidget(m_plotColumnsSpin, row, 1);
  ++row;

  layoutLayout->setRowStretch(row, 1);

  mainLayout->addWidget(layoutGroup, 1, 1);

  // connections
  connect(bgColourBtn, SIGNAL(clicked()), this, SLOT(slotSelectBackgroundColour()));
}

void CWPlotPropertiesConfig::apply(CPlotProperties &prop) const
{
  // get pens from the samples ...
  for (int i=0; i<4; ++i) {
    prop.setPen(i+1, m_curveSample[i]->pen());
  }

  prop.setScaleControl(Spectrum, m_scaleEdit[0]->scaleControl());
  prop.setScaleControl(SpecMax,  m_scaleEdit[1]->scaleControl());
  prop.setScaleControl(Residual, m_scaleEdit[2]->scaleControl());

  prop.setBackgroundColour(m_bgColour);

  prop.setColumns(m_plotColumnsSpin->value());
}

void CWPlotPropertiesConfig::slotSelectBackgroundColour()
{
  // popup a color selection dialog to set the colour of the background
  QColor result = QColorDialog::getColor(m_bgColour, this);

  if (result.isValid()) {
    m_bgColour = result;

    for (int i=0; i<4; ++i) {
      m_curveSample[i]->setBackgroundColour(m_bgColour);
    }
  }
}
