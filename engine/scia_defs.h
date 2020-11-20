#ifndef  __SCIA_DEFS                        /* Avoid multiple includes */
#define  __SCIA_DEFS

/* include all standard libraries here to make central check of
 include files from autoconf

*/
#if HAVE_CONFIG_H
#include <config.h>
#else
/* Preset usefull values if no configure environment is available (WINDOWS)*/
#define HAVE_STDINT_H 0
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define PACKAGE "scial2_ascii"
#define PACKAGE_BUGREPORT "klaus.bramstedt@iup.physik.uni-bremen.de"
#define PACKAGE_NAME "sciaL2_ascii"
#define PACKAGE_VERSION "0.98.2"
#define PACKAGE_STRING "sciaL2_ascii 0.98.2"
/* Define to 1 if your processor stores words with the most significant byte
   last, like Intel and Alpha. */
#define __LITTLE_ENDIAN__ 1

#endif

#define __LITTLE_ENDIAN__ 1
#define HAVE_STDLIB_H 1

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#if HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#if  HAVE_UNISTD_H
#include <unistd.h>
#endif

// !!! #if !HAVE_GETOPT
// !!! #include "getopt.h"
// !!! #endif
// !!!
// !!! #if !HAVE_GETDELIM
// !!! #include "getdelim.h"
// !!! #endif
// !!!
// !!! #if !HAVE_GETLINE
// !!! #include "getline.h"
// !!! #endif
// !!!
// !!! #if !HAVE_SCANDIR
// !!! #include "scandir.h"
// !!! #endif


/* Get size_t, FILE, ssize_t.  And getline, if available.  */
# include <stddef.h>
# include <stdio.h>
# include <sys/types.h>

// !!! #if !HAVE_DECL_GETDELIM
// !!! ssize_t getdelim (char **lineptr, size_t *n, int delimiter, FILE *stream);
// !!! #endif /* !HAVE_GETDELIM */
// !!! #if !HAVE_DECL_GETLINE
// !!! ssize_t getline (char **lineptr, size_t *n, FILE *stream);
// !!! #endif /* !HAVE_GETLINE */


#include <ctype.h>
#include <math.h>

/* make it possible to use this stuff in C++ */

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */



/**** some definitions adapted from NADC library ****/
#define GOME_DATE_LENGTH           25
#define SCIA_DATE_LENGTH           28

#define MAX_UTC_STRING     28
#define MAX_L2N_PROD_STR   8

#define PDS_MPH_LENGTH        1247
#define PDS_DSD_LENGTH         280

#define NUM_COORDS              4

#define MAX_CLUSTER            64

#define NPARAM 5
#define NCHANNEL 8
#define NPIXEL 8192
#define MAX_PIXELS 8192
#define NPMD 7
#define MAX_DETECTOR_PIXELS  1024
#define MAX_CHANNELS         8
				/* BCPS per second, float for calcul. */
#define BCPS_FACTOR          16.0
				/* BCPS per second, integer for counter etc. */
#define BCPS_PER_SECOND      16


				/* SOME limb numbers */
				/* limb levels */
#define MAX_LIMB_LEVELS      35
#define LIMB_LEVELS          35
                                /* Duration of a LIMB state in s */
#define LIMB_DURATION_SEC 59.0625
				/* duration one scan 24BCPS = 1.5s  */
#define BCPS_PER_LIMBSCAN    24
				/* duration scan to scan 24+3 BCPS */
#define BCPS_LIMBSCAN_TO_LIMBSCAN 27
				/* Factors for converting integer angles */
				/*  etc. to real angles */
#define ANGLE_FAC 1e-6
#define HEIGHT_FAC 1e-5
#define INT_TIME_FAC 16

#define DU_FAC 3.719385638E-17  	/* = 1 / 2.69e16 */
				/* Some time factors */
#define SECOND_PER_HOUR 3600
#define SECOND_PER_DAY 86400
#define MIKROSEC_PER_SEC 1000000

				/* Some maximum sizes in variable DSR */
#define DOAS_BIAS_MAX_FIT_PAR   15
#define DOAS_BIAS_MAX_CROSS_COR 105 /* (15*(15-1))/2 */
#define CLOUDS_AEROSOL_L2N_MAX_PMD 32

BEGIN_C_DECLS

enum err_flag {FATAL, WARNING, INFO};

#define SCIA_ERROR( stop_flag, mess, arg ) \
   scia_err ( stop_flag, __FILE__, __LINE__, mess, arg );

/* simple error routine */
int scia_err (enum err_flag err_flag,
	      const char *source_file, int line, const char* message,
	      const char* arg);

				/* Error codes, used in the various routines of the
				   SCIA library */
typedef enum SCIA_err {
    OK,		/* No error */
    SCIA_ERROR,	/* unspecified error */
    MPH_ERROR, /* Error in reading MPH (wrong product?) */
    SPH_ERROR, /* Error in reading SPH (wrong product?) */
    DSD_ERROR, /* Error in reading DSDs (Format change??) */
    FILE_NOT_FOUND,	/* File cannot be opened */
    FILE_NOT_CLOSED,	/* File nannot be closed
			   (not opened?) */
    SCIA_DSD_NOT_ATTACHED,	/* DSD not attached */
    OUT_OF_MEMORY,		/* Allocation operation failed */
    /* Level 2 NRT/OL errorcodes */
    SCIA_BAD_SPECIFICATION,	/* Invalid DOAS/BIAS request */
    SCIA_MDS_EMPTY,		/* No data for requested MDS */
    SCIA_LIMB_MDS_EMPTY,        /* Limb MDS empty */
    SCIA_END_OF_MDS,		/* No more MDS available */
    SCIA_GEO_MISMATCH,		/* Geolocation and MDS do not fit */
    SCIA_PRODUCT_INCONSISTENCY, /* Something is very wrong ... */
				/* ... Geo-coadd something is not impl. yet */
    SCIA_GEO_INTEGR_TIME_NOT_IMPLEMENTED,

    SCIA_PRODUCTNAME_MISMATCH,	/* Name for molecule used in DSD and fitwindow
				   string does not match */
				/*  for ...next... functions*/
    /* Level 1c errorcodes */
    END_OF_DATA, /* Behind last data */
    END_OF_NADIR,	/* Behind last nadir data */
    END_OF_LIMB, 	/* Behind last limb data */
    END_OF_OCCULTATION,/* Behind last occultation data */
    CLUSTER_WAVELENGTH_MISMATCH, /* Available cluster not in
				    wavelength window */
    NO_CLUSTER_MATCH_IN_STATE,
    NO_SCIADATA,
    NO_NADIR,
    NO_LIMB,
    NO_OCC,
    NO_MON,
    NO_SUN_REF,
    NO_NADIR_DATA,	/*  */
    NO_PMD_DATA,	/*  */
    NO_LIMB_DATA,	/*  */
    NO_OCC_DATA,	/*  */
    NO_MON_DATA,	/*  */
    NO_SUN_REF_DATA,	/*  */
    NO_ADS_STATES, /* ADS STATES not found */
    NO_FIXED_WAVELENGTH, /* */
    NO_GADS_CAL_OPTIONS, /* Error in open
			    GADS_CAL_OPTIONS */
    END_OF_NADIR_STATES, /* No more Nadir states */
    END_OF_STATES, /* No more Nadir states */
    INCONSISTENT_STATE,  /* Inconsistency in state */
    WL_OUT_OF_RANGE,     /* bad user wavelength */
    NO_L1C_FILE,         /* not a l1c product */
    /* Level 1B errorcodes */
    NO_L1B_FILE,         /* not a l1b product */
    ADS_READ_ERROR,		/* Error in Reading ADS/GADS */
    WRITE_1B_DSR_GADS_INCONSISTENT, /* */
    SCIA_GOME_EXTRA_PRODUCT 	/* New molecules in GOME */
} SCIA_err;

typedef int L1_MDS_TYPE;

enum MDS_TYPE_ENUM {
    MDS_NADIR,
    MDS_LIMB,
    MDS_OCC,
    MDS_MON,
    MAX_MDS_TYPES
};




END_C_DECLS

#endif /* END __SCIA_DEFS */
