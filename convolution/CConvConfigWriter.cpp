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
#include "CConvConfigWriter.h"
#include "ConfigWriterUtils.h"

#include "constants.h"

#include "debugutil.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

CConvConfigWriter::CConvConfigWriter(const mediate_convolution_t *properties) :
  m_properties(properties)
{
}

CConvConfigWriter::~CConvConfigWriter()
{
}

QString CConvConfigWriter::write(const QString &fileName)
{
  QString msg;
  FILE *fp = fopen(fileName.toLocal8Bit().constData(), "w");

  if (fp == NULL) {
    QTextStream stream(&msg);

    stream << "Failed to open file '" << fileName << "' for writing.";
    return msg;
  }

  // fp is open for writing ...

  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<convolution>\n");

  writePaths(fp);

  writeGeneral(fp, &(m_properties->general));

  fprintf(fp, "  <con_slit>\n");
  writeSlitFunction(fp, 4, &(m_properties->conslit));
  fprintf(fp, "  </con_slit>\n  <dec_slit>\n");
  writeSlitFunction(fp, 4, &(m_properties->decslit));
  fprintf(fp, "  </dec_slit>\n");
  writeFilter(fp, 2, "low", &(m_properties->lowpass));
  writeFilter(fp, 2, "high", &(m_properties->highpass));

  fprintf(fp, "</convolution>\n");

  if (fclose(fp)) {
    QTextStream stream(&msg);

    stream << "Error writing to the project file '" << fileName << "'";
  }

  return msg;
}

void CConvConfigWriter::writeGeneral(FILE *fp, const mediate_conv_general_t *d)
{
  QString tmpStr;
  CPathMgr *pathMgr = CPathMgr::instance();

  fprintf(fp, "  <general convol=");

  switch (d->convolutionType) {
  case CONVOLUTION_TYPE_STANDARD:
    fprintf(fp, "\"std\""); break;
  case CONVOLUTION_TYPE_I0_CORRECTION:
    fprintf(fp, "\"iocorr\""); break;
  default:
    fprintf(fp, "\"none\""); break;
  }

  fprintf(fp, " conver=");
  switch (d->conversionType) {
  case CONVOLUTION_CONVERSION_AIR2VAC:
    fprintf(fp, "\"air2vac\""); break;
  case CONVOLUTION_CONVERSION_VAC2AIR:
    fprintf(fp, "\"vac2air\""); break;
  default:
    fprintf(fp, "\"none\""); break;
  }

  fprintf(fp, " shift=\"%.3f\" conc=\"%.3g\" rmhdr=\"%s\"",
	  d->shift, d->conc, (d->noheader ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->inputFile));
  fprintf(fp, " input=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->outputFile));
  fprintf(fp, " output=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->solarRefFile));
  fprintf(fp, " ref=\"%s\"", tmpStr.toLocal8Bit().data());

  fprintf(fp, " />\n");
}
