/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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

