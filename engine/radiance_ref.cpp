#include <string>
#include <vector>
#include <map>

#include "netcdfwrapper.h"

extern "C" {
  #include "radiance_ref.h"
  #include "comdefs.h"
}

using std::string;
using std::vector;
using std::map;

namespace {
  // The following struct types are for internal use only.

  // a map of radiance reference spectra for each rad as ref filename
  // (in case different analysis windows use different radiance references)
  typedef map<string,vector<vector<double>>> ReferenceMap;

  static ReferenceMap reference_radiance;
  static ReferenceMap reference_wavelength;
};

static vector<vector<double>> loadRadAsRef(const NetCDFFile& reffile, const char *variable) {
  // get dimensions:
  const size_t size_pixel = reffile.dimLen("col_dim");
  const size_t nlambda = reffile.dimLen("spectral_dim");

  // we only need reference data for rows where use_row = 1:
  vector<int> use_row(size_pixel);
  const size_t start[] = {0};
  const size_t count[] = {size_pixel};
  reffile.getVar("use_row", start, count, use_row.data());

  vector<vector<double>> result(size_pixel);
  for(size_t i=0; i<size_pixel; ++i) {
    if (use_row[i] == 0) {
      continue; // skip rows for which no valid reference is available.
    }
    vector<double>& ref_pixel = result[i];
    ref_pixel.resize(nlambda);

    // the matrix has dimensions
    // (pixel, spectral_channel)
    size_t start[] = {i, 0};
    size_t count[] = {1, nlambda};

    reffile.getVar(variable, start, count, ref_pixel.data());

  }

  return result;
}

int radiance_ref_load(const char *filename, int pixel, double *lambda, double *spectrum, int n_wavel, int *use_row) {
  int rc = ERROR_ID_NO;

  auto radiance = reference_radiance.find(filename);
  auto wavelength = reference_wavelength.find(filename);

  try{
    if (radiance == reference_radiance.end()) {
      // reference radiances for this filename not yet loaded
      NetCDFFile refFile(filename);
      radiance = reference_radiance.insert(radiance, ReferenceMap::value_type(filename, loadRadAsRef(refFile,"reference_radiance")));
      wavelength = reference_wavelength.insert(wavelength, ReferenceMap::value_type(filename, loadRadAsRef(refFile,"reference_wavelength")));
    }
    if (radiance->second.at(pixel).empty()) { // iff use_row[pixel] == 0
      *use_row = 0;
      return rc;
    }
    for (size_t i = 0; i < n_wavel; ++i) {
      lambda[i] = wavelength->second.at(pixel)[i];
      spectrum[i] = radiance->second.at(pixel)[i];
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_TROPOMI_REF, filename, pixel, e.what());
  }

  return rc;
}

void radiance_ref_clear_cache(void) {
  reference_radiance.clear();
  reference_wavelength.clear();
}
