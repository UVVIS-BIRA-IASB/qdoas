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

#ifndef _CVALIDATOR_H_GUARD
#define _CVALIDATOR_H_GUARD

#include <QValidator>

// CSzaValidator : Allow '0.000 to 180.000' with 3 decimal precision

class CSzaValidator : public QValidator
{
 public:
  CSzaValidator(QObject *obj);

  virtual QValidator::State validate(QString &input, int &pos) const;
  virtual void fixup(QString &input) const;
};


// CDoubleValidator - QDoubleValidator is broken (in Qt 4.2.3)

class CDoubleExpFmtValidator : public QValidator
{
 public:
  CDoubleExpFmtValidator(QObject *obj);
  CDoubleExpFmtValidator(double bottom, double top, int sigFigures, QObject *obj);

  virtual QValidator::State validate(QString &input, int &pos) const;
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

  virtual QValidator::State validate(QString &input, int &pos) const;
  virtual void fixup(QString &input) const;

  void setRange(double bottom, double top);
  void setDecimals(int decimals);

 private:
  double m_bottom, m_top;
  int m_decimals;
};

#endif
