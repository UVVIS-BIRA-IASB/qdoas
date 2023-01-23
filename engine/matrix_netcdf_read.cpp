
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
//! \copyright QDOAS is distributed under GNU General Public License
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
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or (at
//  your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
// FUNCTION      MATRIX_netcdf_LoadXS
// -----------------------------------------------------------------------------
//!
//! \fn           RC MATRIX_netcdf_LoadXS(const char *fileName,MATRIX_OBJECT *pMatrix,int nl,int nc,double xmin,double xmax,int allocateDeriv2,int reverseFlag,bool *use_row,const char *callingFunction)
//! \details      Load a cross section from a netCDF file
//! \param   [in] fileName           the name of the file to load
//! \param   [in] groupName          the name of the group to load
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

RC MATRIX_netcdf_LoadXS(const char *fileName,MATRIX_OBJECT *pMatrix,
                        int nl,int nc,double xmin,double xmax,
                        int allocateDeriv2,int reverseFlag, bool *use_row,const char *callingFunction) 
 {
  // Declarations

  char     fullPath[MAX_ITEM_TEXT_LEN];                                         //!< \details the complete file name to load
  int      imin,imax;                                                           //!< \details indexes ranges for xmin,xmax
  INDEX    i,j;                                                                 //!< \details indexes for browsing lines and columns in matrix
  double **matrix,**deriv2,                                                     //!< \details resp. pointers to the matrix to load and to the second derivatives
           xMin,xMax,                                                           //!< \details define the range of values to load for the first column of the matrix
           tempValue;                                                           //!< \details a value of the matrix to load
  RC       rc;                                                                  //!< \details return code
  double firstCalib;

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  FILES_RebuildFileName(fullPath,fileName,1);                                   // build the complete path and file name

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Print("File to load : %s\n",fullPath);
  #endif

  firstCalib=(double)0.;
  rc=ERROR_ID_NO;

  xMin=min(xmin,xmax);
  xMax=max(xmin,xmax);

  // Reset matrix

  MATRIX_Free(pMatrix, __func__);
  
  // Try to open the file and load metadata

  try
   {
    NetCDFFile current_file;                                                    //!< \details Pointer to the current netCDF file
    NetCDFGroup root_group;                                                     //!< \details pointer to the first group
    vector<float> wve;                                                          //!< \details vector for wavelengths 
    vector<double> spe;                                                         //!< \details vector for xs at a specific row
    int n_wavelength;                                                           //!< \details number of wavelengths
    int n_rows;                                                                 //!< \details number of rows (for satellites)
    double *dwve;
    size_t start[2],count[2];
    
    dwve=NULL;
    current_file = NetCDFFile(fullPath,NC_NOWRITE);                             // open file
    
    root_group = current_file.getGroup("QDOAS_CROSS_SECTION_FILE");  // go to the root
    
    n_wavelength=root_group.dimLen("n_wavelength");      
    n_rows=root_group.dimLen("dim_y");   
    
    if (((nl!=0) && (n_wavelength!=nl)) || ((nc!=0) && (n_rows+1!=nc)))
     rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_BAD_LENGTH,fullPath);
    else if ((dwve=(double *)MEMORY_AllocDVector((char *)__func__,"dwve",0,n_wavelength-1))==NULL)
      rc=ERROR_ID_ALLOC;
    else
     {
      start[0] = start[1]=0;   
      count[0] = 1;                                                  
      count[1] = n_wavelength;
       
      root_group.getVar("wavelength",start,count,2,(float)0.,wve); 
      for (i=0;i<n_wavelength;i++)
       dwve[i]=(double)wve[i];
      
      if (fabs(xmax-xmin)<EPSILON)
       {
        imin=0;
        imax=n_wavelength-1;
       }
      else
       {
        imin=FNPixel((double *)dwve,xmin,n_wavelength,PIXEL_CLOSEST);
        imax=FNPixel((double *)dwve,xmax,n_wavelength,PIXEL_CLOSEST);
       }
      
      nl=(imax-imin+1); 
      
      start[1]=imin;
      
      if (!(rc=MATRIX_Allocate(pMatrix,nl,n_rows+1,0,0,allocateDeriv2,callingFunction)))
       {
        matrix=pMatrix->matrix;
        deriv2=pMatrix->deriv2;
        
        memcpy(matrix[0],&dwve[imin],sizeof(double)*nl);
        
        for (j=1;j<=n_rows;j++)

          if ((use_row==NULL) || use_row[j-1])
           {
            start[0] = j-1;                                            
            root_group.getVar("cross_section",start,count,2,(double)0.,spe);   
            memcpy(matrix[j],spe.data(),sizeof(double)*nl);
           }

         
        // Flip up/down the matrix

        if (reverseFlag && (matrix[0][0]>matrix[0][1]))
         for (i=0; i<nl/2 ; ++i)
          for (j=0; j<=n_rows; ++j)                                              // index 0 includes wavelengths
            if ((j==0) || (use_row==NULL) || use_row[j-1])
             {
              tempValue=matrix[j][i];
              matrix[j][i]=matrix[j][nl-1-i];
              matrix[j][nl-1-i]=tempValue;
             }
           
        // Calculate second derivatives of the columns of the matrix for future interpolation

        if (allocateDeriv2)
         for (j=1; (j<=n_rows) && !rc; ++j)
           if ((use_row==NULL) || use_row[j-1])
            rc=SPLINE_Deriv2(((double *)matrix[0]),
                             ((double *)matrix[j]),
                             ((double *)deriv2[j]),
                             pMatrix->nl,
                             callingFunction);
       }
     }

    current_file.close();
    
    if (dwve!=NULL)
     MEMORY_ReleaseDVector("MATRIX_netcdf_LoadXS ","dwve",dwve,0);
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

