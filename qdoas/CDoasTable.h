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


#ifndef _CDOASTABLE_GUARD
#define _CDOASTABLE_GUARD

#include <QLabel>
#include <QFrame>
#include <QScrollBar>
#include <QList>
#include <QColor>
#include <QStringList>
#include <QVariant>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPoint>
#include <QMenu>

#include <QResizeEvent>
#include "constants.h"

class CDoasTableColumn;
class CDoasTableColumnHeader;

class CDoasTable : public QFrame
{
Q_OBJECT
 public:
  CDoasTable(const QString &label, int headerHeight = 24, QWidget *parent = 0);

  int headerHeight(void) const;

  void createColumnEdit(const QString &label, int columnWidth);                                 // strings
  void createColumnEdit(int minimum, int maximum, const QString &label, int columnWidth);       // integers
  void createColumnEdit(double minimum, double maximum, int decimals, const QString &label, int columnWidth); // doubles
  void createColumnCombo(const QString &label, int columnWidth, const QStringList &tags);
  void createColumnCheck(const QString &label, int columnWidth);

  virtual void addRow(int height, const QString &label, QList<QVariant> &cellData);
  virtual void removeRow(int rowIndex);
  int rowCount(void) const;
  QString rowLabel(int rowIndex) const;
  int columnCount(void) const;
  void setColumnOffset(int offset);

  void setColumnEnabled(int columnIndex, bool enabled);
  void setCellEnabled(int rowIndex, int columnIndex, bool enabled);

  bool isColumnEnabled(int columnIndex) const;

  QList<QVariant> getCellData(int rowIndex) const;
  QVariant getCellData(int rowIndex, int columnIndex) const;

  void headerChanged(void);
  virtual void cellDataChanged(int row, int column, const QVariant &cellData);

  void notifyCellDataChanged(int row, const CDoasTableColumn *column, const QVariant &cellData);

 protected:
  void addColumn(CDoasTableColumn *column); // for adding custon columns in derived tables
  int rowIndexAtPosition(int yPixel) const;
  void setHeaderLabel(int rowIndex, const QString &label);

  void setCellData(int rowIndex, int columnIndex, const QVariant &cellData);

  QWidget* directAccessToCellWidget(int rowIndex, int columnIndex); // Try NOT to use this

  void updateCols(int wid, int hei);
  void updateCols(void);
  virtual void resizeEvent(QResizeEvent *e);

  void calcHorizontalScrollRange(void);
  void calcVerticalScrollRange(void);

 public slots:
  void slotMovedHorizontalScrollBar(int value);
  void slotMovedVerticalScrollBar(int value);

 private:
  QScrollBar *m_hsb, *m_vsb;

  int m_titleHeight, m_sbThickness, m_centralWidth, m_centralHeight;

  CDoasTableColumnHeader *m_header;
  QList<CDoasTableColumn*> m_columnList;
  QList<int> m_rowHeightList;
  int m_columnOffset;
};

inline int CDoasTable::headerHeight(void) const { return m_titleHeight; }
inline int CDoasTable::rowCount(void) const { return m_rowHeightList.count(); }


class CDoasTableColumn : public QObject
{
Q_OBJECT
 public:
  CDoasTableColumn(const QString &label, CDoasTable *owner, int minimumWidth);

  void setColumnHorizontalPosition(int xPosition);

  void setViewportHeight(int vpHeight);
  void hide(void);
  void setEnabled(bool enabled);
  void setRowEnabled(int rowIndex, bool enabled);

  bool isEnabled(void) const;

  void setRowOffset(int offset);
  int rowOffset(void) const;

  int columnWidth(void) const;
  int rowCount(void) const;
  void addRow(int height, const QVariant &cellData);
  void removeRow(int rowIndex);

  virtual QVariant getCellData(int rowIndex) const = 0;

  void setCellDataWrapper(int rowIndex, const QVariant &cellData);
  QWidget* directAccessToCellWidget(int rowIndex); // do NOT use this

 protected:
  virtual void setCellData(int rowIndex, const QVariant &cellData) = 0;
  virtual QWidget* createCellWidget(const QVariant &cellData) = 0;
  void resizeWidgets(void);

  void setViewportBackgroundColour(const QColor &c);
  void setCellBorders(int xB, int yB);

  const QWidget* getWidget(int rowIndex) const;
  QWidget* getWidgetNonConst(int rowIndex);
  CDoasTable* owner(void) const;

  QLabel *m_header;

 private:
  void layoutAndDisplay(void);

 public slots:
   void slotCellDataChanged(const QWidget *src, const QVariant &cellData);

 private:
  CDoasTable *m_owner;
  int m_rowOffset;
  int m_xPosition;
  int m_xBorder, m_yBorder;

  const int m_minimumWidth; // minimal width of the column, in pixels.

  QList<QWidget*> m_widgetList;
  QFrame *m_viewport; // visible region for the child widgets - parented to the viewport
};

inline CDoasTable* CDoasTableColumn::owner(void) const { return m_owner; }
inline int CDoasTableColumn::rowOffset(void) const { return m_rowOffset; }
inline void CDoasTableColumn::setCellDataWrapper(int rowIndex, const QVariant &cellData) { setCellData(rowIndex, cellData); }
inline QWidget* CDoasTableColumn::directAccessToCellWidget(int rowIndex) { return getWidgetNonConst(rowIndex); }


class CDoasTableColumnHeader : public CDoasTableColumn
{
 public:
  CDoasTableColumnHeader(const QString &label, CDoasTable *owner);

  virtual QVariant getCellData(int rowIndex) const;

  void setLabel(int rowIndex, const QString &label);

  void addRow(int height, const QString &label);

 protected:
  virtual void setCellData(int rowIndex, const QVariant &cellData);
  virtual QWidget* createCellWidget(const QVariant &cellData);
};

class CDoasTableColumnLineEdit : public QLineEdit
{
Q_OBJECT
 public:
  CDoasTableColumnLineEdit(QWidget *parent = 0);

 public slots:
  void slotTextChanged(const QString &newText);

 signals:
 void signalTextChanged(const QWidget *src, const QVariant &cellData);
};

class CDoasTableColumnEdit : public CDoasTableColumn
{
 public:
  CDoasTableColumnEdit(const QString &label, CDoasTable *owner, int width);

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual void setCellData(int rowIndex, const QVariant &cellData);
  virtual QWidget* createCellWidget(const QVariant &cellData);
};

class CDoasTableColumnIntEdit : public CDoasTableColumnEdit
{
 public:
  CDoasTableColumnIntEdit(int minimum, int maximum, const QString &label, CDoasTable *owner, int width);

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);

 private:
  int m_minimum, m_maximum;
};

class CDoasTableColumnDoubleEdit : public CDoasTableColumnEdit
{
 public:
  CDoasTableColumnDoubleEdit(double minimum, double maximum, int decimals,
			     const QString &label, CDoasTable *owner, int width);

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);

 private:
  double m_minimum, m_maximum;
  int m_decimals;
};

class CDoasTableColumnComboBox : public QComboBox
{
Q_OBJECT
 public:
  CDoasTableColumnComboBox(int type=ANLYS_COMBO_NONE,const QString &excludedSymbol= QString(),QWidget *parent = 0);
  bool eventFilter(QObject *o, QEvent *e);
  int GetIndexDiffOrtho(QString text);
  int GetIndexCorrection(QString text);
  int GetIndex(QString text);
  void initialSelection(const QString &text);

 public slots:
  void slotTextChanged(const QString &newText);
  void list_context_menu(QPoint pos);
  void slotSymbolListChanged(const QStringList &symbols);

 signals:
  void signalTextChanged(const QWidget *src, const QVariant &cellData);

 private:
  QString m_excludedSymbol, m_pendingInitial;
  QStringList m_symbols;
  QMenu *m_menu;
  int m_type;
};

class CDoasTableColumnCombo : public CDoasTableColumn
{
 public:
  CDoasTableColumnCombo(const QStringList &tags, const QString &label, CDoasTable *owner, int width);

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual void setCellData(int rowIndex, const QVariant &cellData);
  virtual QWidget* createCellWidget(const QVariant &cellData);

private:
  QStringList m_tags;
};

class CDoasTableColumnCheckBox : public QCheckBox
{
Q_OBJECT
 public:
  CDoasTableColumnCheckBox(QWidget *parent = 0);

 public slots:
  void slotStateChanged(int);

 signals:
  void signalStateChanged(const QWidget *src, const QVariant &cellData);
};

class CDoasTableColumnCheck : public CDoasTableColumn
{
 public:
  CDoasTableColumnCheck(const QString &label, CDoasTable *owner, int width);

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual void setCellData(int rowIndex, const QVariant &cellData);
  virtual QWidget* createCellWidget(const QVariant &cellData);
};

#endif

