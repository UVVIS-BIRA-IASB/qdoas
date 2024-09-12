/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QCoreApplication>
#include <QIODevice>
#include <QTextStream>
#include <QVariant>

#include "CPreferences.h"

#include "debugutil.h"


#if defined(APP_QDOAS)
#define PREF_KEY "Qdoas"
#elif defined(APP_CONV)
#define PREF_KEY "Conv"
#elif defined(APP_RING)
#define PREF_KEY "Ring"
#elif defined(APP_USAMP)
#define PREF_KEY "Usamp"
#else
#error "An APPLICATION preference key is not defined"
#endif


// initialise static data

CPreferences *CPreferences::m_instance = NULL;

CPreferences* CPreferences::instance(void)
{
  if (m_instance == NULL)
    m_instance = new CPreferences;

  return m_instance;
}

CPreferences::~CPreferences()
{
  // flush data to permanent storage
  m_settings->sync();
  delete m_settings;

  m_instance = NULL;
}

CPreferences::CPreferences()
{
  QCoreApplication::setOrganizationDomain("www.oma.be");

  m_settings = new QSettings("BIRA-IASB", PREF_KEY);
}

// interface for saving/restoring preferences...

QSize CPreferences::windowSize(const QString &key, const QSize &fallback) const
{
  m_settings->beginGroup("WindowSize");
  QSize tmp = m_settings->value(key, fallback).toSize();
  m_settings->endGroup();

  return tmp;
}

void CPreferences::setWindowSize(const QString &key, const QSize &size)
{
  m_settings->beginGroup("WindowSize");
  m_settings->setValue(key, size);
  m_settings->endGroup();
}

QString CPreferences::directoryName(const QString &key, const QString &fallback) const
{
  m_settings->beginGroup("Directory");
  QString tmp = m_settings->value(key, fallback).toString();
  m_settings->endGroup();

  return tmp;
}

void CPreferences::setDirectoryName(const QString &key, const QString &directory)
{
  m_settings->beginGroup("Directory");
  m_settings->setValue(key, directory);
  m_settings->endGroup();
}

void CPreferences::setDirectoryNameGivenFile(const QString &key, const QString &fileName)
{
  // trim the file from the directory
  setDirectoryName(key, CPreferences::dirName(fileName));
}

QString CPreferences::fileExtension(const QString &key, int index, const QString &fallback) const
{
  QString tmp(key);
  QTextStream stream(&tmp);

  stream << index;

  m_settings->beginGroup("FileExtension");
  tmp = m_settings->value(tmp, fallback).toString();
  m_settings->endGroup();

  return tmp;
}

void CPreferences::setFileExtension(const QString &key, int index, const QString &extension)
{
  QString tmp(key);
  QTextStream stream(&tmp);

  stream << index;

  m_settings->beginGroup("FileExtension");
  m_settings->setValue(tmp, extension);
  m_settings->endGroup();
}

void CPreferences::setFileExtensionGivenFile(const QString &key, int index, const QString &fileName)
{
  int pos = fileName.lastIndexOf('.');
  if (pos == -1) {
    // default to all files ...
    setFileExtension(key, index, "*");
  }
  else {
    pos =  fileName.length() - pos - 1;
    if (pos > 0)
      setFileExtension(key, index, fileName.right(pos));
    else
      setFileExtension(key, index, "*");
  }
}

QList<int> CPreferences::columnWidthList(const QString &key, const QList<int> &fallback) const
{
  QList<int> tmp;

  m_settings->beginGroup("ColumnWidth");
  int n = m_settings->beginReadArray(key);

  if (n > 0) {
    int i = 0;
    while (i<n) {
      m_settings->setArrayIndex(i);
      tmp.push_back(m_settings->value("width").toInt());
      ++i;
    }
    while (i < fallback.count()) {
      tmp.push_back(fallback.at(i));
      ++i;
    }
  }
  else {
    tmp = fallback;
  }

  m_settings->endArray();
  m_settings->endGroup();

  return tmp;
}

void CPreferences::setColumnWidthList(const QString &key, const QList<int> &widthList)
{
  m_settings->beginGroup("ColumnWidth");

  m_settings->beginWriteArray(key);
  int i = 0;
  while (i < widthList.count()) {
    m_settings->setArrayIndex(i);
    m_settings->setValue("width", widthList.at(i));
    ++i;
  }

  m_settings->endArray();
  m_settings->endGroup();
}

QPen CPreferences::plotPen(const QString &key, const QPen &fallback) const
{
  QPen result(fallback);

  m_settings->beginGroup("PlotProperties");

  QVariant v = m_settings->value(key);
  if (v.canConvert(QVariant::Pen))
    result = v.value<QPen>();

  m_settings->endGroup();

  return result;
}

void CPreferences::setPlotPen(const QString &key, const QPen &pen)
{
  m_settings->beginGroup("PlotProperties");

  QVariant v = pen;
  m_settings->setValue(key, v);

  m_settings->endGroup();
}

QColor CPreferences::plotColour(const QString &key, const QColor &fallback) const
{
  QColor result(fallback);

  m_settings->beginGroup("PlotProperties");

  QVariant v = m_settings->value(key);
  if (v.canConvert(QVariant::Color))
    result = v.value<QColor>();

  m_settings->endGroup();

  return result;
}

void CPreferences::setPlotColour(const QString &key, const QColor &colour)
{
  m_settings->beginGroup("PlotProperties");

  QVariant v = colour;
  m_settings->setValue(key, v);

  m_settings->endGroup();
}

CScaleControl CPreferences::plotScale(const QString &key, const CScaleControl &fallback) const
{
  CScaleControl retValue;

  m_settings->beginGroup("PlotProperties");

  QVariant v = m_settings->value(key);
  if (v.isValid()) {
    QString str = v.toString();
    QTextStream stream(&str, QIODevice::ReadOnly);
    int tmpFixed;
    double tmpMin, tmpMax;

    stream >> tmpFixed >> tmpMin >> tmpMax;

    retValue = CScaleControl(tmpFixed, tmpMin, tmpMax);
  }

  m_settings->endGroup();

  return retValue;
}

void CPreferences::setPlotScale(const QString &key, const CScaleControl &scaleControl)
{
  m_settings->beginGroup("PlotProperties");

  QString str;
  QTextStream stream(&str);

  stream << (int)(scaleControl.isFixedScale() ? 1 : 0) << " " << scaleControl.minimum() << " " << scaleControl.maximum();

  m_settings->setValue(key, QVariant(str));

  m_settings->endGroup();
}

int CPreferences::plotLayout(const QString &key, int fallback) const
{
  m_settings->beginGroup("PlotProperties");

  QVariant v = m_settings->value(key);
  if (v.isValid())
    fallback = v.toInt();

  m_settings->endGroup();

  return fallback;
}

void CPreferences::setPlotLayout(const QString &key, int layoutValue)
{
  m_settings->beginGroup("PlotProperties");

  QVariant v(layoutValue);
  m_settings->setValue(key, v);

  m_settings->endGroup();
}


QString CPreferences::baseName(const QString &fileName)
{
  QString result = fileName;
  int index = result.lastIndexOf('/');
  if (index == -1) index = result.lastIndexOf('\\');
  if (index != -1)
    result.remove(0,index+1);

  return result;
}

QString CPreferences::dirName(const QString &fileName)
{
  int index = fileName.lastIndexOf('/');
  if (index == -1) index = fileName.lastIndexOf('\\');
  if (index != -1)
    return fileName.left(index);

  // no '/' or '\' characters - dirname is empty.
  return QString();
}

