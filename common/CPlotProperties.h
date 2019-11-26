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


#ifndef _CPLOTPROPERTIES_H_GUARD
#define _CPLOTPROPERTIES_H_GUARD

#include <vector>
#include <QPen>
#include <QColor>
#include <QPrinter>

#include "mediate_types.h"

#include "CScaleControl.h"

class CPlotProperties
{
 public:
  CPlotProperties();
  CPlotProperties(const CPlotProperties &other);

  CPlotProperties& operator=(const CPlotProperties &rhs);

  // curve number ranges from 1 to 4
  const QPen& pen(int curveNumber) const;
  void setPen(int curveNumber, const QPen &pen);

  const QColor& backgroundColour(void) const;
  void setBackgroundColour(const QColor &c);

  const CScaleControl& scaleControl(enum ePlotScaleType scaleType) const;
  void setScaleControl(enum ePlotScaleType scaleType, const CScaleControl &scaleControl);

  int columns(void) const;
  void setColumns(int nColumns);

  QPrinter::PageSize printPaperSize(void) const;
  void setPrintPaperSize(QPrinter::PageSize paperSize);
  QPrinter::Orientation printPaperOrientation(void) const;
  void setPrintPaperOrientation(QPrinter::Orientation orientation);

 private:
  QPen m_defaultPen;
  std::vector<QPen> m_pens;
  std::vector<CScaleControl> m_scales;
  QColor m_bgColour;
  int m_columns;
  QPrinter::PageSize m_printPaperSize;
  QPrinter::Orientation m_printPaperOrientation;
};

inline const QColor& CPlotProperties::backgroundColour(void) const { return m_bgColour; }
inline void CPlotProperties::setBackgroundColour(const QColor &c) { m_bgColour = c; }
inline int CPlotProperties::columns(void) const { return m_columns; }
inline void CPlotProperties::setColumns(int nColumns) { m_columns = (nColumns < 1) ? 1 : nColumns; }
inline QPrinter::PageSize CPlotProperties::printPaperSize(void) const { return m_printPaperSize; }
inline void CPlotProperties::setPrintPaperSize(QPrinter::PageSize paperSize) { m_printPaperSize = paperSize; }
inline QPrinter::Orientation CPlotProperties::printPaperOrientation(void) const { return m_printPaperOrientation; }
inline void CPlotProperties::setPrintPaperOrientation(QPrinter::Orientation orientation) { m_printPaperOrientation = orientation; }

#endif
