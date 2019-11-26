
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  GEOCENTRIC MOON POSITIONS COMPUTATION
//  Name of module    :  MOON.C
//  Creation date     :  1996
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
//  FUNCTIONS
//
//  CONVERSION FUNCTIONS
//
//  DiffTime - return the difference (as a number of seconds) between two dates
//  Sec2Date - convert a number of seconds in a date
//  String2Date - DD/MM/YYYY HH:MM:SS -> Date(day,month,year,hour,min,sec)
//  Date2String - Date(day,month,year,hour,min,sec) -> YYYY.MMDDddddd (string)
//  String2Calendar - YYYY.MMDDddddd (string) -> Date(year,month,fractional day)
//
//  TIME FUNCTIONS
//
//  JulianDay - return the julian day in the gregorian calendar ([2],24)
//
//  MOON COORDINATES
//
//  MoonPosition - give the geocentric coordinates of the moon ([2],137)
//  Equatorial - transformation from ecliptical into equatorial coordinates ([2],40)
//  LocalHourAngle - calculate the local hour angle ([2],40)
//  Horizontal - transformation from equatorial into horizontal coordinates ([2],40)
//  ParallaxCorrection - calculate the parallax correction ([2],133)
//  MoonFraction - calculate the illuminated fraction of the moon disk
//
//  MOON_GetPosition - calculate the position of the moon at a given observation site
//                     and at a given time
//
//  ----------------------------------------------------------------------------
//
//  REFERENCES :
//
//  [1] Astronomy on the Personal Computer, Montenbruck & Pfleger, 1989-1994
//  [2] Astronomical formulae for calculators, Meeus, 1979
//  Qdoas is a cross-platform application for spectral analysis with the DOAS
//  algorithm (Differential Optical Analysis software)
//
//  ----------------------------------------------------------------------------

// ============
// HEADER FILES
// ============

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "doas.h"

// ====================
// STRUCTURE DEFINITION
// ====================

typedef struct _string2date
 {
  int    year,
         month,
         day,
         hour,
         min,
         sec;
  double nbSec;
 }
STRING2DATE;

// =====================
// CONSTANTS DEFINITIONS
// =====================

// Important Julian epochs
// Cfr. [1], pages 15,17

#define JD1900 (double) 2415020.      // 1900 January 0.5
#define JB1950 (double) 2433282.423   // 1950 January 0.923 (Besselian year)
#define JD1950 (double) 2433282.5     // 1950 January 1.
#define JD2000 (double) 2451545.      // 2000 January 1.5 introduced in 1984

#define JT JD1900   // the julian epoch to use in formula below

// Obliquity

#define E1950 (double) 23.4457889    // obliquity for standard equinox of 1950
#define E2000 (double) 23.43929111   // obliquity for standard equinox of 2000

#define EPS E1950

// degrees-radians conversion

#define RAD (double) 0.0174532925199433

// ==================
// MACROS DEFINITIONS
// ==================

#define Deg2Rad(X) ((double) fmod(RAD*X,PI2))

static int dayMonth[]={0,31,59,90,120,151,181,212,243,273,304,334};

// ====================
// CONVERSION FUNCTIONS
// ====================

// -----------------------------------------------------------------------------
// FUNCTION      DiffTime
// -----------------------------------------------------------------------------
// PURPOSE       return the difference (as a number of seconds) between two dates
//
// INPUT         pStartDate,pEndDate : the two dates
//
// RETURN        the number of seconds between the two input dates
// -----------------------------------------------------------------------------

double DiffTime(STRING2DATE *pStartDate,STRING2DATE *pEndDate)
 {
  // Declarations

  double nbSec;
  int nbBis;

  // number of leap years

  nbBis=(pEndDate->year-pStartDate->year+(((pStartDate->year%4)+3)%4))/4;

  // return the time difference in seconds between the two dates

  nbSec=(double)((pEndDate->year-pStartDate->year)*365.+nbBis)*86400.;
  nbSec=nbSec+pEndDate->nbSec-pStartDate->nbSec;

  return nbSec;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Sec2Date
// -----------------------------------------------------------------------------
// PURPOSE       convert a number of seconds in a date
//
// INPUT         refYear    the reference year
//               nbsec      the number of seconds
//
// OUTPUT        pDate      the components of the date in a structure
// -----------------------------------------------------------------------------

void Sec2Date(int refYear, double nbSec, STRING2DATE *pDate)
 {
  // Declarations

  double nbDay;
  int nbYear,nbBis;
  INDEX i;

  // Time decomposition

  nbDay       = (double) floor(nbSec/86400.); nbSec -= (double) nbDay*86400.;
  pDate->hour = (int) floor(nbSec/3600. ); nbSec -= (double) pDate->hour*3600.;
  pDate->min  = (int) floor(nbSec/60.   ); nbSec -= (double) pDate->min*60.;
  pDate->sec  = (int) nbSec;

  // Date decomposition

  nbYear = (int) floor(nbDay/365.);                                             // number of julian years
  nbDay  = (double) nbDay-nbYear*365.+1.;                                       // number of days in the remaining year

  nbBis  = (int)(nbYear+(int)(((refYear%4)+3)%4))/4;                            // number of extra days for leap years
  nbDay -= nbBis;

  for (i=1;i<12;i++)
   if (nbDay<=dayMonth[i])
    break;

  if (nbDay<=0.)
   {
    nbDay+=365.;
    nbYear--;
    i=12;
   }
  else if (((refYear+nbYear)%4==0) && (i>2))
   {
    if (nbDay-1<=dayMonth[i-1]) i--;
    if (i>2) nbDay-=1.;
   }

  // Date decomposition

  pDate->year=nbYear+refYear;
  pDate->month=i;
  pDate->day=(int)(nbDay-dayMonth[i-1]);
 }

// -----------------------------------------------------------------------------
// FUNCTION      String2Date
// -----------------------------------------------------------------------------
// PURPOSE       conversion DD/MM/YYYY HH:MM:SS -> Date(day,month,year,hour,min,sec)
//
// INPUT         stringDate  the date string (DD/MM/YYYY HH:MM:SS format)
//
// OUTPUT        pDate       the components of the date in a structure
// -----------------------------------------------------------------------------

void String2Date(char *stringDate,STRING2DATE *pDate)
 {
  // String date decomposition

  sscanf(stringDate,"%02d/%02d/%d %02d:%02d:%02d",
        &pDate->day,&pDate->month,&pDate->year,                                 // Date decomposition
        &pDate->hour,&pDate->min,&pDate->sec);                                  // Time decomposition

  // Number of seconds since 1/1 of the specified year

  pDate->nbSec = (double) dayMonth[pDate->month-1]+pDate->day-1.;               // julian day number

  if (((pDate->year%4)==0)&&(pDate->month>2))                                   // add an extra day for leap years
   pDate->nbSec+=(double)1.;

  pDate->nbSec=(double)pDate->nbSec*86400.+                                     // number of seconds in julian days
                       pDate->hour*3600.+                                       // number of seconds in hours
                       pDate->min*60.+                                          // number of seconds in minutes
                       pDate->sec;                                              // number of seconds in seconds
 }

// -----------------------------------------------------------------------------
// FUNCTION      String2Calendar
// -----------------------------------------------------------------------------
// PURPOSE       conversion Date(day,month,year,hour,min,sec) -> YYYY.MMDDddddd (string)
//
// INPUT         pDate               the components of the date in a structure
//
// OUTPUT        strDate             the date in string format
//
// RETURN        a pointer to the output string
// -----------------------------------------------------------------------------

char *Date2String(STRING2DATE *pDate,char *stringDate)
 {
 	// Declaration

  double fracDay;

  // Date conversion

  fracDay=(double) pDate->hour*3600.+pDate->min*60.+pDate->sec;
  fracDay/=(double)0.864;
  sprintf(stringDate,"%d.%02d%02d%05ld",
          pDate->year,pDate->month,pDate->day,(long)floor(fracDay+0.5));

  // Return

  return stringDate;
 }

// -----------------------------------------------------------------------------
// FUNCTION      String2Calendar
// -----------------------------------------------------------------------------
// PURPOSE       conversion YYYY.MMDDddddd (string) -> Date(year,month,fractional day)
//
// INPUT         strDate             the date in string format
//
// OUTPUT        pYear,pMonth,pDay   the components of the date
// -----------------------------------------------------------------------------

void String2Calendar(char *strDate,double *pYear,double *pMonth,double *pDay)
 {
  // Declaration

  double calendarDate;

  // Initialization

  calendarDate = (double) atof(strDate);

  // Date decomposition

  *pYear = floor(calendarDate);
  *pMonth = floor((calendarDate-(*pYear))*100.);
  *pDay = (calendarDate-(*pYear)-0.01*(*pMonth))*10000.;
 }

// ==========================
// TIME COMPUTATION FUNCTIONS
// ==========================

// -----------------------------------------------------------------------------
// FUNCTION      JulianDay
// -----------------------------------------------------------------------------
// PURPOSE       return the julian day in the gregorian calendar
//
// INPUT         year,month,day : the components of the date
//
// RETURN        the julian day in the gregorian calendar
// -----------------------------------------------------------------------------

double JulianDay(double year,double month,double day)
 {
  // Declarations

  double A,B;                                                                   // Gregorian calendar correction

  // Only for january and february months

  if (month<=2.)
   {
    year -= (double) 1.;
    month += (double) 12.;
   }

  // Gregorian calendar correction

  A = floor(year*0.01);         // A = int(year/100)
  B = 2.-A+floor(A*0.25);       // B = 2-A+int(A/4)

  // Julian day computation with correction

  return ((double) floor(year*365.25) +
                   floor((month+1.)*30.6001) +
                   day + 1720994.5 + B);
 }

// ================
// MOON COORDINATES
// ================

// -----------------------------------------------------------------------------
// FUNCTION      MoonPosition
// -----------------------------------------------------------------------------
// PURPOSE       give the geocentric coordinates of the moon
//
// INPUT         strCalendarDate  the calendar date in string format
//
// OUTPUT        lambda   geocentric longitude
//               beta     geocentric latitude
//               pi       equatorial horizontal parallax
// -----------------------------------------------------------------------------

void MoonPosition(char *strCalendarDate,double *lambda,double *beta,double *pi)
 {
  // Declarations //

  double year,month,day,           // For calendar date decomposition
         T,                        // The number of julian centuries
         l_,                       // Moon's mean longitude
         m,                        // Sun's mean anomaly
         m_,                       // Moon's mean anomaly
         d,                        // Moon's mean elongation
         f,                        // Mean distance of moon from its ascending node
         om,                       // Longitude of moon's ascending node
         t1,t2,t3,t4,              // Additive periodic terms
         e;                        // Coefficient

  // number of cen uries

  String2Calendar(strCalendarDate,&year,&month,&day);
  T=(double)(JulianDay(year,month,day)-JT)/36525.;

  // Initializations

  l_ = (double) 270.434164+T*(481267.8831-T*(0.001133-T*0.0000019));
  m  = (double) 358.475833+T*(35999.0498-T*(0.000150+T*0.0000033));
  m_ = (double) 296.104608+T*(477198.8491+T*(0.009192+T*0.0000144));
  d  = (double) 350.737486+T*(445267.1142-T*(0.001436-T*0.0000019));
  f  = (double) 11.250889+T*(483202.0251-T*(0.003211+T*0.0000003));
  om = (double) 259.183275-T*(1934.1420-T*(0.002078+T*0.0000022));
  e  = (double) 1.-T*(0.002495+T*0.00000752);

  // Additive periodic terms

  t1 = (double) sin(RAD*(51.2+20.2*T));
  t2 = (double) sin(RAD*(346.560+T*(132.870-T*0.0091731)));
  t3 = (double) sin(RAD*(om));
  t4 = (double) sin(RAD*(om+275.05-2.3*T));

  l_ += (double) 0.000233*t1 + 0.003964*t2 + 0.001964*t3;
  m  -= (double) 0.001778*t1;
  m_ += (double) 0.000817*t1 + 0.003964*t2 + 0.002541*t3;
  d  += (double) 0.002011*t1 + 0.003964*t2 + 0.001964*t3;
  f  += (double) 0.003964*t2 - 0.024691*t3 - 0.004328*t4;

  // Radians conversion

  m  = (double) Deg2Rad(m );
  m_ = (double) Deg2Rad(m_);
  d  = (double) Deg2Rad(d );
  f  = (double) Deg2Rad(f );

  // Geocentric longitude

  *lambda = l_ + 6.288750*sin(m_)
               + 1.274018*sin(2.*d-m_)
               + 0.658309*sin(2.*d)
               + 0.213616*sin(2.*m_)
               - 0.114336*sin(2.*f)
               + 0.058793*sin(2.*(d-m_))
               + 0.053320*sin(2.*d+m_)
               - 0.034718*sin(d)
               + 0.015326*sin(2.*(d-f))
               - 0.012528*sin(2.*f+m_)
               - 0.010980*sin(2.*f-m_)
               + 0.010674*sin(4.*d-m_)
               + 0.010034*sin(3.*m_)
               + 0.008548*sin(4.*d-2.*m_)
               + 0.005162*sin(m_-d)
               + 0.003996*sin(2.*(m_+d))
               + 0.003862*sin(4.*d)
               + 0.003665*sin(2.*d-3.*m_)
               + 0.002602*sin(m_-2.*(f+d))
               - 0.002349*sin(m_+d)
               - 0.001773*sin(m_+2.*(d-f))
               - 0.001595*sin(2.*(f+d))
               - 0.001110*sin(2.*(m_+f))
               + 0.000892*sin(m_-3.*d)
               + 0.000550*sin(m_+4.*d)
               + 0.000538*sin(4.*m_)
               + 0.000486*sin(2.*m_-d)

               + e*( 0.057212*sin(2.*d-m-m_)
                   + 0.045874*sin(2.*d-m)
                   + 0.041024*sin(m_-m)
                   - 0.185596*sin(m)
                   - 0.030465*sin(m+m_)
                   - 0.007910*sin(m-m_+2.*d)
                   - 0.006783*sin(2.*d+m)
                   + 0.005000*sin(m+d)
                   + 0.004049*sin(m_-m+2.*d)
                   + 0.002695*sin(2.*m_-m)
                   + 0.002396*sin(2.*(d-m_)-m)
                   - 0.002125*sin(2.*m_+m)
                   + 0.001220*sin(4.*d-m-m_)
                   - 0.000811*sin(m+m_+2.*d)
                   + 0.000761*sin(4.*d-m-2.*m_)
                   + 0.000693*sin(m-2.*(m_-d))
                   + 0.000598*sin(2.*(d-f)-m)
                   + 0.000521*sin(4.*d-m)

               + e*( 0.002249*sin(2.*(d-m))
                   - 0.002079*sin(2.*m)
                   + 0.002059*sin(2.*(d-m)-m_)
                   + 0.000717*sin(m_-2.*m)
                   + 0.000704*sin(m_-2.*(m+d))));

  // Geocentric latitude

  *beta = (double) (5.128189*sin(f)
                  + 0.280606*sin(m_+f)
                  + 0.277693*sin(m_-f)
                  + 0.173238*sin(2.*d-f)
                  + 0.055413*sin(2.*d+f-m_)
                  + 0.046272*sin(2.*d-f-m_)
                  + 0.032573*sin(2.*d+f)
                  + 0.017198*sin(2.*m_+f)
                  + 0.009267*sin(2.*d+m_-f)
                  + 0.008823*sin(2.*m_-f)
                  + 0.004323*sin(2.*(d-m_)-f)
                  + 0.004200*sin(2.*d+f+m_)
                  + 0.001828*sin(4.*d-f-m_)
                  - 0.001750*sin(3.*f)
                  - 0.001487*sin(f+d)
                  + 0.001330*sin(f-d)
                  + 0.001106*sin(f+3.*m_)
                  + 0.001020*sin(4.*d-f)
                  + 0.000833*sin(f+4.*d-m_)
                  + 0.000781*sin(m_-3.*f)
                  + 0.000670*sin(f+4.*d-2.*m_)
                  + 0.000606*sin(2.*d-3.*f)
                  + 0.000597*sin(2.*(d+m_)-f)
                  + 0.000450*sin(2.*(m_-d)-f)
                  + 0.000439*sin(3.*m_-f)
                  + 0.000423*sin(f+2.*(d+m_))
                  + 0.000422*sin(2.*d-f-3.*m_)
                  + 0.000331*sin(f+4.*d)
                  - 0.000283*sin(m_+3.*f)

                  + e*( 0.008247*sin(2.*d-m-f)
                      + 0.003372*sin(f-m-2.*d)
                      + 0.002472*sin(2.*d+f-m-m_)
                      + 0.002222*sin(2.*d+f-m)
                      + 0.002072*sin(2.*d-f-m-m_)
                      + 0.001877*sin(f-m+m_)
                      - 0.001803*sin(f+m)
                      + 0.001570*sin(m_-m-f)
                      - 0.001481*sin(f+m+m_)
                      + 0.001417*sin(f-m-m_)
                      + 0.001350*sin(f-m)
                      + 0.000492*sin(2.*d+m_-m-f)
                      + 0.000367*sin(m+f+2.*d-m_)
                      - 0.000353*sin(m+f+2.*d)
                      + 0.000317*sin(2.*d+f-m+m_)
                      + e*0.000306*sin(2.*(d-m)-f)))

                      * ((double) 1.
                      - 0.0004664*cos(RAD*(om))
                      - 0.0000754*cos(RAD*(om+275.05-2.30*T)));

  // Geocentric parallax

  *pi = (double) 0.950724
               + 0.051818*cos(m_)
               + 0.009531*cos(2.*d-m_)
               + 0.007843*cos(2.*d)
               + 0.002824*cos(2.*m_)
               + 0.000857*cos(2.*d+m_)
               - 0.000271*cos(d)
               - 0.000198*cos(2.*f-m_)
               + 0.000173*cos(3.*m_)
               + 0.000167*cos(4.*d-m_)
               + 0.000103*cos(4.*d-2.*m_)
               - 0.000084*cos(2.*(m_-d))
               + 0.000079*cos(2.*(d+m_))
               + 0.000072*cos(4.*d)
               - 0.000033*cos(3.*m_-2.*d)
               - 0.000030*cos(m_+d)
               - 0.000029*cos(2.*(f-d))
               - 0.000023*cos(2.*(f-d)+m_)

               + e*( 0.000533*cos(2.*d-m)
                   + 0.000401*cos(2.*d-m-m_)
                   + 0.000320*cos(m_-m)
                   - 0.000264*cos(m+m_)
                   - 0.000111*cos(m)
                   - 0.000083*cos(2.*d+m)
                   + 0.000064*cos(2.*d-m+m_)
                   - 0.000063*cos(2.*d+m-m_)
                   + 0.000041*cos(m+d)
                   + 0.000035*cos(2.*m_-m)
                   - 0.000029*cos(2.*m_+m)
                   + 0.000019*cos(4.*d-m-m_)
                   + e*0.000026*cos(2.*(d-m)));

  // Values to return

  *lambda = fmod(*lambda,360.);
  *beta = fmod(*beta,360.);
  *pi = fmod(*pi,360.);
 }

// -----------------------------------------------------------------------------
// FUNCTION      Equatorial
// -----------------------------------------------------------------------------
// PURPOSE       transformation from ecliptical into equatorial coordinates
//
// INPUT         lambda  geocentric longitude
//               beta    geocentric latitude
//
// OUTPUT        alpha   right ascension of the celestial equator
//               delta   declination of the celestial equator
// -----------------------------------------------------------------------------

void Equatorial(double lambda,double beta,double *alpha,double *delta)
 {
  // Declarations

  double sin_alpha,sin_delta,cos_lambda,eps;

  // Transformation from degrees into radian

  lambda = (double) RAD*lambda;
  beta   = (double) RAD*beta;
  eps    = (double) RAD*EPS;

  // Transformation from ecliptical into equatorial coordinates

  cos_lambda = cos(lambda);

  sin_alpha  = sin(lambda)*cos(eps)-tan(beta)*sin(eps);
  sin_delta  = sin(beta)*cos(eps)+cos(beta)*sin(eps)*sin(lambda);

  // Right ascension and declination of the celestial equator

  *alpha=(fabs(cos_lambda)<(double)1.e-6)?(double)90.:(double) fmod(atan2(sin_alpha,cos_lambda)/RAD,360.);

  if (fabs(sin_delta)>(double)1.)
   *delta=(sin_delta<(double)0.)?(double)-90.:(double)90.;
  else
   *delta = (double) fmod(asin(sin_delta)/RAD,360.);
 }

double T0;

// -----------------------------------------------------------------------------
// FUNCTION      LocalHourAngle
// -----------------------------------------------------------------------------
// PURPOSE       calculate the local hour angle
//
// INPUT         stringDate  string including date and time
//               longitude   longitude of the observation site
//               alpha       equatorial right ascension
//
// RETURN        the local hour angle
// -----------------------------------------------------------------------------

double LocalHourAngle(char *stringDate,double longitude,double alpha)
 {
  // Declarations

  double year,month,day,                                                        // calendar date decomposition
         calendarDate,                                                          // fractional date (floating point format)
         theta,                                                                 // Sidereal time at Greenwich
         H;                                                                     // Local hour angle

  // Date decomposition

  String2Calendar(stringDate,&year,&month,&day);

  // Sidereal time at Greenwich computed from Ephemeris time at 0h UT

  T0     = (double) (JulianDay(year,month,floor(day))-JT)/36525.;
  theta  = (double) (6.6460656+(2400.051262+0.00002581*T0)*T0)/24.;
  theta -= (double) floor(theta); // unit is a number of revolutions

  // Correction for any time UT

  calendarDate  = atof(stringDate)*10000.;
  calendarDate -= floor(calendarDate);

  theta = (theta+calendarDate*1.002737908)*24.;                                 // unit is hour

  // Local hour angle

  H = theta*15. /* degrees */ - longitude - alpha;

  return (fmod(H,360.));
 }

// -----------------------------------------------------------------------------
// FUNCTION      Horizontal
// -----------------------------------------------------------------------------
// PURPOSE       transformation from equatorial to horizontal coordinates
//
// INPUT         delta       equatorial declination
//               latitude    latitude of the observation site
//               H           local hour angle
//
// OUTPUT        A           azimuth measured westward from the south
//               h           altitude above or below the horizon
// -----------------------------------------------------------------------------

void Horizontal(double delta,double latitude,double H,double *A,double *h)
 {
  // Declarations

  double cos_A,sin_h;                                                           // temporary working variables

  latitude  = (double) RAD*latitude;
  delta     = (double) RAD*delta;
  H         = (double) RAD*H;

  // Transformation from equatorial to horizontal coordinates

  cos_A = cos(H)*sin(latitude)-tan(delta)*cos(latitude);
  sin_h = sin(latitude)*sin(delta)+cos(latitude)*cos(delta)*cos(H);

  // Local horizontal coordinates : azimuth and altitude above the horizon

  *A=(fabs(cos_A)<(double)1.e-6)?(double)90.:(double) fmod(atan2(H,cos_A)/RAD,360.);

  if (fabs(sin_h)>(double)1.)
   *h=(sin_h<(double)0.)?(double)-90.:(double)90.;
  else
   *h = (double) fmod(asin(sin_h)/RAD,360.);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ParallaxCorrection
// -----------------------------------------------------------------------------
// PURPOSE       calculate the parallax correction
//
// INPUT         latitude   latitude of the observation site
//               altitude   observer's height above sea-level
//               parallax   parallax
//               H          the geocentric hour angle
//
// OUTPUT        alpha      right ascension
//               delta      declination
// -----------------------------------------------------------------------------

void ParallaxCorrection(double  latitude,double  altitude,double parallax,double H,
                        double *alpha,double *delta)
 {
  // Declarations

  double ratio,u,                                                               // the ratio of the polar radius to the equatorial radius
         rsin,rcos,                                                             // polar coordinates
         sin_dalpha,cos_dalpha,                                                 // correction for right ascension
         sin_delta,cos_delta,                                                   // correction for declination
         dalpha,                                                                // parallaxes in right ascension
         delta_rad;                                                             // declination expressed in radians

  // Transformation from degrees into radian

  latitude = (double) RAD*latitude;
  parallax = (double) RAD*parallax;
 *delta    = (double) RAD*(*delta);
  H        = (double) RAD*H;

  // Initialization

  ratio   = (double) 0.99664719;

  // Polar coordinates

  u=atan2(ratio*sin(latitude),cos(latitude));

  rsin=ratio*sin(u)+altitude*sin(latitude)/6378140.;
  rcos=cos(u)+altitude*cos(latitude)/6378140.;

  // parallax in right ascension

  sin_dalpha = (double) -1.*rcos*sin(parallax)*sin(H);
  cos_dalpha = (double) cos((*delta))-rcos*sin(parallax)*cos(H);

  dalpha = (double) atan2(sin_dalpha,cos_dalpha);

  // parallax in declination

  sin_delta = (double) (sin((*delta))-rsin*sin(parallax))*cos(dalpha);
  cos_delta = (double) cos_dalpha;

  delta_rad = (double) atan2(sin_delta,cos_delta);

  *alpha += (double) dalpha/RAD;
  *delta  = (double) delta_rad/RAD;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MoonFraction
// -----------------------------------------------------------------------------
// PURPOSE       calculate the illuminated fraction of the moon disk
//
// INPUT         T       ephemeris time
//               lambda  geocentric longitude
//               beta    geocentric latitude
//
// RETURN        the illuminated fraction of the moon disk
// -----------------------------------------------------------------------------

double MoonFraction(double T,double lambda,double beta)
 {
  // Declarations

  double deg_to_rad,rad_to_deg,pi2,                                             // conversion degrees <-> radians

         L,                                                                     // geometric mean longitude of the sun
         M,                                                                     // sun's mean anomaly
         C,                                                                     // center of the sun
         theta,                                                                 // sun's true longitude
         m,                                                                     // sun's mean anomaly
         m_,                                                                    // moon's mean anomaly
         d,                                                                     // working variable
         i;                                                                     // the moon's phase angle

  // Initializations

  deg_to_rad = (double) DOAS_PI/180.;
  rad_to_deg = (double) 180./DOAS_PI;
  pi2 = (double) 2.*DOAS_PI;

  // Sun's true longitude

  L = (double) 279.69668 + (36000.76892+0.0003025*T)*T;
  M = (double) 358.47583 + (35999.04975-(0.000150+0.0000033*T)*T)*T;

  M = (double) fmod(deg_to_rad*M,pi2);

  C = (double) (1.919460-(0.004789+0.000014*T)*T)*sin(M) +
               (0.020094-0.0001*T)*sin(M+M) +
                0.000293*sin(M+M+M);

  theta = (double) fmod(L+C,360.);

  // Moon's phase angle

  m  = (double) 358.475833+T*(35999.0498-T*(0.000150+T*0.0000033));
  m_ = (double) 296.104608+T*(477198.8491+T*(0.009192+T*0.0000144));

  theta = (double) fmod(deg_to_rad*(lambda-theta),pi2);
  beta = (double) fmod(deg_to_rad*beta,pi2);
  m = (double) fmod(deg_to_rad*m,pi2);
  m_ = (double) fmod(deg_to_rad*m_,pi2);

  d = (double) acos(cos(theta)*cos(beta));
  i = (double) 180.-fmod(rad_to_deg*d,180.)-0.1468*sin(d)*(1.-0.0549*sin(m_))/(1.-0.0167*sin(m));

  // Illuminated fraction of the moon's disk

  return (double) (1.+cos(fmod(deg_to_rad*i,pi2)))*0.5;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MOON_GetPosition
// -----------------------------------------------------------------------------
// PURPOSE       calculate the position of the moon at a given observation site
//               and at a given time
//
// INPUT         inputDate   input date and time for moon positions calculation
//               longitude   longitude of the observation site
//               latitude    latitude of the observation site
//               altitude    altitude of the observation site
//
//  OUTPUT       pA          azimuth, measured westward from the south
//               ph          altitude above the horizon
//               pFrac       illuminated fraction of the moon
// -----------------------------------------------------------------------------

void MOON_GetPosition(char *inputDate,double longitude,double latitude,double altitude,
                      double *pA,double *ph,double *pFrac)
 {
  // Declarations
                           // INPUT DATE AND TIME TRANSFORMATION
  char stringDate[20];    // Date(day,month,year,hour,min,sec) -> YYYY.MMDDddddd
  STRING2DATE dateTime;      // DD/MM/YYYY HH:MM:SS -> Date(day,month,year,hour,min,sec)

                           // GEOCENTIC COORDINATES
  double lambda,           // geocentric longitude
         beta,             // geocentric latitude
         pi,               // parallax

                           // EQUATORIAL COORDINATES
         alpha,            // right ascension
         delta,            // declination
         H;                // local hour angle

  // -< ARGUMENTS READ OUT >----------------------------------------------------

  String2Date(inputDate,&dateTime);
  Date2String(&dateTime,stringDate);

  // -< GEOCENTRIC COORDINATES OF THE MOON >------------------------------------

  MoonPosition(stringDate, // INPUT  : calendar date in string format
               &lambda,    // OUTPUT : celestial geocentric longitude
               &beta,      //          celestial geocentric latitude
               &pi);       //          celestial equatorial horizontal parallax

  // -< TRANSFORMATION FROM ECLIPTICAL INTO EQUATORIAL COORDINATES >------------

  Equatorial(lambda,
             beta,
             &alpha,       // right ascension
             &delta);      // declination

  // -< LOCAL HOUR ANGLE >------------------------------------------------------

  H = LocalHourAngle(stringDate,longitude,alpha);

  // -< CORRECTION FOR PARALLAX >-----------------------------------------------

  ParallaxCorrection(latitude,altitude,pi,H,&alpha,&delta);

  // -< TRANSFORMATION FROM EQUATORIAL INTO HORIZONTAL COORDINATES >------------

  Horizontal(delta,latitude,H,pA,ph);

  *pFrac=MoonFraction(T0,       // INPUT  : ephemeris time
                      lambda,   //          geocentric longitude
                      beta);    //          geocentric latitude
 }
