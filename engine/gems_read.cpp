
#include <map>
#include <cassert>
#include <vector>
#include <cstdint>

#include <iostream>
#include <sstream>


#include "gems_read.h"
#include "netcdfwrapper.h"

extern "C" {
#include <math.h>
#include "zenithal.h"
#include "output_netcdf.h"
#include "kurucz.h"
#include "engine_context.h"
#include "analyse.h"
#include "output.h"
#include "winthrd.h"
#include "mediate.h"
#include "spline.h"
#include "vector.h"
}

using std::vector;
using std::string;
using std::map;
using std::cout;

static NetCDFFile radiance_file;

static size_t n_wve;       // number of wavelengths
static size_t n_rows;      // number of rows, cross-track
static size_t n_images;    // number of images, along-track
static bool   wve_reordering_flag;   
static int    loadReferenceFlag=0; // if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
                                   // N/A here because files of the current orbit are not pre-loaded

// ====================== 
// STRUCTURES DEFINITIONS
// ======================

typedef struct gems_reference
 {
  string     ref_filename;
  size_t     ref_n_wve;
  size_t     ref_n_rows;
  vector<int>ref_use_rows;
  NetCDFFile ref_file;
 }
GEMS_REFERENCE;

GEMS_REFERENCE ref_list[2*MAX_FENO]; // 2 ref spectra possible per analysis windows
static int nref=0;

struct data_fields {
  vector<float> sza, saa, vza, vaa, lon, lat, exp_time;
  vector <unsigned short> gpqf,snow_index,alt;   // ground pixel quality flag
  vector <unsigned char> xqf;    // xtrack quality flag
  vector <double> acq_time;
};

static struct data_fields radiance_file_data;
static double radiance_fillvalue;
static int gems_orbit_year=0,gems_orbit_month=0,gems_orbit_day=0;

void GEMS_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day) {
  *orbit_year=gems_orbit_year;
  *orbit_month=gems_orbit_month;
  *orbit_day=gems_orbit_day;
}

static void read_data_fields(NetCDFFile& orbit_file) //,bool *use_row,INDEX *use_row_index) 
 {
  // Float fields one dimension
  
  std::pair<string, vector<float>&> ffields1[] = 
   {
    {"exposure_time", radiance_file_data.exp_time}
   };
   
  // Float fields two dimensions

  std::pair<string, vector<float>&> ffields2[] = {
    {"sun_zenith_angle", radiance_file_data.sza},
    {"sun_azimuth_angle", radiance_file_data.saa},
    {"sc_zenith_angle", radiance_file_data.vza},
    {"sc_azimuth_angle", radiance_file_data.vaa},
    {"pixel_longitude", radiance_file_data.lon},
    {"pixel_latitude", radiance_file_data.lat}
  };
  
  std::pair<string, vector<unsigned char>&> cfields[] = {
    {"xtrack_quality_flag", radiance_file_data.xqf}
  };
    
  std::pair<string, vector<double>&> dfields[] = {
    {"image_acquisition_time", radiance_file_data.acq_time}
  };
  
  std::pair<string, vector<unsigned short>&> ifields[] = {
    {"ground_pixel_quality_flag", radiance_file_data.gpqf},
    {"snow_index", radiance_file_data.snow_index},
    {"terrain_height", radiance_file_data.alt}
  };

  const size_t start[] = {0, 0},start_t[]={0};
  const size_t count[] = {n_images,n_rows},count_t[]={n_images};
   
  for (auto& f: ffields1) {
    if (orbit_file.hasVar(f.first)) {
       f.second.resize(n_images);
       orbit_file.getVar(f.first, start_t, count_t, f.second.data());   // n_images
    }
  }
  
  for (auto& f: ffields2) {
    if (orbit_file.hasVar(f.first)) {
       f.second.resize(n_rows * n_images);
       orbit_file.getVar(f.first, start, count, f.second.data());
    }
  }
  
  for (auto& f: cfields) {
    if (orbit_file.hasVar(f.first)) {
       f.second.resize(n_rows * n_images);
       orbit_file.getVar(f.first, start, count, f.second.data());
    }
  }

  for (auto& f: ifields) {
    if (orbit_file.hasVar(f.first)) {
       f.second.resize(n_rows * n_images);
       orbit_file.getVar(f.first, start, count, f.second.data());
    }
  }
  
  for (auto& f: dfields) {
    if (orbit_file.hasVar(f.first)) {
       f.second.resize(n_images);
       orbit_file.getVar(f.first, start_t, count_t, f.second.data());
    }
  }
}

void GEMS_CloseReferences(void)
 {
  for (int indexRef=0;indexRef<nref;indexRef++)
   {
    GEMS_REFERENCE *pRef=&ref_list[indexRef];

    pRef->ref_filename="";
    pRef->ref_n_wve=0;
    pRef->ref_n_rows=0;
    pRef->ref_file.close();
   }
 }
 
RC GEMS_LoadReference(char *filename,int indexFenoColumn,double *lambda,double *spectrum,int *nwve)
 {
   // Declarations

   GEMS_REFERENCE *pRef;
   int i;
   RC rc;

   // Initializations

   rc=ERROR_ID_NO;
   *nwve=0;

   // Search for existing reference

   for (i=0;i<nref;i++)
    if (ref_list[i].ref_filename==filename)
     break;

   if ((i==nref) && (nref<2*MAX_FENO))
    {
     try
      {
       pRef=&ref_list[nref];

       pRef->ref_file=NetCDFFile(filename);
       pRef->ref_n_wve = pRef->ref_file.dimLen("dim_image_band");
       pRef->ref_n_rows = pRef->ref_file.dimLen("dim_image_y");
       pRef->ref_filename=filename;

       nref++;
      } catch(std::runtime_error& e) {
       rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
     }
    }

   if (!rc && (i<nref))
    {
     pRef=&ref_list[i];

     try
      {
       bool reorder_flag=pRef->ref_file.hasAttr("wavelength_position_reorder");
       
       const size_t start[] = {(reorder_flag)?(size_t)indexFenoColumn:0,(reorder_flag)?0:(size_t)indexFenoColumn};   
       const size_t count[] = {(reorder_flag)?1:pRef->ref_n_wve,(reorder_flag)?pRef->ref_n_wve:1}; 

       pRef->ref_file.getVar("wavelength", start, count, lambda);
       pRef->ref_file.getVar("image_pixel_values", start, count, spectrum);
        
       *nwve=pRef->ref_n_wve; 
      }
     catch(std::runtime_error& e)
      {
       rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                           (string { "Can not read reference file " } + filename).c_str());
      }
    }

       // TODO : check nwve and if null -> should reset the corresponding use row
       
   // Return

   return rc;
 }
 
int GEMS_init(ENGINE_CONTEXT *pEngineContext,char *ref_filename,int* n_wavel_temp) 
 {
  int rc=ERROR_ID_NO;
  try 
   {
    NetCDFFile reference_file(ref_filename);
    int col_dim = reference_file.dimLen((pEngineContext->radAsRefFlag)?"col_dim":"dim_image_y");
    int spectral_dim = reference_file.dimLen((pEngineContext->radAsRefFlag)?"spectral_dim":"dim_image_band");
    
    if (!pEngineContext->radAsRefFlag)
     {
      if (!(rc=GEMS_LoadReference(ref_filename,0,pEngineContext->buffers.lambda,pEngineContext->buffers.spectrum,n_wavel_temp)))
       {
        ANALYSE_swathSize=col_dim;
        for(int i=0; i<ANALYSE_swathSize; ++i) 
         NDET[i] = spectral_dim;
       } 
     }
    else if (ANALYSE_swathSize != col_dim) 
     std::cout << "ERROR: swathSize != col_dim!" << std::endl; 

    *n_wavel_temp = spectral_dim;
   }  
  catch(std::runtime_error& e) 
   {
    return rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
   }
   
  return rc; 
 } 
  

int GEMS_Set(ENGINE_CONTEXT *pEngineContext) {
  bool has_irradiance,has_radiance;
  int orbit_year,orbit_month,orbit_day;
  
  int rc = 0;

  try {
    radiance_file = NetCDFFile(pEngineContext->fileInfo.fileName);
    radiance_file_data = data_fields();
    
    if (radiance_file.hasAttr("file_generation_time"))
     {
      string str=radiance_file.getAttText("file_generation_time");
      sscanf(str.c_str(),"%04d%02d%02d",&gems_orbit_year,&gems_orbit_month,&gems_orbit_day);
     }
    
    has_radiance=(radiance_file.dimID("dim_image_x")!=-1)?true:false;
    has_irradiance=!has_radiance;

    n_images = (has_radiance)?radiance_file.dimLen("dim_image_x"):1;
    n_wve=radiance_file.dimLen("dim_image_band");
    n_rows=radiance_file.dimLen("dim_image_y");
    wve_reordering_flag=radiance_file.hasAttr("wavelength_position_reorder");
    
//     for (int i=0;i<n_rows;i++)
//      pEngineContext->project.instrumental.use_row_index[i]=
//       (pEngineContext->project.instrumental.use_row[i])?n_rows_used++:ITEM_NONE;

    pEngineContext->recordNumber = n_rows * n_images;  // could be a third dimension
    
    pEngineContext->n_alongtrack= n_images;
    pEngineContext->n_crosstrack= n_rows;
    
    ANALYSE_swathSize = n_rows;

    for (int i=0; i<MAX_SWATHSIZE; ++i)
     NDET[i] = n_wve;

    if ((THRD_id==THREAD_TYPE_KURUCZ) && (has_irradiance))
     {
      radiance_fillvalue = radiance_file.getFillValue<float>("image_pixel_values");
      if (pEngineContext->project.asciiResults.newcalibFlag && strlen(pEngineContext->project.asciiResults.newCalibPath))
       netcdf_open_calib(pEngineContext, pEngineContext->project.asciiResults.newCalibPath,n_rows,n_wve); // An output file should be specified
     }
    else if (has_radiance)
     {
      pEngineContext->recordInfo.satellite.latitude=radiance_file.getAttDouble("nominal_sub_latitude");
      pEngineContext->recordInfo.satellite.longitude=radiance_file.getAttDouble("nominal_sub_longitude");
      pEngineContext->recordInfo.satellite.altitude=radiance_file.getAttDouble("nominal_satellite_height");
      
      radiance_fillvalue = radiance_file.getFillValue<float>("image_pixel_values");
      read_data_fields(radiance_file); // ,pEngineContext->project.instrumental.use_row,pEngineContext->project.instrumental.use_row_index);
     }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

int GEMS_Read(ENGINE_CONTEXT *pEngineContext, int record)
 {
  int rc=0;
  int nbad=0;

  short *pixelQF;

  pixelQF=NULL;

  assert(record > 0);
  
  const size_t i_alongtrack = (record - 1) / n_rows;
  const size_t i_crosstrack = (record - 1) % n_rows;
  
  RECORD_INFO *pRecord = &pEngineContext->recordInfo;

  if (!pEngineContext->project.instrumental.use_row[i_crosstrack])
   rc=ERROR_ID_FILE_RECORD;
  else if ((pixelQF=(short *)MEMORY_AllocBuffer(__func__,"pixelQF",n_wve,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL)
   rc=ERROR_ID_ALLOC;   
  else
   {
    try
     {
      if ((THRD_id==THREAD_TYPE_KURUCZ) || (radiance_file.dimID("dim_image_x")==-1))
       {
        const size_t start[] = {(wve_reordering_flag)?i_crosstrack:0,(wve_reordering_flag)?0:i_crosstrack}; 
        const size_t count[] = {(wve_reordering_flag)?1:n_wve,(wve_reordering_flag)?n_wve:1};         
        pRecord->i_alongtrack=i_alongtrack;
        pRecord->i_crosstrack=i_crosstrack;

        radiance_file.getVar("wavelength", start, count, pEngineContext->buffers.lambda);
        radiance_file.getVar("image_pixel_values", start, count, pEngineContext->buffers.spectrum);
       }
      else if (radiance_file.dimID("dim_image_x")!=-1)
       {
        const size_t start_s[] = {(wve_reordering_flag)?i_alongtrack:0,i_crosstrack,(wve_reordering_flag)?0:i_alongtrack};
        const size_t count_s[] = {(wve_reordering_flag)?1:n_wve,1,(wve_reordering_flag)?n_wve:1};
        const size_t start_w[] = {(wve_reordering_flag)?i_crosstrack:0,(wve_reordering_flag)?0:i_crosstrack};
        const size_t count_w[] = {(wve_reordering_flag)?1:n_wve,(wve_reordering_flag)?n_wve:1};
        
        radiance_file.getVar("wavelength", start_w, count_w, pEngineContext->buffers.lambda);
        radiance_file.getVar("image_pixel_values", start_s, count_s, pEngineContext->buffers.spectrum);
        radiance_file.getVar("bad_pixel_mask", start_s, count_s, pixelQF);

        nbad=0;
        for (int i=0;i<(int)n_wve;i++)
          if (((pEngineContext->buffers.pixel_QF[i]=pixelQF[i])!=0) || 
               (pEngineContext->buffers.spectrum[i]==radiance_fillvalue))
            nbad++;
          
        // spectrum is rejected if all pixels are marked as bad
          
        if (nbad==(int)n_wve)  
         rc=ERROR_ID_FILE_RECORD;
        else
         {
          pRecord->i_alongtrack=i_alongtrack;
          pRecord->i_crosstrack=i_crosstrack;

          int geoIndex2=i_alongtrack*n_rows+i_crosstrack; 
          
          pRecord->latitude = radiance_file_data.lat.size() ? radiance_file_data.lat[geoIndex2] : QDOAS_FILL_FLOAT;
          pRecord->longitude = radiance_file_data.lon.size() ? radiance_file_data.lon[geoIndex2] : QDOAS_FILL_FLOAT;
          pRecord->Zm = radiance_file_data.sza.size() ? radiance_file_data.sza[geoIndex2] : QDOAS_FILL_FLOAT;
          pRecord->Azimuth = radiance_file_data.saa.size() ? radiance_file_data.saa[geoIndex2] : QDOAS_FILL_FLOAT;
          pRecord->zenithViewAngle = radiance_file_data.vza.size() ? radiance_file_data.vza[geoIndex2] : QDOAS_FILL_FLOAT;
          pRecord->azimuthViewAngle = radiance_file_data.vaa.size() ? radiance_file_data.vaa[geoIndex2] : QDOAS_FILL_FLOAT;
          pRecord->ground_pixel_QF = radiance_file_data.gpqf.size() ? radiance_file_data.gpqf[geoIndex2] : QDOAS_FILL_USHORT;
          pRecord->xtrack_QF = radiance_file_data.xqf.size() ? radiance_file_data.xqf[geoIndex2] : QDOAS_FILL_USHORT;
          pRecord->Tint=radiance_file_data.exp_time.size() ? radiance_file_data.exp_time[i_alongtrack] : QDOAS_FILL_FLOAT;

          if (pRecord->Tint<=0.)
            pRecord->Tint=1.;
          
          double nsec,mjd,sumdays,nDaysInYear,hour,mm,sec;
          int year;

          nsec=radiance_file_data.acq_time.size() ? radiance_file_data.acq_time[i_alongtrack] : 0.;
//           if ((nsec>0.) && (radiance_file_data.exp_time.size()))
//            for (int i=0;i<i_alongtrack;i++)
//             nsec+=radiance_file_data.exp_time[i];

          nsec-=12*3600.;    // reference time should be 01/01/2000 12 UTC.
          mjd=floor(nsec/86400.);
          
          for (year=2000,sumdays=(double)0.,nDaysInYear=(double)366.;
               sumdays+nDaysInYear<mjd;)
           {
            year++;
            sumdays+=nDaysInYear;
            nDaysInYear=((year%4)==0)?(double)366.:(double)365.;
           }
     
          
          pRecord->present_datetime.thedate.da_year=year;
          pRecord->present_datetime.thedate.da_mon=(short)ZEN_FNCaljmon(year,(int)floor(mjd-sumdays+1.));
          pRecord->present_datetime.thedate.da_day=(short)ZEN_FNCaljday(year,(int)floor(mjd-sumdays+1.));
          
          nsec-=mjd*86400;
          hour=floor(nsec/3600.);
          nsec-=hour*3600;
          mm=floor(nsec/60.);
          nsec-=mm*60.;
          sec=floor(nsec);
          
          pRecord->present_datetime.thetime.ti_hour=(short)hour;
          pRecord->present_datetime.thetime.ti_min=(short)mm;
          pRecord->present_datetime.thetime.ti_sec=(short)sec;
          pRecord->present_datetime.millis=(int)floor((nsec-sec)*1000);  
         }
       }
      else
        rc=ERROR_ID_FILE_RECORD;
     }
    catch(std::runtime_error& e)
     {
      rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
     }
   }

  if (pixelQF!=NULL)
   MEMORY_ReleaseBuffer(__func__,"pixelQF",pixelQF);
  
  return rc;
 }

void gems_clean(void) {
  radiance_file.close();
  
  gems_orbit_year=gems_orbit_month=gems_orbit_day=0;

  for (int i=0; i<MAX_SWATHSIZE; ++i)
   NDET[i] = GEMS_INIT_LENGTH;

  n_wve = n_images = n_rows = 0;
}

RC GEMS_LoadCalib(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn,void *responseHandle) {

  const int n_wavel=n_wve;
  RC rc=ERROR_ID_NO;

  // don't continue when current file has an error, or if we are
  // working with automatic references and don't need to create a new
  // reference:

  // Browse analysis windows and load missing data

  for (int indexFeno=0; indexFeno<NFeno && !rc; indexFeno++) {
       FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
       pTabFeno->NDET=n_wavel;
       pTabFeno->rc = 0;

     if (!pTabFeno->useRefRow) continue;

     memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda,sizeof(double)*n_wavel);
     memcpy(pTabFeno->SrefEtalon,pEngineContext->buffers.spectrum,sizeof(double)*n_wavel);
  }

  // Wavelength calibration alignment

  KURUCZ_Init(0,indexFenoColumn);

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION GEMS_LoadAnalysis
// -----------------------------------------------------------------------------
//!
//! \fn      RC GEMS_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
//! \details Load analysis parameters depending on the reference spectrum
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.\n
//! \param   [in]  responseHandle  address where to transmit error to the user interface\n
//! \return  ERROR_ID_FILE_END if the requested record number is not found\n
//!          ERROR_ID_FILE_RECORD if the requested record doesn't satisfy current criteria (for example for the selection of the reference)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC GEMS_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) 
 {
  const int n_wavel=n_wve;
  // int n_wavel_tmp;
  int saveFlag= pEngineContext->project.spectra.displayDataFlag;

  RC rc=0;

  // don't continue when current file has an error, or if we are
  // working with automatic references and don't need to create a new
  // reference:
  
  if (rc || (pEngineContext->analysisRef.refAuto && !loadReferenceFlag) )
    return rc;

  int useUsamp=0,useKurucz=0,useRef2=0;
  
  // Browse analysis windows and load missing data

  for (int indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++) {

   for (int indexFeno=0; indexFeno<NFeno && !rc; indexFeno++) {
     FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
     pTabFeno->NDET=n_wavel;
     pTabFeno->rc = 0;
     
     if (!pTabFeno->useRefRow) continue;
     
     // Load calibration and reference spectra
     
     // For gems : we always use the irradiance spectrum as reference for calibration but there is the possibility of a radasref2

     if (!pTabFeno->hidden && !pTabFeno->gomeRefFlag) { 
       // use irradiance 
       // GEMS_LoadReference(pTabFeno->ref1,indexFenoColumn,pTabFeno->LambdaRef,pTabFeno->SrefEtalon,n_wavel_temp);

       // we consider ref1
//        if (!pTabFeno->useRadAsRef1 || // use the irradiance
//            !(rc=SPLINE_Vector(pTabFeno->LambdaRadAsRef1,pTabFeno->SrefRadAsRef1,pTabFeno->Deriv2RadAsRef1,pTabFeno->n_wavel_ref1,pTabFeno->LambdaRef,pTabFeno->SrefEtalon,n_wavel,SPLINE_CUBIC)))
//            // this is RadAsRef
//          rc = VECTOR_NormalizeVector(pTabFeno->SrefEtalon-1,pTabFeno->NDET,&pTabFeno->refNormFact,"GEMS_LoadAnalysis (Reference) ");
       
       if (!rc){
         if ((pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) &&!strlen(pTabFeno->ref2))
          memcpy(pTabFeno->Sref,pTabFeno->SrefEtalon,sizeof(double)*n_wavel);
         else if ((pTabFeno->useRadAsRef2)) {
          rc=SPLINE_Vector(pTabFeno->LambdaRadAsRef2,pTabFeno->SrefRadAsRef2,pTabFeno->Deriv2RadAsRef2,pTabFeno->n_wavel_ref2,pTabFeno->LambdaRef,pTabFeno->Sref,n_wavel,SPLINE_CUBIC);
          if (rc == 0) rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,n_wavel,&pTabFeno->refNormFact,"GEMS_LoadAnalysis (Reference) ");
         }
        }
       pTabFeno->useEtalon=pTabFeno->displayRef=1;

       // Browse symbols

       for (int indexTabCross=0; indexTabCross<pTabFeno->NTabCross; indexTabCross++) {
         CROSS_REFERENCE *pTabCross=&pTabFeno->TabCross[indexTabCross];
         WRK_SYMBOL *pWrkSymbol=&WorkSpace[pTabCross->Comp];

         // Cross sections and predefined vectors

         if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
              ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
               ((indexTabCross==pTabFeno->indexCommonResidual) ||
                (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE)))))) {
             rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET);
         }

       // Gaps : rebuild subwindows on new wavelength scale

       doas_spectrum *new_range = spectrum_new();
       int DimL=0;
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
       // new spectral windows
       pTabFeno->fit_properties.specrange = new_range;

       if (!rc) rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,indexFenoColumn);

       if ( !rc && ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) && pTabFeno->xsToConvolute)) {
         rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,indexFenoColumn,pSlitOptions->slitFunction.slitWveDptFlag);
       }

       if (strlen(pTabFeno->ref2))
        useRef2++;
      }
     }
 
     memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double) *pTabFeno->NDET);
     memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double) *pTabFeno->NDET);

     useUsamp+=pTabFeno->useUsamp;
     useKurucz+=pTabFeno->useKurucz;
     if (rc != ERROR_ID_NO){
        pTabFeno->rc = rc;
        rc = ERROR_ID_NO;
     }
   }

  // Wavelength calibration alignment

  if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ)) {
    KURUCZ_Init(0,indexFenoColumn);

    if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,indexFenoColumn)) !=ERROR_ID_NO))
    {
       // Error on one irradiance spectrum shouldn't stop the analysis of other spectra
       ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_IMAGER_CALIB, 1+indexFenoColumn);
       printf("Kurucz (2) returns error %d for row %d\n",rc,indexFenoColumn+1); 
       for (int indexWindow=0;indexWindow<NFeno;indexWindow++)
         TabFeno[indexFenoColumn][indexWindow].rcKurucz=rc;
       rc=ERROR_ID_NO;
     }
/*     for (int indexFeno=0; indexFeno<NFeno && !rc; indexFeno++) 
      {
       FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
       if (!pTabFeno->hidden)
     
           {
            printf("Kurucz (2) returns error %d for row %d\n",rc,indexFenoColumn);
            pTabFeno->rc=rc;
            pTabFeno->useRefRow=false;
            rc=ERROR_ID_NO;
           }
      }   */
      // goto EndGEMS_LoadAnalysis;
  }
 }

  // Build undersampling cross sections

  if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ) && !(rc=ANALYSE_UsampLocalAlloc(0))) {
    // ANALYSE_UsampLocalFree();

   for (int indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)

    if (((rc=ANALYSE_UsampLocalAlloc(0)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(0,0,indexFenoColumn)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(1,ITEM_NONE,indexFenoColumn)) !=ERROR_ID_NO))

      goto EndGEMS_LoadAnalysis;
  }

  // Automatic reference selection
  
   if ((THRD_id!=THREAD_TYPE_KURUCZ) && useRef2)
//      ( (gome1netCDF_loadReferenceFlag && !(rc=GOME1NETCDF_NewRef(pEngineContext,responseHandle))) || useRef2))
    for (int indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)
      rc=ANALYSE_AlignReference(pEngineContext,2,responseHandle,indexFenoColumn); // 2 is for automatic mode

  // if (!rc) gome1netCDF_loadReferenceFlag=0;

EndGEMS_LoadAnalysis:

  return rc;
}

