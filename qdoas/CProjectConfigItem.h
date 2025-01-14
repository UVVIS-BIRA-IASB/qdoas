/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPROJECTCONFIGITEM_H_GUARD
#define _CPROJECTCONFIGITEM_H_GUARD

#include <vector>

#include "mediate.h"
#include "CProjectConfigTreeNode.h"

class CAnalysisWindowConfigItem
{
 public:
  CAnalysisWindowConfigItem();

  bool setName(const std::string &name);
  const std::string& name(void) const;

  void setEnabled(bool enabled);
  bool isEnabled(void) const;

  mediate_analysis_window_t* properties(void); // WARNING : allows (by design) poking at the internals ...
  const mediate_analysis_window_t* properties(void) const;

  int SetProperties(mediate_analysis_window_t *p) const;

 private:
  std::string m_name;
  bool m_enabled;
  mediate_analysis_window_t m_awProp;
};

class CProjectConfigItem
{
 public:
  ~CProjectConfigItem();
  CProjectConfigItem();

  void setName(const std::string &name);
  const std::string& name(void) const;

  void setEnabled(bool enabled);
  bool isEnabled(void) const;

  mediate_project_t* properties(void); // WARNING : allows (by design) poking at the internals ...
  const mediate_project_t* properties(void) const;

  CAnalysisWindowConfigItem* issueNewAnalysisWindowItem(void); // retains ownership
  const std::vector<const CAnalysisWindowConfigItem*>& analysisWindowItems(void) const;

  CProjectConfigTreeNode* rootNode(void); // WARNING : allows (by design) poking at the internals ...
  const CProjectConfigTreeNode* rootNode(void) const;

  int SetProperties(mediate_project_t *p) const;

 private:
  std::string m_name;
  bool m_enabled;
  mediate_project_t m_projProp;
  CProjectConfigTreeNode *m_root;
  std::vector<const CAnalysisWindowConfigItem*> m_awItemList;
};

class CSiteConfigItem
{
 public:
  CSiteConfigItem();

  void setSiteName(const std::string &name);
  void setAbbreviation(const std::string &abbreviation);
  void setLongitude(double longitude);
  void setLatitude(double latitude);
  void setAltitude(double altitude);

  const std::string& siteName(void) const;
  const std::string& abbreviation(void) const;
  double longitude(void) const;
  double latitude(void) const;
  double altitude(void) const;

 private:
  std::string m_siteName, m_abbreviation;
  double m_longitude, m_latitude, m_altitude;
};

class CSymbolConfigItem
{
 public:
  CSymbolConfigItem(const std::string &name, const std::string &description);

  const std::string& symbolName(void) const;
  const std::string& symbolDescription(void) const;

 private:
  std::string m_name, m_description;
};

inline void CSiteConfigItem::setSiteName(const std::string &siteName) { m_siteName = siteName; }
inline void CSiteConfigItem::setAbbreviation(const std::string &abbreviation) { m_abbreviation = abbreviation; }
inline void CSiteConfigItem::setLongitude(double longitude) { m_longitude = longitude; }
inline void CSiteConfigItem::setLatitude(double latitude) { m_latitude = latitude; }
inline void CSiteConfigItem::setAltitude(double altitude) { m_altitude = altitude; }

inline const std::string& CSiteConfigItem::siteName(void) const { return m_siteName; }
inline const std::string& CSiteConfigItem::abbreviation(void) const { return m_abbreviation; }
inline double CSiteConfigItem::longitude(void) const { return m_longitude; }
inline double CSiteConfigItem::latitude(void) const { return m_latitude; }
inline double CSiteConfigItem::altitude(void) const { return m_altitude; }

inline const std::string& CSymbolConfigItem::symbolName(void) const { return m_name; }
inline const std::string& CSymbolConfigItem::symbolDescription(void) const { return m_description; }

#endif
