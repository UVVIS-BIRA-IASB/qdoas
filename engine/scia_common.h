#ifndef  __SCIA_COMMON                       /* Avoid multiple includes */
#define  __SCIA_COMMON

#include "scia_defs.h"

BEGIN_C_DECLS

typedef enum  SCIA_PRODUCT {SCIA_L2N,
			    SCIA_L2O,
			    SCIA_L2M,
			    SCIA_L1B,
			    SCIA_L1C,
			    MIPAS_L2,
			    GOME_L2,
			    SCIA_MAX_PRODUCT
} SCIA_PRODUCT;


extern char SCIA_PRODUCT_ACR [SCIA_MAX_PRODUCT][12];

#pragma pack(push,1)


typedef struct MPH {
        /*  Product File name */
    char product[63];
        /*  Processing Stage FlagN = Near Real Time, T = test product, V= fully
            validated (fully consolidated) product, S = special product.
            Letters between N and V (with the exception of T and S) indicate
            steps in the consolidation process, with letters closer to V */
    char proc_stage[2];
        /*  Reference Document Describing Product AA-BB-CCC-DD-EEEE_V/I&Oslash;&Oslash;
            (23 characters, including blank space characters) where AA-BB-CCC-DD-EEEE
            is the ESA standard document no. and V/I is the Version / Issue.
            If the reference document is the Products Specifications PO-RS-MDA-GS-2009,
            the version and revision have to refer to the volume 1 of the document,
            where the status (version/revision) of all volumes can be found.
            If not used, set to _______________________ */
    char ref_doc[24];
        /*  Spare */
    char spare_1[40];
        /*  Acquisition Station ID (up to 3 codes) If not used, set to ____________________ */
    char acquisition_station[21];
        /*  Processing Center ID which generated current product If not
            used, set to ______ */
    char proc_center[7];
        /*  UTC Time of Processing (product generation time)UTC Time format.
            If not used, set to ___________________________. */
    char proc_time[28];
        /*  Software Version number of processing softwareFormat: Name
            of processor (up to 10 characters)/ version number (4 characters)
            -- left justified (any blanks added at end). If not used, set to
            ______________.e.g.
            MIPAS/2.31____ */
    char software_ver[15];
        /*  Spare */
    char spare_2[40];
        /*  UTC start time of data sensing (first measurement in first data
            record) UTC Time format. If not used, set to
	    ___________________________. */
    char sensing_start[28];
        /*  UTC stop time of data sensing (last measurements last data record)
            UTC Time format. If not used, set to
	    ___________________________. */
    char sensing_stop[28];
        /*  Spare */
    char spare_3[40];
        /*  Phasephase letter. If not used, set to X. */
    char phase[2];
        /*  CycleCycle number. If not used, set to +000. */
    char cycle[5];
        /*  Start relative orbit number If not used, set to +00000 */
    char rel_orbit[7];
        /*  Start absolute orbit number.If not used, set to +00000. */
    char abs_orbit[7];
        /*  UTC of ENVISAT state vector. UTC time format. If not used, set
            to ___________________________. */
    char state_vector_time[28];
        /*  DUT1=UT1-UTC. If not used, set to +.000000. */
    char delta_ut1[9];
        /*  X Position in Earth-Fixed reference. If not used, set to +0000000.000. */
    char x_position[13];
        /*  Y Position in Earth-Fixed reference. If not used, set to +0000000.000. */
    char y_position[13];
        /*  Z Position in Earth-Fixed reference. If not used, set to +0000000.000. */
    char z_position[13];
        /*  X velocity in Earth fixed reference. If not used, set to +0000.000000. */
    char x_velocity[13];
        /*  Y velocity in Earth fixed reference. If not used, set to +0000.000000. */
    char y_velocity[13];
        /*  Z velocity in Earth fixed reference. If not used, set to +0000.000000. */
    char z_velocity[13];
        /*  Source of Orbit Vectors */
    char vector_source[3];
        /*  Spare */
    char spare_4[40];
        /*  UTC time corresponding to SBT below(currently defined to be
            given at the time of the ascending node state vector). If not used,
            set to ___________________________. */
    char utc_sbt_time[28];
        /*  Satellite Binary Time (SBT) 32bit integer time of satellite
            clock. Its value is unsigned (=&gt;0). If not used, set to +0000000000. */
    char sat_binary_time[12];
        /*  Clock Step Sizeclock step in picoseconds. Its value is unsigned
            (=&gt;0). If not used, set to +0000000000. */
    char clock_step[12];
        /*  Spare */
    char spare_5[32];
        /*  UTC time of the occurrence of the Leap SecondSet to
	    ___________________________
            if not used. */
    char leap_utc[28];
        /*  Leap second sign(+001 if positive Leap Second, -001 if negative)Set
            to +000 if not used. */
    char leap_sign[5];
        /*  Leap second errorif leap second occurs within processing segment
            = 1, otherwise = 0If not used, set to 0. */
    char leap_err[2];
        /*  Spare */
    char spare_6[40];
        /*  1 or 0. If 1, errors have been reported in the product. User should
            then refer to the SPH or Summary Quality ADS of the product for
            details of the error condition. If not used, set to 0. */
    char product_err[2];
        /*  Total Size Of Product (# bytes DSR + SPH+ MPH) */
    char tot_size[22];
        /*  Length Of SPH(# bytes in SPH) */
    char sph_size[12];
        /*  Number of DSDs(# DSDs) */
    char num_dsd[12];
        /*  Length of Each DSD(# bytes for each DSD, all DSDs shall have the
            same length) */
    char dsd_size[12];
        /*  Number of DSs attached(not all DSDs have a DS attached) */
    char num_data_sets[12];
        /*  Spare */
    char spare_7[40];
} MPH ;


/*****************************************/
/* DSD : Dataset Descriptors */

typedef struct DSD
{
    char name[29];
    char type;
    char filename[63];
    unsigned int offset;
    unsigned int size;
    unsigned int num_dsr;
    int dsr_size;
    char spare [32];
}
DSD;



/*
 * compound data types; adapted from NADC lib
 */

typedef struct MJD
{
  int days;
  unsigned int secnd;
  unsigned int musec;
}
MJD;

typedef struct Coord
{
  int lat;
  int lon;
}
Coord;

/* Coord with deg values */
typedef struct Coord_deg
{
  float lat;
  float lon;
}
Coord_deg;

typedef struct Geolocation {
        /*  Start time of the scan phase of the state */
    MJD dsr_time;
        /*  Attachment Flag (set to 1 if all MDSRs corresponding to this ADSR
            are blank, set to zero otherwise) */
    char attach_flag;
        /*  4 geolocation co-ordinates */
    Coord coord_grd[4];
} Geolocation;

typedef struct GeoL
{
  float esm_pos;
  float asm_pos;
  float sza_toa[3];
  float saa_toa[3];
  float los_zen[3];
  float los_azi[3];
  float sat_height;
  float earth_radius;
  Coord sub_sat;
  Coord tangent_ground_point[3];
  float tangent_height[3];
  float doppler_shift;
}
GeoL;

typedef struct GeoN
{
  float esm_pos;
  float sza_toa[3];
  float saa_toa[3];
  float los_zen[3];
  float los_azi[3];
  float sat_height;
  float earth_radius;
  Coord sub_sat;
  Coord corner_coord[4];
  Coord centre_coord ;
}
GeoN;

typedef struct GeoCal
{
  float esm_pos;
  float asm_pos;
  float sza;
  Coord sub_sat;
}
GeoCal;


/* Geolocation Nadir as common set */

#pragma pack(pop)

/* Reading routines for the above structures */
/* Defined in scia_common.c */

void MJD_getbin (FILE* unit, MJD *var);
void MJD_array_getbin (FILE* unit, MJD *var, int nr);

void GeoN_getbin (FILE* unit, GeoN *var);
void GeoN_array_getbin (FILE* unit, GeoN *var, int nr);

void GeoL_getbin (FILE* unit, GeoL *var);
void GeoL_array_getbin (FILE* unit, GeoL *var, int nr);

void GeoCal_getbin (FILE* unit, GeoCal *var);
void GeoCal_array_getbin (FILE* unit, GeoCal *var, int nr);

void Coord_getbin (FILE* unit, Coord *var);
void Coord_array_getbin (FILE* unit, Coord *var, int nr);

void Coordf_getbin (FILE* unit, Coord_deg *var);
void Coordf_array_getbin (FILE* unit, Coord_deg *var, int nr);

void Coord_deg_getbin (FILE* unit, Coord_deg *var);
void Coord_deg_array_getbin (FILE* unit, Coord_deg *var, int nr);

/* Writing routines for the above structures */
/* defined in scia_common_write.c */

void MJD_putbin (FILE* unit, MJD *var);
void MJD_array_putbin (FILE* unit, MJD *var, int nr);

void GeoN_putbin (FILE* unit, GeoN *var);
void GeoN_array_putbin (FILE* unit, GeoN *var, int nr);

void GeoL_putbin (FILE* unit, GeoL *var);
void GeoL_array_putbin (FILE* unit, GeoL *var, int nr);

void GeoCal_putbin (FILE* unit, GeoCal *var);
void GeoCal_array_putbin (FILE* unit, GeoCal *var, int nr);

void Coord_putbin (FILE* unit, Coord *var);
void Coord_array_putbin (FILE* unit, Coord *var, int nr);

void Coord_deg_putbin (FILE* unit, Coord_deg *var);
void Coord_deg_array_putbin (FILE* unit, Coord_deg *var, int nr);


/* Reading the main product header MPH */
/* This is ASCII Format, therefore not bin... but fscanf is used */

SCIA_err Read_MPH (FILE* unit, MPH *mph);
SCIA_err Write_MPH (FILE* unit, MPH *mph);

SCIA_PRODUCT scia_product (MPH *mph);

SCIA_err Read_DSD (FILE* unit, DSD *dsd);
SCIA_err Write_DSD (FILE* unit, DSD *dsd);

SCIA_err set_DSD_offset (FILE* unit, DSD *dsd);

/*********************************************************************\
 * Compare two dates MJD
 * MJD compare : 0 equal  -1 earlier  1 later
 * kb 23.04.01
\*********************************************************************/
/* Standard C compare routine for MJDs */
int MJD_compare (const MJD *const mjd1, const MJD *const mjd2);

/**********************************************************************
 * add 1/16 seconds / seconds / days to MJD
 **********************************************************************/
MJD MJD_add (MJD mjd, int sec_16);
MJD MJDadd_sec (MJD mjd, double sec);
MJD MJDadd_day (MJD mjd, double day);

/* Calculate mjd2 - mjd1 in seconds */
double MJD_diff (MJD *mjd1, MJD *mjd2);

/* declaration for subroutine UTC_string */
void UTC_String(MJD* time, char* string);
/*    with upper-case month */
void UTC_STRING(MJD* time, char* string);

/* calculate julian day from time struct */
double JULDAY( struct tm myclock );

int yyyymmdd_2_MJD( const char yyyymmdd[],
		    MJD* mjd);

/* calc frac days after 1/1/2002 */
double JDAY_01012000 ( MJD* time );

double JDAY_01012000_ymd ( const char yyyymmdd[] );
/* create UTC String as  yyyymmdd_hhmmss  from UTC_string */
void yyyymmdd_string(char* utc, char* yyyymmdd);

/* create UTC String as  yyyymmdd_hhmmss  from UTC_string
   and catch numbers for data/time parts */

void yyyymmdd_string_num (char* utc, char* yyyymmdd,
			  int *yyyy, int *mm, int *dd, int *hh, int *min, int *ss,
			  int *ms);
/* convert date string from yyyy-mm-dd hh:mm:ss.xx to
   Envisat format  25-SEP-2006 02:32:39.880363 */
void alpha_month (const char* yyyy_mm_dd, char *dd_mmm_yyyy);
/* create yyyymmdd_hhmmss  from MJD */
void MJD_2_yyyymmdd (MJD* time, char* yyyymmdd);
/* ----------------------------------------------------------------- *\
 * calculate orbit from time                                         *
 * input : julian day since 1.1.2000                                 *
 * output : corresponding orbit number                               *
 * ----------------------------------------------------------------- */
int orbit_julian (double jday);
int orbit_mjd (MJD* mjd);

/* translate  'julian day since 1.1.2000'  to MJD */
MJD JDAY_01012000_2_MJD (double jday);
/* translate  Julian Day to MJD */
MJD JDAY_2_MJD (double jday);

/* calculate (rough) anx time of orbit */
MJD anx_time (int orbit);

/* calculate (solar) local time for a date and location in hours  */
double sol_local_time (const char* utc, Coord_deg pos);

/*---------------------------------------------------------------------
 * get current system time in MJD (for filenames/ processing time)
 *---------------------------------------------------------------------*/

/* give back as MJD */
MJD system_mjd (void);

/* write in ascii string as yyyymmdd_hhmmss (needs 16 chars space in systime) */
void system_ymd (char *sys_time);

/* calculate the mid point between two coordinates */
Coord middle_coord (Coord p1, Coord p2);

Coord_deg middle_coord_deg (Coord_deg p1, Coord_deg p2);


/* Angle recalculations, based on angles.f in SCIATRAN 2.1
   --Input:
   sza_in, float los_in, float saa_in, float z_in : input angles at altitude z_in
   earth_radius : earth radius :-)
   --Output:
   *sza_out, *los_out, *saa_out,  *z_out : output angles at altitude z_out
   tangent_height  : calculated tangent height for consistency check
*/

double rad(double grad);
double grad(double rad);


int ANGLES (double sza_in, double los_in, double saa_in, double z_in,
	    double *sza_out, double *los_out, double *saa_out, double z_out,
	    double earth_radius,
	    double *tangent_height);


/* little helpber to replace space and tab by underscore */
char* str_unspace (const char* str);
/* little helpber to replace slash by underscore */
char* str_unslash (const char* str);


/* linear interpolation (or extrapolation...)  */
/* input:
   x0,y0, x1,y1   points between linear interpolation is requested
   x, n_x         n_x  values in array x, where to interpolate
   output:
   y              y    interpolated values
*/
int linear_interpol (double *y, double* x, int n_x,
		     double x0, double y0, double x1, double y1);

/* smooth an array with boxcar/triangular of width box_width */
/*  y, ny : y input array of n_y values, will be
        overwritten by the smoothed values
    box_width: width of the boxcar (odd number);
     smoothing is performed until the edge of the array!
*/
enum smooth_type {SMOOOTH_BOXCAR, SMOOTH_TRIANGULAR, SMOOTH_MEDIAN};

int smooth (double *y, int n_y, int box_width, enum smooth_type stype);

/* little debug helper functions */
int print_array (double* data, int n_data, char* name);

int print_iarray (int* data, int n_data, char* name);

END_C_DECLS
#endif /* __SCIA_COMMON */


