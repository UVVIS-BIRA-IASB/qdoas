#ifndef  __SCIA_L1C                       /* Avoid multiple includes */
#define  __SCIA_L1C

#include "scia_defs.h"
#include "scia_common.h"

/* DSD name enum */
    extern char DS_NAME_SCI_NLC_1C [40][29];

typedef enum DSD_SCI_NLC_1C
{
    SCI_NLC_1C_SUMMARY_QUALITY,
    SCI_NLC_1C_GEOLOCATION,
    SCI_NLC_1C_INSTRUMENT_PARAMS,
    SCI_NLC_1C_LEAKAGE_CONSTANT,
    SCI_NLC_1C_LEAKAGE_VARIABLE,
    SCI_NLC_1C_PPG_ETALON,
    SCI_NLC_1C_SPECTRAL_BASE,
    SCI_NLC_1C_SPECTRAL_CALIBRATION,
    SCI_NLC_1C_SUN_REFERENCE,
    SCI_NLC_1C_POL_SENS_NADIR,
    SCI_NLC_1C_POL_SENS_LIMB,
    SCI_NLC_1C_POL_SENS_OCC,
    SCI_NLC_1C_RAD_SENS_NADIR,
    SCI_NLC_1C_RAD_SENS_LIMB,
    SCI_NLC_1C_RAD_SENS_OCC,
    SCI_NLC_1C_ERRORS_ON_KEY_DATA,
    SCI_NLC_1C_SLIT_FUNCTION,
    SCI_NLC_1C_SMALL_AP_SLIT_FUNCTION,
    SCI_NLC_1C_STATES,
    SCI_NLC_1C_CAL_OPTIONS,
    SCI_NLC_1C_NADIR,
    SCI_NLC_1C_NADIR_PMD,
    SCI_NLC_1C_NADIR_FRAC_POL,
    SCI_NLC_1C_LIMB,
    SCI_NLC_1C_LIMB_PMD,
    SCI_NLC_1C_LIMB_FRAC_POL,
    SCI_NLC_1C_OCCULTATION,
    SCI_NLC_1C_OCCULTATION_PMD,
    SCI_NLC_1C_OCCULTATION_FRAC_POL,
    SCI_NLC_1C_MONITORING,
    SCI_NLC_1C_LEVEL_0_PRODUCT,
    SCI_NLC_1C_LEAKAGE_FILE,
    SCI_NLC_1C_PPG_ETALON_FILE,
    SCI_NLC_1C_SPECTRAL_FILE,
    SCI_NLC_1C_SUN_REF_FILE,
    SCI_NLC_1C_KEY_DATA_FILE,
    SCI_NLC_1C_M_FACTOR_FILE,
    SCI_NLC_1C_INIT_FILE,
    SCI_NLC_1C_ORBIT_FILE,
    SCI_NLC_1C_ATTITUDE_FILE,
    MAX_DS_NAME_SCI_NLC_1C
} DSD_SCI_NLC_1C;

#pragma pack(push,1)

typedef struct SPH_SCI_NLC_1C {
        /*  SPH descriptor */
    char sph_descriptor[29];
        /*  Value: +000 = No stripline continuity, the product is a complete
            segment Other: Stripline Counter */
    char stripline_continuity_indicator[5];
        /*  Value: +001 to NUM_SLICES Default value if no stripline continuity
            = +001 */
    char slice_position[5];
        /*  Number of slices in this stripline Default value if no continuity
            = +001 */
    char num_slices[5];
        /*  Start time of the measurement data in this product. UTC time of
            first MDSR */
    char start_time[28];
        /*  Time of the end of the measurement data in this product (last MDSR
            time). UTC time of last MDSR */
    char stop_time[28];
        /*  Latitude of the satellite nadir at the START_TIME. WGS84 latitude,
            positive = North */
    char start_lat[12];
        /*  Longitude of the satellite nadir at the START_TIME. WGS84 longitude,
            positive = East, 0 = Greenwich */
    char start_long[12];
        /*  Latitude of the satellite nadir at the STOP_TIME. WGS84 latitude,
            positive = North */
    char stop_lat[12];
        /*  Longitude of the satellite nadir at the STOP_TIME. WGS84 longitude,
            positive = East, 0 = Greenwich */
    char stop_long[12];
        /* Since Version 6, spare is replaced bin init_version, decont flag */
    char init_version[5]; /* Version init files (Version 6 onwards)           */
    char decont[9];			/* Decontamination flag */
        /*  Spare (no longer used since Version 6) */
    char spare_1[50];
        /*  Key Data version (pattern XX.XX, e.g. 01.25) */
    char key_data_version[6];
        /*  Key Data version (pattern XX.XX, e.g. 01.25) */
    char m_factor_version[6];
        /*  Range of spectral calibration error ??(summary): GOOD if ? &lt;=
            0.02FAIR if 0.02 &lt; ? &lt;= 0.05 BAD&Oslash; if ? &gt; 0.05 */
    char spectral_cal_check_sum[5];
        /*  Number of saturated detector pixels n (summary): GOOD if n = 0
            FAIR if 0 &lt; n &lt;= 100 BAD&Oslash; if n &gt; 100 */
    char saturated_pixel[5];
        /*  Number of dead detector pixels n (summary): GOOD if n = 0 FAIR if
            0 &lt; n &lt;= 10 BAD&Oslash; if n &gt; 10 */
    char dead_pixel[5];
        /*  Difference between measurement and calibrated dark signal
            (summary): */
    char dark_check_sum[5];
        /*  Number of Nadir states where MDSRs have been stored, example:
            +025 */
    char no_of_nadir_states[5];
        /*  Number of Limb states where MDSRs have been stored, example:
            +025 */
    char no_of_limb_states[5];
        /*  Number of Occultation states where MDSRs have been stored, example:
            +025 */
    char no_of_occultation_states[5];
        /*  Number of WLS, SLS, and Sun diffuser states where MDSRs have been
            stored and the corresponding processing has been completed
            (i.e. 'newly calculated' ADSRs stored), plus the Number of Monitoring
            states where MDSRs have been stored, example: +025 */
    char no_of_moni_states[5];
        /*  Number of states present in the processing time window applied
            to the level-0 product but not counted in other fields counting
            different types of states. Exemple: +025 */
    char no_of_noproc_states[5];
        /*  Number of processed complete dark states */
    char comp_dark_states[5];
        /*  Number of incomplete dark states */
    char incomp_dark_states[5];
        /*  Spare */
    char spare_2[4];
} SPH_SCI_NLC_1C ;




typedef struct Cal_Options {
        /*  Level 1b Product name */
    char l1b_product_name[62];
        /*  Geolocation Filter Flag (-1 filter applied, 0 not used) */
    char geo_filter_flag;
        /*  Start Latitude */
    int start_lat;
        /*  Start Longitude */
    int start_lon;
        /*  End Latitude */
    int end_lat;
        /*  End Longitude */
    int end_lon;
        /*  Time Filter Flag (-1 filter applied, 0 not used) */
    char time_filter_flag;
        /*  Filter Start time */
    MJD start_time;
        /*  Filter Stop time */
    MJD stop_time;
        /*  Measurement Category Filter Flag (-1 filter applied, 0 not used) */
    char category_filter_flag;
        /*  Selected Measurment Category */
    unsigned short category[5];
        /*  Process Nadir MDS Flag (-1 MDS created, 0 not written) */
    char nadir_mds_flag;
        /*  Process Limb MDS Flag (-1 MDS created, 0 not written) */
    char limb_mds_flag;
        /*  Process Occultation MDS Flag (-1 MDS created, 0 not written) */
    char occ_mds_flag;
        /*  Process Monitoring MDS Flag (-1 MDS created, 0 not written) */
    char mon_mds_flag;
        /*  PMD MDS Flag (-1 MDS created, 0 not written) */
    char pmd_mds_flag;
        /*  Fractional Polarisation MDS Flag (-1 MDS created, 0 not written) */
    char frac_pol_mds_flag;
        /*  Slit Function GADS Flag (-1 GADS copied, 0 not copied) */
    char slit_function_gads_flag;
        /*  Sun Mean Reference GADS Flag (-1 GADS copied, 0 not copied) */
    char sun_mean_ref_gads_flag;
        /*  Leakage Current GADS Flag (-1 GADS copied, 0 not copied) */
    char leakage_current_gads_flag;
        /*  Spectral Calibration GADS Flag (-1 GADS copied, 0 not copied) */
    char spectral_cal_gads_flag;
        /*  Polarisation Sensitivity GADS Flag (-1 GADS copied, 0 not copied) */
    char pol_sens_gads_flag;
        /*  Radiance Sensitivity GADS Flag (-1 GADS copied, 0 not copied) */
    char rad_sens_gads_flag;
        /*  PPG/Etalon GADS Flag (-1 GADS copied, 0 not copied) */
    char ppg_etalon_gads_flag;
        /*  Number of Nadir Clusters selected */
    unsigned short num_nadir_clusters;
        /*  Number of Limb Clusters selected */
    unsigned short num_limb_clusters;
        /*  Number of Occultation Clusters selected */
    unsigned short num_occ_clusters;
        /*  Number of Monitoring Clusters selected */
    unsigned short num_mon_clusters;
        /*  Nadir Cluster Flags (-1 used, 0 not used) */
    char nadir_cluster_flag[64];
        /*  Limb Cluster Flags (-1 used, 0 not used) */
    char limb_cluster_flag[64];
        /*  Occultation Cluster Flags (-1 used, 0 not used) */
    char occ_cluster_flag[64];
        /*  Monitoring Cluster Flags (-1 used, 0 not used) */
    char mon_cluster_flag[64];
        /*  Memory Effect Correction Flag (-1 applied, 0 not applied) */
    char mem_effect_cal_flag;
        /*  Leakage Current Calibration Flag (-1 applied, 0 not applied) */
    char leakage_current_cal_flag;
        /*  Straylight Calibration Flag (-1 applied, 0 not applied) */
    char straylight_cal_flag;
        /*  PPG Calibration Flag (-1 applied, 0 not applied) */
    char ppg_cal_flag;
        /*  Etalon Calibration Flag (-1 applied, 0 not applied) */
    char etalon_cal_flag;
        /*  Spectral Calibration Flag (-1 applied, 0 not applied) */
    char spectral_cal_flag;
        /*  Polarisation Calibration Flag (-1 applied, 0 not applied) */
    char polarisation_cal_flag;
        /*  Radiance Calibration Flag (-1 applied, 0 not applied) */
    char radiance_cal_flag;
} CAL_OPTIONS_GADS ;


typedef struct Nadir_Pmd {
        /*  Start time of scan phase */
    MJD dsr_time;
        /*  Length of this DSR in bytes */
    unsigned int dsr_length;
        /*  Quality Indicator (-1 for blank MDSR, 0 otherwise) */
    char quality_flag;
        /*  Orbit phase after eclipse of the state (range: 0-1) */
    float orb_phase;
        /*  Measurement Category */
    unsigned short meas_cat;
        /*  State ID */
    unsigned short state_id;
        /*  Duration of scan phase of the state */
    unsigned short dur_scan_phase;
        /*  Number of Integrated PMD Values */
    unsigned short num_pmd;
        /*  Number of Geolocation Values */
    unsigned short num_geo;
        /*  Integrated PMD values */
    float *int_pmd;
        /*  Geolocation */
    GeoN *geo_nadir;
    GeoL *geo_limb;
 } Nadir_Pmd ;


/* 1C structures as available in 1c: names to be adapted */
typedef struct ClCon
{
  unsigned char id;
  unsigned char channel;
  unsigned short pixel_nr;
  unsigned short length;
  float pet;
  unsigned short int_time;
  unsigned short coadd;
  unsigned short n_read;
  unsigned char type;

    /* These fields are calculated : both are usually equal integration time */
    /*   but in case of extreme short pets different */
    // unsigned short meas_time;  	/* real used measurement time */
    // float true_int_time;
}
ClCon;


/*****************************************/

/* sun reference */
typedef struct gads_sun_ref
{
  char id[2];
  float wavel[NPIXEL];
  float spectrum[NPIXEL];
  float precision[NPIXEL];
  float accuracy[NPIXEL];
  float etalon[NPIXEL];
  float asm_pos;
  float esm_pos;
  float sun_elev;
  float pmd_mean[NPMD];
  float out_of_band_nd_out[NPMD];
  float out_of_band_nd_in[NPMD];
  float doppler;
}
gads_sun_ref;


/* states */
typedef struct Ads_States
{
  MJD StartTime;
  unsigned char  mds_attached;
  unsigned char  flag_reason;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short duration;
  unsigned short longest_int_time;
  unsigned short num_clusters;
  ClCon Clcon[MAX_CLUSTER];
  unsigned char  flag_mds;
  unsigned short num_aux;
  unsigned short num_pmd;
  unsigned short num_int;
  unsigned short int_times[MAX_CLUSTER];
  unsigned short num_polar[MAX_CLUSTER];
  unsigned short total_polar;
  unsigned short num_dsr;
  unsigned int  length_dsr;
}
ADS_STATES;

/* 1C special structures for measurement data */

/* constant part of 1c MDS */
typedef struct mds_1c_constant
{
  MJD StartTime;
  unsigned int length;
  char quality;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short cluster_id;
  unsigned short nobs;
  unsigned short npixels;
  unsigned char unit_flag;
} mds_1c_constant;

/* constant part of 1c integrated PMD MDS */
typedef struct mds_1c_pmd_constant
{
  MJD StartTime;
  unsigned long length;
  char quality;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short duration;
  unsigned short npmd;
  unsigned short ngeo;
}
mds_1c_pmd_constant;

/* constant part of 1c fract. polarisation MDS */
typedef struct mds_1c_pol_constant
{
  MJD StartTime;
  unsigned long length;
  char quality;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short duration;
  unsigned short ngeo;
  unsigned short npol;
  unsigned short nint;
  unsigned short int_times[64];
  unsigned short repetition[64];
}
mds_1c_pol_constant;

#pragma pack(pop)

// void Cal_Options_getbin (FILE* unit, Cal_Options *var);
//
// void ads_states_getbin (FILE* unit, Ads_States *var);
// void Clcon_getbin (FILE* unit, ClCon *var);
// void Clcon_array_getbin (FILE* unit, ClCon *var, int nr);
// void gads_sun_ref_getbin (FILE* unit, gads_sun_ref *var);
// void mds_1c_constant_getbin (FILE* unit, mds_1c_constant *var);

SCIA_err Read_SPH_SCI_NLC_1C (FILE* unit, SPH_SCI_NLC_1C *sph);


#endif
