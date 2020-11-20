
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Modify options defined in the xml configuration file
//  Name of module    :  QDOASXML.C
//  Program Language  :  C/C++
//  Creation date     :  13 August 2012
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
//  Options for the analysis are retrieved from a QDOAS configuration file in
//  XML format.  Since August 2012, it is possible to modify the original
//  options using -xml switch in the doas_cl command line.  This module parses
//  the new user commands.  Syntax should be :
//
//     doas_cl .... -xml "<xml path>=<xml value>
//
//  where <xml path> : is a path defining the field to modify according to the
//                     xml file structure.  For example :
//
//    /project/analysis_window/files/refone should modify the reference file
//
//        <xml value> : is the new value to assign to the selected xml field
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  QDOASXML_Parse : module entry point to parse xml commands
//
//  ----------------------------------------------------------------------------

#include "qdoasxml.h"
#include <cstdlib>

void ProjectApplyDouble(const CProjectConfigItem *p,                            // project to modify
                        QString *pXmlKey,                                       // the path of the field to replace
                        QString *pXmlValue,                                     // string with the new value
                        double *pDoubleField)                                   // pointer to the field to change (should point within the previous target structure)
 {
     // Declarations

     char msgString[MAX_STR_LEN+1];

     // Build message with old and new values

  sprintf(msgString,"%s : %.2lf replaced by %s (%s)",
                     pXmlKey->toLocal8Bit().constData(),
                    *pDoubleField,pXmlValue->toLocal8Bit().constData(),p->name().toLocal8Bit().constData());
  std::cout << msgString << std::endl;

  // Replace the old value by the new one

  *pDoubleField=(double)atof(pXmlValue->toLocal8Bit().constData());
 }

void ProjectApplyInt(const CProjectConfigItem *p,                               // project to modify
                     QString *pXmlKey,                                          // the path of the field to replace
                     QString *pXmlValue,                                        // string with the new value
                     int *pIntField)                                            // pointer to the field to change (should point within the previous target structure)
 {
     // Declarations

     char msgString[MAX_STR_LEN+1];

     // Build message with old and new values

  sprintf(msgString,"%s : %d replaced by %s (%s)",
                     pXmlKey->toLocal8Bit().constData(),
                    *pIntField,pXmlValue->toLocal8Bit().constData(),p->name().toLocal8Bit().constData());
  std::cout << msgString << std::endl;

  // Replace the old value by the new one

  *pIntField=atoi(pXmlValue->toLocal8Bit().constData());
 }

void ProjectApplyChoice(const CProjectConfigItem *p,                            // project to modify
                        QString *pXmlKey,                                       // the path of the field to replace
                        QString *pXmlValue,                                     // string with the new value
                        const char *optionsList[],                              // list of possible options
                        int nOptions,                                           // number of options in the previous list
                        int *pNewIndexOption)                                   // index of the new option
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

    sprintf(msgString,"%s : %s replaced by %s (%s)",
                       pXmlKey->toLocal8Bit().constData(),
                       optionsList[*pNewIndexOption],pXmlValue->toLocal8Bit().constData(),p->name().toLocal8Bit().constData());

    std::cout << msgString << std::endl;

    // Replace the old value by the new one

    *pNewIndexOption=indexOption;
   }
 }

void ProjectApplyString(const CProjectConfigItem *p,                               // project to modify
                     QString *pXmlKey,                                          // the path of the field to replace
                     QString *pXmlValue,                                        // string with the new value
                     char *field)                                            // pointer to the field to change (should point within the previous target structure)
 {
     // Declarations

     char msgString[MAX_STR_LEN+1];

     // Build message with old and new values

  sprintf(msgString,"%s : %s replaced by %s (%s)",
                     pXmlKey->toLocal8Bit().constData(),field,
                     pXmlValue->toLocal8Bit().constData(),p->name().toLocal8Bit().constData());
  std::cout << msgString << std::endl;

  // Replace the old value by the new one

  strcpy(field,pXmlValue->toLocal8Bit().constData());
 }

// ===================================
// PROJECT PROPERTIES : SELECTION PAGE
// ===================================

RC ParseSelection(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlKey,QString *pXmlValue,const CProjectConfigItem *p)
 {
     // Declarations

  mediate_project_t newProjectProperties;
  const char *geoSelectionMode[]={"none","circle","rectangle","sites"};
     int indexField;
     RC  rc;

     // Initializations

     memcpy(&newProjectProperties,(mediate_project_t *)p->properties(),sizeof(mediate_project_t));
     rc=ERROR_ID_NO;

     for (indexField=startingField;indexField<xmlFieldN;indexField++)
      {
          if (xmlFields.at(indexField)=="sza")
           {
               if (indexField+1>=xmlFieldN)
                std::cout << "sza attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="min")
                ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.szaMinimum);
               else if (xmlFields.at(indexField+1)=="max")
                ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.szaMaximum);
               else if (xmlFields.at(indexField+1)=="delta")
                ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.szaDelta);
           }
    else if (xmlFields.at(indexField)=="record")
     {
               if (indexField+1>=xmlFieldN)
                std::cout << "record attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="min")
                ProjectApplyInt(p,pXmlKey,pXmlValue,&newProjectProperties.selection.recordNumberMinimum);
               else if (xmlFields.at(indexField+1)=="max")
                ProjectApplyInt(p,pXmlKey,pXmlValue,&newProjectProperties.selection.recordNumberMaximum);
     }
          else if (xmlFields.at(indexField)=="cloud")
           {
               if (indexField+1>=xmlFieldN)
                std::cout << "cloud attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="min")
                ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.cloudFractionMinimum);
               else if (xmlFields.at(indexField+1)=="max")
                ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.cloudFractionMaximum);
           }
          else if (xmlFields.at(indexField)=="geolocation")
           {
               if (indexField+1>=xmlFieldN)
                std::cout << "geolocation attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="selected")
       ProjectApplyChoice(p,pXmlKey,pXmlValue,geoSelectionMode,PRJCT_SPECTRA_MODES_MAX,&newProjectProperties.selection.geo.mode);
               else if (xmlFields.at(indexField+1)=="circle")
                {
                    if (indexField+2>=xmlFieldN)
                  std::cout << "geolocation/circle attribute is missing" << std::endl;
                 else if (xmlFields.at(indexField+2)=="radius")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.circle.radius);
                 else if (xmlFields.at(indexField+2)=="long")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.circle.centerLongitude);
                 else if (xmlFields.at(indexField+2)=="lat")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.circle.centerLatitude);
                }
               else if (xmlFields.at(indexField+1)=="rectangle")
                {
                    if (indexField+2>=xmlFieldN)
                  std::cout << "geolocation/rectangle attribute is missing" << std::endl;
                 else if (xmlFields.at(indexField+2)=="west")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.rectangle.westernLongitude);
                 else if (xmlFields.at(indexField+2)=="east")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.rectangle.easternLongitude);
                 else if (xmlFields.at(indexField+2)=="south")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.rectangle.southernLatitude);
                 else if (xmlFields.at(indexField+2)=="north")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.rectangle.northernLatitude);
                }
               else if (xmlFields.at(indexField+1)=="sites")
                {
                    if (indexField+2>=xmlFieldN)
                  std::cout << "geolocation/sites attribute is missing" << std::endl;
                 else if (xmlFields.at(indexField+2)=="radius")
                  ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.selection.geo.sites.radius);
                }
            else
       std::cout << pXmlKey->toLocal8Bit().constData() << " unknown path" << std::endl;
           }
          else
     std::cout << pXmlKey->toLocal8Bit().constData() << " unknown path" << std::endl;
   }

  p->SetProperties((mediate_project_t *)&newProjectProperties);

  // Return

  return rc;
 }

// ==================================
// PROJECT PROPERTIES : ANALYSIS PAGE
// ==================================

RC ParseAnalysis(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlKey,QString *pXmlValue,const CProjectConfigItem *p)
 {
     // Declarations

  mediate_project_t newProjectProperties;
     int indexField;
     RC  rc;

     // Initializations

     memcpy(&newProjectProperties,(mediate_project_t *)p->properties(),sizeof(mediate_project_t));
     rc=ERROR_ID_NO;

     for (indexField=startingField;indexField<xmlFieldN;indexField++)
      {
          if ((xmlFields.at(indexField)=="method") ||
              (xmlFields.at(indexField)=="fit") ||
              (xmlFields.at(indexField)=="unit") ||
              (xmlFields.at(indexField)=="interpolation") ||
              (xmlFields.at(indexField)=="gap"))

           std::cout << pXmlKey->toLocal8Bit().constData() << " can not be changed" << std::endl;

          else if (xmlFields.at(indexField)=="converge")
           ProjectApplyDouble(p,pXmlKey,pXmlValue,&newProjectProperties.analysis.convergenceCriterion);
          else if (xmlFields.at(indexField)=="max_iterations")
           ProjectApplyInt(p,pXmlKey,pXmlValue,&newProjectProperties.analysis.maxIterations);
          else
           std::cout << pXmlKey->toLocal8Bit().constData() << " unknown path" << std::endl;
      }

  p->SetProperties((mediate_project_t *)&newProjectProperties);

  // Return

  return rc;
 }

// =====================================
// PROJECT PROPERTIES : CALIBRATION PAGE
// =====================================

RC ParseCalibration(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlKey,QString *pXmlValue,const CProjectConfigItem *p)
 {
  // Declarations

  mediate_project_t newProjectProperties;
  int indexField;
  RC  rc;

  // Initializations

  rc=ERROR_ID_NO;
  memcpy(&newProjectProperties,(mediate_project_t *)p->properties(),sizeof(mediate_project_t));

  for (indexField=startingField;indexField<xmlFieldN;indexField++)
   {
    // top attributes

    if (xmlFields.at(indexField)=="line")
     {
      if (indexField+1>=xmlFieldN)
       std::cout << "line attributes are missing" << std::endl;
      else if (xmlFields.at(indexField+1)=="slfFile")
       ProjectApplyString(p,pXmlKey,pXmlValue,newProjectProperties.calibration.slfFile);
      else
       std::cout << pXmlKey->toLocal8Bit().constData() << " can not be changed " << std::endl;
     }
   }

  p->SetProperties((mediate_project_t *)&newProjectProperties);

  // Return

  return rc;
 }

// ======================================
// PROJECT PROPERTIES : INSTRUMENTAL PAGE
// ======================================

RC ParseInstrumental(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlValue,const CProjectConfigItem *p)
 {
   // Declarations

   mediate_project_t newProjectProperties;
   int indexField;
   RC  rc;

   // Initializations

   memcpy(&newProjectProperties,(mediate_project_t *)p->properties(),sizeof(mediate_project_t));
   rc=ERROR_ID_NO;

   for (indexField=startingField;indexField<xmlFieldN;indexField++)
    {
     if (xmlFields.at(indexField)=="format")
      std::cout << "project/instrumental/format field can not be changed" << std::endl;
     else if (xmlFields.at(indexField)=="site")
      std::cout << "project/instrumental/site field can not be changed" << std::endl;
     else if (xmlFields.at(indexField)=="ascii")
      std::cout << "project/instrumental/ascii field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="logger")
      std::cout << "project/instrumental/logger field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="acton")
      std::cout << "project/instrumental/acton field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="pdaegg")
      std::cout << "project/instrumental/pdaegg field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="pdaeggold")
      std::cout << "project/instrumental/pdaeggold field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="ccdohp96")
      std::cout << "project/instrumental/ccdohp96 field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="ccdha94")
      std::cout << "project/instrumental/ccdha94 field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="saozvis")
      std::cout << "project/instrumental/saozvis field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="saozefm")
      std::cout << "project/instrumental/saozefm field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="mfc")
      std::cout << "project/instrumental/mfc field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="mfcbira")
      std::cout << "project/instrumental/mfcbira field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="mfcstd")
      std::cout << "project/instrumental/mfcstd field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="rasas")
      std::cout << "project/instrumental/rasas field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="pdasieasoe")
      std::cout << "project/instrumental/pdasieasoe field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="ccdeev")
      std::cout << "project/instrumental/ccdeev field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="gdpascii")
      std::cout << "project/instrumental/gdpascii field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="gdpbin")
      std::cout << "project/instrumental/gdpbin field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="sciapds")
      std::cout << "project/instrumental/sciapds field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="uoft")
      std::cout << "project/instrumental/uoft field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="noaa")
      std::cout << "project/instrumental/noaa field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="omi")
      {
       if (indexField+1>=xmlFieldN)
        std::cout << "omi attribute is missing" << std::endl;
       else if (xmlFields.at(indexField+1)=="type")
        std::cout << "project/instrumental/omi/type field can not be changed" << std::endl;
       else if (xmlFields.at(indexField+1)=="min")
        std::cout << "project/instrumental/omi/min field can not be changed" << std::endl;
       else if (xmlFields.at(indexField+1)=="max")
        std::cout << "project/instrumental/omi/max field can not be changed" << std::endl;
       else if (xmlFields.at(indexField+1)=="ave")
        std::cout << "project/instrumental/omi/ave field can not be changed" << std::endl;
       else if (xmlFields.at(indexField+1)=="trackSelection")
        {
         std::cout << "project/instrumental/omi/trackSelection : " << newProjectProperties.instrumental.omi.trackSelection << " replaced by " << pXmlValue->toLocal8Bit().constData() << std::endl;
         strcpy(newProjectProperties.instrumental.omi.trackSelection,pXmlValue->toLocal8Bit().constData());
        }
       else if (xmlFields.at(indexField+1)=="xTrackMode")
        {
         std::cout << "project/instrumental/omi/xTrackMode : " << pXmlValue->toLocal8Bit().constData() << std::endl;
         newProjectProperties.instrumental.omi.xtrack_mode = str_to_mode(pXmlValue->toLocal8Bit().constData());
        }
       else if (xmlFields.at(indexField+1)=="calib")
        std::cout << "project/instrumental/omi/calib field can not be changed" << std::endl;
       else if (xmlFields.at(indexField+1)=="instr")
        std::cout << "project/instrumental/omi/instr field can not be changed yet" << std::endl;
      }
     else if (xmlFields.at(indexField)=="tropomi")
      {
       if (indexField+1>=xmlFieldN)
        std::cout << "tropomi attribute is missing" << std::endl;
       else if (xmlFields.at(indexField+1)=="trackSelection")
        {
         std::cout << "project/instrumental/tropomi/trackSelection : " << newProjectProperties.instrumental.tropomi.trackSelection << " replaced by " << pXmlValue->toLocal8Bit().constData() << std::endl;
         strcpy(newProjectProperties.instrumental.tropomi.trackSelection,pXmlValue->toLocal8Bit().constData());
        }
      }
     else if (xmlFields.at(indexField)=="apex")
      {
       if (indexField+1>=xmlFieldN)
        std::cout << "apex attribute is missing" << std::endl;
       else if (xmlFields.at(indexField+1)=="trackSelection")
        {
         std::cout << "project/instrumental/apex/trackSelection : " << newProjectProperties.instrumental.apex.trackSelection << " replaced by " << pXmlValue->toLocal8Bit().constData() << std::endl;
         strcpy(newProjectProperties.instrumental.apex.trackSelection,pXmlValue->toLocal8Bit().constData());
        }
      }
     else if (xmlFields.at(indexField)=="gome2")
      std::cout << "project/instrumental/gome2 field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="mkzy")
      std::cout << "project/instrumental/mkzy field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="biramobile")
      std::cout << "project/instrumental/biramobile field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="biraairborne")
      std::cout << "project/instrumental/biraairborne field can not be changed yet" << std::endl;
     else if (xmlFields.at(indexField)=="oceanoptics")
      std::cout << "project/instrumental/oceanoptics field can not be changed yet" << std::endl;
    }


   p->SetProperties(( mediate_project_t *)&newProjectProperties);

   // Return

   return rc;
 }

// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

void AnalysisWindowApplyInt(const QList<const CAnalysisWindowConfigItem*>awList,        // list of analysis windows
                               QString *pWindowName,                                    // the name of a specific analysis window (all windows will be modified if this string is empty)
                               QString *pXmlKey,                                        // the path of the field to replace
                               int newValue,                                            // new value
                               mediate_analysis_window_t *pNewAnalysisWindow,           // target structure for analysis windows properties
                               int *pIntField)                                          // pointer to the field to change (should point within the previous target structure)
{
  // Declarations

  QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();
  char msgString[MAX_STR_LEN+1];

  // Browse

  while (awIt!=awList.end()) {
    if (pWindowName->isEmpty() || (*pWindowName==(*awIt)->name())) {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %d replaced by %d (%s)",
              pXmlKey->toLocal8Bit().constData(),
              *pIntField,newValue,(*awIt)->name().toLocal8Bit().constData());
      std::cout << msgString << std::endl;
      *pIntField=newValue;
      (*awIt)->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
    }
    ++awIt;
  }
}

void AnalysisWindowApplyDouble(const QList<const CAnalysisWindowConfigItem*>awList,     // list of analysis windows
                               QString *pWindowName,                                    // the name of a specific analysis window (all windows will be modified if this string is empty)
                               QString *pXmlKey,                                        // the path of the field to replace
                               QString *pXmlValue,                                      // string with the new value
                               mediate_analysis_window_t *pNewAnalysisWindow,           // target structure for analysis windows properties
                               double *pDoubleField)                                    // pointer to the field to change (should point within the previous target structure)
{
  // Declarations

  QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();
  char msgString[MAX_STR_LEN+1];

  // Browse

  while (awIt!=awList.end()) {
    if (pWindowName->isEmpty() || (*pWindowName==(*awIt)->name())) {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %.2lf replaced by %s (%s)",
              pXmlKey->toLocal8Bit().constData(),
              *pDoubleField,pXmlValue->toLocal8Bit().constData(),(*awIt)->name().toLocal8Bit().constData());
      std::cout << msgString << std::endl;
      *pDoubleField=(double)atof(pXmlValue->toLocal8Bit().constData());
      (*awIt)->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
    }
    ++awIt;
  }
}

void AnalysisWindowApplyString(const QList<const CAnalysisWindowConfigItem*>awList,     // list of analysis windows
                               QString *pWindowName,                                    // the name of a specific analysis window (all windows will be modified if this string is empty)
                               QString *pXmlKey,                                        // the path of the field to replace
                               QString *pXmlValue,                                      // string with the new value
                               mediate_analysis_window_t *pNewAnalysisWindow,           // target structure for analysis windows properties
                               char *field)                                    // pointer to the field to change (should point within the previous target structure)
 {
     // Declarations

     QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();
     char msgString[MAX_STR_LEN+1];

     // Browse

  while (awIt!=awList.end())
   {
       if (pWindowName->isEmpty() || (*pWindowName==(*awIt)->name()))
        {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %s replaced by %s (%s)",
              pXmlKey->toLocal8Bit().constData(),
              field,pXmlValue->toLocal8Bit().constData(),(*awIt)->name().toLocal8Bit().constData());
      std::cout << msgString << std::endl;
      strcpy(field,pXmlValue->toLocal8Bit().constData());
      (*awIt)->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
     }
    ++awIt;
   }
 }

RC ParseAnalysisWindow(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlKey,QString *pXmlValue,const CProjectConfigItem *p)
{
  // Declarations

  const QList<const CAnalysisWindowConfigItem*> awList = p->analysisWindowItems();
  mediate_analysis_window_t newAnalysisProperties;
  // QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();

  QString windowName=QString();

  int indexField;
  int displayField;
  int filesField;
  RC  rc;

  // Initializations

  displayField=filesField=0;
  rc=ERROR_ID_NO;

  for (indexField=startingField;indexField<xmlFieldN;indexField++)
    {
      // top attributes

      if (!displayField && !filesField)
        {
          if ((xmlFields.at(indexField)=="disable") ||
              (xmlFields.at(indexField)=="kurucz"))

            std::cout << pXmlKey->toLocal8Bit().constData() << " can not be changed" << std::endl;

          // Fitting interval

          else if (xmlFields.at(indexField)=="min")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.fitMinWavelength);
          else if (xmlFields.at(indexField)=="max")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.fitMaxWavelength);
          else if (xmlFields.at(indexField)=="resol_fwhm")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.resolFwhm);
          else if (xmlFields.at(indexField)=="lambda0")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.lambda0);
          else if (xmlFields.at(indexField)=="display")
            std::cout << "project/analysis_window/display fields can not be changed" << std::endl;
          else if (xmlFields.at(indexField)=="refsel")
            {
              if ((*pXmlValue!="auto") && (*pXmlValue!="file"))
                std::cout << pXmlKey->toLocal8Bit().constData() << " do not support " << pXmlValue->toLocal8Bit().constData() << std::endl;
              else
                AnalysisWindowApplyInt(awList,&windowName,pXmlKey,(*pXmlValue=="auto")?ANLYS_REF_SELECTION_MODE_AUTOMATIC:ANLYS_REF_SELECTION_MODE_FILE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
            }
          else if (xmlFields.at(indexField)=="files")
            filesField=1;
          else
            windowName=xmlFields.at(indexField);
        }

      // files section

      else if (filesField)
        {
          if (xmlFields.at(indexField)=="refone")
            AnalysisWindowApplyString(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,newAnalysisProperties.refOneFile);
          else if (xmlFields.at(indexField)=="reftwo")
            AnalysisWindowApplyString(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,newAnalysisProperties.refTwoFile);
          else if (xmlFields.at(indexField)=="residual")
            AnalysisWindowApplyString(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,newAnalysisProperties.residualFile);
          else if (xmlFields.at(indexField)=="szacenter")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.refSzaCenter);
          else if (xmlFields.at(indexField)=="szadelta")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.refSzaDelta);
          else if (xmlFields.at(indexField)=="minlon")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.refMinLongitude);
          else if (xmlFields.at(indexField)=="maxlon")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.refMaxLongitude);
          else if (xmlFields.at(indexField)=="minlat")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.refMinLatitude);
          else if (xmlFields.at(indexField)=="maxlat")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.refMaxLatitude);
          else if (xmlFields.at(indexField)=="refns")
            AnalysisWindowApplyInt(awList,&windowName,pXmlKey,atoi(pXmlValue->toLocal8Bit().constData()),&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
          else if (xmlFields.at(indexField)=="cloudfmin")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.cloudFractionMin);
          else if (xmlFields.at(indexField)=="cloudfmax")
            AnalysisWindowApplyDouble(awList,&windowName,pXmlKey,pXmlValue,&newAnalysisProperties,&newAnalysisProperties.cloudFractionMax);
          else if (xmlFields.at(indexField)=="maxdoasrefmode")
            {
              if ((*pXmlValue!="scan") && (*pXmlValue!="sza"))
                std::cout << pXmlKey->toLocal8Bit().constData() << " do not support " << pXmlValue->toLocal8Bit().constData() << std::endl;
              else
                AnalysisWindowApplyInt(awList,&windowName,pXmlKey,(*pXmlValue=="scan")?ANLYS_MAXDOAS_REF_SCAN:ANLYS_MAXDOAS_REF_SZA,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
            }
          else if (xmlFields.at(indexField)=="scanmode")
            {
                 if (*pXmlValue=="before")
                  AnalysisWindowApplyInt(awList,&windowName,pXmlKey,ANLYS_MAXDOAS_REF_SCAN_BEFORE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else if (*pXmlValue=="after")
                  AnalysisWindowApplyInt(awList,&windowName,pXmlKey,ANLYS_MAXDOAS_REF_SCAN_AFTER,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else if (*pXmlValue=="average")
                  AnalysisWindowApplyInt(awList,&windowName,pXmlKey,ANLYS_MAXDOAS_REF_SCAN_AVERAGE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else if (*pXmlValue=="interpolate")
                  AnalysisWindowApplyInt(awList,&windowName,pXmlKey,ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else
               std::cout << pXmlKey->toLocal8Bit().constData() << " do not support " << pXmlValue->toLocal8Bit().constData() << std::endl;
            }

          // pixel selection for GOME-ERS2 -> not supported

            else if ((xmlFields.at(indexField)=="east") ||
                     (xmlFields.at(indexField)=="center") ||
                     (xmlFields.at(indexField)=="west") ||
                     (xmlFields.at(indexField)=="backscan"))

                    std::cout << pXmlKey->toLocal8Bit().constData() << " can not be changed" << std::endl;
        }
    }


  // Return

  return rc;
 }

RC QDOASXML_Parse(QList<QString> &xmlCommands,const CProjectConfigItem *p)
 {
  QList<QString>::const_iterator it = xmlCommands.begin();

  QString newXmlCmd;
  QStringList xmlParts;
  QStringList xmlFields;
  QString xmlKey,xmlValue;

  int xmlFieldsN,indexField,projectField,analysisField;
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

         projectField=analysisField=0;

         for (indexField=0;(indexField<xmlFieldsN) && !rc;indexField++)
          {
        if (projectField)
         {
             if (xmlFields.at(indexField)=="display")
               std::cout << xmlKey.toLocal8Bit().constData() << " fields can not be changed" << std::endl;
             else if (xmlFields.at(indexField)=="selection")
              rc=ParseSelection(xmlFields,xmlFieldsN,indexField+1,&xmlKey,&xmlValue,p);
             else if (xmlFields.at(indexField)=="analysis")
              rc=ParseAnalysis(xmlFields,xmlFieldsN,indexField+1,&xmlKey,&xmlValue,p);
             else if (xmlFields.at(indexField)=="instrumental")
              rc=ParseInstrumental(xmlFields,xmlFieldsN,indexField+1,&xmlValue,p);
             else if (xmlFields.at(indexField)=="analysis_window")
                 rc=ParseAnalysisWindow(xmlFields,xmlFieldsN,indexField+1,&xmlKey,&xmlValue,p);
             else if (xmlFields.at(indexField)=="calibration")
                 rc=ParseCalibration(xmlFields,xmlFieldsN,indexField+1,&xmlKey,&xmlValue,p);

                break;
         }
              else if (xmlFields.at(indexField)=="project")
               projectField=1;
          }
     }


    ++it;
   }

  return rc;
 }

