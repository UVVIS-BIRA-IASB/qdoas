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

#ifndef _DEBUG_UTIL_H_GUARD
#define _DEBUG_UTIL_H_GUARD

#include <assert.h>

#ifdef DEBUG
#include <iostream>
#define DBGCMD(Y) std::cout << Y << std::endl
#else
#define DBGCMD(Y)
//#define assert(X)
#endif

#if defined(LVL4)
#define TRACE4(X) DBGCMD("(4) " << X)
#define TRACE3(X) DBGCMD("(3) " << X)
#define TRACE2(X) DBGCMD("(2) " << X)
#define TRACE1(X) DBGCMD("(1) " << X)
#define  TRACE(X) DBGCMD("(.) " << X)
#elif defined(LVL3)
#define TRACE4(X)
#define TRACE3(X) DBGCMD("(3) " << X)
#define TRACE2(X) DBGCMD("(2) " << X)
#define TRACE1(X) DBGCMD("(1) " << X)
#define  TRACE(X) DBGCMD("(.) " << X)
#elif defined(LVL2)
#define TRACE4(X)
#define TRACE3(X)
#define TRACE2(X) DBGCMD("(2) " << X)
#define TRACE1(X) DBGCMD("(1) " << X)
#define  TRACE(X) DBGCMD("(.) " << X)
#elif defined(LVL1)
#define TRACE4(X)
#define TRACE3(X)
#define TRACE2(X)
#define TRACE1(X) DBGCMD("(1) " << X)
#define  TRACE(X) DBGCMD("(.) " << X)
#else
#define TRACE4(X)
#define TRACE3(X)
#define TRACE2(X)
#define TRACE1(X)
#define  TRACE(X) DBGCMD("(.) " << X)
#endif

#endif
