/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPLOTIMAGE_H_GUARD
#define _CPLOTIMAGE_H_GUARD

#include <string>
#include <vector>

#include "mediate_types.h"

class CPlotImage
{
 public:
  CPlotImage(const char *filename,const char *title) : m_file(filename), m_title(title) {};

 const std::string& GetFile(void) const;
 const std::string& GetTitle(void) const;

 private:
  std::string m_file,m_title;
};

inline const std::string& CPlotImage::GetFile(void) const { return m_file; }
inline const std::string& CPlotImage::GetTitle(void) const { return m_title; }

struct SPlotImage
{
 int page;
 const CPlotImage *plotImage;

 SPlotImage(int p,const CPlotImage *i) : page(p),plotImage(i) {}
};

#endif
