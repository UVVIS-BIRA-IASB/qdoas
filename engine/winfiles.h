#ifndef FILES_H
#define FILES_H

#include <stdio.h>

#include "comdefs.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// ================
// FILES PROCESSING
// ================

// ------------------------------------
// CONSTANTS AND STRUCTURES DEFINITIONS
// ------------------------------------

// Base for building file filter
// -----------------------------

typedef struct _fileType
 {
  char fileType[MAX_ITEM_DESC_LEN+1];          // type of files
  char fileExt[12];                            // extension associated to this type of files
  char defaultPath[DOAS_MAX_PATH_LEN+1];            // default path
 }
FILE_TYPE;

// Paths for files
// ---------------

typedef struct _filePath
 {
  char path[DOAS_MAX_PATH_LEN+1];
  int   count;
 }
FILES_PATH;

// ----------------
// GLOBAL VARIABLES
// ----------------

extern char FILES_configuration[];            // configuration file
extern FILE_TYPE FILES_types[];                     // types of files supported by application
extern int FILES_version;                           // program version
extern FILES_PATH *FILES_paths;                     // all paths implied in configuration file
extern int FILES_nPaths;                            // the size of the previous buffer

// ----------
// PROTOTYPES
// ----------

// Load data from files
// --------------------

void   FILES_CompactPath(char *newPath,char *path,int useFileName,int addFlag);
char *FILES_RebuildFileName(char *newPath,const char *path,int useFileName);
void   FILES_ChangePath(char *oldPath,char *newPath,int useFileName);
void   FILES_RemoveOnePath(char *path);
void   FILES_RetrievePath(char *pathString,SZ_LEN pathStringLength,char *fullFileName,SZ_LEN fullFileNameLength,int indexFileType,int changeDefaultPath);

RC     FILES_GetMatrixDimensions(FILE *fp,const char *fileName,int *pNl,int *pNc,const char *callingFunction,int errorType);
RC     FILES_LoadMatrix(FILE *fp,const char *fileName,double **matrix,int base,int nl,int nc,const char *callingFunction,int errorType);

// Select a file
// -------------

char  *FILES_BuildFileName(char *fileName,MASK fileType);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
