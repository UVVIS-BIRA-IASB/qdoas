#ifndef MFC_READ_H
#define MFC_READ_H

#include <stdio.h>

#include "doas.h"

#pragma pack(push, 1)

struct _TOldFlags {
  int   mode;
  int   smooth;
  int   deg_reg;
  char Null[8];
  char Ref[8];
};

typedef struct _TBinaryMFC {
  char     version[20];           //     version number (not of interest)                           0
  int       no_chan;              // !!! number of channels - 1 (usually 1023)                      20
  char p_spec_32bit[4];           // 32-bit pointer used by DOASIS at runtime (?).
  char     specname[20];          //     optional name of the spectrum                              32
  char     site[20];              //     name of measurement site                                   52
  char     spectroname[20];       //     name of spectrograph                                       72
  char     scan_dev[20];          //     name of scan device, e.g. PDA                              92
  char     first_line[80];                                                               //        112
  float     elevation;             //     elevation viewing angle                        //        192
  char     spaeter[72];
  int       ty;                    //     spectrum flags, can be used to distinguish between
                                   //     different types of spectrum (e.g. straylight,
                                   //     offset, dark current...
  char     dateAndTime[28]; //  this is a char date[9], char start_time[9], stop_time[9], char dummy;
  int       low_lim;
  int       up_lim;
  int       plot_low_lim;
  int       plot_up_lim;
  int       act_chno;
  int       noscans;               // !!! number of scans added in this spectrum
  float     int_time;              // !!! integration time in seconds
  float     latitude;              //     latitude of measurement site
  float     longitude;             //     longitude of measurement site
  int       no_peaks;
  int       no_bands;
  float     min_y;                 //     minmum of spectrum
  float     max_y;                 //     maximum of spectrum
  float     y_scale;
  float     offset_Scale;
  float     wavelength1;           // !!! wavelength of channel 0
  float     average;               //     average signal of spectrum
  float     dispersion[3];         // !!! dispersion given as a polynomial:
                                   //     wavelength=wavelength1 + dispersion[0]*C + dispersion[1]*C^2
                                   //                            + dispersion[2]*C^3;   C: channel number
                                   //                              (0..1023)
  float     opt_dens;
  struct _TOldFlags OldFlags;
  char     FileName[8];           //     filename of spectrum
  char     backgrnd[8];
  int       gap_list[40];
  char      p_comment_32bit[4];
  int       reg_no;
  char p_prev_32bit[4], p_next_32bit[4];   //  2 32-bit pointers, presumably used by DOASIS at runtime
} TBinaryMFC;

#pragma pack(pop)

extern TBinaryMFC MFC_header;
extern char MFC_fileDark[MAX_STR_SHORT_LEN+1],  // dark current file name
  MFC_fileOffset[MAX_STR_SHORT_LEN+1];  // offset file name

RC MFC_LoadOffset(ENGINE_CONTEXT *pEngineContext);
RC MFC_LoadDark(ENGINE_CONTEXT *pEngineContext);
RC MFC_ReadRecord(char *fileName,TBinaryMFC *pHeaderSpe,double *spe,TBinaryMFC *pHeaderDrk,double *drk,TBinaryMFC *pHeaderOff,double *off,unsigned int mask,unsigned int maskSpec,unsigned int revertFlag);
RC MFC_ReadRecordStd(ENGINE_CONTEXT *pEngineContext,char *fileName,
                     TBinaryMFC *pHeaderSpe,double *spe,
                     TBinaryMFC *pHeaderDrk,double *drk,
                     TBinaryMFC *pHeaderOff,double *off);
RC    MFC_ResetFiles(ENGINE_CONTEXT *pEngineContext);
INDEX MFC_SearchForCurrentFileIndex(ENGINE_CONTEXT *pEngineContext);
int   MFC_AllocFiles(ENGINE_CONTEXT *pEngineContext);
RC    SetMFC(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC    ReliMFC(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,unsigned int mfcMask);
RC    ReliMFCStd(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC    MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC    MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

RC MFC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

#endif
