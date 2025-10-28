/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CVALIDATOR_H_GUARD 
#define _CVALIDATOR_H_GUARD

#include <QValidator>

class CRecordValidator : public QValidator
{
 public:
  CRecordValidator(QObject *obj);

  void setRange(int bottom, int top);
  virtual QValidator::State validate(QString &input, int&) const;

 private:
  int m_bottom, m_top;
};

// CSzaValidator : Allow '0.000 to 180.000' with 3 decimal precision

class CSzaValidator : public QValidator
{
 public:
  CSzaValidator(QObject *obj);

  virtual QValidator::State validate(QString &input, int&) const;
  virtual void fixup(QString &input) const;
};


// CDoubleValidator - QDoubleValidator is broken (in Qt 4.2.3)

class CDoubleExpFmtValidator : public QValidator
{
 public:
  CDoubleExpFmtValidator(QObject *obj);
  CDoubleExpFmtValidator(double bottom, double top, int sigFigures, QObject *obj);

  virtual QValidator::State validate(QString &input, int&) const;
  virtual void fixup(QString &input) const;

  void setRange(double bottom, double top);
  void setSignificantFigures(int sigFigures);

 private:
  double m_bottom, m_top;
  int m_sigFigures;
};

class CDoubleFixedFmtValidator : public QValidator
{
 public:
  CDoubleFixedFmtValidator(QObject *obj);
  CDoubleFixedFmtValidator(double bottom, double top, int decimals, QObject *obj);

  virtual QValidator::State validate(QString &input, int&) const;
  virtual void fixup(QString &input) const;

  void setRange(double bottom, double top);
  void setDecimals(int decimals);

 private:
  double m_bottom, m_top;
  int m_decimals;
};

#endif
