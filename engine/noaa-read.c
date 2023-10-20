
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Read spectra from measurements performed by the NOAA
//  Name of module    :  NOAA-read.c
//  Creation date     :  13 June 2005
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
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  This module has been written according to the format description provided by
//  Megan MELAMED and Roy MILLER, NOAA - Aeronomy Laboratory at Boulder.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetVmeSpec - calculate the number of spectra measured the current day;
//  VmeSpecReadRecord - read a record in the case of individual spectra in several files;
//  ReliVmeSpec - read a record in the VmeSpec format.
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
#include "bin_read.h"
#include "winthrd.h"
#include "zenithal.h"
#include "stdfunc.h"

// =======================
// DEFINITION OF CONSTANTS
// =======================

#define    MaxNumGaps               30
#define MaxNumPnt              1024
#define MaxBandNum               31

#define NOAA_OFFSET   4854L                                                     // the offset in bytes for observations

enum       DataRecordGenerationType    {Suitcase,Hybrid,Hybrid2,Suitcase2,Kludge,Import,Hybrid3,Kludge2};
enum       DataClassType                  {FixedPattern,RawData,Crossection,Spectrum,Residual,DetResponse,SGResponse,Deviation,FitResidual,LeakCurrent};
enum       ObservationType          {Field,Air,Lab};
enum       LightSourceType          {DirectSun,DirectMoon,Sky,TungstenStrip,QuartzHalogen,
                                                       XeArc,Helium,Neon,Argon,Xenon,Krypton,Mercury,
                                                       Dark,Tritium,Flashlight,OffAxisSky,S16,S17,S18,S19,S20};
enum       PreselectorNameType      {None,TripleFilter,DoubleJY,T3,T4,T5,T6,T7,T8,T9,T10};

// ========================
// DEFINITION OF STRUCTURES
// ========================

// Description of the format (as provided by Megan MELAMED and Roy MILLER from NOAA)

#pragma pack(1)

typedef struct TimeRecordType
 {
        unsigned int macTime;                                                                  // {seconds since 1904}
        float zone;                                                                            // {Hours to add to macTime to obtain GMT}
    }
NOAA_TimeRecordType;

typedef struct PlaceRecordType
 {
        char name[256];
        float latitude;                                                                     //    {degrees, North positive }
        float longitude;                                                                 //    {degrees, East positive  }
        float height;                                                                             //    {meters above sea level }
        float zone;                                                                      //    {hours to ADD to local time to produce universal time (GMT)}
    }
NOAA_PlaceRecordType;

typedef struct    FieldRecordType
 {
        char observingConditions[256];
        short lightSource;                                                            // LightSourceType
        NOAA_PlaceRecordType placeRecord;
        float pressure;                                                               // {millibars               }
        float temperature;                                                            // {degrees centigrade      }
        float humidity;                                                               // {percent relative        }
        float windSpeed;                                                              // {meters per second       }
        float windDirection;                                                          // {degrees, East positive  }
        float sunAzimuth;                                                             // {degrees, East positive  }
        float sunZenith;                                                              // {degrees, from vertical  }
        float moonAzimuth;                                                            // {degrees, East positive  }
        float moonZenith;                                                             // {degrees, from vertical  }
        float viewAzimuth;                                                            // {degrees, East positive  }
        float viewZenith;                                                             // {degrees, from vertical  }
        float spare1;                                                                 //
        float spare2;                                                                 //
        float spare3;                                                                 //
        float spare4;                                                                 //
    }
NOAA_FieldRecordType;

typedef struct    AircraftRecordType
 {
        char observingConditions[256];
        short lightSource;                                                            // LightSourceType
        float latitude;                                                               // {degrees, North positive }
        float longitude;                                                              // {degrees, East positive  }
        float heading;                                                                // {degrees, clockwise from north}
        float pitch;                                                                  // {degrees, Nose up positive}
        float roll;                                                                   // {Right wing down positive}
        float yaw;                                                                    // {Right of heading positive}
        float trueAirSpeed;                                                           // {meters per second       }
        float altitude;                                                               // {meters, Above sea level }
        float pressure;                                                               // {millibars               }
        float temperature;                                                            // {degrees centigrade      }
        float humidity;                                                               // {percent relative        }
        float windSpeed;                                                              // {meters per second       }
        float windDirection;                                                          // {degrees, East positive  }
        float sunAzimuth;                                                             // {degrees, East positive  }
        float sunZenith;                                                              // {degrees, from vertical  }
        float moonAzimuth;                                                            // {degrees, East positive  }
        float moonZenith;                                                             // {degrees, from vertical  }
        float viewAzimuth;                                                            // {degrees, East positive  }
        float viewZenith;                                                             // {degrees, from vertical  }
    }
NOAA_AircraftRecordType;

typedef struct    LaboratoryRecordType
 {
        char observingConditions[256];
        char gasName[256];
        short lightSource;                                                            // LightSourceType
        float concentration;                                                          // {Number per cubic cm     }
        float pathLength;                                                             // {Centimeters             }
        float pressure;                                                               // {Millibars               }
        float temperature;                                                            // {Degrees centigrade      }
        float flow;                                                                   // {Cubic CM. per second    }
        float crossectScaleFactor;                                                    // {Crossections only       }
 }
NOAA_LaboratoryRecordType;

typedef struct    ForeOpticsRecordType
 {
        short preselectorName; //    PreselectorNameType;
        short filterNumber;
        float dialSetting;
        float lambdaMax;
        float lambdaMin;
        char  polarizerInUse;
        char  foDummy;
        short polarizerAngle;
    }
NOAA_ForeOpticsRecordType;

typedef struct    SpectrometerRecordType
 {
        short spectrometerNumber;
        short orderNumber;
        float dialSetting;
        float gratingLinesPerMM;
        float gratingInputAngle;
        float gratingOutputAngle;
        float cameraFocalLength;
        short spacer;                                                                 // {unused - defaulted to $3F80 **}
        short band;
        float wavelengthAtDetCenter;                                                  // {nanometers**}
        float dispersion;                                                             // {nm/diode**}
        float entranceSlitWidth;                                                      // {microns                 }
        float spectrometerTempTop;                                                    // {¡C at top of spectrometer (inner) case*}
        float spectrometerTempBot;                                                    // {¡C at bottom of spectrometer (inner) case*}
    }
NOAA_SpectrometerRecordType;

//    {*Note: After a int32_t hiatus, temperature measurements were resumed, starting with
//     BlackSG2.23. The "Bottom" temp is now the ROOM temperature outside the instrument case!}
//     {**Until13 Apr 93, the space used by 'spacer' and 'band' was used for a Real named
//      'finalMagnification'. This was never properly used, and was always defaulted to 1.00,
//      so we took the $0000 half for band.  The 'spacer' half is $3F80 = 16256 (bit pattern
//      from writing 1.00 in all four bytes). When 'band' > zero, the definitions for
//      'wavelengthAtDetCenter' and 'dispersion' should be looked up (using GetBandInfo2),
//      rather than using the values directly.}

typedef struct    DetectorRecordType
 {
        short detectorNumber;
        short numberOfDiodes;
        float diodeWidth;                                                             // {microns                 }
        float detectorTemperature;                                                    // {degrees Centigrade      }
 }
NOAA_DetectorRecordType;

typedef struct    ExposureDiodeRecordType
 {
        char  xdInUse;                                                                //
        char  xdNewDarkCountRate;                                                     // {true = not just a copy of earlier measurement}
        float xdDarkCountRate    ;                                                          // {counts/second}
        int   xdTerminalCount;
 }
NOAA_ExposureDiodeRecordType;

typedef struct    WavelengthRecordType
 {
        char  wlExists;
        char  wlSpareBool;
        float wlDispToCenter;
        float wlCoeff[4];
        char wlMethod[12];                                                           // {Program/version used to calibrate.}
        int   wlID0;                                                                  // {These fields identify the calibration standard,É    }
        int   wlID1;                                                                  // {using pointers,times, or recNums (to be determined).}
 }
NOAA_WavelengthRecordType;

// {If "wlExists" = true, this info provides a wavelength scale for "spectralArray"
//  (defined below). The wavelength for element number K of "spectralArray" is:
//  WAVELENGTH(K) = ·( wlCoeff[N] * ((K - K0)/1023)^N),
//  where N = 0,1,2,3 and K0 = "wlDispToCenter" + the smallest index of "spectralArray".
//  In programs using this Pascal interface, K0 = wlDispToCenter + 1, but in C and LabVIEW
//  programs (which index "spectralArray" from 0..1023) K0 = wlDispToCenter. This info is
//  added to support development of LabVIEW routines prior to adoption of a proper data base.}

typedef struct    DataRecordType
 {
     char dataClass;                                                                        // DataClassType
        char generation;                                                                    // DataRecordGenerationType

        // {Fields in this section must total 256 bytes exactly!
        //  They replace "fileNameOfStandard :Str255".}

        char  patternApplied;
        char  detResponseApplied;
        char  sgResponseApplied;
        char  exitSlitApplied;
        float dcOffset;                                                               // {Offset & clock noise Fourier components for aÉ}
        float f2;                                                                     // {Ésingle read. These fields are filled by BlackSGÉ}
        float f4Sin;                                                                  // {Éwhen pattern applied is "clock" or "clk + offset.}
        float f4Cos;                                                                  // {Amplitude is in DN's.}
        short patternRecNum;                                                          // {Originally intended to be a record number of
                                                                                                           //  FixedPattern applied. Now it indicates the
                                                                                                           //  method selected for pattern removal. 0=none,
                                                                                                           //  1=bkgd,2=clock,3=offset,4=clk+offset.
                                                                                                           //  -1=cleared value when the field meant recNum.}
        short detResponseRecNum;                                                      // {recNum in file ReticonNN.rsp--never used}
        short sgResponseRecNum;                                                       // {recNum in file SpectrographMM.rsp--never used}
        short numberOfReads;
        char  strayLightRemoved;                                                      // {true=scattered sunlight spectrum was subtracted; used only by Jerry.}
        char  dataFlipped;                                                            // {spectralData has been turned end-for-end, probably
                                                                                                           //  because detector was mounted upside-down. This field
                                                                                                           //  is needed to indicate that data from "bad" diode J will
                                                                                                           //  appear in spectralData[1025 - J].}
        short strayLightRecNum;                                                       // {recNum in the SAME .spc file--used only by Jerry}
        short firstSaturated;                                                         //
        short lastSaturated;                                                                   // {diode number}
        char  progVersion[12];                                                        // {String[x]; total storage is x+1 bytes!}
        short externalMod;                                                                     // {mods made by "external" programs, 0=no mods}
        short internalMod;                                                                     // {mods made by "official" programs, 0=no mods}

        NOAA_ExposureDiodeRecordType  exposureDiodeRecord;                             //  ExposureDiodeRecordType
        NOAA_WavelengthRecordType  wavelengthRecord;                                   //  WavelengthRecordType

        short exposureTime;                                                           // {seconds elapsed during summation (incl. dead time)}
        float aveDeviation;
        short spare[73];
                             // {End of size-sensitive section}

        short  integrationCycles;
        float integrationTime;                                                              // {seconds                }
        float centerOfBrightness;                                                           // {seconds                }
        float shift;                                                                        // {diodes                    }
        float stretch;                                                                      // {diodes                    }
        float diodesInExitSlit;                                                             // {diodes applied as exit slit, or recommended value}
        float maxData;                                                                      // {Maximum data value        }
        float minData;                                                                      // {Minimum data value        }
        float aveData;                                                                      // {Average value of data    }
        short numberOfGapsInteger;
        short startGap[MaxNumGaps];
        short endGap[MaxNumGaps];
        float spectralData[MaxNumPnt];
 }
NOAA_DataRecordType;

typedef union ObservationCaseType
 {
     NOAA_FieldRecordType Field;
     NOAA_AircraftRecordType Air;
     NOAA_LaboratoryRecordType Lab;
 }
NOAA_ObservationCaseType;

typedef struct InputDataRecordType
 {
        char generalMessage[256];
        NOAA_TimeRecordType timeRecord;
        NOAA_ForeOpticsRecordType    foreOpticsRecord;
        NOAA_SpectrometerRecordType spectrometerRecord;
        NOAA_DetectorRecordType detectorRecord;
        NOAA_DataRecordType dataRecord;
        short observationType;
        NOAA_ObservationCaseType observationCase;
 }
NOAA_InputDataRecordType;

// ===================
// GLOBAL DECLARATIONS
// ===================


// ===================
// STATIC DECLARATIONS
// ===================


// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SetNOAA
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of spectra measured the current day
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_FILE_EMPTY if the file is empty;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetNOAA(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  SZ_LEN fileLength;                                                            // the length in bytes of the file
  uint32_t offset;                                                                 // offset in the file
  short observationType;                                                        // observation type
  RC rc;                                                                        // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SetNOAA",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  rc=ERROR_ID_NO;

  if (specFp==NULL)
   rc=ERROR_SetLast("SetNOAA",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);

  // Get an approximation of the number of spectra

  else if (!(fileLength=STD_FileLength(specFp)))
   rc=ERROR_SetLast("SetNOAA",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else if ((pEngineContext->buffers.recordIndexes=(uint32_t *)MEMORY_AllocBuffer("SetNOAA","recordIndexes",
           (pEngineContext->recordIndexesSize=(int)(fileLength/NOAA_OFFSET+2L)),sizeof(uint32_t),0,MEMORY_TYPE_ULONG))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
       // Get the number of records (the size of records depend on the observation type)

       for (pEngineContext->recordNumber=0,offset=0L;
           (pEngineContext->recordNumber<pEngineContext->recordIndexesSize) && (offset<fileLength);
            pEngineContext->recordNumber++)
        {
            pEngineContext->buffers.recordIndexes[pEngineContext->recordNumber]=offset;

            fseek(specFp,offset+NOAA_OFFSET,0L);
            fread(&observationType,sizeof(short),1,specFp);

            offset+=NOAA_OFFSET+sizeof(short);

            switch(observationType)
             {
           // ------------------------------------------------------------------------
                 case Field :
                  offset+=sizeof(NOAA_FieldRecordType);
                 break;
        // ------------------------------------------------------------------------
                 case Air :
                  offset+=sizeof(NOAA_AircraftRecordType);
                 break;
        // ------------------------------------------------------------------------
                 case Lab :
                  offset+=sizeof(NOAA_LaboratoryRecordType);
                 break;
     // ------------------------------------------------------------------------
             }
        }

       pEngineContext->buffers.recordIndexes[pEngineContext->recordNumber]=offset;

       if (!pEngineContext->recordNumber)
        rc=ERROR_SetLast("SetNOAA",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SetNOAA",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      NOAA_Date
// -----------------------------------------------------------------------------
// PURPOSE       Convert a number of seconds date in the equivalent YYYY/MM/DD hh:mm:ss format
//
// INPUT         sec          the number of seconds since 1904
// INPUT/OUTPUT  pDate,pTime  pointers to resp. date and time in usual format
// -----------------------------------------------------------------------------

void NOAA_Date(double sec, struct date *pDate, struct time *pTime)
 {
  // Declarations

  int year;
  double sumSec,nDaysInYear,tmpSec;

  // Initializations

  memset(pDate,0,sizeof(SHORT_DATE));
  memset(pTime,0,sizeof(struct time));

  // get the number of years since 2000

  for (year=1904,sumSec=(double)0.,nDaysInYear=(double)366.;
       sumSec+nDaysInYear*86400.<sec;)
   {
    year++;
    sumSec+=nDaysInYear*86400.;
    nDaysInYear=((year%4)==0)?(double)366.:(double)365.;
   }

  // Get date from the year and the calendar day

  pDate->da_year= year;
  pDate->da_mon=(char)ZEN_FNCaljmon(year,(int)floor((sec-sumSec)/86400+1.));
  pDate->da_day=(char)ZEN_FNCaljday(year,(int)floor((sec-sumSec)/86400+1.));

  tmpSec=fmod(sec,(double)86400.);

  // Get time

  pTime->ti_hour=(char)(floor(tmpSec/3600.));
  tmpSec=fmod(tmpSec,(double)3600.);
  pTime->ti_min=(char)(floor(tmpSec/60.));
  tmpSec=fmod(tmpSec,(double)60.);
  pTime->ti_sec=(char)(tmpSec);
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliNOAA
// -----------------------------------------------------------------------------
// PURPOSE       Read a record in the NOAA format
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
// RETURN        ERROR_ID_FILE_END if the end of the file is reached;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_FILE_RECORD if the record doesn't satisfy criteria
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC ReliNOAA(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  NOAA_InputDataRecordType *pRecordNoaa;                                        // pointer to the current record
  int obsType;                                                                  // observation type
  INDEX   i;
  RC      rc;                                                                   // return code
  double  tmLocal;                                                              // local time

  // Initializations

  const int n_wavel = NDET[0];
  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  pRecordNoaa=NULL;
  rc=ERROR_ID_NO;

  // Read the record

  if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else if ((pRecordNoaa=(NOAA_InputDataRecordType *)MEMORY_AllocBuffer("ReliNOAA","pRecordNoaa",sizeof(NOAA_InputDataRecordType),1,0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
   {
       fseek(specFp,pBuffers->recordIndexes[recordNo-1],SEEK_SET);
       fread(pRecordNoaa,pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1],1,specFp);

       obsType=pRecordNoaa->observationType;

    // swap bytes

    swap_bytes_float((unsigned char *)&pRecordNoaa->timeRecord.macTime);
          swap_bytes_short((unsigned char *)&pRecordNoaa->timeRecord.zone);

    swap_bytes_short((unsigned char *)&pRecordNoaa->foreOpticsRecord.filterNumber);
          swap_bytes_float((unsigned char *)&pRecordNoaa->foreOpticsRecord.dialSetting);
          swap_bytes_float((unsigned char *)&pRecordNoaa->foreOpticsRecord.lambdaMax);
          swap_bytes_float((unsigned char *)&pRecordNoaa->foreOpticsRecord.lambdaMin);
          swap_bytes_short((unsigned char *)&pRecordNoaa->foreOpticsRecord.polarizerAngle);

          swap_bytes_short((unsigned char *)&pRecordNoaa->spectrometerRecord.spectrometerNumber);
          swap_bytes_short((unsigned char *)&pRecordNoaa->spectrometerRecord.orderNumber);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.dialSetting);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.gratingLinesPerMM);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.gratingInputAngle);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.gratingOutputAngle);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.cameraFocalLength);
          swap_bytes_short((unsigned char *)&pRecordNoaa->spectrometerRecord.spacer);
          swap_bytes_short((unsigned char *)&pRecordNoaa->spectrometerRecord.band);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.wavelengthAtDetCenter);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.dispersion);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.entranceSlitWidth);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.spectrometerTempTop);
          swap_bytes_float((unsigned char *)&pRecordNoaa->spectrometerRecord.spectrometerTempBot);

          swap_bytes_short((unsigned char *)&pRecordNoaa->detectorRecord.detectorNumber);
          swap_bytes_short((unsigned char *)&pRecordNoaa->detectorRecord.numberOfDiodes);
          swap_bytes_float((unsigned char *)&pRecordNoaa->detectorRecord.diodeWidth);
          swap_bytes_float((unsigned char *)&pRecordNoaa->detectorRecord.detectorTemperature);

          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.dcOffset);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.f2);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.f4Sin);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.f4Cos);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.patternRecNum);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.detResponseRecNum);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.sgResponseRecNum);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.numberOfReads);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.strayLightRecNum);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.firstSaturated);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.lastSaturated);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.externalMod);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.internalMod);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.exposureDiodeRecord.xdDarkCountRate);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.exposureDiodeRecord.xdTerminalCount);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.wavelengthRecord.wlDispToCenter);

          for (i=0;i<4;i++)
           swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.wavelengthRecord.wlCoeff[i]);

       swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.wavelengthRecord.wlID0);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.wavelengthRecord.wlID1);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.exposureTime);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.aveDeviation);

          for (i=0;i<73;i++)
           swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.spare[i]);

          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.integrationCycles);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.integrationTime);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.centerOfBrightness);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.shift);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.stretch);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.diodesInExitSlit);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.maxData);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.minData);
          swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.aveData);
          swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.numberOfGapsInteger);

          for (i=0;i<MaxNumGaps;i++)
           swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.startGap[i]);
          for (i=0;i<MaxNumGaps;i++)
           swap_bytes_short((unsigned char *)&pRecordNoaa->dataRecord.endGap[i]);
          for (i=0;i<MaxNumPnt;i++)
           swap_bytes_float((unsigned char *)&pRecordNoaa->dataRecord.spectralData[i]);

          switch(obsType)
           {
         // --------------------------------------------------------------------------
               case Field :

          swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.placeRecord.latitude);
          swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.placeRecord.longitude);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.placeRecord.height);
          swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.placeRecord.zone);

             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.pressure);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.temperature);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.humidity);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.windSpeed);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.windDirection);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.sunAzimuth);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.sunZenith);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.moonAzimuth);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.moonZenith);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.viewAzimuth);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.viewZenith);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.spare1);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.spare2);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.spare3);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Field.spare4);

               break;
         // --------------------------------------------------------------------------
               case Air :

             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.latitude);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.longitude);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.heading);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.pitch);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.roll);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.yaw);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.trueAirSpeed);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.altitude);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.pressure);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.temperature);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.humidity);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.windSpeed);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.windDirection);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.sunAzimuth);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.sunZenith);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.moonAzimuth);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.moonZenith);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.viewAzimuth);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Air.viewZenith);

               break;
         // --------------------------------------------------------------------------
               case Lab :

             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Lab.concentration);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Lab.pathLength);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Lab.pressure);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Lab.temperature);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Lab.flow);
             swap_bytes_float((unsigned char *)&pRecordNoaa->observationCase.Lab.crossectScaleFactor);

               break;
   // --------------------------------------------------------------------------
     }

       switch(obsType)
        {
           // ------------------------------------------------------------------------
                 case Field :

                  pRecord->Zm=(double)pRecordNoaa->observationCase.Field.sunZenith;
                  pRecord->Azimuth=(double)pRecordNoaa->observationCase.Field.sunAzimuth;
                  pRecord->elevationViewAngle=(float)pRecordNoaa->observationCase.Field.viewZenith;
                  pRecord->azimuthViewAngle=(float)pRecordNoaa->observationCase.Field.viewAzimuth;
                  pRecord->latitude=(float)pRecordNoaa->observationCase.Field.placeRecord.latitude;
                  pRecord->longitude=(float)pRecordNoaa->observationCase.Field.placeRecord.longitude;
                  pRecord->maxdoas.measurementType=(pRecord->elevationViewAngle<(double)80.)?PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS:PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;

                 break;
        // ------------------------------------------------------------------------
                 case Air :

                  pRecord->Zm=(double)pRecordNoaa->observationCase.Air.sunZenith;
                  pRecord->Azimuth=(double)pRecordNoaa->observationCase.Air.sunAzimuth;
                  pRecord->elevationViewAngle=(float)90.-pRecordNoaa->observationCase.Air.viewZenith;
                  pRecord->azimuthViewAngle=(float)pRecordNoaa->observationCase.Air.viewAzimuth;
                  pRecord->latitude=(float)pRecordNoaa->observationCase.Air.latitude;
                  pRecord->longitude=(float)pRecordNoaa->observationCase.Air.longitude;
                  pRecord->maxdoas.measurementType=(pRecord->elevationViewAngle<(double)80.)?PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS:PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;

                 break;
        // ------------------------------------------------------------------------
                 case Lab :

                  pRecord->Zm=pRecord->Azimuth=(double)-1.;
                  pRecord->elevationViewAngle=pRecord->azimuthViewAngle=-1.;
                  pRecord->maxdoas.measurementType=PRJCT_INSTR_MAXDOAS_TYPE_NONE;


                 break;
     // ------------------------------------------------------------------------
        }

       NOAA_Date(pRecordNoaa->timeRecord.macTime+pRecordNoaa->timeRecord.zone*3600.,&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime);

    // Get information on the current record

    pRecord->NSomme=(int)pRecordNoaa->dataRecord.integrationCycles;               // number of accumulations
    pRecord->Tint=(double)pRecordNoaa->dataRecord.integrationTime;                // integration time
    pRecord->TDet=(double)pRecordNoaa->detectorRecord.detectorTemperature;        // detector temperature
    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
    pRecord->TotalAcqTime=pRecord->TotalExpTime=(double)pRecord->NSomme*pRecord->Tint;
    pRecord->TimeDec=(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.+pRecord->present_datetime.thetime.ti_sec/3600.;

    // The spectrum

    for (i=0;i<n_wavel;i++)
     pBuffers->spectrum[i]=(double)pRecordNoaa->dataRecord.spectralData[i];

    // Determine the local time

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    // Search for a reference spectrum

    if (dateFlag && (pRecord->localCalDay>localDay))
     rc=ERROR_ID_FILE_END;

    else if ((pRecord->NSomme<=0) ||
             (dateFlag && (pRecord->localCalDay!=localDay)))

     rc=ERROR_ID_FILE_RECORD;

    else if (dateFlag)
     pEngineContext->lastRefRecord=recordNo;
   }

  if (pRecordNoaa!=NULL)
   MEMORY_ReleaseBuffer("ReliNOAA","pRecord",pRecordNoaa);

  // Return

  return rc;
 }

