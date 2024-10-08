/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CPlotProperties.h"


CPlotProperties::CPlotProperties() :
  m_defaultPen(Qt::black),
  m_scales(3),
  m_bgColour(Qt::white),
  m_columns(1),
  m_printPaperSize(QPageSize::A4),
  m_printPaperOrientation(QPageLayout::Portrait)
{
  // a pen for each of the 4 curves
  m_pens.push_back(m_defaultPen);
  m_pens.push_back(m_defaultPen);
  m_pens.push_back(m_defaultPen);
  m_pens.push_back(m_defaultPen);
}

CPlotProperties::CPlotProperties(const CPlotProperties &other) :
  m_defaultPen(Qt::black),
  m_pens(other.m_pens),
  m_scales(other.m_scales),
  m_bgColour(other.m_bgColour),
  m_columns(other.m_columns),
  m_printPaperSize(other.m_printPaperSize),
  m_printPaperOrientation(other.m_printPaperOrientation)
{
}

CPlotProperties& CPlotProperties::operator=(const CPlotProperties &rhs)
{
  if (&rhs != this) {
    m_pens = rhs.m_pens;
    m_scales = rhs.m_scales;
    m_bgColour = rhs.m_bgColour;
    m_columns = rhs.m_columns;
    m_printPaperSize = rhs.m_printPaperSize;
    m_printPaperOrientation = rhs.m_printPaperOrientation;
  }

  return *this;
}

const QPen& CPlotProperties::pen(int curveNumber) const
{
  size_t index = --curveNumber;

  if (index < m_pens.size())
    return m_pens[index];

  return m_defaultPen;
}

void CPlotProperties::setPen(int curveNumber, const QPen &pen)
{
  size_t index = --curveNumber;

  if (index < m_pens.size())
    m_pens[index] = pen;
}

const CScaleControl& CPlotProperties::scaleControl(enum ePlotScaleType scaleType) const
{
  size_t index = scaleType;

  if (index < m_scales.size())
    return m_scales[index];

  return m_scales[0]; // default
}

void CPlotProperties::setScaleControl(enum ePlotScaleType scaleType, const CScaleControl &scaleControl)
{
  size_t index = scaleType;

  if (index < m_scales.size())
    m_scales[index] = scaleControl;
}

