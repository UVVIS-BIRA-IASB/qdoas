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


#ifndef _CSCALECONTROL_H_GUARD
#define _CSCALECONTROL_H_GUARD

class CScaleControl
{
 public:
  CScaleControl();
  CScaleControl(bool fixedScale, double minimum, double maximum);
  CScaleControl(const CScaleControl &other);
  
  CScaleControl& operator=(const CScaleControl &rhs);

  bool isFixedScale(void) const;
  double minimum(void) const;
  double maximum(void) const;

 private:
  bool m_fixedScale;
  double m_minimum, m_maximum;
};

inline bool CScaleControl::isFixedScale(void) const { return m_fixedScale; }
inline double CScaleControl::minimum(void) const { return m_minimum; }
inline double CScaleControl::maximum(void) const { return m_maximum; }

#endif
