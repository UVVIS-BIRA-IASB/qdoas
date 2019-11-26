
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  SAOZ read out routines
//  Name of module    :  SAOZ-READ.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in 97
//
//  Ref               :  CNRS, France
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
//  This module contains routines needed to read the data from SAOZ instruments.
//  These instruments have been developed by J-P Pommereau (CNRS, France) at the
//  end of the eighties to provide NO2 and O3 total columns.  Due to a very
//  simple concept, they have been installed at a large number of NDSC stations.
//
//  Two formats are described in this package :
//
//        PRJCT_INSTR_FORMAT_SAOZ_[VIS/UV] for 512 VIS/UV detectors;
//        PRJCT_INSTR_FORMAT_SAOZ_EFM for 1024 detectors (EFM format);
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  Ht_Reli - read the data from HP file in SAOZ 512 format and bytes inversion;
//
//  SetSAOZ - calculate the size and the number of records for a new file
//            in SAOZ 512 format;
//
//  ReliSAOZ - read SAOZ spectra in 512 format
//
//  SetSAOZEfm - calculate the size and the number of records for a new file
//               in SAOZ 1024 EFM format;
//
//  ReliSAOZEfm - read SAOZ spectra in 1024 EFM format
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "winsites.h"
#include "winthrd.h"
#include "vector.h"
#include "zenithal.h"
#include "stdfunc.h"

// ===============
// 512 SAOZ FORMAT
// ===============

#define PIXMAXUV 210    // number of pixels really used in UV spectra

// ----------------------
// Description of records
// ----------------------

#pragma pack(push,1)

typedef struct _saozUV
 {
  short  Spec[201],                                                             // encrypted spectrum
         Ind[30],                                                               // coefficients needed for building original spectrum
         Param[9],                                                              // encrypted measurements data (integration time, detector temperature ...
         Coef[8];                                                               // coefficients needed for building original measurements data
 }
SAOZ_UV;

typedef struct _saozV
 {
  short  Spec[503],                                                             // encrypted spectrum
         Ind[45],                                                               // coefficients needed for building original spectrum
         Param[9],                                                              // encrypted measurements data (integration time, detector temperature ...
         Coef[8];                                                               // coefficients needed for building original measurements data
 }
SAOZ_V;

typedef union _saoz
 {
  SAOZ_V Sv;                                                                    // Visible
  SAOZ_UV Suv;                                                                  // UV
 }
SAOZ;

#pragma pack(pop)

// ---------------------------------------
// Arrays with available integration times
// ---------------------------------------

static double Tps[] = { 0.027, 0.037, 0.052, 0.072, 0.1, 0.14, 0.19, 0.27,
                       0.37, 0.52, 0.72, 1., 1.4, 1.9, 2.7, 3.7, 5.2, 7.2,
                       10., 14., 19., 27., 37., 52., 72., 100., 140., 190.,
                       270., 370., 520., 720. };

// ----------------------------------------
// HP->DOS FILES FORMAT CONVERSION ROUTINES
// ----------------------------------------

// -----------------------------------------------------------------------------
// FUNCTION      Ht_Reli
// -----------------------------------------------------------------------------
// PURPOSE       Read the data from HP file in SAOZ 512 format and bytes inversion
//
// INPUT         fp      pointer to the spectra file;
//               Size    the number of bytes to read;
//               Byte    the number of bytes of individual items (4 for double, ...)
//
// OUTPUT        ptr     the receiving buffer;
//
// RETURN        0 when the end of file is reached; 1 otherwise.
// -----------------------------------------------------------------------------

int Ht_Reli(FILE *fp,char *Ptr,unsigned int Size,int Byte)
 {
  // Declarations

  unsigned int i;
  char c;

  // Bytes read out

  if (!fread(Ptr,Size,1,fp))
   return(0);

  // Bytes inversion for each item in the buffer

  switch ( Byte )
   {
    // Array of integer

    case 2 : for ( i=0; i<Size; i+=2 )

                 { c = Ptr[i]; Ptr[i] = Ptr[i+1]; Ptr[i+1] = c; }

             break;

    // Array of double

    case 8 : for ( i=0; i<Size; i+=8 )

                 { c = Ptr[i+0]; Ptr[i+0] = Ptr[i+7]; Ptr[i+7] = c;
                   c = Ptr[i+1]; Ptr[i+1] = Ptr[i+6]; Ptr[i+6] = c;
                   c = Ptr[i+2]; Ptr[i+2] = Ptr[i+5]; Ptr[i+5] = c;
                   c = Ptr[i+3]; Ptr[i+3] = Ptr[i+4]; Ptr[i+4] = c;   }

             break;
   }

  return ( 1 );
 }

// -----------------------------------------------------------------------------
// FUNCTION      SetSAOZ
// -----------------------------------------------------------------------------
// PURPOSE       calculate the size and the number of records for a new file
//               in SAOZ 512 format
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the spectra file to read;
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//               pEngineContext->recordSize, the size of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC SetSAOZ (ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  int recordSize;                                                               // size of record
  int domain;
  SAOZ saoz;                                                                    // data record
  RC rc;                                                                        // return code

  // Initializations

  domain=pEngineContext->project.instrumental.saoz.spectralRegion;

  recordSize=(domain==PRJCT_INSTR_SAOZ_REGION_VIS)?sizeof(SAOZ_V):sizeof(SAOZ_UV);
  pEngineContext->recordNumber=pEngineContext->recordSize=0;
  rc=ERROR_ID_NO;

  // Verify the input pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("SetSAOZ",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Header read out

    fseek(specFp,256L,SEEK_SET);

    if (!Ht_Reli(specFp,(char *)&saoz,recordSize,2))
     rc=ERROR_SetLast("SetSAOZ",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
    else
     {
      // Calculate the size of records

      switch((domain==PRJCT_INSTR_SAOZ_REGION_VIS)?saoz.Sv.Param[3]:saoz.Suv.Param[3])
       {
     // -----------------------------------------------------------------------
        case 1990 :
        case 1991 :
         pEngineContext->recordSize = recordSize-6L;
        break;
     // -----------------------------------------------------------------------
        default :
         pEngineContext->recordSize = recordSize;
        break;
     // -----------------------------------------------------------------------
       }

      pEngineContext->recordNumber=(int)((int32_t)STD_FileLength(specFp)-256L)/pEngineContext->recordSize;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliSAOZ
// -----------------------------------------------------------------------------
// PURPOSE       Read SAOZ spectra in 512 format
//
// INPUT         pEngineContext : information on the file to read out
//               recordNo  : the index of the record to read out
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               namesFp   : pointer to the names file
//               darkFp    : pointer to the dark currents file
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_FILE_NOT_FOUND : the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy input criteria
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliSAOZ(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  SAOZ              saoz;                                                       // current record
  int               IndSec[100], N, IndMin, Base, Co1, Co2,                     // data read out from the input file
                    i, j, k;                                                    // indexes for loops and arrays
  int               domain;                                                     // spectral region
  char              str[6], *string,                                            // string buffers and pointers
                   *pSaoz,                                                      // pointer to the current record
                    names[20];                                                  // names of the current record
  const char *DIGITS="0123456789";

  struct date       day;                                                        // date of measurement in terms of day, month and year
  OBSERVATION_SITE *pSite;                                                      // pointer to the observation site in list
  double            longit,latit,azimuth,                                       // geolocation coordinates of observation site
                   *spectrum,                                                   // substitution variable for spectrum
                    invSum,                                                     // inverse of the number of scans
                    Coef1, Coeff, InvCoef, L,                                   // temporary variables used for decoding the spectrum and measurement data
                    tmLocal;                                                    // measurement local time in seconds
  INDEX             indexSite;                                                  // index of the observation site in list
  short            *spec,*ind,*param,*coef;                                     // substitution variables
  RC                rc;                                                         // return code

  // Initializations
  const int n_wavel = NDET[0];

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  domain=pEngineContext->project.instrumental.saoz.spectralRegion;

  if (domain==PRJCT_INSTR_SAOZ_REGION_VIS)
   {
    pSaoz=(char *)&saoz.Sv;
    spec=saoz.Sv.Spec;
    ind=saoz.Sv.Ind;
    param=saoz.Sv.Param;
    coef=saoz.Sv.Coef;
   }
  else
   {
    pSaoz=(char *)&saoz.Suv;
    spec=saoz.Suv.Spec;
    ind=saoz.Suv.Ind;
    param=saoz.Suv.Param;
    coef=saoz.Suv.Coef;
   }

  memset(names,0,20);
  spectrum=pBuffers->spectrum;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliSAOZ",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Set file pointers

  else
   {
    if (namesFp!=NULL)
     fseek(namesFp,(int32_t)20L*(recordNo-1)+260L,SEEK_SET);

    fseek(specFp,(int32_t)pEngineContext->recordSize*(recordNo-1)+256L,SEEK_SET);

    // Record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||
         !Ht_Reli(specFp,pSaoz,((domain==PRJCT_INSTR_SAOZ_REGION_VIS)?sizeof(SAOZ_V):sizeof(SAOZ_UV))-16,2))
     rc=ERROR_ID_FILE_END;
    else
     {
      // Measurement data

      switch ( param[3] )
       {
     // -----------------------------------------------------------------------
        case 1990 :

         Ht_Reli ( specFp, (char *)coef, 10, 2 );

         pRecord->BestShift = (double) param[8];
         pRecord->NSomme    = (int) param[7];
         pRecord->Tint      = (double) param[6]*0.01;

         pRecord->Tm   =

              ( (double) coef[4] + 32768. ) +
              ( (double) coef[3] + 32768. ) * 65536. + 2.107123407E11 -
              ( (double) pRecord->NSomme * pRecord->Tint * 0.5 );

        break;
     // -----------------------------------------------------------------------
        case 1991 :

         Ht_Reli ( specFp, (char *)coef, 10, 2 );

         pRecord->BestShift = (double) param[8];
         pRecord->NSomme    = (int) param[7];
         pRecord->Tint      = (double) Tps[param[6]];

         pRecord->Tm   =

              ( (double) coef[4] + 32768. ) +
              ( (double) coef[3] + 32768. ) * 65536. + 2.107123407E11 -
              ( (double) pRecord->NSomme * pRecord->Tint * 0.5 );

        break;
     // -----------------------------------------------------------------------
        default :

         Ht_Reli ( specFp, (char *)coef, 16, 2 );

         pRecord->BestShift = (double)coef[7];
         pRecord->NSomme    = (int)coef[4];
         pRecord->Tint      = (double)coef[3]*0.01;

         pRecord->Tm   =

              ( (double) coef[6] + 32768. ) +
              ( (double) coef[5] + 32768. ) * 65536. + 2.107123407E11 -
              ( (double) pRecord->NSomme * pRecord->Tint * 0.5 );

        break;
     // -----------------------------------------------------------------------
       }

      if (namesFp!=NULL)
       names[15]=0;

      strncpy(pRecord->Nom,names,20);

      // Geolocation coordinates

      if ((indexSite=SITES_GetIndex(pEngineContext->project.instrumental.observationSite))!=ITEM_NONE)
       {
        pSite=&SITES_itemList[indexSite];

        longit=-pSite->longitude;  // sign is inverted
        latit=pSite->latitude;

        pRecord->Zm = ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longit,&latit,&azimuth);
       }
      else
       pRecord->Zm=(double)atof(pRecord->Nom+12)*0.1;

      pRecord->Azimuth  = (double)-1;

      // Date and time of the current measurement

      day.da_year  = ZEN_FNCaljye (&pRecord->Tm);
      day.da_mon   = (char) ZEN_FNCaljmon (ZEN_FNCaljye(&pRecord->Tm),ZEN_FNCaljda(&pRecord->Tm));
      day.da_day   = (char) ZEN_FNCaljday (ZEN_FNCaljye(&pRecord->Tm),ZEN_FNCaljda(&pRecord->Tm));

      // Fill data

      string = ZEN_FNCaljti ( &pRecord->Tm, str );

      memcpy(&pRecord->present_datetime.thedate, &day, sizeof(day));

      pRecord->present_datetime.thetime.ti_hour = (unsigned char)( ( strchr(DIGITS,string[0]) - DIGITS ) * 10 + ( strchr(DIGITS,string[1]) - DIGITS ) );
      pRecord->present_datetime.thetime.ti_min  = (unsigned char)( ( strchr(DIGITS,string[3]) - DIGITS ) * 10 + ( strchr(DIGITS,string[4]) - DIGITS ) );
      pRecord->present_datetime.thetime.ti_sec  = (unsigned char)0;

      pRecord->TDet = (double) param[0] * 0.08138 - 273.1;
      pRecord->TotalExpTime=pRecord->TotalAcqTime = (double) pRecord->NSomme*pRecord->Tint;
      pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.;

      tmLocal=pRecord->Tm+THRD_localShift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

      Base = (int)coef[0];
      Co1  = (int)coef[1];
      Co2  = (int)coef[2];

      // Rebuild the original spectrum

      VECTOR_Init ( spectrum, (double) 1., n_wavel );

      for ( i=0, k=(domain==PRJCT_INSTR_SAOZ_REGION_VIS)?45:30; i<k; IndSec[i] = (int)ind[i], i++ );
      for ( j=0, k=(domain==PRJCT_INSTR_SAOZ_REGION_VIS)?100:70; i<k; IndSec[i++] = (int)spec[j++] );

      if ((Co1<(int)1) ||
         ((Coef1=(double)Co2*log((double)2.)+log((double)Co1))>(double)709.) ||
         ((Coeff=(double)exp((double)Coef1))==(double)0.) ||
          (dateFlag && (pRecord->localCalDay!=localDay)))

       rc=ERROR_ID_FILE_RECORD;

      else
       {
        InvCoef = (double) 1. / Coeff;

        i=9; N=0; k=(domain==PRJCT_INSTR_SAOZ_REGION_VIS)?n_wavel:PIXMAXUV;
        do
         {
          L = (double) 32767. * (int) ( IndSec[N]/1000 );
          j = ( IndSec[N++]%1000 ) - 1;
          while ( i<j ) spectrum[i++] = (double) L;
         }
        while ( j < k );

        IndMin = (N>((domain==PRJCT_INSTR_SAOZ_REGION_VIS)?45:30)) ? N-((domain==PRJCT_INSTR_SAOZ_REGION_VIS)?36:21) : 9;
        j = IndMin - 9;
        i = 9;

        while ( i<IndMin ) spectrum[i++] = (double) 1.;

        // Rebuild the spectrum

        while ( i < k )
         {
          spectrum[i]=(double)(spectrum[i]+spec[j++])*InvCoef+Base;
          i++;
         }

        for (i=0,invSum=(double)1./pRecord->NSomme;i<n_wavel;i++)
         {
          spectrum[i]*=invSum;
          if ( spectrum[i] < (double) 1. )
           spectrum[i] = (double) 1.;
         }
       }
     }

    if ((strlen(names)>0) &&
      (((pEngineContext->project.instrumental.saoz.spectralType==PRJCT_INSTR_SAOZ_TYPE_ZENITHAL) && (names[11]!='Z')) ||
       ((pEngineContext->project.instrumental.saoz.spectralType==PRJCT_INSTR_SAOZ_TYPE_POINTED) && (names[11]!='P'))))
     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }

// ===============
// 1024 EFM FORMAT
// ===============

// ---------------------------------------------------------------------------------------
// Description of the records (see Eric Dalmeida, CNRS, for further details on the format)
// ---------------------------------------------------------------------------------------

typedef struct
 {
  unsigned char            Exist,TailleHead;
  unsigned short TailleSpec;
  unsigned short NumSpec,Code;
  unsigned char            M_An,M_Mois,M_Jour;
  unsigned char            M_Heur,M_Min,M_Sec;
  short int       Longi,Latid;
  unsigned short           Altit,N_somm;
  unsigned char            iT_int,CrcS;
  short int       T_det,T_cais;
  short int       Dizen,Shift;
  unsigned char            P_com;
  unsigned char            R_Heur,R_Min,R_Sec;
  unsigned short            Param[8];
  int32_t            GPStim;
  short int       Tout,Tsond;
  unsigned short            Press;
  unsigned char            Humid;
  unsigned char            Libre;
 }
EFM_1024;

typedef struct
 {
  char            Exist,TailleHead;
  short            TailleSpec;
  short            NumSpec,Code;
  char            M_An,M_Mois,M_Jour;
  char            M_Heur,M_Min,M_Sec;
  short int               Longi,Latid;
  short            Altit,N_somm;
  char            iT_int,CrcS;
  short int               T_det,T_cais;
  short int               Dizen,Shift;
/*  BYTE            P_com;
  BYTE            R_Heur,R_Min,R_Sec;
  WORD            Param[8];
  long            GPStim;
  short int       Tout,Tsond;
  WORD            Press;
  BYTE            Humid;
  BYTE            Libre;*/
  float tempe[8]; /* u 6 */
  int num_spec; /* u 1 */
  double date; /* u 2 number of days since 1899/12/30 */
  double longitude; /* u 3 */
  double latitude; /* u 3 */
  double altitude; /* u 3 */
  double sza; /* u 4 */
  double t_int; /* u 5 conf.spectros */
  int nsomme; /* u 5 conf.spectros */
  double tdet;
 }
EFM_2048;

// -------------------------------------
// Array of authorized integration times
// -------------------------------------

static double Texpos[] =
 {
  0.027, 0.037, 0.052, 0.072, 0.10, 0.14, 0.19, 0.27, 0.37,
  0.52, 0.72, 1.0, 1.4, 1.9, 2.7, 3.7, 5.2, 7.2, 10.0, 14.0,
  19.0, 27.0, 37.0, 52.0, 72.0, 100.0, 140.0, 190.0, 270.0,
  370.0, 520.0, 720.0
 };

// -----------------------------------------------------------------------------
// FUNCTION      SetSAOZEfm
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of records for a new file
//               in SAOZ 1024 EFM format
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the spectra file to read;
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC SetSAOZEfm(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  unsigned int curvenum;                    // number of spectra in the file
  RC rc;                            // return code

  // Initializations

  rc=ERROR_ID_NO;
  pEngineContext->recordNumber=0;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("SetSAOZEfm",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (fread(&curvenum,sizeof(int),1,specFp) && (curvenum>0))
   pEngineContext->recordNumber=curvenum;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliSAOZEfm
// -----------------------------------------------------------------------------
// PURPOSE       Read SAOZ spectra in 1024 EFM format
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_FILE_NOT_FOUND : the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliSAOZEfm(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  EFM_1024     header;                                                          // record header
  double      *spectrum,SMax;                                                   // the current spectrum and its maximum value
  double       tmLocal;                                                         // measurement local time
  struct date  today;                                                           // date of the current measurement
  INDEX        i;                                                               // browse pixels in the spectrum
  RC           rc;                                                              // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  spectrum=(double *)pEngineContext->buffers.spectrum;
  SMax=(double)0.;
  rc=ERROR_ID_NO;
  const int n_wavel = NDET[0];

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliSAOZEfm",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    // Complete the reading of the record

    fseek(specFp,(int32_t)sizeof(unsigned int)+(recordNo-1)*sizeof(EFM_1024),SEEK_SET);
    fread(&header,sizeof(EFM_1024),1,specFp);
    fseek(specFp,(int32_t)sizeof(unsigned int)+(recordNo-1)*sizeof(double)*n_wavel+pEngineContext->recordNumber*sizeof(EFM_1024),SEEK_SET);
    fread(spectrum,sizeof(double)*n_wavel,1,specFp);

    if ((today.da_year=header.M_An)<30)
     today.da_year+= 2000;
    else if (today.da_year<130)
     today.da_year+= 1900;
    else if (today.da_year<1930)
     today.da_year+= 100;

    today.da_mon=header.M_Mois;
    today.da_day=header.M_Jour;

    // Build the spectrum

    if (header.N_somm>0)
     for (i=1,SMax=(double)spectrum[0];i<n_wavel;i++)
      {
       if ((double)spectrum[i]>SMax)
        SMax=(double)spectrum[i];
       spectrum[i]/=header.N_somm;
      }

    // Data on the current spectrum

    pRecord->TDet = (double)header.T_det*0.01;
    pRecord->Tint = (double)Texpos[header.iT_int];
    pRecord->NSomme = header.N_somm;
    pRecord->Zm = (double)header.Dizen*0.01;

    pRecord->present_datetime.thetime.ti_hour=header.M_Heur;
    pRecord->present_datetime.thetime.ti_min=header.M_Min;
    pRecord->present_datetime.thetime.ti_sec=header.M_Sec;

    memcpy(&pRecord->present_datetime.thedate, &today, sizeof(today));

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
    pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)header.N_somm*pRecord->Tint;
    pRecord->TimeDec=(double)header.M_Heur+header.M_Min/60.+header.M_Sec/3600.;

    pRecord->longitude=(double)-header.Longi/100.;
    pRecord->latitude=(double)header.Latid/100.;
    pRecord->altitude=(double)header.Altit;

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    if ((SMax<=(double)1000.) || (header.N_somm<=0) || (dateFlag && (pRecord->localCalDay!=localDay)))
     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }
