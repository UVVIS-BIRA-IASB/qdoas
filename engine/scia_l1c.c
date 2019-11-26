#include "bin_read.h"
#include "scia_common.h"
#include "scia_defs.h"
#include "scia_l1c.h"


/* Array with DSD Names */

char DS_NAME_SCI_NLC_1C [40][29] = {
    "SUMMARY_QUALITY",
    "GEOLOCATION",
    "INSTRUMENT_PARAMS",
    "LEAKAGE_CONSTANT",
    "LEAKAGE_VARIABLE",
    "PPG_ETALON",
    "SPECTRAL_BASE",
    "SPECTRAL_CALIBRATION",
    "SUN_REFERENCE",
    "POL_SENS_NADIR",
    "POL_SENS_LIMB",
    "POL_SENS_OCC",
    "RAD_SENS_NADIR",
    "RAD_SENS_LIMB",
    "RAD_SENS_OCC",
    "ERRORS_ON_KEY_DATA",
    "SLIT_FUNCTION",
    "SMALL_AP_SLIT_FUNCTION",
    "STATES",
    "CAL_OPTIONS",
    "NADIR",
    "NADIR_PMD",
    "NADIR_FRAC_POL",
    "LIMB",
    "LIMB_PMD",
    "LIMB_FRAC_POL",
    "OCCULTATION",
    "OCCULTATION_PMD",
    "OCCULTATION_FRAC_POL",
    "MONITORING",
    "LEVEL_0_PRODUCT",
    "LEAKAGE_FILE",
    "PPG_ETALON_FILE",
    "SPECTRAL_FILE",
    "SUN_REF_FILE",
    "KEY_DATA_FILE",
    "M_FACTOR_FILE",
    "INIT_FILE",
    "ORBIT_FILE",
    "ATTITUDE_FILE" };

SCIA_err Read_SPH_SCI_NLC_1C (FILE* unit, SPH_SCI_NLC_1C *sph)

{
    int err;
    unsigned n;
    char nl[2];
    char *tmp = (char*) sph;
    /* Zeros in the complete structure, so all strings have trailing \0 */
    for (n=0; n<sizeof(SPH_SCI_NLC_1C); n++)
	tmp[n] = 0;

    err = fscanf(unit,
		 "SPH_DESCRIPTOR=\"%28c\"%1c"
		 "STRIPLINE_CONTINUITY_INDICATOR=%4c%1c"
		 "SLICE_POSITION=%4c%1c"
		 "NUM_SLICES=%4c%1c"
		 "START_TIME=\"%27c\"%1c"
		 "STOP_TIME=\"%27c\"%1c"
		 "START_LAT=%11c<10-6degN>%1c"
		 "START_LONG=%11c<10-6degE>%1c"
		 "STOP_LAT=%11c<10-6degN>%1c"
		 "STOP_LONG=%11c<10-6degE>%1c"
		 "%50c%1c"
		 "KEY_DATA_VERSION=\"%5c\"%1c"
		 "M_FACTOR_VERSION=\"%5c\"%1c"
		 "SPECTRAL_CAL_CHECK_SUM=\"%4c\"%1c"
		 "SATURATED_PIXEL=\"%4c\"%1c"
		 "DEAD_PIXEL=\"%4c\"%1c"
		 "DARK_CHECK_SUM=\"%4c\"%1c"
		 "NO_OF_NADIR_STATES=%4c%1c"
		 "NO_OF_LIMB_STATES=%4c%1c"
		 "NO_OF_OCCULTATION_STATES=%4c%1c"
		 "NO_OF_MONI_STATES=%4c%1c"
		 "NO_OF_NOPROC_STATES=%4c%1c"
		 "COMP_DARK_STATES=%4c%1c"
		 "INCOMP_DARK_STATES=%4c%1c"
		 "%4c%1c"
		 ,
		 sph->sph_descriptor, nl,
		 sph->stripline_continuity_indicator, nl,
		 sph->slice_position, nl,
		 sph->num_slices, nl,
		 sph->start_time, nl,
		 sph->stop_time, nl,
		 sph->start_lat, nl,
		 sph->start_long, nl,
		 sph->stop_lat, nl,
		 sph->stop_long, nl,
		 sph->spare_1, nl,
		 sph->key_data_version, nl,
		 sph->m_factor_version, nl,
		 sph->spectral_cal_check_sum, nl,
		 sph->saturated_pixel, nl,
		 sph->dead_pixel, nl,
		 sph->dark_check_sum, nl,
		 sph->no_of_nadir_states, nl,
		 sph->no_of_limb_states, nl,
		 sph->no_of_occultation_states, nl,
		 sph->no_of_moni_states, nl,
		 sph->no_of_noproc_states, nl,
		 sph->comp_dark_states, nl,
		 sph->incomp_dark_states, nl,
		 sph->spare_2, nl );

/* Not valid for 1C, 1B identifier(strange!!)
    if ( strncmp ("SCI_NLC_1C", sph->sph_descriptor,10) != 0)
	return SPH_ERROR;
*/
    if (err != 50)
	return SPH_ERROR;
    /* spare_1 in version 6.00 or later contains */
    /* INIT_VERSION + DECONT in later Versions (with SRON extractor) */
    /* next lines copy  entries or */
    memcpy (sph->init_version, sph->spare_1+13, 4);
    memcpy (sph->decont,  sph->spare_1+25, 8);
    return OK;
}
