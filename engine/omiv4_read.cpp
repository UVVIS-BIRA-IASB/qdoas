/* Copyright (C) 2023 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 */

#include <cassert>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <tuple>
#include <array>

#include <boost/multi_array.hpp>

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
namespace fs = std::filesystem;

template<typename T>
using array2d = boost::const_multi_array_ref<T, 2>;

namespace {
  // Due to compression settings of OMI Collection 4 L1B, we need a large NetCDF/HDF5 cache to get acceptable performance.
  const size_t nc_cache_size = 150 * 1024 * 1024;

  const map<int, const string> band_names = {
    {PRJCT_INSTR_OMI_TYPE_UV1, "BAND1"},
    {PRJCT_INSTR_OMI_TYPE_UV2, "BAND2"},
    {PRJCT_INSTR_OMI_TYPE_VIS, "BAND3"}};

  class orbit_file;
  class spectrum;

  std::unique_ptr<orbit_file> current_orbit;

  // An automatic radiance reference is generated from all radiance files in the same directory as current orbit file.
  // Keep the list of those files:
  vector<string> reference_orbit_files;

  // irradiance spectra for each row, for each irradiance filename:
  map<string,vector<spectrum>> irradiance_references;

  // track if we have initialized NDET already.  In analysis mode, this is initialized
  // based on irradiance references; in browse/export mode, this is initialized during OMIV4_set
  bool have_init_ndet = false;

  struct geodata {
    vector<float> sza, vza, saa,  vaa,
      lon, lat,
      lon_bounds, lat_bounds,
      sat_lon, sat_lat, sat_alt;
  };

  struct spectrum {
    spectrum(size_t size_spectral=0) : lambda(size_spectral), spec(size_spectral), sigma(size_spectral) {}
    spectrum(vector<double> lambda, vector<double> spec, vector<double> sigma) : lambda(lambda), spec(spec), sigma(sigma) {
      if (lambda.size() != spec.size() || lambda.size() != sigma.size()) {
        throw std::runtime_error("Spectrum wavelength, radiance and sigma vectors must have equal size.");
      }
    }
    vector<double> lambda;
    vector<double> spec;
    vector<double> sigma;
  };

  vector<double> calculate_wavelengths(const vector<double>& coeffs, int ref_col, size_t num_wavelengths) {
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

  vector<double> get_wavelengths(const NetCDFGroup& instrGroup, size_t i_scanline, size_t i_groundpixel, size_t num_wavelengths, size_t num_coeffs, int ref_col){
    vector<double> coeffs(num_coeffs);

    const size_t start[] = {0, i_scanline, i_groundpixel, 0};
    const size_t count[] = {1, 1, 1, num_coeffs};

    instrGroup.getVar("wavelength_coefficient", start, count, coeffs.data() );

    return calculate_wavelengths(coeffs, ref_col, num_wavelengths);
  }

  vector<double> get_wavelengths_irrad(const NetCDFGroup& instrGroup, size_t i_pixel, size_t num_wavelengths){
    size_t start_lref[] = {0};
    size_t count_lref[] = {1};
    int ref_col;
    instrGroup.getVar("wavelength_reference_column", start_lref, count_lref, &ref_col);

    size_t num_coeffs = instrGroup.dimLen("n_wavelength_poly");
    vector<double> coeffs(num_coeffs);

    const size_t start[] = {0, 0, i_pixel, 0};
    const size_t count[] = {1, 1, 1, num_coeffs};

    const size_t *pstart = start;
    const size_t *pcount = count;
    // mean irradiance spectrum doesn't have a "scanline" dimension
    // for the wavelength_coefficient variable => use the last 3 start[]/count[] entries.
    if (instrGroup.numDims("wavelength_coefficient") == 3) {
      pstart = &start[1];
      pcount = &count[1];
    }

    instrGroup.getVar("wavelength_coefficient", pstart, pcount, coeffs.data() );

    return calculate_wavelengths(coeffs, ref_col, num_wavelengths);

  }

  vector<spectrum> load_irradiance_ref(const string& filename, const string& band) {
    NetCDFFile refFile(filename);
    auto irrObsGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/OBSERVATIONS");
    auto instrGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/INSTRUMENT");
    // get dimensions:
    size_t size_pixel = irrObsGroup.dimLen("pixel");
    size_t nlambda = irrObsGroup.hasDim("spectral_channel") ?
      irrObsGroup.dimLen("spectral_channel") :
      irrObsGroup.dimLen("spectral");  // mean irradiance spectrum uses dimension name "spectral"

    vector<spectrum> result(size_pixel);
    for(size_t i=0; i<size_pixel; ++i) {
      spectrum& ref_pixel = result[i];

      auto lambda = get_wavelengths_irrad(instrGroup, i, nlambda);
      vector<double> irra(nlambda);
      vector<double> noise(nlambda);

      // irradiance & irradiance_noise have dimensions
      // (time, scanline, pixel, spectral_channel)
      size_t start_irr[] = {0, 0, i, 0};
      size_t count_irr[] = {1, 1, 1, nlambda};
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

  class orbit_file {

  public:
    orbit_file(string filename, string band) : filename(filename), band(band),
                                               ncfile(filename, NetCDFFile::Mode::read, nc_cache_size) {
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

      size_t start[] = {0, 0, 0};
      size_t count[] = {1, size_scanline, size_groundpixel};
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
      xtrack_quality.resize(size_scanline * size_groundpixel);
      obsGroup.getVar("xtrack_quality", start, count, xtrack_quality.data());
    }

    void get_record_geodata(RECORD_INFO *pRecord, int record) {
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

    spectrum radiance(size_t i_scanline, size_t i_pixel) {
      // dimensions of radiance & error are
      // ('time','scanline','ground_pixel','spectral_channel')
      const size_t start[] = {0, i_scanline, i_pixel, 0};
      const size_t count[] = {1, 1, 1, size_spectral};

      const auto obsGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");
      vector<double> rad(size_spectral);
      vector<double> sigma;
      vector<signed char> rad_noise(size_spectral);
      obsGroup.getVar("radiance", start, count, rad.data());
      obsGroup.getVar("radiance_noise", start, count, rad_noise.data() );

      const auto instrGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
      vector<double> wavel = get_wavelengths(instrGroup, i_scanline, i_pixel, size_spectral,
                                                    size_wavelength_coeffs, wavelength_ref_col);

      // calculate radiance random error; use radiance fill value if noise variable contains a fill value.
      for (size_t i=0; i < rad.size(); ++i) {
        double ri = rad[i];
        double ni = rad_noise[i];
        if (ni != radiance_fill) {
          sigma.push_back(ri/(std::pow(10.0, ni/10.0)));
        } else {
          sigma.push_back(radiance_fill);
        }
      }

      return spectrum(std::move(wavel), std::move(rad), std::move(sigma));
    }

    vector<vector<double>> nominal_wavelengths() {
      vector<vector<double>> result(size_groundpixel);
      NetCDFGroup instrGroup = ncfile.getGroup(band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
      size_t count_wl[] = {1, 1, size_spectral};

      for(size_t i=0; i<size_groundpixel; ++i) {
        result[i].resize(size_spectral);
        size_t start_wl[] = {0, i, 0};
        instrGroup.getVar("wavelength", start_wl, count_wl, result[i].data());
      }
      return result;
    }

    void get_datetime(size_t i_scanline, struct datetime *date_time) {
      get_utc_date(reference_time, delta_time[i_scanline], date_time);
    }

    void get_orbit_date(int &year, int &month, int &day) {
      year = orbit_year;
      month = orbit_month;
      day = orbit_day;
    }

    const string& get_filename() {return filename;};
    const string& get_band() {return band;}

    size_t n_records() {return size_scanline * size_groundpixel;}
    size_t n_alongtrack() {return size_scanline;}
    size_t n_crosstrack() {return size_groundpixel;}
    size_t n_lambda() {return size_spectral;}

    double fill_value() {return radiance_fill;}

    const geodata& get_geodata() {return orbit_geodata;}
    const vector<uint16_t>& get_xtrack_quality() {return xtrack_quality;}

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
    vector<uint16_t> xtrack_quality; // row anomaly impact
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

  vector<string> find_reference_orbits(const string& filename) {
    vector<string> result;
    const size_t prefix_len = strlen("OMI-Aura_L1-OML1BRXX"); // length of the standard OMI L1B radiance filename prefix
    const string prefix(fs::path(filename).filename().string().substr(0, prefix_len));
    for (auto && f: fs::directory_iterator(fs::path(filename).parent_path())) {
      const string basename = f.path().filename().string();
      // keep only files in same directory starting with <prefix> and ending with ".nc":
      if (basename.rfind(prefix, 0) == 0 &&
          basename.compare(basename.size() - 3, 3, ".nc") == 0) {
        result.push_back(f.path().string());
      }
    }
    return result;
  }

  // check if an earthshine observation matches selection criteria to include it in the earthshine reference.
  bool use_as_reference(double lon, double lat, double sza, const FENO *pTabFeno) {
    float lon_min = pTabFeno->refLonMin;
    float lon_max = pTabFeno->refLonMax;
    float lat_min = pTabFeno->refLatMin;
    float lat_max = pTabFeno->refLatMax;
    float sza_min = pTabFeno->refSZA - pTabFeno->refSZADelta;
    float sza_max = pTabFeno->refSZA + pTabFeno->refSZADelta;

    // if a range (0.0,0.0) is chosen ( < EPSILON), we don't select based on this parameter
    bool use_lon = lon_max - lon_min > EPSILON;
    bool use_lat = lat_max - lat_min > EPSILON;
    bool use_sza = sza_max - sza_min > EPSILON;

    bool lon_ok = false;
    bool lat_ok = false;
    bool sza_ok = false;
    if (lon_min <= lon && lon_max >= lon)
      lon_ok = true;
    if (lat_min <= lat && lat_max >= lat)
      lat_ok =true;
    if (sza_min <= sza && sza_max >= sza)
      sza_ok = true;

    return(lon_ok || !use_lon)
      && (lat_ok || !use_lat)
      && (sza_ok || !use_sza);
  }

  void prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, const vector<string>& reference_orbit_files, void *responseHandle) {
    // Reference spectra per TabFeno which needs them:
    map<int, vector<vector<double>>> reference_spectra;
    map<int, vector<vector<double>>> reference_sigmasquared;
    map<int, vector<vector<size_t>>> num_spectra;

    for (int win=0; win!=NFeno; ++win) {
      // Take TabFeno for row 0; radiance ref settings should be equal for all rows.
      const FENO *pTabFeno = &TabFeno[0][win];
      if(!pTabFeno->hidden
         && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
         && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) {
        auto& refs = reference_spectra[win];
        auto& sigmas = reference_sigmasquared[win];
        auto& counts = num_spectra[win];
        for (size_t row=0; row != current_orbit->n_crosstrack(); ++row) {
          // Initialize a vector of 0's for every row.
          refs.push_back(vector<double>(current_orbit->n_lambda()));
          sigmas.push_back(vector<double>(current_orbit->n_lambda()));
          counts.push_back(vector<size_t>(current_orbit->n_lambda()));
        }
      }
    }

    for (const string& f: reference_orbit_files) {
      orbit_file orbit(f, current_orbit->get_band());
      // Loop over all observations in the orbit, check if an observation should be used in the automatic reference for one or more analysis windows
      const auto& geo = orbit.get_geodata();
      auto extent = boost::extents[orbit.n_alongtrack()][orbit.n_crosstrack()];
      const array2d<float> latitudes(geo.lat.data(), extent);
      const array2d<float> longitudes(geo.lon.data(), extent);
      const array2d<float> szangles(geo.sza.data(), extent);
      for (size_t scan=0; scan != orbit.n_alongtrack(); ++scan) {
        for (size_t row=0; row != orbit.n_crosstrack(); ++row){
          // placeholder for the actual spectrum.  We will fill it with the radiance for (scan, row) as
          // soon as we find an analysis window which can use it in the reference.
          std::optional<spectrum> radiance;

          const float lon_l2 = longitudes[scan][row];
          const float lon = lon_l2 >= 0. ? lon_l2 : lon_l2 + 360.; // longitudes in range [0., 360.]
          const float lat = latitudes[scan][row];
          const float sza = szangles[scan][row];
          for (int win=0; win!=NFeno; ++win) {
            const FENO *pTabFeno = &TabFeno[row][win];
            if (pTabFeno->hidden) continue;

            if(pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
               && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
               && use_as_reference(lon,lat,sza,pTabFeno)) {
              // read the spectrum if it hasn't been read yet:
              if (!radiance.has_value()) {
                radiance = orbit.radiance(scan,row);
              }

              auto& refs = reference_spectra[win];
              auto& sigmas = reference_sigmasquared[win];
              auto& counts = num_spectra[win];

              const spectrum& rad = radiance.value();
              // add non-fill values to average radiance and error
              for (size_t i=0; i!=current_orbit->n_lambda(); ++i) {
                double ri = rad.spec[i];
                double si = rad.sigma[i];
                if (ri != orbit.fill_value() && si != orbit.fill_value()) {
                  refs[row][i] += ri;
                  sigmas[row][i] += si*si;
                  ++counts[row][i];
                }
              }
            }
          }
        }
      }
    }

    // We assume nominal_wavelength grid for the current orbit file is valid for all radiances used.
    vector<vector<double>> wavelengths = current_orbit->nominal_wavelengths();
    for (auto& iref : reference_spectra) {
      auto& [win, refs] = iref;
      auto& sigmas = reference_sigmasquared[win];
      auto& counts = num_spectra[win];

      for (size_t row=0; row!=refs.size(); ++row) {
        // Average radiances and errors
        for (size_t i=0; i!=current_orbit->n_lambda(); ++i) {
          refs[row][i] /= counts[row][i];
          sigmas[row][i] = std::sqrt(sigmas[row][i])/counts[row][i];
        }

        // Interpolate onto reference grid
        auto& tabFeno = TabFeno[row][win];
        const size_t n_wavel = tabFeno.NDET;
        vector<double> derivs(wavelengths[row].size());
        int rc = SPLINE_Deriv2(wavelengths[row].data(), refs[row].data(), derivs.data() , derivs.size(), __func__);
        if (rc) {
          throw std::runtime_error("Error performing spline interpolation on radiance reference spectrum.");
        }
        SPLINE_Vector(wavelengths[row].data(), refs[row].data(), derivs.data(), derivs.size(), tabFeno.LambdaRef, tabFeno.Sref, n_wavel, SPLINE_CUBIC);
        VECTOR_NormalizeVector(tabFeno.Sref-1, n_wavel, &tabFeno.refNormFact, __func__);
      }
    }

    for(int i=0; i<ANALYSE_swathSize; i++) {
      // fit wavelength shift between calibrated solar irradiance
      // and automatic reference spectrum and apply this shift to
      // absorption crosssections
      int rc = ANALYSE_AlignReference(pEngineContext,2,responseHandle,i);
      if (rc) {
        throw std::runtime_error("Error aligning radiance reference with calibrated irradiance spectrum.");
      }
    }
  }
}

void OMIV4_cleanup() {
  current_orbit.reset();

  have_init_ndet = false;

  irradiance_references.clear();
  reference_orbit_files.clear();
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
    const auto& spec = current_orbit->radiance(indexScanline, indexPixel);

    auto lambda = pEngineContext->buffers.lambda;
    auto spectrum =pEngineContext->buffers.spectrum;
    auto sigma =pEngineContext->buffers.sigmaSpec;

    // Fill pEngineContext buffers:
    //  - drop pixels with fill values,
    //  - shift non-fill values forward,
    //  - fill up the remaining space at the end with dummy values.
    size_t j=0;
    for (size_t i=0; i<current_orbit->n_lambda(); ++i) {
      double li = spec.lambda[i];
      double ri = spec.spec[i];
      double si = spec.sigma[i];

      if (ri != current_orbit->fill_value() && si != current_orbit->fill_value()) {
        lambda[j]=li;
        spectrum[j]=ri;
        sigma[j]=si;
        j++;
      }
    }

    if (j == 0) {
      // All fill values, can't use this spectrum:
      throw std::runtime_error("Spectrum consists of only fill values");
    }
    // If we have not filled the buffer completely due to fill values, fill up with dummy values at the end:
    double last_lambda = lambda[j-1];
    for (size_t i=j; i<current_orbit->n_lambda(); ++i) {
      last_lambda+=1;
      lambda[i]=last_lambda;
      spectrum[i]=0;
      sigma[i]=1;
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_NETCDF, e.what());
  }

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;

  pRecord->i_alongtrack = indexScanline;
  pRecord->i_crosstrack = indexPixel;

  pRecord->useErrors = 1;
  pRecord->xtrack_QF = current_orbit->get_xtrack_quality()[record - 1];
  current_orbit->get_record_geodata(pRecord, record);
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

// Initialize OMI state from irradiance references:
// - Set spectral band string
// - Initialize ANALYSE_swathSize and NDET[] based on input file dimensions
// - Read the irradiance references per detector row, for each analysis window
//
// TODO: check: if running browse/export spectra (or run calibration?) this function is not called -> make sure any required variables are properly initialized in that case as well
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
        continue;
      }

      auto this_ref = load_irradiance_ref(analysis_windows[i].refOneFile, band);

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

int OMIV4_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle) {
  // First check if the current orbit file is part of the set of
  // reference radiance files, in which case the current radiance
  // reference is valid and we don't have to do anything
  if (std::find(reference_orbit_files.begin(), reference_orbit_files.end(), pEngineContext->fileInfo.fileName) != reference_orbit_files.end()) {
    return ERROR_ID_NO;
  }

  // If we get here, we need to build a new radiance reference.
  try {
    auto ref_files = find_reference_orbits(current_orbit->get_filename());
    prepare_automatic_reference(pEngineContext, ref_files, responseHandle);
    // if reference selection was succesful, update the list of current orbit files.
    reference_orbit_files = std::move(ref_files);
  } catch (std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_REFERENCE_SELECTION, e.what());
  }
  return ERROR_ID_NO;
}
