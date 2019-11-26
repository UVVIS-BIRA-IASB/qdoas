#ifndef ANALYSE_H
#define ANALYSE_H

#include "doas.h"
#include "matrix.h"
#include "fit_properties.h"

typedef struct anlyswin_cross_section ANALYSIS_CROSS;
typedef struct anlyswin_shift_stretch ANALYSIS_SHIFT_STRETCH;
typedef struct anlyswin_gap ANALYSIS_GAP;
typedef struct anlyswin_output ANALYSIS_OUTPUT;
typedef struct _AnalyseLinearParameters ANALYSE_LINEAR_PARAMETERS;
typedef struct _AnalyseNonLinearParameters ANALYSE_NON_LINEAR_PARAMETERS;

struct _AnalyseLinearParameters {
 	char symbolName[MAX_ITEM_TEXT_LEN];
 	int polyOrder;
 	int baseOrder;
 	int storeFit;
 	int storeError;
};

struct _AnalyseNonLinearParameters {
 	char symbolName[MAX_ITEM_TEXT_LEN];
 	char crossFileName[MAX_ITEM_TEXT_LEN];
 	int fitFlag;
 	double initialValue;
 	double deltaValue;
 	double minValue;
 	double maxValue;
 	int storeFit;
 	int storeError;
};

// Analysis window description
// ---------------------------

enum _analysisType {
  ANALYSIS_TYPE_FWHM_NONE,                                                      // no fwhm fit
  ANALYSIS_TYPE_FWHM_CORRECTION,                                                // fwhm correction between spectrum and reference based on their temperature
  ANALYSIS_TYPE_FWHM_KURUCZ,                                                    // fwhm fit in Kurucz procedure
  ANALYSIS_TYPE_FWHM_NLFIT                                                      // fit the difference of resolution between spectrum and reference
};

enum linear_offset_mode {
  NO_LINEAR_OFFSET,  // no linear offset fit
  LINEAR_OFFSET_RAD, // linear offset normalized by 1/I
  LINEAR_OFFSET_REF  // linear offset normalized by 1/I0
};

// Symbols used in a project
// -------------------------

enum _wrkSymbolType
 {
  WRK_SYMBOL_CROSS,
  WRK_SYMBOL_CONTINUOUS,
  WRK_SYMBOL_PREDEFINED,
  WRK_SYMBOL_SPECTRUM
 };

#pragma pack(push,1)

struct _wrkSymbol {
  char        type,                           // type of symbol
                symbolName[MAX_STR_LEN+1],      // name of symbol
                crossFileName[MAX_STR_LEN+1],   // name of cross section file
                amfFileName[MAX_STR_LEN+1];     // name of AMF file name
  MATRIX_OBJECT xs;                             // cross sections (wavelength+cross section(s))
};

/*! \brief Symbol cross reference */
typedef struct _crossReference
 {
  int    Comp,                                                                  // index of component in WrkSpace list
         crossAction,                                                           // action (interpolation, convolution) to process on cross section before analysis
         crossCorrection,                                                       // correction to apply on cross section before analysis (the result of the correction shall be submitted to action)
         IndSvdA,                                                               // index of column in SVD matrix
         IndSvdP,                                                               // index of column in SVD matrix
         IndOrthog,                                                             // order in orthogonal base
         IndSubtract,                                                           // index of the cross section from which to subtract the current one
         FitConc,                                                               // flag set if concentration is to be fit (non linear method) or modified (linear method)
         FitFromPrevious,                                                       // flag set if the value of the concentration has to be retrieved from a previous window
         FitParam,                                                              // flag set if non linear parameter (other than shift or stretch) is to be fit
         FitShift,                                                              // flag set if shift is to be fit
         FitStretch,                                                            // flag set if stretch order 1 is to be fit
         FitStretch2,                                                           // flag set if stretch order 2 is to be fit
         TypeStretch,                                                           // order of stretch to fit
         isPukite,                                                              // 0 if not a Pukiter term, 1 if Pukite term to calculate, 2 if Pukite term as pre-convolved XS
         indexPukite1,                                                          // index of the cross section to use for Pukite1
         indexPukite2,                                                          // index of the cross section to use for Pukite2
         molecularCrossIndex;                                                   // index of the cross section to use for molecular correction

  int    display,                                                                 // flag set if fit is to be displayed
         amfType,                                                               // type of AMF
         filterFlag;                                                            // flag set if symbol is to be filteres

  double Fact,                                                                  // normalization factors
         I0Conc,
         InitConc,                                                              // initial concentration
         InitParam,                                                             // initial non linear parameter
         InitShift,                                                             // initial shift
         InitStretch,                                                           // initial stretch order 1
         InitStretch2,                                                          // initial stretch order 2
         DeltaConc,                                                             // step for concentration
         DeltaParam,                                                            // step for non linear parameter
         DeltaShift,                                                            // step for shift
         DeltaStretch,                                                          // step for stretch order 1
         DeltaStretch2,                                                         // step for stretch order 2
         DeltaScale,                                                            // step for scaling factor order 1
         DeltaScale2,                                                           // step for scaling factor order 2
         MinShift,                                                              // maximum value for shift
         MaxShift,                                                              // maximum value for shift
         MinParam,                                                              // minimum value for parameter
         MaxParam,                                                              // maximum value for parameter
         MinConc,                                                               // minimum value for the concentration if fitted
         MaxConc,                                                               // maximum value for the concentration if fitted
        *vector,                                                                // copy of vector
        *Deriv2,                                                                // second derivative
        *vectorBackup,                                                          // backup of the vector (in the case the original one is modified during the process; for example, molecular ring)
        *Deriv2Backup,                                                          // backup of the second derivative (in the case the original one is modified during the process; for example molecular ring)
        *molecularCrossSection;                                                 // for molecular ring, sigma-sigma_RRS (cross section minus cross section convolved with Raman)
} CROSS_REFERENCE;

// Results
// -------
/*! Fit results for a fitted cross section. */
struct _crossResults {
  char   StoreParam,               // flag set if non linear parameter is to be written into output file
         StoreShift,                // flag set if shift is to be written into output file
         StoreStretch,              // flag set if stretch order 1 is to be written into output file
         StoreScale,                // flag set if scaling factor order 1 is to be written into output file
         StoreParamError,           // flag set if error on non linear parameter is to be written into output file
         StoreError,                // flag set if error on previous parameters is to be written into output file
         StoreAmf,                  // flag set if air mass factor is to be written into output file
         StoreSlntCol,              // flag set if slant column is to be written into output file
         StoreSlntErr,              // flag set if error on slant column is to be written into output file
         StoreVrtCol,               // flag set if vertical column is to be written into output file
         StoreVrtErr;               // flag set if error on vertical column is to be written into output file

  double Param,                     // non linear parameter returned by CurFitMethod
         Shift,                     // shift returned by CurFitMethod
         Stretch,                   // stretch order 1 returned by CurFitMethod
         Stretch2,                  // stretch order 2 returned by CurFitMethod
         Scale,                     // scaling factor order 1 returned by CurFitMethod
         Scale2,                    // scaling factor order 2 returned by CurFitMethod
         SigmaParam,                // error on param
         SigmaShift,                // error on shift
         SigmaStretch,              // error on stretch order 1
         SigmaStretch2,             // error on stretch order 2
         SigmaScale,                // error on scaling factor order 1
         SigmaScale2,               // error on scaling factor order 2
         SlntCol,                   // slant column
         SlntErr,                   // error on slant column
         SlntFact,                  // slant column factor
         VrtCol,                    // vertical column
         VrtErr,                    // error on vertical column
         VrtFact,                   // vertical column factor
         ResCol,                    // residual column
         Amf;                       // air mass factor

  INDEX  indexAmf;                  // index of AMF data in AMF table cross reference
};

/*! \brief Configuration data related to an analysis window. */
struct _feno {
                                                                                // copy of data from analysis window panel

  char          windowName[MAX_ITEM_NAME_LEN+1];                                // name of analysis window
  char          refFile[MAX_ITEM_TEXT_LEN],                                   // reference file in reference file selection mode
                  ref1[MAX_ITEM_TEXT_LEN],                                    // first reference spectrum (in order to replace the SrefEtalon in the old ANALYSIS_WINDOWS structure)
                  ref2[MAX_ITEM_TEXT_LEN],                                    // second reference spectrum (in order to replace the SrefEtalon in the old ANALYSIS_WINDOWS structure)
                  residualsFile[MAX_ITEM_TEXT_LEN];
  double          refSZA,refSZADelta,refMaxdoasSZA,refMaxdoasSZADelta;          // in automatic reference selection mode, SZA constraints
  int             refSpectrumSelectionMode;                                     // reference spectrum selection mode
  int             refSpectrumSelectionScanMode;
  int             refMaxdoasSelectionMode;                                      // for MAXDOAS measurements, selection of the reference spectrum based on the scan or the SZA
  double          cloudFractionMin,cloudFractionMax;
  // char            refAM[MAX_ITEM_TEXT_LEN],refPM[MAX_ITEM_TEXT_LEN];        // in automatic reference selection mode, names of the spectra files selected for the reference spectra (specific file format : MFC)
  INDEX           indexRefMorning,indexRefAfternoon,                            // in automatic reference selection mode, index of selected records
                  indexRef,                                                     // in automatic reference selection mode, index of current selected record
                  indexRefScanBefore,indexRefScanAfter;

  double          ZmRefMorning,ZmRefAfternoon,Zm,Zm2,                           // in automatic reference selection mode, zenithal angles of selected records
                  oldZmRefMorning,oldZmRefAfternoon,                            // make a copy of previous zenithal angles
                  TimeDec,Tm,TimeDec2,Tm2,                                      // in automatic reference selection mode, measurement time of selected record
                  TDet;                                                         // temperature of reference

  double          resolFwhm;

  struct date     refDate;                                                      // in automatic reference selection mode, date of selected record
  int             displaySpectrum;                                              // force display spectrum
  int             displayResidue;                                               // force display residue
  int             displayTrend;                                                 // force display trend
  int             displayRefEtalon;                                             // force display alignment of reference on etalon
  int             displayFits;                                                  // force display fits
  int             displayPredefined;                                            // force display predefined parameters
  int             displayRef;

  int             displayFlag;                                                  // summary of the previous flag
  int             displayLineIndex;                                             // index of the current line
  int             hidden;                                                       // flag set if window is hidden e.g. for Kurucz calibration
  int             useKurucz;                                                    // flag set if Kurucz calibration is to be used for a new wavelength scale
  int             useUsamp;                                                     // flag set if undersampling correction is requested
  int             amfFlag;                                                      // flag set if there is a wavelength dependence of AMF for one or several cross sections
  int             useEtalon;                                                    // flag set if etalon reference is used
  int             xsToConvolute;                                                // flag set if high resolution cross sections to convolute real time
  int             xsToConvoluteI0;
  int             xsPukite;

  double         *LambdaRef,                                                    // absolute reference wavelength scale
                 *LambdaK,                                                      // new wavelength scale after Kurucz
                 *Lambda,                                                       // wavelength scale to use for analysis
                 *LambdaRadAsRef1,                                              // wavelength scale to use for RadAsRef
                 *LambdaRadAsRef2,                                              // wavelength scale to use for RadAsRef
                 *Deriv2RadAsRef1,                                              // second derivatives to interpolate RadAsRef
                 *Deriv2RadAsRef2,                                              // second derivatives to interpolate RadAsRef
                 *Sref,                                                         // reference spectrum
                 *SrefSigma,                                                    // error on reference spectrum
                 *SrefEtalon,                                                   // etalon reference spectrum
                 *SrefRadAsRef1,                                                // RadAsRef reference spectrum
                 *SrefRadAsRef2,                                                // RadAsRef reference spectrum
                  Shift,                                                        // shift found when aligning etalon on reference
                  Stretch,                                                      // stretch order 1 found when aligning etalon on reference
                  Stretch2,                                                     // stretch order 2 found when aligning etalon on reference
                  refNormFact,
                  chiSquare,                                                    // chi square
                  RMS;
  char           *ref_description;                                              // string describing spectra used in automatic reference.
  int             nIter;                                                        // number of iterations
  int             Decomp;                                                       // force SVD decomposition
  struct  fit_properties fit_properties;
  CROSS_REFERENCE TabCross[MAX_FIT];                                            // symbol cross reference
  CROSS_RESULTS   TabCrossResults[MAX_FIT];                                     // results stored per symbol in previous list
  bool           *spikes;                                                       // spikes[i] is true if the residual at pixel i has a spike
  bool           *omiRejPixelsQF;                                               // rejPixelsQF[i] is true if the pixel i is rejected based on pixels QF (OMI only)
  bool            useRefRow;                                                    // flag for using or skipping row in a reference file, 1 if use
  int             NTabCross;                                                    // number of elements in the two previous lists
  INDEX           indexSpectrum,                                                // index of raw spectrum in symbol cross reference
                  indexReference,                                               // index of reference spectrum in symbol cross reference
                  indexFwhmParam[MAX_KURUCZ_FWHM_PARAM],                        // index of 1st predefined parameter when fitting fwhm with Kurucz
                  indexFwhmConst,                                               // index of 'fwhm (constant)' predefined parameter in symbol cross reference
                  indexFwhmOrder1,                                              // index of 'fwhm (order 1)' predefined parameter in symbol cross reference
                  indexFwhmOrder2,                                              // index of 'fwhm (order 2)' predefined parameter in symbol cross reference
                  indexSol,                                                     // index of 'Sol' predefined parameter in symbol cross reference
                  indexOffsetConst,                                             // index of 'offset (constant)' predefined parameter in symbol cross reference
                  indexOffsetOrder1,                                            // index of 'offset (order 1)' predefined parameter in symbol cross reference
                  indexOffsetOrder2,                                            // index of 'offset (order 2)' predefined parameter in symbol cross reference
                  indexCommonResidual,                                          // index of 'Common residual' predefined parameter in symbol cross reference
                  indexUsamp1,                                                  // index of 'Undersampling (phase 1)' predefined parameter in symbol cross reference
                  indexUsamp2,                                                  // index of 'Undersampling (phase 2)' predefined parameter in symbol cross reference
                  indexResol;                                                   // index of 'Resol' predefined parameter in symbol cross reference

  int             OrthoSet[MAX_FIT];                                            // Vecteurs candidats à une orthogonalisation
  int             NOrtho;
  int             DifRLis;                                                      // Nombre de points pour le lissage du log du sp. de ref
  double         *fwhmPolyRef[MAX_KURUCZ_FWHM_PARAM],                           // polynomial coefficients for building wavelength dependence of fwhm for reference
                 *fwhmVector[MAX_KURUCZ_FWHM_PARAM],
                 *fwhmDeriv2[MAX_KURUCZ_FWHM_PARAM],
                  xmean,ymean;                                                  // resp. the spectrum and reference averaged on the fitting window
  int             analysisType;
  int             analysisMethod;
  int             bandType;
  double          refLatMin,refLatMax;
  double          refLonMin,refLonMax;
  int             NDET,
                  n_wavel_ref1,
                  n_wavel_ref2;
  int             gomeRefFlag;
  int             mfcRefFlag;
  RC              rcKurucz;
  int             SvdPDeb,SvdPFin,Dim,LimMin,LimMax,LimN;
  int             rc;
  enum            linear_offset_mode linear_offset_mode;
  int             longPathFlag;                                                 // for Anoop
  INDEX           indexRefOmi;
  int             newrefFlag,
                  useRadAsRef1,                                                 // flag ascertaining that refone is used for RadAsRef, 1 if so
                  useRadAsRef2;                                                 // flag ascertaining that reftwo is used for RadAsRef, 1 if so
  double          preshift;
  double          lambda0;                                                      // wavelength at the spectral window center (output and used for MMF)
  double          lambda0_pukite;                                               // selected wavelength for the normalization of cross sections when Pukite terms are calculated (by default, wavelength at the spectral window center)
  int             molecularCorrection;
};
#pragma pack(pop)

extern FENO         **TabFeno,*Feno;

extern int    ANALYSE_plotKurucz,ANALYSE_plotRef,ANALYSE_indexLine;
extern int    ANALYSE_swathSize;

extern char *ANLYS_crossAction[ANLYS_CROSS_ACTION_MAX];
extern char *ANLYS_amf[ANLYS_AMF_TYPE_MAX];

extern PRJCT_FILTER *ANALYSE_plFilter,*ANALYSE_phFilter;
extern MATRIX_OBJECT ANALYSIS_slitMatrix[NSFP],O3TD;
extern double *x,*Lambda,*LambdaSpec,
                    ANALYSIS_slitParam[NSFP],
                    *ANALYSE_pixels,
                    *ANALYSE_splineX,              // abscissa used for spectra, in the units selected by user
                    *ANALYSE_absolu,               // residual spectrum
                    *ANALYSE_secX,                 // residual spectrum + the contribution of a cross section for fit display
                    *ANALYSE_t,                    // residual transmission in Marquadt-Levenberg not linear method
                    *ANALYSE_tc,                   // residual transmission in Marquadt-Levenberg not linear method
                    *ANALYSE_xsTrav,
                    *ANALYSE_shift,
                    *ANALYSE_zeros,
                    *ANALYSE_ones,
  ANALYSE_nFree;

// ----------
// PROTOTYPES
// ----------

RC ANALYSE_Function (double *X, double *Y, const double *SigmaY, double *Yfit, int Npts,
                      double *fitParamsC, double *fitParamsF,INDEX indexFenoColumn, struct fit_properties *fitprops);
RC   ANALYSE_CheckLambda(WRK_SYMBOL *pWrkSymbol, const double *lambda, const int n_wavel);
RC   ANALYSE_XsInterpolation(FENO *pTabFeno, const double *newLambda,INDEX indexFenoColumn);
RC   ANALYSE_ConvoluteXs(const FENO *pTabFeno,int action,double conc,const MATRIX_OBJECT *pXs,
                         const MATRIX_OBJECT *slitMatrix,const double *slitParam, int slitType,
                         const double *newlambda, double *output, INDEX indexlambdaMin, INDEX indexlambdaMax, const int n_wavel,
                         INDEX indexFenoColumn, int wveDptFlag);
RC   ANALYSE_XsConvolution(FENO *pTabFeno,double *newLambda,MATRIX_OBJECT *slitMatrix,double *slitParam,int slitType,INDEX indexFenoColumn,int wveDptFlag);
RC   ANALYSE_SvdInit(FENO *feno, struct fit_properties *fit, const int n_wavel, const double *lambda);
RC   ANALYSE_CurFitMethod(INDEX indexFenoColumn, const double *Spectre, const double *SigmaSpec, const double *Sref, int n_wavel, double *residuals, double *Chisqr,int *pNiter,double speNormFact,double refNormFact, struct fit_properties *fit);
void ANALYSE_ResetData(void);
RC   ANALYSE_SetInit(ENGINE_CONTEXT *pEngineContext);
RC ANALYSE_fit_shift_stretch(int indexFeno, int indexFenoColumn, const double *spec1, const double *spec2, double *shift, double *stretch, double *stretch2, double *sigma_shift, double *sigma_stretch, double *sigma_stretch2);
RC   ANALYSE_AlignReference(ENGINE_CONTEXT *pEngineContext,int refFlag,void *responseHandle,INDEX indexFenoColumn);
RC   ANALYSE_Spectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

void ANALYSE_SetAnalysisType(INDEX indexFenoColumn);
RC   ANALYSE_LoadRef(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn);
RC   ANALYSE_LoadCross(ENGINE_CONTEXT *pEngineContext, const ANALYSIS_CROSS *crossSectionList,int nCross, const double *lambda,INDEX indexFenoColumn);
RC   ANALYSE_LoadLinear(ANALYSE_LINEAR_PARAMETERS *linearList,int nLinear,INDEX indexFenoColumn);
RC   ANALYSE_LoadNonLinear(ENGINE_CONTEXT *pEngineContext,ANALYSE_NON_LINEAR_PARAMETERS *nonLinearList,int nNonLinear,double *lambda,INDEX indexFenoColumn);
RC   ANALYSE_LoadShiftStretch(const ANALYSIS_SHIFT_STRETCH *shiftStretchList,int nShiftStretch,INDEX indexFenoColumn);
RC   ANALYSE_LoadGaps(ENGINE_CONTEXT *pEngineContext,const ANALYSIS_GAP *gapList,int nGaps,double *lambda,double lambdaMin,double lambdaMax,INDEX indexFenoColumn);
RC   ANALYSE_LoadOutput(const ANALYSIS_OUTPUT *outputList,int nOutput,INDEX indexFenoColumn);
RC   ANALYSE_LoadSlit(const PRJCT_SLIT *pSlit,int kuruczFlag);

RC   ANALYSE_Alloc(void);
void ANALYSE_Free(void);

RC   ANALYSE_UsampBuild(int analysisFlag,int gomeFlag,int indexFenoColumn);
void ANALYSE_UsampGlobalFree(void);
RC   ANALYSE_UsampGlobalAlloc(double lambdaMin,double lambdaMax,int size);
RC   ANALYSE_UsampLocalAlloc(int gomeFlag);
void ANALYSE_UsampLocalFree(void);

enum _pixelSelection {
  PIXEL_BEFORE,
  PIXEL_AFTER,
  PIXEL_CLOSEST
};

RC FNPixel (double *lambdaVector, double lambdaValue, int npts,int pixelSelection);

extern double center_pixel_wavelength(int first, int last);

#endif
