
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  SYMBOLS MODULE
//  Name of module    :  WINSYMB.C
//  Creation date     :  January 1997
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
//  This module handles a list of symbols in the Environment space tree.   These
//  symbols are associated to the different cross sections.  They make the
//  selection of cross section files and their tracking during the DOAS analysis
//  easier (for example, to find their location in the SVD matrix).
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ==================
//  SYMBOLS PROPERTIES
//  ==================
//
//  SymbolsDlgInit - initialize the symbols dialog box with the description of
//                   the selected item in the 'environment space' tree;
//
//  SymbolsSet - update the properties of the selected symbol;
//  SYMB_WndProc - dispatch messages from the symbols dialog box;
//
//  ===================
//  SEARCH FOR A SYMBOL
//  ===================
//
//  SYMB_GetListIndex - search for a symbol associated to a file name;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  SYMB_Alloc - allocate and initialize buffers for the symbols;
//  SYMB_Free - release the buffers allocated for the symbols;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  SYMB_ResetConfiguration - remove the current list of symbols from the
//                            'Environment space' tree;
//  SYMB_LoadConfiguration - load symbols from the wds configuration file;
//  SYMB_SaveConfiguration - save the list of symbols in the wds in the configuration file;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>

#include "winsymb.h"

// =================
// GLOBALS VARIABLES
// =================

SYMBOL_CROSS *SYMB_itemCrossList=NULL;                                          // pointer to the list of user-defined symbols
int SYMB_itemCrossN;                                                            // the number of items in the previous list

// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // SEARCH FOR A SYMBOL
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_GetListIndex
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Search for a symbol associated to a file name
// QDOAS ??? //
// QDOAS ??? // INPUT           symbolList   : the list of symbols in which to search for;
// QDOAS ??? //                 symbolNumber : the size of the input list
// QDOAS ??? //                 fileName     : the name of the file associated to the symbol
// QDOAS ??? //
// QDOAS ??? // RETURN          the index in the list, of the symbol associated to the file
// QDOAS ??? //                 ITEM_NONE if not found
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX SYMB_GetListIndex(SYMBOL *symbolList,int symbolNumber,char *fileName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   char symbolName[MAX_PATH_LEN+1],*ptr;                                        // the name of the symbol to search for
// QDOAS ???   INDEX indexSymbol;                                                            // browse symbols in the input list
// QDOAS ???   SZ_LEN symbolNameLen;                                                         // the length of the name of the symbol
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   memset(symbolName,0,MAX_PATH_LEN+1);
// QDOAS ???   indexSymbol=ITEM_NONE;
// QDOAS ???
// QDOAS ???   if ((symbolList!=NULL) && (symbolNumber>0) && (strlen(fileName)>0))
// QDOAS ???    {
// QDOAS ???     // Retrieve the name of the symbol from the file name
// QDOAS ???
// QDOAS ???     strncpy(symbolName,fileName,MAX_PATH_LEN);
// QDOAS ???
// QDOAS ???     // The symbol name should start the file name (so search for the last '\' (WINDOWS) or '/' (UNIX,LINUX)
// QDOAS ???
// QDOAS ???     if ((ptr=strrchr(fileName,PATH_SEP))!=NULL)
// QDOAS ???      strcpy(symbolName,ptr+1);
// QDOAS ???
// QDOAS ???     // Search for the end of the symbol ('_' if a description is given in the file name or '.' for the file extension)
// QDOAS ???
// QDOAS ???     if (((ptr=strchr(symbolName,'_'))!=NULL) || ((ptr=strchr(symbolName,'.'))!=NULL))
// QDOAS ???        *ptr=0;
// QDOAS ???
// QDOAS ???     // In the case a list of cross sections files is provided, stop at the first file
// QDOAS ???
// QDOAS ???     if ((ptr=strchr(symbolName,';'))!=NULL)
// QDOAS ???      *ptr=0;
// QDOAS ???
// QDOAS ???     // Get the length of the name of the symbol
// QDOAS ???
// QDOAS ???     symbolNameLen=strlen(symbolName);
// QDOAS ???
// QDOAS ???     // Search for the symbol in the input list
// QDOAS ???
// QDOAS ???     for (indexSymbol=0;indexSymbol<symbolNumber;indexSymbol++)
// QDOAS ???      if (((int)strlen(symbolList[indexSymbol].name)==symbolNameLen) &&
// QDOAS ???          !strcasecmp(symbolList[indexSymbol].name,symbolName))
// QDOAS ???       break;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return (indexSymbol<symbolNumber) ? indexSymbol : ITEM_NONE;
// QDOAS ???  }
// QDOAS ???

// -----------------------------------------------------------------------------
// FUNCTION        SYMB_Add
// -----------------------------------------------------------------------------
// PURPOSE         Add a symbol in the list
//
// INPUT           symbolName         the name of the symbol
//                 symbolDescription  the description of the symbol
//
// RETURN          ERROR_ID_BUFFER_FULL if the maximum number of symbols is reached
//                 ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC SYMB_Add(char *symbolName,char *symbolDescription)
 {
 	// Declaration

 	RC rc;

 	// Initialization

 	rc=ERROR_ID_NO;

 	// Check for the buffer limits

  if (SYMB_itemCrossN==MAX_SYMBOL_CROSS)
   rc=ERROR_SetLast("SYMB_Add",ERROR_TYPE_FATAL,ERROR_ID_BUFFER_FULL,"symbols");
  else
   {
   	strcpy(SYMB_itemCrossList[SYMB_itemCrossN].name,symbolName);
   	strcpy(SYMB_itemCrossList[SYMB_itemCrossN].description,symbolDescription);

   	SYMB_itemCrossN++;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        SYMB_Alloc
// -----------------------------------------------------------------------------
// PURPOSE         allocate and initialize buffers for the symbols
//
// RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
//                 ERROR_ID_NO in case of success
// -----------------------------------------------------------------------------

RC SYMB_Alloc(void)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Memory allocation for cross sections symbols

  if ((SYMB_itemCrossList=(SYMBOL_CROSS *)MEMORY_AllocBuffer("SYMB_Alloc","SYMB_itemCrossList",MAX_SYMBOL_CROSS,sizeof(SYMBOL_CROSS),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    memset(SYMB_itemCrossList,0,sizeof(SYMBOL_CROSS)*MAX_SYMBOL_CROSS);

    // Add already predefined symbols

    strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name,"Spectrum");
    strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name,"Ref");
    strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name,"Com");
    strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name,"Usamp1");
    strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name,"Usamp2");
    strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_RESOL].name,"Resol");

    SYMB_itemCrossN=SYMBOL_PREDEFINED_MAX;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        SYMB_Free
// -----------------------------------------------------------------------------
// PURPOSE         Release the buffers allocated for the symbols
// -----------------------------------------------------------------------------

void SYMB_Free(void)
 {
  if (SYMB_itemCrossList)
   MEMORY_ReleaseBuffer("SYMB_Free","SYMB_itemCrossList",SYMB_itemCrossList);

  SYMB_itemCrossList=NULL;
  SYMB_itemCrossN=0;
 }

