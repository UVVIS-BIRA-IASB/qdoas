#ifndef OUTPUT_NETCDF_H
#define OUTPUT_NETCDF_H

#include "doas.h"

#include "output_common.h"


#ifdef __cplusplus
extern "C" {
#endif

  RC netcdf_open(const ENGINE_CONTEXT *pEngineContext, const char *filename);

  void netcdf_close_file(void);

  RC netcdf_write_analysis_data(const bool selected_records[], int num_records, const OUTPUT_INFO *outputRecords);

  RC netcdf_allow_file(const char *filename, const PRJCT_RESULTS *results);
  RC netcdf_save_calib(double *lambda,double *reference,int indexFenoColumn,int n_wavel);
  RC netcdf_open_calib(const ENGINE_CONTEXT *pEngineContext, const char *filename,int col_dim,int spectral_dim);
  void netcdf_close_calib(void);

#ifdef __cplusplus
}
#endif


#endif
