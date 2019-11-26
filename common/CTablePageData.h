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

#ifndef _CTABLEPAGEDATA_H_GUARD
#define _CTABLEPAGEDATA_H_GUARD

#include <map>

#include <QVariant>

struct SCell
{
  int page, row, col;
  QVariant data;

  SCell(int p, int r, int c, const QVariant &d) : page(p), row(r), col(c), data(d) {}
};

struct SCellIndex
{
  int r, c;

  SCellIndex(int row, int col) : r(row), c(col) {}  
  bool operator<(const SCellIndex &rhs) const { return (r<rhs.r || (r==rhs.r && c<rhs.c)); }
};

class CTablePageData
{
 public:
  CTablePageData(int pageNumber);

  void addCell(int row, int col, const QVariant &data);
  
  int pageNumber(void) const;
  int rowCount(void) const;
  int columnCount(void) const;

  bool isEmpty(void) const;
  
  QVariant cellData(int row, int col) const;

 private:
  std::map<SCellIndex,QVariant> m_dataMap;
  int m_pageNumber;
  int m_rows, m_columns;
};

inline int CTablePageData::pageNumber(void) const { return m_pageNumber; }
inline int CTablePageData::rowCount(void) const { return m_rows + 1; }
inline int CTablePageData::columnCount(void) const { return m_columns + 1; }

inline bool CTablePageData::isEmpty(void) const { return m_dataMap.empty(); }

#endif
