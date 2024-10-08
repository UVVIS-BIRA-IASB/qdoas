/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWEDITOR_H_GUARD
#define _CWEDITOR_H_GUARD

#include <QFrame>
#include <QString>
#include <QTextStream>

class CWEditor : public QFrame
{
Q_OBJECT
 public:
  CWEditor(QWidget *parent = 0) : QFrame(parent), m_captionStr("No Title"), m_lastNotification(false) {
    QTextStream(&m_contextTag) << this;
  };

  const QString& editCaption(void) const { return m_captionStr; };
  const QString& editContextTag(void) const {return m_contextTag; };

  virtual void actionCancel(void) {};
  virtual bool actionOk(void) =0;
  virtual void actionHelp(void) =0;

  virtual void takeFocus(void) {};

  bool isAcceptActionOk(void) const;

 protected:
  void notifyAcceptActionOk(bool canDoOk) {
    if (canDoOk != m_lastNotification) {
      m_lastNotification = canDoOk;
      emit signalAcceptOk(m_lastNotification);
    }
  };
  void shortcutActionOk(void) {
    // allows the editor to effectively click the Ok button programatically.
    if (m_lastNotification)
      emit signalShortcutActionOk();
  };

 protected:
  QString m_captionStr, m_contextTag;

 private:
  bool m_lastNotification;

 signals:
  void signalAcceptOk(bool canDoOk);
  void signalShortcutActionOk();

};

inline bool CWEditor::isAcceptActionOk(void) const { return m_lastNotification; }

#endif
