#ifndef WINSYMB_H
#define WINSYMB_H

// =======
// SYMBOLS
// =======

#include "comdefs.h"

#define MAX_SYMBOL_CROSS         60                                          // default maximum number of cross sections symbols

enum _predefinedSymbols {
  SYMBOL_PREDEFINED_SPECTRUM,                                                // spectrum
  SYMBOL_PREDEFINED_REF,                                                     // reference
  SYMBOL_PREDEFINED_COM,                                                     // common residual
  SYMBOL_PREDEFINED_USAMP1,                                                  // undersampling phase 1
  SYMBOL_PREDEFINED_USAMP2,                                                  // undersampling phase 2
  SYMBOL_PREDEFINED_RESOL,                                                   // resol synthetic spectrum (reference spectrum convolved with a very thin slit function)
  SYMBOL_PREDEFINED_MAX
};

// Symbol description
// ------------------

typedef struct _symbol {
  char name[MAX_ITEM_NAME_LEN+1];
  char description[MAX_ITEM_DESC_LEN+1];
} SYMBOL;

typedef SYMBOL SYMBOL_CROSS;

// Global variables
// ----------------

extern SYMBOL_CROSS *SYMB_itemCrossList;                                     // pointer to list of cross sections symbols
extern int SYMB_itemCrossN;

INDEX SYMB_GetListIndex(SYMBOL *symbolList,int symbolNumber,char *symbolName);
RC SYMB_Add(char *symbolName,char *symbolDescription);

RC    SYMB_Alloc(void);
void  SYMB_Free(void);

#endif
