/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CTABLEPAGEDATA_H_GUARD
#define _CTABLEPAGEDATA_H_GUARD

#include <map>

#include <boost/variant.hpp>

using cell_data = boost::variant<void *, int, double, std::string>;

struct SCell
{
  int page, row, col;
  cell_data data;

  SCell(int p, int r, int c, const cell_data &d) : page(p), row(r), col(c), data(d) {}
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

  void addCell(int row, int col, const cell_data &data);

  int pageNumber(void) const;
  int rowCount(void) const;
  int columnCount(void) const;

  bool isEmpty(void) const;

  cell_data cellData(int row, int col) const;

 private:
  std::map<SCellIndex,cell_data> m_dataMap;
  int m_pageNumber;
  int m_rows, m_columns;
};

inline int CTablePageData::pageNumber(void) const { return m_pageNumber; }
inline int CTablePageData::rowCount(void) const { return m_rows + 1; }
inline int CTablePageData::columnCount(void) const { return m_columns + 1; }

inline bool CTablePageData::isEmpty(void) const { return m_dataMap.empty(); }

#endif
