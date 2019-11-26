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


#include <QtGui>

#include <QSize>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QPalette>
#include <QApplication>
#include <QIntValidator>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QAction>

#include "CDoasTable.h"
#include "CValidator.h"

#include "debugutil.h"
#include "constants.h"

CDoasTable::CDoasTable(const QString &label, int headerHeight, QWidget *parent) :
  QFrame(parent),
  m_titleHeight(headerHeight),
  m_sbThickness(16),
  m_columnOffset(0)
{

  m_vsb = new QScrollBar(Qt::Vertical, this);
  m_hsb = new QScrollBar(Qt::Horizontal, this);

  m_vsb->move(0, 0);

  m_header = new CDoasTableColumnHeader(label, this);
  m_header->setColumnHorizontalPosition(m_sbThickness);

  // default minimum size
  m_centralWidth = 100;
  m_centralHeight = 100;

  setMinimumSize(QSize(m_header->columnWidth() + m_sbThickness + m_centralWidth,
                       m_titleHeight + m_sbThickness + m_centralHeight));

  m_hsb->setRange(0, 0);
  m_hsb->setValue(0);

  m_vsb->setRange(0, 0);
  m_vsb->setValue(0);

  connect(m_hsb, SIGNAL(valueChanged(int)), this, SLOT(slotMovedHorizontalScrollBar(int)));
  connect(m_vsb, SIGNAL(valueChanged(int)), this, SLOT(slotMovedVerticalScrollBar(int)));
}

void CDoasTable::addColumn(CDoasTableColumn *column)
{
  if (m_rowHeightList.count() == 0) {
    m_columnList.push_back(column);
    calcHorizontalScrollRange();
  }
  else {
    delete column;
  }
}

int CDoasTable::rowIndexAtPosition(int yPixel) const
{
  if (yPixel < m_titleHeight || m_rowHeightList.empty())
    return -1;

  int y = m_titleHeight;
  int index = m_header->rowOffset();

  while (index < m_rowHeightList.count()) {
    y += m_rowHeightList.at(index); // limit for row index
    if (yPixel < y) break;
    ++index;
  }
  if (index < m_rowHeightList.count())
    return index;

  return -1; // no match
}

void CDoasTable::createColumnEdit(const QString &label, int minimumWidth)
{
  if (m_rowHeightList.count() == 0) {

    CDoasTableColumn *tmp = new CDoasTableColumnEdit(label, this, minimumWidth);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::createColumnEdit(int minimum, int maximum, const QString &label, int minimumWidth)
{
  if (m_rowHeightList.count() == 0) {

    CDoasTableColumn *tmp = new CDoasTableColumnIntEdit(minimum, maximum, label, this, minimumWidth);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::createColumnEdit(double minimum, double maximum, int decimals, const QString &label, int minimumWidth)
{
  if (m_rowHeightList.count() == 0) {

    CDoasTableColumn *tmp = new CDoasTableColumnDoubleEdit(minimum, maximum, decimals, label, this, minimumWidth);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::createColumnCombo(const QString &label, int minimumWidth, const QStringList &tags)
{
  if (m_rowHeightList.count() == 0) {
    CDoasTableColumn *tmp = new CDoasTableColumnCombo(tags, label, this, minimumWidth);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::createColumnCheck(const QString &label, int minimumWidth)
{
  if (m_rowHeightList.count() == 0) {

    CDoasTableColumn *tmp = new CDoasTableColumnCheck(label, this, minimumWidth);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

QString CDoasTable::rowLabel(int rowIndex) const
{
  return m_header->getCellData(rowIndex).toString();
}

void CDoasTable::addRow(int rowHeight, const QString &label, QList<QVariant> &cellData)
{
  // pad the cellData with null QVariants if too short ...
  while (cellData.count() < m_columnList.count()) {
    cellData.push_back(QVariant());
  }

  m_header->addRow(rowHeight, label);

  QList<QVariant>::const_iterator cdIt = cellData.begin();
  QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
  while (it != m_columnList.end()) {
    (*it)->addRow(rowHeight, *cdIt);
    ++it;
    ++cdIt;
  }

  int lastRowIndex = m_rowHeightList.count();
  int columnIndex = 0;

  m_rowHeightList.push_back(rowHeight);

  // notify cell data changed for all columns
  it = m_columnList.begin();
  while (it != m_columnList.end()) {
    QVariant tmp = (*it)->getCellData(lastRowIndex);
    cellDataChanged(lastRowIndex, columnIndex, tmp);
    ++columnIndex;
    ++it;
  }

  calcVerticalScrollRange();
}

void CDoasTable::removeRow(int rowIndex)
{
  if (rowIndex >= 0 && rowIndex < m_rowHeightList.count()) {

    m_header->removeRow(rowIndex);

    QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
    while (it != m_columnList.end()) {
      (*it)->removeRow(rowIndex);
      ++it;
    }

    m_rowHeightList.removeAt(rowIndex);

    updateCols();

    calcVerticalScrollRange();
  }
}

void CDoasTable::setColumnOffset(int offset)
{
  if (offset != m_columnOffset) {

    m_columnOffset = offset;

    CDoasTableColumn *tmp;
    int index = 0;
    int x = m_sbThickness + m_header->columnWidth();

    while (index < m_columnList.count()) {
      tmp = m_columnList.at(index);

      if (index < m_columnOffset) {
	tmp->hide();
      }
      else {
	if (x < width()) {
	  tmp->setColumnHorizontalPosition(x);
	  x += tmp->columnWidth();
	}
	else {
	  tmp->hide();
	}
      }
      ++index;
    }
  }
}

void CDoasTable::setColumnEnabled(int columnIndex, bool enabled)
{
  if (columnIndex >=0 && columnIndex < m_columnList.count())
    m_columnList.at(columnIndex)->setEnabled(enabled);
}

void CDoasTable::setCellEnabled(int rowIndex, int columnIndex, bool enabled)
{
  if (columnIndex >=0 && columnIndex < m_columnList.count())
    m_columnList.at(columnIndex)->setRowEnabled(rowIndex, enabled);
}

bool CDoasTable::isColumnEnabled(int columnIndex) const
{
  if (columnIndex >=0 && columnIndex < m_columnList.count())
    return m_columnList.at(columnIndex)->isEnabled();

  return false;
}

int CDoasTable::columnCount(void) const
{
  return m_columnList.count();
}

QList<QVariant> CDoasTable::getCellData(int rowIndex) const
{
  QList<QVariant> cellData;

  if (rowIndex >= 0 && rowIndex < m_rowHeightList.count()) {

    QList<CDoasTableColumn*>::const_iterator it = m_columnList.begin();
    while (it != m_columnList.end()) {
      cellData.push_back((*it)->getCellData(rowIndex));
      ++it;
    }
  }

  return cellData;
}

QVariant CDoasTable::getCellData(int rowIndex, int columnIndex) const
{
  if (columnIndex >= 0 && columnIndex < m_columnList.count())
    return m_columnList.at(columnIndex)->getCellData(rowIndex);

  return QVariant();
}

void CDoasTable::setHeaderLabel(int rowIndex, const QString &label)
{
  m_header->setLabel(rowIndex, label);

}

void CDoasTable::setCellData(int rowIndex, int columnIndex, const QVariant &cellData)
{
  if (columnIndex >= 0 && columnIndex < m_columnList.count())
    m_columnList.at(columnIndex)->setCellDataWrapper(rowIndex, cellData);
}

QWidget* CDoasTable::directAccessToCellWidget(int rowIndex, int columnIndex)
{
  // Try NOT to use this
  if (columnIndex >= 0 && columnIndex < m_columnList.count())
    return m_columnList.at(columnIndex)->directAccessToCellWidget(rowIndex);

  return NULL;
}

void CDoasTable::headerChanged(void) {
  updateCols();
  update(); // force Qt to redraw the table in case some of the
  // headers have changed width.
}

void CDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
  // do nothing ... default

}

void CDoasTable::notifyCellDataChanged(int row, const CDoasTableColumn *column, const QVariant &cellData)
{
  int columnIndex = m_columnList.indexOf(const_cast<CDoasTableColumn*>(column));
  if (columnIndex != -1) {
    cellDataChanged(row, columnIndex, cellData);
  }
}

void CDoasTable::updateCols(int wid, int hei) {

  m_centralWidth = wid - m_sbThickness - m_header->columnWidth();
  m_centralHeight = hei - m_titleHeight - m_sbThickness;

  m_vsb->resize(m_sbThickness, m_titleHeight + m_centralHeight);

  m_hsb->move(m_sbThickness, hei - m_sbThickness);
  m_hsb->resize(wid - m_sbThickness, m_sbThickness);

  // move the columns and resize them

  m_header->setViewportHeight(m_centralHeight);

  CDoasTableColumn *tmp;
  int index = 0;
  int x = m_sbThickness + m_header->columnWidth();

  while (index < m_columnList.count()) {
    tmp = m_columnList.at(index);
    tmp->setViewportHeight(m_centralHeight);

    if (index < m_columnOffset) {
      tmp->hide();
    }
    else {
      if (x < wid) {
	tmp->setColumnHorizontalPosition(x);
	x += tmp->columnWidth();
      }
      else {
	tmp->hide();
      }
    }
    ++index;
  }

  calcHorizontalScrollRange();
  calcVerticalScrollRange();
}

void CDoasTable::updateCols(void) {
  int wid = m_centralWidth + m_sbThickness + m_header->columnWidth();
  int hei = m_centralHeight +  m_titleHeight + m_sbThickness;
  updateCols(wid, hei);
}

void CDoasTable::resizeEvent(QResizeEvent *e)
{
  int wid = e->size().width();
  int hei = e->size().height();
  updateCols(wid, hei);
}

void CDoasTable::calcHorizontalScrollRange(void)
{
  int sum = 0;

  // get the sum of the widths of all columns
  QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
  while (it != m_columnList.end()) {
    sum += (*it)->columnWidth();
    ++it;
  }

  int index = 0;

  while (index < m_columnList.count()-1 && sum > m_centralWidth) {
    sum -= m_columnList.at(index)->columnWidth();
    ++index;
  }
  m_hsb->setRange(0, index);
}

void CDoasTable::calcVerticalScrollRange(void)
{
  int sum = 0;

  // get the sum of the widths of all columns
  QList<int>::iterator it = m_rowHeightList.begin();
  while (it != m_rowHeightList.end()) {
    sum += *it;
    ++it;
  }

  int index = 0;

  while (index < m_rowHeightList.count()-1 && sum > m_centralHeight) {
    sum -= m_rowHeightList.at(index);
    ++index;
  }
  m_vsb->setRange(0, index);
}

void CDoasTable::slotMovedHorizontalScrollBar(int value)
{
  setColumnOffset(value);
}

void CDoasTable::slotMovedVerticalScrollBar(int value)
{
  m_header->setRowOffset(value);

  QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
  while (it != m_columnList.end()) {
    (*it)->setRowOffset(value);
    ++it;
  }
}

//-------------------------------------

CDoasTableColumn::CDoasTableColumn(const QString &label, CDoasTable *owner, int minimumWidth) :
  QObject(owner),
  m_owner(owner),
  m_rowOffset(0),
  m_xPosition(0),
  m_xBorder(0),
  m_yBorder(0),
  m_minimumWidth(minimumWidth)
{
  m_viewport = new QFrame(owner); // parented to owner
  m_viewport->show();

  m_header = new QLabel(label, owner);
  m_header->setFrameStyle(QFrame::Panel | QFrame::Raised);
  m_header->setLineWidth(2);

  m_header->setMinimumSize(QSize(m_header->fontMetrics().boundingRect(label).width()
                                 + 2*m_header->fontMetrics().averageCharWidth() ,
                                 m_owner->headerHeight()));

  resizeWidgets();
  m_header->show();
}

void CDoasTableColumn::setColumnHorizontalPosition(int xPosition)
{
  m_xPosition = xPosition;

  m_viewport->move(m_xPosition, m_owner->headerHeight());
  m_viewport->show();

  m_header->move(m_xPosition, 0);
  m_header->show();
}

void CDoasTableColumn::setViewportHeight(int vpHeight)
{
  m_viewport->resize(columnWidth(), vpHeight);

  // move child widgets ...
  QWidget *tmp;
  int y = 0;
  int index = m_rowOffset;
  while (index < m_widgetList.count()) {
    tmp = m_widgetList.at(index);

    if (y < vpHeight) {
      tmp->move(m_xBorder, y + m_yBorder);
      tmp->show();
      y += tmp->height() + m_yBorder + m_yBorder;
    }
    else {
      tmp->hide();
    }
    ++index;
  }
}

void CDoasTableColumn::hide(void)
{
  m_viewport->hide();
  m_header->hide();
}

void CDoasTableColumn::setEnabled(bool enabled)
{
  m_header->setEnabled(enabled);
  m_viewport->setEnabled(enabled);
}

bool CDoasTableColumn::isEnabled(void) const
{
  return m_viewport->isEnabled();
}

void CDoasTableColumn::setRowEnabled(int rowIndex, bool enabled)
{
  if (rowIndex >=0 && rowIndex < m_widgetList.count())
    m_widgetList.at(rowIndex)->setEnabled(enabled);
}

void CDoasTableColumn::layoutAndDisplay(void)
{
  // move child widgets ...
  QWidget *tmp;
  int y = 0;
  int index = 0;
  while (index < m_widgetList.count()) {
    tmp = m_widgetList.at(index);

    if (index < m_rowOffset) {
      tmp->hide();
    }
    else {
      if (y < m_viewport->height()) {
	tmp->move(m_xBorder, y + m_yBorder);
	tmp->show();
	y += tmp->height() + m_yBorder + m_yBorder;
      }
      else {
	tmp->hide();
      }
    }
    ++index;
  }
}

void CDoasTableColumn::setRowOffset(int offset)
{
  if (offset != m_rowOffset) {
    m_rowOffset = offset;
    layoutAndDisplay();
  }
}

// Width of the column is calculated to be at least as big as
// "m_minimumWidth", or the width of the biggest widget's
// minimumSize(), whichever is bigger.
int CDoasTableColumn::columnWidth(void) const
{
  int width = std::max(m_minimumWidth, m_header->minimumSize().width());
  for(int i=0; i<rowCount(); ++i) {
      const QWidget *w = dynamic_cast<const QWidget *>(getWidget(i));
      width = std::max(width, w->minimumSize().width());
  }

  return width;
}

int CDoasTableColumn::rowCount(void) const
{
  return m_widgetList.count();
}

void CDoasTableColumn::resizeWidgets(void) {
  int width = columnWidth();

  // update width of header and widgets for all rows:
  m_header->resize(width,m_owner->headerHeight() );
  for (QList<QWidget *>::iterator i = m_widgetList.begin();
       i != m_widgetList.end(); ++i) {
    (*i)->resize(width, (*i)->height());
  }
}

void CDoasTableColumn::addRow(int height, const QVariant &cellData)
{
  QWidget *tmp = createCellWidget(cellData);

  tmp->setParent(m_viewport);
  tmp->resize(columnWidth() - 2 * m_xBorder, height - 2 * m_yBorder);

  // is it visible ??
  int y = 0;
  int index = m_rowOffset;
  while (index < m_widgetList.count()) {
    y += m_widgetList.at(index)->height() + 2 * m_yBorder;
    ++index;
  }
  // y is now the vertical position for the last widget ...
  tmp->move(m_xBorder, y + m_yBorder);
  if (y < m_viewport->height())
    tmp->show();
  else
    tmp->hide();

  m_widgetList.push_back(tmp);
}

void CDoasTableColumn::removeRow(int rowIndex)
{
  if (rowIndex >= 0 && rowIndex < m_widgetList.count()) {

    QWidget *tmp = m_widgetList.at(rowIndex);
    m_widgetList.removeAt(rowIndex);

    tmp->hide();

    layoutAndDisplay();
    delete tmp;

    resizeWidgets();
  }

}

void CDoasTableColumn::setViewportBackgroundColour(const QColor &c)
{
  QPalette palette(m_viewport->palette());
  palette.setColor(QPalette::Window, c);
  m_viewport->setPalette(palette);
  m_viewport->setAutoFillBackground(true);
}

void CDoasTableColumn::setCellBorders(int xB, int yB)
{
  m_xBorder = xB;
  m_yBorder = yB;
}

const QWidget* CDoasTableColumn::getWidget(int rowIndex) const
{
  const QWidget *tmp = NULL;

  if (rowIndex >= 0 && rowIndex <= m_widgetList.count())
    tmp = m_widgetList.at(rowIndex);

  return tmp;
}

QWidget* CDoasTableColumn::getWidgetNonConst(int rowIndex)
{
  QWidget *tmp = NULL;

  if (rowIndex >= 0 && rowIndex <= m_widgetList.count())
    tmp = m_widgetList.at(rowIndex);

  return tmp;
}

void CDoasTableColumn::slotCellDataChanged(const QWidget *src, const QVariant &cellData)
{
  // determine the row
  int rowIndex = m_widgetList.indexOf(const_cast<QWidget*>(src));
  if (rowIndex != -1) {
    m_owner->notifyCellDataChanged(rowIndex, this, cellData);
  }
}

//-------------------------------------

CDoasTableColumnHeader::CDoasTableColumnHeader(const QString &label, CDoasTable *owner) :
  CDoasTableColumn(label, owner, 120)
{
}

void CDoasTableColumnHeader::addRow(int height, const QString &label) {
  CDoasTableColumn::addRow(height, QVariant(label));

  setLabel(rowCount()-1, label);
}

QWidget* CDoasTableColumnHeader::createCellWidget(const QVariant &cellData)
{
  QLabel *tmp = new QLabel;
  tmp->setText(cellData.toString());
  tmp->setLineWidth(2);
  tmp->setFrameStyle(QFrame::Panel | QFrame::Raised);
  return tmp;
}

QVariant CDoasTableColumnHeader::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QLabel *tmp = dynamic_cast<const QLabel*>(p);
    if (tmp)
      return QVariant(tmp->text());
  }

  return QVariant();
}

void CDoasTableColumnHeader::setLabel(int rowIndex, const QString &label)
{
  QWidget *p = getWidgetNonConst(rowIndex);
  if (p) {
    QLabel *tmp = dynamic_cast<QLabel*>(p);
    if (tmp) {
      tmp->setText(label);
      tmp->setMinimumSize(QSize(tmp->fontMetrics().boundingRect(label).width()
                                + 2*tmp->fontMetrics().averageCharWidth(),
                                tmp->height()));
    }
  }

  // adapt widgets in column to a new size if required
  resizeWidgets();

  owner()->headerChanged();
}

void CDoasTableColumnHeader::setCellData(int rowIndex, const QVariant &cellData)
{
  setLabel(rowIndex, cellData.toString());
}


//-------------------------------------

CDoasTableColumnLineEdit::CDoasTableColumnLineEdit(QWidget *parent) :
  QLineEdit(parent)
{
  setFrame(false);
}

void CDoasTableColumnLineEdit::slotTextChanged(const QString &newText)
{
  QVariant cellData(newText);

  emit signalTextChanged(this, cellData);
}

CDoasTableColumnEdit::CDoasTableColumnEdit(const QString &label, CDoasTable *owner, int minimumWidth) :
  CDoasTableColumn(label, owner, minimumWidth)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);
}

QWidget* CDoasTableColumnEdit::createCellWidget(const QVariant &cellData)
{
  CDoasTableColumnLineEdit *tmp = new CDoasTableColumnLineEdit;

  tmp->setText(cellData.toString());

  connect(tmp, SIGNAL(textChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));

  return tmp;
}

QVariant CDoasTableColumnEdit::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QLineEdit *tmp = dynamic_cast<const QLineEdit*>(p);
    if (tmp)
      return QVariant(tmp->text());
  }

  return QVariant();
}

void CDoasTableColumnEdit::setCellData(int rowIndex, const QVariant &cellData)
{
  QWidget *p = getWidgetNonConst(rowIndex);
  if (p) {
    QLineEdit *tmp = dynamic_cast<QLineEdit*>(p);
    if (tmp)
      tmp->setText(cellData.toString());
  }
}

//-------------------------------------

CDoasTableColumnIntEdit::CDoasTableColumnIntEdit(int minimum, int maximum,
						 const QString &label, CDoasTable *owner, int minimumWidth) :
  CDoasTableColumnEdit(label, owner, minimumWidth),
  m_minimum(minimum),
  m_maximum(maximum)
{
}

QWidget* CDoasTableColumnIntEdit::createCellWidget(const QVariant &cellData)
{
  QLineEdit *tmp = dynamic_cast<QLineEdit*>(CDoasTableColumnEdit::createCellWidget(cellData));

  assert (tmp != NULL);

  tmp->setValidator(new QIntValidator(m_minimum, m_maximum, tmp));

  return tmp;
}

QVariant CDoasTableColumnIntEdit::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QLineEdit *tmp = dynamic_cast<const QLineEdit*>(p);
    if (tmp) {
      // ensure the data is valid ... use the validator (essential for cells that are not initialized with a valid value)
      QString tmpStr = tmp->text();
      tmp->validator()->fixup(tmpStr);
      return QVariant(tmpStr.toInt());
    }
  }

  return QVariant();
}

//-------------------------------------

CDoasTableColumnDoubleEdit::CDoasTableColumnDoubleEdit(double minimum, double maximum, int decimals,
						       const QString &label, CDoasTable *owner, int minimumWidth) :
  CDoasTableColumnEdit(label, owner, minimumWidth),
  m_minimum(minimum),
  m_maximum(maximum),
  m_decimals(decimals)
{
}

QWidget* CDoasTableColumnDoubleEdit::createCellWidget(const QVariant &cellData)
{
  QLineEdit *tmp = dynamic_cast<QLineEdit*>(CDoasTableColumnEdit::createCellWidget(cellData));

  assert (tmp != NULL);

  tmp->setValidator(new CDoubleFixedFmtValidator(m_minimum, m_maximum, m_decimals, tmp));

  return tmp;
}

QVariant CDoasTableColumnDoubleEdit::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QLineEdit *tmp = dynamic_cast<const QLineEdit*>(p);
    if (tmp) {
      // ensure the data is valid ... use the validator (essential for cells that are not initialized with a valid value)
      QString tmpStr = tmp->text();
      tmp->validator()->fixup(tmpStr);
      return QVariant(tmpStr.toDouble());
    }
  }

  return QVariant();
}

//-------------------------------------

bool CDoasTableColumnComboBox::eventFilter(QObject *o, QEvent *e) {
  if (e->type() == QEvent::MouseButtonRelease) {
    if (static_cast<QMouseEvent*>(e)->button() == Qt::RightButton) {
      return true;
    }
  }
  return false;
}

void CDoasTableColumnComboBox::list_context_menu(QPoint pos) {
  QAbstractItemView* comboView = view();
  QModelIndex index = comboView->indexAt(pos);
  QAction *selectedItem;

  int optionIndex=-1;

  if ((m_type!=ANLYS_COMBO_NONE) && (m_menu!=NULL) && ((selectedItem=m_menu->exec(comboView->mapToGlobal(pos)))!=NULL) )
   {

    QString selectedText=selectedItem->text();;
    char str[100];

    if ((m_type==ANLYS_COMBO_DIFF_ORTHO) && ((optionIndex=index.row())>ANLYS_DIFFORTHO_DIFFERENTIAL_XS))
     sprintf(str,"%s %s",((optionIndex==ANLYS_DIFFORTHO_ORTHOGONALIZATION)? "Orthogonalize to" : "Subtract from"),selectedText.toLatin1().data());
    else if ((m_type==ANLYS_COMBO_CORRECTION) && ((optionIndex=index.row())==ANLYS_CORRECTION_TYPE_MOLECULAR_RING))
     sprintf(str,"Mol. ring (%s)",selectedText.toLatin1().data());
    else if ((m_type==ANLYS_COMBO_CORRECTION) && ((optionIndex=index.row())==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))
     sprintf(str,"Slope+Mol. ring (%s)",selectedText.toLatin1().data());

    setItemText(optionIndex,str);
   }
}

CDoasTableColumnComboBox::CDoasTableColumnComboBox(int type,const QString &excludedSymbol,QWidget *parent) :
  QComboBox(parent),
  m_excludedSymbol(excludedSymbol),
  m_menu(NULL)
{
  m_type=type;

  QAbstractItemView* comboView = view();
  comboView->viewport()->installEventFilter(this);
  comboView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(comboView, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(list_context_menu(QPoint)));

}

int CDoasTableColumnComboBox::GetIndexDiffOrtho(QString text)
 {
  int index;

  index=-1;

  if (text=="None")
   index=ANLYS_DIFFORTHO_NONE;
  else if (text=="Differential XS")
   index=ANLYS_DIFFORTHO_DIFFERENTIAL_XS;
  else if (text.startsWith("Orthogonalize to") || text.startsWith("Subtract from"))
   {
    QStringList tokens = text.split( " " );
    QString xs = tokens.value( tokens.length() - 1 );

    if (m_symbols.contains(xs))
     index=(text.startsWith("Orthogonalize to"))?ANLYS_DIFFORTHO_ORTHOGONALIZATION:ANLYS_DIFFORTHO_SUBTRACTION;
   }
  else if (m_symbols.contains(text))
   {
    char str[100];

    index=ANLYS_DIFFORTHO_ORTHOGONALIZATION;
    sprintf(str,"Orthogonalize to %s",text.toLatin1().data());
    text=QString(str);
   }

   if (index!=-1)
    setItemText(index,text);

   return index;
 }

int CDoasTableColumnComboBox::GetIndexCorrection(QString text)
 {
  int index;

  index=-1;

  if (text=="None")
   index=ANLYS_CORRECTION_TYPE_NONE;
  else if (text=="Slope")
   index=ANLYS_CORRECTION_TYPE_SLOPE;
  else if (text=="Pukite")
   index=ANLYS_CORRECTION_TYPE_PUKITE;
  else if (text.startsWith("Mol. ring") || text.startsWith("Slope+Mol. ring"))
   {
    QStringList tokens = text.split( " " );
    QString xs = tokens.value( tokens.length() - 1 );
    char str[100];

    sscanf(xs.toLatin1().data(),"(%[^)])",str);

    if (m_symbols.contains(QString(str)))
     index=(text.startsWith("Mol. ring"))?ANLYS_CORRECTION_TYPE_MOLECULAR_RING:ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE;
   }
  else if (m_symbols.contains(text))
   {
    char str[100];

    index=ANLYS_CORRECTION_TYPE_MOLECULAR_RING;
    sprintf(str,"Mol. ring (%s)",text.toLatin1().data());
    text=QString(str);
   }

   if (index!=-1)
    setItemText(index,text);

   return index;
 }

int CDoasTableColumnComboBox::GetIndex(QString text)
 {
  if (m_type==ANLYS_COMBO_DIFF_ORTHO)
   return(GetIndexDiffOrtho(text));
  else if (m_type==ANLYS_COMBO_CORRECTION)
   return(GetIndexCorrection(text));
  else
   return(-1);
 }

void CDoasTableColumnComboBox::slotSymbolListChanged(const QStringList &symbols)
{
  // if an initial value is set, then try and select it ...
  QString text = m_pendingInitial.isNull() ? currentText() : m_pendingInitial;
  int index;

  m_symbols=symbols;

  if (m_menu!=NULL)
   {
    delete(m_menu);
    m_menu=NULL;
   }

  m_menu=new QMenu(this);

  if (symbols.count())
   {
    QStringList::const_iterator it = symbols.begin();
    while (it != symbols.end()) {
      if (*it != m_excludedSymbol)
       m_menu->addAction(*it);
      ++it;
    }
   }

  // try and reselect ...
  index = GetIndex(text);

  if (index != -1) {
    setCurrentIndex(index);
    if (text == m_pendingInitial)
      m_pendingInitial = QString(); // has been set ...
  }

}

void CDoasTableColumnComboBox::initialSelection(const QString &text)
{
  // sets the initial value, which might not yet be in the list ...

  int index = GetIndex(text);
  if (index != -1) {
    setCurrentIndex(index); // exists .. just set it
  }
  else {
    m_pendingInitial = text; // set it and try again later (in slotSymbolListChanged)
  }
}

void CDoasTableColumnComboBox::slotTextChanged(const QString &newText)
{
  QVariant cellData(newText);

  emit signalTextChanged(this, cellData);
}

CDoasTableColumnCombo::CDoasTableColumnCombo(const QStringList &tags, const QString &label, CDoasTable *owner,
				 int minimumWidth) :
  CDoasTableColumn(label, owner, minimumWidth),
  m_tags(tags)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);
}

QWidget* CDoasTableColumnCombo::createCellWidget(const QVariant &cellData)
{
  CDoasTableColumnComboBox *tmp = new CDoasTableColumnComboBox(ANLYS_COMBO_NONE);

  QStringList::iterator it = m_tags.begin();
  while (it != m_tags.end()) {
    tmp->addItem(*it);
    ++it;
  }

  int index = tmp->findText(cellData.toString());
  if (index != -1)
    tmp->setCurrentIndex(index);

  connect(tmp, SIGNAL(currentIndexChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));

  return tmp;
}

QVariant CDoasTableColumnCombo::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QComboBox *tmp = dynamic_cast<const QComboBox*>(p);
    if (tmp)
      return QVariant(tmp->currentText());
  }

  return QVariant();
}

void CDoasTableColumnCombo::setCellData(int rowIndex, const QVariant &cellData)
{
  QWidget *p = getWidgetNonConst(rowIndex);

  if (p) {
    QComboBox *tmp = dynamic_cast<QComboBox*>(p);
    if (tmp) {
      int index = tmp->findText(cellData.toString());
      if (index != -1)
	tmp->setCurrentIndex(index);
    }
  }
}

//-------------------------------------

CDoasTableColumnCheckBox::CDoasTableColumnCheckBox(QWidget *parent) :
  QCheckBox(parent)
{
}

void CDoasTableColumnCheckBox::slotStateChanged(int state)
{
  QVariant cellData(state == Qt::Checked);

  emit signalStateChanged(this, cellData);
}

CDoasTableColumnCheck::CDoasTableColumnCheck(const QString &label, CDoasTable *owner, int minimumWidth) :
  CDoasTableColumn(label, owner, minimumWidth)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(minimumWidth / 3, 0);
}

QWidget* CDoasTableColumnCheck::createCellWidget(const QVariant &cellData)
{
  CDoasTableColumnCheckBox *tmp = new CDoasTableColumnCheckBox;
  tmp->setCheckState(cellData.toBool() ? Qt::Checked : Qt::Unchecked);

  connect(tmp, SIGNAL(stateChanged(int)), tmp, SLOT(slotStateChanged(int)));
  connect(tmp, SIGNAL(signalStateChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));

  return tmp;
}

QVariant CDoasTableColumnCheck::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QCheckBox *tmp = dynamic_cast<const QCheckBox*>(p);
    if (tmp)
      return QVariant(tmp->checkState() == Qt::Checked);
  }

  return QVariant();
}

void CDoasTableColumnCheck::setCellData(int rowIndex, const QVariant &cellData)
{
  QWidget *p = getWidgetNonConst(rowIndex);
  if (p) {
    QCheckBox *tmp = dynamic_cast<QCheckBox*>(p);
    if (tmp)
      tmp->setCheckState(cellData.toBool() ? Qt::Checked : Qt::Unchecked);
  }
}
