
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ZENITH ANGLES CALCULATIONS
//  Name of module    :  ZENITHAL.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in 97
//  Reference         :  Original HT-BASIC SAOZ program written at CNRS, FRANCE;
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  This module contains routines for calculating zenith angles at a given time
//  and a given observation site and some utility functions on date and time.
//  Except the function ZEN_NbSec, all functions in this module have been extracted
//  from the original HT-BASIC SAOZ program in 1990-1991.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  =======================
//  DATE AND TIME FUNCTIONS
//  =======================
//
//  ZEN_FNCaljye - calculate the year from the number of seconds since 01/01/1970;
//
//  ZEN_FNCaljmon - calculate the month in the year from the calendar day;
//
//  ZEN_FNCaljday - calculate the day in the month from the calendar day;
//
//  ZEN_FNCaljda - calculate the calendar day from the number of seconds since
//                 01/01/1970;
//
//  ZEN_FNCaljti - set a string with the time calculated from a number of seconds
//                 since 01/01/1970;
//
//  ZEN_FNCaldti - calculate the decimal time from the number of seconds since
//                 01/01/1970;
//
//  ZEN_FNCrtjul - calculate the number of daysfrom the number of seconds since
//                 01/01/1970;
//
//  ZenFNCjulrt - calculate the number of seconds from the number of days since
//                01/01/1970 (this function is the reciprocal of the previous one);
//
//  ZEN_NbSec - calculate the number of seconds since 01/01/1970 for a given
//              date and time;
//
//  =======================
//  ZENITH ANGLES FUNCTIONS
//  =======================
//
//  ZenFNAngle - calculate the arc tan of an angle;
//
//  ZenSun - calculate the ecliptical coordinates of the sun;
//
//  ZenCoordEquat - transformation of ecliptical coordinates into equatorial ones;
//
//  ZenFNSideral - calculate the sidereal time at Greenwich;
//
//  ZenFNZenithal - calculate the solar zenith angle;
//
//  ZEN_FNTdiz - calculate the zenith and azimuth angle from a given fractional
//               day and the geolocation coordinates of a given observation site;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <math.h>
#include <time.h>
#include <stdio.h>

#include "zenithal.h"
#include "doas.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define     HTB80   (double) 2.111823360E+11

#define     Lobase      278.9654
#define     Lopas       0.985647342
#define     Lot2        3E-4

#define     Pbase       281.238
#define     Ppas        47067E-9
#define     Pt2         45E-5

#define     Exc         0.01675062
#define     Exct       -418E-7
#define     Exct2      -137E-9
#define     Kep         5

// ================
// STATIC VARIABLES
// ================

double RadToDeg = ( double ) 360. / PI2;
double DegToRad = ( double ) PI2 / 360.;

//  =======================
//  DATE AND TIME FUNCTIONS
//  =======================

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCaljye
// -----------------------------------------------------------------------------
// PURPOSE       calculate the year from the number of seconds since 01/01/1970
//
// INPUT         Tm   the number of seconds since 01/01/1970
//
// RETURN        the calculated year
// -----------------------------------------------------------------------------

int ZEN_FNCaljye(double *Tm)
 {
  // Declarations

  unsigned int Jc, An;   // Jc : the number of days since 01/01/1980
  double fan,fjc;        // An : the number of years since 01/01/1980

  // Calculate the year from the number of seconds since 01/01/1970

  fjc = (double) ( *Tm-HTB80 ) / 86400.;
  Jc  = (unsigned int)fjc;
  fan = (double) Jc / 365.25 + 1.;
  An  = (unsigned int)fan;

  // Return

  return ( (int) An+1979 );
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCaljmon
// -----------------------------------------------------------------------------
// PURPOSE       calculate the month in the year from the calendar day
//
// INPUT         Year    the current year
//               Julian  the calendar day
//
// RETURN        the month in the year
// -----------------------------------------------------------------------------

int ZEN_FNCaljmon(int Year,int Julian)
 {
  // Declarations

  int First[] = {  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 9999 };
  int Month=0;

  // Calculate the month in the year

  if ( (Julian>365+((Year%4)==0)) || (Julian<1) )
   ERROR_SetLast((char *)"ZEN_FNCaljmon",ERROR_TYPE_WARNING,ERROR_ID_JULIAN_DAY,Julian,Year);
  else
   while ( Julian > (First[Month] + (int) (((Year%4)==0) && (Month>1))) ) Month++;

  // Return

  return ((int)Month);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCaljday
// -----------------------------------------------------------------------------
// PURPOSE       calculate the day in the month from the calendar day
//
// INPUT         Year    the current year
//               Julian  the calendar day
//
// RETURN        the day in the month
// -----------------------------------------------------------------------------

int ZEN_FNCaljday(int Year,int Julian)
 {
  // Declarations

  int First[] = {  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 9999 };
  int Month=0;

  // Calculate the day in the month

  if ( (Julian > 365+((Year%4)==0)) || (Julian<1) )
   ERROR_SetLast((char *)"ZEN_FNCaljday",ERROR_TYPE_WARNING,ERROR_ID_JULIAN_DAY,Julian,Year);
  else
   while ( Julian > (First[Month] += (int) (((Year%4)==0) && (Month>1))) ) Month++;

  // Return

  return ((Month>0)?(int)Julian-First[Month-1]:Month);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCaljda
// -----------------------------------------------------------------------------
// PURPOSE       calculate the calendar day from the number of seconds since
//               01/01/1970
//
// INPUT         Tm  the number of seconds since 01/01/1970
//
// RETURN        the calendar day
// -----------------------------------------------------------------------------

int ZEN_FNCaljda(const double *Tm)
 {
  // Declarations

  unsigned int Jc, An, Nj;
  double fan,fjc;

  // Calculate the calendar day

  fjc = (double) ( *Tm-HTB80 ) / 86400.;
  Jc  = (unsigned int)fjc;
  fan = (double) Jc / 365.25;
  An  = (unsigned int) fan;
  Nj  = (unsigned int) 365 * An--;

  // Return

  return ((unsigned int)Jc-Nj-An/4);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCaljti
// -----------------------------------------------------------------------------
// PURPOSE       set a string with the time calculated from a number of seconds
//               since 01/01/1970
//
// INPUT         Tm    the number of seconds since 01/01/1970
//
// OUTPUT        str   the string to set with the time calculated from Tm
//
// RETURN        the pointer to the outptut string str
// -----------------------------------------------------------------------------

char *ZEN_FNCaljti(double *Tm,char *str)
 {
  // Declarations

  double    Jc, NbreSec, NHeures, NMinut;
  static const char *DIGITS = "0123456789";
  double Tm1980;

  // Calculate the time in hours and minutes from the number of seconds since 01/01/1970

  Tm1980  = (double) ( *Tm-HTB80 );

  Jc      = (unsigned int) ( Tm1980 / 86400. );
  NbreSec = (double) Tm1980 - Jc * 86400.;
  NHeures = (int) ( NbreSec / 3600. );
  NbreSec = (double) NbreSec - NHeures * 3600.;
  NMinut  = (int) NbreSec / 60.;

  // Set the output string with the calculated time

  str[0] = DIGITS[(int) NHeures/10];
  str[1] = DIGITS[(int) NHeures%10];
  str[2] = ':';
  str[3] = DIGITS[(int) NMinut/10];
  str[4] = DIGITS[(int) NMinut%10];
  str[5] = '\0';

  // Return

  return ((char *)str);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCaldti
// -----------------------------------------------------------------------------
// PURPOSE       calculate the decimal time from the number of seconds since
//               01/01/1970
//
// INPUT         Tm  the number of seconds since 01/01/1970
//
// RETURN        the decimal time
// -----------------------------------------------------------------------------

double ZEN_FNCaldti(const double *Tm)
 {
  // Declarations

  double Jc, NbreSec, NHeures;
  double Tm1980, TiDeci;

  // Calculate the decimal time

  Tm1980  = (double) ( *Tm-HTB80 );

  Jc      = (unsigned int) ( Tm1980 / 86400. );
  NbreSec = (double) Tm1980 - Jc * 86400.;
  NHeures = (int) ( NbreSec / 3600. );
  NbreSec = (double) NbreSec - NHeures * 3600.;

  TiDeci = NHeures + NbreSec / 3600.;

  // Return

  return ((double)TiDeci);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNCrtjul
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of days from the number of seconds since
//               01/01/1970
//
// INPUT         NbreSec  the number of seconds since 01/01/1970
//
// RETURN        the number of days since 01/01/1970
// -----------------------------------------------------------------------------

double ZEN_FNCrtjul(double *NbreSec)
 {
  return ((double)(*NbreSec-(double)211434710400.)/86400.+31776.);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZenFNCjulrt
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of seconds from the number of days since
//               01/01/1970 (this function is the reciprocal of the previous one)
//
// INPUT         NbreJours  the number of days since 01/01/1970
//
// RETURN        the number of seconds since 01/01/1970
// -----------------------------------------------------------------------------

double ZenFNCjulrt(double *NbreJours)
 {
  return((double)(*NbreJours-31776.)*86400.+(double)211434710400.);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_NbSec
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of seconds since 01/01/1970 for a given
//               date and time
//
// INPUT         flag         0, use the given date and time;
//                            1, use the current date and time;
//
//               pToday,pNow  the pointed structures contain the given date and
//                            time if flag is 0;
//
// OUTPUT        pToday,pNow  the pointed structures are filled with the current
//                            date and time if flag is 1;
//
// RETURN        the number of seconds since 01/01/1970 for the date and time
//               respectively pointed by pToday and pNow.
// -----------------------------------------------------------------------------

double ZEN_NbSec(struct date *pToday,struct time *pNow,int flag)
 {
  // Declarations

  double tm;                         // number of seconds
  int    calendar;                   // calendar date
  int    bissext;                    // number of leap years

  // if flag==1, get the current date and time

  if (flag)
   {
   	// Local declarations

    time_t today;                                                               // current date and time as a time_t number
    char datetime[20];                                                          // current date and time as a string
    int day,mon,year,hour,min,sec;                                              // decomposition of the current date

    // Get the current date and time

    today=time(NULL);

    // Convert into a string

    strftime(datetime,20,"%d/%m/%Y %H:%M:%S",localtime(&today));
    sscanf(datetime,"%2d/%2d/%4d %2d:%2d:%2d",&day,&mon,&year,&hour,&min,&sec);

    // Fill the input structure

    pToday->da_year=year;
    pToday->da_mon=(char)mon;
    pToday->da_day=(char)day;

    pNow->ti_hour=(unsigned char)hour;
    pNow->ti_min=(unsigned char)min;
    pNow->ti_sec=(unsigned char)sec;
   }

  // calculate the calendar date

  switch (pToday->da_mon)
   {
    case  1 : calendar =   0 + pToday->da_day; break;
    case  2 : calendar =  31 + pToday->da_day; break;
    case  3 : calendar =  59 + pToday->da_day; break;
    case  4 : calendar =  90 + pToday->da_day; break;
    case  5 : calendar = 120 + pToday->da_day; break;
    case  6 : calendar = 151 + pToday->da_day; break;
    case  7 : calendar = 181 + pToday->da_day; break;
    case  8 : calendar = 212 + pToday->da_day; break;
    case  9 : calendar = 243 + pToday->da_day; break;
    case 10 : calendar = 273 + pToday->da_day; break;
    case 11 : calendar = 304 + pToday->da_day; break;
    case 12 : calendar = 334 + pToday->da_day; break;
    default : calendar =   0 + pToday->da_day; break;
   }

  // if the current year is a leap one and date is higher than 28 february, take one day more into account

  if ( ( (pToday->da_year%4) == 0 ) && ( calendar > 59 ) && ( pToday->da_mon > 2 ) ) calendar++;

  // calculate the number of leap years between the current year and 1980

  bissext = (int) ((double)(pToday->da_year-1980.+3.) * 0.25);

  // convert the given date and time into a number of seconds since 01/01/1970

  tm = (double)    HTB80 + (pToday->da_year-1980.)*31536000.
                +  86400. * bissext
                + (calendar-1.)*86400. + pNow->ti_hour*3600. + pNow->ti_min*60.
                +  pNow->ti_sec;

  // return

  return ((double) tm);
 }

// =======================
// ZENITH ANGLES FUNCTIONS
// =======================

// -----------------------------------------------------------------------------
// FUNCTION      ZenFNAngle
// -----------------------------------------------------------------------------
// PURPOSE       calculate the arc tan of an angle
//
// INPUT         Coang             the cosinus of the angle
//               Siang             the sinus of the angle
//
// RETURN        the arc tan of an angle for which cos and sin are resp. given
//               by Coang and Siang
// -----------------------------------------------------------------------------

double ZenFNAngle ( double *Coang, double *Siang )
 {
  // Declaration

  double Angle;

  if (fabs(*Coang)<(double)1.e-6)
   Angle=(*Siang<0.)?270.:90.;
  else
   {
    // Calculate arc tan

    Angle=(double)atan2(*Siang,*Coang);

    if (Angle<0)
     Angle+=(double)PI2;

    Angle*=(double)RadToDeg;
   }

  // Return

  return ( Angle );
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZenSun
// -----------------------------------------------------------------------------
// PURPOSE       calculate the ecliptical coordinates of the sun
//
// INPUT         Nbj               the fractional calendar day
//
// OUTPUT        pLongit,pLatit    ecliptical coordinates
// -----------------------------------------------------------------------------

void ZenSun ( double Nbj, double *pLongit, double *pLatit )
 {
  // Declarations

  double century, century2;
  double P, Ltempo, Excen, M, U, V, Arg;
  int K;

  // Initializations

  century  =  Nbj / 36525.;
  century2 = century * century;

  P      = fmod ( (double) Pbase  + Ppas*Nbj  + Pt2*century2, 360. );
  Ltempo = fmod ( (double) Lobase + Lopas*Nbj + Lot2*century2, 360. );
  Excen  = (double) Exc + Exct*century + Exct2*century2;

  // Solve Kepler's equation

  U = M = (double) ( Ltempo - P ) * DegToRad;

  for ( K = 0; K <= Kep; U = (double) M + Excen * sin ( U ), K++ );

  Arg = tan ( U*0.5 ) * sqrt ( (double) (1+Excen)/(1-Excen) );

  V = (double) 2.*RadToDeg*atan(Arg);

  if ( V < 0. ) V += 360.;

  // Ecliptical coordinates

  *pLongit = (double) fmod ( (double) V + P, 360. );
  *pLatit  = (double) 0.;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZenCoordEquat
// -----------------------------------------------------------------------------
// PURPOSE       transformation of ecliptical coordinates into equatorial ones
//
// INPUT         pLongit,pLatit    ecliptical coordinates
//
// OUTPUT        pDecli,pAscen     equatorial coordinates
// -----------------------------------------------------------------------------

void ZenCoordEquat(double *pLongit,double *pLatit,double *pDecli,double *pAscen)
 {
  // Declarations

  double RadIncli, CosIncl, SinIncl,
         RadLongit, RadLatit, CosLong, CosLat, SinLong, SinLat,
         SinDecl, CosAsc, SinAsc;

  // Conversion of angles from degrees into radians

  RadIncli  = (double)  23.45  * DegToRad;                                      /* Inclin. Eclipt % Equat.  */
  RadLatit  = (double) *pLatit  * DegToRad;                                     /* Latitude ‚cliptique      */
  RadLongit = (double) *pLongit * DegToRad;                                     /* Longitude ‚cliptique     */

  // Trigonometric calculations

  CosIncl = (double) cos ( RadIncli  );
  SinIncl = (double) sin ( RadIncli  );
  CosLat  = (double) cos ( RadLatit  );
  SinLat  = (double) sin ( RadLatit  );
  CosLong = (double) cos ( RadLongit );
  SinLong = (double) sin ( RadLongit );

  // Declination

  SinDecl = (double) CosIncl*SinLat + SinIncl*CosLat* SinLong;

  if (fabs(SinDecl)>(double)1.)
   *pDecli=(SinDecl<(double)0.)?(double)-90.:(double)90.;
  else
   *pDecli = (double) fmod(asin(SinDecl)*RadToDeg,360.);

  if (*pDecli<0)
   *pDecli+=360.;

  // Right ascension

  SinAsc = (double) CosIncl*CosLat*SinLong - SinIncl*SinLat;
  CosAsc = (double) CosLat * CosLong;
  *pAscen = (double) ZenFNAngle ( &CosAsc, &SinAsc );
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZenFNSideral
// -----------------------------------------------------------------------------
// PURPOSE       calculate the sidereal time at Greenwich
//
// INPUT         N          the fractional calendar day
//               Hs         the real part of N
//
// RETURN        the sidereal time at Greenwich
// -----------------------------------------------------------------------------

double ZenFNSideral(double N,double Hs)
 {
  double  Sideral = (double) 98.965 + N*0.985647342 + Hs*360.;
  return ( (double) fmod ( Sideral, 360. ) ) ;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZenFNZenithal
// -----------------------------------------------------------------------------
// PURPOSE       calculate the solar zenith angle
//
// INPUT         pHourAngle  the local hour angle;
//               pDecli      declination of the celestial equator;
//               pLatit      the latitude of the observation site;
//
// RETURN        the zenith angle calculated from the previous inputs.
// -----------------------------------------------------------------------------

double ZenFNZenithal(double *pHourAngle,double *pDecli,double *pLatit)
 {
  // Declarations

  double RadHor, RadDecli, RadLat, CosZen;

  // Angles conversion from degrees to radians

  RadHor   = (double) *pHourAngle * DegToRad;
  RadDecli = (double) *pDecli   * DegToRad;
  RadLat   = (double) *pLatit   * DegToRad;

  // Calculate zenith angle

  CosZen = (double)sin(RadDecli)*sin(RadLat)+cos(RadLat)*cos(RadDecli)*cos(RadHor);

  return((double)acos(CosZen)*RadToDeg);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ZEN_FNTdiz
// -----------------------------------------------------------------------------
// PURPOSE       calculate the zenith and azimuth angle from a given fractional
//               day and the geolocation coordinates of a given observation site
//
// INPUT         calendarDay           the fractional calendar day;
//               pLongitude            the longitude, positive WESTWARDS;
//               pLatitude             the latitude, positive northwards;
//
// OUTPUT        pAzimuth              the azimuth angle;
//
// RETURN        the zenith angle
// -----------------------------------------------------------------------------

double ZEN_FNTdiz(double calendarDay,double *pLongitude,double *pLatitude,double *pAzimuth)
 {
  // Declarations

  double Longit,Latit,Decli,Ascen,Ang_Hor,cosA;

  //
  // BE CAREFUL :
  //
  // geographical longitudes are measured positively westwards from the meridian of Greenwich and
  // negatively to the east so for azimuthal angles, azimuth angles must be negatives in the morning (east)
  // following the convention used for longitudes
  //

  ZenSun(calendarDay,&Longit,&Latit);                                           // ecliptical coordinates for the sun
  ZenCoordEquat(&Longit,&Latit,&Decli,&Ascen);                                  // equatorial coordinates

  // Local hour angle

  Ang_Hor = (double) ZenFNSideral(calendarDay,calendarDay-(int)calendarDay)
          - (double)(*pLongitude)-Ascen;

  // Calculate the azimuth angle

  cosA=(double)cos(Ang_Hor*DegToRad)*sin(*pLatitude*DegToRad)-tan(Decli*DegToRad)*cos(*pLatitude*DegToRad);

  if (pAzimuth!=NULL)
   {
    if (fabs(cosA)<(double)1.e-6)
     *pAzimuth=(sin(Ang_Hor*DegToRad)<0.)?270.:90.;
    else
     *pAzimuth=fmod(atan2(sin(Ang_Hor*DegToRad),cosA)*RadToDeg,360.);
   }

  // Return the zenith angle

  return ( (double) ZenFNZenithal ( &Ang_Hor, &Decli, pLatitude ) );
 }


// EXAMPLE OF CALL : Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&NbSec),&longit,&latit,&Azimuth);
