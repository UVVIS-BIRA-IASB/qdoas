/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
