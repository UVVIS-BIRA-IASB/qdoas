

//  ----------------------------------------------------------------------------
//! \addtogroup general
//! @{
//!
//! \file      matrix_netcdf_read.cpp
//! \brief
//! \details
//! \details
//! \authors   Caroline FAYT (qdoas@aeronomie.be)
//! \date      12/08/2020 (creation date)
//! \bug
//! \todo
//!
//! @}
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//
//  ----------------------------------------------------------------------------
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//        BIRA-IASB
//        Belgian Institute for Space Aeronomy
//        Ringlaan 3 Avenue Circulaire
//        1180     UCCLE
//        BELGIUM
//        qdoas@aeronomie.be
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string>
#include <vector>
#include <array>
#include <set>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include <cassert>
#include <cmath>

#include "netcdfwrapper.h"
#include "matrix_netcdf_read.h"

extern "C" {
#include "comdefs.h"
#include "analyse.h"
 #include "matrix.h"
#include "spline.h"
#include "vector.h"
#include "winfiles.h"
}

using std::string;
using std::vector;
using std::set;
using std::min;
using std::max;

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_netcdf_Load
// -----------------------------------------------------------------------------
//!
//! \fn           RC MATRIX_netcdf_LoadXS(const char *fileName,MATRIX_OBJECT *pMatrix,int nl,int nc,double xmin,double xmax,int allocateDeriv2,int reverseFlag,bool *use_row,const char *callingFunction)
//! \details      Load a cross section from a netCDF file
//! \param   [in] fileName           the name of the file to load
//! \param   [in] pMatrix            pointer to the structure that will receive the data loaded from the input file
//! \param   [in] nl                 the number of lines of the matrix to load (0 to retrieve the dimensions automatically from the file)
//! \param   [in] nc                 the number of columns of the matrix to load (0 to retrieve the dimensions automatically from the file
//! \param   [in] xmin               the lower limit of the range of wavelength values to load for the first column of the matrix
//! \param   [in] xmax               the upper limit of the range of wavelength values to load for the first column of the matrix
//! \param   [in] allocateDeriv2     1 to allocate buffers for the second derivatives
//! \param   [in] reverseFlag        1 to reverse the matrix (flip up/down)
//! \param   [in] use_row            vector of flags for rows to use (specific for satellites)
//! \param   [in] callingFunction    the name of the calling function
//! \return  ERROR_ID_NO on success
// -----------------------------------------------------------------------------

RC MATRIX_netcdf_Load(const char *fileName,MATRIX_OBJECT *pMatrix,
                      int nl,int nc,double xmin,double xmax,
                      int allocateDeriv2,int reverseFlag, bool *use_row,const char *callingFunction)
 {
  // Declarations

  char     fullPath[MAX_ITEM_TEXT_LEN];                                         //!< \details the complete file name to load
  int      imin,imax;                                                           //!< \details indexes ranges for xmin,xmax
  double **matrix,**deriv2,                                                     //!< \details resp. pointers to the matrix to load and to the second derivatives
           tempValue;                                                           //!< \details a value of the matrix to load
  RC       rc;                                                                  //!< \details return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  FILES_RebuildFileName(fullPath,fileName,1);                                   // build the complete path and file name

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Print("File to load : %s\n",fullPath);
  #endif

  rc=ERROR_ID_NO;

  // Reset matrix

  MATRIX_Free(pMatrix, __func__);

  // Try to open the file and load metadata

  try
   {
    int n_wavelength;                                                           //!< \details number of wavelengths
    int n_rows;                                                                 //!< \details number of rows (for satellites)
    double *dwve = NULL;

    NetCDFFile current_file(fullPath, NetCDFFile::Mode::read, 180 * 1024 * 1024);  // open file, use large cache to handle big cross sections.
    bool have_qdoas_matrix = false;
    // We can read files which have a "QDOAS_CROSS_SECTION_FILE" group, or files with a generic "qdoas_matrix" variable.
    if (current_file.hasVar("qdoas_matrix")) {
      have_qdoas_matrix = true;
    }
    NetCDFGroup root_group = have_qdoas_matrix ? current_file : current_file.getGroup("QDOAS_CROSS_SECTION_FILE");  // go to the root

    n_wavelength=root_group.dimLen("n_wavelength");
    n_rows=root_group.dimLen("dim_y");

    if (((nl!=0) && (n_wavelength!=nl)) || ((nc!=0) && (n_rows+1!=nc)))
     rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_LENGTH,fullPath);
    else if ((dwve=(double *)MEMORY_AllocDVector(__func__,"dwve",0,n_wavelength-1))==NULL)
      rc=ERROR_ID_ALLOC;
    else
     {
      auto wve = root_group.getVar<double>("wavelength");
      for (int i=0; i<n_wavelength; i++) {
        dwve[i]=wve[i];
      }

      if (fabs(xmax-xmin)<EPSILON) {
        imin=0;
        imax=n_wavelength-1;
      } else {
        if ((xmin < wve[0]) || (xmax > wve[n_wavelength - 1])) {
          std::stringstream ss;
          ss << "Requested wavelength range "
             << xmin << " - " << xmax
             << " is outside the domain of " << fileName;
          throw std::runtime_error(ss.str());
        }

        imin=FNPixel((double *)dwve,xmin,n_wavelength,PIXEL_CLOSEST);
        imax=FNPixel((double *)dwve,xmax,n_wavelength,PIXEL_CLOSEST);
      }

      nl=(imax-imin+1);

      if (!(rc=MATRIX_Allocate(pMatrix,nl,n_rows+1,0,0,allocateDeriv2,callingFunction)))
       {
        matrix=pMatrix->matrix;
        deriv2=pMatrix->deriv2;

        memcpy(matrix[0],&dwve[imin],sizeof(double)*nl);

        for (size_t j=1; j <= static_cast<size_t>(n_rows); ++j) {
          if ((use_row==NULL) || use_row[j-1]) {
            const size_t start[] = {j-1, static_cast<size_t>(imin)};
            const size_t count[] = {1, static_cast<size_t>(nl)};
            root_group.getVar(have_qdoas_matrix ? "qdoas_matrix" : "cross_section", start, count, matrix[j]);
          }
        }
        // Flip up/down the matrix

        if (reverseFlag && (matrix[0][0]>matrix[0][1]))
         for (int i=0; i<nl/2 ; ++i)
          for (int j=0; j<=n_rows; ++j)                                              // index 0 includes wavelengths
            if ((j==0) || (use_row==NULL) || use_row[j-1])
             {
              tempValue=matrix[j][i];
              matrix[j][i]=matrix[j][nl-1-i];
              matrix[j][nl-1-i]=tempValue;
             }

        // Calculate second derivatives of the columns of the matrix for future interpolation

        if (allocateDeriv2)
         for (int j=1; (j<=n_rows) && !rc; ++j)
           if ((use_row==NULL) || use_row[j-1])
            rc=SPLINE_Deriv2(((double *)matrix[0]),
                             ((double *)matrix[j]),
                             ((double *)deriv2[j]),
                             pMatrix->nl,
                             callingFunction);
       }
     }
    if (dwve!=NULL)
     MEMORY_ReleaseDVector(__func__, "dwve", dwve, 0);
   }
  catch (std::runtime_error& e)
   {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());  // in case of error, capture the message
   }

  // Release allocated buffers

  if (rc)
   MATRIX_Free(pMatrix,__func__);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  if (!rc)
   {
    int basel=0,basec=0;
    DEBUG_PrintVar("the matrix",pMatrix->matrix,basel,basel+nl-1,basec,basec+nc-1,NULL);
    if (allocateDeriv2)
     DEBUG_PrintVar("the 2nd derivatives",pMatrix->deriv2,basel,basel+nl-1,basec+1,basec+nc-1,NULL);
   }
  DEBUG_FunctionStop(__func__, rc);
  #endif

  // Return

  return rc;
 }
