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


#include <QTextStream>

#include "CPathMgr.h"
#include "CUsampConfigWriter.h"
#include "ConfigWriterUtils.h"

#include "constants.h"

#include "debugutil.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

CUsampConfigWriter::CUsampConfigWriter(const mediate_usamp_t *properties) :
  m_properties(properties)
{
}

CUsampConfigWriter::~CUsampConfigWriter()
{
}

QString CUsampConfigWriter::write(const QString &fileName)
{
  QString msg;
  FILE *fp = fopen(fileName.toLocal8Bit().constData(), "w");

  if (fp == NULL) {
    QTextStream stream(&msg);

    stream << "Failed to open file '" << fileName << "' for writing.";
    return msg;
  }

  // fp is open for writing ...
  QString tmpStr;
  CPathMgr *pathMgr = CPathMgr::instance();

  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<usamp>\n");

  writePaths(fp);

  const mediate_usamp_t *d = m_properties; // convenience

  fprintf(fp, "  <general type=");
  switch (d->methodType) {
  case OPTICAL_DENSITY_FIT:
    fprintf(fp, "\"ODF\"");
    break;
  case INTENSITY_FIT:
    fprintf(fp, "\"ML+SVD\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " shift=\"%g\" rmhdr=\"%s\"", d->shift,
	  (d->noheader ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->outputPhaseOneFile));
  fprintf(fp, " outphase1=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->outputPhaseTwoFile));
  fprintf(fp, " outphase2=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->solarRefFile));
  fprintf(fp, " ref=\"%s\"", tmpStr.toLocal8Bit().data());

  fprintf(fp, " >\n");

  writeSlitFunction(fp, 4, &(m_properties->slit));

  fprintf(fp, "  </general>\n</usamp>\n");

  if (fclose(fp)) {
    QTextStream stream(&msg);

    stream << "Error writing to the project file '" << fileName << "'";
  }

  return msg;
}

