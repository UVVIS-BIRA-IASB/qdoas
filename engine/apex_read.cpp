
#include <map>
#include <cassert>
#include <vector>
#include <cstdint>

#include "apex_read.h"

#include "netcdfwrapper.h"

extern "C" {
#include "engine_context.h"
#include "analyse.h"
#include "output.h"
#include "winthrd.h"
}

using std::vector;
using std::string;
using std::map;

static NetCDFFile radiance_file;

static map<string,vector<vector<double>>> reference_radiances;
static map<string,vector<double> > reference_wavelengths;

static string init_filename;
static size_t spectral_dim; // number of wavelengths
static size_t row_dim; // along track
static size_t col_dim; // cross-track

enum _apex_fields
 {
  APEX_FIELD_SZA,
  APEX_FIELD_VZA,
  APEX_FIELD_RAA,
  APEX_FIELD_LON,
  APEX_FIELD_LAT,
  APEX_FIELD_MAX
 };

static struct netcdf_data_fields apex_data_fields[APEX_FIELD_MAX]=
 {
   { "", "solar_zenith_angle", { 0, 0, 0, 0, 0, 0 }, 0, NC_DOUBLE, NULL },      // APEX_FIELD_SZA
   { "", "viewing_zenith_angle", { 0, 0, 0, 0, 0, 0 }, 0, NC_DOUBLE, NULL },    // APEX_FIELD_VZA
   { "", "relative_azimuth_angle", { 0, 0, 0, 0, 0, 0 }, 0, NC_DOUBLE, NULL },  // APEX_FIELD_RAA
   { "", "longitude", { 0, 0, 0, 0, 0, 0 }, 0, NC_DOUBLE, NULL },               // APEX_FIELD_LON
   { "", "latitude", { 0, 0, 0, 0, 0, 0 }, 0, NC_DOUBLE, NULL }                 // APEX_FIELD_LAT
 };

enum _sempas_fields
 {
  SEMPAS_FIELD_DATETIME,
  SEMPAS_FIELD_DATETIME_START,
  SEMPAS_FIELD_DATETIME_END,
  SEMPAS_FIELD_EXPOSURE_TIME,
  SEMPAS_FIELD_TOTAL_TIME,
  SEMPAS_FIELD_SCANS_NUMBER,
  SEMPAS_FIELD_REJECTED_NUMBER,
  SEMPAS_FIELD_SOLAR_ZENITH_ANGLE,
  SEMPAS_FIELD_VIEWING_ELEVATION_ANGLE,
  SEMPAS_FIELD_VIEWING_AZIMUTH_ANGLE,
  SEMPAS_FIELD_MAX
 };

static struct netcdf_data_fields sempas_data_fields[SEMPAS_FIELD_MAX]=
 {
   { "", "datetime", { 0, 0, 0, 0, 0, 0 }, 0, NC_SHORT, NULL },                 // SEMPAS_FIELD_DATETIME
   { "", "datetime_start", { 0, 0, 0, 0, 0, 0 }, 0, NC_SHORT, NULL },           // SEMPAS_FIELD_DATETIME_START
   { "", "datetime_end", { 0, 0, 0, 0, 0, 0 }, 0, NC_SHORT, NULL },             // SEMPAS_FIELD_DATETIME_END
   { "", "exposure_time", { 0, 0, 0, 0, 0, 0 }, 0, NC_FLOAT, NULL },            // SEMPAS_FIELD_EXPOSURE_TIME
   { "", "total_time", { 0, 0, 0, 0, 0, 0 }, 0, NC_INT, NULL },                 // SEMPAS_FIELD_TOTAL_TIME
   { "", "scans_number", { 0, 0, 0, 0, 0, 0 }, 0, NC_INT, NULL },               // SEMPAS_FIELD_SCANS_NUMBER
   { "", "rejected_scans_number", { 0, 0, 0, 0, 0, 0 }, 0, NC_INT, NULL },      // SEMPAS_FIELD_REJECTED_NUMBER
   { "", "solar_zenith_angle", { 0, 0, 0, 0, 0, 0 }, 0, NC_FLOAT, NULL },       // SEMPAS_FIELD_SOLAR_ZENITH_ANGLE
   { "", "viewing_elevation_angle", { 0, 0, 0, 0, 0, 0 }, 0, NC_FLOAT, NULL },  // SEMPAS_FIELD_VIEWING_ELEVATION_ANGL
   { "", "viewing_azimuth_angle", { 0, 0, 0, 0, 0, 0 }, 0, NC_FLOAT, NULL }     // SEMPAS_FIELD_VIEWING_AZIMUTH_ANGLE
 };

static struct netcdf_data_fields *radiance_file_data=NULL;
int useSempas=0;

static double radiance_fillvalue;

static vector<vector<double> > load_reference_radiances(const NetCDFFile& reference_file) {
  vector<vector<double> > radiances(reference_file.dimLen("col_dim"));
  const size_t reference_spectral_dim = reference_file.dimLen("spectral_dim");
  for (size_t i=0; i<radiances.size(); ++i) {
    vector<double>& rad = radiances[i];
    rad.resize(reference_spectral_dim);
    const size_t start[] = {i, 0};
    const size_t count[] = {1, reference_spectral_dim};
    reference_file.getVar("reference_radiance", start, count, rad.data());
  }
  return radiances;
}

int apex_init(const char *reference_filename, ENGINE_CONTEXT *pEngineContext) {
  try {
    NetCDFFile reference_file(reference_filename);
    col_dim = reference_file.dimLen("col_dim");
    spectral_dim = reference_file.dimLen("spectral_dim");
    ANALYSE_swathSize = col_dim;

    for (size_t i=0; i< col_dim; ++i) {
      pEngineContext->project.instrumental.use_row[i] = true;
      if (spectral_dim > NDET[i])
        return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                             "spectral_dim too large. This version supports spectra of maximum length "
                             TOSTRING(APEX_INIT_LENGTH));
      NDET[i] = spectral_dim;
    }
    init_filename = reference_filename;
  } catch(std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return ERROR_ID_NO;
}

int apex_set(ENGINE_CONTEXT *pEngineContext) {
  int rc = 0;
  char projectAttribute[256];

  try {

    radiance_file = NetCDFFile(pEngineContext->fileInfo.fileName);

//    release_data(radiance_file_data,(useSempas)?SEMPAS_FIELD_MAX:APEX_FIELD_MAX);

    if (!nc_inq_att(radiance_file.groupID(),NC_GLOBAL, "project_name",NULL,NULL) &&
        !nc_get_att_text(radiance_file.groupID(), NC_GLOBAL,"project_name",projectAttribute) &&
        !strcmp(projectAttribute,"SEMPAS"))

     useSempas=1;

    radiance_file_data=(useSempas)?sempas_data_fields:apex_data_fields;

    // radiance_file_data = data_fields();

    row_dim = radiance_file.dimLen("row_dim");                                  // number of records
    size_t file_spectral_dim = radiance_file.dimLen("spectral_dim");            // spectral dimension
    size_t file_col_dim = radiance_file.dimLen("col_dim");                      // spatial dimension
    
    if (!spectral_dim) { // e.g. in browsing mode
      spectral_dim = file_spectral_dim;
    } else if (file_spectral_dim != spectral_dim) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Reference file spectral dimension different from measurement file dimension");
    }
    if (!col_dim) {
      col_dim = file_col_dim;
    } else if (file_col_dim != col_dim) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Reference file cross-track dimension different from measurement file dimension");
    }

    if (THRD_id==THREAD_TYPE_SPECTRA)
     {
      ANALYSE_swathSize=col_dim;
      for (int i=0;i<ANALYSE_swathSize;i++)
       NDET[i]=file_spectral_dim;
     }

    pEngineContext->recordNumber = row_dim * col_dim;
    pEngineContext->n_alongtrack= row_dim;
    pEngineContext->n_crosstrack= col_dim;

    size_t start[] = {0, 0};
    size_t count[] = {spectral_dim, 1};

    radiance_file.getVar("radiance_wavelength", start, count, pEngineContext->buffers.lambda);
    radiance_fillvalue = radiance_file.getFillValue<double>("radiance");

    radiance_file.read_data_fields(radiance_file_data,(useSempas)?(int)SEMPAS_FIELD_MAX:(int)APEX_FIELD_MAX);

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

int apex_read(ENGINE_CONTEXT *pEngineContext, int record) {
  int rc = 0;

  assert(record > 0);
  const size_t i_alongtrack = (record - 1) / col_dim;
  const size_t i_crosstrack = (record - 1) % col_dim;

  const size_t start[] = {i_alongtrack, i_crosstrack, 0};
  const size_t count[] = {1, 1, spectral_dim};

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;
  pRecord->i_alongtrack=i_alongtrack;
  pRecord->i_crosstrack=i_crosstrack;

  try {
    radiance_file.getVar("radiance", start, count, pEngineContext->buffers.spectrum);

    if (radiance_file.hasVar("datetime"))
     {
      const size_t start_datetime[] = {i_alongtrack, 0};
      const size_t count_datetime[] = {1, 7};

      short dd[7];
      radiance_file.getVar("datetime", start_datetime, count_datetime, dd);

     }

  } catch(std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  // check the spectrum contains non-fill values:
  bool is_fill = true;
  for (size_t i=0; i!= spectral_dim && is_fill; ++i) {
    if (pEngineContext->buffers.spectrum[i] != radiance_fillvalue)
      is_fill = false;
  }
  if (is_fill) { // only fill-values
    return ERROR_ID_FILE_RECORD; // "Spectrum doesn't match selection criteria"
  }
  else if (!pEngineContext->project.instrumental.use_row[i_crosstrack]) {
    return ERROR_ID_FILE_RECORD;
  }

  if (useSempas)      // for the moment, we use only one dimension
   {
    int nsec1,nsec2;

    if (radiance_file_data[SEMPAS_FIELD_DATETIME].varData!=NULL)
     {
      pRecord->present_datetime.thedate.da_day=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7+2];
      pRecord->present_datetime.thedate.da_mon=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7+1];
      pRecord->present_datetime.thedate.da_year=((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7];
      pRecord->present_datetime.thetime.ti_hour=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7+3];
      pRecord->present_datetime.thetime.ti_min=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7+4];
      pRecord->present_datetime.thetime.ti_sec=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7+5];
      pRecord->present_datetime.millis=((short *)radiance_file_data[SEMPAS_FIELD_DATETIME].varData)[i_alongtrack*7+6];
     }

    if (radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData!=NULL)
     {
      pRecord->startDateTime.thedate.da_day=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7+2];
      pRecord->startDateTime.thedate.da_mon=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7+1];
      pRecord->startDateTime.thedate.da_year=((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7];
      pRecord->startDateTime.thetime.ti_hour=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7+3];
      pRecord->startDateTime.thetime.ti_min=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7+4];
      pRecord->startDateTime.thetime.ti_sec=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7+5];
      pRecord->startDateTime.millis=((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_START].varData)[i_alongtrack*7+6];
     }

    if (radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData!=NULL)
     {
      pRecord->endDateTime.thedate.da_day=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7+2];
      pRecord->endDateTime.thedate.da_mon=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7+1];
      pRecord->endDateTime.thedate.da_year=((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7];
      pRecord->endDateTime.thetime.ti_hour=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7+3];
      pRecord->endDateTime.thetime.ti_min=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7+4];
      pRecord->endDateTime.thetime.ti_sec=(char)((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7+5];
      pRecord->endDateTime.millis=((short *)radiance_file_data[SEMPAS_FIELD_DATETIME_END].varData)[i_alongtrack*7+6];
     }

    nsec1=pRecord->startDateTime.thetime.ti_hour*3600+pRecord->startDateTime.thetime.ti_min*60+pRecord->startDateTime.thetime.ti_sec;
    nsec2=pRecord->endDateTime.thetime.ti_hour*3600+pRecord->endDateTime.thetime.ti_min*60+pRecord->endDateTime.thetime.ti_sec;

    if (nsec2<nsec1)
     nsec2+=86400;

    pRecord->TotalExpTime=(double)nsec2-nsec1;

    pRecord->TotalAcqTime=(double)(radiance_file_data[SEMPAS_FIELD_TOTAL_TIME].varData!=NULL) ? ((int *)radiance_file_data[SEMPAS_FIELD_TOTAL_TIME].varData)[i_alongtrack] : 0;
    pRecord->NSomme=(radiance_file_data[SEMPAS_FIELD_SCANS_NUMBER].varData!=NULL) ? ((int *)radiance_file_data[SEMPAS_FIELD_SCANS_NUMBER].varData)[i_alongtrack] : 0;
    pRecord->rejected=(radiance_file_data[SEMPAS_FIELD_REJECTED_NUMBER].varData!=NULL) ? ((int *)radiance_file_data[SEMPAS_FIELD_REJECTED_NUMBER].varData)[i_alongtrack] : 0;
    pRecord->Tint=(radiance_file_data[SEMPAS_FIELD_EXPOSURE_TIME].varData!=NULL) ? ((float *)radiance_file_data[SEMPAS_FIELD_EXPOSURE_TIME].varData)[i_alongtrack]*0.001 : QDOAS_FILL_FLOAT;
    pRecord->Zm = (radiance_file_data[SEMPAS_FIELD_SOLAR_ZENITH_ANGLE].varData!=NULL) ? ((float *)radiance_file_data[SEMPAS_FIELD_SOLAR_ZENITH_ANGLE].varData)[i_alongtrack] : QDOAS_FILL_FLOAT;
    pRecord->elevationViewAngle = (radiance_file_data[SEMPAS_FIELD_VIEWING_ELEVATION_ANGLE].varData!=NULL) ? ((float *)radiance_file_data[SEMPAS_FIELD_VIEWING_ELEVATION_ANGLE].varData)[i_alongtrack] : QDOAS_FILL_FLOAT;
    pRecord->azimuthViewAngle = (radiance_file_data[SEMPAS_FIELD_VIEWING_AZIMUTH_ANGLE].varData!=NULL) ? ((float *)radiance_file_data[SEMPAS_FIELD_VIEWING_AZIMUTH_ANGLE].varData)[i_alongtrack] : QDOAS_FILL_FLOAT;
   }
  else
   {
    size_t latSize;

    latSize=radiance_file_data[APEX_FIELD_LAT].varDimsLen[0];

    pRecord->latitude = (radiance_file_data[APEX_FIELD_LAT].varData!=NULL) ? ((double *)radiance_file_data[APEX_FIELD_LAT].varData)[(latSize==row_dim)?record -1:0] : QDOAS_FILL_DOUBLE;
    pRecord->longitude = (radiance_file_data[APEX_FIELD_LON].varData!=NULL) ? ((double *)radiance_file_data[APEX_FIELD_LON].varData)[(latSize==row_dim)?record -1:0] : QDOAS_FILL_DOUBLE;
    pRecord->Zm = (radiance_file_data[APEX_FIELD_SZA].varData!=NULL) ? ((double *)radiance_file_data[APEX_FIELD_SZA].varData)[record-1] : QDOAS_FILL_DOUBLE;
    pRecord->zenithViewAngle = (radiance_file_data[APEX_FIELD_VZA].varData!=NULL) ? ((double *)radiance_file_data[APEX_FIELD_VZA].varData)[record-1] : QDOAS_FILL_DOUBLE;
    pRecord->azimuthViewAngle = (radiance_file_data[APEX_FIELD_RAA].varData!=NULL) ? ((double *)radiance_file_data[APEX_FIELD_RAA].varData)[record-1] : QDOAS_FILL_DOUBLE;
   }

  return rc;
}

int apex_get_reference(const char *filename, int i_crosstrack, double *lambda, double *spectrum, int *n_wavel) {
  auto& radiances = reference_radiances[filename];
  auto& lambda_ref = reference_wavelengths[filename];

  if (!radiances.size() ){
    try {
      NetCDFFile reference_file(filename);
      radiances = load_reference_radiances(reference_file);
      lambda_ref.resize(reference_file.dimLen("spectral_dim"));
      const size_t start[] = {0};
      const size_t count[] = {lambda_ref.size()};
      reference_file.getVar("reference_wavelength", start, count, lambda_ref.data() );
    } catch(std::runtime_error& e) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                           (string {"Error reading reference spectra from file '"} + filename + "'").c_str() );
    }
  }

  if (radiances.size() != col_dim || radiances.at(0).size() != spectral_dim) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                         (string { "reference file '" } + filename + "' spectral or column dimension don't match dimensions from '" + init_filename + "'").c_str());
  }
  *n_wavel = spectral_dim;
  for (size_t i=0; i<spectral_dim; ++i) {
    lambda[i] = lambda_ref[i];
    spectrum[i] = radiances.at(i_crosstrack)[i];
  }
  return ERROR_ID_NO;
}

void apex_clean() {

  radiance_file.release_data_fields();
  radiance_file.close();
  useSempas=0;

//  radiance_file_data=data_fields();


  init_filename = "";

  reference_wavelengths.clear();
  reference_radiances.clear();
  spectral_dim = col_dim = row_dim = 0;
}
