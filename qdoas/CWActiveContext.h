/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWACTIVECONTEXT_H_GUARD
#define _CWACTIVECONTEXT_H_GUARD

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include <QList>

#include "CPlotPageData.h"
#include "CImagePageData.h"
#include "RefCountPtr.h"

class CWEditor;
class CWPlotRegion;

class CWActiveContext : public QFrame
{
Q_OBJECT
 public:
  CWActiveContext(QWidget *parent = 0);
  virtual ~CWActiveContext() {};

  void addEditor(CWEditor *editor); // reparents and takes ownership of editor.

  void savePreferences(void) const;

 protected:
  virtual bool event(QEvent *e);
  virtual void resizeEvent(QResizeEvent *e);

  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;

 public slots:
  void slotOkButtonClicked();
  void slotCancelButtonClicked();
  void slotHelpButtonClicked();

  void slotAcceptOk(bool canDoOk);

  void slotPlotPages(const QList< RefCountConstPtr<CPlotPageData> > &pageList);
  void slotCurrentGraphTabChanged(int index);
  void slotCurrentActiveTabChanged(int index);

  void slotEditPlotProperties();
  void slotPrintPlots();
  void slotExportPlots();

 private:
  void discardCurrentEditor(void);
  void moveAndResizeButtons(int hei);
  void moveAndResizeGraph(int hei);
  void moveAndResizeActiveEditor(void);
  void moveAndResizeActiveTab(void);

 signals:
  void signalActivePageChanged(int pageNumber);

 private:
  CWEditor *m_activeEditor;
  QList<CWEditor*> m_editorList;
  QPushButton *m_helpButton, *m_okButton, *m_cancelButton;
  QLabel *m_title;
  QTabBar *m_graphTab, *m_activeTab;
  CWPlotRegion *m_plotRegion;
  QString m_graphTitleStr;

  int m_titleRegionHeight;
  int m_buttonRegionHeight;
  int m_graphTabRegionHeight;
  int m_activeTabRegionWidth;
  int m_centralRegionHeight;
  int m_centralRegionWidth;

  QSize m_minGeneralSize, m_minEditSize;
  bool m_blockActiveTabSlot;
};

#endif
