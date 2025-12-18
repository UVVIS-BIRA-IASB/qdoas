/*! \file output_private.h \brief Contains functions that can be used
  to extract output data and store it in the appropriate output_field
  buffers.  For a registered output field, output_field::get_data will
  normally contain a pointer to one of these functions.*/

#include "engine.h"
#include "analyse.h"

// gcc has an annotation to skip warnings for unused function arguments
#ifdef _MSC_VER
#define UNUSED
#else
#define UNUSED __attribute__ ((unused))
#endif

RC write_spikes(char *spikestring, unsigned int length, bool *spikes,int ndet);
double output_flux(const ENGINE_CONTEXT *pEngineContext, double wavelength, int indexFenoColumn);

static inline FENO *get_tabfeno_calib(struct output_field *this_field UNUSED, int indexFenoColumn) {
  FENO *tabfeno_kurucz = &TabFeno[indexFenoColumn][INDEX_KURUCZ];
  return tabfeno_kurucz->rcKurucz ? NULL : tabfeno_kurucz;
}

static inline FENO *get_tabfeno_analysis(struct output_field *this_field, int indexFenoColumn) {
  return &TabFeno[indexFenoColumn][this_field->index_feno];
}

static inline CROSS_RESULTS *get_cross_results_calib(struct output_field *this_field, int indexFenoColumn, int index_calib) {
  FENO *pTabFeno = &TabFeno[indexFenoColumn][INDEX_KURUCZ];
  return (pTabFeno->rc || pTabFeno->rcKurucz)
    ? NULL
    : &KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].results[index_calib][this_field->index_cross];
}

static inline CROSS_RESULTS *get_cross_results_analysis(struct output_field *this_field, int indexFenoColumn, int index_calib UNUSED ) {
  FENO * pTabFeno = &TabFeno[indexFenoColumn][this_field->index_feno];
  return (pTabFeno->rc)
    ? NULL
    : &pTabFeno->TabCrossResults[this_field->index_cross];
}

static inline void get_specno(struct output_field *this_field UNUSED, int *specno, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *specno = pEngineContext->indexRecord;
}

static inline void get_name(struct output_field *this_field UNUSED, char **name, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *name = strdup(pEngineContext->recordInfo.Nom);
}

static inline void get_filename(struct output_field *this_field UNUSED, char const **name, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  // Keep only the file basename:
  const char *ptr = strrchr(pEngineContext->fileInfo.fileName, PATH_SEP);
  ptr = (ptr!=NULL) ? &ptr[1] : pEngineContext->fileInfo.fileName;
  *name = strdup(ptr);
}

static inline void get_date(struct output_field *this_field UNUSED, struct date *date, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  const struct date *datein = &pEngineContext->recordInfo.present_datetime.thedate;
  date->da_day = datein->da_day;
  date->da_mon = datein->da_mon;
  date->da_year = datein->da_year;
}

static inline void get_date_start(struct output_field *this_field UNUSED, struct date *date, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  const struct date *datein = &pEngineContext->recordInfo.startDateTime.thedate;
  date->da_day = datein->da_day;
  date->da_mon = datein->da_mon;
  date->da_year = datein->da_year;
}

static inline void get_date_end(struct output_field *this_field UNUSED, struct date *date, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  const struct date *datein = &pEngineContext->recordInfo.endDateTime.thedate;
  date->da_day = datein->da_day;
  date->da_mon = datein->da_mon;
  date->da_year = datein->da_year;
}

static inline void get_time(struct output_field *this_field UNUSED, struct time *time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *time = pEngineContext->recordInfo.present_datetime.thetime;
}

static inline void get_datetime(struct output_field *this_field UNUSED, struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  get_date(this_field, &datetime->thedate, pEngineContext, indexFenoColumn, index_calib);
  *datetime = pEngineContext->recordInfo.present_datetime;
}

static inline void get_year(struct output_field *this_field UNUSED, unsigned short *year, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *year = pEngineContext->recordInfo.present_datetime.thedate.da_year;
}

static inline void get_start_datetime(struct output_field *this_field UNUSED, struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  get_date_start(this_field, &datetime->thedate, pEngineContext, indexFenoColumn, index_calib);
  *datetime =pEngineContext->recordInfo.startDateTime;
}

static inline void get_end_datetime(struct output_field *this_field UNUSED, struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  get_date_end(this_field, &datetime->thedate, pEngineContext, indexFenoColumn, index_calib);
  *datetime =pEngineContext->recordInfo.endDateTime;
}

static inline void get_start_time(struct output_field *this_field UNUSED, struct time *time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *time = pEngineContext->recordInfo.startDateTime.thetime;
}

static inline void get_end_time(struct output_field *this_field UNUSED, struct time *time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *time = pEngineContext->recordInfo.endDateTime.thetime;
}

static inline void get_julian(struct output_field *this_field UNUSED, unsigned short *julian, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *julian = ZEN_FNCaljda(&pEngineContext->recordInfo.Tm);
}

static inline void get_frac_julian(struct output_field *this_field UNUSED, double *julian, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *julian = ZEN_FNCaljda(&pEngineContext->recordInfo.Tm) + ZEN_FNCaldti(&pEngineContext->recordInfo.Tm)/24.;
}

static inline void get_frac_time(struct output_field *this_field UNUSED, double *frac_time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *frac_time = ZEN_FNCaldti(&pEngineContext->recordInfo.Tm);
}

static inline void get_frac_time_recordinfo(struct output_field *this_field UNUSED, double *frac_time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *frac_time = pEngineContext->recordInfo.TimeDec;
}

static inline void get_scans(struct output_field *this_field UNUSED, unsigned short *scans, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scans = pEngineContext->recordInfo.NSomme;
}

static inline void get_n_rejected(struct output_field *this_field UNUSED, unsigned short *n_rejected, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *n_rejected = pEngineContext->recordInfo.rejected;
}

static inline void get_t_int(struct output_field *this_field UNUSED, double *t_int, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *t_int = pEngineContext->recordInfo.Tint;
}

static inline void scia_get_sza(struct output_field *this_field UNUSED, float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  memcpy(sza, pEngineContext->recordInfo.scia.solZen, 3 * sizeof(*sza));
}

static inline void gome2_get_sza(struct output_field *this_field UNUSED, float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for (int i=0; i<3; ++i) {
    sza[i] = pEngineContext->recordInfo.gome2.solZen[i];
  }
}

static inline void gdp_get_sza(struct output_field *this_field UNUSED, float *sza, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for (int i=0; i<3; ++i) {
    sza[i] = pEngineContext->recordInfo.gome.sza[i];
  }
}

static inline void get_sza(struct output_field *this_field UNUSED, float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *sza = pEngineContext->recordInfo.Zm;
}

static inline void scia_get_azim(struct output_field *this_field UNUSED, float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  memcpy(azim, pEngineContext->recordInfo.scia.solAzi, 3 * sizeof(*azim));
}

static inline void gome2_get_azim(struct output_field *this_field UNUSED, float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for (int i=0; i<3; ++i) {
    azim[i] = pEngineContext->recordInfo.gome2.solAzi[i];
  }
}

static inline void gdp_get_azim(struct output_field *this_field UNUSED, float *azim, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for (int i=0; i<3; ++i) {
    azim[i] = pEngineContext->recordInfo.gome.azim[i];
  }
}

static inline void get_azim(struct output_field *this_field UNUSED, float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *azim = pEngineContext->recordInfo.Azimuth;
}

static inline void get_tdet(struct output_field *this_field UNUSED, float *tdet, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *tdet = pEngineContext->recordInfo.TDet;
}

static inline void get_sky(struct output_field *this_field UNUSED, unsigned short *skyobs, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *skyobs = pEngineContext->recordInfo.SkyObs;
}

static inline void get_bestshift(struct output_field *this_field UNUSED, float *bestshift, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *bestshift = pEngineContext->recordInfo.BestShift;
}

static inline void get_pixel_number(struct output_field *this_field UNUSED, unsigned short *pixel_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *pixel_number = pEngineContext->recordInfo.gome.pixelNumber;
}

static inline void get_pixel_type(struct output_field *this_field UNUSED, unsigned short *pixel_type, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *pixel_type = pEngineContext->recordInfo.gome.pixelType;
}

static inline void get_orbit_number(struct output_field *this_field UNUSED, int *orbit_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *orbit_number = pEngineContext->recordInfo.satellite.orbit_number;
}

static inline void get_cloud_fraction(struct output_field *this_field UNUSED, float *cloud_fraction, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *cloud_fraction = pEngineContext->recordInfo.satellite.cloud_fraction;
}

static inline void get_cloud_top_pressure(struct output_field *this_field UNUSED, float *cloud_top_pressure, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *cloud_top_pressure = pEngineContext->recordInfo.satellite.cloud_top_pressure;
}

static inline void get_scia_state_index(struct output_field *this_field UNUSED, unsigned short *scia_state_index, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scia_state_index = pEngineContext->recordInfo.scia.stateIndex;
}

static inline void get_scia_state_id(struct output_field *this_field UNUSED, unsigned short *scia_state_id, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scia_state_id = pEngineContext->recordInfo.scia.stateId;
}

static inline void get_scia_quality(struct output_field *this_field UNUSED, unsigned short *scia_quality, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scia_quality = pEngineContext->recordInfo.scia.qualityFlag;
}

static inline void get_sat_latitude(struct output_field *this_field UNUSED, float *latitude, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *latitude = pEngineContext->recordInfo.satellite.latitude;
}

static inline void get_sat_longitude(struct output_field *this_field UNUSED, float *longitude, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *longitude = pEngineContext->recordInfo.satellite.longitude;
}

static inline void get_sat_altitude(struct output_field *this_field UNUSED, float *altitude, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *altitude = pEngineContext->recordInfo.satellite.altitude;
}

static inline void get_sat_sza(struct output_field *this_field UNUSED, float *sza, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *sza = pEngineContext->recordInfo.satellite.sza;
}

static inline void get_sat_saa(struct output_field *this_field UNUSED, float *saa, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *saa = pEngineContext->recordInfo.satellite.saa;
}

static inline void get_sat_vza(struct output_field *this_field UNUSED, float *vza, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *vza = pEngineContext->recordInfo.satellite.vza;
}

static inline void get_earth_radius(struct output_field *this_field UNUSED, float *radius, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *radius = pEngineContext->recordInfo.satellite.earth_radius;
}

static inline void get_view_elevation(struct output_field *this_field UNUSED, float *view_elevation, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *view_elevation = pEngineContext->recordInfo.elevationViewAngle;
}

static inline void get_view_zenith(struct output_field *this_field UNUSED, float *view_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *view_zenith = pEngineContext->recordInfo.zenithViewAngle;
}

static inline void get_view_azimuth(struct output_field *this_field UNUSED, float *view_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *view_azimuth = pEngineContext->recordInfo.azimuthViewAngle;
}

static inline void scia_get_los_zenith(struct output_field *this_field UNUSED, float *los_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  memcpy(los_zenith, pEngineContext->recordInfo.scia.losZen, 3 * sizeof(*los_zenith));
}

static inline void gome2_get_los_zenith(struct output_field *this_field UNUSED, float *los_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for (int i=0; i<3; ++i) {
    los_zenith[i] = pEngineContext->recordInfo.gome2.losZen[i];
  }
}

static inline void gdp_get_los_zenith(struct output_field *this_field UNUSED, float *los_zenith, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for(int i=0; i<3; i++) {
    los_zenith[i] = pEngineContext->recordInfo.gome.vza[i];
  }
}

static inline void get_los_zenith (struct output_field *this_field UNUSED, float *los_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *los_zenith = pEngineContext->recordInfo.zenithViewAngle;
}

static inline void scia_get_los_azimuth(struct output_field *this_field UNUSED, float *los_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  memcpy(los_azimuth, pEngineContext->recordInfo.scia.losAzi, 3 * sizeof(*los_azimuth));
}

static inline void gome2_get_los_azimuth(struct output_field *this_field UNUSED, float *los_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for (int i=0; i<3; ++i) {
    los_azimuth[i] = pEngineContext->recordInfo.gome2.losAzi[i];
  }
}

static inline void gdp_get_los_azimuth(struct output_field *this_field UNUSED, float *los_azimuth, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for(int i=0; i<3; i++) {
    los_azimuth[i] = pEngineContext->recordInfo.gome.vaa[i];
  }
}

static inline void get_los_azimuth (struct output_field *this_field UNUSED, float *los_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *los_azimuth = pEngineContext->recordInfo.azimuthViewAngle;
}

static inline void get_longitude (struct output_field *this_field UNUSED, float *longitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *longitude = pEngineContext->recordInfo.longitude;
}

static inline void get_corner_longitudes(struct output_field *this_field UNUSED, float *longitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for(int i=0; i<4; i++)
    longitude[i] = (float)pEngineContext->recordInfo.satellite.cornerlons[i];
}

static inline void get_latitude (struct output_field *this_field UNUSED, float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *latitude = pEngineContext->recordInfo.latitude;
}

static inline void get_corner_latitudes(struct output_field *this_field UNUSED, float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  for(int i=0; i<4; i++)
    latitude[i] = (float)pEngineContext->recordInfo.satellite.cornerlats[i];
}

static inline void get_altitude(struct output_field *this_field UNUSED, float *altitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *altitude = pEngineContext->recordInfo.altitude;
}

static inline void mkzy_get_scanning_angle(struct output_field *this_field UNUSED, float *scanning_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scanning_angle = pEngineContext->recordInfo.mkzy.scanningAngle;
}

static inline void get_scanning_angle(struct output_field *this_field UNUSED, float *scanning_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scanning_angle = pEngineContext->recordInfo.als.scanningAngle;
}

static inline void get_compass_angle(struct output_field *this_field UNUSED, float *compass_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *compass_angle = pEngineContext->recordInfo.als.compassAngle;
}

static inline void get_pitch_angle(struct output_field *this_field UNUSED, float *pitch_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *pitch_angle = pEngineContext->recordInfo.als.pitchAngle;
}

static inline void get_roll_angle(struct output_field *this_field UNUSED, float *roll_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *roll_angle = pEngineContext->recordInfo.als.rollAngle;
}

static inline void get_filter_number(struct output_field *this_field UNUSED, int *filter_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *filter_number = pEngineContext->recordInfo.ccd.filterNumber;
}

static inline void get_meastype(struct output_field *this_field UNUSED, int *meastype, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *meastype = pEngineContext->recordInfo.maxdoas.measurementType;
}

static inline void ccd_get_head_temperature(struct output_field *this_field UNUSED, double *head_temperature, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *head_temperature = pEngineContext->recordInfo.ccd.headTemperature;
}

static inline void get_cooling_status(struct output_field *this_field UNUSED, int *cooling_status, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *cooling_status = pEngineContext->recordInfo.coolingStatus;
}

static inline void get_mirror_error(struct output_field *this_field UNUSED, int *mirror_error, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *mirror_error = pEngineContext->recordInfo.mirrorError;
}

static inline void gome2_get_mdr_number(struct output_field *this_field UNUSED, int *mdr_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *mdr_number = pEngineContext->recordInfo.gome2.mdrNumber;
}

static inline void gome2_get_observation_index(struct output_field *this_field UNUSED, int *observation_index, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *observation_index = pEngineContext->recordInfo.gome2.observationIndex;
}

static inline void gome2_get_scan_direction(struct output_field *this_field UNUSED, int *scan_direction, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scan_direction = pEngineContext->recordInfo.gome2.scanDirection;
}

static inline void gome2_get_observation_mode(struct output_field *this_field UNUSED, unsigned short *observation_mode, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *observation_mode = pEngineContext->recordInfo.gome2.observationMode;
}

static inline void gome2_get_saa(struct output_field *this_field UNUSED, int *saa, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *saa = pEngineContext->recordInfo.gome2.saaFlag;
}

static inline void gome2_get_sunglint_risk(struct output_field *this_field UNUSED, int *sunglint_risk, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *sunglint_risk = pEngineContext->recordInfo.gome2.sunglintDangerFlag;
}

static inline void gome2_get_sunglint_high_risk(struct output_field *this_field UNUSED, int *sunglint_high_risk, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *sunglint_high_risk = pEngineContext->recordInfo.gome2.sunglintHighDangerFlag;
}

static inline void gome2_get_rainbow(struct output_field *this_field UNUSED, int *rainbow, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *rainbow = pEngineContext->recordInfo.gome2.rainbowFlag;
}

static inline void get_diodes(struct output_field *this_field UNUSED, float *diodes, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  memcpy(diodes, pEngineContext->recordInfo.ccd.diodes, 4 * sizeof(*diodes));
}

static inline void get_precalculated_flux(struct output_field *this_field UNUSED, float *fluxes, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
     fluxes[0]=pEngineContext->recordInfo.ccd.wve1;
     fluxes[1]=pEngineContext->recordInfo.ccd.flux1;
     fluxes[2]=pEngineContext->recordInfo.ccd.wve2;
     fluxes[3]=pEngineContext->recordInfo.ccd.flux2;
}

static inline void get_target_azimuth(struct output_field *this_field UNUSED, float *azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *azimuth = pEngineContext->recordInfo.ccd.targetAzimuth;
}

static inline void get_target_elevation(struct output_field *this_field UNUSED, float *elevation, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *elevation = pEngineContext->recordInfo.ccd.targetElevation;
}

static inline void get_saturated_flag(struct output_field *this_field UNUSED, unsigned short *saturated_flag, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *saturated_flag = pEngineContext->recordInfo.ccd.saturatedFlag;
}

static inline void get_alongtrack_index(struct output_field *this_field UNUSED, int *row, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *row = 1 + pEngineContext->recordInfo.i_alongtrack;
}

static inline void get_crosstrack_index(struct output_field *this_field UNUSED, int *row, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *row = 1 + pEngineContext->recordInfo.i_crosstrack;
}

static inline void get_groundpixelqf(struct output_field *this_field UNUSED, unsigned short *groundpixelqf, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *groundpixelqf = pEngineContext->recordInfo.ground_pixel_QF;
}

static inline void get_xtrackqf(struct output_field *this_field UNUSED, unsigned short *xtrackqf, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *xtrackqf = pEngineContext->recordInfo.xtrack_QF;
}

static inline void get_omi_configuration_id(struct output_field *this_field UNUSED, unsigned short *configuration_id, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *configuration_id = pEngineContext->recordInfo.omi.instrumentConfigurationId;
}

static inline void get_cloud_mask(struct output_field *this_field UNUSED, unsigned short *cloud_mask, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *cloud_mask = pEngineContext->recordInfo.tempo.cloud_mask;
}

static inline void get_red(struct output_field *this_field UNUSED, float *red, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *red = pEngineContext->recordInfo.tempo.red;
}

static inline void get_blue(struct output_field *this_field UNUSED, float *blue, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *blue = pEngineContext->recordInfo.tempo.blue;
}

static inline void get_green(struct output_field *this_field UNUSED, float *green, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *green = pEngineContext->recordInfo.tempo.green;
}

static inline void get_uav_servo_byte_sent(struct output_field *this_field UNUSED, unsigned short *servo_sent_position, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *servo_sent_position = pEngineContext->recordInfo.uavBira.servoSentPosition;
}

static inline void get_uav_servo_byte_received(struct output_field *this_field UNUSED, unsigned short *servo_received_position, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *servo_received_position = pEngineContext->recordInfo.uavBira.servoReceivedPosition;
}

static inline void get_uav_insideTemp(struct output_field *this_field UNUSED, float *uav_insideTemp, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_insideTemp = pEngineContext->recordInfo.uavBira.insideTemp;
}

static inline void get_uav_outsideTemp(struct output_field *this_field UNUSED, float *uav_outsideTemp, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_outsideTemp = pEngineContext->recordInfo.uavBira.outsideTemp;
}

static inline void get_uav_pressure(struct output_field *this_field UNUSED, float *uav_pressure, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_pressure = pEngineContext->recordInfo.uavBira.pressure;
}

static inline void get_uav_humidity(struct output_field *this_field UNUSED, float *uav_humidity, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_humidity = pEngineContext->recordInfo.uavBira.humidity;
}

static inline void get_uav_dewpoint(struct output_field *this_field UNUSED, float *uav_dewpoint, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_dewpoint = pEngineContext->recordInfo.uavBira.dewPoint;
}

static inline void get_uav_pitch(struct output_field *this_field UNUSED, float *uav_pitch, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_pitch = pEngineContext->recordInfo.uavBira.pitch;
}

static inline void get_uav_roll(struct output_field *this_field UNUSED, float *uav_roll, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_roll = pEngineContext->recordInfo.uavBira.roll;
}

static inline void get_uav_heading(struct output_field *this_field UNUSED, float *uav_heading, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *uav_heading = pEngineContext->recordInfo.uavBira.heading;
}

static inline void get_rc(struct output_field *this_field UNUSED, int *rc, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *rc = pEngineContext->recordInfo.rc;
}

static inline void get_slant_column(struct output_field *this_field, double *slant_column, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *slant_column = ( pTabCrossResults && pTabCrossResults->SlntFact!=(double)0.)
    ? pTabCrossResults->SlntCol/pTabCrossResults->SlntFact
    : QDOAS_FILL_DOUBLE;
}

static inline void get_slant_err(struct output_field *this_field, double *slant_err, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *slant_err = ( pTabCrossResults && pTabCrossResults->SlntFact!=(double)0.)
    ? pTabCrossResults->SlntErr/pTabCrossResults->SlntFact
    : QDOAS_FILL_DOUBLE;
}

static inline void get_shift(struct output_field *this_field, double *shift, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *shift = ( pTabCrossResults ) ? pTabCrossResults->Shift : QDOAS_FILL_DOUBLE;
}

static inline void get_center_wavelength(struct output_field *this_field, double *lambda, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  struct fit_properties *fit  = &this_field->get_tabfeno(this_field, indexFenoColumn)->fit_properties;
  *lambda  =pTabCrossResults
    ? center_pixel_wavelength(spectrum_start(fit->specrange), spectrum_end(fit->specrange))
    : QDOAS_FILL_DOUBLE;
}

static inline void get_shift_err(struct output_field *this_field, double *shift_err, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *shift_err = ( pTabCrossResults ) ? pTabCrossResults->SigmaShift : QDOAS_FILL_DOUBLE;
}

static inline void get_stretches(struct output_field *this_field, double *stretch, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  CROSS_REFERENCE *pTabCross = this_field->get_tabfeno(this_field, indexFenoColumn)->TabCross;
  if (pTabCrossResults) {
    stretch[0] = pTabCross->FitStretch ? pTabCrossResults->Stretch : QDOAS_FILL_DOUBLE;
    stretch[1] = pTabCross->FitStretch2 ? pTabCrossResults->Stretch2 : QDOAS_FILL_DOUBLE;
  } else {
    stretch[0] = stretch[1] = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_stretch_err(struct output_field *this_field, double *stretch_error, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *stretch_error = (pTabCrossResults) ? pTabCrossResults->SigmaStretch : QDOAS_FILL_DOUBLE;
}

static inline void get_stretch2_err(struct output_field *this_field, double *stretch_error, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *stretch_error = (pTabCrossResults) ? pTabCrossResults->SigmaStretch2 : QDOAS_FILL_DOUBLE;
}

static inline void get_stretch_errors(struct output_field *this_field, double *stretch_errors, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    stretch_errors[0] = pTabCrossResults->SigmaStretch;
    stretch_errors[1] = pTabCrossResults->SigmaStretch2;
  } else {
    stretch_errors[0] = stretch_errors[1] = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_scale(struct output_field *this_field, double *scale, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale = ( pTabCrossResults ) ? pTabCrossResults->Scale : QDOAS_FILL_DOUBLE;
}

static inline void get_scale2(struct output_field *this_field, double *scale, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale = ( pTabCrossResults ) ? pTabCrossResults->Scale2 : QDOAS_FILL_DOUBLE;
}

static inline void get_scales(struct output_field *this_field, double *scales, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    scales[0] = pTabCrossResults->Scale;
    scales[1] = pTabCrossResults->Scale2;
  } else {
    scales[0] = scales[1] = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_scale_err(struct output_field *this_field, double *scale_error, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale_error = (pTabCrossResults)
    ? pTabCrossResults->SigmaScale
    : QDOAS_FILL_DOUBLE;
}

static inline void get_scale2_err(struct output_field *this_field, double *scale_error, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale_error = (pTabCrossResults)
    ? pTabCrossResults->SigmaScale2
    : QDOAS_FILL_DOUBLE;
}

static inline void get_scale_errors(struct output_field *this_field, double *scale_errors, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    scale_errors[0] = pTabCrossResults->SigmaScale;
    scale_errors[1] = pTabCrossResults->SigmaScale2;
  } else {
    scale_errors[0] = scale_errors[1] = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_param(struct output_field *this_field, double *param, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    *param = (this_field->get_tabfeno(this_field, indexFenoColumn)->TabCross[this_field->index_cross].IndSvdA)
      ? pTabCrossResults->SlntCol
      : pTabCrossResults->Param;
  } else {
    *param = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_param_err(struct output_field *this_field, double *param_err, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    *param_err = (this_field->get_tabfeno(this_field, indexFenoColumn)->TabCross[this_field->index_cross].IndSvdA)
      ? pTabCrossResults->SlntErr
      : pTabCrossResults->SigmaParam;
  } else {
    *param_err = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_amf(struct output_field *this_field, float *amf, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *amf = pTabCrossResults ? pTabCrossResults->Amf : QDOAS_FILL_FLOAT;
}

static inline void get_vrt_col(struct output_field *this_field, double *vrt_col, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *vrt_col = ( pTabCrossResults && (pTabCrossResults->VrtFact != (double)0.) )
    ? pTabCrossResults->VrtCol/pTabCrossResults->VrtFact
    : QDOAS_FILL_DOUBLE;
}

static inline void get_vrt_err(struct output_field *this_field, double *vrt_err, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *vrt_err = ( pTabCrossResults && (pTabCrossResults->VrtFact!=(double)0.) )
    ? pTabCrossResults->VrtErr/pTabCrossResults->VrtFact
    : QDOAS_FILL_DOUBLE;
}

static inline void get_refzm(struct output_field *this_field, float *refzm, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib UNUSED) {
     FENO *pTabFeno=this_field->get_tabfeno(this_field, indexFenoColumn);
  *refzm = (((pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA) ||
            ((pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SCAN) &&
            ((pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_BEFORE) || (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_AFTER)) &&
             (pEngineContext->recordInfo.maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH))) && (pEngineContext->indexRecord!=pTabFeno->indexRef))?(float) pTabFeno->Zm:QDOAS_FILL_FLOAT;

             // scan interpolate and average : currently refzm=QDOAS_FILL_FLOAT
             //                                could be determined by interpolating or averaging the measurement times of zenith before and after
             //                                in this case, the geolocation should be provided
}

static inline void get_refnumber(struct output_field *this_field, int *refnumber, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib UNUSED) {
     FENO *pTabFeno=this_field->get_tabfeno(this_field, indexFenoColumn);
  *refnumber= (((pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA) ||
               ((pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SCAN) &&
               (pEngineContext->recordInfo.maxdoas.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_ZENITH))) && (pEngineContext->indexRecord!=pTabFeno->indexRef))?
                pTabFeno->indexRef:((pEngineContext->project.asciiResults.file_format==ASCII)?-1:QDOAS_FILL_INT);
}

static inline void get_refnumber_before(struct output_field *this_field, int *refnumber, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  *refnumber= (float) this_field->get_tabfeno(this_field, indexFenoColumn)->indexRefScanBefore;
  if ((*refnumber==ITEM_NONE) && (pEngineContext->project.asciiResults.file_format!=ASCII))
   *refnumber=QDOAS_FILL_INT;
}

static inline void get_refnumber_after(struct output_field *this_field, int *refnumber, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  *refnumber= (float) this_field->get_tabfeno(this_field, indexFenoColumn)->indexRefScanAfter;
  if ((*refnumber==ITEM_NONE) && (pEngineContext->project.asciiResults.file_format!=ASCII))
   *refnumber=QDOAS_FILL_INT;
}

static inline void get_ref_shift(struct output_field *this_field, float *ref_shift, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  *ref_shift = (float) this_field->get_tabfeno(this_field, indexFenoColumn)->Shift;
}

static inline void get_corr(struct output_field *this_field, double *corr, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  if (pTabFeno && pTabFeno->TabCross[this_field->index_cross].Fact != (double) 0.) {
    CROSS_REFERENCE *TabCross = pTabFeno->TabCross;
    *corr = pTabFeno->fit_properties.covar[TabCross[this_field->index_cross2].IndSvdA][TabCross[this_field->index_cross].IndSvdA]*pTabFeno->chiSquare/
      (TabCross[this_field->index_cross].Fact*TabCross[this_field->index_cross2].Fact);
  } else
    *corr = QDOAS_FILL_DOUBLE;
}

static inline void get_covar(struct output_field *this_field, double *covar, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  if (pTabFeno && pTabFeno->TabCross[this_field->index_cross].Fact != (double) 0.) {
    CROSS_REFERENCE *TabCross = pTabFeno->TabCross;
    *covar = pTabFeno->fit_properties.covar[TabCross[this_field->index_cross2].IndSvdA][TabCross[this_field->index_cross].IndSvdA]*pTabFeno->chiSquare/
      (TabCross[this_field->index_cross].Fact*TabCross[this_field->index_cross2].Fact
       * pTabFeno->TabCrossResults[this_field->index_cross].SlntErr *  pTabFeno->TabCrossResults[this_field->index_cross2].SlntErr);
  }
}

static inline void get_spikes(struct output_field *this_field, char **spike_list, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *spike_list = malloc(50);
  write_spikes(*spike_list, 50, pTabFeno->spikes, pTabFeno->NDET);
}

static inline void get_residual_spectrum(struct output_field *this_field, double *residualSpectrum, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  
  if (pTabFeno->residualSpectrum!=NULL)
   memcpy(residualSpectrum,pTabFeno->residualSpectrum,sizeof(double)*this_field->data_cols);
  else
   for (size_t i=0; i!=this_field->data_cols; ++i)
     residualSpectrum[i]=QDOAS_FILL_DOUBLE;
}



static inline void omi_get_rejected_pixels(struct output_field *this_field, char **pixel_list, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *pixel_list = malloc(50);
  write_spikes(*pixel_list, 50, pTabFeno->omiRejPixelsQF, pTabFeno->NDET);
}

static inline void get_rms(struct output_field *this_field, double *rms, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *rms = (!pTabFeno->rc)?pTabFeno->RMS:QDOAS_FILL_DOUBLE;
}

static inline void get_rms_calib(struct output_field *this_field, double *rms, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  *rms = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].rms[index_calib];
}

static inline void get_wavelength_calib(struct output_field *this_field, double *wavelength, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {

  *wavelength = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].wve[index_calib];
}

static inline void get_n_iter(struct output_field *this_field, int *n_iter, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *n_iter = (!pTabFeno->rc) ? pTabFeno->nIter : QDOAS_FILL_INT;
}

static inline void get_n_iter_calib(struct output_field *this_field, int *n_iter, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  *n_iter = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].nIter[index_calib];
}

static inline void get_num_bands(struct output_field *this_field, int *num_bands, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  struct fit_properties *fit = (index_calib == ITEM_NONE)
    ? &this_field->get_tabfeno(this_field, indexFenoColumn)->fit_properties
    : &KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].subwindow_fits[index_calib];
  *num_bands = fit ? fit->DimL : QDOAS_FILL_INT;
}

static inline void get_processing_error_flag(struct output_field *this_field, int *error_flag, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *error_flag = (pTabFeno == NULL || pTabFeno->rc) ? 1 : 0;
}

static inline void get_chisquare(struct output_field *this_field, double *chisquare, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *chisquare = (!pTabFeno->rc)?pTabFeno->chiSquare : QDOAS_FILL_DOUBLE;
}

static inline void get_chisquare_calib(struct output_field *this_field, double *chisquare, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib) {
  *chisquare = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].chiSquare[index_calib];
}

static inline void get_rc_analysis(struct output_field *this_field, int *rc, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  rc[0] = pTabFeno->rcKurucz;
  rc[1] = pTabFeno->rc;
}

static inline void get_rc_calib(struct output_field *this_field, int *rc, const ENGINE_CONTEXT *pEngineContext UNUSED, int indexFenoColumn, int index_calib UNUSED) {
  *rc = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].rc;
}

static inline void get_flux(struct output_field *this_field, double *flux, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib UNUSED) {
  if (pEngineContext->project.instrumental.use_row[indexFenoColumn] ) {
    *flux = output_flux(pEngineContext, OUTPUT_fluxes[this_field->index_flux], indexFenoColumn);
  } else {
    *flux = QDOAS_FILL_DOUBLE;
  }
}

static inline void get_cic(struct output_field *this_field, double *cic, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib UNUSED) {
  double flux1 = output_flux(pEngineContext, OUTPUT_cic[this_field->index_cic][1], indexFenoColumn);
  *cic = (flux1 != (double) 0.)
    ? output_flux(pEngineContext, OUTPUT_cic[this_field->index_cic][0], indexFenoColumn)/flux1
    : QDOAS_FILL_DOUBLE;
}

static inline void get_gps_start_time(struct output_field *this_field UNUSED,struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *datetime = pEngineContext->recordInfo.uavBira.startTime;
}

static inline void get_gps_end_time(struct output_field *this_field UNUSED, struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *datetime = pEngineContext->recordInfo.uavBira.endTime;
}

static inline void get_longitude_end (struct output_field *this_field UNUSED, float *longitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *longitude = pEngineContext->recordInfo.uavBira.longitudeEnd;
}

static inline void get_latitude_end (struct output_field *this_field UNUSED, float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *latitude = pEngineContext->recordInfo.uavBira.latitudeEnd;
}

static inline void get_altitude_end (struct output_field *this_field UNUSED, float *altitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *altitude = pEngineContext->recordInfo.uavBira.altitudeEnd;
}

static inline void get_total_exp_time (struct output_field *this_field UNUSED, double *total_exp_time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *total_exp_time = pEngineContext->recordInfo.TotalExpTime;
}

static inline void get_total_acq_time (struct output_field *this_field UNUSED, double *total_acq_time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *total_acq_time = pEngineContext->recordInfo.TotalAcqTime;
}

static inline void get_lambda(struct output_field *this_field UNUSED, double *lambda, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  const int n_wavel = NDET[indexFenoColumn];
  memcpy(lambda,pEngineContext->buffers.lambda,sizeof(double)*n_wavel);
}

static inline void get_spectrum(struct output_field *this_field UNUSED, double *spectrum, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  const int n_wavel = NDET[indexFenoColumn];
  memcpy(spectrum,pEngineContext->buffers.spectrum,sizeof(double)*n_wavel);
}

static inline void get_scan_index(struct output_field *this_field UNUSED, int *scanIndex, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *scanIndex = pEngineContext->recordInfo.maxdoas.scanIndex;
}

static inline void get_zenith_before_index(struct output_field *this_field UNUSED, int *zenithBeforeIndex, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *zenithBeforeIndex = (pEngineContext->recordInfo.maxdoas.zenithBeforeIndex!=ITEM_NONE)?pEngineContext->recordInfo.maxdoas.zenithBeforeIndex+1:ITEM_NONE;
}

static inline void get_zenith_after_index(struct output_field *this_field UNUSED, int *zenithAfterIndex, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn UNUSED, int index_calib UNUSED) {
  *zenithAfterIndex = (pEngineContext->recordInfo.maxdoas.zenithAfterIndex!=ITEM_NONE)?pEngineContext->recordInfo.maxdoas.zenithAfterIndex+1:ITEM_NONE;
}


// write_spikes:
// concatenate all pixels containing spikes into a single string for output.

RC write_spikes(char *spikestring, unsigned int length, bool *spikes,int ndet) {
  strcpy(spikestring,"");
  char num[10];
  RC rc = 0;
  int nspikes = 0;

  int i;

  if (spikes!=NULL)
   for (i=0; i< ndet; i++)
    {
     if (spikes[i])
      {
       (nspikes++ > 0 ) ? sprintf(num,",%d",i): sprintf(num,"%d",i);
       if(strlen(num) + strlen(spikestring) < length) {
        strcat(spikestring,num);
       } else {
        rc = 1;
        break;
       }
      }
    }

  if (!nspikes)
   strcpy(spikestring,"");

  return rc;
}
