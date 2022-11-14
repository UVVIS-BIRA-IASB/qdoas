/*! \file output_formats.h
  \brief Enumeration and file extensions for different output formats.  Shared between interface code and output module.
 */

#ifndef OUTPUT_FORMATS_H
#define OUTPUT_FORMATS_H

#define HDFEOS_OBJ_LEN_MAX 256 // taken from HE5_HdfEosDef.h
#define OUTPUT_HDF5_DEFAULT_GROUP "QDOAS Results"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

  /*! \brief Supported output formats.*/
  enum output_format {
    UNDEFINED=-1,
    ASCII,
    HDFEOS5,
    NETCDF,
    LAST_OUTPUT_FORMAT = NETCDF
  };

  enum output_format output_get_format(const char *fileext);
  extern const char *output_file_extensions[]; // defined in output.c

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
