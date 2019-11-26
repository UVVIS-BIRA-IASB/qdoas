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


#ifndef _CPLOTIMAGE_H_GUARD
#define _CPLOTIMAGE_H_GUARD

#include <QString>
#include <QList> 

#include "mediate_types.h"

class CPlotImage
{
 public:
  CPlotImage(const char *filename,const char *title) : m_file(filename), m_title(title) {};
  
 const QString& GetFile(void) const; 
 const QString& GetTitle(void) const;

 private:
  QString m_file,m_title;  
};

inline const QString& CPlotImage::GetFile(void) const { return m_file; } 
inline const QString& CPlotImage::GetTitle(void) const { return m_title; }

struct SPlotImage
{
 int page;
 const CPlotImage *plotImage;

 SPlotImage(int p,const CPlotImage *i) : page(p),plotImage(i) {}
};

#endif
