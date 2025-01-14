/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QResizeEvent>
#include <QPalette>
#include <QColor>
#include <QBrush>
#include <QList>

#include "CWEditor.h"
#include "CWActiveContext.h"
#include "CWPlotPage.h"
#include "CWPlotRegion.h"
#include "CWPlotPropertiesEditor.h"
#include "CWPlotPropertiesConfig.h"

#include "debugutil.h"

const int cBorderSize = 5;
const QRgb cEditTitleBackgroundColour  = 0xFF5986EC;
const QRgb cEditTitleTextColour        = 0xFFFFFFFF;
const QRgb cGraphTitleBackgroundColour = 0xFFF59F43;
const QRgb cGraphTitleTextColour       = 0xFF000000;

using std::shared_ptr;

CWActiveContext::CWActiveContext(QWidget *parent) :
  QFrame(parent),
  m_activeEditor(NULL),
  m_titleRegionHeight(0),
  m_buttonRegionHeight(0),
  m_graphTabRegionHeight(0),
  m_activeTabRegionWidth(0),
  m_centralRegionHeight(0),
  m_centralRegionWidth(0),
  m_blockActiveTabSlot(false)
{

  // Layout is managed explicitly

  // title string - change background colour
  m_title = new QLabel(this);
  m_title->setAlignment(Qt::AlignHCenter);
  m_title->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
  m_title->setLineWidth(1);

  QPalette palette(m_title->palette());
  palette.setColor(QPalette::Window, QColor(cGraphTitleBackgroundColour));
  palette.setColor(QPalette::WindowText, QColor(cGraphTitleTextColour));
  m_title->setPalette(palette);

  m_title->setAutoFillBackground(true);

  m_title->move(0,0);
  // title region height
  QSize tmpSize = m_title->sizeHint();
  if (tmpSize.isValid())
    m_titleRegionHeight = tmpSize.height();
  else
    m_titleRegionHeight = 15;

  // buttons
  m_helpButton = new QPushButton("Help", this);
  m_okButton = new QPushButton("Ok", this);
  m_cancelButton = new QPushButton("Cancel", this);
  m_cancelButton->setDefault(true);

  int minWidth = 4 * cBorderSize;

  tmpSize = m_helpButton->sizeHint();
  if (tmpSize.isValid()) {
    minWidth += tmpSize.width();
    if (tmpSize.height() > m_buttonRegionHeight)
      m_buttonRegionHeight = tmpSize.height();
    m_helpButton->resize(tmpSize);
  }

  tmpSize = m_okButton->sizeHint();
  if (tmpSize.isValid()) {
    minWidth += tmpSize.width();
    if (tmpSize.height() > m_buttonRegionHeight)
      m_buttonRegionHeight = tmpSize.height();
    m_okButton->resize(tmpSize);
  }

  tmpSize = m_cancelButton->sizeHint();
  if (tmpSize.isValid()) {
    minWidth += tmpSize.width();
    if (tmpSize.height() > m_buttonRegionHeight)
      m_buttonRegionHeight = tmpSize.height();
    m_cancelButton->resize(tmpSize);
  }

  // border around the buttons
  if (!m_buttonRegionHeight) {
    tmpSize = QSize(75,22);
    m_helpButton->resize(tmpSize);
    m_okButton->resize(tmpSize);
    m_cancelButton->resize(tmpSize);
    m_buttonRegionHeight = tmpSize.height();
  }
  m_buttonRegionHeight += 2 * cBorderSize;

  // graph tab-bar
  m_graphTab = new QTabBar(this);
  m_graphTab->setShape(QTabBar::TriangularSouth);
  m_graphTab->addTab("Qdoas"); // need one tab to get a sensible height from sizeHint.

  tmpSize = m_graphTab->sizeHint();
  if (tmpSize.isValid()) {
    if (tmpSize.width() > minWidth)
      minWidth = tmpSize.width();
    m_graphTabRegionHeight = tmpSize.height();
  }
  else {
    m_graphTabRegionHeight = 20;
  }

  // active tab-bar

  // The items in the tab bar are kept in sync with the editor list, with 'off-by-one'
  // indexing because tab-0 is always the plot.
  m_activeTab = new QTabBar(this);
  m_activeTab->setShape(QTabBar::RoundedEast);
  m_activeTab->addTab("Plot"); // need one tab to get a sensible width from sizeHint.

  tmpSize = m_activeTab->sizeHint();
  if (tmpSize.isValid()) {
    if (tmpSize.width() > minWidth)
      minWidth = tmpSize.width();
    m_activeTabRegionWidth = tmpSize.width();
  }
  else {
    m_activeTabRegionWidth = 20;
  }

  // plt region and properties
  CPlotProperties prop;
  CWPlotPropertiesConfig::loadFromPreferences(prop);

  m_plotRegion = new CWPlotRegion(this);
  m_plotRegion->setProperties(prop);

  // this might be adjusted in edit mode ...
  m_minGeneralSize = QSize(minWidth, m_titleRegionHeight + m_graphTabRegionHeight + 50);
  setMinimumSize(m_minGeneralSize);

  // connections
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOkButtonClicked()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelpButtonClicked()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));

  connect(m_graphTab, SIGNAL(currentChanged(int)),
      this, SLOT(slotCurrentGraphTabChanged(int)));
  connect(m_activeTab, SIGNAL(currentChanged(int)),
      this, SLOT(slotCurrentActiveTabChanged(int)));

  // explicitly hide the edit stuff to start with
  m_helpButton->hide();
  m_okButton->hide();
  m_cancelButton->hide();

  // explicitly hide the active tab
  m_activeTab->hide();
}

void CWActiveContext::addEditor(CWEditor *editor)
{
  // configure/reconfigure the layout.

  if (m_editorList.empty()) {
    m_centralRegionWidth = width() - m_activeTabRegionWidth;
    m_centralRegionHeight = height() - m_titleRegionHeight - m_buttonRegionHeight;
    moveAndResizeActiveTab();

    // add the tab and the editor to the list
    editor->setParent(this);
    m_editorList.push_back(editor);

    // take the last part of the context tag name as the tab label
    if (editor->editContextTag().contains(':')) {
      QString label = editor->editContextTag();
      m_activeTab->addTab(label.remove(0, label.lastIndexOf(':')+1));
    }
    else
      m_activeTab->addTab(editor->editContextTag());

    // set this tab as active ...
    m_activeTab->setCurrentIndex(1);
  }
  else {
    // check for duplicates

    int index = 0;
    QList<CWEditor*>::iterator it = m_editorList.begin();
    while (it != m_editorList.end() && (*it)->editContextTag() != editor->editContextTag()) {
      ++it;
      ++index;
    }
    if (it != m_editorList.end()) {
      // exists already ...
      delete editor;
    }
    else {
      index = m_editorList.size();
      editor->setParent(this);
      m_editorList.push_back(editor);

      // take the last part of the context tag name as the tab label
      if (editor->editContextTag().contains(':')) {
    QString label = editor->editContextTag();
    m_activeTab->addTab(label.remove(0, label.lastIndexOf(':')+1));
      }
      else
    m_activeTab->addTab(editor->editContextTag());

    }

    // set this tab as active ...
    m_activeTab->setCurrentIndex(index + 1);
  }
}

void CWActiveContext::savePreferences(void) const
{
  CWPlotPropertiesConfig::saveToPreferences(m_plotRegion->properties());
}

QSize CWActiveContext::minimumSizeHint() const
{
  return QSize(m_minGeneralSize.width() + 30, m_minGeneralSize.height());
}

QSize CWActiveContext::sizeHint() const
{
  return QSize(m_minGeneralSize.width() + 300, m_minGeneralSize.height() + 300);
}

bool CWActiveContext::event(QEvent *e)
{
  if (e->type() == QEvent::LayoutRequest) {

    // only need to do something if the editor is active
    if (m_activeEditor)
      moveAndResizeActiveEditor();

    e->accept();
    return true;
  }

  return QFrame::event(e);
}

void CWActiveContext::resizeEvent(QResizeEvent *e)
{
  int wid = e->size().width();
  int hei = e->size().height();

  // resize the title to the full width - already positioned
  m_title->resize(wid, m_titleRegionHeight);

  // determine the size of the central region ...
  m_centralRegionHeight = hei - m_titleRegionHeight;
  if (m_activeEditor) {
    // an editor must be visible
    m_centralRegionHeight -= m_buttonRegionHeight;
  }
  else {
    m_centralRegionHeight -= m_graphTabRegionHeight;
  }
  m_centralRegionWidth = wid;
  if (m_activeTab->count() > 1) {
    m_centralRegionWidth -= m_activeTabRegionWidth;
    // the active tab is displayed - reposition and resize
    moveAndResizeActiveTab();
  }

  if (m_activeEditor) {
    moveAndResizeButtons(hei);
    // position the editor ...
    moveAndResizeActiveEditor();
  }
  else {
    moveAndResizeGraph(hei);
   }
}

void CWActiveContext::moveAndResizeActiveTab(void)
{
  m_activeTab->move(m_centralRegionWidth, m_titleRegionHeight);
  m_activeTab->resize(m_activeTabRegionWidth, m_centralRegionHeight + m_buttonRegionHeight);
}

void CWActiveContext::moveAndResizeButtons(int hei)
{
  int tmpW, tmpH;

  // position the widgets in the control region
  tmpH = hei - m_buttonRegionHeight + cBorderSize;
  m_helpButton->move(cBorderSize, tmpH);

  tmpW = m_centralRegionWidth - cBorderSize - m_cancelButton->width();
  m_cancelButton->move(tmpW, tmpH);
  tmpW -= cBorderSize + m_okButton->width();
  m_okButton->move(tmpW, tmpH);
}

void CWActiveContext::moveAndResizeGraph(int hei)
{
  // position and resize the tab widget
  m_graphTab->move(0, hei - m_graphTabRegionHeight);
  m_graphTab->resize(m_centralRegionWidth, m_graphTabRegionHeight);

  // scroll area
  // m_plotPage->layoutPlots(wid - 16); TODOTODO
  m_plotRegion->move(0, m_titleRegionHeight);
  m_plotRegion->resize(m_centralRegionWidth, m_centralRegionHeight);
}

void CWActiveContext::moveAndResizeActiveEditor(void)
{
  // active editor guaranteed to be valid

  QSize tmpSize = m_activeEditor->minimumSizeHint();

  m_minEditSize = tmpSize;
  m_minEditSize.rheight() += m_titleRegionHeight + m_buttonRegionHeight;

  setMinimumSize(m_minEditSize.expandedTo(m_minGeneralSize));

  // will the minimum reasonable size fit in the available space ?
  if (tmpSize.isValid() && tmpSize.width() <= m_centralRegionWidth && tmpSize.height() <= m_centralRegionHeight) {
    // yes - try and make it the full size

    int wid = m_centralRegionWidth;
    int hei = m_centralRegionHeight;

    // check for stronger upper limits
    tmpSize = m_activeEditor->maximumSize();
    if (tmpSize.isValid()) {
      if (wid > tmpSize.width())
        wid = tmpSize.width();
      if (hei > tmpSize.height())
        hei = tmpSize.height();
    }

    m_activeEditor->move((m_centralRegionWidth - wid)/ 2, m_titleRegionHeight);
    m_activeEditor->resize(wid, hei);
  }
  else {
    // wont fit ... but resize to the minimum
    m_activeEditor->resize(tmpSize);
  }
}

void CWActiveContext::discardCurrentEditor(void)
{
  // guaranteed that m_activeEditor is not null

  // find this editor in the list in order to get its index
  int index = 0;
  QList<CWEditor*>::iterator it = m_editorList.begin();
  while (it != m_editorList.end() && *it != m_activeEditor) {
    ++it;
    ++index;
  }
  assert(it != m_editorList.end());
  m_editorList.erase(it);

  disconnect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));
  disconnect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
  m_activeEditor->hide();

  // if this was the last editor then hide the active tab
  if (m_editorList.empty()) {
    m_centralRegionWidth = width();
    m_activeTab->hide();
  }

  CWEditor *oldActive = m_activeEditor;

  m_blockActiveTabSlot = true;
  m_activeTab->removeTab(index + 1); // slot will return without doing anything
  m_blockActiveTabSlot = false;
  slotCurrentActiveTabChanged(m_activeTab->count()-1); // make the last tab active

  // Delete, but not immediately, since the signalShortcutActionOk is sent from a method
  // of m_activeEditor.
  oldActive->deleteLater();
}

void CWActiveContext::slotOkButtonClicked()
{
  if (m_activeEditor) {
    // only discard if the action was a success - the editor MUST provide its own
    // feedback to the user. This just prevents a silent no-op.
    if (m_activeEditor->actionOk())
     {
      discardCurrentEditor();
     }
  }
}

void CWActiveContext::slotCancelButtonClicked()
{
  if (m_activeEditor) {
    m_activeEditor->actionCancel();
    discardCurrentEditor();
  }
}

void CWActiveContext::slotHelpButtonClicked()
{
  if (m_activeEditor)
    m_activeEditor->actionHelp();
}

void CWActiveContext::slotAcceptOk(bool canDoOk)
{
  m_okButton->setEnabled(canDoOk);
}

void CWActiveContext::slotPlotPages(const QList<shared_ptr<const CPlotPageData> > &pageList)
{
  int pageNumber;

  int activePageNumber = m_plotRegion->pageDisplayed();
  int activeTabIndex = 0;

  // adjust the number of tabs
  int nPages = pageList.count();
  int index = m_graphTab->count();
  while (index > nPages)
    m_graphTab->removeTab(--index);

  while (pageList.count() > index)
    index = m_graphTab->addTab(QString()) + 1;

  // build a list of pages that are to be retained (those in pageList that are 'empty')
  // and reset the tabs...
  index = 0;
  QList<int> retainedList;
  QList<shared_ptr<const CPlotPageData> >::const_iterator it = pageList.begin();

  while (it !=  pageList.end()) {
    pageNumber = (*it)->pageNumber();

    if ((*it)->isEmpty()) {
      // if must already exist for this to be meaningful ...
      QString tmpTag;
      if (m_plotRegion->pageExists(pageNumber, tmpTag)) {
    retainedList.push_back(pageNumber);
      }
      m_graphTab->setTabText(index, tmpTag);
    }
    else {
      // Set the tab label and store the page number as TabData
      m_graphTab->setTabText(index, QString::fromStdString((*it)->tag()));
    }
    m_graphTab->setTabData(index, QVariant(pageNumber));

    // try and reselect the same active page as before ...
    if (pageNumber == activePageNumber)
      activeTabIndex = index;

    ++index;
    ++it;
  }

  m_plotRegion->removePagesExcept(retainedList);

  // add the additional pages (non empty)
  it = pageList.begin();
  while (it != pageList.end()) {
    pageNumber = (*it)->pageNumber();
    if (!(*it)->isEmpty()) {
      m_plotRegion->addPage(*it);
    }
    ++it;
  }

  if (m_graphTab->count()) {

    if (!m_activeEditor)
      m_graphTab->show();
    if (m_graphTab->currentIndex() == activeTabIndex) {
      slotCurrentGraphTabChanged(activeTabIndex);
    }
    else {
      m_graphTab->setCurrentIndex(activeTabIndex);
    }
  }
  else
    m_graphTab->hide();
}

void CWActiveContext::slotCurrentGraphTabChanged(int index)
{
  int pageNumber = (index == -1) ? -1 : m_graphTab->tabData(index).toInt();

  m_plotRegion->displayPage(pageNumber);

  // set the graph title
  m_graphTitleStr = m_plotRegion->pageTitle(pageNumber);
  if (!m_activeEditor)
    m_title->setText(m_graphTitleStr);

  emit signalActivePageChanged(pageNumber);
}

void CWActiveContext::slotCurrentActiveTabChanged(int index)
{
  if (m_blockActiveTabSlot) return;

  if (index == 0) {
    // changed to graph page ...
    if (m_activeEditor) {
      disconnect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));
      disconnect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
      m_activeEditor->hide();
      m_activeEditor = NULL;

      m_helpButton->hide();
      m_okButton->hide();
      m_cancelButton->hide();
    }

    m_centralRegionHeight= height() - m_titleRegionHeight - m_graphTabRegionHeight;

    setMinimumSize(m_minGeneralSize);
    moveAndResizeGraph(height());

    m_plotRegion->show();
    m_graphTab->show();

    // change title colour scheme
    QPalette palette(m_title->palette());
    palette.setColor(QPalette::Window, QColor(cGraphTitleBackgroundColour));
    palette.setColor(QPalette::WindowText, QColor(cGraphTitleTextColour));
    m_title->setPalette(palette);

    m_title->setText(m_graphTitleStr);

  }
  else {
    // changed to an editor ...
    if (m_activeEditor) {
      // Already have a visible set of buttons and the active tab is visible.
      // Disconnect its signalAcceptOk & signalShortcutActionOk
      // signals and make it invisible.
      disconnect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));
      disconnect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
      m_activeEditor->hide();
      m_activeEditor = NULL;
    }
    else {
      // graph was visible - hide it and make the buttons visible

      m_centralRegionHeight= height() - m_titleRegionHeight - m_buttonRegionHeight;
      moveAndResizeButtons(height());

      m_plotRegion->hide();
      m_graphTab->hide();

      m_helpButton->show();
      m_okButton->show();
      m_cancelButton->show();

      m_activeTab->show();
    }

    // change title colour scheme
    QPalette palette(m_title->palette());
    palette.setColor(QPalette::Window, QColor(cEditTitleBackgroundColour));
    palette.setColor(QPalette::WindowText, QColor(cEditTitleTextColour));
    m_title->setPalette(palette);

    m_activeEditor = m_editorList.at(index - 1);

    moveAndResizeActiveEditor();

    connect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));
    connect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
    m_okButton->setEnabled(m_activeEditor->isAcceptActionOk());

    m_title->setText(m_activeEditor->editCaption());

    m_activeEditor->show();

    // give it focus
    m_activeEditor->takeFocus();
  }
}

void CWActiveContext::slotEditPlotProperties()
{
  addEditor(new CWPlotPropertiesEditor(m_plotRegion));
}

void CWActiveContext::slotPrintPlots()
{
  m_plotRegion->printVisiblePage();
}

void CWActiveContext::slotExportPlots()
{
  m_plotRegion->exportVisiblePage();
}
