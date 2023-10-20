/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
