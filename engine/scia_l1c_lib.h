#ifndef  __SCIA_L1C_LIB                        /* Avoid multiple includes */
#define  __SCIA_L1C_LIB

#include "scia_defs.h"
#include "scia_l1c.h"

#define SCAN_LIMB_DURATION 24
#define SCAN_LIMB_STEP_DURATION 3

/* MDS types */
/*
typedef enum L1C_MDS_type {
    NADIR       ,
    LIMB        ,
    OCCULTATION ,
    MONITORING  ,
    MAX_MDS_TYPES
} L1C_MDS_type;
*/

/* Reading Type list */
typedef enum L1C_read {
  READ_NADIR=1,
  READ_LIMB,
  READ_OCCULTATION,
  READ_SUN,
  READ_MONITORING
} L1C_read;

/* Solar Measurement types */
/* Fill in all known types - to be extended */
typedef enum solar_type {
  STANDARD=0,
  ESM_DIFF_SUN_REF=0,
  OCC_SUN_REF,
  SUBSOLAR_SUN_REF,
  SUN_D1=0,
  SUN_O =1,
  SUN_S =2,
  SUN_D2=3,
  SUN_D3=4,
  SUN_D4=5,
  SUN_D5=6,
  SUN_D6=7,
  SUN_D7=8,
  SUN_D8=9,
  SUN_D9=10,
} solar_type;

#pragma pack(push,1)

typedef struct User_File_Info
{
    /* MPH */
    char *product ; /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
    char *software_ver ; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
    int abs_orbit; /* Start absolute orbit number               */
    /* SPH */
    char *key_data_version ; /* Key Data version (pattern XX              */
    char *m_factor_version ; /* Key Data version (pattern XX              */
    char *init_version ;     /* Version Initialisation files */
    char *decont ;           /* Decontamination flag */
    /* GADS calibration options */
    char *l1b_product_name; 	/* L1b product name */
    char cal_applied[17];	/* Numbers [0-7] of applied calibrations as string */
} User_File_Info;


typedef struct user_data_pmd
{
    float pmd[7];
} user_data_pmd;


/* structure for data in user format */

typedef struct user_data
{
    unsigned int n_wl;		/* number of spectral points */
    unsigned short *pixel_nr;	/* pixel numbers */
    unsigned short *pixel_ch;	/* channel of pixels */
    unsigned short *pixel_cls;	/* cluster */
    unsigned short *pixel_coadd; /* co-adding, applied by reading routine */
    float *wl;			/* wavelength */
    float *wl_err;		/* wavelength error */
    float *signal;		/* signal science channel */
    float *signal_err;		/* signal error */
    float int_time;		/* integration time (seconds) */
    MJD mjd;			/* Date (Julian days after 1.1.2000 */
    char date[29];		/* Date string */
    L1_MDS_TYPE type;		/* which kind of data (nadir/limb/occ/monit?) */
    GeoN  geo_nadir;		/* Structure for Geolocation */
    GeoL  geo_limb;
    GeoCal geo_cal;
    unsigned int n_pmd;		/*  */
    user_data_pmd *pmd;		/* integrated PMD readouts (if available)*/
				/* orbit*/
    GeoN   *pmd_geo_nadir;	/* Array for PMD - Geolocation */
    GeoL   *pmd_geo_limb;

    int n_readout;              /* Number of readout */
    int n_state;                /* Number of state */
/* State information (same for each readout in state) */
    int state_id;
    int category;
    char state_date[29];
    Coord state_corner[4];
/* information about the product
   (same for each readout)*/
    /* MPH */
    char *product ;             /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
    char *software_ver ; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
    int abs_orbit; /* Start absolute orbit number               */
    /* SPH */
    char *key_data_version ; /* Key Data version (pattern XX              */
    char *m_factor_version ; /* Key Data version (pattern XX              */
    char *init_version ;     /* Version Initialisation files */
    char *decont ;           /* Decontamination flag */
    /* gads calibration options */
    char *l1b_product_name; 	/* L1b product name */
    char *cal_applied;	/* Numbers [0-7] of applied calibrations as string */
} user_data;

/* structure for solar data in user format */

typedef struct user_data_solar
{
    char sun_spect_id[3];	/* String with id of solar spectrum */
    unsigned int n_wl;		/* number of spectral points */
    unsigned short *pixel_nr;	/* pixel numbers */
    unsigned short *pixel_ch;	/* channel of pixels */
    float *wl;			/* wavelength */
    float *signal;		/* signal science channel */
    float *signal_err;		/* signal error */
    MJD mjd;                    /* Date (Julian days after 1.1.2000 */
    char date[28];		/* Date string */
/* information about the product
   (same for each readout)*/
    /* MPH */
    char *product ;             /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
    char *software_ver ; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
    int abs_orbit; /* Start absolute orbit number               */
    /* SPH */
    char *key_data_version ; /* Key Data version (pattern XX              */
    char *m_factor_version ; /* Key Data version (pattern XX              */
    char *init_version ;     /* Version Initialisation files */
    char *decont ;           /* Decontamination flag */
    /* GADS calibration options */
    char *l1b_product_name; 	/* L1b product name */
    char *cal_applied;	/* Numbers [0-7] of applied calibrations as string */
} user_data_solar;




/* Structure holding data of one state  */

typedef struct state_cluster_data
{
    mds_1c_constant mds_const;
    int type;
    short quality;
    int cluster_id;
    unsigned int n_geo;
    unsigned int n_wl;
    unsigned short *pixel; /* pixel_nr[n_wl] */
    float *wl;     /* wl[n_wl] */
    float *wl_err; /* wl_err[n_wl] */
    float *signal;    /* rad[n_geo*n_wl] */
    float *signal_err; /* rad_err[n_geo*n_wl] */
    GeoN  *geo_nadir; /* geo_nadir[n_geo] */
    GeoL  *geo_limb;
    GeoCal *geo_cal;
} state_cluster_data;

/* Sub structure to info_l1c to store info of used cluster */
typedef struct cur_used_cluster
{
    short id;			/* cluster id */
    int coadd;  		/* coadding factor to reach integration time */
				/* of window */
    int pix_start;		/* absolut start pix of cluster (0-8191)*/
    int pix_length;		/*  length of cluster */
    int pix_n;			/*  number of pixels really used */
} cur_used_cluster;

typedef struct _stateClconMeastime
 {
  unsigned short clconMeasTime[MAX_CLUSTER];
 }
STATE_CLCON_MEASTIME;

/* (DSD) info structure */
typedef struct info_l1c
{
  FILE *FILE_l1c;		/* File handler for Lv1C - file */
  MPH mph;
  SPH_SCI_NLC_1C sph;
  User_File_Info user_file_info; /* Store user important information of product */
  int num_dsd;		/* No. of DSDs */
  DSD states;		/* state summary DSD */
  DSD states_geolocation; /* geolocation of states DSD */
  DSD nadir;		/* nadir DSD */
  DSD limb;		/* limb DSD */
  DSD occ;	        /* occultation  DSD */
  DSD mon;	        /* monitoring DSD */
  DSD nadir_pmd;	/* nadir PMD DSD */
  DSD limb_pmd;		/* limb PMD DSD */
  DSD occ_pmd;	        /* occultation PMD DSD */
  DSD nadir_pol;	/* nadir fract. polarisaton DSD */
  DSD limb_pol;		/* limb fract. polarisaton DSD */
  DSD occ_pol;	        /* occultation fract. polarisaton DSD */
  DSD sun_ref;	        /* sun reference DSD */
  DSD specbas;	        /* spectral calibration basis DSD */
  DSD specpar;	        /* spectral calibration paramters DSD */
  DSD cal_options;	/* calibration options DSD */
  /* other DSDs may be added */
  int n_mds[MAX_MDS_TYPES]; /* No. of Measurem. data sets
				 (previous 4 numbers); */
  int n_pmd_mds[MAX_MDS_TYPES]; /* No of PMD datasets for each type; */

  ADS_STATES *ads_states;    /* Complete State info (Clusterdefs!)*/
  STATE_CLCON_MEASTIME *stateClconMeasTime;
  Geolocation *ads_states_geolocation;

  int *idx_states[MAX_MDS_TYPES]; /* index to state in ads_states */
  int n_states[MAX_MDS_TYPES];    /* number of indexes */

  int mds_offset[MAX_MDS_TYPES];     /* actual offset of MDS  */
  int mds_pmd_offset[MAX_MDS_TYPES]; /* actual offset of PMD-DS  */

//    MJD *mjd_mds;
//    int *idx_mds;
//    int *meas_cat;



//    MJD *mjd_nadir_mds; /* Collect starttimes of MDS nadir */
//    int *idx_nadir_mds; /* Index to states of MDS nadir */
//    MJD *mjd_limb_mds; /* Collect starttimes of MDS limb */
//    int *idx_limb_mds; /* Index to states of MDS limb */

  float wl[NPIXEL];  /* Fixed wawelength grid */
  CAL_OPTIONS_GADS cal_options_GADS;   /*Calibration Options GADS*/

  int cluster_ids[MAX_MDS_TYPES][64];	/* List of clusterids in File */
  int max_cluster_ids[MAX_MDS_TYPES];	/* Number of clusterids */

  int cur_cluster_ids[64];	/* List of clusterids in this state */
  int cur_max_cluster_ids;	/* Number of current clusterids */

  int cur_state_nr[MAX_MDS_TYPES]; /* index to current state */

  int cur_mds_read[MAX_MDS_TYPES]; /* counter for MDS already read */

  int cur_readout_in_state;	/* Actual readout for window in state */
  int cur_max_readout_in_state; /* Max readouts in state */
  int cur_pix_start;		/* Actual pix_window */
  int cur_pix_end;		/* Actual pix_window */
  int cur_pix_start_arr[64];	/* Actual pix_windows */
  int cur_pix_end_arr[64];	/* Actual pix_windows */
  int n_cur_pix;		/* number of pix_windows */

  float cur_wl_start;		/* Actual wl_window */
  float cur_wl_end;		/* Actual wl_window */
  float cur_wl_start_arr[64];	/* Actual wl_windows */
  float cur_wl_end_arr[64];	/* Actual wl_windows */
  float cur_wl_channel_arr[64];	/* Actual wl_windows */
  int n_cur_wl;		/* number of wl_windows */
				/* flag for real output */
  int cur_pix_output_flag[MAX_PIXELS];

  int wanted_clusters[64];	/* User may define cluster only */
  int n_wanted_clusters;	/* Number of user defined clusters */

  int set_int_time;		/* wanted integration time for larger pixels */
  int max_int_time;           /* largest integration time in cluster set */

//    short cur_used_clusters[64];/* List of really used clusters in current state */
				/* actual cluster data */
				/* coadding factor to reach integration time */
				/* of window */
//    int coadd_cur_used_clusters[64];
				/*  absolut start pix of cluster (0-8191)*/
//    int pix_start_cur_used_clusters[64];
				/*  length of cluster */
//    int pix_end_cur_used_clusters[64];
				/*  number of pixels really used */
//    int pix_n_cur_used_clusters[64];
				/*  list of really used clusters in current state*/
  cur_used_cluster cur_used_cl[64];
				/*  total number of really used pixels */
  int sum_pix_cur_used_clusters;
  int geo_cur_used_clusters;	/* geolocation to use for output */
  int max_cur_used_clusters;  /* number of used clusters */
  int cur_it;			/* Integration time currently used */
  int cur_num_pmd;
				/* Data in one state */
  state_cluster_data st_cl_data[64];
  Nadir_Pmd nadir_pmd_data;
    /* flag for recaculating limb TH */
  int cfi_limb_flag;
} info_l1c;

#pragma pack(pop)

/*************************************************************
 * User routines
 *************************************************************/


				/* Open L1c, specify what to read */
SCIA_err openL1c (char* FILE_name, info_l1c *info);

				/* Close L1c */
SCIA_err closeL1c (info_l1c *info);


				/* Read Solar spectrum */
SCIA_err read_solar (info_l1c *info,
		      char *sun_spec_id,
		      user_data_solar *ud);
				/* Read nadir measurements one by one */
SCIA_err read_next_mds (info_l1c *info,
			 user_data *ud, L1_MDS_TYPE type);

				/* Select wavelength window */
SCIA_err set_wl_window (info_l1c *info, float wl_start, float wl_end,
			 int channel);
				/* Select multiple wavelength windows */
SCIA_err set_wl_window_multi (info_l1c *info, float* wl_start, float* wl_end,
			       int *channel, int n_window);
			  /* Select wavelength window (with pixels) */
SCIA_err set_pix_window (info_l1c *info, int pix_start, int pix_end);

                           /* Select clusters for output */
SCIA_err set_clusters (info_l1c *info, int* cluster_id, int n_cluster);

				/* select additional co-adding */
SCIA_err set_int_time (info_l1c *info, int set_int_time);
				/* skip further reading of current state*/
				/* next call to read_next_mds will start with */
				/*  start with next state */
SCIA_err skip_cur_state (info_l1c *info, L1_MDS_TYPE type);


/* free allocated memory */
SCIA_err free_user_data (user_data *ud);
SCIA_err free_user_data_solar (user_data_solar *ud);


/* make pixel id ranges from wavelengt windows */

SCIA_err get_pixel_range_wl (info_l1c *info, int nwl_window,
			      float *wlstart, float *wlend ,
			      int *idx0, int *idx1);

/* new routine for defining wl-window */
SCIA_err set_wl_window_multi_new (info_l1c *info,
				   float* wl_start, float* wl_end,
				   int* wl_clus,
				   int n_window);
/*************************************************************
 * Internal routines
 ************************************************************/

/* rea solar spectrum */
SCIA_err read_solar_complete (info_l1c *info,
			       solar_type sol,
			       float *wl[],
			       float *irr[],
			       float *irr_err[]);




SCIA_err ADS_state (info_l1c *info);

void ReadClusterInfo(ADS_STATES * state);

SCIA_err Fixed_Wavelength_grid (info_l1c *info);

SCIA_err Read_cal_options_GADS (info_l1c *info);

SCIA_err calc_cluster_window (info_l1c *info, L1_MDS_TYPE type);

SCIA_err coadd_signal (unsigned int n_wl, unsigned int n_coadd,
			float *signal,           /* Input: rad[n_coadd*n_wl] */
			float *signal_err,       /* Input: rad_err[n_coadd*n_wl] */
			float *add_signal,       /* Output: rad[n_wl] */
			float *add_signal_err);   /* Output: rad_err[n_wl] */


/* Read MPH. SPH, DSD */

//SCIA_err Read_MPH (FILE* unit, MPH *mph);
//SCIA_err Read_SPH (FILE* unit, SPH *sph);
//SCIA_err Read_DSD (FILE* unit, DSD *dsd);


#endif
