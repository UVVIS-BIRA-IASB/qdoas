#ifndef OUTPUT_NETCDF_H
#define OUTPUT_NETCDF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "doas.h"
#include "output_common.h"

  RC netcdf_open(const ENGINE_CONTEXT *pEngineContext, const char *filename,int num_records);

  void netcdf_close_file(void);

  RC netcdf_write_analysis_data(const bool selected_records[], int num_records, const OUTPUT_INFO *outputRecords);

  RC netcdf_allow_file(const char *filename, const PRJCT_RESULTS *results);
  RC netcdf_save_calib(double *lambda,double *reference,int indexFenoColumn,int n_wavel);
  RC netcdf_open_calib(const ENGINE_CONTEXT *pEngineContext, const char *filename,int col_dim,int spectral_dim);
  void netcdf_close_calib(void);
  
  RC netcdf_save_xs(double *lambda,double *reference,int indexFenoColumn,int n_wavel);
  RC netcdf_open_xs(void *pEngineContext, const char *filename,int col_dim,int spectral_dim);
  void netcdf_close_xs(void);

#ifdef __cplusplus
}
#endif


#endif
