#ifndef STDFUNC_H
#define STDFUNC_H

#include <stdio.h>
#include <time.h>

// ======================================
// stdfunc.h : STANDARD UTILITY FUNCTIONS
// ======================================

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

char      *STD_StrTrim(char *str);
int         STD_Sscanf(char *line,char *formatString,...);
long        STD_FileLength(FILE *fp);

char       *STD_Strupr(char *n);
char       *STD_Strlwr(char *n);
char       *STD_StrRep(char *n,char oldchar,char newchar);

int         STD_IsDir(char *filename);

time_t STD_timegm(register struct tm * t);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
