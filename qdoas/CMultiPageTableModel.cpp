/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include "CMultiPageTableModel.h"

#include "debugutil.h"

using std::shared_ptr;

class qvariant_visitor {
public:
  QVariant operator()(void *p) const {
    return QVariant();
  }

  QVariant operator()(int i) const {
    return QVariant(i);
  }

  QVariant operator()(double d) const {
    return QVariant(d);
  }

  QVariant operator()(std::string s) const {
    return QVariant(QString::fromStdString(s));
  }
};


void CMultiPageTableModel::addPage(shared_ptr<const CTablePageData> page)
{
  if (!page) return;

  // must not already exist
  std::map<int,shared_ptr<const CTablePageData> >::iterator it = m_pageMap.find(page->pageNumber());
  if (it == m_pageMap.end())
    m_pageMap.insert(std::map< int,shared_ptr<const CTablePageData> >::value_type(page->pageNumber(), page));
}

void CMultiPageTableModel::removeAllPages(void)
{
  if (m_currentPage) {
    // notify that the active page is going to disapear ...
    int lastRow = m_currentPage->rowCount() - 1;
    int lastCol = m_currentPage->columnCount() - 1;
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_pageMap.clear();
    m_currentPage = shared_ptr<const CTablePageData>();
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
  QList<shared_ptr<const CTablePageData> > retained;
  std::map< int,shared_ptr<const CTablePageData> >::iterator it;

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

  if (m_currentPage && !pageNumberList.contains(m_currentPage->pageNumber())) {
    // notify that the active page is going to disapear ...
    int lastRow = m_currentPage->rowCount() - 1;
    int lastCol = m_currentPage->columnCount() - 1;
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_pageMap.clear();
    m_currentPage = shared_ptr<const CTablePageData>();
    endRemoveRows();
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    endRemoveColumns();
  }
  else {
    m_pageMap.clear();
  }

  // now put the retained pages back
  while (!retained.isEmpty()) {
   shared_ptr<const CTablePageData> page(retained.takeFirst());

    m_pageMap.insert(std::map< int,shared_ptr<const CTablePageData> >::value_type(page->pageNumber(), page));
  }
}

void CMultiPageTableModel::setActivePage(int pageNumber)
{
  int lastRow, lastCol;

  if (m_currentPage) {
    // notify that the active page is going to disapear ...
    lastRow = m_currentPage->rowCount() - 1;
    lastCol = m_currentPage->columnCount() - 1;
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_currentPage = shared_ptr<const CTablePageData>();
    endRemoveRows();
    endRemoveColumns();
  }

  std::map< int,shared_ptr<const CTablePageData> >::iterator it = m_pageMap.find(pageNumber);
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
  if (!parent.isValid() && m_currentPage)
    return m_currentPage->columnCount();

  return 0;
}

int CMultiPageTableModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid() && m_currentPage)
    return m_currentPage->rowCount();

  return 0;
}

QVariant CMultiPageTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || !m_currentPage)
    return QVariant();

  QVariant data = std::visit(qvariant_visitor(),
                             m_currentPage->cellData(index.row(), index.column()));

  // respond to display and alignment roles
  if (role == Qt::DisplayRole) {
    return data;
  }
  else if (role == Qt::TextAlignmentRole) {
    QVariant::Type type = data.type();

    return QVariant((type == QVariant::Int || type == QVariant::Double) ?
            Qt::AlignRight : Qt::AlignLeft);
  }

  return QVariant();
}

// TODO
void CMultiPageTableModel::slotTablePages(const QList<shared_ptr<const CTablePageData> > &pageList)
{
  int activePage = (m_currentPage) ? m_currentPage->pageNumber() : -1;

  QList<int> retainedList;

  // build a list of the pages to be retained (empty pages)
  auto it = pageList.begin();
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

