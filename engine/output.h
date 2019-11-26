#ifndef OUTPUT_H
#define OUTPUT_H

#include "engine.h"
#include "doas.h"
#include "output_formats.h"

/*! \file output.h \brief Output module interface.*/

void OUTPUT_ResetData(void);

RC OUTPUT_CheckPath(ENGINE_CONTEXT *pEngineContext,char *path,int format);

RC OUTPUT_RegisterData(const ENGINE_CONTEXT *pEngineContext);
RC OUTPUT_RegisterSpectra(const ENGINE_CONTEXT *pEngineContext);

/*! \brief Write all saved output data to disk. */
RC OUTPUT_FlushBuffers(ENGINE_CONTEXT *pEngineContext);

/*! \brief Save the results of the current spectrum in the output fields.
  \param [in] indexFenoColumn detector row of the processed record.

  \param [in,out] pEngineContext   structure including information on the current record

  \retval Non zero value return code if the function failed
  \retval ERROR_ID_NO on success
*/
RC OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn);

RC OUTPUT_ReadAmf(const char *symbolName,const char *amfFileName,char amfType,INDEX *pIndexAmf);
RC OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lambda,double *xs,int n_wavel);
RC OUTPUT_LocalAlloc(ENGINE_CONTEXT *pEngineContext);
RC OUTPUT_Alloc(void);
void OUTPUT_Free(void);

/*! \brief For GOME-2/Sciamachy automatic reference spectrum: file
    from which the reference was generated. */
extern char OUTPUT_refFile[DOAS_MAX_PATH_LEN+1];
/*! \brief For GOME-2/Sciamachy automatic reference spectrum: number
    of spectra used. */
extern int         OUTPUT_nRec;
/*! \brief 1 to save lambda and/or spectra with Export spectra option; 0 otherwise */
extern int         OUTPUT_exportSpectraFlag;

// Air Mass Factors (AMF) table cross reference
// --------------------------------------------
typedef struct _amfReference {
  char    type,                          // type of symbol
           symbolName[MAX_STR_LEN+1],      // name of symbol
           amfFileName[MAX_STR_LEN+1];     // name of AMF file
  double **Phi;                           // AMF data
  double **deriv2;                        // AMF second derivatives for spline calculations
  double **xs;                            // cross sections
  double **xsDeriv2;                      // cross sections second derivatives
  int      PhiLines,                      // number of lines in Phi matrix
           PhiColumns,                    // number of columns in Phi matrix
           xsLines,                       // number of lines in Param matrix
           xsColumns;                     // number of columns in Param matrix
} AMF_SYMBOL;

/*! \brief List of cross sections with associated AMF file */
extern AMF_SYMBOL *OUTPUT_AmfSpace;

#define     MAX_FLUXES    20
#define     MAX_CIC       20
#define     MAX_RESULTS  500   // 250 measurements the morning; 250 measurements the afternoon.

// Fill values, copied from the netCDF default fill values (NC_FILL_BYTE etc in netcdf.h)
extern const char QDOAS_FILL_BYTE;
extern const char QDOAS_FILL_CHAR;
extern const short QDOAS_FILL_SHORT;
extern const int QDOAS_FILL_INT;
extern const float QDOAS_FILL_FLOAT;
extern const double QDOAS_FILL_DOUBLE;
extern const unsigned char QDOAS_FILL_UBYTE;
extern const unsigned short QDOAS_FILL_USHORT;
extern const unsigned int QDOAS_FILL_UINT;
extern const long long QDOAS_FILL_INT64;
extern const unsigned long long QDOAS_FILL_UINT64;
extern const char *QDOAS_FILL_STRING;

#endif
