/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPATHSUBHANDLERS_H_GUARD
#define _CPATHSUBHANDLERS_H_GUARD

#include "CConfigHandler.h"


class CPathSubHandler : public CConfigSubHandler
{
 public:
  CPathSubHandler(CConfigHandler *master) : CConfigSubHandler(master), m_index(-1) {};

  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString>& atts) override;
  virtual bool character(const QString &ch);
  virtual void end(const Glib::ustring& element);

 private:
  int m_index;
  QString m_path;
};

#endif
