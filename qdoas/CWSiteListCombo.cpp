/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include "CWSiteListCombo.h"

CWSiteListCombo::CWSiteListCombo(QWidget *parent) :
  QComboBox(parent),
  CSitesObserver()
{
  repopulate();
}

void CWSiteListCombo::updateNewSite(const std::string &newSiteName)
{
  QString selected = currentText();

  repopulate();

  int index = findText(selected);
  if (index != -1)
    setCurrentIndex(index);
}

void CWSiteListCombo::updateDeleteSite(const std::string &siteName)
{
  updateNewSite(siteName);
}

void CWSiteListCombo::repopulate()
{
  clear();

  addItem("No Site Specified");

  // populate from the workspace
  int nSites;
  mediate_site_t *siteList = CWorkSpace::instance()->siteList(nSites);

  if (siteList != NULL) {
    for (int i=0; i<nSites; ++i) {
      addItem(QString(siteList[i].name));
    }
    delete [] siteList;
  }
}
