/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
#include <QFileDialog>

#include "CWTableRegion.h"
#include "CPreferences.h"

#include "debugutil.h"

CWTableRegion::CWTableRegion(QWidget *parent) :
  QTableView(parent)
{
  m_model = new CMultiPageTableModel(this);
  setModel(m_model);

  setGridStyle(Qt::DotLine);

  // hide the headers ... not useful in this context
  horizontalHeader()->hide();
  verticalHeader()->hide();
}

void CWTableRegion::slotTablePages(const QList<std::shared_ptr<const CTablePageData> > &pageList)
{
  m_model->slotTablePages(pageList);
}

void CWTableRegion::slotDisplayPage(int pageNumber)
{
  m_model->setActivePage(pageNumber);

  setVisible(false); // if the table is visible, only the currently
                     // visible parts of the Table are taken into
                     // account when computing the column width using
                     // resize...ToContents()
  resizeRowsToContents();
  resizeColumnsToContents();
  setVisible(true);
}

void CWTableRegion::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  menu.addAction("Save As...", this, SLOT(slotSaveAs()));

  menu.exec(e->globalPos());
}

void CWTableRegion::slotSaveAs()
{
  // here - filename - also remember about dir -> file in output

  CPreferences *prefs = CPreferences::instance();

  QString fileName = QFileDialog::getSaveFileName(this, "SaveAs Table",
                          prefs->directoryName("Table"),
                          "Text file (*.txt)");

  if (!fileName.isEmpty()) {

    // add a .txt extension if there is no extension set
    if (!fileName.contains('.'))
      fileName += ".txt";

    prefs->setDirectoryNameGivenFile("Table", fileName);

    FILE *fp = fopen(fileName.toLocal8Bit().constData(), "w");

    if (fp != NULL) {
      int rows = m_model->rowCount();
      int cols = m_model->columnCount();

      for (int j=0; j<rows; ++j) {
    for (int i=0; i<cols; ++i) {
      QString tmp(m_model->index(j, i, QModelIndex()).data().toString());

      if (i != 0)
        fputc('\t', fp);
      fprintf(fp, "%s", tmp.toLocal8Bit().constData());
    }
    fputc('\n', fp);
      }

      fclose(fp);
    }
  }
}
