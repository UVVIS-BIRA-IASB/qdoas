#include <array>
#include <algorithm>
#include <map>
#include <cassert>
#include <sstream>
#include <ctime>
#include <string>

#include <netcdf.h>

#include "netcdfwrapper.h"
#include "mediate_xsconv_output_netcdf.h"
 
using std::string;
using std::vector;
using std::array;
using std::map;
using std::to_string;

extern "C" {
#include "engine.h"
#include "output_formats.h"
#include "xsconv.h"
#include "matrix.h"
#include "mediate_xsconv.h"
#include "winfiles.h"
}

static NetCDFFile output_file;
static NetCDFGroup output_group;

// const char *QDOAS_FILL_STRING = "";
// const char QDOAS_FILL_BYTE = -127;
// const char QDOAS_FILL_CHAR = 0;
// const short QDOAS_FILL_SHORT = -32767;
// const int QDOAS_FILL_INT = -2147483647;
// const float QDOAS_FILL_FLOAT = 9.9692099683868690e+36f; /* near 15 * 2^119 */
const double QDOAS_FILL_DOUBLE = (double)9.9692099683868690e+306;
// const unsigned char QDOAS_FILL_UBYTE = 255;
// const unsigned short QDOAS_FILL_USHORT = 65535;
// const unsigned int QDOAS_FILL_UINT = 4294967295U;
// const long long QDOAS_FILL_INT64 = -9223372036854775806LL;
// const unsigned long long QDOAS_FILL_UINT64 = 18446744073709551614ULL;

RC netcdf_create_xs_var(const char *varname,vector<int>& dimids,vector<size_t>& chunksizes)
 {
  try
   {
    const int varid = output_group.defVar(varname, dimids, NC_DOUBLE);

    output_group.defVarChunking(varid, NC_CHUNKED, chunksizes.data());
    output_group.defVarDeflate(varid);
    output_group.defVarFletcher32(varid, NC_FLETCHER32);

    output_group.putAttr("_FillValue", QDOAS_FILL_DOUBLE, varid);
   }
  catch (std::runtime_error& e)
   {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
   }

  return 0;
 }
 
RC netcdf_save_wve(double *lambda,int n_wavel)
 {
  const size_t start[] = {0,0};
  const size_t count[] = {1,(size_t)n_wavel};

  try
   {
    output_group.putVar("wavelength", start, count, lambda);
   }
  catch (std::runtime_error& e)
   {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
   }

  return 0;
 }

RC netcdf_save_xs(double *xs,int indexFenoColumn,int n_wavel)
 {
  const size_t start[] = {(size_t)indexFenoColumn,0};
  const size_t count[] = {1,(size_t)n_wavel};

  try
   {
    output_group.putVar("cross_section", start, count, xs);
   }
  catch (std::runtime_error& e)
   {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
   }

  return 0;
 }

RC netcdf_save_convolution(void *engineContext) 
 {
  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  vector<int> dimids,dimids_w;
  vector<size_t>chunksizes,chunksizes_w;
  PRJCT_FILTER *pLFilter,*pHFilter;
  INDEX slitType;
  int id;
  char new_filename[DOAS_MAX_PATH_LEN+1],error_message[DOAS_MAX_PATH_LEN+1];
  FILE *fp;
  
  strcpy(new_filename,pEngineContext->path);
  FILES_BuildFileName(new_filename,pEngineContext->crossFile,FILE_TYPE_NETCDF);
  
  if ((fp=fopen(new_filename,"rb"))!=NULL)
   {
    fclose(fp);
    sprintf(error_message,"%s already exists",new_filename);
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, error_message);
   }
  
  pLFilter=(PRJCT_FILTER *)&pEngineContext->lfilter;
  pHFilter=(PRJCT_FILTER *)&pEngineContext->hfilter;

  try {
    // Open the file in writing mode

    output_file = NetCDFFile(new_filename , NetCDFFile::Mode::write );

    // Create attributes

    time_t curtime = time(NULL);
    output_file.putAttr("created",string(ctime(&curtime) ) );
    output_file.putAttr("description","Cross section convolved with the QDOAS convolution tools");
    output_file.putAttr("title","Cross section convolution");
    output_file.putAttr("high_resolution_cross_section",pEngineContext->crossFile);
    output_file.putAttr("calibration_file",pEngineContext->calibrationFile); 
    output_file.putAttr("shift_applied",to_string(pEngineContext->shift));
    output_file.putAttr("convolution_type",mediateConvolutionTypesStr[pEngineContext->convolutionType]);

    if (pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)
     {
      if (pEngineContext->convolutionType==CONVOLUTION_TYPE_I0_CORRECTION)
       {
        output_file.putAttr("kurucz_file",pEngineContext->kuruczFile);
        output_file.putAttr("concentration_in_I0_(mol/cm**2)",pEngineContext->conc);
       }

      slitType=pEngineContext->slitConv.slitType;
      output_file.putAttr("slit_function_type",XSCONV_slitTypes[slitType]+(pEngineContext->slitConv.slitWveDptFlag)?" wavelength dependent":"");

      if (slitType==SLIT_TYPE_FILE)
       output_file.putAttr("slit_function_file",pEngineContext->slitConv.slitFile);
      else if (pEngineContext->slitConv.slitWveDptFlag)
       {
        if (strlen(pEngineContext->slitConv.slitFile))
         output_file.putAttr("slit_function_file",pEngineContext->slitConv.slitFile);
        if (strlen(pEngineContext->slitConv.slitFile2))
         output_file.putAttr("slit_function_file_2",pEngineContext->slitConv.slitFile2);
        if (strlen(pEngineContext->slitConv.slitFile3))
         output_file.putAttr("slit_function_file_3",pEngineContext->slitConv.slitFile3);          // Super gaussian
       }
      else
       {
        if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
         output_file.putAttr("gaussian_FWHM",to_string(pEngineContext->slitConv.slitParam));
        if (slitType==SLIT_TYPE_ERF)
         output_file.putAttr("boxcar_width",to_string(pEngineContext->slitConv.slitParam2));
        if (slitType==SLIT_TYPE_AGAUSS)
         output_file.putAttr("asymmetry_factor",to_string(pEngineContext->slitConv.slitParam2));
        if (slitType==SLIT_TYPE_SUPERGAUSS)
         {
          output_file.putAttr("exponential_term",to_string(pEngineContext->slitConv.slitParam2));
          output_file.putAttr("asymmetry_factor",to_string(pEngineContext->slitConv.slitParam3));
         }

        if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
         {
          output_file.putAttr("resolution",to_string(pEngineContext->slitConv.slitParam));
          output_file.putAttr("phase",to_string(pEngineContext->slitConv.slitParam2));
         }

        if (slitType==SLIT_TYPE_VOIGT)
         {
          output_file.putAttr("gaussian_FWHM",to_string(pEngineContext->slitConv.slitParam));
          output_file.putAttr("lorentz/Gauss_ratio",to_string(pEngineContext->slitConv.slitParam2));
         }
       }

      if (slitType==SLIT_TYPE_INVPOLY)
       output_file.putAttr("polynomial_degree",to_string(pEngineContext->slitConv.slitParam2));

      slitType=pEngineContext->slitDConv.slitType;

      if ((slitType!=SLIT_TYPE_FILE) || strlen(pEngineContext->slitDConv.slitFile))
       {
        output_file.putAttr("deconvolution_slit_function_type",XSCONV_slitTypes[slitType]+(pEngineContext->slitConv.slitWveDptFlag)?" wavelength dependent":"");

        if (slitType==SLIT_TYPE_FILE)
         output_file.putAttr("deconvolution_slit_function_file ",pEngineContext->slitDConv.slitFile);
        else if (pEngineContext->slitDConv.slitWveDptFlag)
         {
          if (strlen(pEngineContext->slitDConv.slitFile))
           output_file.putAttr("deconvolution_slit_function_file",pEngineContext->slitDConv.slitFile);
          if (strlen(pEngineContext->slitDConv.slitFile2))
           output_file.putAttr("deconvolution_slit_function_file_2",pEngineContext->slitDConv.slitFile2);
          if (strlen(pEngineContext->slitDConv.slitFile3))
           output_file.putAttr("deconvolution_slit_function_file_3",pEngineContext->slitDConv.slitFile3);          // Super gaussian
         }
        else
         {
          if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
           output_file.putAttr("deconvolution_gaussian_FWHM",to_string(pEngineContext->slitDConv.slitParam));
          if (slitType==SLIT_TYPE_ERF)
           output_file.putAttr("deconvolution_oxcar width",to_string(pEngineContext->slitDConv.slitParam2));
          if (slitType==SLIT_TYPE_AGAUSS)
           output_file.putAttr("deconvolution_symmetry factor",to_string(pEngineContext->slitDConv.slitParam2));
          if (slitType==SLIT_TYPE_SUPERGAUSS)
           {
            output_file.putAttr("deconvolution_xponential term",to_string(pEngineContext->slitDConv.slitParam2));
            output_file.putAttr("deconvolution_symmetry factor",to_string(pEngineContext->slitDConv.slitParam3));
           }

          if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
           {
            output_file.putAttr("deconvolution_resolution",to_string(pEngineContext->slitDConv.slitParam));
            output_file.putAttr("deconvolution_phase",to_string(pEngineContext->slitDConv.slitParam2));
           }

          if (slitType==SLIT_TYPE_VOIGT)
           {
            output_file.putAttr("deconvolution_gaussian_FWHM",to_string(pEngineContext->slitDConv.slitParam));
            output_file.putAttr("deconvolution_Lorentz/Gauss_ratio",to_string(pEngineContext->slitDConv.slitParam2));
           }
         }

        if (slitType==SLIT_TYPE_INVPOLY)
         output_file.putAttr("deconvolution_polynomial_degree",to_string(pEngineContext->slitDConv.slitParam2));
       }
     }

    // Low pass filtering

    if (pLFilter->type!=PRJCT_FILTER_TYPE_NONE)
     {
      output_file.putAttr("low_pass_filter",mediateConvolutionFilterTypes[pLFilter->type]);
      output_file.putAttr("low_pass_filter_number_of_iterations",to_string(pLFilter->filterNTimes));

      if (pLFilter->type==PRJCT_FILTER_TYPE_KAISER)
       {
        output_file.putAttr("low_pass_filter_cutoff_frequency",to_string(pLFilter->kaiserCutoff));
        output_file.putAttr("low_pass_filter_pass_band",to_string(pLFilter->kaiserPassBand));
        output_file.putAttr("low_pass_filter_tolerance",to_string(pLFilter->kaiserTolerance));
       }
      else if (pLFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
       output_file.putAttr("low_pass_filter_gaussian_FWHM",to_string(pLFilter->fwhmWidth));
      else if ((pLFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pLFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
       output_file.putAttr("low_pass_filter_width",to_string(pLFilter->filterWidth));
      else if (pLFilter->type==PRJCT_FILTER_TYPE_SG)
       {
        output_file.putAttr("low_pass_filter_width",to_string(pLFilter->filterWidth));
        output_file.putAttr("low_pass_filter_order",to_string(pLFilter->filterOrder));
       }
     }
     
    // High pass filtering

    if (pHFilter->type!=PRJCT_FILTER_TYPE_NONE)
     {
      output_file.putAttr("high_pass_filter",mediateConvolutionFilterTypes[pHFilter->type]);
      output_file.putAttr("high_pass_filter_number_of_iterations",pHFilter->filterNTimes);

      if (pHFilter->type==PRJCT_FILTER_TYPE_KAISER)
       {
        output_file.putAttr("high_pass_filter_cutoff_frequency",to_string(pHFilter->kaiserCutoff));
        output_file.putAttr("high_pass_filter_pass_band",to_string(pHFilter->kaiserPassBand));
        output_file.putAttr("high_pass_filter_tolerance",to_string(pHFilter->kaiserTolerance));
       }
      else if (pHFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
       output_file.putAttr("high_pass_filter_gaussian_FWHM",to_string(pHFilter->fwhmWidth));
      else if ((pHFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pHFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
       output_file.putAttr("high_pass_filter_width",to_string(pHFilter->filterWidth));
      else if (pHFilter->type==PRJCT_FILTER_TYPE_SG)
       {
        output_file.putAttr("high_pass_filter_width",to_string(pHFilter->filterWidth));
        output_file.putAttr("high_pass_filter_order",to_string(pHFilter->filterOrder));
       }
     }

    output_file.putAttr("high_pass_filter_Number of ground pixels",to_string(pEngineContext->n_groundpixel_output));

    // Create dimensions
    
    output_file.defDim("n_wavelength",pEngineContext->xsNew.nl);
    output_file.defDim("dim_y",pEngineContext->n_groundpixel_output);
    output_file.defDim("dim_1",1);
    output_group = output_file.defGroup("QDOAS_CROSS_SECTION_FILE");
    
//     nc_inq_dimid(output_file.groupID(),"n_wavelength",&id);
//     dimids.push_back(id);
//     chunksizes.push_back((size_t)pEngineContext->xsNew.nl);
//     netcdf_create_xs_var("wavelength",dimids,chunksizes);
// 
//     nc_inq_dimid(output_file.groupID(),"dim_y",&id);
//     dimids.push_back(id);
//     chunksizes.push_back((size_t)pEngineContext->n_groundpixel_output);
//     netcdf_create_xs_var("cross_section",dimids,chunksizes);
    

    nc_inq_dimid(output_file.groupID(),"dim_y",&id);
    dimids.push_back(id);
    chunksizes.push_back((size_t)pEngineContext->n_groundpixel_output);
    nc_inq_dimid(output_file.groupID(),"n_wavelength",&id);
    dimids.push_back(id);
    chunksizes.push_back((size_t)pEngineContext->xsNew.nl);
    netcdf_create_xs_var("cross_section",dimids,chunksizes);
    
    nc_inq_dimid(output_file.groupID(),"dim_1",&id);
    dimids_w.push_back(id);
    chunksizes_w.push_back((size_t)1);
    nc_inq_dimid(output_file.groupID(),"n_wavelength",&id);
    dimids_w.push_back(id);
    chunksizes_w.push_back((size_t)pEngineContext->xsNew.nl);
    netcdf_create_xs_var("wavelength",dimids_w,chunksizes_w);
    
    netcdf_save_wve(pEngineContext->xsNew.matrix[0],pEngineContext->xsNew.nl);
    
    if (pEngineContext->n_groundpixel_slit==1)
     for (int i=0;i<pEngineContext->n_groundpixel_output;i++)
       netcdf_save_xs(pEngineContext->xsNew.matrix[1],i,pEngineContext->xsNew.nl);
     
    output_file.close(); 
     
  } catch (std::runtime_error& e) {
    output_file.close();
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
  }
  return 0;
}

RC netcdf_save_ring(void *engineContext) 
 {
  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  vector<int> dimids, dimids_w;
  vector<size_t>chunksizes,chunksizes_w;
  INDEX slitType;
  int id;
  
  char new_filename[DOAS_MAX_PATH_LEN+1],error_message[DOAS_MAX_PATH_LEN+1];
  FILE *fp;
  
  strcpy(new_filename,pEngineContext->path);
  FILES_BuildFileName(new_filename,pEngineContext->crossFile,FILE_TYPE_NETCDF);
  
  if ((fp=fopen(new_filename,"rb"))!=NULL)
   {
    fclose(fp);
    sprintf(error_message,"%s already exists",new_filename);
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, error_message);
   }
  
  try {
    // Open the file in writing mode

    output_file = NetCDFFile(new_filename, NetCDFFile::Mode::write);

    // Create attributes

    time_t curtime = time(NULL);
    output_file.putAttr("created",string(ctime(&curtime) ) );
    output_file.putAttr("description","Ring cross section generated with the QDOAS ring tools");
    output_file.putAttr("title","Ring cross section");
    output_file.putAttr("high_resolution_kurucz_file",pEngineContext->kuruczFile);
    output_file.putAttr("calibration_file",pEngineContext->calibrationFile); 

    slitType=pEngineContext->slitConv.slitType;
    output_file.putAttr("slit_function_type",XSCONV_slitTypes[slitType]+(pEngineContext->slitConv.slitWveDptFlag)?" wavelength dependent":"");

    if (slitType==SLIT_TYPE_FILE)
     output_file.putAttr("slit_function_file",pEngineContext->slitConv.slitFile);
    else if (pEngineContext->slitConv.slitWveDptFlag)
     {
      if (strlen(pEngineContext->slitConv.slitFile))
       output_file.putAttr("slit_function_file",pEngineContext->slitConv.slitFile);
      if (strlen(pEngineContext->slitConv.slitFile2))
       output_file.putAttr("slit_function_file_2",pEngineContext->slitConv.slitFile2);
      if (strlen(pEngineContext->slitConv.slitFile3))
       output_file.putAttr("slit_function_file_3",pEngineContext->slitConv.slitFile3);          // Super gaussian
     }
    else
     {
      if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
       output_file.putAttr("gaussian_FWHM",to_string(pEngineContext->slitConv.slitParam));
      if (slitType==SLIT_TYPE_ERF)
       output_file.putAttr("boxcar_width",to_string(pEngineContext->slitConv.slitParam2));
      if (slitType==SLIT_TYPE_AGAUSS)
       output_file.putAttr("asymmetry_factor",to_string(pEngineContext->slitConv.slitParam2));
      if (slitType==SLIT_TYPE_SUPERGAUSS)
       {
        output_file.putAttr("exponential_term",to_string(pEngineContext->slitConv.slitParam2));
        output_file.putAttr("asymmetry_factor",to_string(pEngineContext->slitConv.slitParam3));
       }

      if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
       {
        output_file.putAttr("resolution",to_string(pEngineContext->slitConv.slitParam));
        output_file.putAttr("phase",to_string(pEngineContext->slitConv.slitParam2));
       }

      if (slitType==SLIT_TYPE_VOIGT)
       {
        output_file.putAttr("gaussian_FWHM",to_string(pEngineContext->slitConv.slitParam));
        output_file.putAttr("lorentz/Gauss_ratio",to_string(pEngineContext->slitConv.slitParam2));
       }

      if (slitType==SLIT_TYPE_INVPOLY)
       output_file.putAttr("polynomial_degree",to_string(pEngineContext->slitConv.slitParam2));
     }
 
    // Create dimensions
    
    output_file.defDim("n_wavelength",pEngineContext->xsNew.nl);
    output_file.defDim("dim_y",pEngineContext->n_groundpixel_output);
    output_file.defDim("dim_1",1);
    output_group = output_file.defGroup("QDOAS_CROSS_SECTION_FILE");
    
    nc_inq_dimid(output_file.groupID(),"dim_y",&id);
    dimids.push_back(id);
    chunksizes.push_back((size_t)pEngineContext->n_groundpixel_output);
    nc_inq_dimid(output_file.groupID(),"n_wavelength",&id);
    dimids.push_back(id);
    chunksizes.push_back((size_t)pEngineContext->xsNew.nl);
    netcdf_create_xs_var("cross_section",dimids,chunksizes);
    
    nc_inq_dimid(output_file.groupID(),"dim_1",&id);
    dimids_w.push_back(id);
    nc_inq_dimid(output_file.groupID(),"n_wavelength",&id);
    dimids_w.push_back(id);
    chunksizes_w.push_back((size_t)pEngineContext->xsNew.nl);
    netcdf_create_xs_var("wavelength",dimids_w,chunksizes_w);
    
    netcdf_save_wve(pEngineContext->xsNew.matrix[0],pEngineContext->xsNew.nl);
    
    if (pEngineContext->n_groundpixel_slit==1)
     for (int i=0;i<pEngineContext->n_groundpixel_output;i++)
       netcdf_save_xs(pEngineContext->xsNew.matrix[1],i,pEngineContext->xsNew.nl);
     
    output_file.close();
     
  } catch (std::runtime_error& e) {
    output_file.close();
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
  }
  return 0;
}

