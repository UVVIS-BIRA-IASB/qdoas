
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  LOW-LEVEL DEFINITIONS
//  Name of module    :  COMDEFS.H
//  Creation date     :  29 September 2004
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  The present module contains the compiler directives and the definitions that
//  couldn't be supported by all operating systems or compilers.  It also
//  includes some definitions related to the low-level functions of the package
//  (errors handling, debugging, memory allocation...).
//
//  ----------------------------------------------------------------------------

#if !defined(__COMDEFS_)
#define __COMDEFS_

#include <stdbool.h>

#ifndef WIN32
#include <unistd.h>
#define strnicmp strncasecmp
#endif

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// ===================
// COMPILATION CONTROL
// ===================

// ===============
// INCLUDE HEADERS
// ===============

typedef int         INDEX,RC;                                            // RC holds for return code
typedef unsigned int MASK,SZ_LEN;

// Macros

// min and max macros conflict with some c++ headers. In c++ we can
// use std::min and std::max.
#ifndef __cplusplus

#ifndef max
#define max(a,b) ( ( (a) >= (b) ) ? (a) : (b) )                                 // returns the maximum between two numbers
#endif
#ifndef min
#define min(a,b) ( ( (a) <= (b) ) ? (a) : (b) )                                 // returns the minimum between two numbers
#endif

#endif // __cplusplus

// double macro expansion trick to convert preprocessor symbol values
// to strings
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Strings lengths

#define MAX_STR_LEN                            1023                             // usual length for string buffers
#define MAX_STR_SHORT_LEN                      1023     // Before 511           // usual length for short strings
#define MAX_MSG_LEN                            1023                             // maximum length for messages
#define DOAS_MAX_PATH_LEN                           1023                             // maximum length for path and file names
#define MAX_FCT_LEN                              63                             // maximum length for the names of functions
#define MAX_VAR_LEN                              63                             // maximum length for the names of variables
#define MAX_ITEM_TEXT_LEN                      4192     // Before 511           // item text length
#define MAX_ITEM_NAME_LEN                       127                             // name of a symbol
#define MAX_ITEM_DESC_LEN                       255                             // description of a symbol

// Math constants

#define DOAS_PI     (double) 3.14159265358979323846
#define PI2         (double) 6.28318530717958647692
#define PIDEMI      (double) 1.57079632679489661923

// Other constants definitions

#define ITEM_NONE                              (int)-1                          // default value for an index or a return code out of range

#if defined(WIN32) && WIN32
    #define PATH_SEP '/'                                                        // path separator is different according to the OS (Windows or Unix/Linux)
    #define COMMENT_CHAR ';'                                                    // characters to use for commented lines
#else
    #define PATH_SEP '/'
    #define COMMENT_CHAR '#'
#endif

// =====================
// STRUCTURES DEFINITION
// =====================

// The struct date and struct time structures come from the dos.h file provided
// with the Turbo/Borland compilers. As it seems that these are not supported
// by all compilers and OS, we define them below but do not include the dos.h

#pragma pack(push,1)

struct time
 {
  unsigned char ti_min;                                                         /* Minutes */
  unsigned char ti_hour;                                                        /* Hours */
  unsigned char ti_hund;                                                        /* Hundredths of seconds */
  unsigned char ti_sec;                                                         /* Seconds */
 };

struct date
 {
  int  da_year;                                                                 /* Year */
  char da_day;                                                                  /* Day of the month */
  char da_mon;                                                                  /* Month (1 = Jan)  */
 };

struct datetime {
  struct time thetime;
  struct date thedate;
  int millis;
  int microseconds;
};

typedef struct _slit SLIT;
typedef struct _filter PRJCT_FILTER;

// The structure below supports the year in short format instead of integer.
// This is useful to support files with records including this struct date but
// written under DOS or Windows 16 bits.

typedef struct _shortDate
 {
  short da_year;                                                                /* Year */
  char  da_day;                                                                 /* Day of the month */
  char  da_mon;                                                                 /* Month (1 = Jan)  */
 }
SHORT_DATE;

#pragma pack(pop)

// ===============
// TABLE OF ERRORS
// ===============

// Standard errors id

#define ERROR_ID_NO                               0                             // no error
#define ERROR_ID_BUG                             10                             // we hit a bug or limitation in QDOAS (e.g. max number of output fields reached)
#define ERROR_ID_ALLOC                          100                             // buffer allocation error
#define ERROR_ID_ALLOCVECTOR                    101                             // double type vector allocation error
#define ERROR_ID_ALLOCMATRIX                    102                             // double type matrix allocation error
#define ERROR_ID_BUFFER_FULL                    103                             // a buffer is full and can not receive objects anymore
#define ERROR_ID_COMMANDLINE                    105                             // syntax error in command line
#define ERROR_ID_MEDIATE                        106
#define ERROR_ID_SWATHSIZE                      107                             // swath size > MAX_SWATHSIZE

// Files

#define ERROR_ID_FILE_NOT_FOUND                 121                             // can not open a file in reading mode (file not found)
#define ERROR_ID_FILE_EMPTY                     122                             // the file is empty or not large enough
#define ERROR_ID_FILE_OPEN                      123                             // can not open a file in writing mode
#define ERROR_ID_FILE_END                       124                             // end of the file is reached
#define ERROR_ID_FILE_RECORD                    125                             // the current file record doesn't match the spectra selection criteria
#define ERROR_ID_FILE_BAD_FORMAT                126                             // unknown file format
#define ERROR_ID_FILE_BAD_LENGTH                127
#define ERROR_ID_WAVELENGTH                     128                             // bad wavelength calibration in the input file
#define ERROR_ID_FILE_OVERWRITE                 129                             // data not saved; change the output file name
#define ERROR_ID_FILE_NOT_SPECIFIED             130                             // a required file name is missing from the configuration
#define ERROR_ID_FILE_STAT                      131                             // cannot call fstat on file
#define ERROR_ID_DIR_NOT_FOUND                  132                             // directory does not exist
#define ERROR_ID_XS_BAD_WAVELENGTH              133                             // the cross section should be defined on the same grid as the reference one when 'None' is selected as Interp/conv action
#define ERROR_ID_XS_COLUMNS                     134                             // the cross section does not have the expected number of columns
#define ERROR_ID_XS_RING                        135                             // ring cross section does not contain 4 columns
#define ERROR_ID_XS_FILENAME                    136                             // no file name for cross section
#define ERROR_ID_REF_DATA                       137                             // For imager: reference for a column does not contain data (fill values)
#define ERROR_ID_MISSING_INITIAL_CALIB          138                             // Configuration doesn't provide an initial wavelength calibration (from reference spectrum or calibration file)
#define ERROR_ID_REF_SOLAR_IDENTICAL            139                             // Reference spectrum and solar spectrum are identical

// Debug

#define ERROR_ID_DEBUG_START                    201                             // debug mode already running
#define ERROR_ID_DEBUG_STOP                     202                             // debug mode not open
#define ERROR_ID_DEBUG_FCTTYPE                  203                             // unknown function type
#define ERROR_ID_DEBUG_LEVELS                   204                             // number of levels of function calls out of range
#define ERROR_ID_DEBUG_FCTBLOCK                 205                             // incorrect function block

// Memory

#define ERROR_ID_MEMORY_STACKNOTEMPTY           301                             // stack not empty at the end of the memory control
#define ERROR_ID_MEMORY_STACKNOTALLOCATED       302                             // stack not allocated
#define ERROR_ID_MEMORY_STACKALLOCATED          303                             // stack already allocated
#define ERROR_ID_MEMORY_OBJECTNOTFOUND          304                             // object not found in the stack
#define ERROR_ID_MEMORY_DEFMATRIX               305                             // the definition of a matrix is uncompleted
#define ERROR_ID_MEMORY_RELEASE                 306                             // try to release an object not in the stack

// Low-level math operations

#define ERROR_ID_DIVISION_BY_0                  501                             // division by zero
#define ERROR_ID_OVERFLOW                       502                             // exponential overflow
#define ERROR_ID_LOG                            503                             // log error
#define ERROR_ID_BAD_ARGUMENTS                  504                             // the input arguments may be wrong
#define ERROR_ID_MATRIX_DIMENSION               505                             // matrix dimensions must agree
#define ERROR_ID_SQRT_ARG                       506                             // sqrt argument error
#define ERROR_ID_POW                            507                             // pow overflow
#define ERROR_ID_NORMALIZE                      508                             // vector norm is zero

// Output

#define ERROR_ID_AMF                            801                             // AMF not calculated
#define ERROR_ID_NOTHING_TO_SAVE                802                             // nothing to save
#define ERROR_ID_OUTPUT_NETCDF                  803                             // NetCDF file already contains the group we want to write to

// High-level math functions

#define ERROR_ID_SVD_ILLCONDITIONED           1101                              // ill-conditionned matrix
#define ERROR_ID_SVD_ARG                       1102                             // bad arguments
#define ERROR_ID_SPLINE                        1110                             // spline interpolation requests increasing absissae
#define ERROR_ID_VOIGT                         1111                             // Voigt function failed
#define ERROR_ID_ERF                           1112                             // error with the calculation of the erf function
#define ERROR_ID_JULIAN_DAY                    1120                             // error in the calculation of the Julian day
#define ERROR_ID_MATINV                        1130                             // matrix inversion failed
#define ERROR_ID_CONVERGENCE                   1140                             // the algorithm doesn't converge
#define ERROR_ID_USAMP                         1150                             // undersampling tool failed
#define ERROR_ID_OPTIONS                       1160                             // incompatible options in the analysis
#define ERROR_ID_GAUSSIAN                      1170                             // calculation of an effective slit function failed
#define ERROR_ID_SLIT                          1180                             // bad input slit function types for resolution adjustment
#define ERROR_ID_SLIT_T                        1190                             // bad input slit function types for resolution adjustment with temperature dependency
#define ERROR_ID_GAPS                          1200                             // invalid window for calibration
#define ERROR_ID_ANALYSIS                      1210                             // analysis aborted
#define ERROR_ID_MSGBOX_FIELDEMPTY             1220                             // field empty or invalid
#define ERROR_ID_NFREE                         1230                             // problem with the number of degrees of freedom
#define ERROR_ID_ORTHOGONAL_BASE               1240                             // problem with the orthogonalisation of cross sections
#define ERROR_ID_ORTHOGONAL_CASCADE            1250                             // problem with the orthogonalisation of cross sections
#define ERROR_ID_FWHM                          1260                             // wrong value of the FWHM
#define ERROR_ID_FWHM_INCOMPATIBLE_OPTIONS     1261                             // Can't apply fitting of slit function parameters with calibration and resolution correction
#define ERROR_ID_OUT_OF_RANGE                  1270                             // field is out of range
#define ERROR_ID_FILE_AUTOMATIC                1280                             // no automatic reference selection can be performed on this type of file
#define ERROR_ID_REFERENCE_SELECTION           1281                             // can't find (enough) spectra matching automatic reference criteria
#define ERROR_ID_REF_ALIGNMENT                 1285                             // problem with the alignment of the reference spectrum in one analysis window
#define ERROR_ID_NO_REF                        1290                             // no reference file found in the specified file
#define ERROR_ID_VZA_REF                       1291                             // no reference for this vza bin.
#define ERROR_ID_CONVOLUTION                   1295                             // incompatibility with convolution options
#define ERROR_ID_NO_RESULT_PREVIOUS_WINDOW     1296                             // when using result from previous window as fixed column value: cannot link molecule with a molecule from a previous analysis window
#define ERROR_ID_IMAGER_CALIB                  1297                             // calibration error for imager row
#define ERROR_ID_L1WAVELENGTH                  1298                             // L1 wavelength calibration incorrect
#define ERROR_ID_PUKITE                        1299                             // missing pukite cross section

// Specific file format

#define ERROR_ID_GDP_BANDINDEX                 1300                             // band is not present in the GDP file

#define ERROR_ID_PDS                           1400                             // error in the SCIA PDS file
#define ERROR_ID_BEAT                          1401                             // error in the GOME2/OMI format
#define ERROR_ID_OMI_SWATH                     1402                             // OMI requested swath not present
#define ERROR_ID_OMI_REF                       1403                             // no irradiance found
#define ERROR_ID_OMI_REFSIZE                   1404                             // irradiance and spectra swaths do not have the same size
#define ERROR_ID_OMI_PIXELQF                   1405                             // spectrum rejected based on pixels quality flags
#define ERROR_ID_HDFEOS                        1406                             // error in hdfeos file
#define ERROR_ID_HDFEOS5_SWATH                 1407                             // can not create swath
#define ERROR_ID_HDFEOS5_FILE_EXISTS           1408                             // output file already exists, not allowed for HDF-EOS5
#define ERROR_ID_HDFEOS5_DEFFIELD              1409                             // error creating data/geolocation field
#define ERROR_ID_HDFEOS5_DEFDIM                1410                             // error creating dimension
#define ERROR_ID_HDFEOS5_WRITEFIELD            1411                             // error writing to field
#define ERROR_ID_HDFEOS5_WRITEATTR             1412                             // error writing attribute
#define ERROR_ID_HDFEOS5_SETFILL               1413                             // error setting fill value for field
#define ERROR_ID_HDFEOS5_GETFILL               1414                             // error getting fill value for field
#define ERROR_ID_NETCDF                        1500
#define ERROR_ID_TROPOMI_REF                   1600                             // requested reference spectrum not found
#define ERROR_ID_FILE_FORMAT                   2000                             // bad file format

// =========
// DEBUGGING
// =========

// The following flag allows to easily enable/disable the debugging and to avoid
// the overload due to the presence of the debug functions in the code by removing
// them from the compilation as far as debug calls are included in a
// #if defined(__DEBUG_) / #endif block.

#define __DEBUG_                   0                                            // 1 to enable the debug mode, 0 to disable the debug mode

#define __DEBUG_DOAS_SVD_          0                                            // SVD decomposition
#define __DEBUG_DOAS_SHIFT_        0                                            // interpolation of vectors
#define __DEBUG_DOAS_DATA_         0                                            // load data
#define __DEBUG_DOAS_OUTPUT_       0                                            // output
#define __DEBUG_DOAS_FILE_         0                                            // file
#define __DEBUG_DOAS_CONFIG_       1                                            // config

// Types of functions to debug

#define  DEBUG_FCTTYPE_ALL     0xFF
#define  DEBUG_FCTTYPE_MEM     0x01                                              // memory allocation
#define  DEBUG_FCTTYPE_GUI     0x02                                              // user interface
#define  DEBUG_FCTTYPE_MATH    0x04                                              // math function
#define  DEBUG_FCTTYPE_APPL    0x08                                              // application related
#define  DEBUG_FCTTYPE_UTIL    0x10                                              // utility function
#define  DEBUG_FCTTYPE_FILE    0x20                                              // file management
#define  DEBUG_FCTTYPE_CONFIG  0x40                                              // check the config

// Authorize the debugging double type variables allocated by MEMORY_AllocDVector or MEMORY_AllocDMatrix

enum { DEBUG_DVAR_NO, DEBUG_DVAR_YES };

// Structures definition

typedef union _debugVarPtr                                                      // use a different definition of the pointer according to the type of the variable to debug
 {
  char   **ucharArray;                                                         // pointer to a string array
  char    *ucharVector;                                                        // pointer to a string
  short   **shortArray;                                                         // pointer to a short type array
  short    *shortVector;                                                        // pointer to a short type vector
  unsigned short  **ushortArray;                                                        // pointer to a unsigned short type array
  unsigned short   *ushortVector;                                                       // pointer to a unsigned short type vector
  int     **intArray;                                                           // pointer to a integer type array
  int      *intVector;                                                          // pointer to a integer type vector
  long    **longArray;                                                          // pointer to a long type array
  long     *longVector;                                                         // pointer to a long type vector
  float   **floatArray;                                                         // pointer to a float type array
  float    *floatVector;                                                        // pointer to a float type vector
  double   *doubleVector;                                                       // pointer to a double type vector
  double  **doubleArray;                                                        // pointer to a double type array
 }
DEBUG_VARPTR;

typedef struct _debugVariables                                                  // information on variables to debug
 {
  char         varName[MAX_VAR_LEN+1];                                         // the name of the variable to debug
  DEBUG_VARPTR  varData;                                                        // pointer to the buffer to print out
  int           varNl,varNc;                                                    // the size of the variable to debug
  int           varNlOff,varNcOff;                                              // the offset to apply to resp. index of lines and columns
  INDEX         varNlMin,varNlMax,varNcMin,varNcMax;                            // these indexes define the area of the vector or the matrix to print out
  int           varType;                                                        // the type of the variable to debug
  int           varMatrixFlag;                                                  // 1 if the object to debug is a matrix, 0 for a vector
 }
DEBUG_VARIABLE;

// Prototypes

void DEBUG_Print(const char *formatString,...);
void DEBUG_PrintVar(char *message,...);
RC   DEBUG_FunctionBegin(char *fctName,MASK fctType);
RC   DEBUG_FunctionStop(char *fctName,RC rcFct);
RC   DEBUG_Start(char *fileName,char *fctName,MASK fctMask,int nLevels,int varFlag,int resetFlag);
RC   DEBUG_Stop(char *callingFct);

// ===============
// ERRORS HANDLING
// ===============

// Error types (November 2007 : modify the values for QDoas compatibility, see mediate_types.h)

#define ERROR_TYPE_UNKNOWN    (int)-1
#define ERROR_TYPE_WARNING          2                                           // WarningEngineError
#define ERROR_TYPE_FATAL            3                                           // FatalEngineError
#define ERROR_TYPE_DEBUG            1                                           // InformationEngineError
#define ERROR_TYPE_FILE             1                                           // InformationEngineError

// Description of an error

typedef struct _errorDescription
 {
  int   errorType;                                                              // type of error (warning, fatal error, ...)
  int   errorId;                                                                // id number of the error
  char errorFunction[MAX_FCT_LEN+1];                                            // name of the calling function that produced the error
  char errorString[MAX_MSG_LEN+1];                                              // error message
 }
ERROR_DESCRIPTION;

// Prototypes

RC ERROR_DisplayMessage(void *responseHandle);
RC ERROR_SetLast(const char *callingFunction,int errorType,RC errorId,...);
RC ERROR_GetLast(ERROR_DESCRIPTION *pError);
bool ERROR_Fatal(void);

// ===============
// MEMORY HANDLING
// ===============

// Constants definition

#define MEMORY_STACK_SIZE        5000                                           // maximum objets allowed in the stack

// type of objects

enum _memoryTypes
 {
  MEMORY_TYPE_UNKNOWN,                                                          // unknown
  MEMORY_TYPE_PTR,                                                              // pointer
  MEMORY_TYPE_STRING,                                                           // character/string
  MEMORY_TYPE_SHORT,                                                            // short
  MEMORY_TYPE_USHORT,                                                           // unsigned short
  MEMORY_TYPE_INT,                                                              // integer
  MEMORY_TYPE_LONG,                                                             // long
  MEMORY_TYPE_FLOAT,                                                            // float
  MEMORY_TYPE_DOUBLE,                                                           // double
  MEMORY_TYPE_STRUCT,                                                           // structure
  MEMORY_TYPE_MAX
 };

#define MEMORY_TYPE_INDEX MEMORY_TYPE_INT
#define MEMORY_TYPE_ULONG MEMORY_TYPE_LONG
#define MEMORY_TYPE_UINT  MEMORY_TYPE_INT

// Information on allocated objects

typedef struct _memory
 {
  char  callingFunctionName[MAX_FCT_LEN+1];                                    // name of the calling function
  char  bufferName[MAX_VAR_LEN+1];                                             // name of the buffer
  char *pBuffer;                                                               // pointer to the allocated buffer
  int    itemNumber;                                                            // number of items in buffer
  int    itemSize;                                                              // size of item
  int    offset;                                                                // index of the first item
  int    type;                                                                  // type of object
 }
MEMORY;

// Global variables

extern int MEMORY_stackSize;                                                    // the size of the stack of allocated objects
extern const char *MEMORY_types[MEMORY_TYPE_MAX];                                    // available types for allocated objects

// Prototypes
void    *MEMORY_AllocBuffer(const char *callingFunctionName, const char *bufferName,int itemNumber,int itemSize,int offset,int type);
void     MEMORY_ReleaseBuffer(const char *callingFunctionName, const char *bufferName,void *pBuffer);
double  *MEMORY_AllocDVector(const char *callingFunctionName, const char *bufferName,int nl,int nh);
void     MEMORY_ReleaseDVector(const char *callingFunctionName, const char *bufferName,double *v,int nl);
double **MEMORY_AllocDMatrix(const char *callingFunctionName, const char *bufferName,int nrl,int nrh,int ncl,int nch);
void     MEMORY_ReleaseDMatrix(const char *callingFunctionName, const char *bufferName,double **m,int ncl,int nrl);

RC       MEMORY_Alloc(void);
RC       MEMORY_End(void);

RC       MEMORY_GetInfo(DEBUG_VARIABLE *pVariable,char *pBuffer);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
