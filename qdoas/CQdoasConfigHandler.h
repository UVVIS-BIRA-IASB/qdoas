/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CQDOASCONFIGHANDLER_H_GUARD
#define _CQDOASCONFIGHANDLER_H_GUARD

#include "CConfigHandler.h"
#include "CProjectConfigItem.h"


class CQdoasConfigHandler;

class CQdoasConfigHandler : public CConfigHandler
{
 public:

  void addProjectItem(CProjectConfigItem item);             // takes ownership of item
  const std::vector<CProjectConfigItem>& projectItems() const; // items in returned list have the same lifetime as 'this'

  void addSiteItem(CSiteConfigItem item);             // takes ownership of item
  const std::vector<CSiteConfigItem>& siteItems() const; // items in returned list have the same lifetime as 'this'

  void addSymbol(const std::string &symbolName, const std::string &symbolDescription);
  const std::vector<CSymbolConfigItem>& symbolItems() const; // items in returned list have the same lifetime as 'this'

protected:
  virtual void start_subhandler(const Glib::ustring& name,
                                const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  std::vector<CProjectConfigItem> m_projectItemList;
  std::vector<CSiteConfigItem> m_siteItemList;
  std::vector<CSymbolConfigItem> m_symbolList;
};

//-------------------------------------------------------------------

class CQdoasConfigSubHandler : public CConfigSubHandler
{
public:
  CQdoasConfigSubHandler(CQdoasConfigHandler *master) : CConfigSubHandler(master) {};

  virtual CQdoasConfigHandler *master() { return static_cast<CQdoasConfigHandler *>(m_master); };

};

//-------------------------------------------------------------------
// specfic handlers that need the CQdoasConfigHandler interface

class CSiteSubHandler : public CQdoasConfigSubHandler
{
 public:
  CSiteSubHandler(CQdoasConfigHandler *master);

  virtual void start(const Glib::ustring&name,
                     const std::map<Glib::ustring, std::string>& attributes) override;
};

//-------------------------------------------------------------------

class CSymbolSubHandler : public CQdoasConfigSubHandler
{
 public:
  CSymbolSubHandler(CQdoasConfigHandler *master);

  virtual void start(const Glib::ustring& name,
                     const std::map<Glib::ustring, std::string>& attributes) override;
};

//-------------------------------------------------------------------

class CProjectSubHandler : public CQdoasConfigSubHandler
{
 public:
  CProjectSubHandler(CQdoasConfigHandler *master);

  virtual void start(const Glib::ustring& element,
                     const std::map<Glib::ustring, std::string>& atts) override;
  virtual void start(const std::map<Glib::ustring, std::string>& atts) override;
  virtual void end(void) override;

 private:
  CProjectConfigItem m_project;
};

#endif
