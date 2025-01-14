
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Modify options defined in the xml configuration file
//  Name of module    :  CONVXML.C
//  Program Language  :  C/C++
//  Creation date     :  3 September 2012
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
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
//      thomas.danckaert@aeronomie.be
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  Options for the convolution tool are retrieved from a configuration file in
//  XML format.  Since September 2012, it is possible to modify the original
//  options using -xml switch in the doas_cl command line.  This module parses
//  the new user commands.  Syntax should be :
//
//     doas_cl .... -xml "<xml path>=<xml value>
//
//  where <xml path> : is a path defining the field to modify according to the
//                     xml file structure.  For example :
//
//  /con_slit/slit_func/file/file should modify the name of the slit function file
//
//        <xml value> : is the new value to assign to the selected xml field
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  CONVXML_Parse : module entry point to parse xml commands
//
//  ----------------------------------------------------------------------------

#include <iostream>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "convxml.h"

#include "constants.h"

using std::string;
using std::vector;

const char *convFalseTrue[]={"false","true"};

void ConvApplyString(const string& xmlKey,                                      // the path of the field to replace
                     const string& xmlValue,                                    // string with the new value
                     char *newstring)                                           // pointer to the field to change
{
  std::cout << xmlKey << ": " << newstring << " replaced by " << xmlValue << std::endl;
  // Replace the old value by the new one
  strcpy(newstring,xmlValue.c_str());
}

void ConvApplyChoice(const string& xmlKey,                                          // the path of the field to replace
                     const string& xmlValue,                                        // string with the new value
                     const char *optionsList[],                                 // list of possible options
                     int nOptions,                                              // number of options in the previous list
                     int *pNewIndexOption)                                      // index of the new option
{
  int indexOption = 0;
  for (indexOption=0; indexOption<nOptions; indexOption++)
    if (xmlValue == optionsList[indexOption])
      break;

  if (indexOption<nOptions) {
    // Print message with old and new values
    std::cout << xmlKey << ": " << optionsList[*pNewIndexOption]
              << " replaced by " << xmlValue << std::endl;
    *pNewIndexOption=indexOption;
   }
}

// =========
// SLIT PAGE
// =========

RC ParseSlit(const vector<string>& xmlFields, size_t xmlFieldN, size_t startingField, const string& xmlKey, const string& xmlValue, mediate_slit_function_t *pSlit) {
  int slitFuncFlag=0;
  RC rc=ERROR_ID_NO;

  for (size_t indexField=startingField; indexField<xmlFieldN; ++indexField) {
    if (slitFuncFlag) {
      if (xmlFields.at(indexField)=="file") {
        if (indexField+1>=xmlFieldN)
          std::cout << xmlFields.at(indexField) << " attribute is missing" << std::endl;
        else if (xmlFields.at(indexField+1)=="file")
          ConvApplyString(xmlKey,xmlValue,pSlit->file.filename);
        else if (xmlFields.at(indexField+1)=="file2")
          ConvApplyString(xmlKey,xmlValue,pSlit->file.filename2);
        else if (xmlFields.at(indexField+1)=="wveDptFlag")
          ConvApplyChoice(xmlKey,xmlValue,convFalseTrue,2,&pSlit->file.wveDptFlag);
        break;
      } else {
        std::cout << xmlFields.at(indexField) << " fields can not be changed yet" << std::endl;
      }
    } else if (xmlFields.at(indexField)=="slit_func") {
      slitFuncFlag=1;
    } else {
      std::cout << xmlKey << " unknown path" << std::endl;
    }
  }

  // Return

  return rc;
 }

RC CONVXML_Parse(const vector<string> &xmlCommands,mediate_convolution_t *properties) {
  RC rc=ERROR_ID_NO;

  for (const auto& xml_cmd : xmlCommands) {
    vector<string> xmlParts;
    boost::split(xmlParts, xml_cmd, boost::is_any_of("="));
    if (xmlParts.size()==2) {
      const auto xmlKey=xmlParts.at(0);
      const auto xmlValue=xmlParts.at(1);

      vector<string> xmlFields;
      boost::split(xmlFields, xmlKey, boost::is_any_of("/"));
      size_t xmlFieldsN=xmlFields.size();

      for (size_t indexField=0; (indexField<xmlFieldsN) && !rc; ++indexField) {
        if (xmlFields.at(indexField)=="general") {
          if (xmlFields.at(indexField+1)=="calib") {
            ConvApplyString(xmlKey, xmlValue, properties->general.calibrationFile);        // the path of the field to replace
            break;
          }
        } else if (xmlFields.at(indexField)=="con_slit") {
          rc=ParseSlit(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,&properties->conslit);
          break;
        } else if (xmlFields.at(indexField)=="dec_slit") {
          std::cout << xmlKey << " fields can not be changed yet" << std::endl;
        } else if (xmlFields.at(indexField)=="lowpass_filter") {
          std::cout << xmlKey << " fields can not be changed yet" << std::endl;
        } else if (xmlFields.at(indexField)=="highpass_filter") {
          std::cout << xmlKey << " fields can not be changed yet" << std::endl;
        }
      }
    }
    if (rc) {
      break;
    }
  }
  return rc;
}

RC RINGXML_Parse(const vector<string> &xmlCommands,mediate_ring *properties)
 {
  RC rc = ERROR_ID_NO;

  for (const auto& xml_cmd : xmlCommands) {
    vector<string> xmlParts;
    boost::split(xmlParts, xml_cmd, boost::is_any_of("="));
    if (xmlParts.size()==2) {
      const auto xmlKey=xmlParts.at(0);
      const auto xmlValue=xmlParts.at(1);

      vector<string> xmlFields;
      boost::split(xmlFields, xmlKey, boost::is_any_of("/"));
      size_t xmlFieldsN=xmlFields.size();

      for (size_t indexField=0;(indexField<xmlFieldsN) && !rc;indexField++) {
        if (xmlFields.at(indexField)=="general") {
          if (xmlFields.at(indexField+1)=="calib")
            ConvApplyString(xmlKey,xmlValue,properties->calibrationFile);        // the path of the field to replace
          else
            rc=ParseSlit(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,&properties->slit);
          break;
        }
      }
    }
    if (rc) {
      break;
    }
   }

  return rc;
 }

