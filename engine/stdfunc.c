
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  STANDARD UTILITY FUNCTIONS
//  Name of module    :  STDFUNC.C
//  Creation date     :  21 October 2004
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
//  FUNCTIONS
//
//  =======
//  STRINGS
//  =======
//
//  STD_StrTrim - trim trailing spaces and unusable characters (carriage return...)
//
//  StrGetFormat - retrieve from a format string, the format of the first token
//
//  STD_Sscanf - this function should work as sscanf except that it's dedicated
//               to string retrieved from WinDOAS configuration files
//
//  STD_Strupr/STD_Strlwr -
//
//      these functions convert a string to upper / lower case characters,
//      to replace similar strupr/strlwr functions when those are not supported
//
//  =====================
//  FILES AND DIRECTORIES
//  =====================
//
//  STD_IsDir - this function checks whether the given name is a directory,
//              to replace a similar Windows function
//
//  STD_FileLength - this function returns the length (size) of a file,
//                   to replace the original filelength function that is not
//                   supported by all compilers
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  This module provides some utility functions that couldn't be supported by
//  all operating systems and compilers.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "stdfunc.h"
#include "doas.h"

#define MAX_FORMAT_LEN    15                                                    // maximum number of characters for a string describing the format of a token

// =======
// STRINGS
// =======

// -----------------------------------------------------------------------------
// FUNCTION      STD_StrTrim
// -----------------------------------------------------------------------------
// PURPOSE       Trim trailing spaces and unusable characters (carriage return...)
//
// INPUT         str  the string to trim
//
// RETURN        a pointer to the input string
// -----------------------------------------------------------------------------

char *STD_StrTrim(char *str)
 {
 	// Declaration

 	int i;

 	// Browse characters from the end of the string

 	for (i=strlen(str)-1;i>=0;i--)
 	 if ((str[i]=='\n') || (str[i]=='\r') || (str[i]=='\t') || (str[i]==' '))
 	  str[i]='\0';
 	 else
 	  break;

  // Return

  return str;
 }

// -----------------------------------------------------------------------------
// FUNCTION      StrGetFormat
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve from a format string, the format of the first token
//
// INPUT         formatString  a format string (for several tokens)
//
// OUTPUT        formatToken   the format of the first token
//
// RETURN        a pointer to the format of the next expected token
//               NULL if the end of the format string is reached
// -----------------------------------------------------------------------------

char *StrGetFormat(char *formatString,char *formatToken)
 {
  // Declarations

  int   typeSpecified=0,                                                        // flag : 1 is the type has been specified in the format token, 0 otherwise
        percentFlag=0,                                                          // flag : 1 when the percent character is met
        bracketOpen=0,                                                          // flag : 1 if the bracket is open after a percent character
        bracketEnd=0,                                                           // flag : 1 to close the bracket
        otherFlag=0;                                                            // flag : 1 if the current character do not set any of the previous flag

  INDEX i;

  // Initialization

  memset(formatToken,0,MAX_FORMAT_LEN);

  // Browse characters in the input format string

  for (i=0;
      (formatString[i]!='\0') &&                                                // stop if the null character is found
     ((formatString[i]!='%') || (!percentFlag && !otherFlag)) &&                //   or if the percent flag starts a new token string
      !bracketEnd && !typeSpecified;                                            //   or at the end of a bracketted expression or of a format token
       i++)
   {
    // the percent character starts the token string

    if (formatString[i]=='%')
     percentFlag=1;

    // meet a bracket to exclude characters

    else if ((formatString[i]=='[') && percentFlag)
     bracketOpen=1;
    else if ((formatString[i]==']') && bracketOpen)
     bracketEnd=1;
    else if ((percentFlag) &&
            ((formatString[i]=='d') ||
             (formatString[i]=='f') ||
             (formatString[i]=='s')))

     typeSpecified=1;
    else
     otherFlag=1;

    // Copy the character in the token

    formatToken[i]=formatString[i];
   }

  formatToken[i]='\0';

  // Return

  return (formatString[i]=='\n')?NULL:&formatString[i];
 }

// QTDOAS ??? // -----------------------------------------------------------------------------
// QTDOAS ??? // FUNCTION      STD_Sscanf
// QTDOAS ??? // -----------------------------------------------------------------------------
// QTDOAS ??? // PURPOSE       this function should work as sscanf except that it's dedicated
// QTDOAS ??? //               to string retrieved from WinDOAS configuration files
// QTDOAS ??? //
// QTDOAS ??? // INPUT         line          a line retrieved from the WinDOAS configuration file
// QTDOAS ??? //               formatString  the format string
// QTDOAS ??? //               ...           the list of expected arguments
// QTDOAS ??? //
// QTDOAS ??? // OUTPUT        formatToken   the format of the first token
// QTDOAS ??? //
// QTDOAS ??? // RETURN        a pointer to the format of the next expected token
// QTDOAS ??? //               NULL if the end of the format string is reached
// QTDOAS ??? //
// QTDOAS ??? // REMARK        as this function is dedicated to one application, all the
// QTDOAS ??? //               possible situations may not to have been explored
// QTDOAS ??? // -----------------------------------------------------------------------------
// QTDOAS ???
// QTDOAS ??? int STD_Sscanf(char *line,char *formatString,...)
// QTDOAS ???  {
// QTDOAS ???   // Declarations
// QTDOAS ???
// QTDOAS ???   char tokenFormat[MAX_FORMAT_LEN+1];                                          // the expected format for the current token
// QTDOAS ???   char *tfPtr,*tkOld,*tkNext;                                                  // pointers to substrings
// QTDOAS ???   INT strFlag,intFlag,doubleFlag;                                               // flag set according to the expected type in the format string
// QTDOAS ???   va_list argList;                                                              // variable arguments list
// QTDOAS ???   void  *argPtr;                                                                // pointer to the next argument in the previous list
// QTDOAS ???   int ntoken;                                                                   // the number of tokens read by the function
// QTDOAS ???   INDEX i;                                                                      // browse characters
// QTDOAS ???
// QTDOAS ???   // Open the variable argument list
// QTDOAS ???
// QTDOAS ???   va_start(argList,formatString);
// QTDOAS ???
// QTDOAS ???   // Use the format string to split the line in tokens
// QTDOAS ???
// QTDOAS ???   for (tfPtr=StrGetFormat(formatString,tokenFormat),tkOld=line,ntoken=0;
// QTDOAS ???       (tfPtr!=NULL) && strlen(tokenFormat) && (tkOld!=NULL) && *tkOld!='\n';
// QTDOAS ???        tkOld=tkNext)
// QTDOAS ???    {
// QTDOAS ???     strFlag=intFlag=doubleFlag=0;
// QTDOAS ???
// QTDOAS ???     // ================
// QTDOAS ???     // FORMATTED STRING
// QTDOAS ???     // ================
// QTDOAS ???
// QTDOAS ???     if (tokenFormat[0]=='%')
// QTDOAS ???      {
// QTDOAS ???       // Retrieve the expected type from the format string
// QTDOAS ???
// QTDOAS ???       for (i=1;(tokenFormat[i]!='\0') && !intFlag && !doubleFlag && !strFlag;i++)
// QTDOAS ???
// QTDOAS ???        if (tokenFormat[i]=='d')
// QTDOAS ???         intFlag=1;
// QTDOAS ???        else if (tokenFormat[i]=='f')
// QTDOAS ???         doubleFlag=1;
// QTDOAS ???        else if ((tokenFormat[i]=='[') || (tokenFormat[i]=='^'))
// QTDOAS ???         strFlag=1;
// QTDOAS ???
// QTDOAS ???       // Read the token
// QTDOAS ???
// QTDOAS ???       if (!sscanf(tkOld,tokenFormat,(argPtr=(void *)va_arg(argList,void *))))
// QTDOAS ???        {
// QTDOAS ???         // In case the expected token doesn't exist, initialize it
// QTDOAS ???
// QTDOAS ???         if (intFlag)
// QTDOAS ???          *((int *)argPtr)=0;
// QTDOAS ???         else if (doubleFlag)
// QTDOAS ???          *((double *)argPtr)=(double)0.;
// QTDOAS ???         else
// QTDOAS ???          *((char *)argPtr)='\0';
// QTDOAS ???
// QTDOAS ???         tkNext=tkOld;
// QTDOAS ???        }
// QTDOAS ???
// QTDOAS ???       else if ((intFlag || doubleFlag))
// QTDOAS ???        {
// QTDOAS ???         if ((tkNext=strchr(tkOld,','))==NULL)
// QTDOAS ???          tkNext=strchr(tkOld,'\n');
// QTDOAS ???        }
// QTDOAS ???       else if (strlen((char *)argPtr))
// QTDOAS ???        {
// QTDOAS ???         if ((tkNext=strstr(tkOld,(char *)argPtr))!=NULL)
// QTDOAS ???          tkNext+=strlen((char *)argPtr);
// QTDOAS ???        }
// QTDOAS ???
// QTDOAS ???       ntoken++;
// QTDOAS ???      }
// QTDOAS ???
// QTDOAS ???     // =================
// QTDOAS ???     // BYPASS CHARACTERS
// QTDOAS ???     // =================
// QTDOAS ???
// QTDOAS ???     else if ((tkNext=strstr(tkOld,tokenFormat))!=NULL)
// QTDOAS ???      tkNext+=strlen(tokenFormat);
// QTDOAS ???
// QTDOAS ???     // Go to the next token
// QTDOAS ???
// QTDOAS ???     if (tfPtr!=NULL)
// QTDOAS ???      tfPtr=StrGetFormat(tfPtr,tokenFormat);
// QTDOAS ???    }
// QTDOAS ???
// QTDOAS ???   // Close the argument list
// QTDOAS ???
// QTDOAS ???   va_end(argList);
// QTDOAS ???
// QTDOAS ???   // return
// QTDOAS ???
// QTDOAS ???   return ntoken;
// QTDOAS ???  }

// -----------------------------------------------------------------------------
// FUNCTION      STD_Strupr / STD_Strlwr
// -----------------------------------------------------------------------------
// PURPOSE       these functions convert a string to upper / lower case characters,
//               to replace similar strupr/strlwr functions when those are not supported
//
// INPUT         character string to convert
//
// OUTPUT        converted character string
//
// RETURN        converted character string
// -----------------------------------------------------------------------------

char *STD_Strupr(char *n)
 {
  int i;
  for (i=0;n[i];i++)
  	n[i] = (char)toupper(n[i]);
  return n;
 }

char *STD_Strlwr(char *n)
 {
  int i;
  for (i=0;n[i];i++)
  	n[i] = (char)tolower(n[i]);
  return n;
 }

// -----------------------------------------------------------------------------
// FUNCTION      STD_StrRep
// -----------------------------------------------------------------------------
// PURPOSE       Replace all occurrences of character old by character new in n
//
// INPUT         n : character string to convert
//               oldchar : character to replace
//               newchar ; new character
//
// OUTPUT        converted character string
//
// RETURN        converted character string
// -----------------------------------------------------------------------------

char *STD_StrRep(char *n,char oldchar,char newchar)
 {
  int i;
  for (i=0;n[i];i++)
   if (n[i]==oldchar)
  	 n[i] = newchar;
  return n;
 }

// =====================
// FILES AND DIRECTORIES
// =====================

// -----------------------------------------------------------------------------
// FUNCTION      STD_IsDir
// -----------------------------------------------------------------------------
// PURPOSE       this function checks whether the given name is a directory,
//               to replace a similar Windows function
//
// INPUT         filename
//
// OUTPUT        --
//
// RETURN        rc = 1 : is a directory
//                    0 : is a file
//                   -1 : does not exist, error, or something like that
// -----------------------------------------------------------------------------

int STD_IsDir(char *filename)
 {
  struct stat fileinfo;
  int rc;

  if ( stat(filename,&fileinfo) == -1 )
   rc=-1;
  else
   {
   	if ( ( fileinfo.st_mode & S_IFMT ) == S_IFDIR )
     rc=1;
    else
     rc=0;
   }

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      STD_FileLength
// -----------------------------------------------------------------------------
// PURPOSE       this function returns the length (size) of a file,
//               to replace the original filelength function that is not
//               supported by all compilers
//
// INPUT         filename pointer
//
// OUTPUT        --
//
// RETURN        size of the file (in bytes)
// -----------------------------------------------------------------------------

long STD_FileLength(FILE *fp)
 {
 	// Declarations

  int32_t fileSize=0L;                                                             // the size of the file
  int32_t oldPos;                                                                  // the position of the file pointer

  // Calculate the file size

  if (fp!=NULL)
   {
    oldPos=ftell(fp);                                                           // get the current position of the pointer
    fseek(fp,0,SEEK_END);                                                       // go to the end of the file
    fileSize=ftell(fp);                                                         // return the size of the file
    fseek(fp,oldPos,SEEK_SET);                                                  // go back to the previous position
   }

  // Return

  return(fileSize);
 }

/* timegm implementation taken from
  http://www.catb.org/esr/time-programming/

  "The following implementation does the calendrical calculations and
  is thread-safe, but may fail for calendar dates after
  2038-01-19T03:14:07Z or before 1901-12-13T20:45:53Z if any of int or
  long or time_t are 32 bits. If all are 64 bits, there are still
  overflow vulnerabilities enough billions of years in the future or
  past, or if you poke stupidly huge values into the input struct tm
  members."*/
time_t STD_timegm(register struct tm * t)
/* struct tm to seconds since Unix epoch */
{
    register long year;
    register time_t result;
#define MONTHSPERYEAR   12      /* months per calendar year */
    static const int cumdays[MONTHSPERYEAR] =
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    /*@ +matchanyintegral @*/
    year = 1900 + t->tm_year + t->tm_mon / MONTHSPERYEAR;
    result = (year - 1970) * 365 + cumdays[t->tm_mon % MONTHSPERYEAR];
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    if ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0) &&
        (t->tm_mon % MONTHSPERYEAR) < 2)
        result--;
    result += t->tm_mday - 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    if (t->tm_isdst == 1)
        result -= 3600;
    /*@ -matchanyintegral @*/
    return (result);
}
