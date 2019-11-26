
#include "CProjectConfigItem.h"


CAnalysisWindowConfigItem::CAnalysisWindowConfigItem() :
  m_enabled(true)
{
  initializeMediateAnalysisWindow(&m_awProp);
}

bool CAnalysisWindowConfigItem::setName(const QString &name)
{
  if (name.length() < (int)sizeof(m_awProp.name)) {

    strcpy(m_awProp.name, name.toLocal8Bit().data());
    m_name = name;

    return true;
  }

  return false;
}

const QString& CAnalysisWindowConfigItem::name(void) const
{
  return m_name;
}

void CAnalysisWindowConfigItem::setEnabled(bool enabled)
{
  m_enabled = enabled;
}

bool CAnalysisWindowConfigItem::isEnabled(void) const
{
  return m_enabled;
}

mediate_analysis_window_t* CAnalysisWindowConfigItem::properties(void)
{
  // WARNING : allows (by design) poking at the internals ...
  return &m_awProp;
}

const mediate_analysis_window_t* CAnalysisWindowConfigItem::properties(void) const
{
  return &m_awProp;
}

int CAnalysisWindowConfigItem::SetProperties(mediate_analysis_window_t *a) const
{
  // WARNING : allows (by design) poking at the internals ...

  memcpy((void *)&m_awProp,(const void *)a,sizeof(mediate_analysis_window_t));

  return 0;
}

//------------------------------------------------------------

CProjectConfigItem::CProjectConfigItem() :
  m_enabled(true)
{
  m_root = new CProjectConfigFolder("root", true);
  initializeMediateProject(&m_projProp, "", "");
}

CProjectConfigItem::~CProjectConfigItem()
{
  while (!m_awItemList.empty())
    delete m_awItemList.takeFirst();

  delete m_root;
}

void CProjectConfigItem::setName(const QString &name)
{
  m_name = name;
  strncpy(m_projProp.project_name, name.toLocal8Bit().data(), PROJECT_NAME_BUFFER_LENGTH-1);
}

const QString& CProjectConfigItem::name(void) const
{
  return m_name;
}

void CProjectConfigItem::setEnabled(bool enabled)
{
  m_enabled = enabled;
}

bool CProjectConfigItem::isEnabled(void) const
{
  return m_enabled;
}

mediate_project_t* CProjectConfigItem::properties(void)
{
  // WARNING : allows (by design) poking at the internals ...

  return &m_projProp;
}

const mediate_project_t* CProjectConfigItem::properties(void) const
{
  return &m_projProp;
}

int CProjectConfigItem::SetProperties(mediate_project_t *p) const
{
  // WARNING : allows (by design) poking at the internals ...

  memcpy((void *)&m_projProp,(const void *)p,sizeof(mediate_project_t));

  return 0;
}

CAnalysisWindowConfigItem* CProjectConfigItem::issueNewAnalysisWindowItem(void)
{
  CAnalysisWindowConfigItem *tmp = new CAnalysisWindowConfigItem;
  m_awItemList.push_back(tmp);

  // retains ownership
  return tmp;
}

const QList<const CAnalysisWindowConfigItem*>& CProjectConfigItem::analysisWindowItems(void) const
{
  return m_awItemList;
}

CProjectConfigTreeNode* CProjectConfigItem::rootNode(void)
{
  return m_root;
}

const CProjectConfigTreeNode* CProjectConfigItem::rootNode(void) const
{
  return m_root;
}


//------------------------------------------------------------

CSiteConfigItem::CSiteConfigItem() :
  m_longitude(0.0),
  m_latitude(0.0),
  m_altitude(0.0)
{
}

//------------------------------------------------------------

CSymbolConfigItem::CSymbolConfigItem(const QString &name, const QString &description) :
  m_name(name),
  m_description(description)
{
}
