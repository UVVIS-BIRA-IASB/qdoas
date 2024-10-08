/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QTextStream>

#include "CPathMgr.h"
#include "CRingConfigWriter.h"
#include "ConfigWriterUtils.h"

#include "constants.h"

#include "debugutil.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

CRingConfigWriter::CRingConfigWriter(const mediate_ring_t *properties) :
  m_properties(properties)
{
}

CRingConfigWriter::~CRingConfigWriter()
{
}

QString CRingConfigWriter::write(const QString &fileName)
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

  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ring>\n");

  writePaths(fp);

  const mediate_ring_t *d = m_properties; // convenience

  fprintf(fp, "  <general temp=\"%.1f\" normalize=\"%s\" rmhdr=\"%s\" save_raman=\"%s\"", d->temperature,(d->normalize ? sTrue : sFalse),
      (d->noheader ? sTrue : sFalse),(d->saveraman ? sTrue : sFalse));
  
  fprintf(fp," output_format=\"%s\"",(d->formatType==CONVOLUTION_FORMAT_NETCDF)?"netcdf":"ascii");
  fprintf(fp, " pixels=\"%d\"",d->n_groundpixel);

  tmpStr = pathMgr->simplifyPath(QString(d->outputFile));
  fprintf(fp, " output=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toLocal8Bit().data());

  tmpStr = pathMgr->simplifyPath(QString(d->solarRefFile));
  fprintf(fp, " ref=\"%s\"", tmpStr.toLocal8Bit().data());

  fprintf(fp, " >\n");

  writeSlitFunction(fp, 4, &(m_properties->slit));

  fprintf(fp, "  </general>\n</ring>\n");

  if (fclose(fp)) {
    QTextStream stream(&msg);

    stream << "Error writing to the project file '" << fileName << "'";
  }

  return msg;
}

