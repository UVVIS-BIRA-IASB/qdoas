
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  FILES MODULE
//  Name of module    :  WINFILES.C
//  Creation date     :  1997
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
//  ================
//  PATHS PROCESSING
//  ================
//
//  FILES_CompactPath - replace paths by a number in order to compact path strings;
//  FILES_RemoveOnePath - decrease the number of times a path is used;
//  FILES_RebuildFileName - rebuild a file path from a compacted string;
//  FILES_ChangePath - change the path of a file;
//
//  FILES_RetrievePath - for a given file type, change the current directory to the last
//                       directory used for this file type of to the one specified in the
//                       given file name;
//
//  =========
//  ALL PATHS
//  =========
//
//  FilesResetAllPaths - reset all the paths associated to the different files types;
//  FilesLoadAllPaths - load all paths from wds configuration file;
//  FilesSaveAllPaths - all paths safe keeping into the selected wds configuration file;
//
//  =============
//  DEFAULT PATHS
//  =============
//
//  FilesResetDefaultPaths - reset default paths;
//  FilesLoadDefaultPaths - load default paths from wds configuration file;
//  FilesSaveDefaultPaths - default paths safe keeping into the selected wds configuration file;
//
//  =====================================
//  UTILITIES FOR LOADING DATA FROM FILES        !!! obsolete, to replace by MATRIX_Load
//  =====================================
//
//  FILES_GetMatrixDimensions - retrieve the dimensions of the matrix from a file;
//  FILES_LoadMatrix - load a matrix from a file
//
//  ================================================
//  OPEN/SAVE COMMON DIALOG BOX FOR SELECTING A FILE
//  ================================================
//
//  FilesBuildFilter - build a filter string
//
//  FILES_Open - load and display "Open/Save" windows common dialog box;
//  FILES_BuildFileName - build a file name from a given name and extension;
//  FILES_Select - select a file of a given type;
//  FILES_Insert - MENU_CONTEXT_INSERT_FILES context menu command processing;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  FilesResetConfiguration - dispatch the reset command to all data structures;
//  FILES_LoadConfiguration - load WINDOAS settings from configuration file;
//  FilesSaveConfiguration - save WinDOAS settings in the configuration file;
//
//  ================================
//  "FILES" MENU COMMANDS PROCESSING
//  ================================
//
//  FILES_EnableSaveOption - enable/Disable "Save" menu option;
//  FILES_MenuNew - process MENU_FILE_NEW menu command;
//  FILES_MenuOpen - process MENU_FILE_OPEN menu command;
//  FILES_MenuSave - process MENU_FILE_SAVE menu command;
//  FILES_MenuSaveAs - process MENU_FILE_SAVE_AS menu command;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  FILES_Alloc - allocate and initialize buffers for file paths;
//  FILES_Free - release buffers allocated for file paths;
//
//  ----------------------------------------------------------------------------

// ===========
// DEFINITIONS
// ===========

#include <unistd.h>
#include <string.h>

#include "winfiles.h"

#include "doas.h"
#include "stdfunc.h"

FILE_TYPE FILES_types[FILE_TYPE_MAX] =
 {
  { "All Files", "*", "" },                                                     // FILE_TYPE_ALL
  { "ASCII Files","asc","" },                                                   // FILE_TYPE_ASCII_EXPORT
  { "ASCII Spectra Files","asc","" },                                           // FILE_TYPE_ASCII_SPECTRA
  { "Raw Spectra Files", "spe", "" },                                           // FILE_TYPE_SPECTRA
  { "Cross sections Files", "xs*", "" },                                        // FILE_TYPE_CROSS
  { "Reference spectra Files", "ref", "" },                                     // FILE_TYPE_REF
  { "SZA dependent AMF Files", "amf_sza", "" },                                 // FILE_TYPE_AMF_SZA
  { "Climatology dependent AMF Files", "amf_cli", "" },                         // FILE_TYPE_AMF_CLI
  { "Wavelength dependent AMF Files", "amf_wve", "" },                          // FILE_TYPE_AMF_WVE
  { "Spectra names Files", "nms", "" },                                         // FILE_TYPE_NAMES
  { "Dark current Files", "drk", "" },                                          // FILE_TYPE_DARK
  { "Interpixel variability Files", "vip", "" },                                // FILE_TYPE_INTERPIXEL
  { "Detector not linearity Files", "dnl", "" },                                // FILE_TYPE_NOT_LINEARITY
  { "Calibration Files", "clb", "" },                                           // FILE_TYPE_CALIB
  { "Kurucz Files", "Ktz*", "" },                                               // FILE_TYPE_CALIB_KURUCZ
  { "Slit functions", "slf", "" },                                              // FILE_TYPE_SLIT
  { "Instrumental functions", "ins", "" },                                      // FILE_TYPE_INSTR
  { "Filter Files", "flt", ""},                                                 // FILE_TYPE_FILTER
  { "Fits Files","fit",""},                                                     // FILE_TYPE_FIT
  { "WINDOAS settings","wds", "" },                                             // FILE_TYPE_INI
  { "Bitmap file","bmp","" },                                                   // FILE_TYPE_BMP
  { "Residuals","res","" },                                                     // FILE_TYPE_RES
  { "Paths","","" },                                                            // FILE_TYPE_PATH
  { "Configuration","cfg",""}                                                   // FILE_TYPE_CFG
 };

FILE_TYPE FILES_typeSpectra[FILE_TYPE_SPECTRA_MAX] =
 {
  { "ASCII Spectra files with header","asc","" },                               // WITH COMMENT
  { "ASCII Spectra files without header", "asc", "" }                           // WITHOUT COMMENT
 };

char FILES_configuration[DOAS_MAX_PATH_LEN+1];                                      // configuration file
// QDOAS ??? int FILES_version=HELP_VERSION_MAX-1;                                           // file version
FILES_PATH *FILES_paths;                                                        // all paths implied in configuration file

int FILES_nPaths=FILES_PATH_MAX;                                                // the size of the FILES_paths

// ================
// PATHS PROCESSING
// ================

// -----------------------------------------------------------------------------
// FUNCTION      FILES_CompactPath
// -----------------------------------------------------------------------------
// PURPOSE       replace paths by a number in order to compact path strings
//
// INPUT         path          the original path
//               useFileName   add the file name to the new path
//               addFlag       increment the number of times the path is used
//
// OUTPUT        newPath       the new path (%x\<file name>)
// -----------------------------------------------------------------------------

void FILES_CompactPath(char *newPath,char *path,int useFileName,int addFlag)
 {
  // Declarations

  char  pathTmp[DOAS_MAX_PATH_LEN+1],*ptr;
  INDEX  indexPath,indexFirst;
  SZ_LEN pathLength,pathTmpLength;

  // Initialization

  ptr=NULL;

  if (strlen(path))
   {
    // Initializations

    memcpy(pathTmp,path,DOAS_MAX_PATH_LEN+1);
    indexFirst=ITEM_NONE;

    // Extract file name

    if (useFileName && ((ptr=strrchr(pathTmp,PATH_SEP))!=NULL))
     *ptr++=0;

    if (pathTmp[0]!='%')
     {
      // Search for path in list

      for (indexPath=0,pathTmpLength=strlen(pathTmp);indexPath<FILES_nPaths;indexPath++)
       if (((pathLength=strlen(FILES_paths[indexPath].path))==pathTmpLength) && !strcasecmp(FILES_paths[indexPath].path,pathTmp))
        break;
       else if (addFlag && !pathLength && (indexFirst==ITEM_NONE))
        indexFirst=indexPath;

      // Add a new path in list

      if ((indexPath==FILES_nPaths) && (indexFirst!=ITEM_NONE))
       {
        memcpy(FILES_paths[indexFirst].path,pathTmp,DOAS_MAX_PATH_LEN+1);
        FILES_paths[indexFirst].count=0;
        indexPath=indexFirst;
       }
     }
    else
     sscanf(pathTmp,"%%%d",&indexPath);

    // Rebuild path

    if ((indexPath>=0) && (indexPath<FILES_nPaths))
     {
      if (useFileName && (ptr!=NULL))
       sprintf(newPath,"%%%d%c%s",indexPath,PATH_SEP,ptr);
      else
       sprintf(newPath,"%%%d",indexPath);

      if (addFlag)
       FILES_paths[indexPath].count++;
     }
    else if (newPath!=path)
     memcpy(newPath,path,DOAS_MAX_PATH_LEN+1);
   }
  else
   memset(newPath,0,DOAS_MAX_PATH_LEN);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_RemoveOnePath
// -----------------------------------------------------------------------------
// PURPOSE       decrease the number of times a path is used
//
// INPUT         path          the original path
// -----------------------------------------------------------------------------

void FILES_RemoveOnePath(char *path)
 {
  // Declaration

  INDEX indexPath;

  // Remove path

  if (strlen(path) && (path[0]=='%') && (sscanf(path,"%%%d",&indexPath)>=1) &&
     (indexPath>=0) && (indexPath<FILES_nPaths) && (FILES_paths[indexPath].count>0))

   FILES_paths[indexPath].count--;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_RebuildFileName
// -----------------------------------------------------------------------------
// PURPOSE       rebuild a file path from a compacted string
//
// INPUT         path          the compacted path
//               useFileName   add the file name to the new path
//
// OUTPUT        newPath       the new path (%x\<file name>)
//
// RETURN        pointer to the new path
// -----------------------------------------------------------------------------

char *FILES_RebuildFileName(char *newPath,const char *path,int useFileName)
 {
 	char pathTmp[DOAS_MAX_PATH_LEN+1],*ptr;

 	strcpy(pathTmp,path);

 	if (!useFileName)
 	 {
 	 	if ((ptr=strrchr(pathTmp,PATH_SEP))==NULL)
 	 	 pathTmp[0]='\0';
 	 	else
 	 	 *ptr='\0';
 	 }

 	strcpy(newPath,pathTmp);

// QDOAS ???  // Declarations
// QDOAS ???
// QDOAS ???  char pathTmp[DOAS_MAX_PATH_LEN+1],*ptr;
// QDOAS ???  INDEX indexPath;
// QDOAS ???
// QDOAS ???  // Initialization
// QDOAS ???
// QDOAS ???  if (path[0]=='%')
// QDOAS ???   {
// QDOAS ???    strcpy(pathTmp,path);
// QDOAS ???    ptr=NULL;
// QDOAS ???
// QDOAS ???    // Extract file name
// QDOAS ???
// QDOAS ???    if (useFileName && ((ptr=strrchr(pathTmp,PATH_SEP))!=NULL))
// QDOAS ???     *ptr++=0;
// QDOAS ???
// QDOAS ???    if ((sscanf(path,"%%%d",&indexPath)>0) && (indexPath>=0) && (indexPath<FILES_nPaths))
// QDOAS ???     {
// QDOAS ???      if (useFileName && (ptr!=NULL))
// QDOAS ???       sprintf(newPath,"%s%c%s",FILES_paths[indexPath].path,PATH_SEP,ptr);
// QDOAS ???      else
// QDOAS ???       strcpy(newPath,FILES_paths[indexPath].path);
// QDOAS ???     }
// QDOAS ???   }
// QDOAS ???  else if (path!=newPath)
// QDOAS ???   strcpy(newPath,path);

  // Return

  return newPath;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_ChangePath
// -----------------------------------------------------------------------------
// PURPOSE       change the path of a file
//
// INPUT         oldPath       the old path
//               useFileName   add the file name to the new path
//
// OUTPUT        newPath       the new path
// -----------------------------------------------------------------------------

void FILES_ChangePath(char *oldPath,char *newPath,int useFileName)
 {
  FILES_RemoveOnePath(oldPath);
  FILES_CompactPath(oldPath,newPath,useFileName,1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_RetrievePath
// -----------------------------------------------------------------------------
// PURPOSE       For a given file type, change the current directory to the last
//               directory used for this file type of to the one specified in the
//               given file name
//
// INPUT         pathStringLength    maximum length for the output path string
//               fullFileName        the given file name
//               fullFileNameLength  the length of the given file name
//               indexFileType       the file type to focus on
//               changeDefaultPath   1 to update the default path associated to
//                                   the given file type
//
// OUTPUT        pathString          the new path
// -----------------------------------------------------------------------------

void FILES_RetrievePath(char *pathString,SZ_LEN pathStringLength,
                        char *fullFileName,SZ_LEN fullFileNameLength,
                        int    indexFileType,int changeDefaultPath)
 {
  // Declarations

  SZ_LEN currentLength;
  char *ptr;

  // Initialization

  currentLength=0;

  // Set path to the default one associated to the type of selected file

  if (pathString!=FILES_types[indexFileType].defaultPath)
   memcpy(pathString,FILES_types[indexFileType].defaultPath,DOAS_MAX_PATH_LEN+1);

  if ((fullFileName!=NULL) && (fullFileNameLength>0) && (fullFileNameLength<=pathStringLength))
   {
    // Use the selected file path as the default one

    memcpy(pathString,fullFileName,DOAS_MAX_PATH_LEN+1);

    if ((ptr=strrchr(pathString,PATH_SEP))!=NULL)
     {
      *ptr='\0';
      currentLength=strlen(pathString);
      memset(ptr,0,pathStringLength-currentLength);
     }

    if ((ptr=strrchr(pathString,PATH_SEP))==NULL)
     pathString[currentLength]=PATH_SEP;
   }

  // Set current directory as the default path

  if (!strlen(FILES_RebuildFileName(pathString,pathString,0)))
   getcwd(pathString,sizeof(pathString));

// WIN32-only:   GetCurrentDirectory(sizeof(pathString),pathString);

  if (changeDefaultPath)
   FILES_ChangePath(FILES_types[indexFileType].defaultPath,pathString,0);
 }

// =========
// ALL PATHS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      FilesResetAllPaths
// -----------------------------------------------------------------------------
// PURPOSE       reset all the paths associated to the different files types
// -----------------------------------------------------------------------------

void FilesResetAllPaths(void)
 {
  memset(FILES_paths,0,sizeof(FILES_PATH)*FILES_nPaths);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesLoadAllPaths
// -----------------------------------------------------------------------------
// PURPOSE       load all paths from wds configuration file
//
// INPUT         fileLine  the current line in the wds configuration file
// -----------------------------------------------------------------------------

void FilesLoadAllPaths(char *fileLine)
 {
  // Declarations

  char path[DOAS_MAX_PATH_LEN+1];           // path extracted from fileLine
  INDEX indexPath;

  // Initialization

  memset(path,0,DOAS_MAX_PATH_LEN+1);

  // Line decomposition

  sscanf(fileLine,"%%%d=%[^',']",&indexPath,path);

  // Path safe keeping

  if ((indexPath>=0) && (indexPath<FILES_nPaths))
   memcpy(FILES_paths[indexPath].path,path,DOAS_MAX_PATH_LEN+1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesSaveAllPaths
// -----------------------------------------------------------------------------
// PURPOSE       all paths safe keeping into the selected wds configuration file
//
// INPUT         fp            pointer to the current wds configuration file;
//               sectionName   name of the all paths section in the configuration file
// -----------------------------------------------------------------------------

void FilesSaveAllPaths(FILE *fp,char *sectionName)
 {
  // Declaration

  INDEX indexPath;

  // Print section name

  fprintf(fp,"[%s]\n\n",sectionName);

  // Browse types of files

  for (indexPath=0;indexPath<FILES_nPaths;indexPath++)
   if (strlen(FILES_paths[indexPath].path) && FILES_paths[indexPath].count)
    fprintf(fp,"%%%d=%s,%d\n",indexPath,FILES_paths[indexPath].path,FILES_paths[indexPath].count);

  fprintf(fp,"\n");
 }

// =============
// DEFAULT PATHS
// =============

// -----------------------------------------------------------------------------
// FUNCTION      FilesResetDefaultPaths
// -----------------------------------------------------------------------------
// PURPOSE       reset default paths
// -----------------------------------------------------------------------------

void FilesResetDefaultPaths(void)
 {
 	// Declaration

  INDEX indexFileType;

  // Browse the different file types

  for (indexFileType=0;indexFileType<FILE_TYPE_MAX;indexFileType++)
   memset(FILES_types[indexFileType].defaultPath,0,DOAS_MAX_PATH_LEN+1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesLoadDefaultPaths
// -----------------------------------------------------------------------------
// PURPOSE       load default paths from the wds configuration file
//
// INPUT         fileLine  the current line in the wds configuration file
// -----------------------------------------------------------------------------

void FilesLoadDefaultPaths(char *fileLine)
 {
  // Declarations

  char keyName[MAX_ITEM_TEXT_LEN],                                           // name on the left of '=' symbol in a wds statement
        defaultPath[DOAS_MAX_PATH_LEN+1];                                            // default path extracted from fileLine
  INDEX indexFileType;

  // Initialization

  memset(defaultPath,0,DOAS_MAX_PATH_LEN);

  // Line decomposition

  sscanf(fileLine,"%[^=]=%[^\n]",keyName,defaultPath);

  // Search for keyName in types of files list

  for (indexFileType=0;indexFileType<FILE_TYPE_MAX;indexFileType++)
   if (!strcasecmp(FILES_types[indexFileType].fileType,keyName))
    FILES_CompactPath(FILES_types[indexFileType].defaultPath,defaultPath,0,1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesSaveDefaultPaths
// -----------------------------------------------------------------------------
// PURPOSE       default paths safe keeping into the selected wds configuration file
//
// INPUT         fp            pointer to the current wds configuration file;
//               sectionName   name of the default paths section in the configuration file
// -----------------------------------------------------------------------------

void FilesSaveDefaultPaths(FILE *fp,char *sectionName)
 {
  // Declaration

  INDEX indexFileType;

  // Print section name

  fprintf(fp,"[%s]\n\n",sectionName);

  // Browse types of files

  for (indexFileType=0;indexFileType<FILE_TYPE_MAX;indexFileType++)
   fprintf(fp,"%s=%s\n",FILES_types[indexFileType].fileType,FILES_types[indexFileType].defaultPath);

  fprintf(fp,"\n");
 }

// =====================================
// UTILITIES FOR LOADING DATA FROM FILES
// =====================================

// -----------------------------------------------------------------------------
// FUNCTION      FILES_GetMatrixDimensions (obsolete, to replace by MATRIX_Load)
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve the dimensions of the matrix from a file
//
// INPUT         fp               pointer to the file to load;
//               fileName         the name of the file to load;
//               callingFunction  the name of the calling function;
//               errorType        type of error to return;
//
// OUTPUT        pNl,pNc          the dimension of the matrix to load
//
// RETURN        ERROR_ID_NO in case of success
//               ERROR_ID_ALLOC on allocation error
//               ERROR_ID_FILE_NOT_FOUND if can not open the input file
//               ERROR_ID_FILE_EMPTY if the input file is empty
// -----------------------------------------------------------------------------

RC FILES_GetMatrixDimensions(FILE *fp,const char *fileName,int *pNl,int *pNc,const char *callingFunction,int errorType)
 {
  // Declarations

  char *oldColumn,*nextColumn;                                                 // line of files
  int    nl,nc,                                                                 // dimensions of the matrix
         lineLength,                                                            // length of file line
         fileLength;                                                            // total length of file
  double tempValue;                                                             // temporary value
  RC     rc;                                                                    // return code

  // Initializations

  rc=ERROR_ID_NO;
  oldColumn=nextColumn=NULL;
  nl=nc=0;

  if (!(fileLength=STD_FileLength(fp)))
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_FILE_NOT_FOUND,fileName);

  // Allocate buffers for lines of file

  else if (((oldColumn=(char *)MEMORY_AllocBuffer("FILES_GetMatrixDimensions ","oldColumn",fileLength,sizeof(char),0,MEMORY_TYPE_STRING))==NULL) ||
           ((nextColumn=(char *)MEMORY_AllocBuffer("FILES_GetMatrixDimensions ","nextColumn",fileLength,sizeof(char),0,MEMORY_TYPE_STRING))==NULL))
   rc=ERROR_ID_ALLOC;
  else
   {
    // Go to the first no comment line

    while (fgets(oldColumn,fileLength,fp) && ((strchr(oldColumn,';')!=NULL) || (strchr(oldColumn,'*')!=NULL)));

    // Determine the number of columns

    for (nc=0;strlen(oldColumn);nc++)
     {
      lineLength=strlen(oldColumn);
      oldColumn[lineLength++]='\n';
      oldColumn[lineLength]=0;

      memset(nextColumn,0,fileLength);
      sscanf(oldColumn,"%lf %[^'\n']",(double *)&tempValue,nextColumn);
      strcpy(oldColumn,nextColumn);
     }

    // Determine the number of lines

    for (nl=1;!feof(fp) && fgets(oldColumn,fileLength,fp);)
     if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL))
      nl++;
   }

  if (!nl || !nc)
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_FILE_EMPTY,fileName);

  // Release allocated buffers

  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_GetMatrixDimensions ","oldColumn",oldColumn);
  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_GetMatrixDimensions ","nextColumn",nextColumn);

  // Return

  *pNl=nl;
  *pNc=nc;

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_LoadMatrix (obsolete, to replace by MATRIX_Load)
// -----------------------------------------------------------------------------
// PURPOSE       Load a matrix from file
//
// INPUT         fp               pointer to the file to load;
//               fileName         the name of the file to load;
//               base,nl,nc       resp., starting index for lines, number of lines and number of columns
//               errorType        type of error to return;
//
// OUTPUT        matrix           the matrix
//
// RETURN        ERROR_ID_NO in case of success
//               ERROR_ID_ALLOC on allocation error
//               ERROR_ID_FILE_NOT_FOUND if can not open the input file
// -----------------------------------------------------------------------------

RC FILES_LoadMatrix(FILE *fp,const char *fileName,double **matrix,int base,int nl,int nc,const char *callingFunction,int errorType)
 {
  // Declarations

  char *oldColumn,*nextColumn;                             // buffers for file lines
  int    lineLength,                                        // length of a file line
         fileLength;                                        // total length of file
  INDEX  i,j;                                               // index for browsing lines and columns in file
  RC     rc;                                                // return code

  // Initializations

  oldColumn=nextColumn=NULL;
  rc=ERROR_ID_NO;

  // File open

  if (!(fileLength=STD_FileLength(fp)))
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_FILE_NOT_FOUND,fileName);

  // Allocate buffers for lines of file

  else if (((oldColumn=(char *)MEMORY_AllocBuffer("FILES_LoadMatrix ","oldColumn",fileLength,sizeof(char),0,MEMORY_TYPE_STRING))==NULL) ||
           ((nextColumn=(char *)MEMORY_AllocBuffer("FILES_LoadMatrix ","oldColumn",fileLength,sizeof(char),0,MEMORY_TYPE_STRING))==NULL))
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_ALLOC);
  else
   {
    // File read out

    for (i=base,nl+=base-1;(i<=nl) && fgets(oldColumn,fileLength,fp);)

     if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL))
      {
       for (j=1;j<=nc;j++)
        {
         lineLength=strlen(oldColumn);

         oldColumn[lineLength++]='\n';
         oldColumn[lineLength]=0;

         memset(nextColumn,0,fileLength);
         sscanf(oldColumn,"%lf %[^'\n']",(double *)&matrix[j][i],nextColumn);
         strcpy(oldColumn,nextColumn);
        }

       i++;
      }
   }

  // Release allocated buffers

  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_LoadMatrix ","oldColumn",oldColumn);
  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_LoadMatrix ","nextColumn",nextColumn);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_BuildFileName
// -----------------------------------------------------------------------------
// PURPOSE       Build a file name from a given name and extension
//
// INPUT/OUTPUT  fileName  the original file name
// INPUT         fileType  the type of file to build
//
// OUTPUT        pointer to the updated file name
// -----------------------------------------------------------------------------

char *FILES_BuildFileName(char *fileName,MASK fileType)
 {
  // Declarations

  SZ_LEN fileNameLength;
  char *ptr;

  // Replace file extension by the correct one

  if (((fileNameLength=strlen(fileName))!=0) && (fileType>FILE_TYPE_ALL))
   {
    if ((ptr=strrchr(fileName,'.'))==NULL)
     ptr=&fileName[fileNameLength];

    sprintf(ptr,".%s",FILES_types[fileType].fileExt);
   }

  // Return

  return fileName;
 }

