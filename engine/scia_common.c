
#include "scia_defs.h"
#include "bin_read.h"
#include "scia_common.h"

#if HAVE_LIBGSL
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_double.h>
#endif

/* Identifier for the various SCIA-Products, corresponds to enum SCIA_PRODUCT*/

char SCIA_PRODUCT_ACR [SCIA_MAX_PRODUCT][12] =
{"SCI_NL__2P",
 "SCI_OL__2P",			/* to be checked!!!! */
 "SCI_RV__2P",
 "SCI_NL__1P",
 "SCI_NLC_1C",
 "MIP_NL__2P" };

/* Reading the main product header MPH */
/* This is ASCII Format, therefore not bin... but fscanf is used */

SCIA_err Read_MPH (FILE* unit, MPH *mph)

{
    unsigned n;
    int err;
    char nl[2];
    char *tmp = (char*) mph;
    /* Zeros in the complete structure, so all strings have trailing \0 */
    for (n=0; n<sizeof(MPH); n++)
	tmp[n] = 0;

    err = fscanf(unit,
		 "PRODUCT=\"%62c\"%1c"
		 "PROC_STAGE=%1c%1c"
		 "REF_DOC=\"%23c\"%1c"
		 "%40c%1c"
		 "ACQUISITION_STATION=\"%20c\"%1c"
		 "PROC_CENTER=\"%6c\"%1c"
		 "PROC_TIME=\"%27c\"%1c"
		 "SOFTWARE_VER=\"%14c\"%1c"
		 "%40c%1c"
		 "SENSING_START=\"%27c\"%1c"
		 "SENSING_STOP=\"%27c\"%1c"
		 "%40c%1c"
		 "PHASE=%1c%1c"
		 "CYCLE=%4c%1c"
		 "REL_ORBIT=%6c%1c"
		 "ABS_ORBIT=%6c%1c"
		 "STATE_VECTOR_TIME=\"%27c\"%1c"
		 "DELTA_UT1=%8c<s>%1c"
		 "X_POSITION=%12c<m>%1c"
		 "Y_POSITION=%12c<m>%1c"
		 "Z_POSITION=%12c<m>%1c"
		 "X_VELOCITY=%12c<m/s>%1c"
		 "Y_VELOCITY=%12c<m/s>%1c"
		 "Z_VELOCITY=%12c<m/s>%1c"
		 "VECTOR_SOURCE=\"%2c\"%1c"
		 "%40c%1c"
		 "UTC_SBT_TIME=\"%27c\"%1c"
		 "SAT_BINARY_TIME=%11c%1c"
		 "CLOCK_STEP=%11c<ps>%1c"
		 "%32c%1c"
		 "LEAP_UTC=\"%27c\"%1c"
		 "LEAP_SIGN=%4c%1c"
		 "LEAP_ERR=%1c%1c"
		 "%40c%1c"
		 "PRODUCT_ERR=%1c%1c"
		 "TOT_SIZE=%21c<bytes>%1c"
		 "SPH_SIZE=%11c<bytes>%1c"
		 "NUM_DSD=%11c%1c"
		 "DSD_SIZE=%11c<bytes>%1c"
		 "NUM_DATA_SETS=%11c%1c"
		 "%40c%1c"
		 ,
		 mph->product, nl,
		 mph->proc_stage, nl,
		 mph->ref_doc, nl,
		 mph->spare_1, nl,
		 mph->acquisition_station, nl,
		 mph->proc_center, nl,
		 mph->proc_time, nl,
		 mph->software_ver, nl,
		 mph->spare_2, nl,
		 mph->sensing_start, nl,
		 mph->sensing_stop, nl,
		 mph->spare_3, nl,
		 mph->phase, nl,
		 mph->cycle, nl,
		 mph->rel_orbit, nl,
		 mph->abs_orbit, nl,
		 mph->state_vector_time, nl,
		 mph->delta_ut1, nl,
		 mph->x_position, nl,
		 mph->y_position, nl,
		 mph->z_position, nl,
		 mph->x_velocity, nl,
		 mph->y_velocity, nl,
		 mph->z_velocity, nl,
		 mph->vector_source, nl,
		 mph->spare_4, nl,
		 mph->utc_sbt_time, nl,
		 mph->sat_binary_time, nl,
		 mph->clock_step, nl,
		 mph->spare_5, nl,
		 mph->leap_utc, nl,
		 mph->leap_sign, nl,
		 mph->leap_err, nl,
		 mph->spare_6, nl,
		 mph->product_err, nl,
		 mph->tot_size, nl,
		 mph->sph_size, nl,
		 mph->num_dsd, nl,
		 mph->dsd_size, nl,
		 mph->num_data_sets, nl,
		 mph->spare_7, nl );

    if (err != 82)
	return MPH_ERROR;
    return OK;
}




/* Identify product by MPH */
SCIA_PRODUCT scia_product (MPH *mph)
{
    int n;
    for (n=0; n<SCIA_MAX_PRODUCT; n++)
	if (strncmp ( SCIA_PRODUCT_ACR[n] , mph->product, 10 ) == 0 )
	    break;
    /* in C++ enum is not necessarily int, therefore int cannot be given */
    /* back here as SCIA_PRODUCT and  SCIA_PRODUCT cannot be used in loop */
//    switch
    switch (n)
    {
	case 0:
	    return SCIA_L2N;
	case 1:
	    return SCIA_L2O;
	case 2:
	    return SCIA_L2M;
	case 3:
	    return SCIA_L1B;
	case 4:
	    return SCIA_L1C;
	case 5:
	    return MIPAS_L2;
	case 6:
	    return GOME_L2;
    }
    return SCIA_MAX_PRODUCT;
	/* SCIA_MAX_PRODUCT is unknown product  */
}


/* Read the Dataset Desc */

SCIA_err Read_DSD (FILE* unit, DSD *dsd)
{
    int err;
    char nl[2];
    err=fscanf(unit,
	   "DS_NAME=\"%28c\"%1c"
	   "DS_TYPE=%1c%1c"
	   "FILENAME=\"%62c\"%1c"
	   "DS_OFFSET=+%20u<bytes>%1c"
	   "DS_SIZE=+%20u<bytes>%1c"
	   "NUM_DSR=+%10u%1c"
	   "DSR_SIZE=%11d<bytes>%1c"
	   "%32c%1c",
	       dsd->name, nl,
	       &dsd->type, nl,
	       dsd->filename, nl,
	       &dsd->offset, nl,
	       &dsd->size, nl,
	       &dsd->num_dsr, nl,
	       &dsd->dsr_size, nl,
	       dsd->spare, nl);
/*
    dsd->name[28]='\0';
    if ( (str_ptr = strpbrk(dsd->name, " ") ) != NULL)
	*str_ptr='\0';

    dsd->filename[62]='\0';
    if ( (str_ptr = strpbrk(dsd->filename, " ") ) != NULL)
	*str_ptr='\0';
	*/
    if ( err != 16 )
	return DSD_ERROR;
    return OK;
}


/* Write the Dataset Desc */

SCIA_err Write_DSD (FILE* unit, DSD *dsd)
{
    int err;
    char nl[2] = "\n";
    err=fprintf(unit,
	   "DS_NAME=\"%.28s\"%.1s"
	   "DS_TYPE=%c%.1s"
	   "FILENAME=\"%.62s\"%.1s"
	   "DS_OFFSET=+%020u<bytes>%.1s"
	   "DS_SIZE=+%020u<bytes>%.1s"
	   "NUM_DSR=+%010u%.1s"
	   "DSR_SIZE=%+011d<bytes>%.1s"
	   "%.32s%.1s",
	       dsd->name, nl,
	       dsd->type, nl,
	       dsd->filename, nl,
	       dsd->offset, nl,
	       dsd->size, nl,
	       dsd->num_dsr, nl,
	       dsd->dsr_size, nl,
	       dsd->spare, nl);

/*    if ( err != 16 )
      return DSD_ERROR; */
    return OK;
}





/* set the filepointer to the Begin of dataset described by this DSD */
SCIA_err set_DSD_offset (FILE* unit, DSD *dsd)
{
    if (dsd->offset == 0)
	return SCIA_DSD_NOT_ATTACHED;
    if (fseek(unit, dsd->offset, SEEK_SET) == -1)
    {
	return SCIA_ERROR;
    }
    return OK;
}



/*********************************************************************\
 * Compare two dates MJD
 * MJD compare : 0 equal  -1 earlier  1 later
 * kb 23.04.01
\*********************************************************************/

int MJD_compare (const MJD *const mjd1, const MJD *const mjd2)
{
    if (mjd1->days < mjd2->days)
	return -1;
    if (mjd1->days > mjd2->days)
	return 1;
				/* Days are equal !! */
    if (mjd1->secnd < mjd2->secnd)
	return -1;
    if (mjd1->secnd > mjd2->secnd)
	return 1;
				/* secnd are equal !! */
    if (mjd1->musec < mjd2->musec)
	return -1;
    if (mjd1->musec > mjd2->musec)
	return 1;
				/* musec are equal !! */
    return 0;
}

/**********************************************************************
 * add 1/16 seconds to MJD
 **********************************************************************/

MJD MJD_add (MJD mjd, int sec_16)
{
    long sec_16_factor = 62500;
    long secnd = mjd.secnd;
    long musec = mjd.musec;
    long musec_2;
    long secnd_2;
    long days_2;
    long secnd_3;
    /* separate full seconds from sec_16 to avoid overflow of integers */
    secnd_3 = sec_16 / 16;
    sec_16  = sec_16 % 16;

    musec_2 = (musec + sec_16 * sec_16_factor) % MIKROSEC_PER_SEC;
    secnd_2 = (musec + sec_16 * sec_16_factor ) / MIKROSEC_PER_SEC;
    if (musec_2 < 0)
    {
	musec_2 += 1000000;
	secnd_2 -= 1;
    }
    secnd_3 += secnd + secnd_2;
    secnd_2 = secnd_3 % SECOND_PER_DAY;
    days_2 = secnd_3 / SECOND_PER_DAY;
    if (secnd_2 < 0)
    {
	secnd_2 += SECOND_PER_DAY;
	days_2 -= 1;
    }

    mjd.musec = musec_2;
    mjd.secnd = secnd_2;
    mjd.days += days_2;
    return mjd;
}



/**********************************************************************
 * add  seconds to MJD
 **********************************************************************/

MJD MJDadd_sec (MJD mjd, double sec)
{
    long secnd = mjd.secnd;
    long musec = mjd.musec;
    long musec_2;
    long musec_3;
    long secnd_2;
    long days_2;
    long secnd_3;

    /* separate full seconds from sec_16 to avoid overflow of integers */
    secnd_3 = (long) floor (sec);
    musec_3 = (long) floor ((sec - floor (sec))* (double) MIKROSEC_PER_SEC) ;

    musec_2 = (musec + musec_3 ) % MIKROSEC_PER_SEC;
    secnd_2 = (musec + musec_3 ) / MIKROSEC_PER_SEC;
    if (musec_2 < 0)
    {
	musec_2 += 1000000;
	secnd_2 -= 1;
    }
    secnd_3 += secnd + secnd_2;
    secnd_2 = secnd_3 % SECOND_PER_DAY;
    days_2 = secnd_3 / SECOND_PER_DAY;
    if (secnd_2 < 0)
    {
	secnd_2 += SECOND_PER_DAY;
	days_2 -= 1;
    }

    mjd.musec = musec_2;
    mjd.secnd = secnd_2;
    mjd.days += days_2;
    return mjd;

}

/**********************************************************************
 * add  days to MJD
 **********************************************************************/

MJD MJDadd_day (MJD mjd, double day)
{
    double days = floor (day);
    mjd.days += (int) days;
    return MJDadd_sec (mjd, (day-days) * SECOND_PER_DAY);
}

/* Calculate mjd2 - mjd1 in seconds */
double MJD_diff (MJD *mjd1, MJD *mjd2)
{
/*    MJD *larger;
    MJD *smaller;
    double sign = 1.0;*/
    int ddays, dsecnd, dmusec;
    double diff;

    /*
    switch  (MJD_compare (mjd1, mjd2))
    {
	case -1:
	    larger = mjd2;
	    smaller = mjd1;
	    break;
	case 0:
	    return 0.0;
	    break;
	case 1:
	    larger = mjd1;
	    smaller = mjd2;
	    sign = -1.0;
	    break;
    }
    */

    ddays = mjd2->days - mjd1->days;
    dsecnd = mjd2->secnd - mjd1->secnd;
    if (dsecnd < 0)
    {
	ddays  -= 1;
	dsecnd += SECOND_PER_DAY;
    }
    dmusec = mjd2->musec - mjd1->musec;
    if (dmusec < 0)
    {
	dsecnd -= 1;
	dmusec += 1000000;
    }
    diff = (double) ddays * SECOND_PER_DAY +
	(double) dsecnd +
	(double) dmusec / MIKROSEC_PER_SEC;

    return diff; /* * sign; */
}


/*--------------------------------------------------------------------*\
**
** routines for creation of UTC String from MJD structure
** (musec are ignored)
**
** Author: S. Noel, IFE/IUP Uni Bremen
**        (Stefan.Noel@iup.physik.uni.bremen.de)
**
** Version: 0.1, 25 Mar 2002
**
** (This routine is based on "MJD_2_ASCII" from the NADC lib.)
**
\*--------------------------------------------------------------------*/


/*+++++++++++++++++++++++++
.IDENTifer   CALDAT
.PURPOSE     return the calendar date for given julian date
.INPUT/OUTPUT
  call as    CALDAT( ijul, &iday, &imon, &iyear );

     input:
            unsigned int ijul     :  Julian day number (starts at noon)
    output:
            int  *iday  :   number of day of the month.
            int  *imon  :   number of the month (1 = january, ... )
            int  *iyear :   number of the year

.RETURNS     nothing
.COMMENTS    static function
             Taken from "Numerical Recipies in C", by William H. Press,
	     Brian P. Flannery, Saul A. Teukolsky, and William T. Vetterling.
	     Cambridge University Press, 1988 (second printing).

-------------------------*/
/*********** NOTE: This is taken from the NADC lib *************/
static
void CALDAT( unsigned int ijul, /*@out@*/ int *iday,
             /*@out@*/ int *imon, /*@out@*/ int *iyear )
{
     int ja, jalpha, jb, jc, jd, je;
/*
 * Gregorian Calander was adopted on Oct. 15, 1582
 */
     const unsigned int IGREG = 15U + 31U * (10U + 12U * 1582U);

     if ( ijul >= IGREG ) {
	  jalpha = (int) (((float) (ijul - 1867216) - 0.25)
			  / 36524.25);
	  ja = ijul + 1 + jalpha - (int) (0.25 * jalpha);
     } else {
	  ja = (int) ijul;
     }
     jb = ja + 1524;
     jc = (int)(6680.0 + ((float) (jb - 2439870) - 122.1) / 365.25);
     jd = (int)(365 * jc + (0.25 * jc));
     je = (int)((jb - jd) / 30.6001);
     *iday = jb - jd - (int) (30.6001 * je);
     *imon = je - 1;
     if ( *imon > 12 ) *imon -= 12;
     *iyear = jc - 4715;
     if ( *imon > 2 ) --(*iyear);
     if ( *iyear <= 0 ) --(*iyear);
}


/*--------------------------------------------------------------------*\
**       create UTC String from MJD structure
** 	 (This routine is based on "MJD_2_ASCII" from the NADC lib.)
\*--------------------------------------------------------------------*/
void UTC_String(MJD* time, char* string)
{
  /*
   * Julian date at 01/01/2000
   */
  static const double jday_01012000 = 2451544.5;
  /*
   * array for month names
   */
  static const char *mon_str[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  int  iday, ihour, imin, imon, isec, iyear;
  unsigned int jday;

  /*
   * convert to julian day
   */

  jday = (unsigned) ((time->days + jday_01012000 + 0.5) );
  CALDAT( jday, &iday, &imon, &iyear );

  ihour = (int)(time->secnd / 3600);
  isec  = (int) time->secnd - (3600 * ihour);
  imin  = isec / 60;
  isec -= 60 * imin;
  (void) sprintf( string, "%.2d-%3s-%.4d %.2d:%.2d:%.2d.%.6u", iday,
		  mon_str[imon-1], iyear, ihour, imin, isec, time->musec);

  return;
}

/*--------------------------------------------------------------------*\
**       create UTC String from MJD structure with UPPER CASE month
** 	 (This routine is based on "MJD_2_ASCII" from the NADC lib.)
\*--------------------------------------------------------------------*/
void UTC_STRING(MJD* time, char* string)
{
  /*
   * Julian date at 01/01/2000
   */
  static const double jday_01012000 = 2451544.5;
  /*
   * array for month names
   */
  static const char *mon_str[12] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };

  int  iday, ihour, imin, imon, isec, iyear;
  unsigned int jday;

  /*
   * convert to julian day
   */

  jday = (unsigned) ((time->days + jday_01012000 + 0.5) );
  CALDAT( jday, &iday, &imon, &iyear );

  ihour = (int)(time->secnd / 3600);
  isec  = (int) time->secnd - (3600 * ihour);
  imin  = isec / 60;
  isec -= 60 * imin;
  (void) sprintf( string, "%.2d-%3s-%.4d %.2d:%.2d:%.2d.%.6u", iday,
		  mon_str[imon-1], iyear, ihour, imin, isec, time->musec);

  return;
}

/*--------------------------------------------------------------------*\
**       create UTC String as yyyymmdd_hhmmss from MJD structure
** 	 (This routine is based on "MJD_2_ASCII" from the NADC lib.)
\*--------------------------------------------------------------------*/
void MJD_2_yyyymmdd (MJD* time, char* yyyymmdd)
{
  /*
   * Julian date at 01/01/2000
   */
  static const double jday_01012000 = 2451544.5;

  int  iday, ihour, imin, imon, isec, iyear;
  unsigned int jday;

  /*
   * convert to julian day
   */

  jday = (unsigned) ((time->days + jday_01012000 + 0.5) );
  CALDAT( jday, &iday, &imon, &iyear );

  ihour = (int)(time->secnd / 3600);
  isec  = (int) time->secnd - (3600 * ihour);
  imin  = isec / 60;
  isec -= 60 * imin;

  (void) sprintf( yyyymmdd, "%04d%02d%02d_%02d%02d%02d",
		  iyear, imon, iday, ihour, imin, isec);
  return;
}


/*+++++++++++++++++++++++++
.IDENTifer   JULDAY
.PURPOSE     return julian day for given the calendar date
.INPUT/OUTPUT
  call as    jday = JULDAY( myclock );

     input:
            struct tm myclock  :  time in tm_format

.RETURNS     Julian day number and fractional day
.COMMENTS    static function
             Taken from "Numerical Recipies in C", by William H. Press,
	     Brian P. Flannery, Saul A. Teukolsky, and William T. Vetterling.
	     Cambridge University Press, 1988 (second printing).
-------------------------*/
double JULDAY( struct tm myclock )
{
     unsigned int ja, jd, ijul;
     int          jy, jm, year, imon;
	 const unsigned int IGREG = 15U + 31U * (10U + 12U * 1582U);

    /* 'struct tm' contains years since 1900 */
     year = myclock.tm_year + 1900;
     imon = myclock.tm_mon + 1;
/*
 * Gregorian Calander was adopted on Oct. 15, 1582
 */


     if ( (jy = year) < 0 ) jy++;
     if ( imon > 2 ) {
	  jm = imon + 1;
     } else {
	  jy--;
	  jm = imon + 13;
     }
     ijul = (unsigned int) (floor( 365.25 * jy )
			    + floor( 30.6001 * jm )
			    + myclock.tm_mday + 1720995U);
/*
 * test whether to change to Gregorian calendar
 */
     jd = myclock.tm_mday + 31U * (imon + 12U * year);
     if ( jd >= IGREG ) {
	  ja = (unsigned int)(0.01 * jy);
	  ijul += 2U - ja + (unsigned int) (0.25 * ja);
     }
     return ijul + (myclock.tm_hour / 24. - 0.5)
	  + (myclock.tm_min / 1440.)
	  + (myclock.tm_sec / 86400.0);
}




/*+++++++++++++++++++++++++
.IDENTifer   ASCII_2_UTC
.PURPOSE     Converts yyyymmdd time into ESA UTC time
.INPUT/OUTPUT
  call as    ASCII_2_UTC( ASCII_Time, &utc_day, &utc_sec );

     input:
            char *ASCII_Time      :  given as DD-MMM-YYYY HH:MM:SS.SSS
    output:
            unsigned int utc_day  :  ESA UTC days since 01.01.1950
            unsigned int utc_msec :  ESA UTC milli-seconds since midnight

.RETURNS     error status : 0 on error, 1 on success
.COMMENTS    none
-------------------------*/
int yyyymmdd_2_MJD( const char yyyymmdd[],
		    MJD* mjd)
/*		  unsigned int *utc_day, unsigned int *utc_msec )*/
{
    int err;

    double      jday, musec;

    struct tm myclock;

    const double jday_01012000 = 2451544.5;

/*
 * initialisation
 */
    mjd->days = 0;
    mjd->secnd = 0U;
    mjd->musec = 0U;

/*20031021_202415*/

/*
 * decomposition of date and time part into numbers into the tm struct
 */
    err = sscanf( yyyymmdd, "%4d%2d%2d",
		  &myclock.tm_year, &myclock.tm_mon,
		  &myclock.tm_mday );
    /* 'struct tm' contains years since 1900 */
    myclock.tm_year -= 1900;
    /* 'struct tm' contains month 0 - 11 */
    myclock.tm_mon -= 1;

    if (err != 3)
	return 0;
    if (strlen (yyyymmdd) > 8)
    {
	err =  sscanf( yyyymmdd + 9, "%2d%2d%2d",
		       &myclock.tm_hour,
		       &myclock.tm_min,
		       &myclock.tm_sec );
	if (err != 3)
	    return 0;
    } else 			/* time part is not given */
    {				/* set it to zeros */
	myclock.tm_hour = 0;
	myclock.tm_min = 0;
	myclock.tm_sec = 0;
    }
    /*  adding 0.1 musec avoid rounding errors */
    /*  in last digit (otherwise f.e. 306177 */
    /*  would be printed read as 306176 */
    myclock.tm_wday = 0;
    myclock.tm_yday = 0;
    myclock.tm_isdst = 0;
    musec = 0;

    /*  Calculate Julian Day */
    jday = JULDAY( myclock );
    jday = jday -  jday_01012000;
    /*  Build MJD */
    mjd->days = (int) floor (jday);
    mjd->secnd = (unsigned int)(jday - mjd->days) * SECOND_PER_DAY;
    mjd->musec = (unsigned int) musec;
    return 1;
}



/*--------------------------------------------------------------------*\
**       create UTC String as  yyyymmdd_hhmmss  from UTC_string
\*--------------------------------------------------------------------*/
void yyyymmdd_string(char* utc, char* yyyymmdd)
{
  /*
   * array for month names
   */
  static const char *mon_str[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static const char *MON_STR[12] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };


  int  n_mon;

  for (n_mon = 0; n_mon<12; n_mon++)
  {
      if (strncmp (MON_STR[n_mon], utc+3, 3) == 0 ||
	  strncmp (mon_str[n_mon], utc+3, 3) == 0)
	  break;
  }
  /*
   * convert to yyyymmdd_hhmmss
   */
				/* "05-AUG-2002 11:15:42.188532" */
  sprintf (yyyymmdd, "%.4s%02d%.2s_%.2s%.2s%.2s",
	   utc+7,
	   n_mon + 1,
	   utc,
	   utc+12,
	   utc+15,
	   utc+18);

  return;
}


/*--------------------------------------------------------------------*\
**       create UTC String as  yyyymmdd_hhmmss  from UTC_string
\*--------------------------------------------------------------------*/
void yyyymmdd_string_num (char* utc, char* yyyymmdd,
		     int *yyyy, int *mm, int *dd, int *hh, int *min, int *ss,
		     int *ms)
{
    yyyymmdd_string (utc, yyyymmdd);
    sscanf (yyyymmdd,"%4d%02d%02d", yyyy, mm, dd);
    sscanf (utc+12, "%2d:%2d:%2d.%6d", hh, min, ss, ms);
    return;
}

/*--------------------------------------------------------------------*\
**       Calculate fractional day  after 1/1/2000
\*--------------------------------------------------------------------*/

double JDAY_01012000 ( MJD* time )
{
   /* Julian date at 01/01/2000
   */
    double jday = (double) time->days +
	(double) time->secnd / SECOND_PER_DAY +
	(double) time->musec /
	((double)SECOND_PER_DAY * (double)MIKROSEC_PER_SEC);
    return jday;
}


double JDAY_01012000_ymd ( const char yyyymmdd[] )
{
    MJD mjd;
    yyyymmdd_2_MJD( yyyymmdd,
		    & mjd);
    return JDAY_01012000 ( &mjd );
}

/* calculate MJD from Julian day since 1.1.2000 */
MJD JDAY_01012000_2_MJD (double jday)
{
    MJD mjd;
    double days;
    double jsecnd,secnd;
    double jmusec;
    days = floor (jday);
    mjd.days = (int) days;
    jsecnd = (jday - days) * SECOND_PER_DAY;

    secnd = floor (jsecnd);
    mjd.secnd = (unsigned int) secnd;
    jmusec = (jsecnd - secnd) * MIKROSEC_PER_SEC;

    mjd.musec = (unsigned int) floor (jmusec);
    return mjd;
}


/* calculate MJD from Julian Day */
MJD JDAY_2_MJD (double jday)
{
    const double jday_01012000 = 2451544.5;
    return JDAY_01012000_2_MJD (jday-jday_01012000);
}

/* convert date string from yyyy-mm-dd hh:mm:ss.xx to
   Envisat format  25-SEP-2006 02:32:39.880363 */
void alpha_month (const char*date, char *utc)
{
  /*
   * array for month names
   */
    int mm=0;
    static const char *mon_str[12] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    mm = atol (date+5);
    if (mm<0 || mm > 11)
	SCIA_ERROR(FATAL, "Month number not detected in alpha_month.",
		   date);

    sprintf (utc, "%.2s-%3s-%.4s %.11s0000",
	     date+8,  mon_str[mm-1], date,
	     date+11);
    return;
}

/* ----------------------------------------------------------------- *\
 * calculate orbit from time                                         *
 * input : julian day since 1.1.2000                                 *
 * output : corresponding orbit number                               *
 * ----------------------------------------------------------------- */

int orbit_julian (double jday)
{
    double t0 = 824.026091; /* ANX time of ref. orbit */
    double orbit0 = 486.; /* no. of ref. orbit */
    double ref_period = 6035.928; /* (default) orbital period in s */

    return (int)( (jday-t0) *24.*3600. / ref_period + orbit0);
}

int orbit_mjd (MJD* mjd)
{
    return orbit_julian (JDAY_01012000 ( mjd) );
}

/* calculate (rough) anx time of orbit */
MJD anx_time (int orbit)
{
    double jday;
    double t0 = 824.026091; /* ANX time of ref. orbit */
    double orbit0 = 486.; /* no. of ref. orbit */
    double ref_period = 6035.928; /* (default) orbital period in s */

    jday = (((double) orbit - orbit0) * ref_period)/86400.0 + t0;

    return JDAY_01012000_2_MJD (jday);
}

/* calculate (solar) local time for a date and location in hours  */
double sol_local_time (const char* utc, Coord_deg pos)
{
    int hh,mm,ss;
    double utc_hour;
    double loc_hour;
    hh = atoi (utc+12);
    mm = atoi (utc+15);
    ss = atoi (utc+18);
     utc_hour = hh + mm/60.0 + ss/3600.0;

    loc_hour = utc_hour+pos.lon/360.0*24.0;
    if (loc_hour>=24.0)
	loc_hour-=24.0;
    return loc_hour;
}



/*---------------------------------------------------------------------
 * get current system time in MJD (for filenames/ processing time)
 *---------------------------------------------------------------------*/

/* give back as MJD */
MJD system_mjd (void)
{
				/*  determine processing time*/
    struct tm *gmt=NULL;
    time_t tt;
    /*  determine processing time */
    /* get processing time */
    time (&tt);
    gmt = gmtime ( &tt );
    return JDAY_2_MJD (JULDAY (*gmt));
}

/* write in ascii string as yyyymmdd_hhmmss (needs 16 chars space in systime) */
void system_ymd (char *sys_time)
{
    MJD mjd = system_mjd();
    MJD_2_yyyymmdd (&mjd, sys_time);
    return;
}


/* calculate the mid point between two points (degree in integer and float) */

Coord middle_coord (Coord p1, Coord p2)
{
    Coord result;
    /*  lat is simple */
    result.lat = (p1.lat+p2.lat)/2;
    /*  case 1 for lon */
    if (p1.lon < -90000000 && p2.lon > 90000000)
    {
	p1.lon += 360000000;
	result.lon = (p1.lon+p2.lon)/2;
	if ( result.lon > 180000000  )
	    result.lon -= 360000000;
	return result;
    }
    /*  case 2 for lon */
    if (p2.lon < -90000000 && p1.lon > 90000000 )
    {
	p2.lon += 360000000;
	result.lon = (p1.lon+p2.lon)/2;
	if ( result.lon > 180000000)
	    result.lon -= 360000000;
	return result;
    }
    /*  default */
    result.lon = (p1.lon+p2.lon)/2;
    return result;
}


Coord_deg middle_coord_deg (Coord_deg p1, Coord_deg p2)
{
    Coord_deg result;
    /*  lat is simple */
    result.lat = (p1.lat+p2.lat)/2;
    /*  case 1 for lon */
    if (p1.lon < -90.0 && p2.lon > 90.0)
    {
	p1.lon += 360;
	result.lon = (p1.lon+p2.lon)/2;
	if ( result.lon > 180.0)
	    result.lon -= 360.0;
	return result;
    }
    /*  case 2 for lon */
    if (p2.lon < -90.0 && p1.lon > 90.0)
    {
	p2.lon += 360;
	result.lon = (p1.lon+p2.lon)/2;
	if ( result.lon > 180.0)
	    result.lon -= 360.0;
	return result;
    }
    /*  default */
    result.lon = (p1.lon+p2.lon)/2;
    return result;
}





/*
!------------------------------------------------------------------------
! This subroutines converts the solar zenith and azimuth angles from
! the TOA (i.e., where the limb LOS intersects the 100 km altitude shell
! according to the solar angle definitions in the SCIAMACHY Level1c files
! for L0-1 processor versions starting with vs5.00) to the solar angles
! at the tangent point. The latter are required for the RT calculations
! using SCIARAYS
!
! Input variables:
!   th    : tangent height
!   sza   : solar zenith angle at TOA
!   saa   : solar azimuth angle at TOA
!
! Output variables
!   szaTp : solar zenith angle at TP
!   saaTp : solar azimuth angle at TP
!
! Routine provided by A. Rozanov
! translated to C by K. Bramstedt
!------------------------------------------------------------------------
*/
double DMAX1 (double v1, double v2)
{
    if (v1 < v2)
	return v2;
    else
	return v1;
}

double SIGN (double x, double y)
{
    x = fabs(x);
    if ( y < 0.0 )
	return -x;
    else
	return x;
}

double PI = 3.141592653589793;

double rad(double grad)
{
    return grad * PI / 180.0;
}
double grad(double rad)
{
    return (float) (rad * 180.0 / PI);
}

/* Angle recalculations, based on angles.f in SCIATRAN 2.1
   --Input:
   sza_in, float los_in, float saa_in, float z_in : input angles at altitude z_in
   earth_radius : earth radius :-)
   --Output:
   *sza_out, *los_out, *saa_out,  *z_out : output angles at altitude z_out
   tangent_height  : calculated tangent height for consistency check
*/


int ANGLES (double sza_in, double los_in, double saa_in, double z_in,
	    double *sza_out, double *los_out, double *saa_out, double z_out,
	    double earth_radius,
	    double *tangent_height)

{
    double     COS_PSI_IN, MJU_IN, PHI_IN, COS_PSI_OUT ;
    double     MJU_OUT, PHI_OUT, Z_IN, EARTH_RADIUS, SD;
    double     Z_OUT, R, H_0, DELTA, SIN_1, SIN_PSI, S1 ;
    double     COS_PHI_0, SIN_PHI_0, ETA_0, ZETA_0, KSI_0;
    double     SIN_PSI_OUT, SIN_OUT;
    /*  set input, including grad -> rad */
/*      COS_PSI_IN = ANG(1); MJU_IN = ANG(2); PHI_IN = ANG(3)*/
    /*  calculate backwards along line of sight instead of forward
    if (z_out > z_in)
    saa_in += 180.0;*/

    COS_PSI_IN = cos (sza_in);
    MJU_IN = cos (los_in);
    PHI_IN = saa_in;
    EARTH_RADIUS = (double) earth_radius;
    Z_IN = (double) z_in;
    Z_OUT = (double) z_out;

    /* start original calculation in angles.f */
    R = EARTH_RADIUS + Z_IN;
    H_0 = R*sqrt(1.0 - (MJU_IN*MJU_IN)) - EARTH_RADIUS;

    SIN_1 = sqrt(1.0 - MJU_IN*MJU_IN);
    COS_PHI_0 = cos(PHI_IN);
    SIN_PHI_0 = sin(PHI_IN);

    if (H_0 > Z_OUT)
    {
	Z_OUT = H_0;
/*	H_0 = Z_OUT;
	fprintf(stderr, "ANGLES(): Tangent height is above TOA\n");
	exit (12); */
    }

    DELTA =  sqrt((2.0 *EARTH_RADIUS + Z_IN + H_0)*(Z_IN - H_0))
	- sqrt((2.0 *EARTH_RADIUS + Z_OUT + H_0)*(Z_OUT - H_0));
    MJU_OUT = (MJU_IN*R - DELTA)/
	sqrt((MJU_IN*R - DELTA)*(MJU_IN*R - DELTA) + (R*SIN_1)*(R*SIN_1));
    SIN_OUT = R*SIN_1/(EARTH_RADIUS + Z_OUT);


    SIN_PSI = sqrt(1.0 - COS_PSI_IN*COS_PSI_IN);
    ZETA_0 = MJU_IN*COS_PSI_IN - SIN_1*SIN_PSI*COS_PHI_0;
    KSI_0 = MJU_IN*SIN_PSI + SIN_1*COS_PSI_IN*COS_PHI_0;


    COS_PSI_OUT = (COS_PSI_IN*R - DELTA*ZETA_0)/
	sqrt((COS_PSI_IN*R -
	      DELTA*ZETA_0)*(COS_PSI_IN*R -
	      DELTA*ZETA_0)
	     + (SIN_PSI*R - DELTA*KSI_0)*(SIN_PSI*R - DELTA*KSI_0)
	     + (DELTA*SIN_1*SIN_PHI_0)*(DELTA*SIN_1*SIN_PHI_0));
    SIN_PSI_OUT = sqrt(1.0 - DMAX1(1.0,COS_PSI_OUT*COS_PSI_OUT));

    ETA_0 = ((R - DELTA*MJU_IN)*SIN_PSI*COS_PHI_0 -
	     DELTA*SIN_1*COS_PSI_IN)/(EARTH_RADIUS + Z_OUT);

    ETA_0 = ETA_0 + 1.0e-39;  /*! numerical stabilization*/

    S1 = ETA_0/(sqrt(ETA_0*ETA_0 + (SIN_PSI*SIN_PHI_0)*(SIN_PSI*SIN_PHI_0)));
    S1 = S1 - SIGN(1.0e-13,S1);/* ! numerical stabilization*/

    SD = R*SIN_PSI*SIN_1*SIN_PHI_0/(EARTH_RADIUS + Z_OUT)/
	(SIN_PSI_OUT*SIN_OUT + 1.0e-78);

    PHI_OUT = SIGN(acos(S1),SD);

/*      ANG(1) = COS_PSI_OUT; ANG(2) = MJU_OUT; ANG(3) = PHI_OUT;*/
    /*write results to output*/
    *sza_out = acos(COS_PSI_OUT);
    *los_out = acos(MJU_OUT);
    *saa_out = PHI_OUT;
    /* set back direction of line of sight
    if (z_out > z_in)
    *saa_out -= 180.0; */
    *tangent_height = H_0;
    return 0;
}

/* little helpber to replace space and tab by underscore */
char* str_unspace (const char* str)
{
    int n;
    int len = strlen (str);
    /* allocate memory  */
    char *unspace = malloc ((len+1) * sizeof(char));
    /* return NULL without memory */
    if (unspace == NULL)
	return unspace;
    /* copy string, changing space/tab */
    for (n=0; n<len; n++)
	if (isspace (str[n]))
	    unspace[n] = '_';
	else
	    unspace[n] = str[n];
    /* add 0 as end of string */
    unspace[n] = '\0';
    /* done */
    return unspace;
}

/* little helpber to replace slash by underscore */
char* str_unslash (const char* str)
{
    int n;
    int len = strlen (str);
    /* allocate memory  */
    char *unspace = malloc ((len+1) * sizeof(char));
    /* return NULL without memory */
    if (unspace == NULL)
	return unspace;
    /* copy string, changing space/tab */
    for (n=0; n<len; n++)
	if (str[n] == '/')
	    unspace[n] = '_';
	else
	    unspace[n] = str[n];
    /* add 0 as end of string */
    unspace[n] = '\0';
    /* done */
    return unspace;
}



/* Error message function, used by the macro SCIA_ERROR */
int scia_err (enum err_flag err_flag,
	      const char *source_file, int line, const char* message,
	      const char* arg)
{
    char error_type [][10] = {"ERROR: ", "WARNING: ", " "};
    fprintf (stderr, "%s:%d: %s%s %s\n", source_file, line,
	     error_type[err_flag], message, arg);
    if (err_flag == FATAL)
	exit(255);
    return 0;
}

/* linear interpolation (or extrapolation...)  */
/* input:
   x0,y0, x1,y1   points between linear interpolation is requested
   x, n_x         n_x  values in array x, where to interpolate
   output:
   y              y    interpolated values
*/

int linear_interpol (double *y, double* x, int n_x,
		     double x0, double y0, double x1, double y1)
{
    int n;
    double c = (y1-y0)/(x1-x0);
    for (n=0; n<n_x; n++)
	y[n] = y0 + (x[n]-x0) * c;
    return 0;
}

// NOT USED /* smooth an array with boxcar/triangular function of width box_width */
// NOT USED /*  y, ny : y input array of n_y values, will be
// NOT USED         overwritten by the smoothed values
// NOT USED     box_width: width of the boxcar (odd number);
// NOT USED      smoothing is performed until the edge of the array!
// NOT USED */
// NOT USED
// NOT USED
// NOT USED int smooth (double *y, int n_y, int box_width, enum smooth_type stype)
// NOT USED {
// NOT USED     int n,j,k;
// NOT USED     int half_bw = (box_width - 1)/2;
// NOT USED     double sum;
// NOT USED     int div;
// NOT USED     double *smoothed;
// NOT USED     /* No smoothing with 1 */
// NOT USED     if (box_width == 1)
// NOT USED 	return n_y;
// NOT USED     /* start smoothing */
// NOT USED /*     if (box_width%2 != 1) */
// NOT USED /* 	SCIA_ERROR (FATAL, "box_width has to be odd!", "");     */
// NOT USED     smoothed = calloc (n_y, sizeof(double));
// NOT USED /*     if (!smoothed) */
// NOT USED /* 	SCIA_ERROR (FATAL,"Out of memory.",""); */
// NOT USED
// NOT USED     switch (stype)
// NOT USED     {
// NOT USED 	case SMOOOTH_BOXCAR:
// NOT USED 	    for (n=0; n<n_y; n++)
// NOT USED 	    {
// NOT USED 		sum = 0.0;
// NOT USED 		for (j=n-half_bw; j <= n+half_bw; j++)
// NOT USED 		{
// NOT USED 		    int i = j;
// NOT USED 		    while (i < 0)
// NOT USED 			i++;
// NOT USED 		    while (i >= n_y)
// NOT USED 			i--;
// NOT USED 		    sum += y[i];
// NOT USED 		}
// NOT USED 		smoothed[n] = sum / (double) box_width;
// NOT USED 	    }
// NOT USED 	    break;
// NOT USED 	case SMOOTH_TRIANGULAR:
// NOT USED 	    div = half_bw+1;
// NOT USED 	    div *= div;
// NOT USED 	    for (n=0; n<n_y; n++)
// NOT USED 	    {
// NOT USED 		sum = 0.0;
// NOT USED 		for (j=n-half_bw, k=0; j <= n+half_bw; j++, k++)
// NOT USED 		{
// NOT USED 		    int i = j;
// NOT USED 		    while (i < 0)
// NOT USED 			i++;
// NOT USED 		    while (i >= n_y)
// NOT USED 			i--;
// NOT USED 		    sum += (k>half_bw ? box_width-k : k+1) *y[i];
// NOT USED 		}
// NOT USED 		smoothed[n] = sum / (double) div;
// NOT USED 	    }
// NOT USED 	    break;
// NOT USED #if HAVE_LIBGSL
// NOT USED 	case SMOOTH_MEDIAN:
// NOT USED 	    median_arr = calloc (box_width, sizeof(double));
// NOT USED 	    div = half_bw+1;
// NOT USED 	    div *= div;
// NOT USED 	    for (n=0; n<n_y; n++)
// NOT USED 	    {
// NOT USED 		sum = 0.0;
// NOT USED 		for (j=n-half_bw, k=0; j <= n+half_bw; j++, k++)
// NOT USED 		{
// NOT USED 		    int i = j;
// NOT USED 		    while (i < 0)
// NOT USED 			i++;
// NOT USED 		    while (i >= n_y)
// NOT USED 			i--;
// NOT USED 		    median_arr[k] = y[i];
// NOT USED 		}
// NOT USED 		gsl_sort (median_arr, 1, box_width);
// NOT USED 		smoothed[n] = gsl_stats_median_from_sorted_data (
// NOT USED 		    median_arr, 1, box_width);
// NOT USED 	    }
// NOT USED 	    break;
// NOT USED #endif
// NOT USED 	default:
// NOT USED 	    SCIA_ERROR (FATAL, "Unknown Smoothing type.", "");
// NOT USED 	    break;
// NOT USED     }
// NOT USED     memcpy (y, smoothed , n_y * sizeof(double));
// NOT USED     free (smoothed);
// NOT USED     return n;
// NOT USED }



/* little debug helper functions */
int print_array (double* data, int n_data, char* name)
{
    int n;
    FILE *out;
    out = fopen(name, "w");
    for (n=0; n<n_data; n++)
	fprintf (out, "%g\n", data[n]);
    fclose (out);
    return 1;
}

int print_iarray (int* data, int n_data, char* name)
{
    int n;
    FILE *out;
    out = fopen(name, "w");
    for (n=0; n<n_data; n++)
	fprintf (out, "%d\n", data[n]);
    fclose (out);
    return 1;
}
/* ************************************* */
