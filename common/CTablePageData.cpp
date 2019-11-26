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


#include "CTablePageData.h"

#include "debugutil.h"

CTablePageData::CTablePageData(int pageNumber) :
  m_pageNumber(pageNumber),
  m_rows(-1),
  m_columns(-1)
{
}

void CTablePageData::addCell(int row, int col, const QVariant &data)
{
  // dont care if the cell already exists ... overwrite it ...

  if (row < 0 || col < 0) return; // indexing is non-negative

  std::map<SCellIndex,QVariant>::iterator it = m_dataMap.find(SCellIndex(row,col));
  if (it == m_dataMap.end()) {
    // does not exist - insert it
    m_dataMap.insert(std::map<SCellIndex,QVariant>::value_type(SCellIndex(row,col), data));

    if (row > m_rows) m_rows = row;
    if (col > m_columns) m_columns = col;
  }
  else {
    // replace the data
    it->second = data;
  }
}

QVariant CTablePageData::cellData(int row, int col) const
{
  std::map<SCellIndex,QVariant>::const_iterator it = m_dataMap.find(SCellIndex(row,col));
  if (it != m_dataMap.end())
    return it->second;

  // default is null data
  return QVariant();
}
