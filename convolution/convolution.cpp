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


// The Qdoas version string is maintained in CWAboutBox.cpp

#include <QApplication>

#include "CWMain.h"

int main(int argc, char *argv[])
{
	// ----------------------------------------------------------------------------

	// to avoid that a thousands comma separator (QT 4.7.3)

	   QLocale qlocale=QLocale::system();
	   qlocale.setNumberOptions(QLocale::OmitGroupSeparator);
	   QLocale::setDefault(qlocale);

	// ----------------------------------------------------------------------------
  QApplication app(argc, argv);

  CWMain main;

  main.show();

  return app.exec();
}

