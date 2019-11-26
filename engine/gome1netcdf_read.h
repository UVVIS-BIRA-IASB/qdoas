
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  READ GOME ERS FILES IN NETCDF FORMAT
//  Name of module    :  GOME1NETCDF_READ.H
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

#ifndef GOME1NETCDF_READ_H
#define GOME1NETCDF_READ_H

#include "doas.h"

#ifdef __cplusplus
extern "C" {
#endif

  RC   GOME1NETCDF_Set(ENGINE_CONTEXT *pEngineContext);
  RC   GOME1NETCDF_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex);
  void GOME1NETCDF_Cleanup(void);
  int  GOME1NETCDF_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);
  int GOME1NETCDF_InitRef(const char *reference_filename, int *n_wavel_temp, ENGINE_CONTEXT *pEngineContext);
  RC   GOME1NETCDF_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

#ifdef __cplusplus
}
#endif

#endif
