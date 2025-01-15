/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CTablePageData.h"

#include "debugutil.h"

CTablePageData::CTablePageData(int pageNumber) :
  m_pageNumber(pageNumber),
  m_rows(-1),
  m_columns(-1)
{
}

void CTablePageData::addCell(int row, int col, const cell_data &data)
{
  // dont care if the cell already exists ... overwrite it ...

  if (row < 0 || col < 0) return; // indexing is non-negative

  auto it = m_dataMap.find(SCellIndex(row,col));
  if (it == m_dataMap.end()) {
    // does not exist - insert it
    m_dataMap.insert(std::map<SCellIndex,cell_data>::value_type(SCellIndex(row,col), data));

    if (row > m_rows) m_rows = row;
    if (col > m_columns) m_columns = col;
  }
  else {
    // replace the data
    it->second = data;
  }
}

cell_data CTablePageData::cellData(int row, int col) const
{
  auto it = m_dataMap.find(SCellIndex(row,col));
  if (it != m_dataMap.end())
    return it->second;

  // default is null data
  return cell_data();
}
