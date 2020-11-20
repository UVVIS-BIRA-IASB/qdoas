#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <stdexcept>

#include <cmath>
#include <cstring>

#include <H5Cpp.h>

#include "omps_read.h"
#include "dir_iter.h"

extern "C" {
#include "analyse.h"
#include "kurucz.h"
#include "mediate.h"
#include "engine_context.h"
#include "spectral_range.h"
#include "fit_properties.h"
#include "spline.h"
#include "vector.h"
#include "winthrd.h"
}

using H5::H5File; using H5::DataSet; using H5::DataSpace; using H5::Group;

using std::cerr; using std::endl;
using std::vector;
using std::set;
using std::string;

namespace {

  struct OMPSGeo {
    vector<char> utcDate;
    vector<float> longitude;
    vector<float> latitude;
    vector<int> groundPixelQualityFlags;
    vector<float> satelliteAzimuth;
    vector<float> satelliteZenithAngle;
    vector<float> solarAzimuth;
    vector<float> solarZenithAngle;
    vector<double> spacecraftAltitude;
    vector<double> spacecraftLatitude;
    vector<double> spacecraftLongitude;
  };

  struct OMPSOrbit {
    unsigned int nXTrack;
    unsigned int nMeasurements;
    unsigned int nLambda;
    vector<double> irradiances;
    vector<double> wavelengths;
    string fileName;
    H5File file;
  };

  OMPSOrbit currentOrbit;
  OMPSGeo geoData;

  template<typename T>
  H5::DataType getDataType(void);

  set<string> referenceFileNames; // set of files for which the current automatic reference spectrum is valid; this is also the set of files from which the spectrum was generated

  template<>
  inline H5::DataType getDataType<float>() {
    return H5::PredType::NATIVE_FLOAT;
  }

  template<>
  inline H5::DataType getDataType<double>() {
    return H5::PredType::NATIVE_DOUBLE;
  }

  template<>
  inline H5::DataType getDataType<char>() {
    return H5::PredType::C_S1;
  }

  template<>
  inline H5::DataType getDataType<int>() {
    return H5::PredType::NATIVE_INT;
  }

  template<typename T>
  void readData(H5File& file, vector<T>& buffer, const char *dataSetName) {
    DataSet d = file.openDataSet(dataSetName);
    DataSpace s = d.getSpace();

    hsize_t nPoints = s.getSimpleExtentNpoints();
    hsize_t len = 1;

    H5::DataType outType;
    if(d.getDataType().getClass() == H5::PredType::C_S1.getClass() ) {
      len = 1 + d.getDataType().getSize(); // +1 for null terminator...
      outType = H5::StrType(H5::PredType::C_S1, len);
    } else {
      outType = getDataType<T>();
    }

    buffer.resize(nPoints*len);
    DataSpace memspace(1, &nPoints);
    d.read(&buffer[0], outType, memspace, s);
  }

  struct Reference {
    vector<double> lambda;
    vector<double> radiance;
    vector<double> sigma;
  };

  Reference loadRef(unsigned int indexMeasurement, unsigned int indexXTrack, H5File &orbitFile) {
    Reference result;

    hsize_t offset[3], count[3];

    offset[0] = indexMeasurement;
    offset[1] = indexXTrack;
    offset[2] = 0;

    count[0] = 1;
    count[1] = 1;

    DataSet radiance = orbitFile.openDataSet("BinScheme1/ScienceData/Radiance");
    hsize_t dims[3];
    DataSpace s = radiance.getSpace();
    s.getSimpleExtentDims(dims, NULL); // dims[0] = nMeasurements, dims[1] = nXtrack, dims[2] = nLambda
    result.lambda.resize(dims[2]);
    result.radiance.resize(dims[2]);
    result.sigma.resize(dims[2]);

    // create DataSpace object for spectrum/lambda/... buffers: rank=1, length = nLambda
    DataSpace memspace(1, &dims[2]);
    count[2] = dims[2];
    s.selectHyperslab(H5S_SELECT_SET, count, offset);
    radiance.read(&result.radiance[0], H5::PredType::NATIVE_DOUBLE, memspace, s);
    radiance.close();

    DataSet lambda = orbitFile.openDataSet("BinScheme1/ScienceData/BandCenterWavelengths");
    s = lambda.getSpace();
    s.selectHyperslab(H5S_SELECT_SET, count, offset);
    lambda.read(&result.lambda[0], H5::PredType::NATIVE_DOUBLE, memspace, s);
    lambda.close();

    DataSet radianceError = orbitFile.openDataSet("BinScheme1/ScienceData/RadianceError");
    s = radianceError.getSpace();
    s.selectHyperslab(H5S_SELECT_SET, count, offset);
    radianceError.read(&result.sigma[0], H5::PredType::NATIVE_DOUBLE, memspace, s);
    radianceError.close();

    return result;
  }

  bool useForReference(float lon, float lat, float sza, FENO *pTabFeno) {
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

  // longitude should be in the range [0,360]
  if(lon < 0)
    lon+=360;

  return ((lon_min <= lon && lon_max >= lon) || !use_lon)
    && ((lat_min <= lat && lat_max >= lat) || !use_lat)
    && ((sza_min <= sza && sza_max >= sza) || !use_sza);
  }

  // look at the geolocations of spectra in this orbit file.  If the
  // geolocation matches the automatic reference criteria for one of
  // the analysis windows, add the spectrum to the list of refSpectra,
  // and store its offset in the list for that analysis window.
  void find_matching_spectra(const string& fileName, vector<Reference>& refSpectra,
                             vector<vector<vector<size_t> > >& offsets) {

    H5File orbitFile = H5File(fileName.c_str(), H5F_ACC_RDONLY);

    hsize_t nXTrack, nMeasurements;

    // check if data from this orbit file can be used in the reference
    // (date, number of detector rows, ...)
    hsize_t dims[3];
    orbitFile.openDataSet("BinScheme1/ScienceData/Radiance").getSpace().getSimpleExtentDims(dims, NULL);
    nMeasurements = dims[0];
    nXTrack = dims[1];
    if(nXTrack != currentOrbit.nXTrack) // wrong number of detector rows
      return;
    // TODO: check orbitNumber
    //    H5::Attribute orbitNumber1 = orbitFile.openAttribute("OrbitNumber");
    //    unsigned int num1;/
    //    orbitNumber1.read(H5::PredType::NATIVE_UINT, &num1);

    // The reference that we are creating can be used to process this orbit, too:
    referenceFileNames.insert(fileName);

    // read longitudes/latitudes/solar zenith angles:
    vector<float> lons, lats, szas;
    readData(orbitFile, lons, "BinScheme1/GeolocationData/Longitude");
    readData(orbitFile, lats, "BinScheme1/GeolocationData/Latitude");
    readData(orbitFile, szas, "BinScheme1/GeolocationData/SolarZenithAngle");
    auto longitudes = reinterpret_cast<float (*)[nXTrack]>(&lons[0]);
    auto latitudes = reinterpret_cast<float (*)[nXTrack]>(&lats[0]);
    auto solarzenithangles = reinterpret_cast<float (*)[nXTrack]>(&szas[0]);

    for(unsigned int i=0; i<nMeasurements; ++i) {
      for(unsigned int j = 0; j < nXTrack; ++j) {
        float lon = longitudes[i][j];
        float lat = latitudes[i][j];
        float sza = solarzenithangles[i][j];

        bool refLoaded = false;
        // loop over all analysis windows and look if the current
        // spectrum can be used in the automatic reference for any of
        // them.
        for(int analysis_window = 0; analysis_window<NFeno; ++analysis_window) {
          FENO *pTabFeno = &TabFeno[j][analysis_window];
          if (!pTabFeno->hidden &&
              pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC &&
              pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC &&
              useForReference(lon, lat, sza, pTabFeno)) {
            // load spectrum and add it to refSpectra for this
            // row, if not yet done
            if(!refLoaded) {
              refSpectra.push_back(loadRef(i, j, orbitFile));
              refLoaded = true;
            }
            // store offset for this spectrum
            offsets[analysis_window][j].push_back(refSpectra.size() -1);
          }
        }
      }
    }
  }

  Reference averageSpectrum(const vector<Reference>& refs, const vector<size_t>& indices, const vector<double>& refLambda) {
    Reference result;

    result.lambda=refLambda;
    result.radiance.resize(refLambda.size());
    result.sigma.resize(refLambda.size());

    vector<double> derivs(refLambda.size());
    vector<double> tempspectrum(refLambda.size());
    vector<double> tempsigma(refLambda.size());

    for(auto i : indices) {
      // interpolate refs[i] on final wavelength grid refLambda
      int rc = SPLINE_Deriv2(&refs[i].lambda[0],&refs[i].radiance[0],&derivs[0],refs[i].lambda.size(),__func__);
      rc |= SPLINE_Vector(&refs[i].lambda[0],&refs[i].radiance[0],&derivs[0],derivs.size(),&refLambda[0],&tempspectrum[0],tempspectrum.size(),SPLINE_CUBIC);
      if (rc) {
        throw std::runtime_error("Error interpolating radiance on automatic reference wavelength grid.");
      }

      rc = SPLINE_Vector(&refs[i].lambda[0],&refs[i].sigma[0],NULL,refs[i].lambda.size(),&refLambda[0],&tempsigma[0],tempsigma.size(),SPLINE_LINEAR);
      if (rc) {
        throw std::runtime_error("Error interpolating radiance errors on automatic reference wavelength grid.");
      }

      for(unsigned int j=0; j<refLambda.size(); ++j) {
        result.radiance[j] += tempspectrum[j];
        result.sigma[j] += tempsigma[j]*tempsigma[j];
      }

    }

    // take averages
    for(unsigned int i=0; i<refLambda.size(); ++i) {
      result.radiance[i] /= indices.size();
      result.sigma[i] = sqrt(result.sigma[i])/indices.size();
    }

    return result;
  }

  RC make_automatic_reference(const ENGINE_CONTEXT *pEngineContext) {
    RC rc = ERROR_ID_NO;

    // vector of all used reference spectra:
    vector<Reference > refSpectra;

    // for each detector row and each analysis window with an
    // automatic reference, we keep a vector of the positions of all
    // the refSpectra we want to use for this row and window
    vector<vector<vector<size_t> > > offsets(NFeno);
    for(int i=0; i<NFeno; ++i) {
      offsets[i].resize(currentOrbit.nXTrack);
    }

    // Browse all files in the same directory as the current input file:
    const char *pathsep = std::strrchr(pEngineContext->fileInfo.fileName, '/');
    string directory = pathsep ? string(pEngineContext->fileInfo.fileName, pathsep) : ".";
    for (auto& filename : dir_iter(directory)) {
      try {
        find_matching_spectra(directory + "/" + filename, refSpectra, offsets);
      } catch (H5::Exception &e) {
        cerr << "invalid file '" << directory << "/" << filename << "', " << endl; // << e.getDetailMsg()
      }
    }

    // generate average spectrum for each analysis window and each detector row
    for(unsigned int i=0; i<currentOrbit.nXTrack; ++i) {
      for(int j=0; j<NFeno; ++j) {
        FENO *pTabFeno = &TabFeno[i][j];

        if(pTabFeno->hidden || !pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)
          continue;

        vector<double> refLambda(currentOrbit.nLambda);
        memcpy(&refLambda[0],pTabFeno->LambdaK, currentOrbit.nLambda*sizeof(refLambda[0]));

        // TODO: check that we have enough matching spectra (how much is enough?)
        Reference ref = averageSpectrum(refSpectra, offsets[j][i], refLambda);

        memcpy(pTabFeno->Sref, &ref.radiance[0], sizeof(ref.radiance[0])*ref.radiance.size());
        // normalize Sref
        VECTOR_NormalizeVector(pTabFeno->Sref-1,NDET[i],&pTabFeno->refNormFact, __func__);

        memcpy(pTabFeno->SrefSigma, &ref.sigma[0], sizeof(ref.sigma[0])*ref.sigma.size());

        memcpy(pTabFeno->LambdaRef,pTabFeno->LambdaK, sizeof(pTabFeno->LambdaK[0]) * currentOrbit.nLambda);
      }
    }
    return rc;
  }

}

RC OMPS_set(ENGINE_CONTEXT *pEngineContext) {
  currentOrbit.file = H5File(pEngineContext->fileInfo.fileName, H5F_ACC_RDONLY);

  try {
    DataSet d = currentOrbit.file.openDataSet("BinScheme1/ScienceData/Radiance");
    DataSpace s = d.getSpace();

    hsize_t dims[3];
    s.getSimpleExtentDims(dims, NULL); // dims[0] = nMeasurements, dims[1] = nXtrack, dims[2] = nLambda
    d.close();

    pEngineContext->recordNumber = dims[0]*dims[1];
    pEngineContext->n_alongtrack= dims[0];
    pEngineContext->n_crosstrack= dims[1];

    currentOrbit.nMeasurements = dims[0];
    currentOrbit.nXTrack = dims[1];
    currentOrbit.nLambda = dims[2];

    ANALYSE_swathSize = dims[1];
    for (int i=0; i<ANALYSE_swathSize; ++i) {
      NDET[i]=dims[2];
    }

    // store metadata in geoData buffers:
    readData(currentOrbit.file, geoData.latitude, "BinScheme1/GeolocationData/Latitude");
    readData(currentOrbit.file, geoData.longitude, "BinScheme1/GeolocationData/Longitude");
    readData(currentOrbit.file, geoData.groundPixelQualityFlags, "BinScheme1/GeolocationData/GroundPixelQualityFlags");
    readData(currentOrbit.file, geoData.satelliteAzimuth, "BinScheme1/GeolocationData/SatelliteAzimuthAngle");
    readData(currentOrbit.file, geoData.satelliteZenithAngle, "BinScheme1/GeolocationData/SatelliteZenithAngle");
    readData(currentOrbit.file, geoData.solarAzimuth, "BinScheme1/GeolocationData/SolarAzimuthAngle");
    readData(currentOrbit.file, geoData.solarZenithAngle, "BinScheme1/GeolocationData/SolarZenithAngle");

    readData(currentOrbit.file, geoData.spacecraftAltitude, "BinScheme1/GeolocationData/SpacecraftAltitude");
    readData(currentOrbit.file, geoData.spacecraftLatitude, "BinScheme1/GeolocationData/SpacecraftLatitude");
    readData(currentOrbit.file, geoData.spacecraftLongitude, "BinScheme1/GeolocationData/SpacecraftLongitude");

    readData(currentOrbit.file, geoData.utcDate, "BinScheme1/GeolocationData/UTC_CCSDS_A");

    // store irradiance & wavelengths:
    readData(currentOrbit.file, currentOrbit.irradiances, "BinScheme1/CalibrationData/SolarFlux");
    readData(currentOrbit.file, currentOrbit.wavelengths, "BinScheme1/CalibrationData/SolarFluxWavelengths");

  } catch (...) {
    cerr << "Exception opening dataset" << endl;
  }
  return 0;
}

RC OMPS_read(ENGINE_CONTEXT *pEngineContext,int record) {

  int rc = 0;

  int indexMeasurement = (record - 1) / currentOrbit.nXTrack;
  int indexXTrack = (record -1) % currentOrbit.nXTrack;

  hsize_t offset[3];
  hsize_t count[3];

  offset[0] = indexMeasurement;
  offset[1] = indexXTrack;
  offset[2] = 0;

  count[0] = 1;
  count[1] = 1;

  try {
    DataSet radiance = currentOrbit.file.openDataSet("BinScheme1/ScienceData/Radiance");
    hsize_t dims[3];
    DataSpace s = radiance.getSpace();
    s.getSimpleExtentDims(dims, NULL); // dims[0] = nMeasurements, dims[1] = nXtrack, dims[2] = nLambda

    // create DataSpace object for spectrum/lambda/... buffers: rank=1, length = nLambda
    DataSpace memspace(1, &dims[2]);
    count[2] = dims[2];
    s.selectHyperslab(H5S_SELECT_SET, count, offset);
    radiance.read(pEngineContext->buffers.spectrum, H5::PredType::NATIVE_DOUBLE, memspace, s);
    radiance.close();

    DataSet lambda = currentOrbit.file.openDataSet("BinScheme1/CalibrationData/BandCenterWavelengths");
    s = lambda.getSpace();
    s.selectHyperslab(H5S_SELECT_SET, count, offset);
    lambda.read(pEngineContext->buffers.lambda, H5::PredType::NATIVE_DOUBLE, memspace, s);
    lambda.close();

    DataSet radianceError = currentOrbit.file.openDataSet("BinScheme1/ScienceData/RadianceError");
    s = radianceError.getSpace();
    s.selectHyperslab(H5S_SELECT_SET, count, offset);
    radianceError.read(pEngineContext->buffers.sigmaSpec, H5::PredType::NATIVE_DOUBLE, memspace, s);
    radianceError.close();

    auto irradiances = reinterpret_cast<double (*)[currentOrbit.nLambda]>(&currentOrbit.irradiances[0]);
    auto irradiance_lambdas = reinterpret_cast<double (*)[currentOrbit.nLambda]>(&currentOrbit.wavelengths[0]);

    memcpy(pEngineContext->buffers.irrad, irradiances[indexXTrack], currentOrbit.nLambda*sizeof(irradiances[0][0]));
    memcpy(pEngineContext->buffers.lambda_irrad, irradiance_lambdas[indexXTrack], currentOrbit.nLambda*sizeof(irradiance_lambdas[0][0]));

  } catch(...) {
    cerr << "error in " <<  __func__ <<endl;
  }

  RECORD_INFO *pRecord=&pEngineContext->recordInfo;
  pRecord->i_alongtrack = indexMeasurement;
  pRecord->i_crosstrack = indexXTrack;
  pRecord->latitude=geoData.latitude[record-1];
  pRecord->longitude=geoData.longitude[record-1];

  auto datetime = &pRecord->present_datetime;

  int year,month,day,hour,min,sec,ms;
  sscanf(&geoData.utcDate[28*(indexMeasurement)], "%04d-%02d-%02dT%02d:%02d:%02d.%06dZ",
         &year, &month, &day, &hour, &min, &sec, &ms);

  datetime->thedate.da_year = year;
  datetime->thedate.da_mon = month;
  datetime->thedate.da_day = day;
  datetime->thetime.ti_hour = hour;
  datetime->thetime.ti_min = min;
  datetime->thetime.ti_sec = sec;

  pRecord->latitude=geoData.latitude[record-1];
  pRecord->longitude=geoData.longitude[record-1];
  pRecord->Zm=geoData.solarZenithAngle[record-1];
  pRecord->Azimuth=geoData.solarAzimuth[record-1];
  pRecord->zenithViewAngle=geoData.satelliteZenithAngle[record-1];
  pRecord->azimuthViewAngle=geoData.satelliteAzimuth[record-1];
  pRecord->useErrors=0; // no errors provided for irradiance?

  return rc;
}

RC OMPS_load_analysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {

  RC rc = ERROR_ID_NO;

  int useKurucz = 0;

  auto wavelengths = reinterpret_cast<double (*)[currentOrbit.nLambda]>(&currentOrbit.wavelengths[0]);
  auto irradiances = reinterpret_cast<double (*)[currentOrbit.nLambda]>(&currentOrbit.irradiances[0]);

  for(size_t j=0; j<currentOrbit.nXTrack; ++j) {
    for(int i=0; i<NFeno; ++i) {
      FENO *pTabFeno=&TabFeno[j][i];
      pTabFeno->NDET = currentOrbit.nLambda;
      memcpy(pTabFeno->LambdaRef, wavelengths[j], currentOrbit.nLambda*sizeof(double));
      memcpy(pTabFeno->Sref, irradiances[j], currentOrbit.nLambda*sizeof(double));

      rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact, __func__);
      if(rc)
        return rc;

      if (!pTabFeno->hidden) {
        // Gaps : rebuild subwindows on new wavelength scale

        doas_spectrum *new_range = spectrum_new();
        int DimL = 0;
        for (int indexWindow = 0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++) {
          int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
          int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

          spectrum_append(new_range, pixel_start, pixel_end);
          DimL += pixel_end - pixel_start +1;
        }

        // Buffers allocation
        FIT_PROPERTIES_free(__func__,&pTabFeno->fit_properties);
        pTabFeno->fit_properties.DimL=DimL;
        FIT_PROPERTIES_alloc(__func__,&pTabFeno->fit_properties);

        pTabFeno->fit_properties.specrange = new_range;

        pTabFeno->Decomp=1;

        if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
            ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) &&
             ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))
          return rc;

        memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);
        memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);
        useKurucz += pTabFeno->useKurucz;
      }
    }

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ)) {
      KURUCZ_Init(0, j);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,pEngineContext->project.spectra.displayDataFlag,0,responseHandle,j))!=ERROR_ID_NO))
        return rc;
    }
  }

  if(pEngineContext->analysisRef.refAuto && referenceFileNames.find(pEngineContext->fileInfo.fileName) == referenceFileNames.end() ) {
    referenceFileNames.clear();
    rc = make_automatic_reference(pEngineContext);

    for(size_t i=0; i<currentOrbit.nXTrack; ++i) {
      // fit wavelength shift between calibrated solar irradiance
      // and automatic reference spectrum and apply this shift to
      // absorption crosssections
      rc = ANALYSE_AlignReference(pEngineContext,2,responseHandle,i);
    }

  }

  return rc;
}

void OMPS_ReleaseBuffers(void) {
  referenceFileNames.clear();
}

// TODO
int OMPS_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day) {

 *orbit_year=*orbit_month=*orbit_day=0;
 return 1;
 // std::istringstream orbit_start(&geoData.utcDate[0]);
 // // time_coverage_start is formatted as "YYYY-MM-DD"
 // char tmp; // to skip "-" chars
 // orbit_start >> *orbit_year >> tmp >> *orbit_month >> tmp >> *orbit_day;
 // return  orbit_start.good() ? 0 : 1;
}
