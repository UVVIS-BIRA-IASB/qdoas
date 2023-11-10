/* Copyright (C) 2023 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 */

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
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

  struct spectrum {
    spectrum(size_t size_spectral=0) : lambda(size_spectral), spec(size_spectral), sigma(size_spectral) {}
    vector<double> lambda;
    vector<double> spec;
    vector<double> sigma;
  };

  vector<double> get_wavelengths(const NetCDFGroup& instrGroup, size_t i_scanline, size_t i_groundpixel, size_t num_wavelengths, size_t num_coeffs, int ref_col){
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

  class orbit_file {

  public:
    orbit_file(string filename, string band) : filename(filename), band(band), ncfile(filename) {
      const auto utc_date = ncfile.getAttText("time_reference");
      reference_time = parse_utc_date(utc_date);
      std::istringstream utc(utc_date);
      utc >> orbit_year;
      utc.ignore(1,'-');
      utc >> orbit_month;
      utc.ignore(1,'-');
      utc >> orbit_day;

      const auto obsGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

      size_scanline = obsGroup.dimLen("scanline");
      size_spectral = obsGroup.dimLen("spectral_channel");
      size_groundpixel = obsGroup.dimLen("ground_pixel");

      size_t start[] = {0, 0};
      size_t count[] = {1, size_scanline};
      delta_time.resize(size_scanline);
      obsGroup.getVar("delta_time", start, count, delta_time.data());

      radiance_fill = obsGroup.getFillValue<double>("radiance");
      noise_fill = obsGroup.getFillValue<signed char>("radiance_noise");

      const auto instrGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
      size_t start_lref[] = {0};
      size_t count_lref[] = {1};
      instrGroup.getVar("wavelength_reference_column", start_lref, count_lref, &wavelength_ref_col);
      size_wavelength_coeffs = instrGroup.dimLen("n_wavelength_poly");

      read_geodata();
    }

    void get_geodata(RECORD_INFO *pRecord, int record) {
      pRecord->latitude= orbit_geodata.lat[record-1];
      pRecord->longitude= orbit_geodata.lon[record-1];
      pRecord->Zm= orbit_geodata.sza[record-1];
      pRecord->Azimuth= orbit_geodata.saa[record-1];
      pRecord->zenithViewAngle= orbit_geodata.vza[record-1];
      pRecord->azimuthViewAngle= orbit_geodata.vaa[record-1];

      // ugly casting because we store the (num_records * 4) corner arrays as a flat array:
      auto lon_bounds = reinterpret_cast<const double(*)[4]>(orbit_geodata.lon_bounds.data());
      auto lat_bounds = reinterpret_cast<const double(*)[4]>(orbit_geodata.lat_bounds.data());
      for (int i=0; i!=4; ++i) {
        pRecord->satellite.cornerlons[i] = lon_bounds[record-1][i];
        pRecord->satellite.cornerlats[i] = lat_bounds[record-1][i];
      }
      pRecord->satellite.longitude = orbit_geodata.sat_lon[record-1];
      pRecord->satellite.latitude = orbit_geodata.sat_lat[record-1];
      pRecord->satellite.altitude = orbit_geodata.sat_alt[record-1];
    }

    void get_radiance(size_t i_scanline, size_t i_pixel,
                      double *lambda, double *spec, double *sigma) {
      // dimensions of radiance & error are
      // ('time','scanline','ground_pixel','spectral_channel')
      const size_t start[] = {0, i_scanline, i_pixel, 0};
      const size_t count[] = {1, 1, 1, size_spectral};

      const auto obsGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");
      vector<double> rad(size_spectral);
      vector<signed char> rad_noise(size_spectral);
      obsGroup.getVar("radiance", start, count, rad.data());
      obsGroup.getVar("radiance_noise", start, count, rad_noise.data() );

      const auto instrGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
      const vector<double> wavel = get_wavelengths(instrGroup, i_scanline, i_pixel, size_spectral,
                                                    size_wavelength_coeffs, wavelength_ref_col);
      // copy non-fill values to buffers:
      size_t j=0;
      for (size_t i=0; i<rad.size(); ++i) {
        double li = wavel[i];
        double ri = rad[i];
        double ni = rad_noise[i];

        if (ri != radiance_fill && ni != noise_fill) {
          lambda[j]=li;
          spec[j]=ri;
          sigma[j]=ri/(std::pow(10.0, ni/10.0));
          j++;
        }
      }

      if (j == 0) {
        // All fill values, can't use this spectrum:
        throw std::runtime_error("Spectrum consists of only fill values");
      }
      // If we have not filled the buffer completely due to fill values, fill up with dummy values at the end:
      double last_lambda = lambda[j-1];
      for (size_t i=j; j<rad.size(); ++i) {
        last_lambda+=1;
        lambda[i]=last_lambda;
        spec[i]=0;
        sigma[i]=1;
      }
    }

    void get_datetime(size_t i_scanline, struct datetime *date_time) {
      get_utc_date(reference_time, delta_time[i_scanline], date_time);
    }

    void get_orbit_date(int &year, int &month, int &day) {
      year = orbit_year;
      month = orbit_month;
      day = orbit_day;
    }

    size_t n_records() {return size_scanline * size_groundpixel;}
    size_t n_alongtrack() {return size_scanline;}
    size_t n_crosstrack() {return size_groundpixel;}
    size_t n_lambda() {return size_spectral;}

  private:
    const string filename;
    const string band;

    NetCDFFile ncfile;

    size_t size_spectral; // number of wavelengths per spectrum
    size_t size_scanline; // number of measurements (i.e. along track)
    size_t size_groundpixel; // number of detector rows (i.e. cross-track)
    size_t size_wavelength_coeffs; // number of wavelength coefficients

    double radiance_fill;
    signed char noise_fill;

    time_t reference_time; // UTC date YYYY-MM-dd 00:00:00

    vector<int> delta_time; // number of milliseconds after reference_time
    geodata orbit_geodata;
    int wavelength_ref_col;

    int orbit_year, orbit_month, orbit_day;

    void read_geodata() {
      const auto geo_group = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/GEODATA");

      // each element of this array contains the name of the netCDF
      // variable, the vector in which we want to store the data, and the
      // size of each element
      using std::ref;
      using std::make_tuple;
      std::array<std::tuple<const string, vector<float>&, size_t>, 11> geovars {
        make_tuple("solar_zenith_angle", ref(orbit_geodata.sza), 1),
        make_tuple("viewing_zenith_angle", ref(orbit_geodata.vza), 1),
        make_tuple("solar_azimuth_angle", ref(orbit_geodata.saa), 1),
        make_tuple("viewing_azimuth_angle", ref(orbit_geodata.vaa), 1),
        make_tuple("latitude", ref(orbit_geodata.lat), 1),
        make_tuple("longitude", ref(orbit_geodata.lon), 1),
        make_tuple("longitude_bounds", ref(orbit_geodata.lon_bounds), 4),
        make_tuple("latitude_bounds", ref(orbit_geodata.lat_bounds), 4),
        make_tuple("satellite_longitude", ref(orbit_geodata.sat_lon), 1),
        make_tuple("satellite_latitude", ref(orbit_geodata.sat_lat), 1),
        make_tuple("satellite_altitude", ref(orbit_geodata.sat_alt), 1)};

      for (auto& var : geovars) {
        const string& name =std::get<0>(var);
        auto& target = std::get<1>(var);
        const size_t elem_size = std::get<2>(var);

        target.resize(size_scanline * size_groundpixel * elem_size);
        const size_t start[] = {0, 0, 0, 0};
        const size_t count[] = {1, size_scanline, size_groundpixel, elem_size};
        geo_group.getVar(name, start, count, target.data() );
      }
    }

  };
}

static const map<int, const string> band_names = {
  {PRJCT_INSTR_OMI_TYPE_UV1, "BAND1"},
  {PRJCT_INSTR_OMI_TYPE_UV2, "BAND2"},
  {PRJCT_INSTR_OMI_TYPE_VIS, "BAND3"}};

static std::unique_ptr<orbit_file> current_orbit;

// irradiance spectra for each row, for each irradiance filename:
static map<string,vector<spectrum>> irradiance_references;

// track if we have initialized NDET already.  In analysis mode, this is initialized
// based on irradiance references; in browse/export mode, this is initialized during OMIV4_set
static bool have_init_ndet = false;


void OMIV4_cleanup() {
  current_orbit.reset();

  have_init_ndet = false;

  irradiance_references.clear();
}

int OMIV4_read(ENGINE_CONTEXT *pEngineContext,int record) {
  assert(record > 0); // record is the requested record number, starting from 1
  int rc = 0;

  const size_t indexScanline = (record - 1) / current_orbit->n_crosstrack();
  const size_t indexPixel = (record - 1) % current_orbit->n_crosstrack();

  // spectral dimension should match NDET value.
  assert(current_orbit->n_lambda() == NDET[indexPixel]);

  if (!pEngineContext->project.instrumental.use_row[indexPixel]) {
    return ERROR_ID_FILE_RECORD;
  }

  // in analysis mode, variables must have been initialized by OMIV4_init()
  if (THRD_id==THREAD_TYPE_ANALYSIS) {
    // We need an irradiance spectrum for the irradiance plot.  We
    // give the one from the first analysis window.
    const spectrum& irrad_ref = irradiance_references.begin()->second.at(indexPixel);
    for (size_t i=0; i<irrad_ref.lambda.size(); ++i) {
      pEngineContext->buffers.lambda_irrad[i] = irrad_ref.lambda[i];
      pEngineContext->buffers.irrad[i] = irrad_ref.spec[i];
    }
  }

  try {
    current_orbit->get_radiance(indexScanline, indexPixel,
                           pEngineContext->buffers.lambda,
                           pEngineContext->buffers.spectrum,
                           pEngineContext->buffers.sigmaSpec);
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;

  pRecord->i_alongtrack = indexScanline;
  pRecord->i_crosstrack = indexPixel;

  pRecord->useErrors = 1;
  current_orbit->get_geodata(pRecord, record);
  current_orbit->get_datetime(indexScanline, &pRecord->present_datetime);

  return rc;
}

int OMIV4_set(ENGINE_CONTEXT *pEngineContext) {
  int rc = 0;

  try {
    current_orbit = std::make_unique<orbit_file>(pEngineContext->fileInfo.fileName, band_names.at(pEngineContext->project.instrumental.omi.spectralType));

    if (!have_init_ndet) { // if not running in analysis mode, NDET has not yet been set at this point
      for (size_t i=0; i!=current_orbit->n_crosstrack(); ++i) {
        NDET[i] = current_orbit->n_lambda();
      }
      have_init_ndet = true;
    }

    pEngineContext->recordNumber = current_orbit->n_records();
    pEngineContext->n_alongtrack = current_orbit->n_alongtrack();
    pEngineContext->n_crosstrack = current_orbit->n_crosstrack();

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

static vector<spectrum> loadReference(const string& filename, const string& band) {
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

  vector<spectrum> result(size_pixel);
  for(size_t i=0; i<size_pixel; ++i) {
    spectrum& ref_pixel = result[i];

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
        ref_pixel.spec.push_back(ij);
        ref_pixel.sigma.push_back(ij/(std::pow(10.0, nj/10.0)));
      }
    }
    // If we skipped some fill values, fill the end of the buffer with dummy values:
    for (size_t j=num_wavelengths; j<nlambda; ++j) {
      lambda_last += 1.;
      ref_pixel.lambda.push_back(lambda_last);
      ref_pixel.spec.push_back(0.);
      ref_pixel.sigma.push_back(1.);
    }
    assert(ref_pixel.spec.size() == nlambda);
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
    const string band = band_names.at(pEngineContext->project.instrumental.omi.spectralType);
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
        cout << "Skipping file " << filename << ": already loaded." << endl;
        continue;
      }

      auto this_ref = loadReference(analysis_windows[i].refOneFile, band);

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
          NDET[col] = this_ref[col].spec.size();
        } else if (NDET[col] != this_ref[col].spec.size()) {
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
  for (size_t i=0; i<ref.spec.size(); ++i) {
    lambda[i] = ref.lambda[i];
    spectrum[i] = ref.spec[i];
    sigma[i] = ref.sigma[i];
  }
  return 0;
}

int OMIV4_get_orbit_date(int *year, int *month, int *day) {
  current_orbit->get_orbit_date(*year, *month, *day);
  return 0;
}
