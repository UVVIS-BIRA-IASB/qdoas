#include "comdefs.h"

#include "scia_defs.h"
#include "scia_common.h"
#include "scia_l1c_lib.h"
#include "bin_read.h"

// #define __DEBUG_L1C__

extern double wvlen_det_pix[8192];

// !!! #include "cfi_interface.h"


/* internal routines */

SCIA_err read_next_pmd_state (info_l1c *info, L1_MDS_TYPE type);
unsigned long Read_PMD_MDS (FILE* unit,
			    long int offset,
			    Nadir_Pmd *pmd, L1_MDS_TYPE type);
SCIA_err GeoN_add (GeoN *geo, /* array with 1c geolocations */
		  int integr_time, /* integr_time of a single pixel */
		  int n_coadd,	   /* how often to "add" */
		    int n_readout, /* readout counter for current state */
		  GeoN *geo_coadd); /* collect result here */

SCIA_err GeoL_add (GeoL *geo, /* array with 1c geolocations */
		  int integr_time, /* integr_time of a single pixel */
		  int n_coadd,	   /* how often to "add" */
		    int n_readout, /* readout counter for current state */
		  GeoL *geo_coadd); /* collect result here */

SCIA_err GeoCal_add (GeoCal *geo, /* array with 1c geolocations */
		  int integr_time, /* integr_time of a single pixel */
		  int n_coadd,	   /* how often to "add" */
		    int n_readout, /* readout counter for current state */
		  GeoCal *geo_coadd); /* collect result here */


/*********************************************************************\
* Open L1c, specify what to read
* reads all DSDs and stores read values in info structure
\*********************************************************************/

SCIA_err openL1c (char* FILE_name, info_l1c *info)
{
    int num_dsd = 0;
    int i, cat;
    DSD dsd;
				/*  set some uninitialized pointer to zero */
    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	info->idx_states[cat] = NULL;
    }
    info->ads_states = NULL;
    info->stateClconMeasTime=NULL;
    info->ads_states_geolocation = NULL;

				/* default is no TH re-calculation */
    info->cfi_limb_flag = 0;

				/*  here open the file */
    info->FILE_l1c = fopen(FILE_name, "rb");
    if (info->FILE_l1c == NULL) {
    DEBUG_Print(
		"Input file %s could not be opened! Abort!\n", FILE_name);
	return FILE_NOT_FOUND;
    }

    Read_MPH (info->FILE_l1c, &info->mph);
    Read_SPH_SCI_NLC_1C (info->FILE_l1c, &info->sph);
#if defined(__DEBUG_L1C__)
    DEBUG_Print("Product %s\n%s\n\n",info->mph.product, info->mph.spare_1);
#endif
    if (strncmp (info->mph.product, "SCI_NLC_1",9) != 0)
	return NO_L1C_FILE;
    /*--------------------------------------------------------------------*\
    ** read MPH
    ** get number of DSDs
    \*--------------------------------------------------------------------*/


    sscanf(info->mph.num_dsd, "%d", &num_dsd);
    info->num_dsd = num_dsd;


    /*--------------------------------------------------------------------*\
    ** read DSDs
    note: num_dsd seems to be too high by 1 (spare entry)!
    \*--------------------------------------------------------------------*/

    for (i=0; i<info->num_dsd-1; i++)
    {
	Read_DSD ( info->FILE_l1c, &dsd );

#if defined(__DEBUG_L1C__)
	DEBUG_Print("%s\n%c\n%s\n%i\n%i\n%i\n%i\n",
		dsd.name,
		dsd.type,
		dsd.filename,
		dsd.offset,
		dsd.size,
		dsd.num_dsr,
		dsd.size);
#endif
	/* check for dsd names */
	if (strncmp(dsd.name, "STATES",6) == 0) {
	    info->states = dsd;
	}
	if (strncmp(dsd.name, "GEOLOCATION",11) == 0) {
	    info->states_geolocation = dsd;
	}
	else if (strncmp(dsd.name, "SUN_REFERENCE",13) == 0) {
	    info->sun_ref = dsd;
	}
	else if (strncmp(dsd.name, "NADIR_PMD",9) == 0) {
	    info->nadir_pmd = dsd;
	}
	else if (strncmp(dsd.name, "LIMB_PMD",8) == 0) {
	    info->limb_pmd = dsd;
	}
	else if (strncmp(dsd.name, "OCCULTATION_PMD",15) == 0) {
	    info->occ_pmd = dsd;
	}
	else if (strncmp(dsd.name, "NADIR_FRAC_POL",14) == 0) {
	    info->nadir_pol = dsd;
	}
	else if (strncmp(dsd.name, "LIMB_FRAC_POL",13) == 0) {
	    info->limb_pol = dsd;
	}
	else if (strncmp(dsd.name, "OCCULTATION_FRAC_POL",20) == 0) {
	    info->occ_pol = dsd;
	}
	else if (strncmp(dsd.name, "NADIR",5) == 0) {
	    info->nadir = dsd;
	}
	else if (strncmp(dsd.name, "LIMB",4) == 0) {
	    info->limb = dsd;
	}
	else if (strncmp(dsd.name, "OCCULTATION",11) == 0) {
	    info->occ = dsd;
	}
	else if (strncmp(dsd.name, "MONITORING",10) == 0) {
	    info->mon = dsd;
	}
	else if (strncmp(dsd.name, "CAL_OPTIONS",11) == 0) {
	    info->cal_options = dsd;
	}
	else {
	    /* do nothing */
	}
    }
#if defined(__DEBUG_L1C__)
    DEBUG_Print("open_lv1c\n");
    DEBUG_Print("NADIR DSD %i\n",info->nadir.num_dsr);
#endif
    /* Put MDS offsets in array */
    info->mds_offset[MDS_NADIR] = info->nadir.offset;
    info->mds_offset[MDS_LIMB] = info->limb.offset;
    info->mds_offset[MDS_OCC] = info->occ.offset;
    info->mds_offset[MDS_MON] = info->mon.offset;
    /* Put MDS num_dsr in array */
    info->n_mds[MDS_NADIR] = info->nadir.num_dsr;
    info->n_mds[MDS_LIMB] = info->limb.num_dsr;
    info->n_mds[MDS_OCC] = info->occ.num_dsr;
    info->n_mds[MDS_MON] = info->mon.num_dsr;
    /* Put MDS num_dsr in array */
    info->n_pmd_mds[MDS_NADIR]       = info->nadir_pmd.num_dsr;
    info->n_pmd_mds[MDS_LIMB]        = info->limb_pmd.num_dsr;
    info->n_pmd_mds[MDS_OCC] = info->occ_pmd.num_dsr;
    info->n_pmd_mds[MDS_MON]  = 0;
    /* Put MDS num_dsr in array */
    info->mds_pmd_offset[MDS_NADIR]       = info->nadir_pmd.offset;
    info->mds_pmd_offset[MDS_LIMB]        = info->limb_pmd.offset;
    info->mds_pmd_offset[MDS_OCC] = info->occ_pmd.offset;
    info->mds_pmd_offset[MDS_MON]  = 0;

    /*  Read Calibr. Options */
    Read_cal_options_GADS (info);

    /*  fill in general user_file_info */
    info->user_file_info.product = info->mph.product;
    info->user_file_info.software_ver = info->mph.software_ver;
    info->user_file_info.abs_orbit = atoi (info->mph.abs_orbit);
    info->user_file_info.key_data_version = info->sph.key_data_version;
    info->user_file_info.m_factor_version = info->sph.m_factor_version;
    info->user_file_info.init_version     = info->sph.init_version;
    info->user_file_info.decont           = info->sph.decont;

    info->user_file_info.l1b_product_name = info->cal_options_GADS.l1b_product_name;
    i = 0;
    *info->user_file_info.cal_applied = '\0';
    if ( info->cal_options_GADS.mem_effect_cal_flag== -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 0");
 i++;
    }
    if ( info->cal_options_GADS.leakage_current_cal_flag == -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 1");
 i++;
    }
    if ( info->cal_options_GADS.straylight_cal_flag== -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 2");
 i++;
    }
    if ( info->cal_options_GADS.ppg_cal_flag == -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 3");
 i++;
    }
    if ( info->cal_options_GADS.etalon_cal_flag == -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 4");
 i++;
    }
    if ( info->cal_options_GADS.spectral_cal_flag  == -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 5");
 i++;
    }
    if ( info->cal_options_GADS.polarisation_cal_flag == -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 6");
 i++;
    }
    if ( info->cal_options_GADS.radiance_cal_flag  == -1 )
    {
 sprintf ( info->user_file_info.cal_applied + 2*i , " 7");
 i++;
    }

/* Read ADS states  */
    if ( ADS_state (info) != OK )
	return NO_ADS_STATES;
/* Count really available states */
#if defined(__DEBUG_L1C__)
    DEBUG_Print("open_lv1c\n"
	    "Num nadir states: %i"
	    "\n Num limb states: %i\n",
	    info->mds_offset[MDS_NADIR], info->mds_offset[MDS_LIMB]);
#endif
/* Build index lists for MDS <> states */
/* Trust that both are in consistent order */
/*  initialize arrays */

DEBUG_Print("num_dsr %d\n",info->states.num_dsr);

    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	info->idx_states[cat] = (int*) malloc (sizeof(int) *
					info->states.num_dsr);
	info->n_states[cat] = 0;
    }
/*  go through all states*/
    for (i=0; i < (int) info->states.num_dsr; i++)
    {
	/* type in product 1-4, index here in program 0-3 */
	int type = info->ads_states[i].flag_mds - 1;

	info->idx_states[type][info->n_states[type]++] = i;
    }

/*  check if corresponding MDS are available */
/*  if not, simply set n_state[type] to zero */
    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	if ( info->n_mds[cat] == 0 )
	    info->n_states[cat] = 0;
    }

/* Read solar spektrum for wl-grid */
    Fixed_Wavelength_grid (info);
/* set various counters to zero */
    info->cur_readout_in_state = 0;
    info->cur_max_readout_in_state= 0;
    info->cur_pix_start = 0;
    info->cur_pix_end = 0;
    info->sum_pix_cur_used_clusters = MAX_PIXELS;

    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	info->cur_mds_read[cat] = 0;
    }
    /* in solar output this might be used  */
    for (i=0; i<MAX_PIXELS; i++)
    {
	info->cur_pix_output_flag[i] = 1;
    }

/* preset to one large wavelengthwindow */
    info->cur_pix_start_arr[0] = 0;
    info->cur_pix_end_arr[0] = MAX_PIXELS-1;
    info->n_cur_pix = 1;

    info->cur_wl_start_arr[0] = 200.0;
    info->cur_wl_end_arr[0] = 8000.0;
    info->n_cur_wl = 1;

    info->n_wanted_clusters = 0;

    info->set_int_time = 0;

    for (i=0; i<MAX_MDS_TYPES; i++)
	info->cur_state_nr[i] = 0;
    return OK;
}

SCIA_err Fixed_Wavelength_grid (info_l1c *info)
{
    int n;
    /* use the pre-launch wavelength grid, stored in wvlen_det_pix.c
       in one large array */
  //  #include "wvlen_det_pix.c"

//    {
//    	FILE *fp;
//    	fp=fopen("toto.dat","a+t");
//    	for (n=0; n<NPIXEL; n++)
//    	 fprintf(fp,"%g\n",wvlen_det_pix[n]);
//    	fclose(fp);
//    }

    for (n=0; n<NPIXEL; n++)
	info->wl[n] = (float)wvlen_det_pix[n];
    return OK;
}

/* make pixel id ranges from wavelengt windows */

SCIA_err get_pixel_range_wl (info_l1c *info, int nwl_window,
			      float *wlstart, float *wlend ,
			      int *idx0, int *idx1)
{
    int n,p;
    for (n=0; n<nwl_window; n++)
    {
	for (p=0; p<NPIXEL; p++)
	{
	    if (wlstart[n] <= info->wl[p])
		break;
	}
	if (p > 0)
	    p--;
	idx0[n] = p;
	for (p=0; p<NPIXEL; p++)
	{
	    if (wlend[n] <= info->wl[p])
		break;
	}
	if (p == NPIXEL)
	    p--;
	idx1[n] = p;
	/* check */
	if (idx0[n] >= idx1[n] )
	    return SCIA_ERROR;
    }
    return OK;
}




SCIA_err set_int_time (info_l1c *info, int set_int_time)
{
    info->set_int_time = set_int_time;
    return OK;
}


/*********************************************************************\
 * Define pixel window from wavelength window
 * kb 23.04.01
 * channel : specify you want to read (1 - 8) to avoid problems in
 *           overlap regions (data from other channel)
 *           channel=0  : Do not restrict channel
\*********************************************************************/

SCIA_err set_wl_window (info_l1c *info, float wl_start, float wl_end,
			 int channel)
{
    int n;
    int search_start=0;
    int search_end=NPIXEL;
#if defined(__DEBUG_L1C__)
    DEBUG_Print("%8.2f %8.2f\n",wl_start,wl_end);
#endif
    if (channel > 0 && channel < 7 )
    {
	search_start = (channel-1) * 1024;
	search_end = channel * 1024;
    }


    for (n=search_start ; n<search_end; n++)
    {
	if (info->wl[n] > wl_start)
	    break;
    }
    if ( n == search_end )
	return WL_OUT_OF_RANGE;
    if ( n > search_start)
	info->cur_pix_start = n-1;
    else
	info->cur_pix_start = n;

    for (n = search_end; n>search_start; n--)
    {
	if ( info->wl[n-1] < wl_end )
	    break;
    }
    if (n == 0)
	return WL_OUT_OF_RANGE;
    info->cur_pix_end = n;

    if (info->cur_pix_start >= info->cur_pix_end)
	return WL_OUT_OF_RANGE;
    info->cur_pix_end_arr[0] = info->cur_pix_end;
    info->cur_pix_start_arr[0] = info->cur_pix_start;
    info->n_cur_pix = 1;
    return OK;
}


SCIA_err set_wl_window_multi_new (info_l1c *info,
				   float* wl_start, float* wl_end,
				   int* wl_channel,
				   int n_window)
{
    int w;
    for (w=0; w< n_window; w++)
    {
	info->cur_wl_start_arr[w] = wl_start[w];
	info->cur_wl_end_arr[w] = wl_end[w];
	info->cur_wl_channel_arr[w] = (float)wl_channel[w];
    }
/* 				/\*  check for distinct windows *\/ */
/* 				/\*  other checks are on input line *\/ */
/*     for (w=0; w< n_window-1; w++) */
/*     { */
/* 	if (info->cur_wl_end_arr[w] >= info->cur_wl_start_arr[w+1]) */
/* 	    return WL_OUT_OF_RANGE; */
/*     }	     */

    info->n_cur_wl = n_window;
    return OK;
}

SCIA_err set_wl_window_multi (info_l1c *info, float* wl_start, float* wl_end,
			       int *channel, int n_window)
{
    int n,w;
    int search_start=0;
    int search_end=NPIXEL;
    int ch;
/*     if (channel > 0 && channel < 7 ) */
/*     { */
/* 	search_start = (channel-1) * 1024; */
/* 	search_end = channel * 1024; */
/*     } */

    for (w=0; w< n_window; w++)
    {
	ch = channel[w] - 1;
	search_start = ch * 1024;
	search_end = (ch+1) * 1024;
	for (n=search_start ; n<search_end; n++)
	{
	    if (info->wl[n] > wl_start[w])
		break;
	}
	if ( n == search_end )
	    return WL_OUT_OF_RANGE;
	if ( n > search_start)
	    info->cur_pix_start_arr[w] = n-1;
	else
	    info->cur_pix_start_arr[w] = n;

	for (n = search_end; n>search_start; n--)
	{
	    if ( info->wl[n-1] < wl_end[w] )
		break;
	}
	if (n == search_start)
	    return WL_OUT_OF_RANGE;
	info->cur_pix_end_arr[w] = n;

	if (info->cur_pix_start_arr[w] >= info->cur_pix_end_arr[w])
	    return WL_OUT_OF_RANGE;
    }
				/*  check for distinct windows */
    for (w=0; w< n_window-1; w++)
    {
	if (info->cur_pix_end_arr[w] >= info->cur_pix_start_arr[w+1])
	    return WL_OUT_OF_RANGE;
    }

    info->n_cur_pix = n_window;
    return OK;
}


/*********************************************************************\
 * Define pixel window from pixel_nr
 * kb 23.04.01
\*********************************************************************/

SCIA_err set_pix_window (info_l1c *info, int pix_start, int pix_end)
{
    if (pix_start < 0 || pix_end > 8192 || pix_start >= pix_end)
	return  WL_OUT_OF_RANGE;
    info->cur_pix_end = pix_end;
    info->cur_pix_start = pix_start;
    info->cur_pix_end_arr[0] = pix_end;
    info->cur_pix_start_arr[0] = pix_start;
    info->n_cur_pix = 1;
    return OK;
}


/*********************************************************************\
 * Define cluster instead of pixel windows from cluster list
 *  in integer array cluster_id  with n_cluster elements
 *
 * kb 31.09.03
\*********************************************************************/

SCIA_err set_clusters (info_l1c *info, int* cluster_id, int n_cluster)

{
    int n;

    if (n_cluster == 0 || n_cluster > 64)
	return  WL_OUT_OF_RANGE;
				/*  usefull numbers? */
    for (n=0; n<n_cluster; n++)
    {
	if (cluster_id[n] < 0 || cluster_id[n] > 64)
	    return  WL_OUT_OF_RANGE;
    }
				/*  ascending order */
    for (n=0; n<n_cluster-1; n++)
    {
	if (cluster_id[n] >= cluster_id[n+1])
	    return  WL_OUT_OF_RANGE;
    }
				/*  copy to info */
    for (n=0; n<n_cluster; n++)
	info->wanted_clusters[n] = cluster_id[n];

    info->n_wanted_clusters = n_cluster;
    				/*  set pixel limit to all pixels */
				/*  cat be set smaller with
				    set_pix_window or set_wl_window */
    info->cur_pix_end = 8191;
    info->cur_pix_start = 0;
    return OK;
}




/*********************************************************************\
 * Close L1c
\*********************************************************************/

SCIA_err closeL1c (info_l1c *info)
{
    int cat;
    if (info->ads_states)
	free (info->ads_states);
	   if (info->stateClconMeasTime)
	    free(info->stateClconMeasTime);
    if (info->ads_states_geolocation)
	free (info->ads_states_geolocation);
    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	if (info->idx_states[cat])
	    free (info->idx_states[cat]);
    }
    if (info->FILE_l1c)
    {
	if (fclose(info->FILE_l1c) != 0) {
	    return FILE_NOT_CLOSED;
	}
    }
  return OK;
}


/*********************************************************************\
 * Free memory in user_data, allocated in read_next....
\*********************************************************************/

SCIA_err free_user_data (user_data *ud)

{
    free (ud->pixel_nr);
    free (ud->pixel_cls);
    free (ud->pixel_ch);
    free (ud->pixel_coadd);
    free (ud->wl);
    free (ud->wl_err);
    free (ud->signal);
    free (ud->signal_err);
    if (ud->n_pmd > 0)
    {
	free (ud->pmd);
	if (ud->pmd_geo_nadir)
	    free (ud->pmd_geo_nadir);
	if (ud->pmd_geo_limb)
	    free (ud->pmd_geo_limb);
    }
    return OK;
}



SCIA_err free_user_data_solar (user_data_solar *ud)
{
    free (ud->pixel_nr);
    free (ud->pixel_ch);
    free (ud->wl);
    free (ud->signal);
    free (ud->signal_err);
    return OK;
}

/*********************************************************************\
 * Read cal_options_GADS
 * kb, 23.04.02
\*********************************************************************/

SCIA_err Read_cal_options_GADS (info_l1c *info)
{
    int n,cluster;

				/* go to GADS cal_options */
    if (fseek(info->FILE_l1c, info->cal_options.offset, SEEK_SET) != -1)
    {
				/* read into memory (include swapping) */
	cal_options_GADS_getbin (info->FILE_l1c, &info->cal_options_GADS);
				/* Set list of included clusters */
	for (n=0; n<MAX_MDS_TYPES; n++)
	    info->max_cluster_ids[n] = 0;
	for (cluster=0; cluster<MAX_CLUSTER; cluster++)
	{
	    if (info->cal_options_GADS.nadir_cluster_flag[cluster] == 0)
		continue;		/* Cluster not available */
	    info->cluster_ids [MDS_NADIR][info->max_cluster_ids[MDS_NADIR]++] = cluster;
	    if (info->cal_options_GADS.limb_cluster_flag[cluster] == 0)
		continue;		/* Cluster not available */
	    info->cluster_ids [MDS_LIMB][info->max_cluster_ids[MDS_LIMB]++] = cluster;
	    if (info->cal_options_GADS.occ_cluster_flag[cluster] == 0)
		continue;		/* Cluster not available */
	    info->cluster_ids [MDS_OCC][info->max_cluster_ids[MDS_OCC]++] = cluster;
	    if (info->cal_options_GADS.mon_cluster_flag[cluster] == 0)
		continue;		/* Cluster not available */
	    info->cluster_ids [MDS_MON][info->max_cluster_ids[MDS_MON]++] = cluster;
	}
	return OK;
    }
    return NO_GADS_CAL_OPTIONS;
}



/*********************************************************************\
 * Read complete state info
\*********************************************************************/

SCIA_err ADS_state (info_l1c *info)
{
    unsigned num_states, num_geo_states, istate, num_use_states;
    ADS_STATES *state;
    Geolocation *geo_state;
    num_states = info->states.num_dsr; /* No. of states */
    num_geo_states = info->states_geolocation.num_dsr;

    if (num_states == 0 || num_states != num_geo_states)
	return NO_ADS_STATES;

    state = (ADS_STATES*) malloc (num_states * sizeof(ADS_STATES));
    geo_state = (Geolocation*) malloc (num_states * sizeof(Geolocation));
    info->stateClconMeasTime = (STATE_CLCON_MEASTIME *)malloc(num_states*sizeof(ADS_STATES));
    info->ads_states = (ADS_STATES*) malloc (num_states * sizeof(ADS_STATES));
    info->ads_states_geolocation =
	(Geolocation*) malloc (num_states * sizeof(Geolocation));

	DEBUG_Print("states offset %d\n",info->states.offset);

    if (fseek(info->FILE_l1c, info->states.offset, SEEK_SET) != -1)
    {
	for (istate=0; istate<num_states;istate++)
	{
	    ads_states_getbin (info->FILE_l1c, &state[istate]);
	}
    }
    else
	return NO_ADS_STATES;

// SCIA !!!     if (fseek(info->FILE_l1c, info->states_geolocation.offset, SEEK_SET) != -1)
// SCIA !!!     {
// SCIA !!!
// SCIA !!! 	for (istate=0; istate<num_states;istate++)
// SCIA !!! 	{
// SCIA !!! 	    Geolocation_getbin (info->FILE_l1c, &geo_state[istate]);
// SCIA !!! 	}
// SCIA !!!    }
// SCIA !!!    else
// SCIA !!!	return NO_ADS_STATES;

    num_use_states = 0;
    for (istate=0; istate<num_states;istate++)
    {
	if ( state[istate].mds_attached == 0 )
	{
	    info->ads_states[num_use_states] = state[istate];
	    info->ads_states_geolocation[num_use_states] =
		geo_state[istate];
	    num_use_states++;
	}
    }

    info->states.num_dsr = num_use_states;
    free (state);
    free (geo_state);
    return OK;
}

/*********************************************************************\
 * read solar
\*********************************************************************/

SCIA_err read_solar (info_l1c *info,
		      char *sun_spec_id,
		      user_data_solar *ud)
{
  gads_sun_ref sun_ref;
  int pix_delta;
  int iset;
  int i,n;

				/* are there sun reference data sets? */
  if (info->sun_ref.num_dsr <= 0) {
    return NO_SUN_REF;
  }
				/* position file */
  if (fseek(info->FILE_l1c, info->sun_ref.offset, SEEK_SET) != 0) {
    return NO_SUN_REF;
  }

				/* choose needed pixels */
  pix_delta = info->sum_pix_cur_used_clusters;

  ud->n_wl = pix_delta;

				/* allocate memory for output */
  ud->pixel_nr = (unsigned short*) malloc(pix_delta*sizeof(unsigned short));
  ud->pixel_ch = (unsigned short*) malloc(pix_delta*sizeof(unsigned short));
  ud->wl = (float*)malloc(pix_delta*sizeof(float));
  ud->signal = (float*)malloc(pix_delta*sizeof(float));
  ud->signal_err = (float*)malloc(pix_delta*sizeof(float));



  for (iset=0; iset<(int)info->sun_ref.num_dsr;iset++) {

      gads_sun_ref_getbin (info->FILE_l1c, &sun_ref);
/*     sun_ref = ReadSunReference(info->FILE_l1c); */

      if (strncmp(sun_ref.id, sun_spec_id, 2) == 0)
      {
	  for (i=0,n=0; i<MAX_PIXELS; i++)
	  {
	      if (info->cur_pix_output_flag[i] == 0)
		  continue;

	      ud->pixel_nr[n] = i%1024;
	      ud->pixel_ch[n] = i/1024+1;
	      ud->wl[n] = sun_ref.wavel[i];
	      ud->signal[n] = sun_ref.spectrum[i];
	      ud->signal_err[n] = sun_ref.accuracy[i];
	      n++;
	/* also available: sun_ref.etalon[i], sun_ref.precision[i] */
	  }
	  break;			/* no need to read further */
      }
  }
  if (iset == (int)info->sun_ref.num_dsr) {
      DEBUG_Print( "No solar data available.\n");
      return NO_SUN_REF_DATA;
  }
  /* Copy Sun_spec_id */
  strncpy (ud->sun_spect_id,  sun_ref.id, 3);
  /* Copy orbit info */
  ud->product            = info->user_file_info.product ;
  ud->software_ver       = info->user_file_info.software_ver ;
  ud->abs_orbit          = info->user_file_info.abs_orbit;
  ud->key_data_version   = info->user_file_info.key_data_version ;
  ud->m_factor_version   = info->user_file_info.m_factor_version ;
  ud->l1b_product_name   = info->user_file_info.l1b_product_name;
  ud->cal_applied        = info->user_file_info.cal_applied;
  ud->init_version       = info->user_file_info.init_version;     /* Version Initialisation files */
  ud->decont             = info->user_file_info.decont      ;           /* Decontamination flag */
  strncpy (ud->date , info->mph.sensing_start, 28);
#if defined(__DEBUG_L1C__)
  DEBUG_Print("solar spectrum successfully read\n");
#endif
  return OK;
}


/* read solar spectrum, but from newly calculated ADS */

SCIA_err read_solar_newly_calculated (info_l1c *info,
				      char *sun_spec_id,
				      user_data_solar *ud)
{
  gads_sun_ref sun_ref;
  int pix_delta;
  int iset;
  int i,n;

				/* are there sun reference data sets? */
  if (info->sun_ref.num_dsr <= 0) {
    return NO_SUN_REF;
  }
				/* position file */
  if (fseek(info->FILE_l1c, info->sun_ref.offset, SEEK_SET) != 0) {
    return NO_SUN_REF;
  }

				/* choose needed pixels */
  pix_delta = info->sum_pix_cur_used_clusters;

  ud->n_wl = pix_delta;

				/* allocate memory for output */
  ud->pixel_nr = (unsigned short*) malloc(pix_delta*sizeof(unsigned short));
  ud->pixel_ch = (unsigned short*) malloc(pix_delta*sizeof(unsigned short));
  ud->wl = (float*)malloc(pix_delta*sizeof(float));
  ud->signal = (float*)malloc(pix_delta*sizeof(float));
  ud->signal_err = (float*)malloc(pix_delta*sizeof(float));



  for (iset=0; iset<(int)info->sun_ref.num_dsr;iset++) {

      gads_sun_ref_getbin (info->FILE_l1c, &sun_ref);
/*     sun_ref = ReadSunReference(info->FILE_l1c); */

      if (strncmp(sun_ref.id, sun_spec_id, 2) == 0)
      {
	  for (i=0,n=0; i<MAX_PIXELS; i++)
	  {
	      if (info->cur_pix_output_flag[i] == 0)
		  continue;

	      ud->pixel_nr[n] = i%1024;
	      ud->pixel_ch[n] = i/1024+1;
	      ud->wl[n] = sun_ref.wavel[i];
	      ud->signal[n] = sun_ref.spectrum[i];
	      ud->signal_err[n] = sun_ref.accuracy[i];
	      n++;
	/* also available: sun_ref.etalon[i], sun_ref.precision[i] */
	  }
	  break;			/* no need to read further */
      }
  }
  if (iset == (int)info->sun_ref.num_dsr) {
      DEBUG_Print( "No solar data available.\n");
      return NO_SUN_REF_DATA;
  }
  /* Copy Sun_spec_id */
  strncpy (ud->sun_spect_id,  sun_ref.id, 3);
  /* Copy orbit info */
  ud->product            = info->user_file_info.product ;
  ud->software_ver       = info->user_file_info.software_ver ;
  ud->abs_orbit          = info->user_file_info.abs_orbit;
  ud->key_data_version   = info->user_file_info.key_data_version ;
  ud->m_factor_version   = info->user_file_info.m_factor_version ;
  ud->l1b_product_name   = info->user_file_info.l1b_product_name;
  ud->cal_applied        = info->user_file_info.cal_applied;
  ud->init_version       = info->user_file_info.init_version;     /* Version Initialisation files */
  ud->decont             = info->user_file_info.decont      ;           /* Decontamination flag */
  strncpy (ud->date , info->mph.sensing_start, 28);
#if defined(__DEBUG_L1C__)
  DEBUG_Print("solar spectrum successfully read\n");
#endif
  return OK;
}



/*********************************************************************\
 * Read MDS (1c) complete
\*********************************************************************/



/*--------------------------------------------------------------------*\
** read MDS (1c cluster data), returns length of DSR
\*--------------------------------------------------------------------*/
unsigned long ReadMDS(FILE* unit, long int offset, L1_MDS_TYPE type,
		      int Cluster, state_cluster_data* data)

{
  mds_1c_constant mds;

				/* variables for variable part of MDS;
				   exact size needs to be dynamically
				   allocated */
  int this_size;

				/*********************/
				/*  Start of routine */
				/*********************/
				/* go to start of MDS */
  fseek (unit, offset,  SEEK_SET);

				/* read constant part */
  mds_1c_constant_getbin (unit, &mds);

				/* create dynamic variables and read
				   variable part */

				/* pixel id */
  this_size = sizeof(unsigned short)*mds.npixels;
  data->pixel = (unsigned short *)malloc(this_size);
  ushort_array_getbin (unit, data->pixel, mds.npixels);

				/* wavelength */
  this_size = sizeof(float)*mds.npixels;
  data->wl = (float *)malloc(this_size);
  float_array_getbin (unit, data->wl, mds.npixels);

				/* wavelength error */
  data->wl_err = (float *)malloc(this_size);
  float_array_getbin (unit, data->wl_err, mds.npixels);

				/* signal */
  this_size = sizeof(float)*mds.npixels*mds.nobs;
  data->signal = (float *)malloc(this_size);
  float_array_getbin (unit, data->signal, mds.npixels*mds.nobs);

				/* signal error */
  data->signal_err = (float *)malloc(this_size);
  float_array_getbin (unit, data->signal_err, mds.npixels*mds.nobs);



#if defined(__DEBUG_L1C__)
  DEBUG_Print("ReadMDS: Cluster, offset, mds.npixels, mds.nobs : %i %li %i %i\n", Cluster, offset, mds.npixels, mds.nobs);
#endif

				/* geolocation (depends on measurement type) */

  switch (type) {

      case MDS_NADIR:
	  this_size = mds.nobs*sizeof(GeoN);
	  data->geo_nadir = (GeoN *)malloc(this_size);
	  GeoN_array_getbin (unit, data->geo_nadir, mds.nobs);
	  break;

      case MDS_LIMB:
      case MDS_OCC:
	  this_size = mds.nobs*sizeof(GeoL);
	  data->geo_limb = (GeoL *)malloc(this_size);
	  GeoL_array_getbin (unit, data->geo_limb, mds.nobs);
	  break;

// SCIA !!!       case MDS_MON:
// SCIA !!! 	  this_size = mds.nobs*sizeof(GeoCal);
// SCIA !!! 	  data->geo_cal = (GeoCal *)malloc(this_size);
// SCIA !!! 	  GeoCal_array_getbin (unit, data->geo_cal, mds.nobs);
      default:
	  ;
  }

				/* set output variables */

  data->n_geo = mds.nobs;
  data->n_wl = mds.npixels;
  data->quality = mds.quality;
  data->mds_const = mds;
  data->type = type;
  return mds.length;
}

/* used to determine next offset */
  /* remember to free dynamic variables later */
  /*    free(pixel_id); */
  /*    free(wavelength); */
  /*    free(wavelength_error); */
  /*    free(signal); */
  /*    free(signal_error); */

  /*    switch (type) { */
  /*    case NADIR: */
  /*      free(geon); */
  /*      break; */
  /*    case LIMB: */
  /*    case OCCULTATION: */
  /*      free(geol); */
  /*      break; */
  /*    case MONITORING: */
  /*      free(geoc); */
  /*      break; */





/*********************************************************************\
 * Basic routine for reading science data of one state
 * kb 31.07.02
\*********************************************************************/


SCIA_err read_next_state (info_l1c *info, L1_MDS_TYPE type)

{
    int cluster,id,i,n_state;

    unsigned long length;

				/* are there data sets? */
  if (info->n_states[type] == 0) {
      DEBUG_Print( "No data for type %i!\n", type);
      return NO_SCIADATA;
  }
		     /* check the availability of clusters in state */
				/*  State info list all clusters originally
				    available in 1B product*/
                                /* Calibration options says, what shoul be
				   included */
  info->cur_max_cluster_ids = 0;
				/*  loop of GADS cluster index(0-63) */
  n_state = info->idx_states[type][info->cur_state_nr[type]];

#if defined(__DEBUG_L1C__)
  DEBUG_Print("read_next_state : n_state= %2i\n", n_state);
#endif

  for (cluster=0; cluster<info->max_cluster_ids[type]; cluster++)
  {
      id = info->cluster_ids [type][cluster];
				/* loop of state cluster ids (1-64 -> -1 !!) */
      for (i=0; i<64; i++)
      {
	  if (id == info->ads_states[n_state].Clcon[i].id -1 )
	  {
				/*  store match (index: 0-63)*/
	      info->cur_cluster_ids[info->cur_max_cluster_ids++] = id;
	  }
      }
  }
		     /* Something is very wrong, if this happens... */
  if ( info->cur_max_cluster_ids == 0 )
      return NO_CLUSTER_MATCH_IN_STATE;

				/* DEBUG_Print("Offset: %ld\n", offset);*/
  for (cluster=0; cluster<info->cur_max_cluster_ids; cluster++)
  {
#if defined(__DEBUG_L1C__)
      DEBUG_Print("read_next_state: %2i %2i   ", cluster, info->cur_cluster_ids[cluster]);
#endif
				/* read next MDSA data */
      length = ReadMDS(info->FILE_l1c, info->mds_offset[type], type,
		       info->cur_cluster_ids[cluster],
		       info->st_cl_data + info->cur_cluster_ids[cluster]);
				/*  count MDS to check whenn all is read */
      info->cur_mds_read[type] ++;
				/* DEBUG_Print("length: %ld, offset: %ld\n", length, offset);*/

				/*  consistency check !!!! */
      /* Here we have to search for the right state, because some stupid Lv1C production programms */
      /* do not delete  state entries of states not in the 1C product */

      while (MJD_compare (&info->st_cl_data[info->cur_cluster_ids[cluster]].mds_const.StartTime,
			 &info->ads_states[n_state].StartTime) > 0)
      {
	  /* goto to next state in list, if current data after current state */
	    n_state = info->idx_states[type][ ++(info->cur_state_nr[type]) ];
#if defined(__DEBUG_L1C__)
	    {
		char date[29];
		UTC_String (&info->ads_states[n_state].StartTime, date);
		fprintf (stderr,"expected state Start time: %s\n", date);
	    }
#endif
      }

      if (MJD_compare (&info->st_cl_data[info->cur_cluster_ids[cluster]].mds_const.StartTime,
		      &info->ads_states[n_state].StartTime) < 0)
      {
	  char date[29];
	  fprintf (stderr,"State <-> MDS mismatch: Fatal product or programm error!\n");
	  fprintf (stderr,"Cluster %i\n", info->cur_cluster_ids[cluster]);

	  UTC_String (&info->ads_states[n_state].StartTime, date);
	  fprintf (stderr,"expected state Start time: %s\n", date);

	  UTC_String (&info->st_cl_data[info->cur_cluster_ids[cluster]].mds_const.StartTime, date);
	  fprintf (stderr,"measurement    Start time: %s\n", date);

	  exit (1);
      }
#if HAVE_LIBCFI
      /* recalculate tangent height */
      if (type == MDS_LIMB && info->cfi_limb_flag == 1)
      {
	  char date[29];
	  int n;
	  GeoL *geo_limb =
	      (info->st_cl_data + info->cur_cluster_ids[cluster])->geo_limb;
	  int n_geo =
	      (info->st_cl_data + info->cur_cluster_ids[cluster])->n_geo;
	  MJD statetime = info->ads_states[n_state].StartTime;
	  int IT = info->ads_states[n_state].
	      Clcon[info->cur_cluster_ids[cluster]].int_time;

	  for (n=0; n< n_geo; n++) {
	      double pos_esm = geo_limb[n].esm_pos;
	      double pos_asm = geo_limb[n].asm_pos;
	      double elev_eff = (pos_esm > 0.0) ?
		  90.0 - pos_esm : 90.0 + pos_esm;
	      double azim_eff = (pos_asm < 0.0) ?
		  pos_asm +360.0 : pos_asm;
	      MJD mjd;
	      cfi_limb cfi;

	      mjd = MJD_add (statetime, IT * n +
			     ((IT * n) / SCAN_LIMB_DURATION) * SCAN_LIMB_STEP_DURATION
			     + IT/2);
	      UTC_String (&mjd, date);

	      cfi_propagate_limb (mjd, &cfi, azim_eff, elev_eff);

	      geo_limb[n].earth_radius = cfi.earth_rad;
	      geo_limb[n].sat_height = cfi.sat_height;
	      geo_limb[n].tangent_height[0] = cfi.tangent_hght;
	      geo_limb[n].tangent_height[1] = cfi.tangent_hght;
	      geo_limb[n].tangent_height[2] = cfi.tangent_hght;

	  }
      }
#endif
				/* increase offset */
      info->mds_offset[type] += length;
  }

  return OK;
}



/*********************************************************************\
 * Clean up memory from previous read nadir state
 * kb 21.07.02
\*********************************************************************/


SCIA_err clear_state (info_l1c *info, L1_MDS_TYPE type)
{
    int cluster,id;
    if (info->cur_state_nr[type] > 0)
    {
	for (cluster=0; cluster<info->cur_max_cluster_ids; cluster++)
	{
	    id = info->cur_cluster_ids[cluster];
	    free (info->st_cl_data[id].pixel);
	    free (info->st_cl_data[id].wl);
	    free (info->st_cl_data[id].wl_err);
	    free (info->st_cl_data[id].signal);
	    free (info->st_cl_data[id].signal_err);
	    switch (info->st_cl_data[id].type)
	    {
		case MDS_NADIR:
		    free (info->st_cl_data[id].geo_nadir);
		    break;
		case MDS_LIMB:
		case MDS_OCC:
		    free (info->st_cl_data[id].geo_limb);
		    break;
		case MDS_MON:
		    free (info->st_cl_data[id].geo_cal);
		    break;
		default:
		    DEBUG_Print( "L1C_error clear_nadir_state("
			    "info_l1c *info) : Invalid measurement type!\n");
		    exit (1);
		    break;
	    }
	}
    }
    return OK;
}


/*
   Helper for next function
 */
SCIA_err check_wl_cluster (info_l1c *info, /* contains wavelength_array */
			    int cl_start, int cl_end, /* cluster range */
			    float wl_start, float wl_end, /* wavelength */
			    int* pix_start, int *pix_end )/* overlap */
{
    int pix;
    /* search pixel before wl_start (or first of cluster) */
    for (pix=cl_end; pix>cl_start; pix--)
	if (wl_start >= info->wl[pix])
	    break;
    *pix_start = pix;
    /* search pixel before wl_end */
    for (pix=cl_end; pix>cl_start; pix--)
	if (wl_end > info->wl[pix-1])
	    break;
    /* add 1 to include wl_end, if not at end of cluster */
    if (pix<cl_end)
	pix++;
    *pix_end = pix;
    return OK;
}


/* compare function for qsort of used cluster */

int compare_cur_used_cluster (const void *sl1,
                              const void *sl2)
{
    if ( ((cur_used_cluster*)sl1)->pix_start <
	 ((cur_used_cluster*)sl2)->pix_start   )
        return -1;
    if ( ((cur_used_cluster*)sl1)->pix_start >
	 ((cur_used_cluster*)sl2)->pix_start   )
        return 1;
    return 0;
}


/*
   Analyse cluster information to select the parts wanted in output
*/
SCIA_err calc_cluster_window (info_l1c *info, L1_MDS_TYPE type)
{
    int n_state,id, channel;
    int cl_start;
    int cl_end;
    int n,w;
    int meas_time;
    unsigned short n_cl, cl;
    int n_used_cl;
    ADS_STATES state;
    int clconMeastime[MAX_CLUSTER];

    n_state = info->idx_states[type][info->cur_state_nr[type]];

    state = info->ads_states[n_state];
				/* cluster information */
				/*  calculate measurement time */
				/*    sometimes different from integration time */
    for (cl=0; cl<state.num_clusters; cl++)
    {
	    // SCIA !!! state.Clcon[cl].meas_time =

	    clconMeastime[cl]=
	      state.longest_int_time / state.Clcon[cl].n_read;

	   // SCIA !!! Never used state.Clcon[cl].true_int_time =
	   // SCIA !!! Never used      state.Clcon[cl].pet * state.Clcon[cl].coadd;
    }
				/* Examine cluster definitions */
				/* Which cluster to use? */
				/*  pre set all to zero */
    n_used_cl = 0;
    info->sum_pix_cur_used_clusters = 0;
    for (n=0; n<MAX_PIXELS ;n++)
	info->cur_pix_output_flag[n] = 0;

    for (n_cl = 0; n_cl < info->cur_max_cluster_ids; n_cl++)
	info->cur_used_cl[n_cl].pix_n = 0;

				/*  loop over clusters (available in 1C-file) */
				/*  search what is wanted in output */
    for (n_cl = 0; n_cl < info->cur_max_cluster_ids; n_cl++)
    {
	id = info->cur_cluster_ids[n_cl];
	channel = state.Clcon[id].channel;

	cl_start = (state.Clcon[id].channel-1) * 1024 + state.Clcon[id].pixel_nr;
	cl_end = cl_start + state.Clcon[id].length;

				/*  select according to cluster list */
	for (n = 0; n < info->n_wanted_clusters ; n++)
	{
				/*  cluster ID in product is 1-64 */
				/*  id in programm starts with 0 as index
				    in C */
	    if (id == info->wanted_clusters[n] -1 )
		break;
	}
	if (info->n_wanted_clusters > 0 && n == info->n_wanted_clusters)
	    continue;		/* if cluster not in non-empty want list, */
				/* try next */
				/*  check for pix window */
	for (w=0; w < info->n_cur_wl; w++)
	{
	    int p_start, p_end, p;
				/* if this wl-window is restricted to */
				/*  another cluster, ignore it */
	    if (info->cur_wl_channel_arr[w] != 0 &&
		info->cur_wl_channel_arr[w] != channel )
		continue;
	    check_wl_cluster (info, /* contains wavelength_array */
			      cl_start, cl_end, /* cluster range */
			      info->cur_wl_start_arr[w],
			      info->cur_wl_end_arr[w],
			      &p_start, &p_end);

/* 	    fprintf (stderr, "id w, cl_start, cl_end, wl_start, wl_end, p_start, p_end, %2d %1d  %4d %4d  %6.1f %6.1f  %4d %4d\n",  */
/* 		     id, w,   cl_start, cl_end, /\* cluster range *\/ */
/* 		     info->cur_wl_start_arr[w], */
/* 		     info->cur_wl_end_arr[w], */
/* 		     p_start, p_end); */

	    for (p=p_start; p<p_end; p++)
	    {
		info->cur_pix_output_flag[p] = 1;
	    }

	    info->cur_used_cl[n_used_cl].pix_n += p_end - p_start;
	    info->sum_pix_cur_used_clusters += p_end - p_start;
	}
	if (info->cur_used_cl[n_used_cl].pix_n == 0)
	    continue;  /* not in pix window, try next cluster */

				/*  Here a wanted cluster is found */
	info->cur_used_cl[n_used_cl].pix_start = cl_start;
	info->cur_used_cl[n_used_cl].pix_length   = state.Clcon[id].length;
	info->cur_used_cl[n_used_cl].id        = id;
	n_used_cl++;
    }
				/*  if available cluster not in wl-window*/
				/*  return error value*/
    if (n_used_cl == 0)
	return CLUSTER_WAVELENGTH_MISMATCH;
    info->max_cur_used_clusters = n_used_cl;

				/*  sort clusters according to start pixel */
                                /*(Channel 2 is ordered */
				/*  in other direction !!!*/

    qsort (info->cur_used_cl, info->max_cur_used_clusters,
	   sizeof (cur_used_cluster),
	   compare_cur_used_cluster);


				/* Determine co-adding factors and
				   Geol. to use*/
				/*    longest meas_time in pix_window */
    info->geo_cur_used_clusters = info->cur_used_cl[0].id;
    for (n_cl = 1; n_cl<info->max_cur_used_clusters; n_cl++)
    {
	id = info->cur_used_cl[n_cl].id;

	if (clconMeastime[id] > clconMeastime[info->geo_cur_used_clusters])
	    info->geo_cur_used_clusters = id;
    }

    meas_time = clconMeastime[info->geo_cur_used_clusters];

    info->max_int_time = meas_time;
				/*  if extended int_time is wanted,
				    set it here, only if larger than
				    calculated one */
    if (info->set_int_time != 0)
    {
	if (info->set_int_time > meas_time)
	    meas_time = info->set_int_time;
    }

    for (n_cl = 0; n_cl<info->max_cur_used_clusters; n_cl++)
    {
	id = info->cur_used_cl[n_cl].id;
				/*   calc co-adding factors  */
	info->cur_used_cl[n_cl].coadd =
	    meas_time / clconMeastime[id] ;

    }

				/* Calculate number of readouts for pix-window */
    switch (type)
    {
	case MDS_NADIR:
	case MDS_MON:
	    info->cur_max_readout_in_state =
		state.duration / meas_time;
	    break;
	case MDS_LIMB:
				/*  state duration is one BCPS to short */
				/*  n_levels = state duration
                                                 / horizontal scan_time
				    n_profile = horizontal meas. time
                                                / int time
				    number of readouts = n_level*n_profiles
				*/
	    info->cur_max_readout_in_state =
		(state.duration + 1) /
		(SCAN_LIMB_DURATION + SCAN_LIMB_STEP_DURATION)
		* SCAN_LIMB_DURATION / meas_time;
	    break;
	case MDS_OCC:
	    info->cur_max_readout_in_state =
		state.duration / meas_time;
	    break;
	default:
	    break;
    }
    info->cur_it = meas_time;

#if defined(__DEBUG_L1C__)
	DEBUG_Print("calc_cluster_window : cur_max_read_out_in_state %i\n",
		info->cur_max_readout_in_state);
#endif
/*	info->cur_max_readout_in_state =
	info->st_cl_data[info->geo_cur_used_clusters].mds_const.nobs;*/
				/*  calculate number of PMD measurements */
				/*  PMD 1/32s    IT in 1/16s */
	info->cur_num_pmd = info->cur_it * 2;
					/* Test output */
    return OK;
}

/*********************************************************************\
 * Coadding data and errors
 * kb 23.04.01
\*********************************************************************/

SCIA_err coadd_signal (unsigned int n_wl, unsigned int n_coadd,
			float *signal,           /* Input: rad[n_coadd*n_wl] */
			float *signal_err,       /* Input: rad_err[n_coadd*n_wl] */
			float *add_signal,       /* Output: rad[n_wl] */
			float *add_signal_err)   /* Output: rad_err[n_wl] */
{
    int nw, nc;
    for (nw = 0; nw < (int) n_wl; nw++)
    {
	add_signal[nw] = 0.0;
	add_signal_err[nw] = 0.0;
	for (nc = 0; nc < (int)n_coadd; nc++)
	{
	    add_signal[nw] += signal[ nc*n_wl + nw ];
	    add_signal_err[nw] += signal_err[ nc*n_wl + nw ] * signal_err[ nc*n_wl + nw ];
	}
	add_signal[nw] = add_signal[nw] / (float)n_coadd;
				/* !!!!!!!!!!! Check coadding errors : OK*/
	add_signal_err[nw] = sqrt (add_signal_err[nw]) / (float)n_coadd;
    }
    return OK;
}

/*********************************************************************\
 * Basic routine for reading Nadir next wavelength window
 * kb 23.04.01
\*********************************************************************/

SCIA_err read_next_mds (info_l1c *info,
			   user_data *ud, L1_MDS_TYPE type)
{
    unsigned int n_pix, n_pix_tmp, n_cl, i_coadd, n_pmd, i;
//    unsigned int pix_start, pix_end;
    unsigned int n_tmp_pix, n_coadd;
    unsigned int n_ro, cur_pix, id;
    int n_state;
    int add_coadd;
    float *tmp_signal;
    float *tmp_signal_err;
    int n_absolut_pix;
    SCIA_err errflag;
    state_cluster_data *data;
#if defined(__DEBUG_L1C__)
	DEBUG_Print("read_next_mds: ");
	DEBUG_Print("cur_state_nr[]: %i mds_offset: %i  read_out %i\n",
		info->cur_state_nr[type], info->mds_offset[MDS_NADIR],
		info->cur_readout_in_state );
#endif
				/* if next state, read state data/info */
    if (info->cur_readout_in_state == 0)
    {
				/* Free previous allocated memory */
	clear_state(info, type);
	if (info->cur_state_nr[type] == info->n_states[type] ||
	    info->cur_mds_read[type] >= info->n_mds[type])
	    return END_OF_NADIR_STATES;
				/* read state in info->st_cl_data */
	read_next_state (info,type);
				/* define used clusters for this state */
	if ( (errflag = calc_cluster_window (info,type)) != OK)
	    return errflag;	/* wl-window - cluster mismatch */
				/* read pmds  */
//	read_next_pmd_state (info, type);
    }
    n_ro = info->cur_readout_in_state;
    n_state = info->idx_states[type][info->cur_state_nr[type]];

    ud->n_wl = info->sum_pix_cur_used_clusters;
				/* allocate memory */
    ud->pixel_nr =
	(unsigned short *) malloc (sizeof (unsigned short) * ud->n_wl);
    ud->pixel_cls =
	(unsigned short *) malloc (sizeof (unsigned short) * ud->n_wl);
    ud->pixel_ch =
	(unsigned short *) malloc (sizeof (unsigned short) * ud->n_wl);
    ud->pixel_coadd =
	(unsigned short *) malloc (sizeof (unsigned short) * ud->n_wl);
    ud->wl =          (float *)malloc (sizeof (float) * ud->n_wl);
    ud->wl_err =      (float *)malloc (sizeof (float) * ud->n_wl);
    ud->signal =      (float *)malloc (sizeof (float) * ud->n_wl);
    ud->signal_err =  (float *)malloc (sizeof (float) * ud->n_wl);

    if (info->n_pmd_mds[type] > 0)
    {
	ud->n_pmd = info->cur_num_pmd;
	ud->pmd = (user_data_pmd*) malloc (sizeof (user_data_pmd) * ud->n_pmd);
	switch (type)
	{
	    case MDS_OCC:
	    case MDS_LIMB:
		ud->pmd_geo_limb = (GeoL*) malloc (sizeof (GeoL) * ud->n_pmd);
		ud->pmd_geo_nadir = NULL;
		break;
	    case MDS_NADIR:
		ud->pmd_geo_nadir = (GeoN*) malloc (sizeof (GeoN) * ud->n_pmd);
		ud->pmd_geo_limb = NULL;
		break;
	    default:
		break;
	}
    }
    else
    {
	ud->n_pmd = 0;
	ud->pmd = NULL;
	ud->pmd_geo_limb = NULL;
	ud->pmd_geo_nadir = NULL;
    }
				/* collect data */
    cur_pix=0;
				/*  Loop over used clusters */
    for (n_cl=0; n_cl < (unsigned int) info->max_cur_used_clusters; n_cl++)
    {
	id = info->cur_used_cl[n_cl].id;
	data = info->st_cl_data + id;
//	pix_start = info->pix_start_cur_used_clusters[n_cl];
//	pix_end = info-sph->stop_long>pix_end_cur_used_clusters[n_cl];
//	n_tmp_pix = pix_end - pix_start;
	n_tmp_pix = info->cur_used_cl[n_cl].pix_n;
	n_coadd = info->cur_used_cl[n_cl].coadd;
				/* co_add signal / err */
	tmp_signal = (float*) malloc (sizeof(float) * n_tmp_pix * n_coadd);
	tmp_signal_err = (float*) malloc (sizeof(float) * n_tmp_pix * n_coadd);
#if defined(__DEBUG_L1C__)
	DEBUG_Print("read_next_mds: Clus: %d n_coadd: %d\n", id+1, n_coadd);
#endif
	for (i_coadd = 0; i_coadd < n_coadd; i_coadd++)
	{
	    for (n_pix=0, n_pix_tmp=0,
		     n_absolut_pix = info->cur_used_cl[n_cl].pix_start;
		 (int)n_pix < info->cur_used_cl[n_cl].pix_length;
		 n_pix++, n_absolut_pix++)
	    {
		if ( info->cur_pix_output_flag[n_absolut_pix] == 0 )
		    continue;
/* #if defined(__DEBUG_L1C__) */
/* 		DEBUG_Print("n_pix, data->n_wl, i_coadd , n_ro , n_coadd" */
/* 			"   %3i %3i %3i %3i %3i  -> %5i ( %4i )\n", */
/* 			n_pix, data->n_wl, i_coadd , n_ro, n_coadd, */
/* 			n_pix + data->n_wl*(i_coadd + n_ro*n_coadd), */
/* 			n_pix_tmp + n_tmp_pix*i_coadd); */
/* #endif		 */
		tmp_signal[n_tmp_pix*i_coadd + n_pix_tmp] =
		    data->signal[n_pix +
				 data->n_wl* (i_coadd + n_ro*n_coadd)];
				/*cur     cur coadd           readouts */
		tmp_signal_err[n_tmp_pix*i_coadd + n_pix_tmp] =
		    data->signal_err[n_pix +
				     data->n_wl * (i_coadd + n_ro*n_coadd)];
		n_pix_tmp++;
	    }
	}
	coadd_signal (n_tmp_pix, n_coadd, tmp_signal, tmp_signal_err,
		      ud->signal+cur_pix, ud->signal_err+cur_pix);
	free (tmp_signal);
	free (tmp_signal_err);
				/* collect other parts, identical for all readouts */
	for (n_pix=0, n_pix_tmp=0,
		 n_absolut_pix = info->cur_used_cl[n_cl].pix_start;
	     (int)n_pix < info->cur_used_cl[n_cl].pix_length;
	     n_pix++, n_absolut_pix++)
	{
	    if ( info->cur_pix_output_flag[n_absolut_pix] == 0 )
		continue;
	    ud->pixel_nr [ cur_pix+n_pix_tmp ] = data->pixel[n_pix];
	    ud->pixel_cls [ cur_pix+n_pix_tmp ] = id + 1;
	    ud->pixel_ch [ cur_pix+n_pix_tmp ] =
		info->ads_states[n_state].Clcon[id].channel;

	    ud->pixel_coadd [ cur_pix+n_pix_tmp ] = n_coadd;
	    ud->wl [ cur_pix+n_pix_tmp ] = data->wl[n_pix];
	    ud->wl_err [ cur_pix+n_pix_tmp ] = data->wl_err[n_pix];
	    n_pix_tmp++;
	}
	cur_pix += n_tmp_pix;
    }
    add_coadd = info->cur_it / info->max_int_time;

    ud->type = type;

    switch (type) {

      case MDS_NADIR:
	  if (add_coadd > 1)
	  {
	       if ( GeoN_add
		  (info->st_cl_data[info->geo_cur_used_clusters].geo_nadir,
		   info->cur_it, add_coadd, n_ro, &(ud->geo_nadir) )
		    != OK )
	       {
		   fprintf (stderr, "Error from GeoN_add:\n add_coadd = %d\n"
			    " info->cur_it = %d\n info->max_int_time = %d\n",
			    add_coadd, info->cur_it, info->max_int_time);
		   exit (1);
	       }
	  }
	  else
	  {
	      ud->geo_nadir = info->st_cl_data
		  [info->geo_cur_used_clusters].geo_nadir[n_ro];
	  }
	  break;

      case MDS_LIMB:
      case MDS_OCC:
	  if (add_coadd > 1)
	  {
	      if ( GeoL_add
		  (info->st_cl_data[info->geo_cur_used_clusters].geo_limb,
		   info->cur_it, add_coadd, n_ro, &(ud->geo_limb) )
		   != OK )
	      {
		  fprintf (stderr, "Error from GeoL_add:\n add_coadd = %d\n"
			   " info->cur_it = %d\n info->max_int_time = %d\n",
			   add_coadd, info->cur_it, info->max_int_time);
		  exit (1);
	      }
	  }
	  else
	  {
	      ud->geo_limb = info->st_cl_data[info->geo_cur_used_clusters].geo_limb[n_ro];
	  }
	  break;

      case MDS_MON:
	  if (add_coadd > 1)
	  {
	      if ( GeoCal_add
		  (info->st_cl_data[info->geo_cur_used_clusters].geo_cal,
		   info->cur_it, add_coadd, n_ro, &(ud->geo_cal) )
		   != OK )
	      {
		  fprintf (stderr, "Error from GeoN_add:\n add_coadd = %d\n"
			   " info->cur_it = %d\n info->max_int_time = %d\n",
			   add_coadd, info->cur_it, info->max_int_time);
		  exit (1);
	      }
	  }
	  else
	  {
	  ud->geo_cal = info->st_cl_data[info->geo_cur_used_clusters].geo_cal[n_ro];
	  }
	  break;
      default:
	  ;
  }

/***********************************************************************
Description of Limb/Occultation Time calculations (IODD 0-1, p 57)
------------------------------------------------------------------------
    The measurement duration of a Limb state is 59.0625 seconds. This
    may be calculated as follows: one azimuth scan has a net
    integration time of 1.5 seconds and the adjustment of the
    elevation mirror for the next azimuth scan takes 3 BCPSs
    corresponding to 0.1875 seconds giving a total of 1.6875 seconds
    for one azimuth scan. 34 azimuth scan plus one dark measurement
    into deep space (which takes the same time as one azimuth scan)
    gives a state duration of: 35 x 1.6875 s = 59.0625 s.
**********************************************************************/
/* change in 2003 via OCR to improve limb-nadir matching:
   further on only 30 azimuth scan plus one dark measurement,
    gives a state duration of: 31 x 1.6875 s = 52.3125 s (837BCPS)
                duration_scan_state = 836 BCPS
 since 2005(?):  additional category 26 available: limb mesosphere on eclipse:
   here we have 24 azimuth scans : 24*1.6875s = 40.5 s (648 BCPS)
                duration_scan_state = 647 BCPS
**********************************************************************/

    /* Calculate date */
    switch (type)
    {
	case MDS_OCC:
	case MDS_NADIR:
	case MDS_MON:
	    ud->mjd = MJD_add(info->ads_states[n_state].StartTime ,
			      info->cur_it * n_ro);
	    break;
	case MDS_LIMB:
	    ud->mjd = MJD_add(info->ads_states[n_state].StartTime ,
			      info->cur_it * n_ro +
			      ((info->cur_it * n_ro) / SCAN_LIMB_DURATION)
			      * SCAN_LIMB_STEP_DURATION);
	    break;

	default:
	    ud->mjd =  info->ads_states[n_state].StartTime;
	    break;
    }
    UTC_String (&ud->mjd, ud->date);
    ud->int_time = info->cur_it / 16.0;

    /* Copy state info  */
    ud->state_id = info->ads_states[n_state].state_id;
    ud->category = info->ads_states[n_state].category;
    UTC_String (&info->ads_states[n_state].StartTime,
		ud->state_date) ;
    for (i=0; i<4; i++)
	ud->state_corner[i] =
	    info->ads_states_geolocation[n_state].coord_grd[i];

    /* Copy orbit info */
    ud->product            = info->user_file_info.product ;
    ud->software_ver       = info->user_file_info.software_ver ;
    /*ud->abs_orbit          = info->user_file_info.abs_orbit;   */
    /* Orbit of file is not reliable (NRT-products not consolidated) */
    /* calculate orbit from state start time */
    ud->abs_orbit          = orbit_mjd (&info->ads_states[n_state].StartTime);
    ud->key_data_version   = info->user_file_info.key_data_version ;
    ud->m_factor_version   = info->user_file_info.m_factor_version ;
    ud->init_version       = info->user_file_info.init_version;     /* Version Initialisation files */
    ud->decont             = info->user_file_info.decont      ;           /* Decontamination flag */
    ud->l1b_product_name   = info->user_file_info.l1b_product_name;
    ud->cal_applied        = info->user_file_info.cal_applied;

    /*  Copy PMD data */
    if (info->n_pmd_mds[type] > 0)
	for (n_pmd=0; n_pmd < ud->n_pmd; n_pmd++)
	{
	    for (i=0; i< 7; i++)
	    {
		ud->pmd[n_pmd].pmd[i] =
		    info->nadir_pmd_data.int_pmd [n_ro * ud->n_pmd * 7
						  + n_pmd*7 + i];
	    }
	    /* Copy PMD geolocation  */
	    switch (type)
	    {
		case MDS_OCC:
		case MDS_LIMB:
		    ud->pmd_geo_limb[n_pmd] =
			info->nadir_pmd_data.geo_limb[n_ro * ud->n_pmd
						      + n_pmd ];
		    break;
		case MDS_NADIR:
		    ud->pmd_geo_nadir[n_pmd] =
			info->nadir_pmd_data.geo_nadir[n_ro * ud->n_pmd
						      + n_pmd ];
		    break;
		default:
		    break;
	    }

	}
    /* Copy counter  */
    ud->n_readout = info->cur_readout_in_state;
    ud->n_state = info->cur_state_nr[type];
    /*  update counter for readout in state */
    info->cur_readout_in_state = (info->cur_readout_in_state+1) %
	info->cur_max_readout_in_state;
    /* update state counter, if necessary */
    if (  info->cur_readout_in_state == 0 )
	info->cur_state_nr[type]++;
    return OK;
}

/* if outside a problem with current state is detected */
/*  this routine skips further readout of state and  */
/*   starts with next one */
SCIA_err skip_cur_state (info_l1c *info, L1_MDS_TYPE type)
{
    info->cur_readout_in_state = 0;
    info->cur_state_nr[type]++;
    return OK;
}


/* Read PMD data for one state */
SCIA_err read_next_pmd_state (info_l1c *info, L1_MDS_TYPE type)

{
    if (info->n_pmd_mds[type] <= 0) {
	return NO_PMD_DATA;
    }
// SCIA !!!    length =  Read_PMD_MDS (info->FILE_l1c, info->mds_pmd_offset[type],
// SCIA !!!			    &info->nadir_pmd_data, type);

// SCIA !!!    info->mds_pmd_offset[type] += length;

    return OK;
}



// unsigned long Read_PMD_MDS (FILE* unit,
// 			    long int offset,
// 			    Nadir_Pmd *pmd,
// 			    L1_MDS_TYPE type)
// {
//     				/* go to start of MDS */
//     fseek (unit, offset,  SEEK_SET);
// 				/*  */
//     MJD_getbin (unit, &pmd->dsr_time);
//     uint_getbin (unit, &pmd->dsr_length);
//     char_getbin (unit, &pmd->quality_flag);
//     float_getbin (unit, &pmd->orb_phase);
//     ushort_getbin (unit, &pmd->meas_cat);
//     ushort_getbin (unit, &pmd->state_id);
//     ushort_getbin (unit, &pmd->dur_scan_phase );
//     ushort_getbin (unit, &pmd->num_pmd);
//     ushort_getbin (unit, &pmd->num_geo);
//     pmd->int_pmd = (float*) malloc (pmd->num_pmd * sizeof(float));
//
//     float_array_getbin (unit, pmd->int_pmd, pmd->num_pmd);
//     /* to be completed .... */
//
//     switch (type) {
//
// 	case MDS_NADIR:
// 	    pmd->geo_nadir = (GeoN*) malloc (pmd->num_pmd * sizeof(GeoN));
// 	    GeoN_array_getbin (unit, pmd->geo_nadir, pmd->num_geo);
// 	    break;
// 	case MDS_LIMB:
// 	case MDS_OCC:
// 	    pmd->geo_limb = (GeoL*) malloc (pmd->num_pmd * sizeof(GeoL));
// 	    GeoL_array_getbin (unit, pmd->geo_limb, pmd->num_geo);
// 	    break;
// 	case MDS_MON:
// 	default:
// 	  ;
//     }
//     return pmd->dsr_length;
// }


/* hopefully outdated... */
SCIA_err Read_DSD_1C (FILE* unit, DSD *dsd)
{
    char *str_ptr;
    fscanf(unit,
	   "DS_NAME=\"%28c\"\n"
	   "DS_TYPE=%1c\n"
	   "FILENAME=\"%62c\"\n"
	   "DS_OFFSET=+%20u<bytes>\n"
	   "DS_SIZE=%21u<bytes>\n"
	   "NUM_DSR=%11u\n"
	   "DSR_SIZE=%11d<bytes>\n",
	   dsd->name,
	   &dsd->type,
	   dsd->filename,
	   &dsd->offset,
	   &dsd->size,
	   &dsd->num_dsr,
	   &dsd->dsr_size);

    dsd->name[28]='\0';
    if ( (str_ptr = strpbrk(dsd->name, " ") ) != NULL)
	*str_ptr='\0';

    dsd->filename[62]='\0';
    if ( (str_ptr = strpbrk(dsd->filename, " ") ) != NULL)
	*str_ptr='\0';

    return OK;
}

/* ***************** */

SCIA_err GeoL_add (GeoL *geo, /* array with 1c geolocations */
		  int integr_time, /* integr_time of a single pixel */
		  int n_coadd,	   /* how often to "add" */
		    int n_readout, /* readout counter for current state */
		  GeoL *geo_coadd) /* collect result here */
{


    int i,n0,n1,n2;
    int geo_n = n_coadd*n_readout;

    geo_coadd->sat_height = 0;
    geo_coadd->earth_radius = 0;
    for (i=0; i< n_coadd; i++)
    {
	geo_coadd->sat_height += geo[geo_n+i].sat_height;
	geo_coadd->earth_radius += geo[geo_n+i].earth_radius;
    }
    geo_coadd->sat_height /= n_coadd;
    geo_coadd->earth_radius /= n_coadd;

    switch (n_coadd)
    {
		/*  coadd two geolocations */
		/*  old: 0--1--2/0--1--2   */
		/*  new: 0------1------2   */
	case 2:
	case 4:
	case 6:
	case 8:
	case 12:
	case 24:
		n0 = geo_n;
		n1 = geo_n + n_coadd/2 -1;
		n2 = geo_n + n_coadd -1;
		/*  angles */
		geo_coadd->sza_toa[0] =
		    geo[n0].sza_toa[0];
		geo_coadd->sza_toa[1] =
		    geo[n1].sza_toa[2];
		geo_coadd->sza_toa[2] =
		    geo[n2].sza_toa[2];

		geo_coadd->los_zen[0] =
		    geo[n0].los_zen[0];
		geo_coadd->los_zen[1] =
		    geo[n1].los_zen[2];
		geo_coadd->los_zen[2] =
		    geo[n2].los_zen[2];

		geo_coadd->saa_toa[0] =
		    geo[n0].saa_toa[0];
		geo_coadd->saa_toa[1] =
		    geo[n1].saa_toa[2];
		geo_coadd->saa_toa[2] =
		    geo[n2].saa_toa[2];

		geo_coadd->los_azi[0] =
		    geo[n0].los_azi[0];
		geo_coadd->los_azi[1] =
		    geo[n1].los_azi[2];
		geo_coadd->los_azi[2] =
		    geo[n2].los_azi[2];

		geo_coadd->tangent_ground_point[0] =
		    geo[n0].tangent_ground_point[0];
		geo_coadd->tangent_ground_point[1] =
		    geo[n1].tangent_ground_point[2];
		geo_coadd->tangent_ground_point[2] =
		    geo[n2].tangent_ground_point[2];

		geo_coadd->tangent_height[0] =
		    geo[n0].tangent_height[0];
/* 		geo_coadd->tangent_height[1] = */
/* 		    geo[n1].tangent_height[2]; */
		/* in case of TH recalculation with CFI,
		   only mid THs are calculated. Start and end TH set to mid TH.
		   Therfore, mid TH has to be the mean of left and right mid TH ..*/
 		geo_coadd->tangent_height[1] =
 		    (geo[n1].tangent_height[1] + geo[n1+1].tangent_height[1]) / 2.0;

		geo_coadd->tangent_height[2] = geo[n2].tangent_height[2];

/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->sub_sat =
		    middle_coord (geo[n1].sub_sat,
				  geo[n1+1].sub_sat);

		geo_coadd->doppler_shift =
		    (geo[n1].doppler_shift +  geo[n1+1].doppler_shift)/2.0;

		geo_coadd->esm_pos =
		    (geo[n1].esm_pos + geo[n1+1].esm_pos)/2.0;

		geo_coadd->asm_pos =
		    (geo[n1].asm_pos + geo[n1+1].asm_pos)/2.0;
		break;
		/* 3 is not even (24->8) */
		/*  coadd two geolocations */
		/*  old: 0--1--2/0--1--2/0--1--2 */
		/*  new: 0----------1----------2 */
	case 3:
		n0 = geo_n;
		n1 = geo_n + 1 ;
		n2 = geo_n + 2 ;
		/*  angles */
		geo_coadd->sza_toa[0] =
		    geo[n0].sza_toa[0];
		geo_coadd->sza_toa[1] =
		    geo[n1].sza_toa[1];
		geo_coadd->sza_toa[2] =
		    geo[n2].sza_toa[2];

		geo_coadd->los_zen[0] =
		    geo[n0].los_zen[0];
		geo_coadd->los_zen[1] =
		    geo[n1].los_zen[1];
		geo_coadd->los_zen[2] =
		    geo[n2].los_zen[2];

		geo_coadd->saa_toa[0] =
		    geo[n0].saa_toa[0];
		geo_coadd->saa_toa[1] =
		    geo[n1].saa_toa[1];
		geo_coadd->saa_toa[2] =
		    geo[n2].saa_toa[2];

		geo_coadd->los_azi[0] =
		    geo[n0].los_azi[0];
		geo_coadd->los_azi[1] =
		    geo[n1].los_azi[1];
		geo_coadd->los_azi[2] =
		    geo[n2].los_azi[2];

		geo_coadd->tangent_ground_point[0] =
		    geo[n0].tangent_ground_point[0];
		geo_coadd->tangent_ground_point[1] =
		    geo[n1].tangent_ground_point[1];
		geo_coadd->tangent_ground_point[2] =
		    geo[n2].tangent_ground_point[2];

		geo_coadd->tangent_height[0] =
		    geo[n0].tangent_height[0];
		geo_coadd->tangent_height[1] =
		    geo[n1].tangent_height[1];
		geo_coadd->tangent_height[2] =
		    geo[n2].tangent_height[2];

/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->sub_sat = geo[n1].sub_sat;

		geo_coadd->doppler_shift = geo[n1].doppler_shift;

		geo_coadd->esm_pos = geo[n1].esm_pos;

		geo_coadd->asm_pos = geo[n1].asm_pos;
		break;

	default:
	    fprintf (stderr, "Integration time of %d not implemented for Limb!",
		     integr_time);

	    return SCIA_ERROR; //SCIA_PRODUCT_INCONSISTENCY;
	    break;
    }
    return OK;
}


SCIA_err GeoCal_add (GeoCal *geo, /* array with 1c geolocations */
		  int integr_time, /* integr_time of a single pixel */
		  int n_coadd,	   /* how often to "add" */
		    int n_readout, /* readout counter for current state */
		  GeoCal *geo_coadd) /* collect result here */
{


    int i,n1;
    int geo_n = n_coadd*n_readout;

    geo_coadd->esm_pos = 0;
    geo_coadd->asm_pos = 0;
    geo_coadd->sza     = 0;
    for (i=0; i< n_coadd; i++)
    {
	geo_coadd->esm_pos += geo[geo_n+i].esm_pos;
	geo_coadd->asm_pos += geo[geo_n+i].asm_pos;
	geo_coadd->sza     += geo[geo_n+i].sza;
    }
    geo_coadd->esm_pos   /= n_coadd;
    geo_coadd->esm_pos   /= n_coadd;
    geo_coadd->sza       /= n_coadd;

    switch (n_coadd)
    {
		/*  coadd two geolocations */
		/*  old: 0--1--2/0--1--2   */
		/*  new: 0------1------2   */
	case 2:
	case 4:
	case 8:
		n1 = geo_n + n_coadd/2 -1;
		/*  angles */

/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->sub_sat =
		    middle_coord (geo[n1].sub_sat,
				  geo[n1+1].sub_sat);
		break;
	default:
	    return SCIA_ERROR; //SCIA_PRODUCT_INCONSISTENCY;
	    break;
    }
    return OK;
}



SCIA_err GeoN_add (GeoN *geo, /* array with 1c geolocations */
		  int integr_time, /* integr_time of a single pixel */
		  int n_coadd,	   /* how often to "add" */
		    int n_readout, /* readout counter for current state */
		  GeoN *geo_coadd) /* collect result here */
{


    int i,n0,n1,n2,nd;
    int geo_n = n_coadd*n_readout;

    geo_coadd->sat_height = 0;
    geo_coadd->earth_radius = 0;
    for (i=0; i< n_coadd; i++)
    {
	geo_coadd->esm_pos += geo[geo_n+i].esm_pos;
	geo_coadd->sat_height += geo[geo_n+i].sat_height;
	geo_coadd->earth_radius += geo[geo_n+i].earth_radius;
    }
    geo_coadd->esm_pos /= n_coadd;
    geo_coadd->sat_height /= n_coadd;
    geo_coadd->earth_radius /= n_coadd;
/* All possibilities less than 16 (1s) */
    /* No complications with integration of backscan, at least
       5 readouts for complete scan sequence */
    if (integr_time <= 16)
    {
	/* Set angles and coordinates */
	switch (n_coadd)
	{
	    case 1:
		/*  angles*/
		for (i=0; i<3; i++)
		{
		    geo_coadd->sza_toa[i] =
			geo[geo_n].sza_toa[i];
		    geo_coadd->los_zen[i] =
			geo[geo_n].los_zen[i];
		    geo_coadd->saa_toa[i] =
			geo[geo_n].saa_toa[i];
		    geo_coadd->los_azi[i] =
			geo[geo_n].los_azi[i];
		}
		/*  coordinates */
		geo_coadd->centre_coord = geo[geo_n].centre_coord;
		for (i=0; i<4; i++)
		    geo_coadd->corner_coord[i] = geo[geo_n].corner_coord[i];
		break;
		/*  coadd two geolocations */
		/*  old: 0--1--2/0--1--2   */
		/*  new: 0------1------2   */
	    case 2:
	    case 4:
	    case 8:
	    case 16:
		n0 = geo_n;
		n1 = geo_n + n_coadd/2 -1;
		n2 = geo_n + n_coadd -1;
		/*  angles */
		geo_coadd->sza_toa[0] =
		    geo[n0].sza_toa[0];
		geo_coadd->sza_toa[1] =
		    geo[n1].sza_toa[2];
		geo_coadd->sza_toa[2] =
		    geo[n2].sza_toa[2];

		geo_coadd->los_zen[0] =
		    geo[n0].los_zen[0];
		geo_coadd->los_zen[1] =
		    geo[n1].los_zen[2];
		geo_coadd->los_zen[2] =
		    geo[n2].los_zen[2];

		geo_coadd->saa_toa[0] =
		    geo[n0].saa_toa[0];
		geo_coadd->saa_toa[1] =
		    geo[n1].saa_toa[2];
		geo_coadd->saa_toa[2] =
		    geo[n2].saa_toa[2];

		geo_coadd->los_azi[0] =
		    geo[n0].los_azi[0];
		geo_coadd->los_azi[1] =
		    geo[n1].los_azi[2];
		geo_coadd->los_azi[2] =
		    geo[n2].los_azi[2];
/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->centre_coord =
		    middle_coord (geo[n1].centre_coord,
				  geo[n1+1].centre_coord);
		geo_coadd->sub_sat =
		    middle_coord (geo[n1].sub_sat,
				  geo[n1+1].sub_sat);

		geo_coadd->esm_pos =
		    (geo[n1].esm_pos + geo[n1+1].esm_pos)/2.0;
		/*  corners */
		geo_coadd->corner_coord[0] = geo[n0].corner_coord[0];
		geo_coadd->corner_coord[1] = geo[n0].corner_coord[1];
		geo_coadd->corner_coord[2] = geo[n2].corner_coord[2];
		geo_coadd->corner_coord[3] = geo[n2].corner_coord[3];
		break;
	    default:
		return SCIA_ERROR; //SCIA_PRODUCT_INCONSISTENCY;
		break;
	}
	return OK;
    }
    /* Integration times larger the 16 (1s) */
    if (integr_time == 160 )
    {
	/*  subset_counter here is -2 */
	switch (n_coadd)
	{
	    case 10:
	    case 20:
	    case 40:
	    case 80:
	    case 160:
		/*
		   |  nd  |
		n0-0--1--2/0--1--2/0--1--2/0--1--2
                   2--------------1--------------0
                n1-0--1--2/0--1--2/0--1--2/0--1--2
                   2--------------1--------------0
		*/

		n0 = geo_n;	/*  */
		nd = n_coadd / 10; /* n_delta */
		n1 = geo_n + nd*5 ;
		/* angles */
		geo_coadd->sza_toa[0] =
		    (geo[ n0 ].sza_toa[0]
		     + geo[ n1 ].sza_toa[0]) / 2;
		geo_coadd->sza_toa[1] =
		    (geo[ n0 + 2*nd - 1 ].sza_toa[2]
		     + geo[ n1 + 2*nd - 1 ].sza_toa[2]) / 2;
		geo_coadd->sza_toa[2] =
		    (geo[ n0 + 4*nd-1 ].sza_toa[2]
		     + geo[ n1 + 4*nd-1 ].sza_toa[2]) / 2;

		geo_coadd->los_zen[0] =
		    (geo[ n0 ].los_zen[0]
		     + geo[ n1 ].los_zen[0]) / 2;
		geo_coadd->los_zen[1] =
		    (geo[ n0 + 2*nd - 1 ].los_zen[2]
		     + geo[ n1 + 2*nd - 1 ].los_zen[2]) / 2;
		geo_coadd->los_zen[2] =
		    (geo[ n0 + 4*nd-1 ].los_zen[2]
		     + geo[ n1 + 4*nd-1 ].los_zen[2]) / 2;

		geo_coadd->saa_toa[0] =
		    (geo[ n0 ].saa_toa[0]
		     + geo[ n1 ].saa_toa[0]) / 2;
		geo_coadd->saa_toa[1] =
		    (geo[ n0 + 2*nd - 1 ].saa_toa[2]
		     + geo[ n1 + 2*nd - 1 ].saa_toa[2]) / 2;
		geo_coadd->saa_toa[2] =
		    (geo[ n0 + 4*nd-1 ].saa_toa[2]
		     + geo[ n1 + 4*nd-1 ].saa_toa[2]) / 2;

		geo_coadd->los_azi[0] =
		    (geo[ n0 ].los_azi[0]
		     + geo[ n1 ].los_azi[0]) / 2;
		geo_coadd->los_azi[1] =
		    (geo[ n0 + 2*nd - 1 ].los_azi[2]
		     + geo[ n1 + 2*nd - 1 ].los_azi[2]) / 2;
		geo_coadd->los_azi[2] =
		    (geo[ n0 + 4*nd-1 ].los_azi[2]
		     + geo[ n1 + 4*nd-1 ].los_azi[2]) / 2;
		/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->centre_coord = geo[ n0 + 2*nd - 1 ].corner_coord[3];
		/* sub-satellite point: mean point between first and last */
		geo_coadd->sub_sat = middle_coord
		    (geo[n0].sub_sat,
		     geo[n1].sub_sat);

		/*  corners */
		geo_coadd->corner_coord[0] = geo[n0].corner_coord[0];
		geo_coadd->corner_coord[1] = geo[n1].corner_coord[1];
		geo_coadd->corner_coord[2] = geo[ n0 + 4*nd-1 ].corner_coord[2];
		geo_coadd->corner_coord[3] = geo[ n1 + 4*nd-1 ].corner_coord[3];
		break;
	    case 1:
		return SCIA_ERROR; //SCIA_GEO_INTEGR_TIME_NOT_IMPLEMENTED;
		break;
	    default:
		return SCIA_ERROR; //SCIA_PRODUCT_INCONSISTENCY;
		break;
	}
	return OK;
    }

    /* 80s IT time - not used in products, but sometimes needed for
       co-add by hand */
    if (integr_time == 80 )
    {
	/*  subset_counter here is -2 */
	switch (n_coadd)
	{
	    case 5:
	    case 10:
	    case 20:
	    case 40:
	    case 80:
		n0 = geo_n;	/*  */
		nd = n_coadd / 5; /* n_delta */

		/* angles */
		geo_coadd->sza_toa[0] =
		    geo[ n0 ].sza_toa[0];
		geo_coadd->sza_toa[1] =
		    geo[ n0 + 2*nd - 1 ].sza_toa[2];
		geo_coadd->sza_toa[2] =
		    geo[ n0 + 4*nd-1 ].sza_toa[2];

		geo_coadd->los_zen[0] =
		    geo[ n0 ].los_zen[0];
		geo_coadd->los_zen[1] =
		    geo[ n0 + 2*nd - 1 ].los_zen[2];
		geo_coadd->los_zen[2] =
		    geo[ n0 + 4*nd-1 ].los_zen[2];

		geo_coadd->saa_toa[0] =
		    geo[ n0 ].saa_toa[0];
		geo_coadd->saa_toa[1] =
		    geo[ n0 + 2*nd - 1 ].saa_toa[2];
		geo_coadd->saa_toa[2] =
		    geo[ n0 + 4*nd-1 ].saa_toa[2];

		geo_coadd->los_azi[0] =
		    geo[ n0 ].los_azi[0];
		geo_coadd->los_azi[1] =
		    geo[ n0 + 2*nd - 1 ].los_azi[2];
		geo_coadd->los_azi[2] =
		    geo[ n0 + 4*nd-1 ].los_azi[2];

		/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->centre_coord = middle_coord (
		    geo[ n0 + 2*nd - 1 ].corner_coord[2],
		    geo[ n0 + 2*nd - 1 ].corner_coord[3]);
		/* sub-satellite point: mean point between first and last */
		geo_coadd->sub_sat = middle_coord
		    (geo[n0].sub_sat,
		     geo[4*nd-1].sub_sat);

		/*  corners */
		geo_coadd->corner_coord[0] = geo[n0].corner_coord[0];
		geo_coadd->corner_coord[1] = geo[n0].corner_coord[1];
		geo_coadd->corner_coord[2] = geo[ n0 + 4*nd-1 ].corner_coord[2];
		geo_coadd->corner_coord[3] = geo[ n0 + 4*nd-1 ].corner_coord[3];
		break;
	    case 1:
		return SCIA_ERROR; //SCIA_GEO_INTEGR_TIME_NOT_IMPLEMENTED;
		break;
	    default:
		return SCIA_ERROR; //SCIA_PRODUCT_INCONSISTENCY;
		break;
	}
	return OK;
    }

	    /* Integration times larger the 16 (1s) */

    if (integr_time == 1040 )
    {
	/*  subset_counter here is -2 */
	switch (n_coadd)
	{
	    case 65:
	    case 130:
	    case 260:
	    case 520:
	    case 1040:
	    case 2080:
		/*
		   |  nd  |
		n0-0--1--2/0--1--2/0--1--2/0--1--2
                   2--------------1--------------0
                n1-0--1--2/0--1--2/0--1--2/0--1--2
                   2--------------1--------------0
		*/

		nd = n_coadd / 5; /* n_delta */
		n0 = geo_n + nd*65;	/*  */

		/* angles */
		geo_coadd->sza_toa[0] =
		    geo[ n0 ].sza_toa[0];
		geo_coadd->sza_toa[1] =
		    geo[ n0 + 2*nd - 1 ].sza_toa[2];
		geo_coadd->sza_toa[2] =
		    geo[ n0 + 4*nd-1 ].sza_toa[2];

		geo_coadd->los_zen[0] =
		    geo[ n0 ].los_zen[0];
		geo_coadd->los_zen[1] =
		    geo[ n0 + 2*nd - 1 ].los_zen[2];
		geo_coadd->los_zen[2] =
		    geo[ n0 + 4*nd-1 ].los_zen[2];

		geo_coadd->saa_toa[0] =
		    geo[ n0 ].saa_toa[0];
		geo_coadd->saa_toa[1] =
		    geo[ n0 + 2*nd - 1 ].saa_toa[2];
		geo_coadd->saa_toa[2] =
		    geo[ n0 + 4*nd-1 ].saa_toa[2];

		geo_coadd->los_azi[0] =
		    geo[ n0 ].los_azi[0];
		geo_coadd->los_azi[1] =
		    geo[ n0 + 2*nd - 1 ].los_azi[2];
		geo_coadd->los_azi[2] =
		    geo[ n0 + 4*nd-1 ].los_azi[2];

		/*  coordinates */
		/*  should be replaced by a more
		    sophisticated approach */
		geo_coadd->centre_coord = middle_coord (
		    geo[ n0 + 2*nd - 1 ].corner_coord[2],
		    geo[ n0 + 2*nd - 1 ].corner_coord[3]);
		/* sub-satellite point: mean point between first and last */
		geo_coadd->sub_sat = middle_coord
		    (geo[n0].sub_sat,
		     geo[4*nd-1].sub_sat);

		/*  corners */
		geo_coadd->corner_coord[0] = geo[n0].corner_coord[0];
		geo_coadd->corner_coord[1] = geo[n0].corner_coord[1];
		geo_coadd->corner_coord[2] = geo[ n0 + 4*nd-1 ].corner_coord[2];
		geo_coadd->corner_coord[3] = geo[ n0 + 4*nd-1 ].corner_coord[3];
		break;
	    case 1:
		return SCIA_ERROR; //SCIA_GEO_INTEGR_TIME_NOT_IMPLEMENTED;
		break;
	    default:
		return SCIA_ERROR; //SCIA_PRODUCT_INCONSISTENCY;
		break;
	}
	return OK;
    }



    return SCIA_ERROR; //SCIA_GEO_INTEGR_TIME_NOT_IMPLEMENTED;
}

