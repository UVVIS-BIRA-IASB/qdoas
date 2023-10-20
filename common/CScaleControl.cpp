/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CScaleControl.h"

CScaleControl::CScaleControl() :
  m_fixedScale(false),
  m_minimum(-1.0),
  m_maximum(1.0)
{
}

CScaleControl::CScaleControl(bool fixedScale, double minimum, double maximum) :
  m_fixedScale(fixedScale),
  m_minimum(minimum),
  m_maximum(maximum)
{
  // ensure sensible ordering ...
  if (m_maximum < m_minimum) {
    m_minimum = maximum;
    m_maximum = minimum;
  }
}

CScaleControl::CScaleControl(const CScaleControl &other) :
  m_fixedScale(other.m_fixedScale),
  m_minimum(other.m_minimum),
  m_maximum(other.m_maximum)
{
}

CScaleControl& CScaleControl::operator=(const CScaleControl &rhs)
{
  if (&rhs != this) {
    m_fixedScale = rhs.m_fixedScale;
    m_minimum = rhs.m_minimum;
    m_maximum = rhs.m_maximum;
  }

  return *this;
}

