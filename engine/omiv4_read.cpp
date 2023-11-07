/* Copyright (C) 2023 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 */

#include <string>
#include <vector>
#include <map>
#include <tuple>

#include "boost/multi_array.hpp"

#include "netcdfwrapper.h"
#include "date_util.h"

extern "C" {
  #include "mediate_analysis_window.h"
  #include "analyse.h"
  #include "constants.h"
  #include "engine_context.h"
  #include "spline.h"
  #include "vector.h"
  #include "winthrd.h"
  #include "omiv4_read.h"
}

using std::string;
using std::vector;
using std::map;

namespace {
    struct geodata {
    vector<float> sza, vza, saa,  vaa,
      lon, lat,
      lon_bounds, lat_bounds,
      sat_lon, sat_lat, sat_alt;
  };

  // irradiance reference
  struct refspec {
    refspec() : lambda(), irradiance(), sigma() {};
    vector<double> lambda;
    vector<double> irradiance;
    vector<double> sigma;
  };

}

static const map<int, const string> band_names = {
  {PRJCT_INSTR_OMI_TYPE_UV1, "BAND1"},
  {PRJCT_INSTR_OMI_TYPE_UV2, "BAND2"},
  {PRJCT_INSTR_OMI_TYPE_VIS, "BAND3"}};

static vector<int> delta_time; // number of milliseconds after reference_time

static NetCDFFile current_file;
static string current_filename;
static string current_band;

static geodata current_geodata;
static int wavelength_ref_col;

// irradiance spectra for each row, for each irradiance filename:
static map<string,vector<refspec>> irradiance_references;

// track if we have initialized NDET already.  In analysis mode, this is initialized
// based on irradiance references; in browse/export mode, this is initialized during OMIV4_set
static bool have_init_ndet = false;

static size_t size_spectral; // number of wavelengths per spectrum
static size_t size_scanline; // number of measurements (i.e. along track)
static size_t size_groundpixel; // number of detector rows (i.e. cross-track)
static size_t size_wavelength_coeffs; // number of wavelength coefficients

static time_t reference_time; // since orbit start date

static geodata read_geodata(const NetCDFGroup& geo_group, size_t n_scanline, size_t n_groundpixel) {

  geodata result;

  // each element of this array contains the name of the netCDF
  // variable, the vector in which we want to store the data, and the
  // size of each element
  using std::ref;
  std::array<std::tuple<const string, vector<float>&, size_t>, 11> geovars {
    make_tuple("solar_zenith_angle", ref(result.sza), 1),
      make_tuple("viewing_zenith_angle", ref(result.vza), 1),
      make_tuple("solar_azimuth_angle", ref(result.saa), 1),
      make_tuple("viewing_azimuth_angle", ref(result.vaa), 1),
      make_tuple("latitude", ref(result.lat), 1),
      make_tuple("longitude", ref(result.lon), 1),
      make_tuple("longitude_bounds", ref(result.lon_bounds), 4),
      make_tuple("latitude_bounds", ref(result.lat_bounds), 4),
      make_tuple("satellite_longitude", ref(result.sat_lon), 1),
      make_tuple("satellite_latitude", ref(result.sat_lat), 1),
      make_tuple("satellite_altitude", ref(result.sat_alt), 1)};

  for (auto& var : geovars) {
    const string& name =std::get<0>(var);
    auto& target = std::get<1>(var);
    const size_t elem_size = std::get<2>(var);

    target.resize(n_scanline * n_groundpixel * elem_size);
    const size_t start[] = {0, 0, 0, 0};
    const size_t count[] = {1, n_scanline, n_groundpixel, elem_size};
    geo_group.getVar(name, start, count, target.data() );
  }

  return result;
}

static vector<double> get_wavelengths(const NetCDFGroup& instrGroup, size_t i_scanline, size_t i_groundpixel, size_t num_wavelengths, size_t num_coeffs, int ref_col){
  vector<double> coeffs(num_coeffs);
  const size_t start[] = {0, i_scanline, i_groundpixel, 0};
  const size_t count[] = {1, 1, 1, num_coeffs};
  instrGroup.getVar("wavelength_coefficient", start, count, coeffs.data() );

  vector <double> wavelengths(num_wavelengths);
  // compute wavelength using polynomial sum_0^N-1 ( c(n) * (i - i_ref)^n)
  int delta_i_ref = -ref_col;  // (i - iref) for i = 0...
  for (auto& lambda: wavelengths) {
    int delta_i_pow = 1; // (i - iref)^n for n = 0...
    for (const auto& coeff: coeffs) {
      lambda += coeff * delta_i_pow;
      delta_i_pow *= delta_i_ref;
    }
    ++delta_i_ref;
  }
  return wavelengths;
}

static void get_geodata(RECORD_INFO *pRecord, const geodata& geo, int record) {
  pRecord->latitude= geo.lat[record-1];
  pRecord->longitude= geo.lon[record-1];
  pRecord->Zm= geo.sza[record-1];
  pRecord->Azimuth= geo.saa[record-1];
  pRecord->zenithViewAngle= geo.vza[record-1];
  pRecord->azimuthViewAngle= geo.vaa[record-1];

  // ugly casting because we store the (num_records * 4) corner arrays as a flat array:
  auto lon_bounds = reinterpret_cast<const double(*)[4]>(geo.lon_bounds.data());
  auto lat_bounds = reinterpret_cast<const double(*)[4]>(geo.lat_bounds.data());
  for (int i=0; i!=4; ++i) {
    pRecord->satellite.cornerlons[i] = lon_bounds[record-1][i];
    pRecord->satellite.cornerlats[i] = lat_bounds[record-1][i];
  }
  pRecord->satellite.longitude = geo.sat_lon[record-1];
  pRecord->satellite.latitude = geo.sat_lat[record-1];
  pRecord->satellite.altitude = geo.sat_alt[record-1];
}

void OMIV4_cleanup() {
  current_file.close();
  current_filename = "";
  current_band = "";

  size_spectral = size_scanline = size_groundpixel = 0;
  reference_time = 0;
  have_init_ndet = false;

  current_geodata = geodata();
  delta_time.clear();
  irradiance_references.clear();
}

int OMIV4_read(ENGINE_CONTEXT *pEngineContext,int record) {
  assert(record > 0); // record is the requested record number, starting from 1
  int rc = 0;

  NetCDFGroup obsGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

  const size_t indexScanline = (record - 1) / size_groundpixel;
  const size_t indexPixel = (record - 1) % size_groundpixel;

  // spectral dimension should match NDET value.
  assert(size_spectral == NDET[indexPixel]);

  if (!pEngineContext->project.instrumental.use_row[indexPixel]) {
    return ERROR_ID_FILE_RECORD;
  }

  // in analysis mode, variables must have been initialized by tropomi_init()
  if (THRD_id==THREAD_TYPE_ANALYSIS) {
    // We need an irradiance spectrum for the irradiance plot.  We
    // give the one from the first analysis window.
    const refspec& irrad_ref = irradiance_references.begin()->second.at(indexPixel);
    for (size_t i=0; i<irrad_ref.lambda.size(); ++i) {
      pEngineContext->buffers.lambda_irrad[i] = irrad_ref.lambda[i];
      pEngineContext->buffers.irrad[i] = irrad_ref.irradiance[i];
    }
  }

  // dimensions of radiance & error are
  // ('time','scanline','ground_pixel','spectral_channel')
  const size_t start[] = {0,indexScanline, indexPixel, 0};
  const size_t start_scale[] = {0,indexScanline, indexPixel};

  const size_t count[] = {1, 1, 1, size_spectral};
  const size_t onecount[] = {1, 1, 1};


  vector<unsigned short int> rad_int16(size_spectral);
  vector<double> rad(size_spectral);
  vector<double> scale(1);
  vector<double> rad_noise(size_spectral);

  try {
    const double fill_rad = obsGroup.getFillValue<double>("radiance");
    obsGroup.getVar("radiance_noise", start, count, rad_noise.data() );
    obsGroup.getVar("radiance", start, count, rad.data());

    const double fill_noise = obsGroup.getFillValue<double>("radiance_noise");
    const auto instrGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
    const vector<double> lambda = get_wavelengths(instrGroup, indexScanline, indexPixel, size_spectral,
                                                   size_wavelength_coeffs, wavelength_ref_col);
    // copy non-fill values to buffers:
    size_t j=0;
    for (size_t i=0; i<rad.size(); ++i) {
      double li = lambda[i];
      double ri = rad[i];
      double ni = rad_noise[i];

      if (ri != fill_rad && ni != fill_noise) {
        pEngineContext->buffers.lambda[j]=li;
        pEngineContext->buffers.spectrum[j]=ri;
        pEngineContext->buffers.sigmaSpec[j]=ri/(std::pow(10.0, ni/10.0));
        j++;
      }
    }

    if (j == 0) {
      // All fill values, can't use this spectrum:
      return ERROR_ID_FILE_RECORD;
    }
    // If we have not filled the buffer completely due to fill values, fill up with dummy values at the end:
    double last_lambda = pEngineContext->buffers.lambda[j-1];
    for (size_t i=j; j<rad.size(); ++i) {
      last_lambda+=1;
      pEngineContext->buffers.lambda[i]=last_lambda;
      pEngineContext->buffers.spectrum[i]=0;
      pEngineContext->buffers.sigmaSpec[i]=1;
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;

  pRecord->i_alongtrack = indexScanline;
  pRecord->i_crosstrack = indexPixel;

  pRecord->useErrors = 1;
  get_geodata(pRecord, current_geodata, record);

  int ms;
  get_utc_date(reference_time, delta_time[indexScanline], &pRecord->present_datetime);

  return rc;
}

int OMIV4_set(ENGINE_CONTEXT *pEngineContext) {
  int rc = 0;

  try {
    current_file = NetCDFFile(pEngineContext->fileInfo.fileName);
    current_filename=pEngineContext->fileInfo.fileName;
    current_band = band_names.at(pEngineContext->project.instrumental.omi.spectralType);

    reference_time = parse_utc_date(current_file.getAttText("time_reference"));

    const auto obsGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

    size_scanline = obsGroup.dimLen("scanline");
    size_spectral = obsGroup.dimLen("spectral_channel");
    size_groundpixel = obsGroup.dimLen("ground_pixel");

    if (!have_init_ndet) { // if not running in analysis mode, NDET has not yet been set at this point
      for (size_t i=0; i!=size_groundpixel; ++i) {
        NDET[i] = size_spectral;
      }
      have_init_ndet = true;
    }

    pEngineContext->recordNumber = size_groundpixel * size_scanline;
    pEngineContext->n_alongtrack = size_scanline;
    pEngineContext->n_crosstrack = size_groundpixel;

    size_t start[] = {0, 0};
    size_t count[] = {1, size_scanline};
    delta_time.resize(size_scanline);
    obsGroup.getVar("delta_time", start, count, delta_time.data());

    const auto geo_group = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/GEODATA");
    current_geodata = read_geodata(geo_group, size_scanline, size_groundpixel);

    const auto instrGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
    size_t start_lref[] = {0};
    size_t count_lref[] = {1};
    instrGroup.getVar("wavelength_reference_column", start_lref, count_lref, &wavelength_ref_col);
    size_wavelength_coeffs = instrGroup.dimLen("n_wavelength_poly");

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

static vector<refspec> loadReference(const string& filename, const string& band) {
  NetCDFFile refFile(filename);
  auto irrObsGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/OBSERVATIONS");
  auto instrGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/INSTRUMENT");
  // get dimensions:
  size_t size_pixel = irrObsGroup.dimLen("pixel");
  size_t nlambda = irrObsGroup.dimLen("spectral_channel");

  size_t start_lref[] = {0};
  size_t count_lref[] = {1};
  int ref_col_irr;
  instrGroup.getVar("wavelength_reference_column", start_lref, count_lref, &ref_col_irr);
  size_t wavelength_coeffs_irr = instrGroup.dimLen("n_wavelength_poly");

  vector<refspec> result(size_pixel);
  for(size_t i=0; i<size_pixel; ++i) {
    refspec& ref_pixel = result[i];

    // irradiance & irradiance_noise have dimensions
    // (time, scanline, pixel, spectral_channel)
    size_t start_irr[] = {0, 0, i, 0};
    size_t count_irr[] = {1, 1, 1, nlambda};

    // calibrated_wavelength has dimensions
    // (time, pixel, spectral_channel)
    size_t start_lambda[] = {0, i, 0};
    size_t count_lambda[] = {1, 1, nlambda};

    auto lambda = get_wavelengths(instrGroup, 0, i, nlambda, wavelength_coeffs_irr, ref_col_irr);
    vector<double> irra(nlambda);
    vector<double> noise(nlambda);

    irrObsGroup.getVar("irradiance", start_irr, count_irr, irra.data());
    irrObsGroup.getVar("irradiance_noise", start_irr, count_irr, noise.data());

    const double fill_irra = irrObsGroup.getFillValue<double>("irradiance");
    const double fill_noise = irrObsGroup.getFillValue<double>("irradiance_noise");

    size_t num_wavelengths = 0;
    double lambda_last = 0.;
    for (size_t j=0; j<nlambda; ++j) {
      double lj = lambda[j];
      double ij = irra[j];
      double nj = noise[j];
      if (ij != fill_irra && nj != fill_noise) {
        ++num_wavelengths;
        lambda_last = lj;
        ref_pixel.lambda.push_back(lj);
        ref_pixel.irradiance.push_back(ij);
        ref_pixel.sigma.push_back(ij/(std::pow(10.0, nj/10.0)));
      }
    }
    // If we skipped some fill values, fill the end of the buffer with dummy values:
    for (size_t j=num_wavelengths; j<nlambda; ++j) {
      lambda_last += 1.;
      ref_pixel.lambda.push_back(lambda_last);
      ref_pixel.irradiance.push_back(0.);
      ref_pixel.sigma.push_back(1.);
    }
    assert(ref_pixel.irradiance.size() == nlambda);
  }

  return result;
}


// Initialize OMI state from irradiance references:
// - Set spectral band string
// - Initialize ANALYSE_swathSize and NDET[] based on input file dimensions
// - Read the irradiance references per detector row, for each analysis window
//
// TODO: check: if running browse/export spectra (or run calibration?) this function is not called -> make sure any required variables are properly initialized in that case as well
// TODO: check size_spectral / from irradiance matches what is found for radiance in OMIV4_set
int OMIV4_init_irradiances(const mediate_analysis_window_t* analysis_windows, int num_windows, const ENGINE_CONTEXT* pEngineContext) {
  int rc = 0;
  try {
    current_band = band_names.at(pEngineContext->project.instrumental.omi.spectralType);
    size_t num_pixels = 0;
    for (size_t i=0; i<MAX_SWATHSIZE; ++i) {
      NDET[i] = 0;
    }
    for (int i=0; i<num_windows; ++i) {
      const string filename = analysis_windows[i].refOneFile;
      // first check if current irradiance file is already loaded
      auto i_reference = irradiance_references.find(filename);
      if (i_reference != irradiance_references.end()) {
        // already have this file from a previous analysis window
        continue;
      }

      auto this_ref = loadReference(analysis_windows[i].refOneFile, current_band);

      // Different analysis windows may use different irradiance reference spectra, but we only allow one set of
      // dimensions per detector column. -> Check if this reference spectrum has identical dimensions as reference
      // spectra from other analysis windows:
      if (num_pixels == 0) {
        num_pixels = this_ref.size();
        ANALYSE_swathSize = num_pixels;
      } else if (num_pixels != this_ref.size()) {
        throw std::runtime_error("Number of cross-track pixels in irradiance " + filename + " doesn't match other references.");
      }
      for (size_t col=0; col<this_ref.size(); ++col) {
        if (NDET[col] == 0) {
          NDET[col] = this_ref[col].irradiance.size();
        } else if (NDET[col] != this_ref[col].irradiance.size()) {
          throw std::runtime_error("Number of wavelengths irradiance " + filename + " doesn't match other references.");
        }
      }
      irradiance_references[filename] = std::move(this_ref);
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  have_init_ndet = true;
  return rc;
}

// Load irradiance reference spectrum for the given ground pixel
// Irradiance refence must have been initialized before by call to OMIV4_init_irradiance.
int OMIV4_get_irradiance_reference(const char* file_name, int pixel, double *lambda, double *spectrum, double *sigma) {
  const auto& ref = irradiance_references.at(file_name).at(pixel);
  for (size_t i=0; i<ref.irradiance.size(); ++i) {
    lambda[i] = ref.lambda[i];
    spectrum[i] = ref.irradiance[i];
    sigma[i] = ref.sigma[i];
  }
  return 0;
}

int OMIV4_get_orbit_date(int *year, int *month, int *day) {
  struct datetime reference_datetime;
  get_utc_date(reference_time, 0, &reference_datetime);
  const auto& date = reference_datetime.thedate;
  *year = date.da_year;
  *month = date.da_mon;
  *day = date.da_day;
  return 0;
}
