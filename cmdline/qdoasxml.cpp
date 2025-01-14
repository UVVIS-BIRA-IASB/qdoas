
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

#include <boost/algorithm/string.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include "qdoasxml.h"

using std::string;
using std::vector;

void ProjectApplyDouble(const CProjectConfigItem *p,                            // project to modify
                        const string& xmlKey,                                   // the path of the field to replace
                        const string& xmlValue,                                 // string with the new value
                        double *pDoubleField)                                   // pointer to the field to change (should point within the previous target structure)
 {
   // Print message with old and new values
  std::cout << xmlKey << ": "
            << *pDoubleField << " replaced by " << xmlValue
            << " (" << p->name() << ")" << std::endl;

  // Replace the old value by the new one
  *pDoubleField = std::stod(xmlValue);
 }

void ProjectApplyInt(const CProjectConfigItem *p,                               // project to modify
                     const string& xmlKey,                                      // the path of the field to replace
                     const string& xmlValue,                                    // string with the new value
                     int *pIntField)                                            // pointer to the field to change (should point within the previous target structure)
 {
  std::cout << xmlKey << ": "
            << *pIntField << " replaced by " << xmlValue
            << " (" << p->name() << ")" << std::endl;
  // Replace the old value by the new one
  *pIntField=stoi(xmlValue);
 }

void ProjectApplyChoice(const CProjectConfigItem *p,                            // project to modify
                        const string& xmlKey,                                   // the path of the field to replace
                        const string& xmlValue,                                 // string with the new value
                        const char *optionsList[],                              // list of possible options
                        int nOptions,                                           // number of options in the previous list
                        int *pNewIndexOption)                                   // index of the new option
{
  int indexOption;

  for (indexOption=0;indexOption<nOptions;indexOption++)
      if (xmlValue==optionsList[indexOption])
       break;

  if (indexOption<nOptions) {
    std::cout << xmlKey << ": " << optionsList[*pNewIndexOption]
              << " replaced by " << xmlValue << " (" << p->name() << ")" << std::endl;

    // Replace the old value by the new one
    *pNewIndexOption=indexOption;
  }
}

void ProjectApplyString(const CProjectConfigItem *p,                               // project to modify
                     const string& xmlKey,                                          // the path of the field to replace
                     const string& xmlValue,                                        // string with the new value
                     char *field)                                            // pointer to the field to change (should point within the previous target structure)
{
  std::cout << xmlKey << ": "
            << field << " replaced by " << xmlValue
            << " ("  << p->name() << ")" << std::endl;
  // Replace the old value by the new one
  strcpy(field,xmlValue.c_str());
}

// ===================================
// PROJECT PROPERTIES : SELECTION PAGE
// ===================================

RC ParseSelection(const vector<string>& xmlFields,int xmlFieldN,int startingField,const string& xmlKey,const string& xmlValue,const CProjectConfigItem *p)
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
                ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.szaMinimum);
               else if (xmlFields.at(indexField+1)=="max")
                ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.szaMaximum);
               else if (xmlFields.at(indexField+1)=="delta")
                ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.szaDelta);
           }
    else if (xmlFields.at(indexField)=="record")
     {
               if (indexField+1>=xmlFieldN)
                std::cout << "record attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="min")
                ProjectApplyInt(p,xmlKey,xmlValue,&newProjectProperties.selection.recordNumberMinimum);
               else if (xmlFields.at(indexField+1)=="max")
                ProjectApplyInt(p,xmlKey,xmlValue,&newProjectProperties.selection.recordNumberMaximum);
     }
          else if (xmlFields.at(indexField)=="cloud")
           {
               if (indexField+1>=xmlFieldN)
                std::cout << "cloud attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="min")
                ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.cloudFractionMinimum);
               else if (xmlFields.at(indexField+1)=="max")
                ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.cloudFractionMaximum);
           }
          else if (xmlFields.at(indexField)=="geolocation")
           {
               if (indexField+1>=xmlFieldN)
                std::cout << "geolocation attribute is missing" << std::endl;
               else if (xmlFields.at(indexField+1)=="selected")
       ProjectApplyChoice(p,xmlKey,xmlValue,geoSelectionMode,PRJCT_SPECTRA_MODES_MAX,&newProjectProperties.selection.geo.mode);
               else if (xmlFields.at(indexField+1)=="circle")
                {
                    if (indexField+2>=xmlFieldN)
                  std::cout << "geolocation/circle attribute is missing" << std::endl;
                 else if (xmlFields.at(indexField+2)=="radius")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.circle.radius);
                 else if (xmlFields.at(indexField+2)=="long")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.circle.centerLongitude);
                 else if (xmlFields.at(indexField+2)=="lat")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.circle.centerLatitude);
                }
               else if (xmlFields.at(indexField+1)=="rectangle")
                {
                    if (indexField+2>=xmlFieldN)
                  std::cout << "geolocation/rectangle attribute is missing" << std::endl;
                 else if (xmlFields.at(indexField+2)=="west")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.rectangle.westernLongitude);
                 else if (xmlFields.at(indexField+2)=="east")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.rectangle.easternLongitude);
                 else if (xmlFields.at(indexField+2)=="south")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.rectangle.southernLatitude);
                 else if (xmlFields.at(indexField+2)=="north")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.rectangle.northernLatitude);
                }
               else if (xmlFields.at(indexField+1)=="sites")
                {
                    if (indexField+2>=xmlFieldN)
                  std::cout << "geolocation/sites attribute is missing" << std::endl;
                 else if (xmlFields.at(indexField+2)=="radius")
                  ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.selection.geo.sites.radius);
                }
            else
       std::cout << xmlKey << " unknown path" << std::endl;
           }
          else
     std::cout << xmlKey << " unknown path" << std::endl;
   }

  p->SetProperties((mediate_project_t *)&newProjectProperties);

  // Return

  return rc;
 }

// ==================================
// PROJECT PROPERTIES : ANALYSIS PAGE
// ==================================

RC ParseAnalysis(const vector<string>& xmlFields,int xmlFieldN,int startingField,const string& xmlKey,const string& xmlValue,const CProjectConfigItem *p)
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

           std::cout << xmlKey << " can not be changed" << std::endl;

          else if (xmlFields.at(indexField)=="converge")
           ProjectApplyDouble(p,xmlKey,xmlValue,&newProjectProperties.analysis.convergenceCriterion);
          else if (xmlFields.at(indexField)=="max_iterations")
           ProjectApplyInt(p,xmlKey,xmlValue,&newProjectProperties.analysis.maxIterations);
          else
           std::cout << xmlKey << " unknown path" << std::endl;
      }

  p->SetProperties((mediate_project_t *)&newProjectProperties);

  // Return

  return rc;
 }

// =====================================
// PROJECT PROPERTIES : CALIBRATION PAGE
// =====================================

RC ParseCalibration(const vector<string>& xmlFields,int xmlFieldN,int startingField,const string& xmlKey,const string& xmlValue,const CProjectConfigItem *p)
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
       ProjectApplyString(p,xmlKey,xmlValue,newProjectProperties.calibration.slfFile);
      else
       std::cout << xmlKey << " can not be changed " << std::endl;
     }
   }

  p->SetProperties((mediate_project_t *)&newProjectProperties);

  // Return

  return rc;
 }

// ======================================
// PROJECT PROPERTIES : INSTRUMENTAL PAGE
// ======================================

RC ParseInstrumental(const vector<string>& xmlFields,int xmlFieldN,int startingField,const string& xmlValue,const CProjectConfigItem *p)
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
         std::cout << "project/instrumental/omi/trackSelection : " << newProjectProperties.instrumental.omi.trackSelection << " replaced by " << xmlValue << std::endl;
         strcpy(newProjectProperties.instrumental.omi.trackSelection,xmlValue.c_str());
        }
       else if (xmlFields.at(indexField+1)=="xTrackMode")
        {
         std::cout << "project/instrumental/omi/xTrackMode : " << xmlValue << std::endl;
         newProjectProperties.instrumental.omi.xtrack_mode = str_to_mode(xmlValue.c_str());
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
         std::cout << "project/instrumental/tropomi/trackSelection : " << newProjectProperties.instrumental.tropomi.trackSelection << " replaced by " << xmlValue << std::endl;
         strcpy(newProjectProperties.instrumental.tropomi.trackSelection,xmlValue.c_str());
        }
      }
     else if (xmlFields.at(indexField)=="apex")
      {
       if (indexField+1>=xmlFieldN)
        std::cout << "apex attribute is missing" << std::endl;
       else if (xmlFields.at(indexField+1)=="trackSelection")
        {
         std::cout << "project/instrumental/apex/trackSelection : " << newProjectProperties.instrumental.apex.trackSelection << " replaced by " << xmlValue << std::endl;
         strcpy(newProjectProperties.instrumental.apex.trackSelection,xmlValue.c_str());
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

void AnalysisWindowApplyInt(const vector<const CAnalysisWindowConfigItem*>& awList,   // list of analysis windows
                            const string& windowName,                                // the name of a specific analysis window (all windows will be modified if this string is empty)
                            const string& xmlKey,                                    // the path of the field to replace
                            int newValue,                                            // new value
                            mediate_analysis_window_t *pNewAnalysisWindow,           // target structure for analysis windows properties
                            int *pIntField)                                          // pointer to the field to change (should point within the previous target structure)
{
  // Declarations
  auto awIt = awList.begin();
  char msgString[MAX_STR_LEN+1];

  // Browse

  while (awIt!=awList.end()) {
    if (windowName.empty() || (windowName==(*awIt)->name())) {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %d replaced by %d (%s)",
              xmlKey.c_str(),
              *pIntField,newValue,(*awIt)->name().c_str());
      std::cout << msgString << std::endl;
      *pIntField=newValue;
      (*awIt)->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
    }
    ++awIt;
  }
}

void AnalysisWindowApplyDouble(const vector<const CAnalysisWindowConfigItem*>awList,     // list of analysis windows
                               const string& windowName,                                // the name of a specific analysis window (all windows will be modified if this string is empty)
                               const string& xmlKey,                                    // the path of the field to replace
                               const string& xmlValue,                                  // string with the new value
                               mediate_analysis_window_t *pNewAnalysisWindow,           // target structure for analysis windows properties
                               double *pDoubleField)                                    // pointer to the field to change (should point within the previous target structure)
{
  // Declarations

  auto awIt = awList.begin();
  char msgString[MAX_STR_LEN+1];

  // Browse

  while (awIt!=awList.end()) {
    if (windowName.empty() || (windowName==(*awIt)->name())) {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %.2lf replaced by %s (%s)",
              xmlKey.c_str(),
              *pDoubleField,xmlValue.c_str(),(*awIt)->name().c_str());
      std::cout << msgString << std::endl;
      *pDoubleField=(double)atof(xmlValue.c_str());
      (*awIt)->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
    }
    ++awIt;
  }
}

void AnalysisWindowApplyString(const vector<const CAnalysisWindowConfigItem*>awList,     // list of analysis windows
                               const string& windowName,                                // the name of a specific analysis window (all windows will be modified if this string is empty)
                               const string& xmlKey,                                    // the path of the field to replace
                               const string& xmlValue,                                  // string with the new value
                               mediate_analysis_window_t *pNewAnalysisWindow,           // target structure for analysis windows properties
                               char *field)                                             // pointer to the field to change (should point within the previous target structure)
 {
   // Declarations

   auto awIt = awList.begin();
   char msgString[MAX_STR_LEN+1];

   // Browse
   while (awIt!=awList.end()) {
     if (windowName.empty() || (windowName==(*awIt)->name())) {
       memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
       sprintf(msgString,"%s : %s replaced by %s (%s)",
               xmlKey.c_str(),
               field,xmlValue.c_str(),(*awIt)->name().c_str());
       std::cout << msgString << std::endl;
       strcpy(field,xmlValue.c_str());
       (*awIt)->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
     }
     ++awIt;
   }
 }

RC ParseAnalysisWindow(const vector<string>& xmlFields,int xmlFieldN,int startingField,const string& xmlKey,const string& xmlValue,const CProjectConfigItem *p)
{
  // Declarations

  auto awList = p->analysisWindowItems();
  mediate_analysis_window_t newAnalysisProperties;
  auto awIt = awList.begin();

  string windowName;

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

            std::cout << xmlKey << " can not be changed" << std::endl;

          // Fitting interval

          else if (xmlFields.at(indexField)=="min")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.fitMinWavelength);
          else if (xmlFields.at(indexField)=="max")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.fitMaxWavelength);
          else if (xmlFields.at(indexField)=="resol_fwhm")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.resolFwhm);
          else if (xmlFields.at(indexField)=="lambda0")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.lambda0);
          else if (xmlFields.at(indexField)=="display")
            std::cout << "project/analysis_window/display fields can not be changed" << std::endl;
          else if (xmlFields.at(indexField)=="refsel")
            {
              if ((xmlValue!="auto") && (xmlValue!="file"))
                std::cout << xmlKey << " do not support " << xmlValue << std::endl;
              else
                AnalysisWindowApplyInt(awList,windowName,xmlKey,(xmlValue=="auto")?ANLYS_REF_SELECTION_MODE_AUTOMATIC:ANLYS_REF_SELECTION_MODE_FILE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
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
            AnalysisWindowApplyString(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,newAnalysisProperties.refOneFile);
          else if (xmlFields.at(indexField)=="reftwo")
            AnalysisWindowApplyString(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,newAnalysisProperties.refTwoFile);
          else if (xmlFields.at(indexField)=="residual")
            AnalysisWindowApplyString(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,newAnalysisProperties.residualFile);
          else if (xmlFields.at(indexField)=="szacenter")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.refSzaCenter);
          else if (xmlFields.at(indexField)=="szadelta")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.refSzaDelta);
          else if (xmlFields.at(indexField)=="minlon")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.refMinLongitude);
          else if (xmlFields.at(indexField)=="maxlon")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.refMaxLongitude);
          else if (xmlFields.at(indexField)=="minlat")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.refMinLatitude);
          else if (xmlFields.at(indexField)=="maxlat")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.refMaxLatitude);
          else if (xmlFields.at(indexField)=="refns")
            AnalysisWindowApplyInt(awList,windowName,xmlKey,atoi(xmlValue.c_str()),&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
          else if (xmlFields.at(indexField)=="cloudfmin")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.cloudFractionMin);
          else if (xmlFields.at(indexField)=="cloudfmax")
            AnalysisWindowApplyDouble(awList,windowName,xmlKey,xmlValue,&newAnalysisProperties,&newAnalysisProperties.cloudFractionMax);
          else if (xmlFields.at(indexField)=="maxdoasrefmode")
            {
              if ((xmlValue!="scan") && (xmlValue!="sza"))
                std::cout << xmlKey << " do not support " << xmlValue << std::endl;
              else
                AnalysisWindowApplyInt(awList,windowName,xmlKey,(xmlValue=="scan")?ANLYS_MAXDOAS_REF_SCAN:ANLYS_MAXDOAS_REF_SZA,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
            }
          else if (xmlFields.at(indexField)=="scanmode")
            {
                 if (xmlValue=="before")
                  AnalysisWindowApplyInt(awList,windowName,xmlKey,ANLYS_MAXDOAS_REF_SCAN_BEFORE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else if (xmlValue=="after")
                  AnalysisWindowApplyInt(awList,windowName,xmlKey,ANLYS_MAXDOAS_REF_SCAN_AFTER,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else if (xmlValue=="average")
                  AnalysisWindowApplyInt(awList,windowName,xmlKey,ANLYS_MAXDOAS_REF_SCAN_AVERAGE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else if (xmlValue=="interpolate")
                  AnalysisWindowApplyInt(awList,windowName,xmlKey,ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE,&newAnalysisProperties,&newAnalysisProperties.refSpectrumSelection);
                 else
               std::cout << xmlKey << " do not support " << xmlValue << std::endl;
            }

          // pixel selection for GOME-ERS2 -> not supported

            else if ((xmlFields.at(indexField)=="east") ||
                     (xmlFields.at(indexField)=="center") ||
                     (xmlFields.at(indexField)=="west") ||
                     (xmlFields.at(indexField)=="backscan"))

                    std::cout << xmlKey << " can not be changed" << std::endl;
        }
    }


  // Return

  return rc;
 }

RC QDOASXML_Parse(vector<string> &xmlCommands,const CProjectConfigItem *p)
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

      int projectField = 0;

      for (size_t indexField=0;(indexField<xmlFieldsN) && !rc;indexField++) {
        if (projectField) {
          if (xmlFields.at(indexField)=="display")
            std::cout << xmlKey << " fields can not be changed" << std::endl;
          else if (xmlFields.at(indexField)=="selection")
            rc=ParseSelection(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,p);
          else if (xmlFields.at(indexField)=="analysis")
            rc=ParseAnalysis(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,p);
          else if (xmlFields.at(indexField)=="instrumental")
            rc=ParseInstrumental(xmlFields,xmlFieldsN,indexField+1,xmlValue,p);
          else if (xmlFields.at(indexField)=="analysis_window")
            rc=ParseAnalysisWindow(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,p);
          else if (xmlFields.at(indexField)=="calibration")
            rc=ParseCalibration(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,p);
          break;
        }
        else if (xmlFields.at(indexField)=="project")
          projectField=1;
      }
    }
    if (rc) {
      break;
    }
  }

  return rc;
 }

