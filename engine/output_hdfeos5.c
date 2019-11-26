/*! \file output_hdfeos5.c \brief Functions for HDF-EOS5 output.

  For data from input files with spectra in a (along-track) x
  (cross-track) matrix (e.g. OMI), we write the output in a similar
  matrix for easier comparison.  For input formats in linear format
  (i.e. GOME-2/Sciamachy/...), output results are also written in this
  format.*/

#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>

#include "HE5_HdfEosDef.h"

#include "engine_context.h"
#include "omi_read.h"
#include "output_common.h"

enum fieldtype { DATA, GEO};

static enum fieldtype get_fieldtype(enum _prjctResults output_field);

static RC create_dimensions(void);
static RC write_calibration_data(void);
static RC write_global_attrs(const ENGINE_CONTEXT *pEngineContext);
static RC write_automatic_reference_info(const ENGINE_CONTEXT *pEngineContext);
static RC create_analysis_data_fields(void);
static void write_to_buffer(void *datbuf, const struct output_field *thefield, int record, const OUTPUT_INFO *recordinfo);
static hid_t get_hdfeos5_type(enum output_datatype datatype);
static void get_hdfeos5_dimension_calibration(char *dimension, const struct output_field *thefield);
static void get_hdfeos5_dimension_analysis(char *dimension, const struct output_field *thefield);
static void get_chunkdims(hsize_t chunkdims[], const struct output_field *thefield);
static void get_edge_calibration(hsize_t edge[], const struct output_field *thefield);
static void get_edge_analysis(hsize_t edge[], const struct output_field *thefield);
static int get_rank(const struct output_field *thefield);
static void replace_chars(char *fieldname);

/*! \brief reference to current output file.*/
static hid_t output_file;
/*! \brief reference to current swath.*/
static hid_t swath_id;

/*! \brief number of detector rows in the swath */
static size_t nXtrack;
/*! \brief number of measurements in the swath */
static size_t nTimes;
/*! \brief length of calibration data */
static size_t nCalibWindows;

/* todo:
 *  - fill values/default values for fields
 */

RC hdfeos5_open(const ENGINE_CONTEXT *pEngineContext, const char *filename) {
  const char *ext=output_file_extensions[HDFEOS5];
  char filename_ext[1 + strlen(filename) + strlen(ext)];
  strcpy(filename_ext,filename);
  strcat(filename_ext, ext);
  RC rc = ERROR_ID_NO;

  const char *swath_name = pEngineContext->project.asciiResults.swath_name;

  // initialize swath dimensions:
  nXtrack = ANALYSE_swathSize;
  nTimes = pEngineContext->recordNumber / nXtrack;
  // find first used row and get number of calibration windows
  for(int firstrow = 0; firstrow<ANALYSE_swathSize; firstrow++) {
    if ( pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI ||
         pEngineContext->project.instrumental.use_row[firstrow] ) {
      nCalibWindows = KURUCZ_buffers[firstrow].Nb_Win;
      break;
    }
  }

  // test if file exists; if it already exists, size should be 0.
  rc = hdfeos5_allow_file(filename_ext);
  if (rc == ERROR_ID_NO) {
    // new file, go ahead
    hid_t result_open = HE5_SWopen(filename_ext, H5F_ACC_TRUNC);
    if(result_open != FAIL) {
      output_file = result_open;
    } else {
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_OPEN, filename_ext);
      goto cleanup;
    }
    hid_t result_create = HE5_SWcreate(output_file, swath_name);
    if (result_create != FAIL) {
      swath_id = result_create;
    } else {
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_SWATH, swath_name, filename_ext);
      goto cleanup;
    }
    rc = create_dimensions();
    if (rc == ERROR_ID_NO)
      rc = write_global_attrs(pEngineContext);
    if (rc == ERROR_ID_NO)
      rc = write_automatic_reference_info(pEngineContext);
    if (rc == ERROR_ID_NO)
      rc = write_calibration_data();
    if (rc == ERROR_ID_NO)
      rc = create_analysis_data_fields();
  }

 cleanup:
  if(rc != ERROR_ID_NO) {
    if(swath_id) {
      HE5_SWdetach(swath_id);
      swath_id = 0;
    }
    if(output_file) {
      HE5_SWclose(output_file);
      output_file = 0;
    }
  }
  return rc;
}

void hdfeos5_close_file(void) {
  assert(output_file); // function should only be called when a file is open
  assert(swath_id); // and when a swath is attached
  HE5_SWdetach(swath_id);
  swath_id = 0;
  HE5_SWclose(output_file);
  output_file = 0;
}

// test if a file of non-zero size already exists at a given location
// if a file already exists, we will not write to this location
RC hdfeos5_allow_file(const char *filename) {
  RC rc = ERROR_ID_NO;
  struct stat stat_output;
  int statrc = stat(filename, &stat_output);
  // if stat returns -1, we assume the file didn't exist, which is ok
  // if stat is successfull, current size of file should be 0
  if (statrc == 0 && stat_output.st_size != 0) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_FILE_EXISTS, filename);
  }
  return rc;
}

/*! \brief Create the required dimensions in the current swath.*/
RC create_dimensions(void) {
  struct dim {
    const char *dimname;
    hsize_t dimsize;
  };

  struct dim swathdims[] = { { "nXtrack", nXtrack},
                             { "nCalibWindows", nCalibWindows},
                             { "nTimes", nTimes },
                             { "date", 3}, // year, month, day
                             { "time", 3}, // hour, min, secs
                             { "datetime", 7}, // hour, min, secs, milliseconds
                             // allow for fields with different number of columns
                             // example: fields such as
                             // azimuth/longitude/... can contain 3-4-5 values
                             { "2", 2}, { "3", 3},
                             { "4", 4}, { "5", 5}, { "6", 6},
                             { "7", 7}, { "8", 8}, { "9", 9} };

  for(size_t i=0; i< (sizeof(swathdims)/sizeof(swathdims[0])); i++) {
    if(swathdims[i].dimsize > 0) {
      // for example, nCalibWindows may be 0 when no calibration is used
      herr_t result = HE5_SWdefdim(swath_id, (char *) swathdims[i].dimname, swathdims[i].dimsize);
      if(result) {
        return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_DEFDIM, swathdims[i].dimname, swathdims[i].dimsize);
      }
    }
  }
  return ERROR_ID_NO; // if we get here, all dimensions were created ok
}

RC write_global_attrs(const ENGINE_CONTEXT *pEngineContext) {
  // Qdoas info
  const char *attr_qdoas = "Qdoas";
  const char *descriptionformat =
    "File created using Qdoas (%s), \n"
    "Belgian Institute for Space Aeronomy (BIRA-IASB)\n"
    "http://uv-vis.bira.be/software/QDOAS";
  hsize_t attrlength = strlen(cQdoasVersionString) + strlen(descriptionformat) + 1;
  char descriptionstring[attrlength];
  sprintf(descriptionstring, descriptionformat, cQdoasVersionString);
  herr_t result = HE5_EHwriteglbattr(output_file, attr_qdoas, HE5T_CHARSTRING, &attrlength, descriptionstring);
  if (result)
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEATTR, attr_qdoas);

  // creation time
  time_t curtime = time(NULL);
  char *time_string = ctime(&curtime);
  const char *attr_time = "CreationTime";
  attrlength = strlen(time_string) + 1;
  result = HE5_EHwriteglbattr(output_file, attr_time, HE5T_CHARSTRING, &attrlength, time_string);
  if (result)
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEATTR, attr_time);

  // input file
  const char *attr_input_file = "InputFile";
  const char *input_dir = strrchr(pEngineContext->fileInfo.fileName,PATH_SEP);
  const char *input_file = input_dir
    ? input_dir + 1
    : pEngineContext->fileInfo.fileName;
  attrlength = strlen(input_file) + 1;
  result = HE5_EHwriteglbattr(output_file, attr_input_file, HE5T_CHARSTRING, &attrlength, (char *)input_file); // cast away const to avoid compiler warning HDF-EOS5
  if (result)
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEATTR, attr_input_file);
  return ERROR_ID_NO;
}

RC write_automatic_reference_info(const ENGINE_CONTEXT *pEngineContext) {
  int format = pEngineContext->project.instrumental.readOutFormat;

  if ( pEngineContext->project.asciiResults.referenceFlag
       && pEngineContext->analysisRef.refAuto ) {
    switch(format) {
    case PRJCT_INSTR_FORMAT_OMI:
    case PRJCT_INSTR_FORMAT_TROPOMI: {
      const char format[] = "%s - row %d automatic reference";
      for(int analysiswindow = 0; analysiswindow < NFeno; analysiswindow++ ){
        for(int row=0; row< ANALYSE_swathSize; ++row) {
          FENO *pTabFeno = &TabFeno[row][analysiswindow];
          if (!pTabFeno->hidden
              && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
              && pTabFeno->ref_description != NULL) {
            size_t length = strlen(pTabFeno->windowName) + sizeof(format) + 10;
            char attrname[length];
            sprintf(attrname, format, pTabFeno->windowName, row);
            replace_chars(attrname);
            size_t attrlength = strlen(pTabFeno->ref_description) + 1;
            herr_t result = HE5_SWwriteattr(swath_id, attrname, HE5T_CHARSTRING, (hsize_t*) &attrlength, pTabFeno->ref_description);
            if(result)
              return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEATTR, attrname);
          }
        }
      }
      return ERROR_ID_NO;
    }
      break;
    default:
      break;
    }
  }
  return ERROR_ID_NO;
}

herr_t set_fill_value(hid_t swath_id, const char *fieldname, const struct output_field *calibfield) {
  const void *fill;
  hid_t he5type;
  switch(calibfield->memory_type) {
  case OUTPUT_STRING:
    // HDF-EOS5 currently does not support fill values for string variables (!?)
    return 0;
    break;
  case OUTPUT_SHORT:
    he5type = HE5T_NATIVE_SHORT;
    fill = &QDOAS_FILL_SHORT;
    break;
  case OUTPUT_USHORT:
    he5type = HE5T_NATIVE_USHORT;
    fill = &QDOAS_FILL_USHORT;
    break;
  case OUTPUT_FLOAT:
    he5type = HE5T_NATIVE_FLOAT;
    fill = &QDOAS_FILL_FLOAT;
    break;
  case OUTPUT_INT:
    he5type = HE5T_NATIVE_INT;
    fill = &QDOAS_FILL_INT;
    break;
  case OUTPUT_DOUBLE:
    he5type = HE5T_NATIVE_DOUBLE;
    fill = &QDOAS_FILL_DOUBLE;
    break;
  case OUTPUT_TIME:
    he5type = HE5T_NATIVE_UCHAR; // time is stored using 3 unsigned chars
    fill = &QDOAS_FILL_UBYTE;
    break;
  case OUTPUT_DATE:
  case OUTPUT_DATETIME:
    he5type = HE5T_NATIVE_INT; // date/datetime are stored as a set of integers
    fill = &QDOAS_FILL_INT;
    break;
  default:
  assert(false); // should never get here
  return 0;
  }
  // cast to (char *) and (void *) to avoid warnings about constness
  return HE5_SWsetfillvalue(swath_id, (char *)fieldname, he5type, (void *)fill);
}

/*! \brief Write calibration data to the current swath.*/
RC write_calibration_data(void) {
  static const char *calib_prefix = "Calib."; // create fields named
  // "Calib.windowname.RMS", "Calib.windowname.Shift", etc

  int first_row = output_data_calib[0].index_row;

  for(unsigned int i=0; i<calib_num_fields; i++) {
    struct output_field calibfield = output_data_calib[i];
    char he5fieldname[strlen(calib_prefix) + strlen(calibfield.windowname) + 1 + strlen(calibfield.fieldname) + 1];
    sprintf(he5fieldname, "%s%s.%s", calib_prefix, calibfield.windowname, calibfield.fieldname);
    replace_chars(he5fieldname);

    // define data fields
    /* when crosstrack dimension > 1, output_data_calib contains a
     * copy of each calibration field for each detector row.  We want
     * to define one HDF-EOS5 datafield with n_crosstrack columns for
     * each calibration field.  Therefore we only call
     * HE5_SWdefdatafield for the first row.
     */
    if(calibfield.index_row == first_row) {
      char calib_dimensions[HE5_HDFE_DIMBUFSIZE];
      get_hdfeos5_dimension_calibration(calib_dimensions, &calibfield);

      int chunkrank = get_rank(&calibfield) ;
      int compparm[5] = {9, 0, 0, 0, 0}; // in practice, only compparm[0] is used?
      // chunking and compression
      hsize_t chunkdims[HE5_DTSETRANKMAX] = {nCalibWindows, nXtrack}; // for calibration: 1 chunk (nCalibWindows x nXtrack)
      get_chunkdims(&chunkdims[2], &calibfield); // other chunk dimensions determined based on field properties
      herr_t result = HE5_SWdefchunk(swath_id, chunkrank, chunkdims);
      result |= HE5_SWdefcomp(swath_id, HE5_HDFE_COMP_SHUF_DEFLATE, compparm);

      hid_t dtype = get_hdfeos5_type(calibfield.memory_type);
      enum fieldtype type = get_fieldtype(calibfield.resulttype);
      result |= set_fill_value(swath_id, he5fieldname, &calibfield);
      result |= (type == DATA)
        ? HE5_SWdefdatafield(swath_id, he5fieldname, calib_dimensions, NULL, dtype, 0)
        : HE5_SWdefgeofield(swath_id, he5fieldname, calib_dimensions, NULL, dtype, 0);
      if(result)
        return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_DEFFIELD, he5fieldname);
    }

    // write calibration data
    if (calibfield.data) {
      int index_row = calibfield.index_row;
      hssize_t start[HE5_DTSETRANKMAX] = {0, index_row};
      hsize_t edge[HE5_DTSETRANKMAX];
      get_edge_calibration(edge, &calibfield);
      herr_t result = HE5_SWwritefield(swath_id, he5fieldname, start, NULL, edge, calibfield.data);
      if(result)
        return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEFIELD, he5fieldname);
    }
  }

  return ERROR_ID_NO;
}

/*! \brief Create the HDF-EOS5 data fields corresponding to the
    registered output fields.*/
RC create_analysis_data_fields(void) {
  char field_dimension[HE5_HDFE_DIMBUFSIZE];
  for(unsigned int i=0; i<output_num_fields; ++i) {
    struct output_field thefield = output_data_analysis[i];

    size_t length = 1 + strlen(thefield.fieldname)
      + (thefield.windowname ? 1 + strlen(thefield.windowname) : 0);
    char he5_fieldname[length];
    if (thefield.windowname)
      sprintf(he5_fieldname, "%s.%s", thefield.windowname, thefield.fieldname);
    else
      sprintf(he5_fieldname, "%s", thefield.fieldname);
    replace_chars(he5_fieldname);

    get_hdfeos5_dimension_analysis(field_dimension, &thefield);
    hid_t dtype = get_hdfeos5_type(thefield.memory_type);

    // set chunking and compression
    int chunkrank = get_rank(&thefield) ;
    int compparm[5] = {9, 0, 0, 0, 0}; // in practice, only compparm[0] is used?
    hsize_t chunkdims[HE5_DTSETRANKMAX] = {0};
    int index_dim = 0;
    chunkdims[index_dim++] = (nTimes > 100) ? 100 : nTimes; // chunks of length 100 along track (settings used for OMI)
    if(nXtrack > 1)
      chunkdims[index_dim++] = (nXtrack > 4) ? nXtrack/4  : 1; // about 4-5 chunks across track
    get_chunkdims(&chunkdims[index_dim], &thefield); // other chunk dimensions determined based on field properties
    HE5_SWdefchunk(swath_id, chunkrank, chunkdims);
    HE5_SWdefcomp(swath_id, HE5_HDFE_COMP_SHUF_DEFLATE, compparm);

    herr_t result = set_fill_value(swath_id, he5_fieldname, &thefield);
    if(result)
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_SETFILL, he5_fieldname);

    enum fieldtype type = get_fieldtype(thefield.resulttype);
    result = (type == DATA)
      ? HE5_SWdefdatafield(swath_id, he5_fieldname, field_dimension, NULL, dtype, 0)
      : HE5_SWdefgeofield(swath_id, he5_fieldname, field_dimension, NULL, dtype, 0);
    if(result)
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_DEFFIELD, he5_fieldname);
    if(thefield.attributes) {
      for(int i=0; i<thefield.num_attributes; i++) {
        hsize_t count[1] = {strlen(thefield.attributes[i].value)+1};
        result = HE5_SWwritelocattr(swath_id, he5_fieldname, thefield.attributes[i].label, HE5T_CHARSTRING, count, (char *)thefield.attributes[i].value);
        if (result)
          return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEATTR, thefield.attributes[i].label);
      }
    }
  }
  return ERROR_ID_NO;
}

/*! \brief Allocate a buffer to hold output data for a HDF-EOS5
 *  variable, and initialize it to the fill value for that variable
 */
static RC initialize_data_buffer(void **datbuf, size_t len, const char *he5_fieldname, enum output_datatype datatype) {

  herr_t result;

  switch (datatype) {
  case OUTPUT_STRING:
    *datbuf = calloc(len, sizeof(char*));
    return 0;
    break;
  case OUTPUT_SHORT: {
    short fill_short;
    short *shortbuf = malloc(len * sizeof(*shortbuf));
    *datbuf = shortbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_short);
    for (unsigned int i=0; i< len; ++i) {
      shortbuf[i] = fill_short;
    }
  }
    break;
  case OUTPUT_USHORT: {
    unsigned short fill_ushort;
    unsigned short *ushortbuf = malloc(len * sizeof(*ushortbuf));
    *datbuf = ushortbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_ushort);
    for (unsigned int i=0; i< len; ++i) {
      ushortbuf[i] = fill_ushort;
    }
  }
    break;
  case OUTPUT_FLOAT: {
    float fill_float;
    float *floatbuf = malloc(len * sizeof(*floatbuf));
    *datbuf = floatbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_float);
    for (unsigned int i=0; i< len; ++i) {
      floatbuf[i] = fill_float;
    }
  }
    break;
  case OUTPUT_INT: {
    int fill_int;
    int *intbuf = malloc(len * sizeof(*intbuf));
    *datbuf = intbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_int);
    for (unsigned int i=0; i< len; ++i) {
      intbuf[i] = fill_int;
    }
  }
    break;
  case OUTPUT_DOUBLE: {
    double fill_double;
    double *doublebuf = malloc(len * sizeof(*doublebuf));
    *datbuf = doublebuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_double);
    for (unsigned int i=0; i< len; ++i) {
      doublebuf[i] = fill_double;
    }
  }
    break;
    // time is stored using 3 unsigned chars
  case OUTPUT_TIME: {
    unsigned char fill_uchar;
    unsigned char *ucharbuf = malloc(3 * len * sizeof(*ucharbuf));
    *datbuf = ucharbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_uchar);
    for (unsigned int i=0; i< 3*len; ++i) {
      ucharbuf[i] = fill_uchar;
    }
  }
    break;
    // date/datetime are stored as a set of integers:
  case OUTPUT_DATE: {
    int fill_int;
    int *intbuf = malloc(3 * len * sizeof(*intbuf));
    *datbuf = intbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_int);
    for (unsigned int i=0; i< 3*len; ++i) {
      intbuf[i] = fill_int;
    }
  }
    break;
  case OUTPUT_DATETIME: {
    int fill_int;
    int *intbuf = malloc(7 * len * sizeof(*intbuf));
    *datbuf = intbuf;
    result = HE5_SWgetfillvalue(swath_id, he5_fieldname, &fill_int);
    for (unsigned int i=0; i< 7*len; ++i) {
      intbuf[i] = fill_int;
    }
  }
    break;
  default:
  assert(false); // should never get here
  return 0;
  }
  if (result == FAIL) {
    free(*datbuf);
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_GETFILL, he5_fieldname);
  }
  return 0;
}

RC hdfeos5_write_analysis_data(const bool selected[], int num_records, const OUTPUT_INFO *outputRecords) {
  for(unsigned int i=0; i<output_num_fields; i++) {
    struct output_field thefield = output_data_analysis[i];

    size_t length = 1 + strlen(thefield.fieldname)
      + (thefield.windowname ? 1 + strlen(thefield.windowname) : 0);
    char he5_fieldname[length];
    if (thefield.windowname)
      sprintf(he5_fieldname, "%s.%s", thefield.windowname, thefield.fieldname);
    else
      sprintf(he5_fieldname, "%s", thefield.fieldname);
    replace_chars(he5_fieldname);

    void *datbuf;
    RC rc = initialize_data_buffer(&datbuf, nTimes * nXtrack * thefield.data_cols, he5_fieldname, thefield.memory_type);
    if (rc)
      return rc;

    for(int record=0; record<num_records; record++) {
      if(selected[record]) {
        write_to_buffer(datbuf, &thefield, record, &outputRecords[record]);
      }
    }
    // write to file
    hssize_t start[HE5_DTSETRANKMAX] = {0};
    hsize_t edge[HE5_DTSETRANKMAX];
    get_edge_analysis(edge, &thefield);
    herr_t result = HE5_SWwritefield(swath_id, he5_fieldname, start, NULL, edge, datbuf);
    free(datbuf);
    if(result)
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS5_WRITEFIELD, he5_fieldname);
  }
  return ERROR_ID_NO;
}

/*! \brief Write a single record of an output field to the output
    buffer.*/
static void write_to_buffer(void *datbuf, const struct output_field *thefield, int recordno, const OUTPUT_INFO *recordinfo) {
  // obtain along-track and cross-track index for this record from
  // spectrum number, stored in recordinfo:
  int index_row = (recordinfo->specno-1) % nXtrack; //specno is 1-based
  int index_measurement = (recordinfo->specno-1) / nXtrack;

  size_t ncols = thefield->data_cols;
  switch(thefield->memory_type) {
  case OUTPUT_INT:
    for(size_t i=0; i<ncols; i++)
      ((int (*)[nXtrack][ncols])datbuf)[index_measurement][index_row][i]
        = ((int (*)[ncols])thefield->data)[recordno][i];
  break;
  case OUTPUT_SHORT:
    for(size_t i=0; i<ncols; i++)
      ((short (*)[nXtrack][ncols])datbuf)[index_measurement][index_row][i]
        = ((short (*)[ncols])thefield->data)[recordno][i];
    break;
  case OUTPUT_USHORT:
    for(size_t i=0; i<ncols; i++)
      ((unsigned short (*)[nXtrack][ncols])datbuf)[index_measurement][index_row][i]
        = ((unsigned short (*)[ncols])thefield->data)[recordno][i];
    break;
  case OUTPUT_STRING:
    for(size_t i=0; i<ncols; i++)
      ((char* (*)[nXtrack][ncols])datbuf)[index_measurement][index_row][i]
        = ((char* (*)[ncols])thefield->data)[recordno][i];
    break;
  case OUTPUT_FLOAT:
    for(size_t i=0; i<ncols; i++) {
      (((float (*)[nXtrack][ncols])datbuf)[index_measurement])[index_row][i]
              = ((float (*)[ncols])thefield->data)[recordno][i];
    }
    break;
  case OUTPUT_DOUBLE:
    for(size_t i=0; i<ncols; i++)
      ((double (*)[nXtrack][ncols])datbuf)[index_measurement][index_row][i]
        = ((double (*)[ncols])thefield->data)[recordno][i];
    break;
  case OUTPUT_DATE:
    for(size_t i=0; i<ncols; i++) {
      struct date date_in = ((struct date (*)[ncols])thefield->data)[recordno][i];
      int *date_out = (((int (*)[nXtrack][ncols][3])datbuf)[index_measurement])[index_row][i];
      date_out[0] = date_in.da_year;
      date_out[1] = date_in.da_mon;
      date_out[2] = date_in.da_day;
    }
    break;
  case OUTPUT_TIME:
    for(size_t i=0; i<ncols; i++) {
      struct time time_in = ((struct time (*)[ncols])thefield->data)[recordno][i];
      unsigned char *time_out = (((unsigned char (*)[nXtrack][ncols][3])datbuf)[index_measurement])[index_row][i];
      time_out[0] = time_in.ti_hour;
      time_out[1] = time_in.ti_min;
      time_out[2] = time_in.ti_sec;
    }
    break;
  case OUTPUT_DATETIME:
    for(size_t i=0; i<ncols; i++) {
      struct datetime datetime_in = ((struct datetime (*)[ncols])thefield->data)[recordno][i];
      int *datetime_out = (((int (*)[nXtrack][ncols][7])datbuf)[index_measurement])[index_row][i];
      datetime_out[0] = datetime_in.thedate.da_year;
      datetime_out[1] = datetime_in.thedate.da_mon;
      datetime_out[2] = datetime_in.thedate.da_day;
      datetime_out[3] = datetime_in.thetime.ti_hour;
      datetime_out[4] = datetime_in.thetime.ti_min;
      datetime_out[5] = datetime_in.thetime.ti_sec;
      datetime_out[6] = (datetime_in.millis != -1)
        ? datetime_in.millis * 1000 // SCIA/OMI/...: milliseconds
        : datetime_in.microseconds; // GOME2 has microseconds
    }
    break;
  }
}

/*! \brief A comma-separated list of the names of the dimensions of an output
    field.*/
void get_hdfeos5_dimension(char *dimension, const struct output_field *thefield) {
  if(nXtrack > 1)
    strcat(dimension, ",nXtrack");

  // for fields with 1 < data_cols < 10: add a string containing the
  // column dimension,  e.g. ",3".
  // dimensions with these names were created in create_dimensions()
  char ncols[3] = "";
  if(thefield->data_cols > 1) {
    assert(thefield->data_cols < 10);
    sprintf(ncols, ",%d", (int) thefield->data_cols);
  }
  strcat(dimension, ncols);
  switch(thefield->memory_type) {
  case OUTPUT_DATE:
    strcat(dimension, ",date");
    break;
  case OUTPUT_TIME:
    strcat(dimension, ",time");
    break;
  case OUTPUT_DATETIME:
    strcat(dimension, ",datetime");
    break;
  default:
    break;
  }
}

void get_hdfeos5_dimension_calibration(char *dimension, const struct output_field *thefield) {
  sprintf(dimension, "nCalibWindows"); // for calibration fields, first dimension is nCalibWindows
  get_hdfeos5_dimension(dimension, thefield); // get other dimensions based on field type
}

void get_hdfeos5_dimension_analysis(char *dimension, const struct output_field *thefield) {
  sprintf(dimension, "nTimes"); // for analysis/run calibration fields, first dimension is nTimes
  get_hdfeos5_dimension(dimension, thefield); // get other dimensions based on field type
}

/*! \brief The number of dimensions of an output field.  Dimensions of size 1
    are not counted.*/
int get_rank(const struct output_field *thefield) {
  int rank = 1;
  if(nXtrack > 1)
    rank++;
  if(thefield->data_cols > 1)
    rank++;
  switch(thefield->memory_type) {
  case OUTPUT_DATE:
  case OUTPUT_TIME:
  case OUTPUT_DATETIME:
    rank++;
    break;
  default:
    break;
  }
  return rank;
}

/*! \brief Get size of a chunk along each dimension of the output field.

    We use a single chunk for the columns of datafields with multiple
    columns, and the different fields of a date/time/datetime
    field  */
void get_chunkdims(hsize_t chunkdims[], const struct output_field *thefield) {
  int index_dim = 0;
  if(thefield->data_cols > 1)
    chunkdims[index_dim++] = thefield->data_cols;
  switch(thefield->memory_type) {
  case OUTPUT_DATE:
    chunkdims[index_dim++] = 3;
    break;
  case OUTPUT_TIME:
    chunkdims[index_dim++] = 3;
    break;
  case OUTPUT_DATETIME:
    chunkdims[index_dim++] = 7;
    break;
  default:
    break;
  }
  assert(index_dim <= HE5_DTSETRANKMAX);
}

/*! \brief Describe the size of the databuffer for a specific output
    field along each dimension. */
void get_edge(hsize_t edge[], const struct output_field *thefield) {
  int index_dim = 0;
  if(thefield->data_cols > 1)
    edge[index_dim++] = thefield->data_cols;
  switch(thefield->memory_type) {
  case OUTPUT_DATE:
    edge[index_dim++] = 3;
    break;
  case OUTPUT_TIME:
    edge[index_dim++] = 3;
    break;
  case OUTPUT_DATETIME:
    edge[index_dim++] = 7;
    break;
  default:
    break;
  }
}

/*! \brief Describe the size of the databuffer for a specific output
    field along each dimension. First dimensions are (nTimes x
    nXtrack), or just (nTimes) when nXtrack = 1, for analysis or
    runcalibration output, we write along the complete cross-track
    dimension */
void get_edge_analysis(hsize_t edge[], const struct output_field *thefield) {
  int index_dim = 0;
  edge[index_dim++] = nTimes;
  if(nXtrack > 1)
    edge[index_dim++] = nXtrack;
  get_edge(&edge[index_dim], thefield);
}

/*! \brief Describe the size of the databuffer for a specific output
    field along each dimension.  For calibration fields, first
    dimensions are (nCalibWindows x nXtrack), but we write calibration
    output for a single row at a time. */
void get_edge_calibration(hsize_t edge[], const struct output_field *thefield) {
  int index_dim = 0;
  edge[index_dim++] = nCalibWindows;
  if(nXtrack > 1)
    edge[index_dim++] = 1; // write a single row at a time.
  get_edge(&edge[index_dim], thefield);
}

/*! \brief Translate data types defined in output.h to data types used
    by the HDF-EOS5 library.*/
hid_t get_hdfeos5_type(enum output_datatype datatype) {
  switch(datatype) {
  case OUTPUT_STRING:
    return HE5T_CHARSTRING;
    break;
  case OUTPUT_SHORT:
    return HE5T_NATIVE_SHORT;
    break;
  case OUTPUT_USHORT:
    return HE5T_NATIVE_USHORT;
    break;
  case OUTPUT_FLOAT:
    return HE5T_NATIVE_FLOAT;
    break;
  case OUTPUT_INT:
    return HE5T_NATIVE_INT;
    break;
  case OUTPUT_DOUBLE:
    return HE5T_NATIVE_DOUBLE;
    break;
  case OUTPUT_TIME:
    return HE5T_NATIVE_UCHAR; // time is stored using 3 unsigned chars
    break;
  case OUTPUT_DATE:
  case OUTPUT_DATETIME:
    return HE5T_NATIVE_INT; // date/datetime are stored as a set of integers
    break;
  default:
  assert(false); // should never get here
  return 0;
  }
}

static enum fieldtype get_fieldtype(enum _prjctResults output_field) {
  enum fieldtype result;
  switch(output_field) {
  case PRJCT_RESULTS_SPECNO:
  case PRJCT_RESULTS_NAME:
  case PRJCT_RESULTS_DATE_TIME:
  case PRJCT_RESULTS_DATE:
  case PRJCT_RESULTS_TIME:
  case PRJCT_RESULTS_YEAR:
  case PRJCT_RESULTS_JULIAN:
  case PRJCT_RESULTS_JDFRAC:
  case PRJCT_RESULTS_TIFRAC:
  case PRJCT_RESULTS_SCANS:
  case PRJCT_RESULTS_NREJ:
  case PRJCT_RESULTS_TINT:
  case PRJCT_RESULTS_SZA:
  case PRJCT_RESULTS_AZIM:
  case PRJCT_RESULTS_TDET:
  case PRJCT_RESULTS_SKY:
  case PRJCT_RESULTS_REFZM:
  case PRJCT_RESULTS_REFNUMBER:
  case PRJCT_RESULTS_REFNUMBER_BEFORE:
  case PRJCT_RESULTS_REFNUMBER_AFTER:
  case PRJCT_RESULTS_PIXEL:
  case PRJCT_RESULTS_PIXEL_TYPE:
  case PRJCT_RESULTS_ORBIT:
  case PRJCT_RESULTS_LONGIT:
  case PRJCT_RESULTS_LATIT:
  case PRJCT_RESULTS_LON_CORNERS:
  case PRJCT_RESULTS_LAT_CORNERS:
  case PRJCT_RESULTS_ALTIT:
  case PRJCT_RESULTS_CLOUD:
  case PRJCT_RESULTS_CLOUDTOPP:
  case PRJCT_RESULTS_LOS_ZA:
  case PRJCT_RESULTS_LOS_AZIMUTH:
  case PRJCT_RESULTS_SAT_HEIGHT:
  case PRJCT_RESULTS_EARTH_RADIUS:
  case PRJCT_RESULTS_VIEW_ELEVATION:
  case PRJCT_RESULTS_VIEW_AZIMUTH:
  case PRJCT_RESULTS_VIEW_ZENITH:
  case PRJCT_RESULTS_SCIA_QUALITY:
  case PRJCT_RESULTS_SCIA_STATE_INDEX:
  case PRJCT_RESULTS_SCIA_STATE_ID:
  case PRJCT_RESULTS_STARTDATE:
  case PRJCT_RESULTS_ENDDATE:
  case PRJCT_RESULTS_STARTTIME:
  case PRJCT_RESULTS_ENDTIME:
  case PRJCT_RESULTS_SCANNING:
  case PRJCT_RESULTS_FILTERNUMBER:
  case PRJCT_RESULTS_MEASTYPE:
  case PRJCT_RESULTS_CCD_HEADTEMPERATURE:
  case PRJCT_RESULTS_COOLING_STATUS:
  case PRJCT_RESULTS_MIRROR_ERROR:
  case PRJCT_RESULTS_COMPASS:
  case PRJCT_RESULTS_PITCH:
  case PRJCT_RESULTS_ROLL:
  case PRJCT_RESULTS_GOME2_MDR_NUMBER:
  case PRJCT_RESULTS_GOME2_OBSERVATION_INDEX:
  case PRJCT_RESULTS_GOME2_SCANDIRECTION:
  case PRJCT_RESULTS_GOME2_OBSERVATION_MODE:
  case PRJCT_RESULTS_GOME2_SAA:
  case PRJCT_RESULTS_GOME2_SUNGLINT_RISK:
  case PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK:
  case PRJCT_RESULTS_GOME2_RAINBOW:
  case PRJCT_RESULTS_CCD_DIODES:
  case PRJCT_RESULTS_CCD_TARGETAZIMUTH:
  case PRJCT_RESULTS_CCD_TARGETELEVATION:
  case PRJCT_RESULTS_SATURATED:
  case PRJCT_RESULTS_INDEX_ALONGTRACK:
  case PRJCT_RESULTS_INDEX_CROSSTRACK:
  case PRJCT_RESULTS_UAV_SERVO_BYTE_SENT:
  case PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED:
  case PRJCT_RESULTS_UAV_INSIDE_TEMP:
  case PRJCT_RESULTS_UAV_OUTSIDE_TEMP:
  case PRJCT_RESULTS_UAV_PRESSURE:
  case PRJCT_RESULTS_UAV_HUMIDITY:
  case PRJCT_RESULTS_UAV_DEWPOINT:
  case PRJCT_RESULTS_UAV_PITCH:
  case PRJCT_RESULTS_UAV_ROLL:
  case PRJCT_RESULTS_UAV_HEADING:
  case PRJCT_RESULTS_SAT_LAT:
  case PRJCT_RESULTS_SAT_LON:
  case PRJCT_RESULTS_SAT_SAA:
  case PRJCT_RESULTS_SAT_SZA:
  case PRJCT_RESULTS_SAT_VZA:
  case PRJCT_RESULTS_STARTGPSTIME :
  case PRJCT_RESULTS_ENDGPSTIME :
  case PRJCT_RESULTS_LONGITEND :
  case PRJCT_RESULTS_LATITEND :
  case PRJCT_RESULTS_ALTITEND :
  case PRJCT_RESULTS_TOTALEXPTIME :
  case PRJCT_RESULTS_TOTALACQTIME :
  case PRJCT_RESULTS_FILENAME :
  case PRJCT_RESULTS_SCANINDEX :
  case PRJCT_RESULTS_ZENITH_BEFORE :
  case PRJCT_RESULTS_ZENITH_AFTER :
    result = GEO;
    break;
    // all other fields are Data fields:
  default:
    result = DATA;
    break;
  case PRJCT_RESULTS_MAX:
    assert(false);
    break;
  }
  return result;
}

/*! \brief Characters "/;," are not allowed in HDF-EOS5 field names.
    This function replaces them with a '-'. */
static void replace_chars(char *fieldname) {
  do {
    switch(*fieldname) {
    case ',':
    case ';':
    case '/':
    case ':':
      *fieldname = '-';
      break;
    default:
      break;
    }
  } while(*fieldname++ != '\0');
}
