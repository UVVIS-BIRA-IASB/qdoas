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


#ifndef _CVIEWCROSSSECTIONDATA_H_GUARD
#define _CVIEWCROSSSECTIONDATA_H_GUARD

#include "mediate_analysis_window.h"

// Wrapper class for any data required to 'view cross sections'.

class CViewCrossSectionData
{
 public:
  CViewCrossSectionData(const mediate_analysis_window_t *aw) : m_aw(*aw) {}; // deep copy

  const mediate_analysis_window_t* analysisWindow(void) const;

 private:
  mediate_analysis_window_t m_aw;
};

inline const mediate_analysis_window_t* CViewCrossSectionData::analysisWindow(void) const { return &m_aw; }

#endif
