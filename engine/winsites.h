#ifndef WINSITES_H
#define WINSITES_H

#include "comdefs.h"
#include "mediate_limits.h"
// Observation site properties

typedef struct _observationSites {
  char name[SITE_NAME_BUFFER_LENGTH];
  char abbrev[SITE_ABBREV_BUFFER_LENGTH];
  double longitude;
  double latitude;
  double altitude;
} OBSERVATION_SITE;

// Global variables
// ----------------

extern OBSERVATION_SITE  *SITES_itemList;                                    // pointer to the list of sites objects
extern int SITES_itemN;                                                      // the number of items in the previous list

// Prototypes
// ----------

RC      SITES_Add(OBSERVATION_SITE *pNewSite);
RC      SITES_Alloc(void);
void    SITES_Free(void);

INDEX   SITES_GetIndex(const char *siteName);

#endif
