/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


// The Qdoas version string is maintained in CWAboutBox.cpp

#include <QApplication>

#include "CWMain.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  CWMain main;

  main.show();

  return app.exec();
}

