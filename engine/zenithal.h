#ifndef ZENITHAL_H
#define ZENITHAL_H

#include "comdefs.h"

extern double RadToDeg;
extern double DegToRad;

double  ZEN_NbSec     ( struct date *today, struct time *now, int flag);
double  ZEN_FNTdiz    ( double NbreJours, double *ObsLong, double *ObsLat,double *pAzimuth );
double  ZEN_FNCrtjul  ( double *NbreSec );
double  ZEN_FNCaldti  ( const double *Tm );
char   *ZEN_Tm2Str    (double *Tm,char *str);
char   *ZEN_FNCaljti  ( double *Tm, char *str );
int     ZEN_FNCaljda  ( const double *Tm );
int     ZEN_FNCaljday ( int Year, int Julian );
int     ZEN_FNCaljye  ( double *Tm );
int     ZEN_FNCaljmon ( int Year, int Julian );

#endif
