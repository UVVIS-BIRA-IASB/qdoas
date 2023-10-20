
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  DEBUG ROUTINES
//  Name of module    :  DEBUG.C
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
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  DEBUG_Print - print a formatted string into the debug file;
//  DEBUG_PrintVar - print out the content of variables allocated by MEMORY_AllocBuffer
//  DEBUG_FunctionBegin - register a function and define its debug level;
//  DEBUG_FunctionStop - unregister a function;
//  DEBUG_Start - start the debugging mode;
//  DEBUG_Stop - stop the debugging mode
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  The module DEBUG.C provides some routines to debug the program (the sequence
//  of functions calls and the content of some variables).  To enter the debug
//  mode, the control flag __DEBUG_ in comdefs.h has to be initialized at 1.
//  The part of the code to debug has to be framed within the calls of
//  DEBUG_Start and DEBUG_Stop functions.  The type of functions to debug has to
//  be specified as parameter of the DEBUG_Start function.
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "comdefs.h"
#include "visual_c_compat.h"
#include "winfiles.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ====================
// CONSTANTS DEFINITION
// ====================

#define DEBUG_MAX_LEVELS          25                                            // maximum number of levels of function calls
#define DEBUG_MAX_VARIABLES       25                                            // maximum number of variables to debug (printing)

// =====================
// STRUCTURES DEFINITION
// =====================

// Information about functions

typedef struct _debugFunction
 {
     char fctName[MAX_FCT_LEN+1];                                                 // the name of the function
     MASK  fctType;                                                                // the type of the function
     int   fctLevel;                                                               // the debugging level of the function
 }
DEBUG_FUNCTION;

// ================
// STATIC VARIABLES
// ================

// Information to control the debugging

static DEBUG_FUNCTION debugFct[DEBUG_MAX_LEVELS];                               // the list of functions registered from the DEBUG_Start call
static int debugNFct=0;                                                         // the number of functions in the debugFct stack

static char  debugFileName[DOAS_MAX_PATH_LEN+1];                                    // name of the debug file
static char  debugIndentStr[DEBUG_MAX_LEVELS*4+1];                             // the indentation string (depend on the function level)
static int    debugFctLevelMax=0;                                               // the maximum number of levels of functions to debug
static MASK   debugFctMask=0;                                                   // mask to filter the functions according to their type
static int    debugFlag=0;                                                      // 1 to allow debug, 0 otherwise
static int    debugVar=0;                                                       // 1 to authorize the debugging of variables, 0 otherwise

// ==================
// PRINTING FUNCTIONS
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      DEBUG_Print
// -----------------------------------------------------------------------------
// PURPOSE       print a formatted string into the debug file
//
// INPUT         formatString   : the formatted string;
//               ...            : arguments of the formatted string.
// -----------------------------------------------------------------------------

void DEBUG_Print(const char *formatString,...)
 {
  // Declarations

  va_list argList;                                                              // variable arguments list
  FILE *fp;                                                                     // file pointer

  if (debugFlag && (debugNFct>0) && (debugNFct<=DEBUG_MAX_LEVELS) && (debugFct[debugNFct-1].fctLevel!=ITEM_NONE))
   {
    // Arguments read out

    va_start(argList,formatString);

    // Write out the formatted string into the specified file

    if ((fp=fopen(debugFileName,"a+t"))!=NULL)
     {
      fprintf(fp,"%s",debugIndentStr);                                          // indentation according to the function level
      vfprintf(fp,formatString,argList);                                        // print out the list of arguments
      fclose(fp);                                                               // close the file
     }

    // Close arguments list

    va_end(argList);
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      DEBUG_PrintVar
// -----------------------------------------------------------------------------
// PURPOSE       print out the content of variables allocated by MEMORY_AllocBuffer
//
// INPUT         message : message to print out before listing the variables;
//               ...     : the list of variables to print out.
// -----------------------------------------------------------------------------

void DEBUG_PrintVar(char *message,...)
 {
  // Declarations

  va_list argList;                                                              // variable arguments list
  char *argPtr;                                                                // pointer to the next argument in the previous list
  DEBUG_VARIABLE varList[DEBUG_MAX_VARIABLES];                                  // the list of the indexes of variables to debug
  DEBUG_VARIABLE *pVar;                                                         // pointer to the current variable in the previous list
  int nVar;                                                                     // the number of variables to debug
  int nl;                                                                       // the number of lines
  INDEX indexVar,indexLine,indexCol,indexNewLine;                               // indexes for loops and arrays
  FILE *fp;                                                                     // file pointer
  RC rc;                                                                        // return code

  // Initialization

  nl=ITEM_NONE;

  if (debugFlag && debugVar && (debugNFct>0) && (debugNFct<=DEBUG_MAX_LEVELS) &&
     (debugFct[debugNFct-1].fctLevel!=ITEM_NONE) && ((fp=fopen(debugFileName,"a+t"))!=NULL))
   {
    fprintf(fp,"%s+++ BEGIN DBVAR %s\n",debugIndentStr,message);

    // Arguments read out

    va_start(argList,message);

    for (nVar=0;(nVar<DEBUG_MAX_VARIABLES) && ((argPtr=(char *)va_arg(argList,char *))!=NULL);)
     {
      pVar=&varList[nVar];

      // Get the information on the variable to debug

         if ((rc=MEMORY_GetInfo(pVar,argPtr))!=ERROR_ID_NO)
          fprintf(fp,"%s+++ Can not debug %p (%d)\n",debugIndentStr,argPtr,rc);
         else
          {
           // Define the area to read out

           if (pVar->varMatrixFlag)                                                // matrix
            {
             pVar->varNlMin=(int)va_arg(argList,int);
             pVar->varNlMax=(int)va_arg(argList,int);
             pVar->varNcMin=(int)va_arg(argList,int);
             pVar->varNcMax=(int)va_arg(argList,int);
            }
           else                                                                    // vector
            {
             pVar->varNlMin=(int)va_arg(argList,int);
             pVar->varNlMax=(int)va_arg(argList,int);
            }

           // Check the type of the variable to debug

           if ((pVar->varType==MEMORY_TYPE_UNKNOWN) || (pVar->varType==MEMORY_TYPE_STRUCT))
            fprintf(fp,"%s+++ Can not debug %s (%s type)\n",debugIndentStr,pVar->varName,MEMORY_types[pVar->varType]);

        // Check the area to print

           else if ((pVar->varNlMin<pVar->varNlOff) || (pVar->varNlMin>pVar->varNl+pVar->varNlOff-1) ||
                    (pVar->varNlMax<pVar->varNlOff) || (pVar->varNlMax>pVar->varNl+pVar->varNlOff-1) ||
                    (pVar->varNcMin<pVar->varNcOff) || (pVar->varNcMin>pVar->varNc+pVar->varNcOff-1) ||
                    (pVar->varNcMax<pVar->varNcOff) || (pVar->varNcMax>pVar->varNc+pVar->varNcOff-1))

         fprintf(fp,"%s+++ Can not debug %s because indexes (%d:%d,%d:%d) are out of range (%d:%d,%d:%d)\n",
                     debugIndentStr,pVar->varName,pVar->varNlMin,pVar->varNlMax,pVar->varNcMin,pVar->varNcMax,
                           pVar->varNlOff,pVar->varNl+pVar->varNlOff-1,pVar->varNcOff,pVar->varNc+pVar->varNcOff-1);

           else if ((nl!=ITEM_NONE) && ((pVar->varNlMax-pVar->varNlMin+1)!=nl))
            fprintf(fp,"%s+++ Can not debug %s (dimensions should agree : %d lines <-> %d lines for the first variable to debug )\n",
                    debugIndentStr,pVar->varName,pVar->varNlMax-pVar->varNlMin+1,nl);

        // Add the variable to the list of variables to debug

        else
            {
             nl=pVar->varNlMax-pVar->varNlMin+1;

             if (pVar->varNc>1)
                 fprintf(fp,"%s+++ Debug %s (%s, %dx%d, %d:%d,%d:%d)\n",debugIndentStr,pVar->varName,MEMORY_types[pVar->varType],
                         nl,(pVar->varNcMax-pVar->varNcMin+1),pVar->varNlMin,pVar->varNlMax,pVar->varNcMin,pVar->varNcMax);
          else
                 fprintf(fp,"%s+++ Debug %s (%s, %d, %d:%d)\n",debugIndentStr,pVar->varName,MEMORY_types[pVar->varType],
                         nl,pVar->varNlMin,pVar->varNlMax);

                nVar++;
            }
          }
     }
    va_end(argList);

    // Print out the content of variables to debug

    if (!nVar)
     fprintf(fp,"%s+++ No variables to debug\n",debugIndentStr);
    else if (nl<=0)
     fprintf(fp,"%s+++ Variables to debug are empty\n",debugIndentStr);
    else

     // Browse lines

     for (indexLine=0;indexLine<nl;indexLine++)
      {
       fprintf(fp,"%s    ",debugIndentStr);

       // Browse variables to debug

       for (indexVar=0;indexVar<nVar;indexVar++)
        {
         pVar=&varList[indexVar];

         indexNewLine=(pVar->varMatrixFlag)?indexLine+pVar->varNlMin:indexLine+pVar->varNlMin-pVar->varNlOff;

         // Browse columns

         if (pVar->varMatrixFlag)
          for (indexCol=pVar->varNcMin-pVar->varNcOff;indexCol<pVar->varNcMax-pVar->varNcOff+1;indexCol++)
           switch(pVar->varType)
            {
          // -------------------------------------------------------------------
          // case MEMORY_TYPE_STRING : // string                                 // possibility of bug, to test !!!
          //  fprintf(fp,"%s\t",pVar->varData.ucharArray[indexCol][indexNewLine]);
          //   break;
          // -------------------------------------------------------------------
             case MEMORY_TYPE_SHORT : // short
              fprintf(fp,"%-6d\t",(int)pVar->varData.shortArray[indexCol][indexNewLine]);
             break;
          // -------------------------------------------------------------------
             case MEMORY_TYPE_USHORT : // unsigned short
              fprintf(fp,"%-6d\t",(int)pVar->varData.ushortArray[indexCol][indexNewLine]);
             break;
          // -------------------------------------------------------------------
             case MEMORY_TYPE_INT : // integer
              fprintf(fp,"%-6d\t",(int)pVar->varData.intArray[indexCol][indexNewLine]);
             break;
          // -------------------------------------------------------------------
             case MEMORY_TYPE_LONG : // long
              fprintf(fp,"%-6ld\t",pVar->varData.longArray[indexCol][indexNewLine]);
             break;
          // -------------------------------------------------------------------
             case MEMORY_TYPE_FLOAT : // float
              fprintf(fp,"%-15.6f\t",pVar->varData.floatArray[indexCol][indexNewLine]);
             break;
          // -------------------------------------------------------------------
             case MEMORY_TYPE_DOUBLE : // double
              fprintf(fp,"%-15.6le\t",pVar->varData.doubleArray[indexCol][indexNewLine]);
             break;
          // -------------------------------------------------------------------
           }
         else
          switch(pVar->varType)
           {
         // --------------------------------------------------------------------
         // case MEMORY_TYPE_STRING : // string                                 // possibility of bug, to test !!!
         //  fprintf(fp,"%s\t",pVar->varData.ucharVector[indexNewLine]);
         // break;
         // --------------------------------------------------------------------
            case MEMORY_TYPE_SHORT : // short
             fprintf(fp,"%-6d\t",(int)pVar->varData.shortVector[indexNewLine]);
            break;
         // --------------------------------------------------------------------
            case MEMORY_TYPE_USHORT : // unsigned short
             fprintf(fp,"%-6d\t",(int)pVar->varData.ushortVector[indexNewLine]);
            break;
         // --------------------------------------------------------------------
            case MEMORY_TYPE_INT : // integer
             fprintf(fp,"%-6d\t",(int)pVar->varData.intVector[indexNewLine]);
            break;
         // --------------------------------------------------------------------
            case MEMORY_TYPE_LONG : // long
             fprintf(fp,"%-6ld\t",pVar->varData.longVector[indexNewLine]);
            break;
         // --------------------------------------------------------------------
            case MEMORY_TYPE_FLOAT : // float
             fprintf(fp,"%-15.6f\t",pVar->varData.floatVector[indexNewLine]);
            break;
         // --------------------------------------------------------------------
            case MEMORY_TYPE_DOUBLE : // double
             fprintf(fp,"%-15.6le\t",pVar->varData.doubleVector[indexNewLine]);
            break;
         // --------------------------------------------------------------------
          }
        }

       fprintf(fp,"\n");
      }

    fprintf(fp,"%s+++ END DBVAR %s\n",debugIndentStr,message);

    // Close the debug file

    fclose(fp);
   }
 }

// ======================================
// REGISTER/UNREGISTER FUNCTIONS TO DEBUG
// ======================================

// -----------------------------------------------------------------------------
// FUNCTION      DEBUG_FunctionBegin
// -----------------------------------------------------------------------------
// PURPOSE       Register a function and define its debug level
//
// INPUT         fctName : the name of the function
//               fctType : the type of the function
//
// RETURN        ERROR_ID_BUFFER_FULL if the stack of functions is full
//               ERROR_ID_NO if the function succeeds
// -----------------------------------------------------------------------------

RC DEBUG_FunctionBegin(char *fctName,MASK fctType)
 {
  // Declarations

  FILE *fp;                                                                     // pointer to the debug file
  int fctLevel;                                                                 // current level of function
  int nFctTypes;                                                                // number of function types in the input mask
  RC rc;                                                                        // return code

  // Initialization

  nFctTypes=0;
  rc=ERROR_ID_NO;

  // Register the function in the dedicated stack
  
  if (debugFlag)                                                                // the debug mode is previously open (DEBUG_Start)
   {
    if (debugNFct==DEBUG_MAX_LEVELS)                                            // the maximum number of functions to register is reached (DEBUG_Start)
     rc=ERROR_SetLast("DEBUG_FunctionBegin",ERROR_TYPE_DEBUG,ERROR_ID_BUFFER_FULL,"functions");
    else if (debugNFct<DEBUG_MAX_LEVELS)
     {
      strncpy(debugFct[debugNFct].fctName,fctName,MAX_FCT_LEN);
      debugFct[debugNFct].fctType=fctType;
      debugFct[debugNFct].fctLevel=ITEM_NONE;

      fctLevel=(debugNFct)?debugFct[debugNFct-1].fctLevel:0;

      // The function will be debugged if the following conditions are verified

      if ((!debugNFct ||                                                        // the function is the first one (registered from DEBUG_Start)
          ((fctLevel!=ITEM_NONE) &&                                             // or the parent function is debugged
           (fctLevel<debugFctLevelMax) &&                                       //    and the maximum number of functions to debug is not reached yet
          ((fctType&debugFctMask)!=0))) &&                                      //    and the type of function is accepted for debugging
          ((fp=fopen(debugFileName,"a+t"))!=NULL))                              // and the debug file can be open
       {                                                                        //     (path and file name should be OK - tested in DEBUG_Start)
        debugFct[debugNFct].fctLevel=(fctType!=DEBUG_FCTTYPE_MEM)?fctLevel+1:fctLevel;

        // Register the function in the file

        if (fctType!=DEBUG_FCTTYPE_MEM)                                         // do not register in the file the functions that are memory type only
         {
          // Output the name of the function

          fprintf(fp,"%s-%d- BEGIN %s",debugIndentStr,
                  debugFct[debugNFct].fctLevel,
                  debugFct[debugNFct].fctName);

          // Update the indentation according to the current level

          memset(debugIndentStr,' ',debugFct[debugNFct].fctLevel*4);

          // Output the type of the function

          if (debugNFct)                                                        // for the function registered from DEBUG_Start, do not account for the
           {                                                                    // function type
               fprintf(fp,"(");

            // Output the type of the function

               if ((fctType&DEBUG_FCTTYPE_MEM)!=0)
             fprintf(fp,"%s",(nFctTypes++)?"/MEM":"MEM");
               if ((fctType&DEBUG_FCTTYPE_GUI)!=0)
                fprintf(fp,"%s",(nFctTypes++)?"/GUI":"GUI");
               if ((fctType&DEBUG_FCTTYPE_MATH)!=0)
             fprintf(fp,"%s",(nFctTypes++)?"/MATH":"MATH");
               if ((fctType&DEBUG_FCTTYPE_APPL)!=0)
                fprintf(fp,"%s",(nFctTypes++)?"/APPL":"APPL");
               if ((fctType&DEBUG_FCTTYPE_UTIL)!=0)
             fprintf(fp,"%s",(nFctTypes++)?"/UTIL":"UTIL");
               if ((fctType&DEBUG_FCTTYPE_FILE)!=0)
                fprintf(fp,"%s",(nFctTypes++)?"/FILE":"FILE");
               if ((fctType&DEBUG_FCTTYPE_CONFIG)!=0)
                fprintf(fp,"%s",(nFctTypes++)?"/CONFIG":"CONFIG");

               fprintf(fp,")");
              }

          fprintf(fp,"\n");
         }

        // Close the file

        fclose(fp);
       }
      //else
      // DEBUG_Print("--- %s %d %d %d %d %d\n",fctName,debugFlag,debugNFct,fctLevel,debugFctLevelMax,(fctType&debugFctMask));
     }

    debugNFct++;                                                                // in debug mode, increment the number of functions anyway
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      DEBUG_FunctionStop
// -----------------------------------------------------------------------------
// PURPOSE       Unregister a function
//
// INPUT         fctName : the name of the function
//               rcFct    : the return code of the function
//
// RETURN        ERROR_ID_DEBUG_FCTBLOCK if a problem with a function block is detected
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC DEBUG_FunctionStop(char *fctName,RC rcFct)
 {
  // Declarations

  FILE *fp;                                                                     // pointer to the debug file
  RC rc;                                                                        // return code

  // Initialization

  rc=ERROR_ID_NO;

  if (debugFlag)
   {
    if (debugNFct>DEBUG_MAX_LEVELS)
     debugNFct--;

    else if ((--debugNFct<0) ||
                (strlen(debugFct[debugNFct].fctName)!=strlen(fctName)) ||
                 strcasecmp(debugFct[debugNFct].fctName,fctName))


rc=ERROR_SetLast("DEBUG_FunctionStop",ERROR_TYPE_DEBUG,ERROR_ID_DEBUG_FCTBLOCK,
                (debugNFct>=0)?debugFct[debugNFct].fctName:(char *) "Unknown",fctName);

    else if ((debugFct[debugNFct].fctLevel!=ITEM_NONE) &&
             (debugFct[debugNFct].fctType!=DEBUG_FCTTYPE_MEM) &&
            ((fp=fopen(debugFileName,"a+t"))!=NULL))
     {
      // Update the indentation according to the current level

      if (debugFct[debugNFct].fctLevel>0)
       debugIndentStr[(debugFct[debugNFct].fctLevel-1)*4]=0;

      // Output the name of the function

      fprintf(fp,"%s-%d- END %s (%d)\n",debugIndentStr,debugFct[debugNFct].fctLevel,debugFct[debugNFct].fctName,rcFct);

      // Close the file

      fclose(fp);
     }
   }

  // Return

  return rc;
 }

// =========================
// ENTER/EXIT THE DEBUG MODE
// =========================

// -----------------------------------------------------------------------------
// FUNCTION      DEBUG_Start
// -----------------------------------------------------------------------------
// PURPOSE       Start the debugging mode
//
// INPUT         fileName   : the name of the debug file
//               callingFct : the name of the calling function
//               fctMask    : mask to filter functions to debug
//               nLevels    : number of levels of functions to debug
//               varFlag    : 1 to authorize the debugging of variables
//                            allocated by MEMORY_AllocBuffer
//               resetFlag  : flag to overwrite (1) or append (0) the debug file
//
// RETURN        ERROR_ID_DEBUG_START if the debug mode was already running
//               ERROR_ID_DEBUG_FCTTYPE on an unknown input function type mask
//               ERROR_ID_DEBUG_LEVELS if the input function level is out of range
//               ERROR_ID_FILE if the debug file can not be open
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC DEBUG_Start(char *fileName,char *callingFct,MASK fctMask,int nLevels,int varFlag,int resetFlag)
 {
  // Declarations

  FILE *fp;                                                                     // pointer to the debug file
  int nFctTypes;                                                                // number of function types in the input mask
  RC rc;                                                                        // return code

  // Initializations

  nFctTypes=0;
  rc=ERROR_ID_NO;

  // Cut the name of the file to the maximum size allowed

  if (strlen(fileName)>DOAS_MAX_PATH_LEN)
   *(fileName+DOAS_MAX_PATH_LEN)='\0';

  // Debug mode already running

  if (debugFlag)
   rc=ERROR_SetLast("DEBUG_Start",ERROR_TYPE_DEBUG,ERROR_ID_DEBUG_START);

  // Check the mask

  else if ((fctMask&(DEBUG_FCTTYPE_MEM|                                         // memory allocation
                     DEBUG_FCTTYPE_GUI|                                         // user interface
                     DEBUG_FCTTYPE_MATH|                                        // math functions
                     DEBUG_FCTTYPE_APPL|                                        // application functions
                     DEBUG_FCTTYPE_UTIL|                                        // utility functions
                     DEBUG_FCTTYPE_CONFIG|                                      // configuration
                     DEBUG_FCTTYPE_FILE))==0)                                   // file management

   rc=ERROR_SetLast("DEBUG_Start",ERROR_TYPE_DEBUG,ERROR_ID_DEBUG_FCTTYPE,fctMask);

  // Check the number of levels

  else if ((nLevels<1) || (nLevels>DEBUG_MAX_LEVELS))
   rc=ERROR_SetLast("DEBUG_Start",ERROR_TYPE_DEBUG,ERROR_ID_DEBUG_LEVELS,DEBUG_MAX_LEVELS);

  // Check the file

  else if ((fp=fopen(fileName,(resetFlag)?"w+t":"a+t"))==NULL)
   rc=ERROR_SetLast("DEBUG_Start",ERROR_TYPE_DEBUG,ERROR_ID_FILE_OPEN,fileName);

  else
   {
       // Header

       fprintf(fp,"\nDEBUG fctMask : ");

       if ((fctMask&DEBUG_FCTTYPE_MEM)!=0)
     fprintf(fp,"%s",(nFctTypes++)?"/MEM":"MEM");
       if ((fctMask&DEBUG_FCTTYPE_GUI)!=0)
        fprintf(fp,"%s",(nFctTypes++)?"/GUI":"GUI");
       if ((fctMask&DEBUG_FCTTYPE_MATH)!=0)
     fprintf(fp,"%s",(nFctTypes++)?"/MATH":"MATH");
       if ((fctMask&DEBUG_FCTTYPE_APPL)!=0)
        fprintf(fp,"%s",(nFctTypes++)?"/APPL":"APPL");
       if ((fctMask&DEBUG_FCTTYPE_UTIL)!=0)
     fprintf(fp,"%s",(nFctTypes++)?"/UTIL":"UTIL");
       if ((fctMask&DEBUG_FCTTYPE_FILE)!=0)
        fprintf(fp,"%s",(nFctTypes++)?"/FILE":"FILE");
       if ((fctMask&DEBUG_FCTTYPE_CONFIG)!=0)
        fprintf(fp,"%s",(nFctTypes++)?"/CONFIG":"CONFIG");

       fprintf(fp," fctLevels %d\n",min(nLevels,DEBUG_MAX_LEVELS));
       fprintf(fp,"--------------------------------------------------------------------------------\n");

       // Initialization of static variables

    strcpy(debugFileName,fileName);

    memset(debugFct,0,DEBUG_MAX_LEVELS*sizeof(DEBUG_FUNCTION));
    memset(debugIndentStr,0,DEBUG_MAX_LEVELS*4+1);

    debugFctMask=fctMask;
    debugFctLevelMax=min(nLevels,DEBUG_MAX_LEVELS);
    debugNFct=0;
    debugFlag=1;
    debugVar=varFlag;

    // Close the debug file

    if (fp!=NULL)
     fclose(fp);

    // Register the block to debug

    DEBUG_FunctionBegin(callingFct,DEBUG_FCTTYPE_ALL);
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      DEBUG_Stop
// -----------------------------------------------------------------------------
// PURPOSE       Stop the debugging mode
//
// RETURN        ERROR_ID_DEBUG_STOP if the debug mode was not running
//               ERROR_ID_DEBUG_FCTBLOCK    if a problem with a function block is detected
//               ERROR_ID_DEBUG_ otherwise
// -----------------------------------------------------------------------------

RC DEBUG_Stop(char *callingFct)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;
  
  // The debug mode is not open

  if (!debugFlag)
   rc=ERROR_SetLast("DEBUG_Stop",ERROR_TYPE_DEBUG,ERROR_ID_DEBUG_STOP);

  else if ((strlen(debugFct[0].fctName)==strlen(callingFct)) &&
           !strcasecmp(debugFct[0].fctName,callingFct))
   {
       // Unregister the calling function

       rc=DEBUG_FunctionStop(debugFct[0].fctName,0);

       // Reset debug variables

    memset(debugFileName,0,DOAS_MAX_PATH_LEN+1);

    debugFctMask=0;
    debugFctLevelMax=0;
    debugNFct=0;
    debugFlag=0;
    debugVar=0;
  }

  // Return

  return rc;
 }
