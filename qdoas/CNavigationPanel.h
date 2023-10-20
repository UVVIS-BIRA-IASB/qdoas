/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CNAVIGATIONPANEL_H_GUARD
#define _CNAVIGATIONPANEL_H_GUARD

#include <QObject>
#include <QAction>
#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>
#include <QTimer>
#include <QColor>

// provide a set of tool-bar actions for navigation

class CNavigationPanel : public QObject
{
Q_OBJECT
 public:
  enum NavButton { eFirst, ePrevious, eNext, eLast };

  CNavigationPanel(QToolBar *toolBar);
  virtual ~CNavigationPanel();

 private:
  QWidget* helperBuildRecordEdit(void);
  QWidget* helperBuildDelayEdit(void);
  void changeBackground(QWidget *widget, const QColor &c);

 public slots:
  void slotSetFileList(const QStringList &fileList);
  void slotSetCurrentFile(int fileIndex, int nRecords);
  void slotSetCurrentRecord(int record, int firstMiddleLast);
  void slotSetEnabled(bool enabled);

 private slots:
  void slotFirstClicked();
  void slotPreviousClicked();
  void slotNextClicked();
  void slotLastClicked();
  void slotStopClicked();
  void slotRecordEditChanged();
  void slotRecordTextEdited(const QString &text);
  void slotDelayEditChanged();
  void slotDelayTextEdited(const QString &text);
  void slotFileSelected(int);
  void slotPlayPauseClicked();
  void slotTimeout();

 signals:
  void signalFirstClicked();
  void signalPreviousClicked();
  void signalNextClicked();
  void signalLastClicked();
  void signalStopClicked();
  void signalStep();
  void signalRecordChanged(int);
  void signalSelectedFileChanged(int);
  void signalPlayStatusChanged(bool);

 private:
  QAction *m_firstBtn, *m_prevBtn, *m_nextBtn, *m_lastBtn, *m_stopBtn, *m_playBtn;
  QLineEdit *m_recordEdit, *m_delayEdit;
  QComboBox *m_fileCombo;

  int m_maxRecord, m_currentRecord;
  bool m_playing;
  bool m_recordTextTouched, m_delayTextTouched;
  QTimer *m_playTimer;
  QIcon m_playIcon, m_pauseIcon;
};

#endif
