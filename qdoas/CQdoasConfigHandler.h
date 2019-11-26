/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _CQDOASCONFIGHANDLER_H_GUARD
#define _CQDOASCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QList>
#include <QString>
#include <QVector>

#include "CConfigHandler.h"

class CProjectConfigItem;
class CSiteConfigItem;
class CSymbolConfigItem;
class CQdoasConfigHandler;

class CQdoasConfigHandler : public CConfigHandler
{
 public:
  virtual ~CQdoasConfigHandler();

  virtual bool startElement(const QString &namespaceURI, const QString &localName,
			    const QString &qName, const QXmlAttributes &atts);

  void addProjectItem(CProjectConfigItem *item);             // takes ownership of item
  QList<const CProjectConfigItem*> projectItems(void) const; // items in returned list have the same lifetime as 'this'
  QList<const CProjectConfigItem*> takeProjectItems(void);   // takes ownership of items (removes them from 'this')

  void addSiteItem(CSiteConfigItem *item);             // takes ownership of item
  QList<const CSiteConfigItem*> siteItems(void) const; // items in returned list have the same lifetime as 'this'

  void addSymbol(const QString &symbolName, const QString &symbolDescription);
  QList<const CSymbolConfigItem*> symbolItems(void) const; // items in returned list have the same lifetime as 'this'

 private:
  QList<const CProjectConfigItem*> m_projectItemList;
  QList<const CSiteConfigItem*> m_siteItemList;
  QList<const CSymbolConfigItem*> m_symbolList;
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

  virtual bool start(const QString &element, const QXmlAttributes &atts);
};

//-------------------------------------------------------------------

class CSymbolSubHandler : public CQdoasConfigSubHandler
{
 public:
  CSymbolSubHandler(CQdoasConfigHandler *master);

  virtual bool start(const QString &element, const QXmlAttributes &atts);
};

//-------------------------------------------------------------------

class CProjectSubHandler : public CQdoasConfigSubHandler
{
 public:
  CProjectSubHandler(CQdoasConfigHandler *master);
  virtual ~CProjectSubHandler();

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool end(void);

 private:
  CProjectConfigItem *m_project;
};

#endif
