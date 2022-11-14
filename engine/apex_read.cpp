
#include <map>
#include <cassert>
#include <vector>
#include <cstdint>

#include "apex_read.h"

#include "netcdfwrapper.h"
#include "engine_context.h"
#include "analyse.h"
#include "output.h"

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

struct data_fields {
  vector<double> sza, vza, raa, lon, lat;
};

static struct data_fields radiance_file_data;
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

int apex_init(const char *reference_filename, ENGINE_CONTEXT *pEngineContext, const int check_size, const int idxColumn,int *useRow) {
  try {
    NetCDFFile reference_file(reference_filename);
    col_dim = reference_file.dimLen("col_dim");
    spectral_dim = reference_file.dimLen("spectral_dim");
    ANALYSE_swathSize = col_dim;
    for (size_t i=0; i< col_dim; ++i) {
      if (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME1_NETCDF) ||
           (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI)) &&
          (check_size==2)) {
         vector<int> use_row(col_dim);
         const size_t start[] = {0};
         const size_t count[] = {col_dim};
         reference_file.getVar("use_row", start, count, use_row.data());
         if (i==idxColumn){
             *useRow = use_row[i];
         }
      } else if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME1_NETCDF) &&
                (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_TROPOMI)) {
         pEngineContext->project.instrumental.use_row[i] = true;
      }
      if ((check_size == 1) && (spectral_dim > NDET[i]))
         return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                             "spectral_dim too large. This version supports spectra of maximum length "
                             TOSTRING(APEX_INIT_LENGTH));
      if (check_size == 1) NDET[i] = spectral_dim;
    }
    init_filename = reference_filename;
  } catch(std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return ERROR_ID_NO;
}

static void read_data_fields(NetCDFFile& orbit_file) {
  std::pair<string, vector<double>&> fields[] = {
    {"solar_zenith_angle", radiance_file_data.sza},
    {"viewing_zenith_angle", radiance_file_data.vza},
    {"relative_azimuth_angle", radiance_file_data.raa},
    {"longitude", radiance_file_data.lon},
    {"latitude", radiance_file_data.lat}};

  const size_t start[] = {0, 0};
  const size_t count[] = {row_dim, col_dim};
  for (auto& f: fields) {
    if (orbit_file.hasVar(f.first)) {
       f.second.resize(row_dim * col_dim);
       orbit_file.getVar(f.first, start, count, f.second.data());
    }
  }
}

int apex_set(ENGINE_CONTEXT *pEngineContext) {
  int rc = 0;

  try {
    radiance_file = NetCDFFile(pEngineContext->fileInfo.fileName);
    radiance_file_data = data_fields();

    row_dim = radiance_file.dimLen("row_dim");
    size_t file_spectral_dim = radiance_file.dimLen("spectral_dim");
    size_t file_col_dim = radiance_file.dimLen("col_dim");
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

    pEngineContext->recordNumber = row_dim * col_dim;
    pEngineContext->n_alongtrack= row_dim;
    pEngineContext->n_crosstrack= col_dim;

    size_t start[] = {0, 0};
    size_t count[] = {spectral_dim, 1};
    radiance_file.getVar("radiance_wavelength", start, count, pEngineContext->buffers.lambda);

    radiance_fillvalue = radiance_file.getFillValue<double>("radiance");

    read_data_fields(radiance_file);

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

  pRecord->latitude = radiance_file_data.lat.size() ? radiance_file_data.lat[record -1] : QDOAS_FILL_DOUBLE;
  pRecord->longitude = radiance_file_data.lon.size() ? radiance_file_data.lon[record-1] : QDOAS_FILL_DOUBLE;
  pRecord->Zm = radiance_file_data.sza.size() ? radiance_file_data.sza[record-1] : QDOAS_FILL_DOUBLE;
  pRecord->zenithViewAngle = radiance_file_data.vza.size() ? radiance_file_data.vza[record-1] : QDOAS_FILL_FLOAT;
  pRecord->azimuthViewAngle = radiance_file_data.raa.size() ? radiance_file_data.raa[record-1] : QDOAS_FILL_FLOAT;

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
  radiance_file.close();

  init_filename = "";
  reference_wavelengths.clear();
  reference_radiances.clear();
  spectral_dim = col_dim = row_dim = 0;
}

RC apex_load_file(char *filename,double *lambda, double *spectrum, int *n_wavel)
 {
   // Declarations

   RC rc;

   // Initializations

   rc=ERROR_ID_NO;

   try
    {
      NetCDFFile reference_file(filename);

      col_dim = reference_file.dimLen("col_dim");
      spectral_dim = reference_file.dimLen("spectral_dim");

      const size_t start[] = {0, 0, 0};
      const size_t count[] = {1, 1, spectral_dim};

  vector<vector<double> > radiances(reference_file.dimLen("col_dim"));
  const size_t reference_spectral_dim = reference_file.dimLen("spectral_dim");
  for (size_t i=0; i<radiances.size(); ++i) {
    vector<double>& rad = radiances[i];
    rad.resize(reference_spectral_dim);
    const size_t start[] = {i, 0};
    const size_t count[] = {1, reference_spectral_dim};
    reference_file.getVar("reference_radiance", start, count, rad.data());
    }
   }
   catch(std::runtime_error& e)
    {
     rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                         (string { "Can not open reference file " } + filename ).c_str());
    }

   // Return

   return rc;
 }
