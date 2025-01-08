#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/math/special_functions/chebyshev.hpp>
#include "boost/multi_array.hpp"

#include "netcdfwrapper.h"

extern "C" {
  #include "engine_context.h"
  #include "winthrd.h"
  #include "tempo_read.h"
}

const size_t TEMPO_XTRACK = 2048;
const size_t TEMPO_SPECTRAL_CHANNEL = 1028;

using std::map;
using std::string;
using std::vector;
using namespace boost::math;

using array3d = boost::multi_array<float, 3>;

namespace {

  const map<int, string> band_groups {
    {PRJCT_INSTR_TEMPO_BAND_UV, "band_290_490_nm"},
    {PRJCT_INSTR_TEMPO_BAND_VIS, "band_540_740_nm"}
  };

  struct irradiance {
    irradiance() : lambda(TEMPO_SPECTRAL_CHANNEL), spec(TEMPO_SPECTRAL_CHANNEL), sigma(TEMPO_SPECTRAL_CHANNEL) {};
    vector<double> lambda;
    vector<double> spec;
    vector<double> sigma;
  };

  struct auxdata {
    vector<float> sza, vza, saa,  vaa,
      lon, lat,
      lon_bounds, lat_bounds,
      sat_lon, sat_lat, sat_alt,
      red, green, blue;
    vector<unsigned char> cloud_mask;
    vector<unsigned int> pixel_quality;
  };

  class orbit_file {
  public:
    orbit_file(const string& filename, int band, string product) :
      filename(filename),
      ncfile(filename),
      band_group(ncfile.getGroup(band_groups.at(band))),
      product(product),
      mirror_step(ncfile.dimLen("mirror_step")),
      radiance_fill(band_group.getFillValue<float>(product)),
      nparams(band_group.getAttr<int>("num_coefficients", band_group.varID("wavecal_params"))[0]), // band_group.dimLen("wavecal_par") should also work, but this somehow returns '35' instead of 1?
      wavecal_params(read_wavecal_params()) { };

    virtual double get_lambda(size_t i_step, size_t i_xtrack, size_t i_spectral) = 0;

    void get_radiance(size_t i_step, size_t i_xtrack,
                      double *wavel, double *rad, double *noise) {
      const size_t start[] = {i_step, i_xtrack, 0};
      const size_t count[] = {1, 1, TEMPO_SPECTRAL_CHANNEL};

      double rad_tmp[TEMPO_SPECTRAL_CHANNEL];
      band_group.getVar(product, start, count, rad_tmp);

      double noise_tmp[TEMPO_SPECTRAL_CHANNEL];
      band_group.getVar(product + "_error", start, count, noise_tmp);

      size_t j=0;
      for (size_t i = 0; i != TEMPO_SPECTRAL_CHANNEL; ++i) {
        double li = get_lambda(i_step, i_xtrack, i);
        double ri = rad_tmp[i];
        double ni = noise_tmp[i];
        if (ri != radiance_fill) {
          wavel[j] = li;
          rad[j] = ri;
          noise[j] = ni;
          ++j;
        }
      }
      if (j == 0) { // all fill values, should somehow return ERROR_ID_RECORD higer up
        for (size_t j = 0; j != TEMPO_SPECTRAL_CHANNEL; ++j) {
          wavel[j] = j;
          rad[j] = 0.;
          noise[j] = 1.;
        }
        return;
      }
      // If we have not filled the buffer completely due to fill
      // values, fill up with dummy values at the end:
      double last_lambda = wavel[j-1];
      for (size_t i = j; j != TEMPO_SPECTRAL_CHANNEL; ++i) {
        last_lambda += 1;
        wavel[i] = last_lambda;
        rad[i] = 0.;
        noise[i] = 1.;
      }
    }

    void get_orbit_date(int *year, int *month, int *day) {
      std::istringstream begin_date(ncfile.getAttText("begin_date"));
      // begn_date is formatted as "YYYY-MM-DD"
      char tmp; // to skip "-" chars
      begin_date >> *year >> tmp >> *month >> tmp >> *day;
    }

  protected:
    const string filename;
    NetCDFFile ncfile;
    NetCDFGroup band_group;
    const string band;
    const string product;

  public:
    const size_t mirror_step;
    const float radiance_fill;

  protected:
    const size_t nparams;
    const array3d wavecal_params;

  private:
    array3d read_wavecal_params() {
      array3d result(boost::extents[mirror_step][TEMPO_XTRACK][nparams]);
      const size_t start[] = {0, 0, 0};
      const size_t count[] = {mirror_step, TEMPO_XTRACK, nparams};
      band_group.getVar("wavecal_params", start, count, result.data());
      // multiply param 0 by 2 to match boost chebyshev conventions:
      for (size_t i=0; i != mirror_step; ++i) {
        for (size_t j=0; j != TEMPO_XTRACK; ++j) {
          result[i][j][0] *= 2.;
        }
      }
      return result;
    };
  };

  class rad_file : public orbit_file {
  public:
    rad_file(const string& filename, int band) : orbit_file(filename, band, "radiance"),
                                                 nominal_wavelength(read_nominal_wavelength()),
                                                 orbit_auxdata(read_auxdata()) { };

    inline virtual double get_lambda(size_t i_step, size_t i_xtrack, size_t i_spectral) final {
      auto nominal = reinterpret_cast<const float (*)[TEMPO_SPECTRAL_CHANNEL]>(nominal_wavelength.data());
      auto params = wavecal_params[i_step][i_xtrack].origin();
      auto x = (2.f * i_spectral) / (TEMPO_SPECTRAL_CHANNEL - 1) - 1.f;
      return nominal[i_xtrack][i_spectral] + chebyshev_clenshaw_recurrence(params, nparams, x);
    };

     void get_record_auxdata(RECORD_INFO *pRecord, int record) {
       pRecord->latitude= orbit_auxdata.lat[record-1];
       pRecord->longitude= orbit_auxdata.lon[record-1];
       pRecord->Zm= orbit_auxdata.sza[record-1];
       pRecord->Azimuth= orbit_auxdata.saa[record-1];
       pRecord->zenithViewAngle= orbit_auxdata.vza[record-1];
       pRecord->azimuthViewAngle= orbit_auxdata.vaa[record-1];

       pRecord->ground_pixel_QF = orbit_auxdata.pixel_quality[record-1];

       pRecord->tempo.red = orbit_auxdata.red[record-1];
       pRecord->tempo.green = orbit_auxdata.green[record-1];
       pRecord->tempo.blue = orbit_auxdata.blue[record-1];
       pRecord->tempo.cloud_mask = orbit_auxdata.cloud_mask[record-1];

       // ugly casting because we store the (num_records * 4) corner arrays as a flat array:
       auto lon_bounds = reinterpret_cast<const float(*)[4]>(orbit_auxdata.lon_bounds.data());
       auto lat_bounds = reinterpret_cast<const float(*)[4]>(orbit_auxdata.lat_bounds.data());
       for (int i=0; i!=4; ++i) {
         pRecord->satellite.cornerlons[i] = lon_bounds[record-1][i];
         pRecord->satellite.cornerlats[i] = lat_bounds[record-1][i];
       }
     };

  private:
    const std::array<float, TEMPO_XTRACK * TEMPO_SPECTRAL_CHANNEL> nominal_wavelength;
    const auxdata orbit_auxdata;

    std::array<float, TEMPO_XTRACK * TEMPO_SPECTRAL_CHANNEL> read_nominal_wavelength() {
      std::array<float, TEMPO_XTRACK * TEMPO_SPECTRAL_CHANNEL> result;
      const size_t start[] = {0, 0};
      const size_t count[] = {TEMPO_XTRACK, TEMPO_SPECTRAL_CHANNEL};
      band_group.getVar("nominal_wavelength", start, count, result.data());
      return result;
    };

    auxdata read_auxdata() {
      auxdata result;
      result.sza = band_group.getVar<float>("solar_zenith_angle");
      result.vza = band_group.getVar<float>("viewing_zenith_angle");
      result.saa = band_group.getVar<float>("solar_azimuth_angle");
      result.vaa = band_group.getVar<float>("viewing_azimuth_angle");
      result.lat = band_group.getVar<float>("latitude");
      result.lon = band_group.getVar<float>("longitude");
      result.lon_bounds = band_group.getVar<float>("longitude_bounds");
      result.lat_bounds = band_group.getVar<float>("latitude_bounds");

      result.pixel_quality = band_group.getVar<unsigned int>("ground_pixel_quality_flag");

      auto cloud_mask_group = ncfile.getGroup("cloud_mask_group");
      result.red = cloud_mask_group.getVar<float>("red");
      result.green = cloud_mask_group.getVar<float>("green");
      result.blue = cloud_mask_group.getVar<float>("blue");
      result.cloud_mask = cloud_mask_group.getVar<unsigned char>("cloud_mask");
      return result;
    };
  };

  class irr_file : public orbit_file {
  public:
    irr_file(const string& filename, int band) : orbit_file(filename, band, "irradiance") {};
    inline virtual double get_lambda(size_t i_step, size_t i_xtrack, size_t i_spectral) final {
      auto params = wavecal_params[i_step][i_xtrack].origin();
      auto x = (2.f * i_spectral) / (TEMPO_SPECTRAL_CHANNEL -1) - 1.f;
      return chebyshev_clenshaw_recurrence(params, nparams, x);
    };
  };

  std::unique_ptr<rad_file> current_orbit;

  map<string, vector<irradiance>> irradiance_references;

  vector<irradiance> load_irradiance_ref(const string& filename, int band) {
    irr_file irr(filename, band);
    vector<irradiance> result;

    for (size_t i=0; i != TEMPO_XTRACK; ++i) {
      irradiance row_irr;
      irr.get_radiance(0, i, row_irr.lambda.data(), row_irr.spec.data(), row_irr.sigma.data());
      result.push_back(std::move(row_irr));
    }
    return result;
  }
};

int TEMPO_set(ENGINE_CONTEXT *pEngineContext) {
  current_orbit = std::make_unique<rad_file>(pEngineContext->fileInfo.fileName,
                                             pEngineContext->project.instrumental.tempo.band);

  pEngineContext->n_alongtrack = current_orbit->mirror_step;
  pEngineContext->n_crosstrack = TEMPO_XTRACK;
  pEngineContext->recordNumber = pEngineContext->n_alongtrack * pEngineContext->n_crosstrack;

  return ERROR_ID_NO;
}

int TEMPO_read(ENGINE_CONTEXT *pEngineContext, int record) {
  int rc = ERROR_ID_NO;

  const size_t index_step = (record - 1) / TEMPO_XTRACK;
  const size_t index_xtrack = (record - 1) % TEMPO_XTRACK;

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;
  pRecord->i_alongtrack = index_step;
  pRecord->i_crosstrack = index_xtrack;

  if (!pEngineContext->project.instrumental.use_row[index_xtrack]) {
    return ERROR_ID_FILE_RECORD;
  }

  if (THRD_id == THREAD_TYPE_ANALYSIS) {
    // We need an irradiance spectrum for the irradiance plot.  We
    // give the one from the first analysis window.
    const auto& irrad_ref = irradiance_references.begin()->second.at(index_xtrack);
    for (size_t i=0; i<irrad_ref.lambda.size(); ++i) {
      pEngineContext->buffers.lambda_irrad[i] = irrad_ref.lambda[i];
      pEngineContext->buffers.irrad[i] = irrad_ref.spec[i];
    }
  }

  try {
    //    const auto& spec = current_orbit->radiance(index_time, index_row);
    auto lambda = pEngineContext->buffers.lambda;
    auto spectrum = pEngineContext->buffers.spectrum;
    auto sigma = pEngineContext->buffers.sigmaSpec;

    current_orbit->get_radiance(index_step, index_xtrack, lambda, spectrum, sigma);
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  current_orbit->get_record_auxdata(pRecord, record);

  return rc;
}

int TEMPO_init_irradiances(const mediate_analysis_window_t* analysis_windows, int num_windows, const ENGINE_CONTEXT* pEngineContext) {
  try {
    for (int i=0; i<num_windows; ++i) {
      const string filename = analysis_windows[i].refOneFile;
      // first check if current irradiance file is already loaded
      if (irradiance_references.find(filename) != irradiance_references.end()) {
        // already have loaded irradiances from this file from a previous analysis window
        continue;
      }
      irradiance_references[filename] = load_irradiance_ref(filename, pEngineContext->project.instrumental.tempo.band);
    }
  } catch(std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return ERROR_ID_NO;
}

int TEMPO_get_irradiance_reference(const char* file_name, int row, double *lambda, double *spectrum, double *sigma) {
  const auto& ref = irradiance_references.at(file_name).at(row);
  for (size_t i=0; i<ref.spec.size(); ++i) {
    lambda[i] = ref.lambda[i];
    spectrum[i] = ref.spec[i];
    sigma[i] = ref.sigma[i];
  }
  return ERROR_ID_NO;
}

void TEMPO_get_orbit_date(int *year, int *month, int *day) {
  current_orbit->get_orbit_date(year, month, day);
}

void TEMPO_cleanup() {
  current_orbit.reset();
  irradiance_references.clear();
}
