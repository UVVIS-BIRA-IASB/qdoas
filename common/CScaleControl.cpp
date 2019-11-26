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

