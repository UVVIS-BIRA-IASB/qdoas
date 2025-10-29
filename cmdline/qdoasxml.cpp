
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
      std::cout << xmlKey << " set to " << xmlValue
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

RC parse_analysis_window(const string& xmlKey, const string& xmlValue, const CProjectConfigItem *p) {
  const char prefix[] = "/project/analysis_window";

  // look for next segment in xmlKey.  We have either
  //
  //  - "/project/analysis_window/<window_name>/<setting>" -> change a setting for a specific analysis window
  //
  //  - "/project/analysis_window/<setting>" -> change a setting for all analysis windows
  size_t next_slash = xmlKey.find("/", sizeof(prefix));
  string next_element = xmlKey.substr(sizeof(prefix),
                                      (next_slash != string::npos) ? next_slash - sizeof(prefix) : string::npos);

  // if no window name is given:
  string window_name = "";
  string setting = xmlKey.substr(sizeof(prefix));
  if (next_slash != string::npos && next_element != "files") {
    // a window name is given:
    window_name = next_element;
    setting = xmlKey.substr(1 + next_slash);
  }

  mediate_analysis_window_t newAnalysisProperties;

  std::map<string, config_pointer> config_keys {
    {"min", &newAnalysisProperties.fitMinWavelength},
    {"max", &newAnalysisProperties.fitMaxWavelength},
    {"resol_fwhm", &newAnalysisProperties.resolFwhm},
    {"lambda0", &newAnalysisProperties.lambda0},
    {"files/refone", newAnalysisProperties.refOneFile},
    {"files/reftwo", newAnalysisProperties.refTwoFile},
    {"files/residual", newAnalysisProperties.residualFile},
    {"files/szacenter", &newAnalysisProperties.refSzaCenter},
    {"files/szadelta", &newAnalysisProperties.refSzaDelta},
    {"files/minlon", &newAnalysisProperties.refMinLongitude},
    {"files/maxlon", &newAnalysisProperties.refMaxLongitude},
    {"files/minlat", &newAnalysisProperties.refMinLatitude},
    {"files/maxlat", &newAnalysisProperties.refMaxLatitude},
    {"files/refns", &newAnalysisProperties.refSpectrumSelection},
    {"files/cloudfmin", &newAnalysisProperties.cloudFractionMin},
    {"files/cloudfmax", &newAnalysisProperties.cloudFractionMax},
    {"files/maxdoasrefmode", std::make_tuple(&newAnalysisProperties.refSpectrumSelection,
                                             vector<string>{"sza","scan"})},
    {"files/scanmode", std::make_tuple(&newAnalysisProperties.refSpectrumSelectionScanMode,
                                       vector<string>{"before","after", "average", "interpolate"})}
  };

  auto i_config = config_keys.find(setting);
  if (i_config != config_keys.end()) {
    auto& awList = p->analysisWindowItems();
    for (auto &aw : awList) {
      if (!window_name.empty() && window_name != aw.name()) {
        continue;
      }
      config_visitor visitor { xmlValue };
      newAnalysisProperties = *aw.properties();
      try {
        std::visit(visitor, i_config->second);
        std::cout << prefix << "/" << setting << " set to " << xmlValue
                  << " ("  << aw.name() << ")" << std::endl;
        aw.SetProperties(&newAnalysisProperties);
      } catch (std::exception &e) {
        std::cerr << "ERROR: invalid setting '" << xmlValue << "' for key " << prefix << "/" << setting << std::endl;
        return -1;
      }
    }
  } else {
    std::cerr << "ERROR: unknown -xml configuration key '" << prefix << "/" << setting << "'" << std::endl;
    return -1;
  }
  return ERROR_ID_NO;
}

RC QDOASXML_Parse(vector<string> &xmlCommands,const CProjectConfigItem *p)
 {
  RC rc = ERROR_ID_NO;

  for (const auto& xml_cmd : xmlCommands) {
    vector<string> xmlParts;
    boost::split(xmlParts, xml_cmd, boost::is_any_of("="));
    if (xmlParts.size()!=2) {
      std::cerr << "ERROR: illegal value for -xml option: '" << xml_cmd << "'"
                << " (expect 'key=value' format)" << std::endl;
      return -1;
    }

    const auto xmlKey=xmlParts.at(0);
    const auto xmlValue=xmlParts.at(1);

    if (xmlKey.rfind("/project/analysis_window", 0) == 0) {
      // starts with "/project/analysis_window:
      rc = parse_analysis_window(xmlKey,xmlValue,p);
    } else {
      rc = parse_project_properties(xmlKey, xmlValue, p);
    }
    if (rc) {
      break;
    }
  }

  return rc;
 }
