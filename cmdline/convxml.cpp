
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

#include "convxml.h"

const char *convFalseTrue[]={"false","true"};

void ConvApplyString(QString *pXmlKey,                                          // the path of the field to replace
                     QString *pXmlValue,                                        // string with the new value
                     char *newstring)                                           // pointer to the field to change
 {
 	// Declarations

 	char msgString[MAX_STR_LEN+1];

 	// Build message with old and new values

  sprintf(msgString,"%s : %s replaced by %s",pXmlKey->toLocal8Bit().constData(),newstring,pXmlValue->toLocal8Bit().constData());
  std::cout << msgString << std::endl;

  // Replace the old value by the new one

  strcpy(newstring,pXmlValue->toLocal8Bit().constData());
 }

void ConvApplyChoice(QString *pXmlKey,                                          // the path of the field to replace
                     QString *pXmlValue,                                        // string with the new value
                     const char *optionsList[],                                 // list of possible options
                     int nOptions,                                              // number of options in the previous list
                     int *pNewIndexOption)                                      // index of the new option
 {
 	// Declarations

 	char msgString[MAX_STR_LEN+1];
 	INDEX indexOption;

  for (indexOption=0;indexOption<nOptions;indexOption++)
  	if (*pXmlValue==optionsList[indexOption])
  	 break;

  if (indexOption<nOptions)
   {
 	  // Build message with old and new values

    sprintf(msgString,"%s : %s replaced by %s",
                       pXmlKey->toLocal8Bit().constData(),
                       optionsList[*pNewIndexOption],pXmlValue->toLocal8Bit().constData());

    std::cout << msgString << std::endl;

    // Replace the old value by the new one

    *pNewIndexOption=indexOption;
   }
 }

// =========
// SLIT PAGE
// =========

RC ParseSlit(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlKey,QString *pXmlValue,mediate_slit_function_t *pSlit)
 {
 	// Declarations

  int slitFuncFlag;
 	int indexField;
 	RC  rc;

 	// Initializations

 	slitFuncFlag=0;
 	rc=ERROR_ID_NO;

 	for (indexField=startingField;indexField<xmlFieldN;indexField++)
 	 {
    if (slitFuncFlag)
     {
     	if (xmlFields.at(indexField)=="file")
     	 {
     	 	if (indexField+1>=xmlFieldN)
     	 	 std::cout << xmlFields.at(indexField).toLocal8Bit().constData() << " attribute is missing" << std::endl;
     	 	else if (xmlFields.at(indexField+1)=="file")
     	 	 ConvApplyString(pXmlKey,pXmlValue,pSlit->file.filename);
     	 	else if (xmlFields.at(indexField+1)=="file2")
     	 	 ConvApplyString(pXmlKey,pXmlValue,pSlit->file.filename2);
     	 	else if (xmlFields.at(indexField+1)=="wveDptFlag")
         ConvApplyChoice(pXmlKey,pXmlValue,convFalseTrue,2,&pSlit->file.wveDptFlag);

        break;
     	 }
     	else
     	 std::cout << xmlFields.at(indexField).toLocal8Bit().constData() << " fields can not be changed yet" << std::endl;
 	 	 }
 	 	else if (xmlFields.at(indexField)=="slit_func")
 	 	 slitFuncFlag=1;
 	 	else
     std::cout << pXmlKey->toLocal8Bit().constData() << " unknown path" << std::endl;
   }

  // Return

  return rc;
 }

RC CONVXML_Parse(QList<QString> &xmlCommands,mediate_convolution_t *properties)
 {
  QList<QString>::const_iterator it = xmlCommands.begin();

  QString newXmlCmd;
  QStringList xmlParts;
  QStringList xmlFields;
  QString xmlKey,xmlValue;

  int xmlFieldsN,indexField;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  while ((it!=xmlCommands.end()) && !rc)
   {
    newXmlCmd=*it;
    xmlParts=newXmlCmd.split("=");
    if (xmlParts.size()==2)
     {
     	xmlKey=xmlParts.at(0);
     	xmlValue=xmlParts.at(1);

     	xmlFields=xmlKey.split("/");
     	xmlFieldsN=xmlFields.size();

     	for (indexField=0;(indexField<xmlFieldsN) && !rc;indexField++)
     	 {
        if (xmlFields.at(indexField)=="general")
         {
         	if (xmlFields.at(indexField+1)=="calib")
         	 {
            ConvApplyString(&xmlKey,&xmlValue,properties->general.calibrationFile);        // the path of the field to replace
            break;
           }
         }
        else if (xmlFields.at(indexField)=="con_slit")
         {
          rc=ParseSlit(xmlFields,xmlFieldsN,indexField+1,&xmlKey,&xmlValue,&properties->conslit);
          break;
         }
        else if (xmlFields.at(indexField)=="dec_slit")
         std::cout << xmlKey.toLocal8Bit().constData() << " fields can not be changed yet" << std::endl;
        else if (xmlFields.at(indexField)=="lowpass_filter")
         std::cout << xmlKey.toLocal8Bit().constData() << " fields can not be changed yet" << std::endl;
        else if (xmlFields.at(indexField)=="highpass_filter")
     	 	 std::cout << xmlKey.toLocal8Bit().constData() << " fields can not be changed yet" << std::endl;
     	 }
     }


    ++it;
   }

  return rc;
 }

RC RINGXML_Parse(QList<QString> &xmlCommands,mediate_ring *properties)
 {
  QList<QString>::const_iterator it = xmlCommands.begin();

  QString newXmlCmd;
  QStringList xmlParts;
  QStringList xmlFields;
  QString xmlKey,xmlValue;

  int xmlFieldsN,indexField;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  while ((it!=xmlCommands.end()) && !rc)
   {
    newXmlCmd=*it;
    xmlParts=newXmlCmd.split("=");
    if (xmlParts.size()==2)
     {
     	xmlKey=xmlParts.at(0);
     	xmlValue=xmlParts.at(1);

     	xmlFields=xmlKey.split("/");
     	xmlFieldsN=xmlFields.size();

     	for (indexField=0;(indexField<xmlFieldsN) && !rc;indexField++)
     	 {
        if (xmlFields.at(indexField)=="general")
         {
          if (xmlFields.at(indexField+1)=="calib")
           ConvApplyString(&xmlKey,&xmlValue,properties->calibrationFile);        // the path of the field to replace
          else
           rc=ParseSlit(xmlFields,xmlFieldsN,indexField+1,&xmlKey,&xmlValue,&properties->slit);

          break;
         }
     	 }
     }


    ++it;
   }

  return rc;
 }

