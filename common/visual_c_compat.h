#ifndef VISUAL_C_COMPAT_H
#define VISUAL_C_COMPAT_H

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define getcwd _getcwd
#endif

#endif
