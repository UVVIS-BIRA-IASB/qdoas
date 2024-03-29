/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>

#include <stdlib.h>

#include "CHelpSystem.h"
#include "CPreferences.h"

// Ask the user to locate the Qdoas help files, and update the preferences if found.
QString CHelpSystem::changeDir(void)
{
  QString helpDir = CPreferences::instance()->directoryName("Help", ".");

  QMessageBox::information(NULL, "Help", "Qdoas can not find the Help directory."
                           "Please select the directory containing the main Qdoas help file"
                           "(index.html).");

  helpDir = QFileDialog::getExistingDirectory(NULL, "Qdoas Help directory", helpDir);

  if (!helpDir.isEmpty() && QFileInfo(helpDir + "/index.html").isFile()) {
    CPreferences::instance()->setDirectoryName("Help", helpDir);
  } else {
    QMessageBox::warning(NULL, "Help", "Could not find Help files.");
  }
  return helpDir;
}

void CHelpSystem::showHelpTopic(const QString &chapter, const QString &key)
{
  QString relPath = "/" + chapter + "/" + key + ".html";

  QString helpFile;
  const char *helpDir = getenv("QDOAS_HELP_DIR");
  if (helpDir != NULL) {
    // check for "QDOAS_HELP_DIR" environment variable, which is set if we are
    // running the linux package.
    helpFile = QDir(helpDir).absolutePath() + relPath;
  } else {
    // If we have the right path in the preferences use that
    helpFile = CPreferences::instance()->directoryName("Help", ".") + helpFile;
  }

#if defined(QDOAS_HELP_PATH)
  // If QDOAS_HELP_PATH was defined at compile time, try that:
  if (!QFileInfo(helpFile).isFile()) {
    if (QString(QDOAS_HELP_PATH).startsWith('.')) {
      // prefix the application path when QDOAS_HELP_PATH is relative
      helpFile = QCoreApplication::applicationDirPath() + "/" + QString(QDOAS_HELP_PATH) + relPath;
    } else {
      helpFile = QString(QDOAS_HELP_PATH) + relPath;
    }
  }
#endif

  // If nothing was found, ask the user to point to the Help directory
  if (!QFileInfo(helpFile).isFile()) {
    helpFile = changeDir() + relPath;
  }

  if (QFileInfo(helpFile).isFile()) {
    QDesktopServices::openUrl(QUrl("file:///" + helpFile)); // file: URI with three slashes for windows!
  }
}
