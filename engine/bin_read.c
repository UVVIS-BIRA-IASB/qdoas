
#include <stdlib.h>
#include <stdio.h>

#include "comdefs.h"
#include "bin_read.h"

void swap_bytes_float(unsigned char *var)
 {
  unsigned char vtmp;

  vtmp=var[0];var[0]=var[3];var[3]=vtmp;
  vtmp=var[1];var[1]=var[2];var[2]=vtmp;
 }

void swap_bytes_short(unsigned char *var)
 {
  unsigned char vtmp;

  vtmp=var[0];var[0]=var[1];var[1]=vtmp;
 }

void swap_bytes_int(unsigned char *var)
 {
  unsigned char vtmp;

  vtmp=var[0];var[0]=var[3];var[3]=vtmp;
  vtmp=var[1];var[1]=var[2];var[2]=vtmp;
 }

void ushort_array_getbin (FILE* unit, unsigned short *var, int nr)
 {
 	int i;

 	fread(var,sizeof(unsigned short)*nr,1,unit);

 	for (i=0;i<nr;i++)
 	 swap_bytes_short((unsigned char *)&var[i]);
 }

void float_array_getbin (FILE* unit, float *var, int nr)
 {
 	int i;

 	fread(var,sizeof(float)*nr,1,unit);

 	for (i=0;i<nr;i++)
 	 swap_bytes_float((unsigned char *)&var[i]);
 }

void GeoN_array_getbin (FILE* unit, GeoN *var, int nr)
 {
  int n,i;

  fread(var,sizeof(GeoN)*nr,1,unit);

  #if defined(__LITTLE_ENDIAN__)

  for (n=0; n<nr; n++)
   {
    swap_bytes_float((unsigned char *)&var[n].esm_pos);

    for (i=0;i<3;i++)
     {
      swap_bytes_float((unsigned char *)&var[n].sza_toa[i]);
      swap_bytes_float((unsigned char *)&var[n].saa_toa[i]);
      swap_bytes_float((unsigned char *)&var[n].los_zen[i]);
      swap_bytes_float((unsigned char *)&var[n].los_azi[i]);
     }

    swap_bytes_float((unsigned char *)&var[n].sat_height);
    swap_bytes_float((unsigned char *)&var[n].earth_radius);

    for (i=0;i<4;i++)
     {
      swap_bytes_int((unsigned char *)&var[n].corner_coord[i].lat);
      swap_bytes_int((unsigned char *)&var[n].corner_coord[i].lon);
     }

    swap_bytes_int((unsigned char *)&var[n].sub_sat.lat);
    swap_bytes_int((unsigned char *)&var[n].sub_sat.lon);
    swap_bytes_int((unsigned char *)&var[n].centre_coord.lat);
    swap_bytes_int((unsigned char *)&var[n].centre_coord.lon);
   }

  #endif
 }


void GeoL_array_getbin (FILE* unit, GeoL *var, int nr)
 {
  int n,i;

  fread(var,sizeof(GeoL)*nr,1,unit);

  #if defined(__LITTLE_ENDIAN__)

  for (n=0; n<nr; n++)
   {
    swap_bytes_float((unsigned char *)&var[n].esm_pos);
    swap_bytes_float((unsigned char *)&var[n].asm_pos);

    for (i=0;i<3;i++)
     {
      swap_bytes_float((unsigned char *)&var[n].sza_toa[i]);
      swap_bytes_float((unsigned char *)&var[n].saa_toa[i]);
      swap_bytes_float((unsigned char *)&var[n].los_zen[i]);
      swap_bytes_float((unsigned char *)&var[n].los_azi[i]);
      swap_bytes_float((unsigned char *)&var[n].tangent_height[i]);

      swap_bytes_int((unsigned char *)&var[n].tangent_ground_point[i].lat);
      swap_bytes_int((unsigned char *)&var[n].tangent_ground_point[i].lon);
     }

    swap_bytes_float((unsigned char *)&var[n].sat_height);
    swap_bytes_float((unsigned char *)&var[n].earth_radius);
    swap_bytes_float((unsigned char *)&var[n].doppler_shift);

    swap_bytes_int((unsigned char *)&var[n].sub_sat.lat);
    swap_bytes_int((unsigned char *)&var[n].sub_sat.lon);
   }
  #endif
 }


// void cal_options_GADS_getbin (FILE* unit, cal_options_GADS *var)
void cal_options_GADS_getbin (FILE* unit, CAL_OPTIONS_GADS *var)
 {
  int i;

  fread(var,sizeof(CAL_OPTIONS_GADS),1,unit);

  #if defined(__LITTLE_ENDIAN__)

   swap_bytes_int((unsigned char *)&var->start_lat);
   swap_bytes_int((unsigned char *)&var->start_lon);
   swap_bytes_int((unsigned char *)&var->end_lat);
   swap_bytes_int((unsigned char *)&var->end_lon);

   swap_bytes_int((unsigned char *)&var->start_time.days);
   swap_bytes_int((unsigned char *)&var->start_time.secnd);
   swap_bytes_int((unsigned char *)&var->start_time.musec);

   swap_bytes_int((unsigned char *)&var->stop_time.days);
   swap_bytes_int((unsigned char *)&var->stop_time.secnd);
   swap_bytes_int((unsigned char *)&var->stop_time.musec);

   for (i=0;i<5;i++)
    swap_bytes_short((unsigned char *)&var->category[i]);

   swap_bytes_short((unsigned char *)&var->num_nadir_clusters);
   swap_bytes_short((unsigned char *)&var->num_limb_clusters);
   swap_bytes_short((unsigned char *)&var->num_occ_clusters);
   swap_bytes_short((unsigned char *)&var->num_mon_clusters);
  #endif
 }


void ads_states_getbin (FILE* unit, ADS_STATES *var)
 {
  int i;

  fread(var,sizeof(ADS_STATES),1,unit);

  #if defined(__LITTLE_ENDIAN__)

   swap_bytes_int((unsigned char *)&var->StartTime.days);
   swap_bytes_int((unsigned char *)&var->StartTime.secnd);
   swap_bytes_int((unsigned char *)&var->StartTime.musec);

   swap_bytes_float((unsigned char *)&var->orbit_phase);

   swap_bytes_short((unsigned char *)&var->category);
   swap_bytes_short((unsigned char *)&var->state_id);
   swap_bytes_short((unsigned char *)&var->duration);
   swap_bytes_short((unsigned char *)&var->longest_int_time);
   swap_bytes_short((unsigned char *)&var->num_clusters);

   swap_bytes_short((unsigned char *)&var->num_aux);
   swap_bytes_short((unsigned char *)&var->num_pmd);
   swap_bytes_short((unsigned char *)&var->num_int);

   DEBUG_Print("Cat %d %d %d %d %d %d %d %d %d\n",
               var->StartTime.days,var->StartTime.secnd,var->StartTime.musec,
              (int)var->category,(int)var->state_id,(int)var->duration,(int)var->longest_int_time,(int)var->num_clusters,(int)var->flag_mds);

   for (i=0;i<MAX_CLUSTER;i++)
    {
     swap_bytes_short((unsigned char *)&var->int_times[i]);
     swap_bytes_short((unsigned char *)&var->num_polar[i]);

     swap_bytes_short((unsigned char *)&var->Clcon[i].pixel_nr);
     swap_bytes_short((unsigned char *)&var->Clcon[i].length);
     swap_bytes_float((unsigned char *)&var->Clcon[i].pet);

     swap_bytes_short((unsigned char *)&var->Clcon[i].int_time);
     swap_bytes_short((unsigned char *)&var->Clcon[i].coadd);
     swap_bytes_short((unsigned char *)&var->Clcon[i].n_read);
    }

   swap_bytes_short((unsigned char *)&var->total_polar);
   swap_bytes_short((unsigned char *)&var->num_dsr);
   swap_bytes_int((unsigned char *)&var->length_dsr);

  #endif
 }

void gads_sun_ref_getbin (FILE* unit, gads_sun_ref *var)
 {
  int i;

  fread(var,sizeof(gads_sun_ref),1,unit);

  #if defined(__LITTLE_ENDIAN__)
  for (i=0;i<NPIXEL;i++)
   {
    swap_bytes_float((unsigned char *)&var->wavel[i]);
    swap_bytes_float((unsigned char *)&var->spectrum[i]);
    swap_bytes_float((unsigned char *)&var->precision[i]);
    swap_bytes_float((unsigned char *)&var->accuracy[i]);
    swap_bytes_float((unsigned char *)&var->etalon[i]);
   }

  swap_bytes_float((unsigned char *)&var->asm_pos);
  swap_bytes_float((unsigned char *)&var->esm_pos);
  swap_bytes_float((unsigned char *)&var->sun_elev);

  for (i=0;i<NPMD;i++)
   {
    swap_bytes_float((unsigned char *)&var->pmd_mean[i]);
    swap_bytes_float((unsigned char *)&var->out_of_band_nd_out[i]);
    swap_bytes_float((unsigned char *)&var->out_of_band_nd_in[i]);
   }

  swap_bytes_float((unsigned char *)&var->doppler);
  #endif
 }


void mds_1c_constant_getbin (FILE* unit, mds_1c_constant *var)
 {
  fread(var,sizeof(mds_1c_constant),1,unit);

  #if defined(__LITTLE_ENDIAN__)
   swap_bytes_int((unsigned char *)&var->StartTime.days);
   swap_bytes_int((unsigned char *)&var->StartTime.secnd);
   swap_bytes_int((unsigned char *)&var->StartTime.musec);
   swap_bytes_int((unsigned char *)&var->length);
   swap_bytes_float((unsigned char *)&var->orbit_phase);
   swap_bytes_short((unsigned char *)&var->category);
   swap_bytes_short((unsigned char *)&var->state_id);
   swap_bytes_short((unsigned char *)&var->cluster_id);
   swap_bytes_short((unsigned char *)&var->nobs);
   swap_bytes_short((unsigned char *)&var->npixels);
  #endif
 }
