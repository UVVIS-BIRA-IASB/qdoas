/* Copyright (C) 2017 Royal Belgian Institute for Space Aeronomy
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
#include <array>
#include <map>
#include <set>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <assert.h>
#include <cassert>
#include <cmath>

#include "boost/multi_array.hpp"

#include "tropomi.h"
#include "tropomi_read.h"
#include "netcdfwrapper.h"
#include "dir_iter.h"
#include "date_util.h"

extern "C" {
#include "winthrd.h"
#include "comdefs.h"
#include "engine_context.h"
#include "mediate.h"
#include "analyse.h"
#include "spline.h"
#include "vector.h"
}

using std::string;
using std::vector;
using std::set;
using std::stringstream;
using std::map;

template<typename T>
using array2d = boost::const_multi_array_ref<T, 2>;

// create an array {"BAND1", "BAND2", ... } in same order as the
// tropomiSpectralBand enum:
const char* band_names[] = {
#define EXPAND(BAND, LABEL ) LABEL,
  TROPOMI_BANDS
#undef EXPAND
};

static const size_t MAX_GROUNDPIXEL = 450;

//static map<string,vector<vector<double>>> reference_matrix;
//static map<string,vector<vector<double>>> reference_wavelengths;

namespace {
  // The following struct types are for internal use only.

  static map<string,vector<vector<double>>> reference_radiance;
  static map<string,vector<vector<double>>> reference_wavelength;

  // irradiance reference
  struct refspec {
    refspec() : lambda(), irradiance(), sigma() {};
    vector<double> lambda;
    vector<double> irradiance;
    vector<double> sigma;
  };

  // earthshine spectrum to be used in earthshine reference:
  struct earth_ref {
    const vector<float>& wavelength;
    const vector<float>& spectrum;
    const vector<float>& error;
    // Because fill values could, in principle, change between different
    // orbit files, we store the fill values together with the spectra.
    const float fill_wavelength;
    const float fill_spectrum;
    const float fill_error;
    earth_ref(const vector<float>& w,
              const vector<float>& s,
              const vector<float>& e, float fw, float fs, float fe) :
      wavelength(w), spectrum(s), error(e),
      fill_wavelength(fw), fill_spectrum(fs), fill_error(fe) {};
  };

  struct geodata {
    vector<double> sza, vza, saa,  vaa,
      lon, lat,
      lon_bounds, lat_bounds,
      sat_lon, sat_lat, sat_alt;
  };
}

// ground_pixel_quality flag meanings
static const unsigned char SOLAR_ECLIPSE = 1,
  SUN_GLINT_POSSIBLE = 2,
  DESCENDING = 4,
  NIGHT = 8,
  GEO_BOUNDARY_CROSSING = 16,
  GEOLOCATION_ERROR = 32;
// spectral_channel_quality flag meanings
static const unsigned char MISSING = 1,
  BAD_PIXEL = 2,
  PROCESSING_ERROR = 4,
  SATURATED = 16,
  TRANSIENT = 32,
  RTS = 64;

// irradiance spectra for each row:
static vector<refspec> irradiance_reference;

// filenames of the orbits for which the current earthshine reference
// is valid (i.e. orbits of the same day)
static vector<string> reference_orbit_files;

static NetCDFFile current_file;
static string current_band;
static vector< vector<double> > nominal_wavelengths; // L1B radiance wavelength calibration
static double fill_nominal_wavelengths; // fill value for L1B radiance wavelength calibration

static geodata current_geodata;

static size_t size_spectral; // number of wavelengths per spectrum
static size_t size_scanline; // number of measurements (i.e. along track)
static size_t size_groundpixel; // number of detector rows (i.e. cross-track)

static time_t reference_time; // orbit start date
static vector<int> delta_time; // number of milliseconds after reference_time
static string current_filename="";


static geodata read_geodata(const NetCDFGroup& geo_group, size_t n_scanline, size_t n_groundpixel) {

  geodata result;

  // each element of this array contains the name of the netCDF
  // variable, the vector in which we want to store the data, and the
  // size of each element
  using std::ref;
  std::array<std::tuple<const string, vector<double>&, size_t>, 11> geovars {
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

int tropomi_set(ENGINE_CONTEXT *pEngineContext) {

  int rc = 0;
  try {
    current_file = NetCDFFile(pEngineContext->fileInfo.fileName);
    current_filename=pEngineContext->fileInfo.fileName;

    current_band = band_names[pEngineContext->project.instrumental.tropomi.spectralBand];

    reference_time = parse_utc_date(current_file.getAttText("time_reference"));

    NetCDFGroup obsGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

    size_scanline = obsGroup.dimLen("scanline");
    size_spectral = obsGroup.dimLen("spectral_channel");
    size_groundpixel = obsGroup.dimLen("ground_pixel");

    pEngineContext->recordNumber = size_groundpixel * size_scanline;
    pEngineContext->n_alongtrack= size_scanline;
    pEngineContext->n_crosstrack= size_groundpixel;

    size_t start[] = {0, 0};
    size_t count[] = {1, size_scanline};
    delta_time.resize(size_scanline);
    obsGroup.getVar("delta_time", start, count, delta_time.data());

    NetCDFGroup instrGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
    size_t start_wl[] = {0, 0, 0};
    size_t count_wl[] = {1, 1, size_spectral};
    nominal_wavelengths.resize(size_groundpixel);

    for(size_t i=0; i<size_groundpixel; ++i) {
      nominal_wavelengths[i].resize(size_spectral);
      start_wl[1] = i;
      instrGroup.getVar("nominal_wavelength", start_wl, count_wl, nominal_wavelengths[i].data());
    }
    fill_nominal_wavelengths = instrGroup.getFillValue<double>("nominal_wavelength");

    const auto geo_group = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/GEODATA");
    current_geodata = read_geodata(geo_group, size_scanline, size_groundpixel);

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
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


int tropomi_read(ENGINE_CONTEXT *pEngineContext,int record) {

  assert(record > 0); // record is the requested record number, starting from 1
  int rc = 0;

  NetCDFGroup obsGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

  const size_t indexScanline = (record - 1) / size_groundpixel;
  const size_t indexPixel = (record - 1) % size_groundpixel;
  size_t n_wavel = 0;

  if (!pEngineContext->project.instrumental.use_row[indexPixel]) {
    return ERROR_ID_FILE_RECORD;
  }

  if (THRD_id==THREAD_TYPE_ANALYSIS) {
     // in analysis mode, variables must have been initialized by tropomi_init()
    assert(irradiance_reference.size() == ANALYSE_swathSize);// || radiance_reference.size() == ANALYSE_swathSize);

    const refspec& ref = irradiance_reference.at(indexPixel);
    n_wavel = NDET[indexPixel] = ref.lambda.size(); // 20/04/2020 : it's better to set NDET to the size of the irradiance

    for (size_t i=0; i<ref.lambda.size(); ++i) {
      pEngineContext->buffers.lambda_irrad[i] = ref.lambda[i];
      pEngineContext->buffers.irrad[i] = ref.irradiance[i];
    }
  } else {
    n_wavel = size_spectral;
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
      double tempfill;
      if(obsGroup.hasVar("radiance_int16")  ){
          obsGroup.getVar("radiance_noise", start, count, rad_noise.data() );
          assert(obsGroup.hasVar("radiance_scaling"));
          obsGroup.getVar("radiance_int16", start, count, rad_int16.data() );
          obsGroup.getVar("radiance_scaling",start_scale,onecount,scale.data() ) ;
          const int fill_rad_int16 = obsGroup.getFillValue<int>("radiance_int16");
          tempfill = obsGroup.getFillValue<double>("radiance_scaling");
          for(int i = 0; i < size_spectral ;i++){
              rad[i] = scale[0]*rad_int16[i];
              if(rad_int16[i]==65535){
                  rad[i]=tempfill;
              }
          }
      }
      else{
          tempfill = obsGroup.getFillValue<double>("radiance");
          obsGroup.getVar("radiance_noise", start, count, rad_noise.data() );
          obsGroup.getVar("radiance", start, count, rad.data());
      }
      const double fill_rad=tempfill;

    const double fill_noise = obsGroup.getFillValue<double>("radiance_noise");
    const vector<double>& lambda = nominal_wavelengths.at(indexPixel);
    // copy non-fill values to buffers:
    size_t j=0;
    for (size_t i=0; i<rad.size() && j<n_wavel; ++i) {
      double li = lambda[i];
      double ri = rad[i];
      double ni = rad_noise[i];

      pEngineContext->buffers.lambda[i]=li;
      if (li != fill_nominal_wavelengths && ri != fill_rad && ni != fill_noise)
       {
        pEngineContext->buffers.spectrum[i]=ri;
        pEngineContext->buffers.sigmaSpec[i]=ri/(std::pow(10.0, ni/10.0));
        j++;
       }
      else
       pEngineContext->buffers.spectrum[i]=pEngineContext->buffers.sigmaSpec[i]=0.;

//       if (li != fill_nominal_wavelengths && ri != fill_rad && ni != fill_noise) {
//         pEngineContext->buffers.lambda[j]=li;
//         pEngineContext->buffers.spectrum[j]=ri;
//         pEngineContext->buffers.sigmaSpec[j]=ri/(std::pow(10.0, ni/10.0));
//         ++j;
//       }
    }

    if (j == 0) {
      // All fill values, can't use this spectrum:
      return ERROR_ID_FILE_RECORD;
    }
    else if (THRD_id!=THREAD_TYPE_ANALYSIS)
     NDET[indexPixel] = size_spectral;

    // check if the earthshine spectrum is shorter than the reference
    // spectrum (e.g.due to different number of fill values).
    // if (j<n_wavel) {
      // This is not a very clean solution, but we assume that
      // reducing NDET[i] is always safe:

     // NDET[indexPixel] = j; -> 20/04/2020 : this causes a big issue in ANALYSE_Spectrum
     //                             for example, memcpy(Sref,Feno->Sref,sizeof(double)*n_wavel);
     //                          in fact, we should keep the original NDET[indexPixel] and manage the
     //                          different size for spectrum as anyway it will be interpolated to the reference grid
    // }

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;

  pRecord->i_alongtrack = indexScanline;
  pRecord->i_crosstrack = indexPixel;

  pRecord->useErrors = 1;
  get_geodata(pRecord, current_geodata, record);

  get_utc_date(reference_time, delta_time[indexScanline], &pRecord->present_datetime);

  return rc;
}

vector<refspec> loadReference(const string& filename, const string& band) {
  NetCDFFile refFile(filename);
  auto irrObsGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/OBSERVATIONS");
  auto instrGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/INSTRUMENT");
  // get dimensions:
  size_t size_pixel = irrObsGroup.dimLen("pixel");
  size_t nlambda = irrObsGroup.dimLen("spectral_channel");

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

    vector<double> lambda(nlambda);
    vector<double> irra(nlambda);
    vector<double> noise(nlambda);

    instrGroup.getVar("calibrated_wavelength", start_lambda, count_lambda, lambda.data());
    irrObsGroup.getVar("irradiance", start_irr, count_irr, irra.data());
    irrObsGroup.getVar("irradiance_noise", start_irr, count_irr, noise.data());

    const double fill_lambda = instrGroup.getFillValue<double>("calibrated_wavelength");
    const double fill_irra = irrObsGroup.getFillValue<double>("irradiance");
    const double fill_noise = irrObsGroup.getFillValue<double>("irradiance_noise");

    for (size_t j=0; j<nlambda; ++j) {
      double lj = lambda[j];
      double ij = irra[j];
      double nj = noise[j];
      if (lj != fill_lambda && ij != fill_irra && nj != fill_noise) {
        ref_pixel.lambda.push_back(lj);
        ref_pixel.irradiance.push_back(ij);
        ref_pixel.sigma.push_back(ij/(std::pow(10.0, nj/10.0)));
      }
    }
  }

  return result;
}

static vector<vector<double>> loadRadAsRef(const NetCDFFile& filename, const char *variable) {
  // get dimensions:
  const size_t size_pixel = filename.dimLen("col_dim");
  const size_t nlambda = filename.dimLen("spectral_dim");

  vector<vector<double>> result(size_pixel);
  for(size_t i=0; i<size_pixel; ++i) {
    vector<double>& ref_pixel = result[i];
    ref_pixel.resize(nlambda);

    // the matrix has dimensions
    // (pixel, spectral_channel)
    size_t start[] = {i, 0};
    size_t count[] = {1, nlambda};

    filename.getVar(variable, start, count, ref_pixel.data());

  }

  return result;
}

int tropomi_init(const char *ref_filename, const ENGINE_CONTEXT *pEngineContext,int* n_wavel_temp) {

  NetCDFFile refFile(ref_filename);
  RC rc = ERROR_ID_NO;
  try {
    if (!pEngineContext->radAsRefFlag){
      irradiance_reference = loadReference(ref_filename,
                                      band_names[pEngineContext->project.instrumental.tropomi.spectralBand]);
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  if (pEngineContext->radAsRefFlag){
     ANALYSE_swathSize = refFile.dimLen("col_dim");
  } else {
     ANALYSE_swathSize = irradiance_reference.size();
  }

  for(int i=0; i<ANALYSE_swathSize; ++i) {
    if (pEngineContext->radAsRefFlag){
       *n_wavel_temp = refFile.dimLen("spectral_dim");
    } else {
       NDET[i] = irradiance_reference[i].lambda.size();
       *n_wavel_temp = NDET[i];
    }
  }

  return rc;
}

// simple method to get the basename of a full file name:
static string basename(const string& filename) {
  auto pathsep = filename.find_last_of('/');
  if (pathsep != string::npos)
    return filename.substr(1+pathsep);
  else
    return filename;
}

// Find all radiance files for the current band from the same day.
static int get_reference_orbits(const bool *use_row,const std::string& input_file, enum tropomiSpectralBand band, const std::string& reference_dir = "") {
  int nfiles=0;
  reference_orbit_files.clear();

  dir_iter reference_orbit_iter;
  if (!reference_dir.empty()) {
    reference_orbit_iter = dir_iter(reference_dir);  // TO FIX : should be reference_dir/YYYY/MM/DD
  } else {
    // Find other radiance files for this day.  This code is specific
    // for the way Tropomi files are stored at BIRA:

    // L1B products are stored in a directory for each day, in a
    // structure <daily_dir>/<orbit_dir>/L1B/L1B-RADIANCE/S5P...*.nc
    // Therefore, we move three levels up from the current file
    // ("/../../.."), and recursively search for files up to 3 levels
    // deep.  We assume the directories we traverse in this way will
    // contain no unwanted files matching our search pattern.

    auto pathsep = input_file.find_last_of('/');
    const string daily_dir = ((pathsep != string::npos) ? input_file.substr(0, pathsep) : string(".")); // !!! + "/../../..";
    // recursively search daily_dir, three levels deep
    reference_orbit_iter = dir_iter(daily_dir, 1);   // !!! 3
   }

  int first_row=ITEM_NONE;

  for (int row=0; row <ANALYSE_swathSize; ++row) {
   if (!use_row[row]) continue;
   first_row=row;
   break;
  }

  if (first_row==ITEM_NONE)
   return 0;

  // We want filenames matching "S5P*L1B_RA_BD<n>*.nc")
  string product("L1B_RA_BD");
  product.push_back('1' + band); // "BD1", "BD2", ... depending on spectralBand
  // look at files in reference_orbit_iter, store results in l1files,
  // if the file is a L1B radiance file for this band, with correct
  // dimensions
  for (auto f : reference_orbit_iter) {
    if (!basename(f).compare(0,3,"S5P")
        && f.find(product) != string::npos
        && !f.compare(f.length() -3, 3, ".nc")) {

      int use_file=0;
      // !!! std::cout << "try : " << basename(f) << std::endl;

      for (int win=0; win!=NFeno; ++win)
       {
        const FENO *pTabFeno = &TabFeno[first_row][win];  // config is the same whatever the row

        if (!pTabFeno->hidden && !use_file)
         {
          float t1,t2,tmp,orbit_t1,orbit_t2;

          t1=13.5-pTabFeno->refLonMin/15.-1.;
          t2=13.5-pTabFeno->refLonMax/15.+1.;

          if (t1>t2)
           {
            tmp=t1;
            t1=t2;
            t2=tmp;
           }

          if (t1<0.)
           t1+=24.;
          if (t2<0.)
           t2+=24.;
          if (t1>24.)
           t1-=24.;
          if (t2>24.)
           t2-=24.;

          stringstream ss(basename(f));   // convert file name to stringstream
          vector <string> tokens;         // vector of string to save tokens
          string tt;

          while(getline(ss,tt,'_'))
            tokens.push_back(tt);

          orbit_t1=std::stof(tokens[5].substr(9,2))+std::stof(tokens[5].substr(11,2))/60.+std::stof(tokens[5].substr(13,2))/3600.-1.;
          orbit_t2=std::stof(tokens[6].substr(9,2))+std::stof(tokens[6].substr(11,2))/60.+std::stof(tokens[6].substr(13,2))/60.+1.;

          if (orbit_t1<0.)
           orbit_t1=0.;
          if (orbit_t2<0.)
           orbit_t2=0.;
          if (orbit_t1>24.)
           orbit_t1=24.;
          if (orbit_t2>24.)
           orbit_t2=24.;

          if (((t1<=t2) && (((orbit_t1>=t1) && (orbit_t1<=t2)) || ((orbit_t2>=t1) && (orbit_t2<=t2)))) ||
              ((t1>t2) && ((orbit_t1>=t1) || (orbit_t2>=t1) || (orbit_t1<=t2) || (orbit_t2<=t2))))
           {
            // !!! std::cout << __func__ << " " << basename(f) << std::endl;
            reference_orbit_files.push_back(f);

            // !!! std::cout << " OK " << basename(f) << " t1-t2 " << t1 << " - " << t2 << " orbit1-orbit2 " << orbit_t1 << " - " << orbit_t2 << std::endl;
            use_file=1;
            nfiles++;
           }
         }

       }
    }
  }

  // !!! std::cout << "Number of files : " << nfiles << std::endl;
  return nfiles;
}
//
// check if an earthshine observation matches selection criteria to
// include it in the earthshine reference.
static bool use_as_reference(double lon, double lat, double sza, const FENO *pTabFeno) {
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

  return((lon_min <= lon && lon_max >= lon) || !use_lon)
    && ((lat_min <= lat && lat_max >= lat) || !use_lat)
    && ((sza_min <= sza && sza_max >= sza) || !use_sza);
}

// Get indices of the narrowest the interval in 'row_wavelengths'
// containing the wavelength range of the analysis window defined in
// pTabFeno.
std::pair<size_t,size_t> get_window_limits(const FENO *pTabFeno, const vector<float>& row_wavelengths, size_t row) {
  // Get outer limits of the analysis window:
  const double lambda_min = pTabFeno->fit_properties.LFenetre[0][0];
  const double lambda_max = pTabFeno->fit_properties.LFenetre[pTabFeno->fit_properties.Z-1][1];
  // Get indices of start and end of the interval (lambda_min, lambda_max) in nominal_wavelengths
  auto i_before = std::lower_bound(row_wavelengths.begin(), row_wavelengths.end(), lambda_min);
  auto i_after = std::upper_bound(i_before, row_wavelengths.end(), lambda_max);
  if (i_before == row_wavelengths.begin() || i_after == row_wavelengths.end()) {
    std::stringstream ss;
    ss << "Analysis window boundaries for window " << pTabFeno->windowName
       << " outside of nominal_wavelength range of row " << row << ": ("
       << lambda_min << ", " << lambda_max << ") <-> ("
       << row_wavelengths.at(0) << ", " << row_wavelengths.back() << ")" ;
    throw std::out_of_range(ss.str());
  }
  return std::pair<size_t, size_t>(std::distance(row_wavelengths.begin(), i_before),
                                   std::distance(row_wavelengths.begin(), i_after));
}

// for each of the 450 possible detector rows, and for each analysis
// window, gather a list of spectra matching the search criteria.
//
// \param pEngineContext ENGINE_CONTEXT containing criteria for the reference.
// \param orbit_files List of file names in which to look.
// \param cache A set that will hold all vectors of wavelengths, spectra and errors, for memory management.
//
// One complication is that each analysis window may have its own
// criteria for reference selection (even though this option is rarely
// used).  Therefore, we need to keep a separate list of reference
// spectra for each analysis window, but we do not want to keep
// multiple copies of same spectrum in memory if it is used for
// multiple analysis windows.  Therefore, we store every spectrum (as
// well as wavelength grids and estimated errors) once in a 'cache',
// and keep a list of references to the data in this cache for every
// analysis window.  We use std::set to store the cache, because
// references to elements of a std::set stay valid as the set grows.
static vector<std::array<vector<earth_ref>, MAX_GROUNDPIXEL>> find_matching_spectra(ENGINE_CONTEXT *pEngineContext,const int nfiles,
                                                                             set<vector<float>>& cache) {
  assert(size_groundpixel <= MAX_GROUNDPIXEL);
  vector<std::array<vector<earth_ref>, MAX_GROUNDPIXEL>> result(NFeno);

  for (const string & fname : reference_orbit_files) {

    NetCDFFile orbit(fname);
    string bname=basename(fname);

    // !!! old loop from Thomas; the problem is that it is difficult to come back to the file name
    // for (const auto & orbit : orbit_files) {

    // 1. read "nominal wavelength" for each row:
    std::array<const vector<float>*, MAX_GROUNDPIXEL> nominal_wavelengths { nullptr };
    NetCDFGroup instrGroup(orbit.getGroup(current_band + "_RADIANCE/STANDARD_MODE/INSTRUMENT"));
    NetCDFGroup obsGroup(orbit.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS"));

    const size_t orbit_spectral = obsGroup.dimLen("spectral_channel");
    const size_t orbit_groundpixel = obsGroup.dimLen("ground_pixel");
    const size_t orbit_scanline = obsGroup.dimLen("scanline");

    if (orbit_groundpixel != size_groundpixel || orbit_spectral != size_spectral) continue;

    for(size_t i=0; i<size_groundpixel; ++i) {
      vector<float> wl(size_spectral);
      const size_t start_wl[] = {0, i, 0};
      const size_t count_wl[] = {1, 1, size_spectral};
      instrGroup.getVar("nominal_wavelength", start_wl, count_wl, wl.data());
      const auto& wavelengths = *cache.insert(wl).first;
      nominal_wavelengths[i] = &wavelengths;
    }

    const auto orbit_fill_wavelengths = instrGroup.getFillValue<float>("nominal_wavelength");
    string radstr="radiance";
        if(obsGroup.hasVar("radiance_scaling")){
            radstr="radiance_scaling";
        }
    const auto orbit_fill_spectra = obsGroup.getFillValue<float>(radstr);
    const auto orbit_fill_errors = obsGroup.getFillValue<float>("radiance_noise");

    // 2. read geolocation data and flags required to evaluate which
    // spectra should be used for the reference.
    const size_t num_obs = orbit_scanline*size_groundpixel;
    vector<float> lons(num_obs), lats(num_obs), szas(num_obs);
    vector<unsigned char> ground_pixel_quality(num_obs);
    // start and  count arguments for variables with observation dimensions (i.e. scanline x row):
    const size_t start_obs[] = {0, 0, 0};
    const size_t count_obs[] = {1, orbit_scanline, size_groundpixel };

    NetCDFGroup geo_group(orbit.getGroup(current_band + "_RADIANCE/STANDARD_MODE/GEODATA"));
    geo_group.getVar("latitude", start_obs, count_obs, lats.data());
    geo_group.getVar("longitude", start_obs, count_obs, lons.data());
    geo_group.getVar("solar_zenith_angle", start_obs, count_obs, szas.data());
    obsGroup.getVar("ground_pixel_quality", start_obs, count_obs, ground_pixel_quality.data());


    auto extent = boost::extents[orbit_scanline][size_groundpixel];
    array2d<float> latitudes(lats.data(), extent);
    array2d<float> longitudes(lons.data(), extent);
    array2d<float> szangles(szas.data(), extent);
    array2d<unsigned char> groundpixelqualityflags(ground_pixel_quality.data(), extent);
    const static unsigned char groundpixel_quality_mask = SOLAR_ECLIPSE | DESCENDING | NIGHT
      | GEO_BOUNDARY_CROSSING | GEOLOCATION_ERROR; // filter everything except SUN_GLINT_POSSIBLE

    // Get indices of nominal_wavelengths corresponding to analysis window limits:
    vector<std::array<std::pair<size_t,size_t>, MAX_GROUNDPIXEL>> window_limits(NFeno);
    for (size_t row=0; row != size_groundpixel; ++row) {
      for (int win=0; win!=NFeno; ++win) {
        const FENO *pTabFeno = &TabFeno[row][win];
        if (pTabFeno->hidden || (!pEngineContext->project.instrumental.use_row[row])) continue; // skip Kurucz calibration windows
        window_limits.at(win).at(row) = get_window_limits(pTabFeno, *nominal_wavelengths.at(row), row);
      }
    }

    int found=0;

    // 3. read radiance & error for matching spectra
    vector<unsigned char> spec_quality(size_spectral); // Temporary buffer for spectral_channel_quality flags
    for (size_t scan=0; scan != orbit_scanline; ++scan) {

      for (size_t row=0; row != size_groundpixel; ++row) {

        if (!pEngineContext->project.instrumental.use_row[row]) continue;

        // start and count indices to read along spectral dimension
        // for one observation:
        const size_t start[] = {0, scan, row, 0};
        const size_t count[] = {1, 1, 1, size_spectral };
        const size_t start_scale[] = {0,scan,row};
        const size_t onecount[] = {1, 1, 1};

        // TODO: check use_row[row]?
        auto lat=latitudes[scan][row];
        auto lon=longitudes[scan][row];
        if (lon <= 0.0)
          lon += 360.0;
        auto sza=szangles[scan][row];

        // Because we keep a separate list of reference spectra for
        // each analysis window, it is possible that a spectrum is
        // used as a reference for mutliple analysis windows.
        //
        // To keep track of this, i_spec is initialized to
        // cache.end().  Once the spectrum and errors for this
        // scanline and row have been read and stored in the cache,
        // i_spec will point tot that element of the cache instead.
        auto i_spec = cache.end();
        auto i_err = cache.end();

        // We want to read spectral_channel_quality flags just once,
        // for each spectrum that satisfies all other criteria
        // (reading these flags for all spectra is *much* too slow).
        // We use a bool to keep track of that:
        bool read_quality_flags = true;

        for (int win=0; win!=NFeno; ++win) {
          const FENO *pTabFeno = &TabFeno[row][win];
          if (pTabFeno->hidden) continue;

          if(pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
             && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
             && !(groundpixelqualityflags[scan][row] & groundpixel_quality_mask) // Skip spectra which are flagged in groundpixelqualityflags
             && use_as_reference(lon,lat,sza,pTabFeno)) {

            if (read_quality_flags) {
              obsGroup.getVar("spectral_channel_quality", start, count, spec_quality.data() );
              read_quality_flags = false;
            }

            // For spectra matching all criteria, check if wavelengths inside our analysis window are flagged:
            auto window_start = window_limits.at(win).at(row).first;
            auto window_end = window_limits.at(win).at(row).second;
            // Check if all spectral channel flags inside the wavelength interval for this analysis window are zero:
            if (std::all_of(spec_quality.begin() + window_start, spec_quality.begin() + window_end,
                            [] (unsigned char flag) { return flag == 0; })) {

              if (i_spec == cache.end()) {
                // spectrum was not yet read, so do that now:
                  vector<float> spec(size_spectral), err(size_spectral);
                  vector<unsigned short int> rad_int16(size_spectral);
                  vector<double> scale(1);
                  double tempfill=-999999;
                  if(obsGroup.hasVar("radiance_int16")) {
                      obsGroup.getVar("radiance_noise", start, count, err.data() );
                      assert(obsGroup.hasVar("radiance_scaling"));
                      obsGroup.getVar("radiance_int16", start, count, rad_int16.data() );
                      obsGroup.getVar("radiance_scaling",start_scale,onecount,scale.data() ) ;
                      const int fill_rad_int16 = obsGroup.getFillValue<int>("radiance_int16");
                      tempfill = obsGroup.getFillValue<double>("radiance_scaling");
                      for(int i = 0; i < size_spectral ;i++){
                          spec[i] = scale[0]*rad_int16[i];
                          if(rad_int16[i]==65535){
                              spec[i]=tempfill;
                          }
                      }
                  }
                  else{
                      obsGroup.getVar("radiance", start, count, spec.data() );
                      obsGroup.getVar("radiance_noise", start, count, err.data() );
                  }

                  //                 std::cout << "; scan " << scan << " row " << row << " lat " << lat << " long " << lon << " sza " << sza << std::endl;
//                 for (auto i = spec.begin(); i != spec.end(); ++i)
//                    std::cout << *i << ' ';
//                 std::cout << std::endl;

                i_spec = cache.insert(std::move(spec)).first;
                i_err = cache.insert(std::move(err)).first;
              }


              // std::cout << "assert" << std::endl;
              // At this point, i_spec and i_err must point to valid elements of our cache.


              assert(i_spec != cache.end() && i_err != cache.end());
              result[win][row].push_back(earth_ref( *(nominal_wavelengths[row]), *i_spec, *i_err,
                                                  orbit_fill_wavelengths, orbit_fill_spectra, orbit_fill_errors));
              found=1;
            }

          }
        }
      }
    }

    orbit.close();
  }

  return result;
}

// Interpolate reference spectra (and errors) onto a common wavelength grid and sum them.
static void sum_refs(vector<double>& sum, vector<double>& variance, const vector<earth_ref>&  spectra, const vector<double>& wavelength_grid) {
  // Initialize results to 0.
  std::fill(sum.begin(), sum.end(), 0.);
  std::fill(variance.begin(), variance.end(), 0.);

  // We need temporary buffers to copy the reference spectra,
  // excluding fill values, and t9o hold the interpolated spectra,
  // before we ad them to the sum.
  vector<double> tempspec, templambda, temperr, derivs;
  vector<double> interpspec(wavelength_grid.size()), interperr(wavelength_grid.size());
  for (const auto &ref : spectra) {
    tempspec.clear(); templambda.clear(); temperr.clear();

    // copy non-fillvalued data to the temporary buffers
    for (size_t i=0; i!=ref.wavelength.size(); ++i) {
      const auto li=ref.wavelength[i];
      const auto si=ref.spectrum[i];
      const auto ei=ref.error[i];
      if (li != ref.fill_wavelength && si != ref.fill_spectrum && ei != ref.fill_error) {
        templambda.push_back(li);
        tempspec.push_back(si);
        temperr.push_back(si/(std::pow(10.0, ei/10.0)));
      }
    }

    // interpolate
    derivs.resize(templambda.size());
    int rc = SPLINE_Deriv2(templambda.data(), tempspec.data(), derivs.data(), derivs.size(), __func__);
    if (rc) throw(std::runtime_error("Error interpolating earthshine spectrum for reference onto common wavelength grid."));
    SPLINE_Vector(templambda.data(), tempspec.data(), derivs.data(), templambda.size(),
                  wavelength_grid.data(), interpspec.data(), wavelength_grid.size(), SPLINE_CUBIC);
    // linear interpolation for errors.
    SPLINE_Vector(templambda.data(), temperr.data(), NULL, templambda.size(),
                  wavelength_grid.data(), interperr.data(), wavelength_grid.size(), SPLINE_LINEAR);
    for (size_t i=0; i!= sum.size(); ++i) {
      sum[i] += interpspec[i];
      variance[i] += interperr[i] * interperr[i];
    }
  }
}

int tropomi_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle) {

  // A radiance reference for Tropomi is created, either
  //
  // - from the L1B radiance files found in the "reference orbit
  //   directory" configured in the "instrumental" tab of projet
  //   properties, or
  //
  // - if no directory is configured, from L1B radiance files found
  //   for the same day as the file we are currently processing (based
  //   on an assumed standard directory layout).

  // First, check if we have already created the automatic reference
  // (possible when processing multiple input files in one go).

  if (// If we are using a manual reference directory, the reference
      // is the same for all input files, and we can just check if a
      // reference has been created (-> check if reference_orbit_files
      // is not empty)
    (strlen(pEngineContext->project.instrumental.tropomi.reference_orbit_dir)
     && !reference_orbit_files.empty()) ||
    (// If we are using daily references, check if the orbit we are
     // processing is in the list of orbits for which the current
     // earthshine ref is valid.
     std::find(reference_orbit_files.begin(), reference_orbit_files.end(),
               pEngineContext->fileInfo.fileName) != reference_orbit_files.end())) {

    // In these cases, we don't have to do anything
    return ERROR_ID_NO;
  }

  // If we reach this point, we must create a new reference spectrum
  try {
    set<vector<float>> cache;
    auto earth_spectra = find_matching_spectra(pEngineContext,get_reference_orbits(pEngineContext->project.instrumental.use_row,
                                                                                   pEngineContext->fileInfo.fileName,
                                                                                   pEngineContext->project.instrumental.tropomi.spectralBand,
                                                                                   pEngineContext->project.instrumental.tropomi.reference_orbit_dir), cache);

    vector<double> wavelength_grid, sum, variance;
    for (size_t row = 0; row!=size_groundpixel; ++row) {
      const int n_wavel=NDET[row];
      wavelength_grid.resize(n_wavel);
      sum.resize(wavelength_grid.size());
      variance.resize(wavelength_grid.size());

      for(int window=0; window < NFeno; ++window) {
        FENO *pTabFeno = &TabFeno[row][window];
        if (!pTabFeno->useRefRow) continue;
        if (pTabFeno->hidden || (pTabFeno->refSpectrumSelectionMode!=ANLYS_REF_SELECTION_MODE_AUTOMATIC)) continue;
        const vector<earth_ref>& refs = earth_spectra[window][row];
//        if (!(rc=tropomi_get_reference(pTabFeno->ref2,row,pTabFeno->LambdaRef,pTabFeno->Sref,pTabFeno->SrefSigma,&n_wavel,0))){
           if (refs.size()) {
             for (size_t i=0; i!=wavelength_grid.size(); ++i) {
               wavelength_grid[i] = pTabFeno->LambdaRef[i];
             }
             sum_refs(sum, variance, refs, wavelength_grid);
             for (size_t i=0; i!=sum.size(); ++i) {
               pTabFeno->Sref[i]=sum[i]/refs.size();
               pTabFeno->SrefSigma[i]=std::sqrt(variance[i])/refs.size();
             }

             VECTOR_NormalizeVector(pTabFeno->Sref-1,n_wavel,&pTabFeno->refNormFact, __func__);
           }
//        }
      }
    }

    // Create reference description string for output, and emit
    // warning message for those rows/analysis windows for which no
    // reference could be created:

    int firstRow=ITEM_NONE;
    for(int window=0; window!= NFeno; ++ window) {
      vector<size_t> failed_rows; // per analysis window, rows for which no reference was found
      for(size_t row=0; row!=size_groundpixel; ++row) {
        FENO *pTabFeno = &TabFeno[row][window];
        if (!pTabFeno->useRefRow) continue;

        // Don't write error messages when automatic reference is not used:
        if (pTabFeno->hidden || !pEngineContext->project.instrumental.use_row[row] || (pTabFeno->refSpectrumSelectionMode != ANLYS_REF_SELECTION_MODE_AUTOMATIC)) continue;

        if (firstRow==ITEM_NONE)
         firstRow=row;

        const vector<earth_ref>& refs = earth_spectra[window][row];
        std::stringstream desc;
        desc << refs.size() << " radiances used";
        free(pTabFeno->ref_description);
        pTabFeno->ref_description = strdup(desc.str().c_str());
        if (!refs.size())
          failed_rows.push_back(row);
      }
      if (!failed_rows.empty() && (firstRow!=ITEM_NONE)) {
        // For each analysis window, emit one warning message with the rows for which no reference was found.
        std::stringstream ss;
        ss << "Analysis window " << TabFeno[firstRow][window].windowName << ": cannot find radiance reference spectra for rows ";
        for (auto ir=failed_rows.begin(); ir!=failed_rows.end(); ++ir) {
          ss << *ir;
          if (ir != failed_rows.end() -1)
            ss << ", ";
        }
        mediateResponseErrorMessage(__func__, ss.str().c_str(), WarningEngineError, responseHandle);
      }
    }

  } catch (std::runtime_error e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_TROPOMI_REF, pEngineContext->fileInfo.fileName, 0, e.what()); // TODO: more specific error message
  }
  for (size_t row = 0; row!= size_groundpixel; ++row) {
    if (!pEngineContext->project.instrumental.use_row[row]) continue;
    int rc = ANALYSE_AlignReference(pEngineContext,2,responseHandle,row);
    if (rc) return rc;
  }

  return ERROR_ID_NO;
}

int tropomi_get_reference(const char *filename, int pixel, double *lambda, double *spectrum, double *sigma, int n_wavel, const int radAsRef) {
  int rc = ERROR_ID_NO;

  auto& radiance_reference = reference_radiance[filename];
  auto& wavelength_reference = reference_wavelength[filename];

  try {
    if (radAsRef){
       NetCDFFile refFile(filename);
       radiance_reference = loadRadAsRef(refFile,"reference_radiance");
       wavelength_reference = loadRadAsRef(refFile,"reference_wavelength");
       if (!radiance_reference.size() || !wavelength_reference.size())
         return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REF_DATA, pixel);

//       *n_wavel = radiance_reference.size();

       for (size_t i = 0; i < n_wavel; ++i) {
         lambda[i] = wavelength_reference.at(pixel)[i];
         spectrum[i] = radiance_reference.at(pixel)[i];
       }
    } else {
       const refspec& r = irradiance_reference.at(pixel);
       if (!r.irradiance.size() )
         return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REF_DATA, pixel);

//       *n_wavel = r.irradiance.size();

       for (size_t i = 0; i < n_wavel; ++i) {
         lambda[i] = r.lambda[i];
         spectrum[i] = r.irradiance[i];
         sigma[i] = r.sigma[i];
       }
    }

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_TROPOMI_REF, filename, pixel, e.what());
  }

  return rc;
}

int tropomi_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day) {
  if (current_filename!="")
   {
    std::istringstream orbit_start(current_file.getAttText("time_coverage_start"));
    // time_coverage_start is formatted as "YYYY-MM-DD"
    char tmp; // to skip "-" chars
    orbit_start >> *orbit_year >> tmp >> *orbit_month >> tmp >> *orbit_day;
    return  orbit_start.good() ? 0 : 1;
   }
  else
   return(0);
}

void tropomi_cleanup(void) {
  current_file.close();
  current_filename="";

  current_geodata = geodata();
  current_band = "";

  size_spectral = size_scanline = size_groundpixel = 0;
  reference_time = 0;

  nominal_wavelengths.clear();
  delta_time.clear();
  irradiance_reference.clear();
  reference_orbit_files.clear();
}
