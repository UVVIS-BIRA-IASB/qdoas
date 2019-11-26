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

#include "CWSplitter.h"
#include "CPreferences.h"

#include "debugutil.h"

CWSplitter::CWSplitter(Qt::Orientation orientation, const char *settingsGroupName, QWidget *parent) :
  QSplitter(orientation, parent),
  m_currentMode(0),
  m_settingsGroupName(settingsGroupName)
{
  // restore the map
  int i, mode, wid;
  std::map<int,int>::iterator it;
  QSettings &settings = CPreferences::instance()->settings();

  settings.beginGroup(m_settingsGroupName);

  int n = settings.beginReadArray("Splitter");
  for (i=0; i<n; ++i) {
    settings.setArrayIndex(i);
    mode = settings.value("mode").toInt();
    wid = settings.value("width").toInt();

    // store in the map
    m_modeToSizeMap.insert(std::map<int,int>::value_type(mode, wid));
  }
  settings.endArray();

  settings.endGroup();
}

void CWSplitter::savePreferences(void)
{
  // store the map as an array of settings
  QSettings &settings = CPreferences::instance()->settings();
  
  settings.beginGroup(m_settingsGroupName);

  settings.beginWriteArray("Splitter");
  std::map<int,int>::const_iterator it = m_modeToSizeMap.begin();
  int i = 0;
  while (it != m_modeToSizeMap.end()) {
    settings.setArrayIndex(i);
    settings.setValue("mode", it->first);
    settings.setValue("width", it->second);
    ++i;
    ++it;
  }
  settings.endArray();
  settings.endGroup();
}

void CWSplitter::slotSetWidthMode(int newMode)
{
  if (count() && newMode != m_currentMode) {
    
    QList<int> tmpSizes(sizes());
    int currentSize = tmpSizes.front(); // current first widget size

    // store the current state
    std::map<int,int>::iterator it = m_modeToSizeMap.find(m_currentMode);
    if (it != m_modeToSizeMap.end())
      it->second = currentSize;
    else
      m_modeToSizeMap.insert(std::map<int,int>::value_type(m_currentMode, currentSize));

    // does the new mode have state to restore ??
    it = m_modeToSizeMap.find(newMode);
    if (it != m_modeToSizeMap.end()) {

      tmpSizes.front() = it->second;  // required size
      tmpSizes.back() += currentSize - it->second; // attempt to conserve the sum

      setSizes(tmpSizes);
    }
      
    // change the mode
    m_currentMode = newMode;
  }
}
