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


#ifndef _CENGINEERROR_H_GUARD
#define _CENGINEERROR_H_GUARD

#include <QString>

class CEngineError
{
 public:
  CEngineError(const QString &tag, const QString &msg, int errorLevel) : m_tag(tag),
                                                                         m_message(msg),
                                                                         m_errorLevel(errorLevel) {};

  const QString& message(void) const;
  const QString& tag(void) const;
  int errorLevel(void) const;

 private:
  QString m_tag, m_message;
  int m_errorLevel;
};

inline const QString& CEngineError::tag(void) const { return m_tag; }
inline const QString& CEngineError::message(void) const { return m_message; }
inline int CEngineError::errorLevel(void) const { return m_errorLevel; }

#endif
