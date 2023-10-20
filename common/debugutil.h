/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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
