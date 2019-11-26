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

#ifndef _CWEDITOR_H_GUARD
#define _CWEDITOR_H_GUARD

#include <QFrame>
#include <QString>

class CWEditor : public QFrame
{
Q_OBJECT
 public:
  CWEditor(QWidget *parent = 0) : QFrame(parent), m_captionStr("No Title"), m_lastNotification(false) {
    m_contextTag.sprintf("%p", this); // unique by default
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
