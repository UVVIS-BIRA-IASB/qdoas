
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  BUFFERS ALLOCATION AND MEMORY CONTROL
//  Name of module    :  MEMORY.C
//  Creation date     :  22 September 1999
//  Modified
//
//    12 october 2004 - the debug mode and errors handling have been improved;
//                    - the stack is not mandatory anymore (could be allocated
//                      in debugging mode only);
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
//  This module has been created for a better memory control in debug mode.
//  Usual buffers allocation/release functions (malloc,free,dvector...) have
//  been replaced by new ones handling a stack of allocated objects and keeping
//  a trace of the calling function.
//
//  REFERENCE
//
//  Numerical Recipes in C
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  MEMORY_AllocBuffer - allocate a new buffer and update the stack of allocated objects;
//  MEMORY_ReleaseBuffer - release a buffer and update the stack of allocated objects;
//  MEMORY_AllocDVector - allocate a vector in double precision with a specified base index;
//  MEMORY_ReleaseDVector - release a vector previously allocated by MEMORY_AllocDVector;
//  MEMORY_AllocDMatrix - allocate a matrix in double precision with specified base indexes for rows and columns;
//  MEMORY_ReleaseDMatrix - release a matrix previously allocated by MEMORY_AllocDMatrix;
//  MEMORY_Alloc - allocate memory for a stack in view of debugging the allocation/release application buffers;
//  MEMORY_End - release the memory allocated for the stack by MEMORY_Alloc;
//  MEMORY_GetInfo - retrieve from the stack the information about an allocated object;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "comdefs.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// ================
// GLOBAL VARIABLES
// ================

int MEMORY_stackSize=MEMORY_STACK_SIZE;                                         // the size of the stack of allocated objects
const char *MEMORY_types[MEMORY_TYPE_MAX]=                                           // available types for allocated objects
 {
 	"unknown",                                                                    // unknown
 	"pointer",                                                                    // pointer
 	"string",                                                                     // character/string
 	"short",                                                                      // short
 	"unsigned short",                                                             // unsigned short
 	"integer",                                                                    // integer
 	"long",                                                                       // long
 	"float",                                                                      // float
 	"double",                                                                     // double
 	"structure",                                                                  // structure
 };

// ================
// STATIC VARIABLES
// ================

static MEMORY *memoryStack=NULL;                                                // stack with current allocated objects
static int     memoryStackObjectsNumber=0;                                      // number of objects currently in the stack
static int32_t    memoryStackBytesNumber=0;                                        // total size used by objects currently in the stack
static int32_t    memoryMaxBytes=0;                                                // maximum number of bytes allocated in one time
static int     memoryMaxObjects=0;                                              // maximum number of objects allocated in one time
static int32_t    memoryMaxObjectsSize=0;                                          // total size used when maximum number of objects is reached

// =========
// FUNCTIONS
// =========


// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_AllocBuffer
// -----------------------------------------------------------------------------
// PURPOSE       allocate a new buffer and update the stack of allocated objects
//
// INPUT         callingFunctionName  : the name of the calling function
//               bufferName           : the name of the buffer in the calling function;
//               itemNumber           : the number of items requested;
//               itemSize             : the size of an item in the new buffer;
//               offset               : the index of the first item (debug only);
//               type                 : type of allocated object (debug only,cfr header)
//
// RETURN        pointer to the allocated buffer;
// -----------------------------------------------------------------------------

void *MEMORY_AllocBuffer(const char *callingFunctionName, const char *bufferName,int itemNumber,int itemSize,int offset,int type)
 {
  // Declarations

  int totalSize;                                                                // requested total size in bytes number for the object
  char *pBuffer;                                                               // pointer to the new allocated buffer
  MEMORY *pMemory;                                                              // pointer to the new allocated object

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_AllocBuffer",DEBUG_FCTTYPE_MEM);
  #endif

  // Initializations

  pBuffer=NULL;
  totalSize=(int)itemNumber*itemSize;

  // Allocate the buffer

  if ((totalSize<=0) || ((pBuffer=(char *)malloc(totalSize))==NULL))
   ERROR_SetLast(callingFunctionName,ERROR_TYPE_FATAL,ERROR_ID_ALLOC,bufferName,itemNumber,itemSize);
  else if (memoryStack!=NULL)
   {
    // Control the number of objects already allocated

    if (memoryStackObjectsNumber==MEMORY_stackSize)
     ERROR_SetLast(callingFunctionName,ERROR_TYPE_DEBUG,ERROR_ID_BUFFER_FULL,"allocated objects");

    // Register the allocated object in the stack

    else
     {
      // Save data on the new allocated object

      pMemory=&memoryStack[memoryStackObjectsNumber];

      memset(pMemory,0,sizeof(MEMORY));

      strncpy(pMemory->callingFunctionName,callingFunctionName,MAX_FCT_LEN);
      strncpy(pMemory->bufferName,bufferName,MAX_VAR_LEN);

      pMemory->pBuffer=(char *)pBuffer;
      pMemory->itemNumber=itemNumber;
      pMemory->itemSize=itemSize;
      pMemory->offset=offset;
      pMemory->type=type;

      memoryStackObjectsNumber++;
      memoryStackBytesNumber+=totalSize;

      // maximum number of objects allocated in one time

      if (memoryStackObjectsNumber>memoryMaxObjects)
       {
        memoryMaxObjects=memoryStackObjectsNumber;
        memoryMaxObjectsSize=memoryStackBytesNumber;
       }

      // maximum number of bytes allocated in one time

      if (memoryStackBytesNumber>memoryMaxBytes)
       memoryMaxBytes=memoryStackBytesNumber;
     }
   }

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Print("Allocate %s (%d bytes) --- Address %08X --- Stack size %d objects (%d bytes)\n",
               bufferName,totalSize,pBuffer,memoryStackObjectsNumber,memoryStackBytesNumber);
  DEBUG_FunctionStop("MEMORY_AllocBuffer",(RC)pBuffer);
  #endif

  // Return

  return (void *)pBuffer;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_ReleaseBuffer
// -----------------------------------------------------------------------------
// PURPOSE       release a buffer and update the stack of allocated objects
//
// INPUT         callingFunctionName : the name of the calling function
//               bufferName          : the name of the buffer in the calling function;
//               pBuffer             : pointer to the buffer to release
//
// NB            names of the calling function and the buffer are used only as
//               information for error message;
// -----------------------------------------------------------------------------

void MEMORY_ReleaseBuffer(const char *callingFunctionName, const char *bufferName,void *pBuffer)
 {
  // Declarations

  INDEX   indexObjects;                                                         // Browse objects in the stack
  MEMORY *pMemory;                                                              // pointer to an object in the stack
  uint32_t   totalSize;                                                            // total size in bytes of the pointed object

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_ReleaseBuffer",DEBUG_FCTTYPE_MEM);
  #endif

  if (memoryStack!=NULL)
   {
    // browse objects in the stack

    for (indexObjects=memoryStackObjectsNumber-1;indexObjects>=0;indexObjects--)
     if (memoryStack[indexObjects].pBuffer==(char *)pBuffer)
      break;

    // remove object from the stack

    if (indexObjects<0){
     ERROR_SetLast(callingFunctionName,ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_RELEASE,bufferName,pBuffer);
    }
    else
     {
      pMemory=&memoryStack[indexObjects];
      totalSize=pMemory->itemSize*pMemory->itemNumber;

      memoryStackBytesNumber-=totalSize;
      memoryStackObjectsNumber--;
      
      if (indexObjects<memoryStackObjectsNumber)
       for (int i=indexObjects;i<memoryStackObjectsNumber;i++)
         memcpy(&memoryStack[i],&memoryStack[i+1],sizeof(MEMORY));
       // memcpy(pMemory,&memoryStack[indexObjects+1],sizeof(MEMORY)*(memoryStackObjectsNumber-indexObjects));
     }
   }

  // Release the allocated object anyway

  if (pBuffer!=NULL){
   free(pBuffer);
  }

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Print("Release %s --- Address %08X --- Stack size %d objects (%d bytes)\n",bufferName,pBuffer,memoryStackObjectsNumber,memoryStackBytesNumber);
  DEBUG_FunctionStop("MEMORY_ReleaseBuffer",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_AllocDVector
// -----------------------------------------------------------------------------
// PURPOSE       allocate a vector in double precision with a specified base index;
//
// INPUT         callingFunctionName  : the name of the calling function;
//               bufferName           : the name of the buffer in the calling function;
//               nl                   : lower index in use;
//               nh                   : higher index in use;
//
// RETURN        pointer to the allocated buffer;
//
// NB            vectors allocated with this function should be released with
//               MEMORY_ReleaseDVector.
// -----------------------------------------------------------------------------

double *MEMORY_AllocDVector(const char *callingFunctionName, const char *bufferName,int nl,int nh)
 {
  // Declaration

  double *v;                                                                    // pointer to the new buffer

  // Register the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_AllocDVector",DEBUG_FCTTYPE_MEM);
  #endif

  // Initialization

  v=NULL;

  if (nh-nl+1<=0)
   ERROR_SetLast(callingFunctionName,ERROR_TYPE_FATAL,ERROR_ID_ALLOCVECTOR,bufferName,nl,nh);

  // Buffer allocation

  else
   v=(double *)MEMORY_AllocBuffer(callingFunctionName,bufferName,(nh-nl+1),sizeof(double),nl,MEMORY_TYPE_DOUBLE);

  // Unregister the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("MEMORY_AllocDVector",((v!=NULL)?(RC)(v-nl):(RC)NULL));
  #endif

  // return

  return((v!=NULL)?(double *)(v-nl):NULL);
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_ReleaseDVector
// -----------------------------------------------------------------------------
// PURPOSE       release a vector previously allocated by MEMORY_AllocDVector
//
// INPUT         callingFunctionName  : the name of the calling function;
//               bufferName           : the name of the buffer in the calling function;
//               v                    : the pointer to the vector;
//               nl                   : lower index in use;
// -----------------------------------------------------------------------------

void MEMORY_ReleaseDVector(const char *callingFunctionName, const char *bufferName,double *v,int nl)
 {
 	// Register the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_ReleaseDVector",DEBUG_FCTTYPE_MEM);
  #endif

  // Release the allocate vector

  if (v!=NULL)
   MEMORY_ReleaseBuffer(callingFunctionName,bufferName,(v+nl));

  // Unregister the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("MEMORY_ReleaseDVector",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_AllocDMatrix
// -----------------------------------------------------------------------------
// PURPOSE       allocate a matrix in double precision with specified base indexes for rows and columns;
//
// INPUT         callingFunctionName  : the name of the calling function;
//               bufferName           : the name of the matrix in the calling function;
//               nrl                  : lower index in use for rows;
//               nrh                  : higher index in use for rows;
//               ncl                  : lower index in use for columns;
//               nch                  : higher index in use for columns;
//
//               here "row" index is the second index, "column" index the first, i.e. the matrix is
//
//               m[i][j], i=ncl..nch, j=nrl..nrh
//
// RETURN        pointer to the allocated buffer;
//
// NB            matrix allocated with this function should be released with
//               MEMORY_ReleaseDMatrix.
// -----------------------------------------------------------------------------

double **MEMORY_AllocDMatrix(const char *callingFunctionName, const char *bufferName,int nrl,int nrh,int ncl,int nch) {
  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_AllocDMatrix",DEBUG_FCTTYPE_MEM);
  #endif

  if ((nch-ncl+1<=0) || (nrh-nrl+1<=0)) {
    ERROR_SetLast(callingFunctionName,ERROR_TYPE_FATAL,ERROR_ID_ALLOCMATRIX,bufferName,nrl,nrh,ncl,nch);
    return NULL;
  }

  // Allocate a buffer for which each item will be a pointer to a column in the matrix
  double **m=MEMORY_AllocBuffer(callingFunctionName,bufferName,nch-ncl+1,sizeof(double *),ncl,MEMORY_TYPE_PTR);
  if (m==NULL)
    return m;
  m -= ncl;

  const int n_cols = nch-ncl+1;
  const int n_rows = nrh-nrl+1;

  // Buffer initialization
  double *buffer = malloc(sizeof(m[0][0])*n_cols*n_rows);
  if (buffer == NULL) {
    MEMORY_ReleaseDMatrix(callingFunctionName,bufferName,m,ncl,nrl);
    return NULL;
  }

  // Set pointers for columns:
  for (int i=0; i<n_cols; ++i) {
    m[ncl+i] = &buffer[i*n_rows]-nrl;
  }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("MEMORY_AllocDMatrix",(RC)m);
  #endif

  return(m);
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_ReleaseDMatrix
// -----------------------------------------------------------------------------
// PURPOSE       release a matrix previously allocated by MEMORY_AllocDMatrix
//
// INPUT         callingFunctionName  : the name of the calling function;
//               bufferName           : the name of the buffer in the calling function;
//               ncl                  : lower index in use for columns;
//               nch                  : higher index in use for columns;
//               nrl                  : lower index in use for rows;
// -----------------------------------------------------------------------------

void MEMORY_ReleaseDMatrix(const char *callingFunctionName,const char *bufferName,double **m,int ncl,int nrl)
 {
  // Register the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_ReleaseDMatrix",DEBUG_FCTTYPE_MEM);
  #endif

  if (m!=NULL) {
    // Release buffer
    free(&m[ncl][nrl]);

    // Release buffer with pointers to columns

    MEMORY_ReleaseBuffer(callingFunctionName,bufferName,m+ncl);
  }

  // Unregister the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("MEMORY_ReleaseDMatrix",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_Alloc
// -----------------------------------------------------------------------------
// PURPOSE       Allocate memory for a stack in view of debugging the
//               allocation/release application buffers
//
// RETURN        ERROR_ID_MEMORY_STACKALLOCATED if the stack is already allocated
//               ERROR_ID_ALLOC if the allocation of the stack failed
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MEMORY_Alloc(void)
 {
 	// Declaration

 	RC rc;                                                                        // return code

 	// Initialization

 	rc=ERROR_ID_NO;

 	if (memoryStack!=NULL)
 	 rc=ERROR_SetLast("MEMORY_Alloc",ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_STACKALLOCATED);
 	else
 	 {

    // Initialize all the static variables handling the stack

    memoryStackObjectsNumber=
    memoryStackBytesNumber=
    memoryMaxBytes=
    memoryMaxObjects=
    memoryMaxObjectsSize=0;

    // Allocate the stack

    if ((memoryStack=(MEMORY *)malloc(MEMORY_stackSize*sizeof(MEMORY)))==NULL)
     rc=ERROR_SetLast("MEMORY_Alloc",ERROR_TYPE_DEBUG,ERROR_ID_ALLOC,"memoryStack",MEMORY_stackSize,sizeof(MEMORY));
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_End
// -----------------------------------------------------------------------------
// PURPOSE       release the memory allocated for the stack by MEMORY_Alloc
//
// RETURN        ERROR_ID_MEMORY_STACKNOTALLOCATED if no stack has been previously allocated
//               ERROR_ID_MEMORY_STACKNOTEMPTY if the stack is not empty before leaving the debugging mode
//               ERROR_ID_NO otherwise
//
// NB            should be called if MEMORY_Alloc has been previously called
// -----------------------------------------------------------------------------

RC MEMORY_End(void)
 {
  // Declarations

  INDEX   indexObjects;                                                         // browse remaining objects in the stack
  MEMORY *pMemory;                                                              // pointer to an allocated object
  RC rc;                                                                        // return code

  // Register the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MEMORY_End",DEBUG_FCTTYPE_MEM);
  #endif

  // Initialization

  rc=ERROR_ID_NO;

  if (memoryStack==NULL)
   rc=ERROR_SetLast("MEMORY_End",ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_STACKNOTALLOCATED);
  else
   {
   	// Browse remaining objects in the stack

   	DEBUG_Print("Number of remaining objects in the stack : %d\n",memoryStackObjectsNumber);

   	if (memoryStackObjectsNumber>0)
   	 {
   	  rc=ERROR_SetLast("MEMORY_End",ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_STACKNOTEMPTY,memoryStackObjectsNumber);
      DEBUG_Print("Allocation/Release error(s) : \n");

      for (indexObjects=0;indexObjects<memoryStackObjectsNumber;indexObjects++)
       {
        pMemory=&memoryStack[indexObjects];

        DEBUG_Print("%-32s %-32s %#8.3fK %08x\n",
                    pMemory->callingFunctionName,
                    pMemory->bufferName,
             (float)pMemory->itemNumber*pMemory->itemSize/1024.,
                    pMemory->pBuffer);
       }
     }

    // Release the allocated stack

    free(memoryStack);

 	  // Reinitialize all the static variables handling the stack

    memoryStackObjectsNumber=
    memoryStackBytesNumber=
    memoryMaxBytes=
    memoryMaxObjects=
    memoryMaxObjectsSize=0;

    memoryStack=NULL;

   }

  // Unregister the function in debugging mode

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("MEMORY_End",0);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MEMORY_GetInfo
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve from the stack the information about an allocated object;
//
// INPUT         pBuffer   : the pointer to the object to search for in the stack
// INPUT/OUTPUT  pVariable : pointer to the structure to receive the information on
//                           the allocated object.
//
// RETURN        ERROR_ID_MEMORY_STACKNOTALLOCATED if no stack has been previously allocated
//               ERROR_ID_MEMORY_OBJECTNOTFOUND if the requested pointer is not found in the stack
//               ERROR_ID_MEMORY_DEFMATRIX if the requested pointer points to a not completed matrix
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MEMORY_GetInfo(DEBUG_VARIABLE *pVariable,char *pBuffer)
 {
 	// Declarations

  MEMORY *pMemory;                                                              // pointer to an object in the stack
 	INDEX i;                                                                      // browse objects in the stack
 	RC rc;                                                                        // return code

 	// Initialization

 	rc=ERROR_ID_NO;

 	// Check that the stack exists

 	if (memoryStack==NULL)
   rc=ERROR_SetLast("MEMORY_GetInfo",ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_STACKNOTALLOCATED);
  else if (pVariable!=NULL)
   {
    memset(pVariable,0,sizeof(DEBUG_VARIABLE));

   	// Browse objects in the stack

   	for (i=0;i<memoryStackObjectsNumber;i++)
   	 if (memoryStack[i].pBuffer-memoryStack[i].offset*memoryStack[i].itemSize==pBuffer)
   	  break;

   	// Object not found

   	if (i>=memoryStackObjectsNumber)
   	 rc=ERROR_SetLast("MEMORY_GetInfo",ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_OBJECTNOTFOUND,pBuffer);
   	else
   	 {
   	  // Retrieve information on the found object

   	  pMemory=&memoryStack[i];

   	  strncpy(pVariable->varName,pMemory->bufferName,MAX_VAR_LEN);
   	  pVariable->varData.ucharVector=(char *)pMemory->pBuffer;

   	  // The variable to debug is a vector

   	  if (pMemory->type!=MEMORY_TYPE_PTR)
   	   {
   	    pVariable->varType=pMemory->type;
   	    pVariable->varNl=pMemory->itemNumber;
   	    pVariable->varNlOff=pMemory->offset;
   	    pVariable->varNc=1;
   	    pVariable->varNcOff=0;
   	    pVariable->varNlMin=0;
   	    pVariable->varNlMax=pVariable->varNl-1;
   	    pVariable->varNcMin=pVariable->varNcMax=0;
   	    pVariable->varMatrixFlag=0;
   	   }

   	  // The variable to debug is a matrix

   	  else if (i+1>=memoryStackObjectsNumber)
   	   rc=ERROR_SetLast("MEMORY_GetInfo",ERROR_TYPE_DEBUG,ERROR_ID_MEMORY_DEFMATRIX,pMemory->bufferName);
   	  else
   	   {
   	    pVariable->varType=(pMemory+1)->type;
   	    pVariable->varNl=(pMemory+1)->itemNumber;
   	    pVariable->varNlOff=(pMemory+1)->offset;
   	    pVariable->varNc=pMemory->itemNumber;
   	    pVariable->varNcOff=pMemory->offset;
   	    pVariable->varNlMin=0;
   	    pVariable->varNlMax=pVariable->varNl-1;
   	    pVariable->varNcMin=0;
   	    pVariable->varNcMax=pVariable->varNc-1;
   	    pVariable->varMatrixFlag=1;
   	   }
   	 }
   }

  // Return

  return rc;
 }
