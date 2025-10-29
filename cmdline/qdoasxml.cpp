
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
#include <map>
#include <tuple>
#include <variant>

#include "qdoasxml.h"

using std::string;
using std::vector;

struct config_visitor {
public:
  config_visitor(const string& value) : xml_value(value) {};

  void operator()(char *p) const {
    strcpy(p, xml_value.c_str());
  }

  void operator()(std::tuple<int *, vector<string>> t) const {
    int val = 0;
    for (const auto& choice : std::get<1>(t)) {
      if (xml_value == choice) {
        *std::get<0>(t) = val;
        return;
      }
      ++val;
    }
    throw std::runtime_error("Invalid choice: " + xml_value);
  }

  void operator()(double *d) const {
    *d = std::stod(xml_value);
  }

  void operator()(int *i) const {
    *i = std::stoi(xml_value);
  }

  void operator()(enum omi_xtrack_mode *m) const {
    *m = str_to_mode(xml_value.c_str());
  }
private:
  const string& xml_value;
};

using config_pointer = std::variant<double *, int *, char *,\
                                    std::tuple<int *, vector<string> >, \
                                    enum omi_xtrack_mode *>;

RC parse_project_properties(const string& xmlKey, const string& xmlValue, const CProjectConfigItem *p) {
  mediate_project_t newProjectProperties = *p->properties();
  std::map<string, config_pointer> config_keys {
    {"/project/selection/sza/min", &newProjectProperties.selection.szaMinimum},
    {"/project/selection/sza/max", &newProjectProperties.selection.szaMaximum},
    {"/project/selection/sza/delta", &newProjectProperties.selection.szaDelta},
    {"/project/selection/record/min", &newProjectProperties.selection.recordNumberMinimum},
    {"/project/selection/record/max", &newProjectProperties.selection.recordNumberMaximum},
    {"/project/selection/cloud/min", &newProjectProperties.selection.cloudFractionMinimum},
    {"/project/selection/cloud/max", &newProjectProperties.selection.cloudFractionMaximum},
    {"/project/selection/geolocation/selected", std::make_tuple(&newProjectProperties.selection.geo.mode,
                                                                vector<string>{"none","circle","rectangle","sites"})},
    {"/project/selection/geolocation/circle/radius", &newProjectProperties.selection.geo.circle.radius},
    {"/project/selection/geolocation/cirle/long", &newProjectProperties.selection.geo.circle.centerLongitude},
    {"/project/selection/geolocation/cirle/lat", &newProjectProperties.selection.geo.circle.centerLatitude},
    {"/project/selection/geolocation/rectangle/west", &newProjectProperties.selection.geo.rectangle.westernLongitude},
    {"/project/selection/geolocation/rectangle/east", &newProjectProperties.selection.geo.rectangle.easternLongitude},
    {"/project/selection/geolocation/rectangle/south", &newProjectProperties.selection.geo.rectangle.southernLatitude},
    {"/project/selection/geolocation/rectangle/north", &newProjectProperties.selection.geo.rectangle.northernLatitude},
    {"/project/selection/geolocation/sites/radius", &newProjectProperties.selection.geo.sites.radius},
    {"/project/analysis/converge", &newProjectProperties.analysis.convergenceCriterion},
    {"/project/analysis/max_iterations", &newProjectProperties.analysis.maxIterations},
    {"/project/calibration/line/slfFile", newProjectProperties.calibration.slfFile},
    {"/project/instrumental/omi/trackSelection", newProjectProperties.instrumental.omi.trackSelection},
    {"/project/instrumental/omi/xTrackMode", &newProjectProperties.instrumental.omi.xtrack_mode},
    {"/project/instrumental/tropomi/trackSelection", newProjectProperties.instrumental.tropomi.trackSelection},
    {"/project/instrumental/apex/trackSelection", newProjectProperties.instrumental.apex.trackSelection}};

  auto i_config = config_keys.find(xmlKey);
  if (i_config != config_keys.end()) {
    config_visitor visitor { xmlValue };
    try {
      std::visit(visitor, i_config->second);
      std::cout << xmlKey << " replaced by " << xmlValue
                << " ("  << p->name() << ")" << std::endl;
    } catch (std::exception &e) {
      std::cerr << "ERROR: invalid setting '" << xmlValue << "' for key " << xmlKey << std::endl;
      return -1;
    }
  } else {
    std::cerr << "ERROR: unknown -xml configuration key '" << xmlKey << "'" << std::endl;
    return -1;
  }
  p->SetProperties(&newProjectProperties);
  return ERROR_ID_NO;
}

// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

void AnalysisWindowApplyInt(const vector<CAnalysisWindowConfigItem>& awList,   // list of analysis windows
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
    if (windowName.empty() || (windowName==awIt->name())) {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)(awIt->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %d replaced by %d (%s)",
              xmlKey.c_str(),
              *pIntField,newValue,awIt->name().c_str());
      std::cout << msgString << std::endl;
      *pIntField=newValue;
      awIt->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
    }
    ++awIt;
  }
}

void AnalysisWindowApplyDouble(const vector<CAnalysisWindowConfigItem>& awList,     // list of analysis windows
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
    if (windowName.empty() || (windowName==awIt->name())) {
      memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)(awIt->properties()),sizeof(mediate_analysis_window_t));
      sprintf(msgString,"%s : %.2lf replaced by %s (%s)",
              xmlKey.c_str(),
              *pDoubleField,xmlValue.c_str(),awIt->name().c_str());
      std::cout << msgString << std::endl;
      *pDoubleField=(double)atof(xmlValue.c_str());
      awIt->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
    }
    ++awIt;
  }
}

void AnalysisWindowApplyString(const vector<CAnalysisWindowConfigItem>& awList,     // list of analysis windows
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
     if (windowName.empty() || (windowName==awIt->name())) {
       memcpy(pNewAnalysisWindow,(mediate_analysis_window_t *)(awIt->properties()),sizeof(mediate_analysis_window_t));
       sprintf(msgString,"%s : %s replaced by %s (%s)",
               xmlKey.c_str(),
               field,xmlValue.c_str(),awIt->name().c_str());
       std::cout << msgString << std::endl;
       strcpy(field,xmlValue.c_str());
       awIt->SetProperties(( mediate_analysis_window_t *)pNewAnalysisWindow);
     }
     ++awIt;
   }
 }

RC ParseAnalysisWindow(const vector<string>& xmlFields,int xmlFieldN,int startingField,const string& xmlKey,const string& xmlValue,const CProjectConfigItem *p)
{
  // Declarations

  auto& awList = p->analysisWindowItems();
  mediate_analysis_window_t newAnalysisProperties;

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
          else if (xmlFields.at(indexField)=="selection" || xmlFields.at(indexField)=="analysis" || xmlFields.at(indexField)=="instrumental" || xmlFields.at(indexField)=="calibration")
            rc = parse_project_properties(xmlKey, xmlValue, p);
          else if (xmlFields.at(indexField)=="analysis_window")
            rc=ParseAnalysisWindow(xmlFields,xmlFieldsN,indexField+1,xmlKey,xmlValue,p);
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

