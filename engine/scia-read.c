//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  SCIAMACHY interface
//  Name of module    :  SCIA_Read.C
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
//  FUNCTIONS
//
//  ===================
//  ALLOCATION ROUTINES
//  ===================
//
//  SCIA_ReleaseBuffers - release buffers allocated by SCIAMACHY readout routines;
//  SciaAllocateClusters - allocate buffers for the definition of clusters;
//
//  =========
//  UTILITIES
//  =========
//
//  SCIA_FromMJD2000ToYMD - convert a MJD2000 date in the equivalent YYYY/MM/DD hh:mm:ss format;
//  SciaGetStateIndex - given a record number, return the state index;
//
//  SciaNadirStates - analyze information on NADIR states;
//  SciaNadirGeolocations - retrieve geolocation data;
//  SciaReadSunRefPDS - read the sun reference spectrum from the PDS file;
//  SciaReadNadirMDSInfo - read information on Nadir measurement data set;
//  SciaReadNadirMDS - rread a NADIR measurement data set;
//
//  SCIA_SetPDS  Retrieve information on useful data sets from the PDS file and
//               load the irradiance spectrum measured at the specified channel
//
//  SCIA_ReadPDS - SCIAMACHY calibrated level 1 data read out;
//
//  REFERENCE
//
//  PDS functions use SCIAMACHY Level 1c read out routines written by Stefan Noel
//  from IFE/IUP Uni Bremen (Stefan.Noel@iup.physik.uni.bremen.de), version 0.1,
//  17 Apr 2002 (first beta release).
//
//  These routines are distributed through the WEB.
//
//  ========
//  ANALYSIS
//  ========
//
//  SCIA_LoadAnalysis - load analysis parameters depending on the irradiance spectrum;
//
//  ----------------------------------------------------------------------------

// ===============
// INCLUDE HEADERS
// ===============

#include <dirent.h>
#include <assert.h>

#include "mediate.h"
#include "engine.h"
#include "kurucz.h"
#include "analyse.h"
#include "output.h"
#include "stdfunc.h"
#include "spline.h"
#include "vector.h"
#include "zenithal.h"
#include "winthrd.h"

#include "ref_list.h"

#include "scia-read.h"
#include "engine_context.h"
#include "bin_read.h"
#include "scia_l1c_lib.h"

static const double vza_bins[] = {0., 6., 12., 18., 24.};

#define NUM_VZA_BINS (sizeof(vza_bins)/sizeof(vza_bins[0]))

static inline size_t find_bin(const double vza) {
  assert(vza >= 0.);

  // search backwards, starting from the last bin
  size_t bin = NUM_VZA_BINS-1;
  for (; vza < vza_bins[bin]; --bin);

  return bin;
}

// references for each analysis window and for each VZA bin, depending on scan direction
static struct reference *(*refs_left_scan)[NUM_VZA_BINS-1]; // ESM_POS < 0
static struct reference *(*refs_right_scan)[NUM_VZA_BINS-1]; // ESM_POS > 0
static struct reference **ref_nadir; // center scan

#define NUM_VZA_REFS (2*NUM_VZA_BINS -1)
// array of pointers to all references, for easy iteration over all references
static struct reference (*vza_refs)[NUM_VZA_REFS];

static void free_vza_refs(void);

// =====================
// STRUCTURES DEFINITION
// =====================

struct scia_ref {
  INDEX  indexFile;
  INDEX  indexRecord;
  INDEX  pixelNumber;
  INDEX  pixelType;
  double sza;
  double latitude;
  double longitude;
  double szaDist;
  double latDist;
  double esm_pos;
};

enum scan_direction {
  BACKSCAN,
  FORWARDSCAN
};

// Geolocations and angles for satellite measurements
struct scia_geoloc {
  // Geolocations
  double lonCorners[4],
         latCorners[4],
         lonCenter,
         latCenter;

  // Angles
  float  solZen[3],
         solAzi[3],
         losZen[3],
         losAzi[3];

  double sat_lon, sat_lat;
  float earthRadius,satHeight;
  float esm_pos;
  enum scan_direction scan;
};

// Get the definition of clusters for a given state

typedef struct _sciaClusDef
 {
  int mdsOffset;                                                                // offset of the current measurement data set from the beginning of the file
  int nobs;                                                                     // total number of observations in the measurement data set
  int startPix;                                                                 // starting pixels
  int npixels;                                                                  // number of pixels in the cluster
  int coadd;                                                                    // coadd factor
 }
SCIA_CLUSDEF;

// Definition of NADIR states

typedef struct _sciaNadirState                                                  // information on a NADIR state
 {
  int stateId;                                                                  // id of the state
  int int_time;                                                                 // highest integration time (accounting for clusters)
  int clusId;                                                                   // id of the cluster with the highest integration time
  int nobs;                                                                     // real number of observations accounting for coadd factors
  double dsrTime;                                                               // starting dsr time
 }
SCIA_NADIR_STATE;

// Information on selected clusters

typedef struct _sciaClusters
 {
  int clusId;                                                                   // id of the selected cluster
  float *spe,*err;                                                              // buffers to read out
  SCIA_CLUSDEF *clusDef;                                                        // get the definition of this cluster in the different states
 }
SCIA_CLUSTER;

#define SCIA_CHANNEL_SIZE 1024 // each channel contains 1024 pixels

int SCIA_clusters[PRJCT_INSTR_SCIA_CHANNEL_MAX][2]=
 {
  {  2,  5 },
  {  8, 10 },
  { 13, 18 },
  { 22, 27 }
 };

// ================
// STATIC VARIABLES
// ================

#define MAX_SCIA_FILES 150 // maximum number of files per day

typedef struct _sciaOrbitFiles                                                  // description of an orbit
 {
   char sciaFileName[MAX_STR_LEN+1];                                           // the name of the file with a part of the orbit
   info_l1c        sciaPDSInfo;                                                  // all internal information about the PDS file like data offsets etc.
   float *sciaSunRef,*sciaSunWve;                                                // the sun reference spectrum and calibration
   struct scia_geoloc *sciaGeolocations;                                           // geolocations
   SCIA_NADIR_STATE *sciaNadirStates;                                            // NADIR states
   SCIA_CLUSTER   *sciaNadirClusters;                                            // definition of NADIR clusters to select
   INDEX           sciaNadirClustersIdx[MAX_CLUSTER];                            // get the indexes of clusters in the previous structure
   int             sciaNadirStatesN,                                             // number of NADIR states
                  sciaNadirClustersN;                                           // number of NADIR clusters to select
   int specNumber;
   int rc;
 }
SCIA_ORBIT_FILE;

static SCIA_ORBIT_FILE sciaOrbitFiles[MAX_SCIA_FILES];                          // list of files for an orbit
static int sciaOrbitFilesN=0;                                                   // the total number of files for the current orbit
static INDEX sciaCurrentFileIndex=ITEM_NONE;                                    // index of the current file in the list
static int sciaTotalRecordNumber;                                               // total number of records for an orbit
static int sciaLoadReferenceFlag=0;

// ===================
// ALLOCATION ROUTINES
// ===================

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by SCIAMACHY readout routines
// -----------------------------------------------------------------------------

void SCIA_ReleaseBuffers(char format) {
  // Declarations

  SCIA_CLUSTER *pCluster;
  SCIA_ORBIT_FILE *pOrbitFile;
  INDEX sciaOrbitFileIndex;
  INDEX indexCluster;

  for (sciaOrbitFileIndex=0;sciaOrbitFileIndex<sciaOrbitFilesN;sciaOrbitFileIndex++)
   {
   	pOrbitFile=&sciaOrbitFiles[sciaOrbitFileIndex];

    // Release buffers common to both formats

    for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
     {
      pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];

      if (pCluster->clusDef!=NULL)
       MEMORY_ReleaseBuffer(__func__,"clusDef",pCluster->clusDef);
      if (pCluster->spe!=NULL)
       MEMORY_ReleaseBuffer(__func__,"spe",pCluster->spe);
      if (pCluster->err!=NULL)
       MEMORY_ReleaseBuffer(__func__,"err",pCluster->err);
     }

    if (pOrbitFile->sciaNadirStates!=NULL)
     MEMORY_ReleaseBuffer(__func__,"sciaNadirStates",pOrbitFile->sciaNadirStates);
    if (pOrbitFile->sciaNadirClusters!=NULL)
     MEMORY_ReleaseBuffer(__func__,"sciaNadirClusters",pOrbitFile->sciaNadirClusters);

    if (pOrbitFile->sciaGeolocations!=NULL)
     MEMORY_ReleaseBuffer(__func__,"sciaGeolocations",pOrbitFile->sciaGeolocations);

    if (pOrbitFile->sciaSunRef!=NULL)
     MEMORY_ReleaseBuffer(__func__,"sciaSunRef",pOrbitFile->sciaSunRef);
    if (pOrbitFile->sciaSunWve!=NULL)
     MEMORY_ReleaseBuffer(__func__,"sciaSunWve",pOrbitFile->sciaSunWve);

    // PDS format

    if (format==PRJCT_INSTR_FORMAT_SCIA_PDS)
     closeL1c(&pOrbitFile->sciaPDSInfo);                                        // Close the current PDS file
   }

  for (sciaOrbitFileIndex=0;sciaOrbitFileIndex<MAX_SCIA_FILES;sciaOrbitFileIndex++)
  	memset(&sciaOrbitFiles[sciaOrbitFileIndex],0,sizeof(SCIA_ORBIT_FILE));

  sciaOrbitFilesN=0;
  sciaCurrentFileIndex=ITEM_NONE;

  free_vza_refs();
}

// -----------------------------------------------------------------------------
// FUNCTION      SciaAllocateClusters
// -----------------------------------------------------------------------------
// PURPOSE       Allocate buffers for the definition of clusters
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
// INPUT         clustersList the list of clusters present in the file
//               nClusters    the number of clusters present in the file
//               nStates      the number of states
//               fileIndex    index of the file for the current orbit
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaAllocateClusters(ENGINE_CONTEXT *pEngineContext,int *clustersList,int nClusters,int nStates,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit file
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to instrumental options of projects properties
  INDEX indexCluster;                                                           // browse used clusters
  SCIA_CLUSTER *pCluster;                                                       // pointer to the current cluster
  int userCluster[MAX_CLUSTER];                                                 // the clusters user selection
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter SciaAllocateClusters\n");

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  pInstr=&pEngineContext->project.instrumental;
  memset(userCluster,0,sizeof(int)*MAX_CLUSTER);
  memset(pOrbitFile->sciaNadirClustersIdx,ITEM_NONE,sizeof(int)*MAX_CLUSTER);   // !!!
  rc=ERROR_ID_NO;

  for (indexCluster=SCIA_clusters[pInstr->scia.sciaChannel][0];indexCluster<=SCIA_clusters[pInstr->scia.sciaChannel][1];indexCluster++)
   if (pInstr->scia.sciaCluster[indexCluster-SCIA_clusters[pInstr->scia.sciaChannel][0]])
    userCluster[indexCluster-1]=1;

  // Allocate a buffer for the definition of clusters

  if ((pOrbitFile->sciaNadirClusters=(SCIA_CLUSTER *)MEMORY_AllocBuffer(__func__,"sciaNadirClusters",MAX_CLUSTER,sizeof(SCIA_CLUSTER),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Browse clusters present in the file; keep only those requested by user

    for (indexCluster=0;(indexCluster<nClusters) && !rc;indexCluster++)
     if (userCluster[clustersList[indexCluster]])
      {
       pCluster=&pOrbitFile->sciaNadirClusters[pOrbitFile->sciaNadirClustersN];

       pCluster->clusId=clustersList[indexCluster];
       pCluster->spe=pCluster->err=NULL;

       if ((pCluster->clusDef=(SCIA_CLUSDEF *)MEMORY_AllocBuffer(__func__,"clusDef",nStates,sizeof(SCIA_CLUSDEF),0,MEMORY_TYPE_STRUCT))==NULL)
        rc=ERROR_ID_ALLOC;
       else
        {
         pOrbitFile->sciaNadirClustersIdx[clustersList[indexCluster]]=pOrbitFile->sciaNadirClustersN;
         pOrbitFile->sciaNadirClustersN++;
        }
      }

    // If requested clusters are not in the file, display error

    if (!rc && !pOrbitFile->sciaNadirClustersN)
     rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }

  return rc;
 }

// =========
// UTILITIES
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_FromMJD2000ToYMD
// -----------------------------------------------------------------------------
// PURPOSE       Convert a MJD2000 date in the equivalent YYYY/MM/DD hh:mm:ss format
//
// INPUT         mjd          the date in MJD2000 format
// INPUT/OUTPUT  pDate,pTime  pointers to resp. date and time in usual format
// -----------------------------------------------------------------------------

void SCIA_FromMJD2000ToYMD(double mjd,struct datetime *datetime)
 {
  // Declarations

  int year;
  double sumDays,nDaysInYear,nDaysInMonth;
  int daysInMonth[]={31,28,31,30,31,30,31,31,30,31,30,31};

  struct date *pDate = &datetime->thedate;
  struct time *pTime = &datetime->thetime;

  // Initializations

  memset(pDate,0,sizeof(*pDate));
  memset(pTime,0,sizeof(*pTime));

  // get the number of years since 2000

  for (year=2000,sumDays=(double)0.,nDaysInYear=(double)366.;
       sumDays+nDaysInYear<mjd;)
   {
    year++;
    sumDays+=nDaysInYear;
    nDaysInYear=((year%4)==0)?(double)366.:(double)365.;
   }

  // Get date from the year and the calendar day

  pDate->da_year=(short)year;
  pDate->da_mon=(char)ZEN_FNCaljmon(year,(int)floor(mjd-sumDays+1.));
  pDate->da_day=(char)ZEN_FNCaljday(year,(int)floor(mjd-sumDays+1.));

  // Get time

  mjd-=(double)floor(mjd);
  pTime->ti_hour=(char)(floor(mjd*24.));
  mjd=mjd*24.-pTime->ti_hour;
  pTime->ti_min=(char)(floor(mjd*60.));
  mjd=mjd*60.-pTime->ti_min;
  pTime->ti_sec=(char)(floor(mjd*60.));
  mjd=mjd*60.-pTime->ti_sec;

  // Round the number of milliseconds and correct date and time if the rounded number of milliseconds is exactly 1000

  int SCIA_ms=(int)floor(mjd*1000+0.5);

  if (SCIA_ms>=1000)
   {
   	SCIA_ms%=1000;
   	if (++pTime->ti_sec>=(unsigned char)60)
   	 {
   	 	pTime->ti_sec%=(unsigned char)60;
   	 	if (++pTime->ti_min>=(unsigned char)60)
     	 {
     	 	pTime->ti_min%=(unsigned char)60;
     	 	if (++pTime->ti_hour>=(unsigned char)24)
     	 	 {
     	 	 	pTime->ti_hour%=(unsigned char)24;
     	 	 	nDaysInMonth=daysInMonth[pDate->da_mon-1];
     	 	 	if (((pDate->da_year%4)==0) && (pDate->da_mon==(char)2))
     	 	 	 nDaysInMonth++;
     	 	 	if (++pDate->da_day>(char)nDaysInMonth)
     	 	 	 {
     	 	 	 	pDate->da_day=1;
     	 	 	 	if (++pDate->da_mon>(char)12)
     	 	 	 	 {
     	 	 	 	 	pDate->da_mon=(char)1;
     	 	 	 	 	pDate->da_year++;
     	 	 	 	 }
     	 	 	 }
     	 	 }
   	 	 }
   	 }
   }
  datetime->millis = SCIA_ms;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaGetStateIndex
// -----------------------------------------------------------------------------
// PURPOSE       Given a record number, return the state index.
//
// INPUT         recordNo       the requested record number;
//               fileIndex      index of the file for the current orbit
//
// OUTPUT        pObs           the total number of observations covered by
//                              previous states
//
// RETURN        the index of the state
// -----------------------------------------------------------------------------

INDEX SciaGetStateIndex(int recordNo,int *pObs,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit file
  INDEX indexState;                                                             // browse states
  int sumObs;                                                                   // accumulate the number of observations in the different states

  // Initialization

  pOrbitFile=&sciaOrbitFiles[fileIndex];

  // Search for the state

  for (indexState=sumObs=0;indexState<pOrbitFile->sciaNadirStatesN;sumObs+=pOrbitFile->sciaNadirStates[indexState].nobs,indexState++)
   if (sumObs+pOrbitFile->sciaNadirStates[indexState].nobs>recordNo)
    break;

  // Return

  *pObs=sumObs;

  return indexState;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaNadirStates
// -----------------------------------------------------------------------------
// PURPOSE       Analyze information on NADIR states
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_FILE_EMPTY if no record is found in the file;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaNadirStates(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  ADS_STATES *pAdsState;                                                        // pointer to the current state (from ADS of PDS file)
  SCIA_NADIR_STATE *pStateInfo;                                                 // pointer to the current state in this program
  SCIA_CLUSTER *pCluster;                                                       // pointer to the definition of the current cluster
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the current cluster
  INDEX indexState,indexCluster;                                                // browse resp. NADIR states and clusters
  int   maxPix[MAX_CLUSTER],                                                    // for the set of selected clusters to read, get the maximum number of pixels and
        maxCoadd[MAX_CLUSTER];                                                  // the maximum coadd factor in order to further determine the maximum size of vectors to allocate
  RC rc;                                                                        // return code

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  pOrbitFile->sciaNadirStatesN=pOrbitFile->sciaPDSInfo.n_states[MDS_NADIR];         // number of NADIR states
  pOrbitFile->specNumber=0;

  memset(maxPix,0,sizeof(int)*MAX_CLUSTER);
  memset(maxCoadd,0,sizeof(int)*MAX_CLUSTER);

  rc=ERROR_ID_NO;

  DEBUG_Print("Number of states : %d\n",pOrbitFile->sciaNadirStatesN);
  DEBUG_Print("Number of selected clusters : %d\n",pOrbitFile->sciaNadirClustersN);
  for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
   DEBUG_Print("    Cluster %2d\n",pOrbitFile->sciaNadirClusters[indexCluster].clusId+1);

  // Allocate a buffer for NADIR states

  if ((pOrbitFile->sciaNadirStates=(SCIA_NADIR_STATE *)MEMORY_AllocBuffer(__func__,"sciaNadirStates",pOrbitFile->sciaNadirStatesN,sizeof(SCIA_NADIR_STATE),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Browse states

    for (indexState=0;indexState<pOrbitFile->sciaNadirStatesN;indexState++)
     {
      pAdsState=&pOrbitFile->sciaPDSInfo.ads_states[pOrbitFile->sciaPDSInfo.idx_states[MDS_NADIR][indexState]];
      pStateInfo=&pOrbitFile->sciaNadirStates[indexState];
      pStateInfo->stateId=pAdsState->state_id;
      pStateInfo->nobs=99999;

      // Search for the cluster with the highest integration time

      for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
       {
        pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
        pClusDef=pCluster->clusDef+indexState;

        if (pClusDef->npixels>maxPix[indexCluster])
         maxPix[indexCluster]=pClusDef->npixels;
        if (pClusDef->nobs<pStateInfo->nobs)
         {
         	// given a set of clusters, the total number of records is determined with the lowest spatial resolution

          pStateInfo->nobs=pClusDef->nobs;
          pStateInfo->clusId=pCluster->clusId;
         }
       }

      DEBUG_Print("State index %-2d id %d nobs %-3d duration %d clus id %d clus duration %d\n",indexState+1,pStateInfo->stateId,pStateInfo->nobs,pAdsState->duration,pStateInfo->clusId,pAdsState->Clcon[pStateInfo->clusId].int_time);

      pOrbitFile->specNumber+=pStateInfo->nobs;
      pStateInfo->dsrTime=(double)pAdsState->StartTime.days+1.*(pAdsState->StartTime.secnd+pAdsState->StartTime.musec/1000000.)/86400.;
      pStateInfo->int_time=pAdsState->Clcon[pStateInfo->clusId].int_time;

      for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
       {
        pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
        pClusDef=pCluster->clusDef+indexState;

        pClusDef->startPix=pAdsState->Clcon[pCluster->clusId].pixel_nr;
        pClusDef->coadd=pClusDef->nobs/pStateInfo->nobs;

        DEBUG_Print("State %d Nobs %d Npixels %d Coadd %d\n",indexState+1,pClusDef->nobs,pClusDef->npixels,pClusDef->coadd);

        // Calculate the maximum size of vectors to allocate

        if (pClusDef->coadd>maxCoadd[indexCluster])
         maxCoadd[indexCluster]=pClusDef->coadd;
       }
     }

    for (indexCluster=0;(indexCluster<pOrbitFile->sciaNadirClustersN) && !rc;indexCluster++)
     {
      pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];

      // Buffers allocation for MDS

      if (((pCluster->spe=(float *)MEMORY_AllocBuffer(__func__,"spe",maxPix[indexCluster]*maxCoadd[indexCluster],sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
          ((pCluster->err=(float *)MEMORY_AllocBuffer(__func__,"err",maxPix[indexCluster]*maxCoadd[indexCluster],sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL))

       rc=ERROR_ID_ALLOC;
     }

    if (!pOrbitFile->specNumber)
     rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaNadirGeolocations
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve geolocation data
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaNadirGeolocations(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
{
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  GeoN *sciaGeoloc;                                                             // geolocations in the PDS format
  struct scia_geoloc *pSciaGeoloc;                                                // geolocations in the WinDOAS format
  SCIA_NADIR_STATE *pState;                                                     // pointer to the current state
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the cluster with the highest integration time in the current state
  uint32_t offset;                                                                 // offset of geolocation data from the beginning of file
  RC rc;                                                                        // return code

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  rc=ERROR_ID_NO;

  // Buffers allocation

  sciaGeoloc=(GeoN *)MEMORY_AllocBuffer(__func__,"sciaGeoloc",pOrbitFile->specNumber,sizeof(GeoN),0,MEMORY_TYPE_STRUCT);
  if (sciaGeoloc != NULL)
    pOrbitFile->sciaGeolocations=(struct scia_geoloc *)MEMORY_AllocBuffer(__func__,"pOrbitFile->sciaGeolocations",pOrbitFile->specNumber,sizeof(struct scia_geoloc),0,MEMORY_TYPE_STRUCT);
  if (pOrbitFile->sciaGeolocations == NULL) {
    rc = ERROR_ID_ALLOC;
    goto cleanup;
  }

  for (int indexState=0, indexRecord=0; indexState<pOrbitFile->sciaNadirStatesN; indexState++) {
    // for the current state, get the cluster with the highest integration time

    pState=&pOrbitFile->sciaNadirStates[indexState];
    pClusDef=&pOrbitFile->sciaNadirClusters[pOrbitFile->sciaNadirClustersIdx[pState->clusId]].clusDef[indexState];

    // calculate offset

    offset=pClusDef->mdsOffset+                                                // beginning of the MDS
      4*sizeof(int)+                                                      // StartTime+length
      2*sizeof(char)+                                                     // quality+unit_flag
      1*sizeof(float)+                                                    // orbit_phase
      5*sizeof(short)+                                                    // category+state_id+cluster_id+nobs+npixels
      (sizeof(unsigned short)+                                                     // pixels id
       2*sizeof(float))*pClusDef->npixels+                                   // wavelength+wavelength errors
      2*pClusDef->npixels*pClusDef->nobs*sizeof(float);                     // signal+error

    // Read geolocations

    fseek(pOrbitFile->sciaPDSInfo.FILE_l1c,offset,SEEK_SET);
    GeoN_array_getbin(pOrbitFile->sciaPDSInfo.FILE_l1c,sciaGeoloc,pClusDef->nobs);

    // Browse observations

    for (int j=0; j<pClusDef->nobs; j++,indexRecord++) {
      pSciaGeoloc=&pOrbitFile->sciaGeolocations[indexRecord];

      // corner longitudes/latitudes:
      for (int k=0; k<4; ++k) {
        pSciaGeoloc->lonCorners[k] = (double) sciaGeoloc[j].corner_coord[k].lon*1.e-6;
        pSciaGeoloc->latCorners[k] = (double) sciaGeoloc[j].corner_coord[k].lat*1.e-6;
      }

      // longitude and latitude at pixel centre

      pSciaGeoloc->lonCenter=(double)sciaGeoloc[j].centre_coord.lon*1e-6;
      pSciaGeoloc->latCenter=(double)sciaGeoloc[j].centre_coord.lat*1e-6;

      // angles

      memcpy(pSciaGeoloc->solZen,sciaGeoloc[j].sza_toa,sizeof(float)*3);// TOA solar zenith angles
      memcpy(pSciaGeoloc->solAzi,sciaGeoloc[j].saa_toa,sizeof(float)*3);// TOA solar azimuth angles
      memcpy(pSciaGeoloc->losZen,sciaGeoloc[j].los_zen,sizeof(float)*3);// LOS zenith angles
      memcpy(pSciaGeoloc->losAzi,sciaGeoloc[j].los_azi,sizeof(float)*3);// LOS azimuth angles

      // Miscellaneous

      pSciaGeoloc->earthRadius=sciaGeoloc[j].earth_radius;              // earth radius useful to convert satellite angles to TOA angles
      pSciaGeoloc->satHeight=sciaGeoloc[j].sat_height;                  // satellite height useful to convert satellite angles to TOA angles

      pSciaGeoloc->sat_lon=sciaGeoloc[j].sub_sat.lon*1.0e-6; // GeoN Coord struct stores lon/lat in 10^-6 degrees as integer
      pSciaGeoloc->sat_lat=sciaGeoloc[j].sub_sat.lat*1.0e-6;

      // Label backscan and forward-scan pixels:
      //
      // In nadir scanning mode, the elevation scan mechanism (esm) is
      // responsible for the scanning movement.  Typical esm positions
      // for a scan are
      //
      // (foward-scan:) -30., -26., ..., 24., 27.,
      // (backscan:) 22., 7., -9. -24.
      //
      // Backscan pixels are therefore the only pixels for which the
      // esm position is lower than the esm position of the previous
      // pixel and larger than the esm position of the next
      // pixel. (The turning points are part of the forward scan)
      //
      // In addition, the first pixel of a state (j==0) is always a
      // forward scan pixel, the last pixel in a state is always a
      // backscan pixel.
      //
      // Therefore test:
      //
      //  (NOT first_pixel AND esm < esm_previous) AND
      //  (last_pixel OR esm_next < ESM) => BACKSCAN
      pSciaGeoloc->esm_pos=sciaGeoloc[j].esm_pos;
      if (j>0 && (pSciaGeoloc->esm_pos < sciaGeoloc[j-1].esm_pos) &&
          ( (1+j) == pClusDef->nobs ||
            sciaGeoloc[j+1].esm_pos < pSciaGeoloc->esm_pos)) {
        pSciaGeoloc->scan = BACKSCAN;
      } else {
        pSciaGeoloc->scan = FORWARDSCAN;
      }
    }
  }

  // Release allocated buffers

 cleanup:
  if (sciaGeoloc!=NULL)
    MEMORY_ReleaseBuffer(__func__,"sciaGeoloc",sciaGeoloc);

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaReadSunRefPDS
// -----------------------------------------------------------------------------
// PURPOSE       Read the sun reference spectrum from the PDS file
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaReadSunRefPDS(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex) {
  BUFFERS *pBuffers=&pEngineContext->buffers;
  SCIA_ORBIT_FILE *pOrbitFile=&sciaOrbitFiles[fileIndex];
  FILE *fp=pOrbitFile->sciaPDSInfo.FILE_l1c;
  RC rc=ERROR_ID_NO;

  // Browse reference spectra
  for (unsigned int i=0; i<pOrbitFile->sciaPDSInfo.sun_ref.num_dsr; ++i) {

    const size_t offset=pOrbitFile->sciaPDSInfo.sun_ref.offset+i*sizeof(gads_sun_ref); // offset of reference spectra from the beginning of the PDS file
    char refId[2];                                                                // id of the reference spectra
    fseek(fp,offset,SEEK_SET);
    size_t n_read=fread(&refId,sizeof(refId),1,fp);
    if (n_read != 1)
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

    // find the reference we have set in instrumental properties
    if ( !strncasecmp(refId, pEngineContext->project.instrumental.scia.sciaReference, 2) ) {

      pOrbitFile->sciaSunRef=(float *)MEMORY_AllocBuffer(__func__,"sciaSunRef",SCIA_CHANNEL_SIZE,sizeof(float),0,MEMORY_TYPE_FLOAT);
      pOrbitFile->sciaSunWve=(float *)MEMORY_AllocBuffer(__func__,"sciaSunWve",SCIA_CHANNEL_SIZE,sizeof(float),0,MEMORY_TYPE_FLOAT);
      if (pOrbitFile->sciaSunRef == NULL || pOrbitFile->sciaSunWve == NULL)
        return ERROR_ID_ALLOC;

      // Read the wavelength calibration for this channel
      fseek(fp,offset+sizeof(refId)+(pEngineContext->project.instrumental.scia.sciaChannel)*SCIA_CHANNEL_SIZE*sizeof(float),SEEK_SET);
      n_read=fread(pOrbitFile->sciaSunWve,sizeof(float),SCIA_CHANNEL_SIZE,fp);
      if (n_read != SCIA_CHANNEL_SIZE)
        return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

      // Read the reference spectrum, offset is 8*SCIA_CHANNEL_SIZE
      fseek(fp,offset+sizeof(refId)+(pEngineContext->project.instrumental.scia.sciaChannel+8)*SCIA_CHANNEL_SIZE*sizeof(float),SEEK_SET);
      n_read=fread(pOrbitFile->sciaSunRef,sizeof(float),SCIA_CHANNEL_SIZE,fp);
      if (n_read != SCIA_CHANNEL_SIZE)
        return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

#if defined(__LITTLE_ENDIAN__)
      for (int i=0;i<SCIA_CHANNEL_SIZE;i++) {
        swap_bytes_float((unsigned char *)(&pOrbitFile->sciaSunWve[i]));
        swap_bytes_float((unsigned char *)(&pOrbitFile->sciaSunRef[i]));
      }
#endif
      break;
    }
  }

  if ( pOrbitFile->sciaSunRef == NULL || // we have not found the reference spectrum
      ((fabs(pOrbitFile->sciaSunWve[0]-999.)<1.) && (fabs(pOrbitFile->sciaSunRef[0]-999.)<1.)))
    return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_PDS,"NO_SUN_REF (2)",pEngineContext->fileInfo.fileName);

  for (int i=0;i<SCIA_CHANNEL_SIZE;i++)
    pBuffers->lambda[i]=((float *)pOrbitFile->sciaSunWve)[i];

  for (int i=0;i<SCIA_CHANNEL_SIZE;i++) {
    pBuffers->lambda_irrad[i]=pOrbitFile->sciaSunWve[i];
    pBuffers->irrad[i]=pOrbitFile->sciaSunRef[i];
  }

  if ((pBuffers->dnl.matrix!=NULL) && (pBuffers->dnl.deriv2!=NULL)) {
    for (int i=0;i<SCIA_CHANNEL_SIZE;i++) {
      double dnl;
      if (!(rc=SPLINE_Vector(pBuffers->dnl.matrix[0],pBuffers->dnl.matrix[1],pBuffers->dnl.deriv2[1],pBuffers->dnl.nl,&pBuffers->irrad[i],&dnl,1,SPLINE_CUBIC))) {
        if (dnl==0.)
          rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"non-linearity of the detector");
        else
          pBuffers->irrad[i]/=(double)dnl;
      }
    }
  }
  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaReadNadirMDSInfo
// -----------------------------------------------------------------------------
// PURPOSE       Read information on Nadir measurement data set
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------
RC SciaReadNadirMDSInfo(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  mds_1c_constant mds;                                                          // information on measurement data set
  SCIA_CLUSTER *pCluster;                                                       // pointer to the current cluster
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the current cluster
  INDEX indexNadirMDS,                                                          // browse NADIR measurement data sets
        indexCluster,                                                           // browse clusters to account for
        indexState;                                                             // index of the current state
  uint32_t offset;                                                                 // offset in file
  RC rc;

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  rc=ERROR_ID_NO;

  if (!rc) {
    DEBUG_Print("Number of MDS : %d %d\n",pOrbitFile->sciaPDSInfo.nadir.num_dsr,pOrbitFile->sciaPDSInfo.max_cluster_ids);

    // Browse NADIR MDS

    for (offset=pOrbitFile->sciaPDSInfo.mds_offset[MDS_NADIR],indexNadirMDS=0;
	     indexNadirMDS<(int)pOrbitFile->sciaPDSInfo.nadir.num_dsr;indexNadirMDS++) {
      // Read the MDS offset

      rc=fseek(pOrbitFile->sciaPDSInfo.FILE_l1c,offset,SEEK_SET);

      mds_1c_constant_getbin(pOrbitFile->sciaPDSInfo.FILE_l1c,&mds);

      if ((mds.category==1) && ((indexCluster=pOrbitFile->sciaNadirClustersIdx[mds.cluster_id-1])!=ITEM_NONE))
       {
        // get the index of the state and the cluster

        pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
        indexState=indexNadirMDS/min(pOrbitFile->sciaPDSInfo.max_cluster_ids[MDS_NADIR],56);
        pClusDef=pCluster->clusDef+indexState;

        // Complete the definition of the cluster

        DEBUG_Print("indexState %d indexCluster %d nobs %d npixels %d offset %ld\n",indexState,indexCluster,mds.nobs,mds.npixels,offset);

        pClusDef->nobs=mds.nobs;
        pClusDef->npixels=mds.npixels;
        pClusDef->mdsOffset=offset;

        pEngineContext->recordInfo.scia.qualityFlag=mds.quality;
       }

      offset+=mds.length;
     }
   }
  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaReadNadirMDS
// -----------------------------------------------------------------------------
// PURPOSE       Read a NADIR measurement data set
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// INPUT         indexState   index of state for the NADIR measurement data set to read out
//               indexRecord  index of record in the current state
//               fileIndex    index of the current file
// -----------------------------------------------------------------------------

RC SciaReadNadirMDS(ENGINE_CONTEXT *pEngineContext,INDEX indexState,INDEX indexRecord,INDEX fileIndex)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  uint32_t offset;                                                                 // offset from the beginning of file for data to read
  SCIA_CLUSTER *pCluster;                                                       // pointer to the current cluster
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the cluster in the current state
  INDEX indexCluster;                                                           // browse cluster to read out
  INDEX i,j;                                                                    // browse position in spectra
  FILE *fp;                                                                     // pointer to the PDS file
  double dnl;
  RC rc;

  // Initializations

  const int n_wavel = NDET[0];
  pBuffers=&pEngineContext->buffers;
  pOrbitFile=&sciaOrbitFiles[fileIndex];
  fp=pOrbitFile->sciaPDSInfo.FILE_l1c;

  rc=ERROR_ID_NO;

  for (i=0;i<n_wavel;i++)
   pBuffers->spectrum[i]=pBuffers->sigmaSpec[i]=(double)0.;

  for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
   {
    // Get the definition of the current cluster

    pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
    pClusDef=&pCluster->clusDef[indexState];

    // bypass MDS header (see mds_1c_constant_getbin to understand the calcul of the offset)

    offset=pClusDef->mdsOffset+                                                 // beginning of the MDS
           4*sizeof(int)+                                                       // StartTime+length
           2*sizeof(char)+                                                      // quality+unit_flag
           1*sizeof(float)+                                                     // orbit_phase
           5*sizeof(short)+                                                     // category+state_id+cluster_id+nobs+npixels

    // don't need pixels id and wavelength+wavelength errors

          (sizeof(unsigned short)+                                                      // pixels id
         2*sizeof(float))*pClusDef->npixels;                                    // wavelength+wavelength errors

    // Spectra read out

    fseek(fp,offset+(pClusDef->npixels*pClusDef->coadd*indexRecord)*sizeof(float),SEEK_SET);
    int n_read=fread(pCluster->spe,sizeof(*pCluster->spe),pClusDef->npixels*pClusDef->coadd,fp);
    if (n_read != pClusDef->npixels*pClusDef->coadd) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT,pOrbitFile->sciaFileName);
    }

    offset+=pClusDef->npixels*pClusDef->nobs*sizeof(float);

    // Spectra errors read out

    fseek(fp,offset+(pClusDef->npixels*pClusDef->coadd*indexRecord)*sizeof(float),SEEK_SET);
    n_read=fread(pCluster->err,sizeof(*pCluster->err),pClusDef->npixels*pClusDef->coadd,fp);
    if (n_read != pClusDef->npixels*pClusDef->coadd) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT,pOrbitFile->sciaFileName);
    }

    #if defined(__LITTLE_ENDIAN__)
    for (i=0;i<pClusDef->npixels*pClusDef->coadd;i++)
     {
      swap_bytes_float((unsigned char *)&pCluster->spe[i]);
      swap_bytes_float((unsigned char *)&pCluster->err[i]);
     }
    #endif

    // Coadd observations

    for (j=0;j<pClusDef->coadd;j++)
     for (i=0;i<pClusDef->npixels;i++)
      {
       pBuffers->spectrum[i+pClusDef->startPix]+=(double)pCluster->spe[pClusDef->npixels*j+i];
       pBuffers->sigmaSpec[i+pClusDef->startPix]+=(double)pCluster->err[pClusDef->npixels*j+i]*pCluster->err[pClusDef->npixels*j+i];
      }

    if (pClusDef->coadd!=0)
     for (i=0;i<pClusDef->npixels;i++)
      {
       pBuffers->spectrum[i+pClusDef->startPix]/=(double)pClusDef->coadd;
       if (fabs(pBuffers->sigmaSpec[i+pClusDef->startPix])>(double)1.e-15)
        pBuffers->sigmaSpec[i+pClusDef->startPix]=(double)sqrt(pBuffers->sigmaSpec[i+pClusDef->startPix])/(double)pClusDef->coadd;
      }
   }

  if ((pBuffers->dnl.matrix!=NULL) && (pBuffers->dnl.deriv2!=NULL)) {
    for (i=0;i<n_wavel;i++) {
      if (!(rc=SPLINE_Vector(pBuffers->dnl.matrix[0],pBuffers->dnl.matrix[1],pBuffers->dnl.deriv2[1],pBuffers->dnl.nl,&pBuffers->spectrum[i],&dnl,1,SPLINE_CUBIC))) {
        if (dnl==(double)0.)
          rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"non linearity of the detector");
        else
          pBuffers->spectrum[i]/=(double)dnl;
      }
    }
  }
  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_SetPDS
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on useful data sets from the PDS file and
//               load the irradiance spectrum measured at the specified channel
//
// INPUT/OUTPUT  pEngineContext interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_PDS             if one of the PDS functions failed
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC SCIA_SetPDS(ENGINE_CONTEXT *pEngineContext) {
  // Declarations

  char filePath[MAX_STR_SHORT_LEN+1];
  char fileFilter[MAX_STR_SHORT_LEN+1];
  char fileExt[MAX_STR_SHORT_LEN+1];
  char filePrefix[MAX_STR_SHORT_LEN+1];
  struct dirent *fileInfo;
  DIR *hDir;
  INDEX indexFile;
  char *ptr,*ptrOld;
  char *_nList[10];
  int _n;

  // Initializations

  _n=0;
  pEngineContext->recordNumber=0;
  int previous_file=sciaCurrentFileIndex;
  sciaCurrentFileIndex=ITEM_NONE;
  SCIA_ORBIT_FILE *pOrbitFile=NULL;
  RC rc=ERROR_ID_NO;

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto) {

    // Close the previous file
    if (sciaOrbitFilesN && previous_file!=ITEM_NONE && previous_file<sciaOrbitFilesN
        && sciaOrbitFiles[previous_file].sciaPDSInfo.FILE_l1c!=NULL) {
      fclose(sciaOrbitFiles[previous_file].sciaPDSInfo.FILE_l1c);
      sciaOrbitFiles[previous_file].sciaPDSInfo.FILE_l1c=NULL;
    }

    // Look for this file in the list of loaded files
    for (indexFile=0;indexFile<sciaOrbitFilesN;indexFile++)
      if (!strcasecmp(pEngineContext->fileInfo.fileName,sciaOrbitFiles[indexFile].sciaFileName))
        break;

    if (indexFile<sciaOrbitFilesN)
      sciaCurrentFileIndex=indexFile;
  }

  if (sciaCurrentFileIndex==ITEM_NONE) { // the file was not previously loaded

    // Release old buffers
    SCIA_ReleaseBuffers(pEngineContext->project.instrumental.readOutFormat);

    // In automatic reference mode, get the list of files to load
    if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto) {
      // this file was no was not previously loaded -> we are in a new
      // directory and need to generate a new automatic reference
      sciaLoadReferenceFlag=1;

      // Get file path
      strcpy(filePath,pEngineContext->fileInfo.fileName);

      // make "ptr" point to start of the file name, and make
      // "filePath" contain the path without the file name
      if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
        strcpy(filePath,".");
      else
        *ptr++=0;

      // Build file filter

      strcpy(fileFilter,ptr);

      for (ptrOld=ptr,_n=0;(((_nList[_n]=strchr(ptrOld+1,'_'))!=NULL) && (_n<10));ptrOld=_nList[_n],_n++);

      if (_n<8 || !pEngineContext->analysisRef.refLon) {  // it's not a standard SCIAMACHY file name, so just use this file
        sciaOrbitFilesN=1;
        strcpy(sciaOrbitFiles[0].sciaFileName,pEngineContext->fileInfo.fileName);
      } else {
        for (ptrOld=fileFilter,_n=0;((ptr=strchr(ptrOld,'_'))!=NULL) && ++_n<4;ptrOld=ptr+1);
        if ((ptr!=NULL) && (_n==4))
          *ptr='\0';

       	// Get the file extension of the original file name

        memset(fileExt,0,MAX_STR_SHORT_LEN);

        if ((ptrOld=strrchr(pEngineContext->fileInfo.fileName,'.'))!=NULL)
         strcpy(fileExt,ptrOld+1);
        else
         strcpy(fileExt,"child");

        // Search for files of the same orbit

        for (hDir=opendir(filePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);) {
          sprintf(sciaOrbitFiles[sciaOrbitFilesN].sciaFileName,"%s/%s",filePath,fileInfo->d_name);

          if (!STD_IsDir(sciaOrbitFiles[sciaOrbitFilesN].sciaFileName)) {
            strcpy(filePrefix,fileInfo->d_name);
            for (ptrOld=filePrefix,_n=0;((ptr=strchr(ptrOld,'_'))!=NULL) && ++_n<4;ptrOld=ptr+1);
            if ((ptr!=NULL) && (_n==4))
              *ptr='\0';

            if ( ( (ptr=strrchr(fileInfo->d_name,'.') ) !=NULL)
                 && !strcasecmp(ptr+1,fileExt) && !strcasecmp(filePrefix,fileFilter) )
              sciaOrbitFilesN++;
          }
        }

        if ( hDir != NULL ) closedir(hDir);

        if (!sciaOrbitFilesN) {
          sciaOrbitFilesN=1;
          strcpy(sciaOrbitFiles[0].sciaFileName,pEngineContext->fileInfo.fileName);
        }
      }
    } else {
      sciaOrbitFilesN=1;
      strcpy(sciaOrbitFiles[0].sciaFileName,pEngineContext->fileInfo.fileName);
    }

    // Load files

    for (sciaTotalRecordNumber=indexFile=0;indexFile<sciaOrbitFilesN;indexFile++) {
      pOrbitFile=&sciaOrbitFiles[indexFile];

      pOrbitFile->sciaPDSInfo.FILE_l1c=NULL;
      pOrbitFile->specNumber=0;

      // Open file
      if (openL1c(pOrbitFile->sciaFileName,&pOrbitFile->sciaPDSInfo) != ERROR_ID_NO) {
        rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_PDS,"openL1c",pOrbitFile->sciaFileName);
      } else if (!(rc=SciaReadSunRefPDS(pEngineContext,indexFile))) { // Read the irradiance data set to get the wavelength calibration

        // Read information on radiances spectra

        if (!pOrbitFile->sciaPDSInfo.n_states[MDS_NADIR])
          rc=ERROR_ID_FILE_EMPTY;
        else if (!(rc=SciaAllocateClusters(pEngineContext,pOrbitFile->sciaPDSInfo.cluster_ids[MDS_NADIR],     // Allocate buffers for clusters
                                           pOrbitFile->sciaPDSInfo.max_cluster_ids[MDS_NADIR],pOrbitFile->sciaPDSInfo.n_states[MDS_NADIR],indexFile)) &&
                 !(rc=SciaReadNadirMDSInfo(pEngineContext,indexFile)) &&                           // get offset of NADIR measurement data set
                 !(rc=SciaNadirStates(pEngineContext,indexFile)) &&                                // determine the number of records from the cluster with highest integration time in the different states
                 !(rc=SciaNadirGeolocations(pEngineContext,indexFile)))                            // Read geolocations

          pEngineContext->recordInfo.satellite.orbit_number=atoi(pOrbitFile->sciaPDSInfo.mph.abs_orbit);
      }

      if (pOrbitFile->sciaPDSInfo.FILE_l1c!=NULL) {
        fclose(pOrbitFile->sciaPDSInfo.FILE_l1c);                               // Close the current PDS file
        pOrbitFile->sciaPDSInfo.FILE_l1c=NULL;
      }

      if (!strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->sciaFileName) )
        sciaCurrentFileIndex=indexFile;

      sciaTotalRecordNumber+=pOrbitFile->specNumber;

      pOrbitFile->rc=rc;
      rc=ERROR_ID_NO;
    }
  }

  if ((sciaCurrentFileIndex==ITEM_NONE) || !(pEngineContext->recordNumber=(pOrbitFile=&sciaOrbitFiles[sciaCurrentFileIndex])->specNumber))
    rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->sciaFileName);
  else {
    pEngineContext->recordInfo.satellite.orbit_number=atoi(pOrbitFile->sciaPDSInfo.mph.abs_orbit);
    if (!(rc=pOrbitFile->rc) && (pOrbitFile->sciaPDSInfo.FILE_l1c==NULL))
      pOrbitFile->sciaPDSInfo.FILE_l1c=fopen(pOrbitFile->sciaFileName,"rb");
  }

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_ReadPDS
// -----------------------------------------------------------------------------
// PURPOSE       SCIAMACHY calibrated level 1 data read out
//
// INPUT         recordNo     index of the record to read
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------
RC SCIA_ReadPDS(ENGINE_CONTEXT *pEngineContext,int recordNo)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  struct scia_geoloc *pSciaData;                                                  // data (geolocation+angles) of the current record
  int stateObs;                                                                 // total number of observations covered by previous states
  INDEX indexState;                                                             // index of the current state
//   double tmp;
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pOrbitFile=&sciaOrbitFiles[sciaCurrentFileIndex];
  rc=ERROR_ID_NO;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
   rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    if ((indexState=SciaGetStateIndex(recordNo-1,&stateObs,sciaCurrentFileIndex))==ITEM_NONE)
     rc=ERROR_ID_FILE_RECORD;
    else
     {
      SciaReadNadirMDS(pEngineContext,indexState,recordNo-stateObs-1,sciaCurrentFileIndex);

      // Geolocation and angles data

      pSciaData=&pOrbitFile->sciaGeolocations[recordNo-1];

      memcpy(pRecord->satellite.cornerlats,pSciaData->latCorners,sizeof(double)*4);
      memcpy(pRecord->satellite.cornerlons,pSciaData->lonCorners,sizeof(double)*4);
      memcpy(pRecord->scia.solZen,pSciaData->solZen,sizeof(float)*3);
      memcpy(pRecord->scia.solAzi,pSciaData->solAzi,sizeof(float)*3);
      memcpy(pRecord->scia.losZen,pSciaData->losZen,sizeof(float)*3);
      memcpy(pRecord->scia.losAzi,pSciaData->losAzi,sizeof(float)*3);

      // Misecellaneous data (for TEMIS)

      pRecord->satellite.earth_radius=pSciaData->earthRadius;
      pRecord->satellite.altitude=pSciaData->satHeight;
      pRecord->scia.stateIndex=indexState;
      pRecord->scia.stateId=pOrbitFile->sciaNadirStates[indexState].stateId;

      pRecord->latitude=pSciaData->latCenter;
      pRecord->longitude=pSciaData->lonCenter;

      pRecord->Zm=pSciaData->solZen[1];
      pRecord->Azimuth=pSciaData->solAzi[1];
      pRecord->zenithViewAngle=pSciaData->losZen[1];
      pRecord->azimuthViewAngle=pSciaData->losAzi[1];
      pRecord->satellite.vza=pSciaData->esm_pos;
      pRecord->satellite.longitude=pSciaData->sat_lon;
      pRecord->satellite.latitude=pSciaData->sat_lat;

      pRecord->Tint=pOrbitFile->sciaNadirStates[indexState].int_time/16.;

      pRecord->TimeDec=(double)pOrbitFile->sciaNadirStates[indexState].dsrTime+(recordNo-stateObs-1)*pOrbitFile->sciaNadirStates[indexState].int_time/16./86400.;
      pRecord->TimeDec=(double)(pRecord->TimeDec-floor(pRecord->TimeDec))*24.;

      // Get date and time

      SCIA_FromMJD2000ToYMD((double)pOrbitFile->sciaNadirStates[indexState].dsrTime+
                            (double)(recordNo-stateObs-1)*pOrbitFile->sciaNadirStates[indexState].int_time/16./86400.,&pRecord->present_datetime);

      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);  // !!!
     }
   }
  return rc;
 }

// =============================
// AUTOMATIC REFERENCE SELECTION
// =============================

RC SCIA_get_vza_ref(double esm_pos, int index_feno, FENO *feno) {
  const size_t bin = find_bin(fabs(esm_pos));
  const struct reference *ref;
  if (bin == 0) {
    ref = ref_nadir[index_feno];
  } else {
    ref = (esm_pos < 0.)
      ? refs_left_scan[index_feno][bin-1]
      : refs_right_scan[index_feno][bin-1];
  }

  if (!ref->n_spectra) {
    const double vza_min = vza_bins[bin];
    const double vza_max = (1+bin) < NUM_VZA_BINS
      ? vza_bins[1+bin]
      : 90.0;
    // TODO: specify if it's a left or right-scan reference?
    return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_VZA_REF, vza_min, vza_max);
  }

  assert((size_t) feno->NDET == ref->n_wavel);

  feno->Shift=ref->shift;
  feno->Stretch=ref->stretch;
  feno->Stretch2=ref->stretch2;
  feno->refNormFact=ref->norm;
  feno->Decomp = 1;
  for (int i=0; i<feno->NDET; ++i) {
    feno->Sref[i] = ref->spectrum[i];
  }

  return ERROR_ID_NO;
}

static void free_vza_refs(void) {
  if(vza_refs != NULL) {
    for (int i=0; i<NFeno; ++i) {
      for (size_t j=0; j<NUM_VZA_REFS; ++j) {
        free(vza_refs[i][j].spectrum);
      }
    }
  }
  free(vza_refs);
  free(refs_left_scan);
  free(refs_right_scan);
  free(ref_nadir);
  vza_refs=NULL;
  refs_left_scan=refs_right_scan=NULL;
  ref_nadir=NULL;
}

static void initialize_vza_refs(void) {
  free_vza_refs();// will free previously allocated structures, if any.

  vza_refs = malloc(NFeno * sizeof(*vza_refs));
  refs_left_scan = malloc(NFeno * sizeof(*refs_left_scan));
  refs_right_scan = malloc(NFeno * sizeof(*refs_right_scan));
  ref_nadir = malloc(NFeno * sizeof(*ref_nadir));

  // Build array of pointers to the collection of VZA references:
  for (int i=0; i<NFeno; ++i) {
    const int n_wavel = TabFeno[0][i].NDET;
    for(size_t j=0; j<NUM_VZA_REFS; ++j) {
      struct reference *ref = &vza_refs[i][j];
      ref->spectrum = malloc(n_wavel*sizeof(*ref->spectrum));
      for (int i=0; i<n_wavel; ++i)
        ref->spectrum[i]=0.;
      ref->n_wavel = n_wavel;
      ref->n_spectra = 0;
      ref->norm = ref->shift = ref->stretch = ref->stretch2 = 0.0;
    }

    ref_nadir[i] = &vza_refs[i][0];
    for (size_t j=1; j<NUM_VZA_BINS; ++j) {
      refs_left_scan[i][j-1] = &vza_refs[i][j];
      refs_right_scan[i][j-1]  = &vza_refs[i][NUM_VZA_BINS-1+j];
    }
  }
}

static int read_record(ENGINE_CONTEXT *pEngineContext, int index_file, int record_index,
                       double *lambda, double *spec, double *spec_error, const int n_wavel) {
  RC rc=ERROR_ID_NO;

  int indexObs;
  int indexState=SciaGetStateIndex(record_index,&indexObs,index_file);
  if (indexState==ITEM_NONE) {
    // TODO: error
    return rc;
  }

  rc=SciaReadNadirMDS(pEngineContext,indexState,record_index-indexObs,index_file);
  if (rc)
    return rc;

  for (int i=0; i< n_wavel; ++i) {
    lambda[i] = pEngineContext->buffers.lambda[i];
    spec[i] = pEngineContext->buffers.spectrum[i];
  }

  return rc;
}

static bool use_as_reference(const struct scia_geoloc *record, const FENO *feno) {
  const double latDelta = fabs(feno->refLatMin - feno->refLatMax);
  const double lonDelta = fabs(feno->refLonMin - feno->refLonMax);

   // SCIA longitudes are in range -180-180 -> convert to range 0-360.
  const double lon = (record->lonCenter > 0) ? record->lonCenter : 360.0+record->lonCenter;

  const bool match_lat = (latDelta <= EPSILON)
    || (record->latCenter >= feno->refLatMin && record->latCenter <= feno->refLatMax);
  const bool match_lon = (lonDelta <= EPSILON)
    || ( (feno->refLonMin < feno->refLonMax
        && lon >=feno->refLonMin && lon <= feno->refLonMax)
        ||
       (feno->refLonMin >= feno->refLonMax
        && (lon >= feno->refLonMin || lon <= feno->refLonMax) ) ); // if refLonMin > refLonMax, we have either lonMin < lon < 360, or 0 < lon < refLonMax
  const bool match_sza = (feno->refSZADelta <= EPSILON)
    || ( fabs(record->solZen[1] - feno->refSZA) <= feno->refSZADelta);

  return (record->scan == FORWARDSCAN) && match_lat && match_lon && match_sza;
}

// create a list of all spectra that match reference selection criteria for one or more analysis windows.
static int find_ref_spectra(struct ref_list *selected_spectra[NFeno][NUM_VZA_REFS], struct ref_list **list_handle) {
  int rc = 0;
  // zero-initialize
  for (int i=0; i<NFeno; ++i) {
    for (size_t j=0; j<NUM_VZA_REFS; ++j) {
      selected_spectra[i][j] = NULL;
    }
  }
  *list_handle = NULL;

  // iterate over all orbit files in same directory
  for (int i=0; i<sciaOrbitFilesN; i++) {
    SCIA_ORBIT_FILE *orbit=&sciaOrbitFiles[i];
    if (orbit->rc)
      continue; // skip this file

    bool close_current_file=false;
    if (orbit->sciaPDSInfo.FILE_l1c==NULL) {
      orbit->sciaPDSInfo.FILE_l1c=fopen(orbit->sciaFileName,"rb");
      close_current_file=true; // if we opened it here, remember to close it again later
    }

    for (int j=0; j<orbit->specNumber; ++j) {
      // each spectrum can be used for multiple analysis windows, so
      // we use one copy, and share the pointer between the different
      // analysis windows. We initialize as NULL, it becomes not-null
      // as soon as it is used in one or more analysis windows:
      struct ref_spectrum *ref = NULL;
      const struct scia_geoloc *record = &orbit->sciaGeolocations[j];

      // check if this spectrum satisfies constraints for one of the analysis windows:
      for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
        const FENO *pTabFeno = &TabFeno[0][analysis_window];
        const int n_wavel = pTabFeno->NDET;
        if (!pTabFeno->hidden
            && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
            && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
            && use_as_reference(record, pTabFeno) ) {

          if (ref == NULL) {
            // ref hasn't been initialized yet for another analysis window, so do that now:
            ref = malloc(sizeof(*ref));
            ref->lambda = malloc(n_wavel*sizeof(*ref->lambda));
            ref->spectrum = malloc(n_wavel*sizeof(*ref->spectrum));

            // store the new reference at the front of the linked list:
            struct ref_list *new = malloc(sizeof(*new));
            new->ref = ref;
            new->next = *list_handle;
            *list_handle = new;

            rc = read_record(&ENGINE_contextRef, i, j, ref->lambda, ref->spectrum, NULL, n_wavel);
            if (rc != ERROR_ID_NO) {
              if (close_current_file) {
                fclose(orbit->sciaPDSInfo.FILE_l1c);
                orbit->sciaPDSInfo.FILE_l1c=NULL;
              }
              return rc;
            }
          }

          // store ref at the front of the list of selected references for this analysis window and vza bin.
          struct ref_list *list_item = malloc(sizeof(*list_item));
          list_item->ref = ref;
          const size_t bin = find_bin(fabs(record->esm_pos));

          // vza bins are identified by their offset:
          // 0 = nadir,
          // (1, NUM_VZA_BINS( = left scan
          // (NUM_VZA_BINS,2*NUM_VZA_BINS-1( = right scan
          // :add the reference
          const size_t vza_offset = (bin == 0 || record->esm_pos < 0.) ? bin : (NUM_VZA_BINS-1 + bin);
          list_item->next = selected_spectra[analysis_window][vza_offset];
          selected_spectra[analysis_window][vza_offset] = list_item;
        }
      }
    }
    if (close_current_file) {
      fclose(orbit->sciaPDSInfo.FILE_l1c);
      orbit->sciaPDSInfo.FILE_l1c=NULL;
    }
  }

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      SciaNewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC SciaNewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {
  // make a copy of the EngineContext structure to read reference data
  RC rc=EngineCopyContext(&ENGINE_contextRef,pEngineContext);

  if (ENGINE_contextRef.recordNumber==0)
    return ERROR_ID_ALLOC;

  // Allocate references
  initialize_vza_refs();

  // 1. look in all candidate orbit files (i.e. orbit files in same
  //    dir)

  // for each analysis window: selected spectra per VZA bin
  // the same spectrum can be used in multiple analysis windows.
  struct ref_list *selected_spectra[NFeno][NUM_VZA_REFS];

  // list_handle: list of references to same set of spectra, used for
  // memory management.  In this list, each spectrum appears only once.
  struct ref_list *list_handle;

  rc = find_ref_spectra(selected_spectra, &list_handle);
  if (rc != ERROR_ID_NO)
    goto cleanup;

  // 2. average spectra per analysis window and per VZA bin
  for (int i=0;(i<NFeno) && (rc<THREAD_EVENT_STOP);i++) {
    FENO *pTabFeno=&TabFeno[0][i];

    if ((pTabFeno->hidden!=1) &&
        (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
        (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)) {

      for (size_t j=0; j<NUM_VZA_REFS; ++j) {
        if (selected_spectra[i][j] == NULL) {
#define MESSAGE " for analysis window %s and VZA bin %zu"
          const int length = strlen(MESSAGE) + strlen(pTabFeno->windowName) + strlen(TOSTRING(MAX_FENO));
          char tmp[length];
          sprintf(tmp, MESSAGE, pTabFeno->windowName, j); // TODO convert ref number back to bin for error message
#undef MESSAGE
          ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REFERENCE_SELECTION, tmp);
          continue;
        }
        struct reference *ref = &vza_refs[i][j];
        rc = average_ref_spectra(selected_spectra[i][j], pTabFeno->LambdaRef, pTabFeno->NDET, ref);
        if (rc != ERROR_ID_NO)
          goto cleanup;

        // align ref w.r.t irradiance reference:
        double sigma_shift, sigma_stretch, sigma_stretch2; // not used here...
        rc = ANALYSE_fit_shift_stretch(1, 0, pTabFeno->SrefEtalon, ref->spectrum,
                                       &ref->shift, &ref->stretch, &ref->stretch2, \
                                       &sigma_shift, &sigma_stretch, &sigma_stretch2);
      }
    }
  }

 cleanup:
  // 3. free lists created in step 1

  // for 'selected_spectra', we only free the 'gome_ref_list'
  // structures, the other components are pointers to spetra owned by
  // the list_handle structure:
  for (int i=0; i<NFeno; ++i) {
    for (size_t j=0; j<NUM_VZA_REFS; ++j) {
      free_ref_list(selected_spectra[i][j], FREE_LIST_ONLY);
    }
  }

  // for 'list_handle', we also free the gome_ref_spectrum* pointers,
  // and the double* pointers 'lambda' & 'spectrum':
  free_ref_list(list_handle, FREE_DATA);

  return rc;
 }

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC SCIA_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow,i;                                  // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  int DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations
  const int n_wavel = NDET[0];

  pOrbitFile=&sciaOrbitFiles[sciaCurrentFileIndex];
  saveFlag=pEngineContext->project.spectra.displayDataFlag;

  if (!(rc=pOrbitFile->rc) && (sciaLoadReferenceFlag || !pEngineContext->analysisRef.refAuto))
   {
    rc=ERROR_ID_NO;
    useKurucz=useUsamp=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
      {
       pTabFeno=&TabFeno[0][indexFeno];
       pTabFeno->NDET=n_wavel;

       // Load calibration and reference spectra

       if (!pTabFeno->gomeRefFlag)
        {
         for (i=0;i<pTabFeno->NDET;i++)
          {
           pTabFeno->LambdaRef[i]=(double)((pOrbitFile->sciaSunWve)[i]);
           pTabFeno->Sref[i]=(double)((pOrbitFile->sciaSunRef)[i]);
          }

         if (!TabFeno[0][indexFeno].hidden)
          {

            if (!(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"SCIA_LoadAnalysis (Reference) ")))
            {
             memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
             pTabFeno->useEtalon=pTabFeno->displayRef=1;

             // Browse symbols

             for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
              {
               pTabCross=&pTabFeno->TabCross[indexTabCross];
               pWrkSymbol=&WorkSpace[pTabCross->Comp];

               // Cross sections and predefined vectors

               if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                    ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                    ((indexTabCross==pTabFeno->indexCommonResidual) ||
                   (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
                    ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET))!=ERROR_ID_NO))

                goto EndSCIA_LoadAnalysis;
              }

             // Gaps : rebuild subwindows on new wavelength scale

             doas_spectrum *new_range = spectrum_new();
             for (indexWindow = 0, DimL=0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++)
              {
               int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
               int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

               spectrum_append(new_range, pixel_start, pixel_end);

               DimL += pixel_end - pixel_start +1;
              }

             // Buffers allocation
             FIT_PROPERTIES_free(__func__,&pTabFeno->fit_properties);
             pTabFeno->fit_properties.DimL=DimL;
             FIT_PROPERTIES_alloc(__func__,&pTabFeno->fit_properties);
             // new spectral windows
             pTabFeno->fit_properties.specrange = new_range;

             pTabFeno->Decomp=1;

             if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
                 ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) &&
                 ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))

              goto EndSCIA_LoadAnalysis;
            }
          }

         memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);
         memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);

         useUsamp+=pTabFeno->useUsamp;
         useKurucz+=pTabFeno->useKurucz;

        }
      }

     // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
       goto EndSCIA_LoadAnalysis;
     }

    // Build undersampling cross sections

    if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
     {
       // ANALYSE_UsampLocalFree();

      if (((rc=ANALYSE_UsampLocalAlloc(0))!=ERROR_ID_NO) ||
          ((rc=ANALYSE_UsampBuild(0,0,0))!=ERROR_ID_NO) ||
          ((rc=ANALYSE_UsampBuild(1,ITEM_NONE,0))!=ERROR_ID_NO))

       goto EndSCIA_LoadAnalysis;
     }

    // Automatic reference selection

    if ((THRD_id!=THREAD_TYPE_KURUCZ) && sciaLoadReferenceFlag && !(rc=SciaNewRef(pEngineContext,responseHandle)))
      rc=ANALYSE_AlignReference(pEngineContext,2,responseHandle,0);

    if (!rc)
     sciaLoadReferenceFlag=0;
   }

  EndSCIA_LoadAnalysis :
  return rc;
 }

void SCIA_get_orbit_date(int *year, int *month, int *day) {
  struct datetime dt = {0};

  SCIA_FromMJD2000ToYMD( sciaOrbitFiles[sciaCurrentFileIndex].sciaNadirStates[0].dsrTime, &dt);

  *year = dt.thedate.da_year;
  *month = dt.thedate.da_mon;
  *day = dt.thedate.da_day;
}
