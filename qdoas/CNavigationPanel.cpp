/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QFile>
#include <QTextStream>
#include <QFontMetrics>
#include <QIntValidator>

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>

#include "CNavigationPanel.h"
#include "CValidator.h"

#include "debugutil.h"

// For reasons unbeknown to me, QWidget::setPalete does not
// work on widgets added to a toolbar. As a workaround, these
// widgets are made a child of a frameless frame, and the frame
// is inserted into the toolbar. I hope this is just a Qt bug...
// Note, the delay edit would be placed in a frame anyway, so that
// the timer icon can be placed next to it.

QWidget* CNavigationPanel::helperBuildRecordEdit(void)
{
  m_recordEdit = new QLineEdit;

  QFontMetrics fm(m_recordEdit->font());

  m_recordEdit->setFixedWidth(fm.horizontalAdvance("00000000000"));
  m_recordEdit->setToolTip("Goto record");

  QFrame *frame = new QFrame;
  frame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setSpacing(0);
  layout->addWidget(m_recordEdit);

  return frame;
}

QWidget* CNavigationPanel::helperBuildDelayEdit(void)
{
  m_delayEdit = new QLineEdit;

  QFontMetrics fm(m_delayEdit->font());

  m_delayEdit->setFixedWidth(fm.horizontalAdvance("0000000")); // XX.XXX seconds [0,60]
  m_delayEdit->setToolTip("Play time delay (s)");
  m_delayEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 60.0, 3, m_delayEdit));
  m_delayEdit->setText("0.000");

  QLabel *delayIcon = new QLabel;
  delayIcon->setPixmap(QPixmap(":/icons/nav_timer_16.png"));

  // a compound widget for the delay interval ...
  QFrame *frame = new QFrame;
  frame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setSpacing(0);

  layout->addSpacing(10);
  layout->addWidget(delayIcon);
  layout->addWidget(m_delayEdit);
  layout->addSpacing(10);

  return frame;
}

CNavigationPanel::CNavigationPanel(QToolBar *toolBar) :
  QObject(toolBar),
  m_maxRecord(0),
  m_currentRecord(0),
  m_playing(false),
  m_recordTextTouched(false),
  m_delayTextTouched(false),
  m_playIcon(":/icons/nav_play_22.png"),
  m_pauseIcon(":/icons/nav_pause_22.png")
{

  // Add the actions to the tool-bar
  m_firstBtn = toolBar->addAction(QIcon(":/icons/nav_first_22.png"), "First record");
  m_prevBtn = toolBar->addAction(QIcon(":/icons/nav_prev_22.png"), "Previous record");
  toolBar->addWidget(helperBuildRecordEdit());
  m_nextBtn = toolBar->addAction(QIcon(":/icons/nav_next_22.png"), "Next record");
  m_lastBtn = toolBar->addAction(QIcon(":/icons/nav_last_22.png"), "Last record");
  m_stopBtn = toolBar->addAction(QIcon(":/icons/nav_stop_22.png"), "End session");
  m_playBtn = toolBar->addAction(m_playIcon, "Play/Pause");
  toolBar->addWidget(helperBuildDelayEdit()); // always enabled ...

  m_fileCombo = new QComboBox;
  m_fileCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  toolBar->addWidget(m_fileCombo);

  // initially disabled
  m_firstBtn->setEnabled(false);
  m_prevBtn->setEnabled(false);
  m_recordEdit->setEnabled(false);
  m_nextBtn->setEnabled(false);
  m_lastBtn->setEnabled(false);
  m_stopBtn->setEnabled(false);
  m_playBtn->setEnabled(false);
  m_fileCombo->setEnabled(false);

  // play timer
  m_playTimer = new QTimer(this);
  m_playTimer->setInterval(0);
  m_playTimer->setSingleShot(true);

  // connections
  connect(m_firstBtn, SIGNAL(triggered()), this, SLOT(slotFirstClicked()));
  connect(m_prevBtn, SIGNAL(triggered()), this, SLOT(slotPreviousClicked()));
  connect(m_nextBtn, SIGNAL(triggered()), this, SLOT(slotNextClicked()));
  connect(m_lastBtn, SIGNAL(triggered()), this, SLOT(slotLastClicked()));
  connect(m_stopBtn, SIGNAL(triggered()), this, SLOT(slotStopClicked()));
  connect(m_playBtn, SIGNAL(triggered()), this, SLOT(slotPlayPauseClicked()));
  connect(m_recordEdit, SIGNAL(returnPressed()), this, SLOT(slotRecordEditChanged()));
  connect(m_recordEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotRecordTextEdited(const QString&)));
  connect(m_delayEdit, SIGNAL(returnPressed()), this, SLOT(slotDelayEditChanged()));
  connect(m_delayEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotDelayTextEdited(const QString&)));

  connect(m_fileCombo, SIGNAL(activated(int)), this, SLOT(slotFileSelected(int)));

  connect(m_playTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

CNavigationPanel::~CNavigationPanel()
{
}

void CNavigationPanel::slotSetFileList(const QStringList &fileList)
{
  // set the list of files in the combobox
  m_fileCombo->clear();
  m_fileCombo->addItems(fileList);
}

void CNavigationPanel::slotSetCurrentFile(int fileIndex, int nRecords)
{
  m_fileCombo->setCurrentIndex(fileIndex);

  m_maxRecord = nRecords;

  // change line edit validator ...
  m_recordEdit->setValidator(new QIntValidator(1, m_maxRecord, m_recordEdit));

  // current record implicitly reset to 0
  m_currentRecord = 0;

  m_recordEdit->setText(QString());
}

// enable and disable access to all features of the nagication panel. Effectively
// the on-off switch.

void CNavigationPanel::slotSetEnabled(bool enable)
{
  // enable/disable state of buttons

  if (enable) {
    // switch on (or stay on) - influenced by 'playing' state

    bool generalActive = (m_maxRecord != 0 && m_fileCombo->count() != 0);
    bool groupBackwardActive = generalActive && (m_currentRecord != 0) && !m_playing;
    bool groupForwardActive = generalActive && (m_currentRecord != m_maxRecord) && !m_playing;

    // groupBackward : down + first
    m_firstBtn->setEnabled(groupBackwardActive);
    m_prevBtn->setEnabled(groupBackwardActive);

    // groupForward : up + last
    m_nextBtn->setEnabled(groupForwardActive);
    m_lastBtn->setEnabled(groupForwardActive);

    m_recordEdit->setEnabled(generalActive && !m_playing);
    m_fileCombo->setEnabled(generalActive && !m_playing);
    m_playBtn->setEnabled(generalActive);
    m_stopBtn->setEnabled(generalActive);
  }
  else {
    // switch off
    if (m_playing) {
      // stop playing and reset the state
      m_playing = false;
      m_playTimer->stop();
      m_playBtn->setIcon(m_playIcon);

      emit signalPlayStatusChanged(m_playing);
    }

    m_firstBtn->setEnabled(false);
    m_prevBtn->setEnabled(false);
    m_nextBtn->setEnabled(false);
    m_lastBtn->setEnabled(false);

    m_playBtn->setEnabled(false);
    m_stopBtn->setEnabled(false);

    m_recordEdit->setEnabled(false);
    m_fileCombo->setEnabled(false);
  }
}

void CNavigationPanel::slotSetCurrentRecord(int record, int firstMiddleLast)
{
  bool groupActive;

  m_currentRecord = record;

  if (m_stopBtn->isEnabled()) {
    // tests that the navigation panel is enabled ... OK to tweak it ...

    // set the display of the record
    QString tmpStr;
    if (m_currentRecord != 0)
      tmpStr.setNum(m_currentRecord);
    m_recordEdit->setText(tmpStr);

    if (m_playing) {
      m_playTimer->start();
    }
    else {
      // use the firstMiddleLast flag to control the status of the buttons

      // down + first (firstMiddleLast == -1 when at the start)
      groupActive = (m_currentRecord > 1) && (firstMiddleLast >= 0);
      m_firstBtn->setEnabled(groupActive);
      m_prevBtn->setEnabled(groupActive);

      // up + last (firstMiddleLast == +1 when at the end)
      groupActive = (m_currentRecord < m_maxRecord) && (firstMiddleLast <= 0);
      m_nextBtn->setEnabled(groupActive);
      m_lastBtn->setEnabled(groupActive);
    }
  }
}

// provides a facade for the toolbutton triggered signals
void CNavigationPanel::slotFirstClicked()
{
  emit signalFirstClicked();
}

void CNavigationPanel::slotPreviousClicked()
{
  emit signalPreviousClicked();
}

void CNavigationPanel::slotNextClicked()
{
  emit signalNextClicked();
}

void CNavigationPanel::slotLastClicked()
{
  emit signalLastClicked();
}

void CNavigationPanel::slotStopClicked()
{
  emit signalStopClicked();
}

void CNavigationPanel::slotPlayPauseClicked()
{
  if (m_playing) {
    // pause ...
    m_playTimer->stop();
    m_playing = false;
    m_playBtn->setIcon(m_playIcon);

    emit signalPlayStatusChanged(m_playing);

    slotSetEnabled(true);
  }
  else {
    // play
    m_playing = true;
    m_playBtn->setIcon(m_pauseIcon);

    // disable the manual control ...
    m_firstBtn->setEnabled(false);
    m_prevBtn->setEnabled(false);
    m_nextBtn->setEnabled(false);
    m_lastBtn->setEnabled(false);

    m_recordEdit->setEnabled(false);
    m_fileCombo->setEnabled(false);

    emit signalPlayStatusChanged(m_playing);

    // call the timeout slot - this will (indirectly) keep firing ...
    slotTimeout();
  }
}

void CNavigationPanel::slotTimeout()
{
// QFile file("qdoas.dbg");
// if (file.open(QIODevice::Append | QIODevice::Text)!=0)
//  {
//   QTextStream out(&file);
//   out << "   CNavigationPanel::slotTimeout " << m_playing <<"\n";
//   file.close();
//  }

  if (m_playing) {
    emit signalStep();
  }
}

void CNavigationPanel::changeBackground(QWidget *widget, const QColor &c)
{
  QPalette palette = widget->palette();
  palette.setColor(widget->backgroundRole(), c); // 0xFFFFE08E
  widget->setPalette(palette);
}

void CNavigationPanel::slotRecordTextEdited(const QString &text)
{
  if (!m_recordTextTouched) {
    changeBackground(m_recordEdit, 0xFFFFE08E);
    m_recordTextTouched = true;
  }
}

void CNavigationPanel::slotRecordEditChanged()
{
  // on a 'return press'
  bool ok;
  int record = m_recordEdit->text().toInt(&ok);

  if (ok && record > 0 && record <= m_maxRecord) {

    changeBackground(m_recordEdit, 0xFFFFFFFF);
    m_recordTextTouched = false;

    if (record != m_currentRecord) {
      emit signalRecordChanged(record);
    }
  }
  else {
    QString tmpStr;
    if (m_currentRecord)
      tmpStr.setNum(m_currentRecord);
    m_recordEdit->setText(tmpStr);
  }
}

void CNavigationPanel::slotDelayTextEdited(const QString &text)
{
  if (!m_delayTextTouched) {
    changeBackground(m_delayEdit, 0xFFFFE08E);
    m_delayTextTouched = true;
  }
}

void CNavigationPanel::slotDelayEditChanged()
{
  // on a 'return press'
  bool ok;
  double tmpDouble = m_delayEdit->text().toDouble(&ok);

  if (ok && tmpDouble >= 0.0 && tmpDouble < 60.0) {

    changeBackground(m_delayEdit, 0xFFFFFFFF);
    m_delayTextTouched = false;

    tmpDouble *= 1000.0; // to miliseconds
    int delay = (int)(tmpDouble + 0.5);
    m_playTimer->setInterval(delay);
  }
}

void CNavigationPanel::slotFileSelected(int index)
{
  emit signalSelectedFileChanged(index);
}
