/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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

  QPageSize::PageSizeId printPaperSize(void) const;
  void setPrintPaperSize(QPageSize::PageSizeId paperSize);

  QPageLayout::Orientation printPaperOrientation(void) const;
  void setPrintPaperOrientation(QPageLayout::Orientation orientation);

 private:
  QPen m_defaultPen;
  std::vector<QPen> m_pens;
  std::vector<CScaleControl> m_scales;
  QColor m_bgColour;
  int m_columns;
  QPageSize::PageSizeId m_printPaperSize;
  QPageLayout::Orientation m_printPaperOrientation;
};

inline const QColor& CPlotProperties::backgroundColour(void) const { return m_bgColour; }
inline void CPlotProperties::setBackgroundColour(const QColor &c) { m_bgColour = c; }
inline int CPlotProperties::columns(void) const { return m_columns; }
inline void CPlotProperties::setColumns(int nColumns) { m_columns = (nColumns < 1) ? 1 : nColumns; }
inline QPageSize::PageSizeId CPlotProperties::printPaperSize(void) const { return m_printPaperSize; }
inline void CPlotProperties::setPrintPaperSize(QPageSize::PageSizeId paperSize) { m_printPaperSize = paperSize; }
inline QPageLayout::Orientation CPlotProperties::printPaperOrientation(void) const { return m_printPaperOrientation; }
inline void CPlotProperties::setPrintPaperOrientation(QPageLayout::Orientation orientation) { m_printPaperOrientation = orientation; }

#endif
