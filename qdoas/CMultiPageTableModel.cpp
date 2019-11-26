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

#include "CMultiPageTableModel.h"

#include "debugutil.h"

void CMultiPageTableModel::addPage(const RefCountConstPtr<CTablePageData> &page)
{
  if (page == 0) return;

  // must not already exist
  std::map< int,RefCountConstPtr<CTablePageData> >::iterator it = m_pageMap.find(page->pageNumber());
  if (it == m_pageMap.end())
    m_pageMap.insert(std::map< int,RefCountConstPtr<CTablePageData> >::value_type(page->pageNumber(), page));
}

void CMultiPageTableModel::removeAllPages(void)
{
  if (m_currentPage != 0) {
    // notify that the active page is going to disapear ...
    int lastRow = m_currentPage->rowCount() - 1;
    int lastCol = m_currentPage->columnCount() - 1;
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_pageMap.clear();
    m_currentPage = RefCountConstPtr<CTablePageData>();
    endRemoveRows();
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    endRemoveColumns();
  }
  else {
    m_pageMap.clear();
  }
} 

void CMultiPageTableModel::removePagesExcept(const QList<int> pageNumberList)
{
  QList< RefCountConstPtr<CTablePageData> > retained;
  std::map< int,RefCountConstPtr<CTablePageData> >::iterator it;

  // create a list of the pages to be retained
  QList<int>::const_iterator pIt = pageNumberList.begin();
  while (pIt != pageNumberList.end()) {

    it = m_pageMap.find(*pIt);
    if (it != m_pageMap.end()) {
      // keep this page ...
      retained.push_back(it->second);
    }

    ++pIt;
  }

  if (m_currentPage != 0 && !pageNumberList.contains(m_currentPage->pageNumber())) {
    // notify that the active page is going to disapear ...
    int lastRow = m_currentPage->rowCount() - 1;
    int lastCol = m_currentPage->columnCount() - 1;
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_pageMap.clear();
    m_currentPage = RefCountConstPtr<CTablePageData>();
    endRemoveRows();
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    endRemoveColumns();
  }
  else {
    m_pageMap.clear();
  }

  // now put the retained pages back
  while (!retained.isEmpty()) {
    RefCountConstPtr<CTablePageData> page(retained.takeFirst());
    
    m_pageMap.insert(std::map< int,RefCountConstPtr<CTablePageData> >::value_type(page->pageNumber(), page));
  }
} 

void CMultiPageTableModel::setActivePage(int pageNumber)
{
  int lastRow, lastCol;

  if (m_currentPage != 0) {
    // notify that the active page is going to disapear ...
    lastRow = m_currentPage->rowCount() - 1;
    lastCol = m_currentPage->columnCount() - 1;
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_currentPage = RefCountConstPtr<CTablePageData>();
    endRemoveRows();
    endRemoveColumns();
  }

  std::map< int,RefCountConstPtr<CTablePageData> >::iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end()) {
    lastRow = (it->second)->rowCount() - 1;
    lastCol = (it->second)->columnCount() - 1;
    beginInsertColumns(QModelIndex(), 0, lastCol);
    beginInsertRows(QModelIndex(), 0, lastRow);
    m_currentPage = (it->second);
    endInsertRows();
    endInsertColumns();
  }
}

int CMultiPageTableModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid() && m_currentPage != 0)
    return m_currentPage->columnCount();

  return 0;
}

int CMultiPageTableModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid() && m_currentPage != 0)
    return m_currentPage->rowCount();

  return 0;
}

QVariant CMultiPageTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || m_currentPage == 0)
    return QVariant();

  // respond to display and alignment roles
  if (role == Qt::DisplayRole) {
    return m_currentPage->cellData(index.row(), index.column());
  }
  else if (role == Qt::TextAlignmentRole) {
    QVariant::Type type = m_currentPage->cellData(index.row(), index.column()).type();
    
    return QVariant((type == QVariant::Int || type == QVariant::Double) ?
		    Qt::AlignRight : Qt::AlignLeft);
  }

  return QVariant();
}

// TODO
void CMultiPageTableModel::slotTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList)
{
  int activePage = (m_currentPage != 0) ? m_currentPage->pageNumber() : -1;

  QList<int> retainedList;

  // build a list of the pages to be retained (empty pages)
  QList< RefCountConstPtr<CTablePageData> >::const_iterator it = pageList.begin();
  while (it != pageList.end()) {
    
    if ((*it)->isEmpty()) {
      retainedList.push_back((*it)->pageNumber());
    }

    ++it;
  }

  removePagesExcept(retainedList);

  // add the additional pages (non empty)
  it = pageList.begin();
  while (it != pageList.end()) {

    if (!(*it)->isEmpty()) {
      addPage(*it);
    }
    ++it;
  }

  // try and restore to the same active page
  setActivePage(activePage);
}

