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

#ifndef _CWSITEEDITOR_H_GUARD
#define _CWSITEEDITOR_H_GUARD

#include <QLineEdit>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWSiteEditor : public CWEditor
{
Q_OBJECT
 public:
  CWSiteEditor(QTreeWidgetItem *editItem = 0, QWidget *parent = 0);

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 private:
  double m_long, m_lat, m_alt;
  unsigned int m_validBits;
  QLineEdit *m_siteName, *m_abbreviation;

 public slots:
   void slotLongitudeChanged(const QString &text);
   void slotLatitudeChanged(const QString &text);
   void slotAltitudeChanged(const QString &text);

};

#endif
