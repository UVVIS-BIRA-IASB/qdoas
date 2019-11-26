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


#include "CWSiteListCombo.h"

CWSiteListCombo::CWSiteListCombo(QWidget *parent) :
  QComboBox(parent),
  CSitesObserver()
{
  repopulate();
}

void CWSiteListCombo::updateNewSite(const QString &newSiteName)
{
  QString selected = currentText();

  repopulate();

  int index = findText(selected);
  if (index != -1)
    setCurrentIndex(index);
}

void CWSiteListCombo::updateDeleteSite(const QString &siteName)
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
