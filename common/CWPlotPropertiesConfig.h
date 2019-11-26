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


#ifndef _CWPLOTPROPERTIESCONFIG_H_GUARD
#define _CWPLOTPROPERTIESCONFIG_H_GUARD

#include <QFrame>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>

#include "CPlotProperties.h"

class CWPlotPropertySample : public QFrame
{
Q_OBJECT
 public:
  CWPlotPropertySample(const QPen &pen, const QColor &bgColour, QWidget *parent = 0);

  const QPen& pen(void) const;

  void setBackgroundColour(const QColor &c);

 protected:
  virtual void paintEvent(QPaintEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);

 public slots:
  void slotSetPenWidth(int penWidth);

 private:
  QPen m_pen;
};

class CWPlotPropertyScale : public QFrame
{
Q_OBJECT
 public:
  CWPlotPropertyScale(const CScaleControl &scaleControl, const QString &name, QWidget *parent = 0);

  const CScaleControl& scaleControl(void) const;

 public slots:
   void slotFixedCheckChanged(int checkState);

 private:
  QCheckBox *m_fixedCheck;
  QLineEdit *m_minEdit, *m_maxEdit;
  mutable CScaleControl m_scale;
};

class CWPlotPropertiesConfig : public QFrame
{
Q_OBJECT
 public:
  CWPlotPropertiesConfig(const CPlotProperties &prop, QWidget *parent = 0);

  void apply(CPlotProperties &prop) const;

  // helpers to plot config save/restore
  static void loadFromPreferences(CPlotProperties &prop);
  static void saveToPreferences(const CPlotProperties &prop);

 public slots:
  void slotSelectBackgroundColour();

 private:
  CWPlotPropertySample *m_curveSample[4];
  CWPlotPropertyScale *m_scaleEdit[3];
  QSpinBox *m_plotColumnsSpin;
  QColor m_bgColour;
};

#endif
