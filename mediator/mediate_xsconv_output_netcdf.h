#ifndef CONVOLUTION_OUTPUT_NETCDF_H
#define CONVOLUTION_OUTPUT_NETCDF_H

#include "engine_xsconv.h" 


#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif
 
  RC netcdf_save_convolution(void *pEngineContext);
  RC netcdf_save_ring(void *pEngineContext);
  
#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
