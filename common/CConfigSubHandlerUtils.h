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


#ifndef _CCCONFIGSUBHANDLERUTILS_H_GUARD
#define _CCCONFIGSUBHANDLERUTILS_H_GUARD

#include "CConfigHandler.h"
#include "mediate_general.h"

//-------------------------------------------------------------------

class CFilteringSubHandler : public CConfigSubHandler
{
 public:
  CFilteringSubHandler(CConfigHandler *master,
		       mediate_filter_t *filter);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_filter_t *m_filter;
};

//-------------------------------------------------------------------

class CSlitFunctionSubHandler : public CConfigSubHandler
{
 public:
  CSlitFunctionSubHandler(CConfigHandler *master,
			  mediate_slit_function_t *function);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_slit_function_t *m_function;
};

//-------------------------------------------------------------------

#endif

