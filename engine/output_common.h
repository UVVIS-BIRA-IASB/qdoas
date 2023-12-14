#ifndef OUTPUT_COMMON_H
#define OUTPUT_COMMON_H

/*! \file output_common.h \brief common variables and functions for the
    output modules.*/

#include "engine.h"
#include "output.h"
#include "QdoasVersion.h"
#include "kurucz.h"
#include "analyse.h"

#define MAX_FIELDS 3600 // maximum number of output fields
#define MAX_CALIB_FIELDS 20000 // maximum number of calibration fields

/*! \brief The different data types that can be stored in an \ref
    output_field structure.

    ::OUTPUT_INT, ::OUTPUT_SHORT etc are standard C datatypes.
    ::OUTPUT_DATE, ::OUTPUT_TIME and ::OUTPUT_DATETIME correspond to
    the \ref date, \ref time and \ref datetime structures
    respectively.
    */
enum output_datatype {
  OUTPUT_INT, /*!< int */
  OUTPUT_SHORT, /*!< short */
  OUTPUT_USHORT, /*!< unsigned short */
  OUTPUT_STRING, /*!< char* */
  OUTPUT_FLOAT, /*!< float */
  OUTPUT_DOUBLE, /*!< double */
  OUTPUT_DATE, /*!< \ref date structure */
  OUTPUT_TIME, /*!< \ref time structure */
  OUTPUT_DATETIME, /*!< \ref datetime structure */
  OUTPUT_RESIDUAL
};

struct field_attribute{
  char *label;
  char *value;
};

struct output_field;

/*! func_void is the generic function prototype for a function that
 * saves data to the output buffers.  It takes a void* buffer to store
 * output, which must be cast to a pointer of the correct type when
 * storing the data.  See save_analysis_data() and save_calib_data()
 */
typedef void (*func_void)(struct output_field *this_field, void *datbuf, ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);

typedef FENO *(*func_tabfeno)(struct output_field *this_field, int indexFenoColumn);
typedef CROSS_RESULTS *(*func_cross_results)(struct output_field *this_field, int indexFenoColumn, int index_calib);

/*! \brief contains all data related to an output field selected by
  the user, and function pointers that tell the program how to get the
  data.

  Each output_field is used to store one type of output data (e.g RMS,
  latitude, slant column density of a single absorber, ...).  The
  buffer used to store the data, output_field::data, is a \c void*,
  and should be cast to the correct data type, given by
  output_field::memory_type, before writing to/reading from it.

  Likewise, the type of output_field::get_data() is \ref func_void, a
  function, which takes a \c void* as an argument.  One should also
  cast the function pointer to the correct type before calling the
  function, i.e. \c func_double for an output field containing values
  of type double, etc.

  Remaining fields contain information that is used by the \ref
  get_data function to get the correct output data.  Depending on the
  kind data that we want to store, different fields may be used.
*/
struct output_field {
  enum _prjctResults resulttype;
  /*!< The content of this field.*/
  char *windowname;
  /*!< Name of the analysis window the data is from.*/
  const char *basic_fieldname;
  char *fieldname;
  char *symbolname;
  /*!< title of the field in the output file */
  const char *format;
  /*!< format string used to generate an ascii string of the data */
  const char *column_number_format;
  /*!< format string used to generate column number in ascii headers.
     Not used for single-column fields, but must not be NULL for
     multicolumn fields!  e.g. \c "(%d)" to have a number in brackets,
     which will print titles such as \c
     "fieldname(1)\tfieldname(2)...".  To label columns
     alphabetically, choose a character format string, i.e. \c
     "(%c)".*/
  struct field_attribute *attributes;
  /*!< Array of attributes of the form "label: value" that can be
     stored with the output field when using an output format which
     supports this. */
  int num_attributes;
  /*!< Length of the attributes array */
  bool column_number_alphabetic;
  /*!< \c true when columns in ascii files should be labeled '(A)', '(B)', ...  */
  enum output_datatype memory_type;
  /*!< actual datatype contained in the *data buffer */
  void *data;
  /*!< buffer containing the data for output */
  size_t data_rows;
  /*!< number of entries which can be stored in the data buffer */
  size_t data_cols;
 /*!< the data can contain multiple columns (i.e. latitude of pixel
   corners) */
  func_void get_data;
  /*!< pointer to function that will store the correct data in the
    buffer */
  int index_feno;
  /*!< for output fields related to an analysis window: index of the
    analysis window in TabFeno (or KuruczFeno in case of a calibration
    field) */
  func_tabfeno get_tabfeno;
 /*!< Pointer to function to retrieve the \ref FENO structure relevant
     for this field.  A different function is used for analyis or
     calibration fields */
  int index_calib;
  /*!< for calibration output fields: index of the Kurucz sub-window*/
  int index_cross;
  /*!< for output fields containing cross section results: index to
     look up the correct \ref CROSS_REFERENCE structure in TabCross*/
  int index_cross2;
  /*!< for correlation/covariance fields: TabCross index of the second
     correlated variable */
  func_cross_results get_cross_results;
 /*!< Pointer to function to retrieve the \ref CROSS_RESULTS structure
     relevant for this field.  A different function is used for
     analyis or calibration fields */
  int index_row;
  /*!< For reference calibration output, a different output field is
     registered for each detector row.  In this case, index_row is
     used to store the number of the row.*/
  int index_cic;
  /*!< For color indexes: index to retrieve Cic for this field from
     OUTPUT_cic */
  int index_flux;
  /*!< For fluxes: index to retrieve the flux from OUTPUT_fluxes */
};

typedef struct _outputInfo
{
  int specno; // number of the spectrum in the file, 1 based
  int i_crosstrack;
  int i_alongtrack;
  int nbColumns;
  int year,month,day;
  float longit,latit;
} OUTPUT_INFO;

/*! \brief Number of configured output fields in
    #output_data_analysis */
extern unsigned int output_num_fields;
/*! \brief Number of configured output fields in #output_data_calib */
extern unsigned int calib_num_fields;

/*! \brief Output fields for analysis results (or, in "run calibration"
   mode, the results of applying the calibration settings to the
   measured spectra).

   The number of configured fields is kept in #output_num_fields */
extern struct output_field output_data_analysis[MAX_FIELDS];
/*! \brief Output fields for the reference spectrum calibration.

  The number of configured fields is kept in #calib_num_fields. */
extern struct output_field output_data_calib[MAX_CALIB_FIELDS];

/*! \brief returns the number of bytes used by an output datatype. */
size_t output_get_size(enum output_datatype datatype);

/** @name Open output file.*/
//!@{
/** \brief Open the output file and, prepare different output fields
    and write calibration data if necessary.

    \param [in] pEngineContext structure including information on

    \param [in] filename the name of the outputFile

    \retval ERROR_ID_FILE_OPEN if the requested file could not be
    opened
    \retval ERROR_ID_NONE else
*/
RC open_output_file(const ENGINE_CONTEXT *pEngineContext, const char *filename);
RC ascii_open(const ENGINE_CONTEXT *pEngineContext, char *filename);
//!@}

/** @name Close output file.*/
//!@{
/** \brief Close current output file. */
//void output_close_file(const ENGINE_CONTEXT *pEngineContext);
void ascii_close_file(void);
//!@}

/** @name Write data.*/
//!@{
/** \brief Write output data of the selected records to the current
    file.

    \param [in] selected_records, num_records boolean array describing
    which records should be included in the output, and its length

    \param [in] outputRecords meta data on the records
    */
//void output_write_data(const bool selected_records[],const ENGINE_CONTEXT *pEngineContext);
void ascii_write_analysis_data(const bool selected_records[], int num_records);
void ascii_write_spectra_data(const bool selected_records[], int num_records);
//!@}

#endif
