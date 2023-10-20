/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CWSITELISTCOMBO_H_GUARD
#define _CWSITELISTCOMBO_H_GUARD

#include <QComboBox>

#include "CWorkSpace.h"

class CWSiteListCombo : public QComboBox, public CSitesObserver
{
 public:
  CWSiteListCombo(QWidget *parent = 0);

  virtual void updateNewSite(const QString &newSiteName);
  virtual void updateDeleteSite(const QString &siteName);

 private:
  void repopulate();

};

#endif
