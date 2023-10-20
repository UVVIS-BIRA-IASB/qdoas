/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWSPLITTER_H_GUARD
#define _CWSPLITTER_H_GUARD

#include <map>

#include <QSplitter>
#include <QString>

// Extends QSplitter to allow control of the first splitter position
// via signal-slot mechanism

class CWSplitter : public QSplitter
{
Q_OBJECT
 public:
  CWSplitter(Qt::Orientation orientation, const char *settingsGroupName, QWidget *parent = 0);

  void savePreferences(void);

  public slots:
    void slotSetWidthMode(int mode);

 private:
  std::map<int,int> m_modeToSizeMap;
  int m_currentMode;
  QString m_settingsGroupName;
};

#endif
