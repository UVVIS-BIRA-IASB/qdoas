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

#include "CValidator.h"

//--------------------------------------------------------------------------------

CSzaValidator::CSzaValidator(QObject *obj) :
  QValidator(obj)
{
}

QValidator::State CSzaValidator::validate(QString &input, int &pos) const
{
  if (input.isEmpty()) return QValidator::Intermediate;

  bool ok;
  double v = input.toDouble(&ok);
  int len = input.length();

  if (ok) {
    if (v < 0.0 || v > 180.0) return QValidator::Invalid;
    if (v < 10.0 && len > 5) return QValidator::Invalid;
    if (v < 100.0 && len > 6)  return QValidator::Invalid;
    return (len > 7) ? QValidator::Invalid : QValidator::Acceptable;
  }

  return QValidator::Invalid;
}

void CSzaValidator::fixup(QString &input) const
{
  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    if (v < 0.0) input = "0.0";
    else if (v > 180.0) input = "180.000";
    else {
      input.clear();
      QTextStream tmpStream(&input);
      tmpStream.setRealNumberNotation(QTextStream::FixedNotation);
      tmpStream.setRealNumberPrecision(3); // 3 decimal places
      tmpStream << v;
    }
  }
  else
    input = "0.0"; // default
}


//--------------------------------------------------------------------------------

CDoubleExpFmtValidator::CDoubleExpFmtValidator(QObject *obj) :
  QValidator(obj)
{
  setRange(-1.0e307, +1.0e307);
  setSignificantFigures(10);
}

CDoubleExpFmtValidator::CDoubleExpFmtValidator(double bottom, double top, int sigFigures, QObject *obj) :
  QValidator(obj)
{
  setRange(bottom, top);
  setSignificantFigures(sigFigures);
}

QValidator::State CDoubleExpFmtValidator::validate(QString &input, int &pos) const
{
  if (input.isEmpty()) return QValidator::Intermediate;

  bool havePoint = false;
  bool haveExp = false;
  int lenMantisa = 0;
  int lenExp = 0;
  int expMag = 0;

  int len = input.length();
  int i = 0;

  while (i < len) {
    if (i == 0) {
      if (input[0] == '.') {
	havePoint = true;
      }
      else if (input[0].isDigit()) {
	++lenMantisa;
      }
      else
	return QValidator::Invalid;
    }
    else {
      if (input[i] == '.') {
	if (havePoint) return QValidator::Invalid;
	havePoint = true;
      }
      else if (input[i] == 'e' || input[i] == 'E') {
	if (haveExp || lenMantisa == 0) return QValidator::Invalid;
	if (lenMantisa > m_sigFigures) return QValidator::Invalid;
	haveExp = true;
	++i;
	if (i < len) {
	  if (input[i].isDigit()) {
	    ++lenExp;
	    expMag = input[i].digitValue();
	  }
	  else if (input[i] != '+' && input[i] != '-')
	    return QValidator::Invalid;
	}
      }
      else if (input[i].isDigit()) {
	if (haveExp) {
	  ++lenExp;
	  if (lenExp > 3) return QValidator::Invalid;
	  expMag *= 10;
	  expMag += input[i].digitValue();
	  if (expMag > 307) return QValidator::Invalid;
	}
	else {
	  ++lenMantisa;
	  if (lenMantisa > m_sigFigures && input[i] != '0') return QValidator::Invalid;
	}
      }
      else
	return QValidator::Invalid;
    }
    ++i;
  }

  // at least potentially ok
  // at least potentially ok
  if (haveExp && lenExp == 0) return QValidator::Intermediate;
  if (lenMantisa == 0) return QValidator::Intermediate;
  
  // should be a real value
  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    if (v >= m_bottom && v <= m_top)
      return QValidator::Acceptable;

    return QValidator::Intermediate;
  }

  return QValidator::Invalid;
}

void CDoubleExpFmtValidator::fixup(QString &input) const
{
  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    if (v < m_bottom)
      input.setNum(m_bottom, 'e', m_sigFigures-1);
    else if (v > m_top)
      input.setNum(m_top, 'e', m_sigFigures-1);
    else
      input.setNum(v, 'e', m_sigFigures-1);
  }
  else
    input.setNum(m_bottom, 'e', m_sigFigures-1); // default to the bottom value
}

void CDoubleExpFmtValidator::setRange(double bottom, double top)
{
  if (bottom < top) {
    m_bottom = bottom;
    m_top = top;
  }
  else {
    m_bottom = top;
    m_top = bottom;
  }
}

void CDoubleExpFmtValidator::setSignificantFigures(int sigFigures)
{
  m_sigFigures = (sigFigures > 1) ? sigFigures : 1;
}


//--------------------------------------------------------------------------------

CDoubleFixedFmtValidator::CDoubleFixedFmtValidator(QObject *obj) :
  QValidator(obj)
{
  setRange(-1.0e307, +1.0e307);
  setDecimals(10);
}

CDoubleFixedFmtValidator::CDoubleFixedFmtValidator(double bottom, double top, int decimals, QObject *obj) :
  QValidator(obj)
{
  setRange(bottom, top);
  setDecimals(decimals);
}

QValidator::State CDoubleFixedFmtValidator::validate(QString &input, int &pos) const
{
  if (input.isEmpty()) return QValidator::Intermediate;

  bool havePoint = false;
  bool haveSign = false;
  int lenDec = 0;

  int len = input.length();
  int i = 0;

  while (i < len) {
    if (i == 0) {
      if (input[0] == '+' && m_top > 0.0) {
	haveSign = true;
      }
      else if (input[0] == '-' && m_bottom < 0.0) {
	haveSign = true;
      }
      else if (input[0] == '.') {
	havePoint = true;
      }
      else if (input[0] == '.') {
	havePoint = true;
      }
      else if (!input[0].isDigit()) {
	return QValidator::Invalid;
      }
    }
    else {
      if (input[i] == '.') {
	if (havePoint) return QValidator::Invalid;
	havePoint = true;
      }
      else if (input[i].isDigit()) {
	if (havePoint) {
	  ++lenDec;
	  if (lenDec > m_decimals) return QValidator::Invalid;
	}
      }
      else
	return QValidator::Invalid;
    }
    ++i;
  }

  // at least potentially ok
  if (len == 1 && (haveSign || havePoint)) return QValidator::Intermediate;
  if (len == 2 && haveSign && havePoint) return QValidator::Intermediate;

  // should be a real value
  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    // three basic ranges ...
    if (m_bottom > 0.0) {
      if (v >= 0.0 && v < m_bottom) return QValidator::Intermediate;
      if (v > m_top) return QValidator::Invalid;
    }
    else if (m_top < 0.0) {
      if (v <= 0.0 && v > m_top) return QValidator::Intermediate;
      if (v < m_bottom) return QValidator::Invalid;
    }
    else {
      if (v < m_bottom || v > m_top) return QValidator::Invalid;
    }
    // must be OK
    return QValidator::Acceptable;
  }

  return QValidator::Invalid;
}

void CDoubleFixedFmtValidator::fixup(QString &input) const
{
  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    if (v < m_bottom)
      input.setNum(m_bottom, 'f', m_decimals);
    else if (v > m_top)
      input.setNum(m_top, 'f', m_decimals);
    else
      input.setNum(v, 'f', m_decimals);
  }
  else {
    // default ... closest to zero
    if (m_bottom > 0.0)
      input.setNum(m_bottom, 'f', m_decimals); // default to the bottom value
    else if (m_top < 0.0)
      input.setNum(m_top, 'f', m_decimals); // default to the bottom value
    else
      input.setNum(0.0, 'f', m_decimals);
  }
}

void CDoubleFixedFmtValidator::setRange(double bottom, double top)
{
  if (bottom < top) {
    m_bottom = bottom;
    m_top = top;
  }
  else {
    m_bottom = top;
    m_top = bottom;
  }
}

void CDoubleFixedFmtValidator::setDecimals(int decimals)
{
  m_decimals = (decimals > 0) ? decimals : 0;
}


