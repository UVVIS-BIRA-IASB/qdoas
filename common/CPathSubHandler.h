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

  virtual void start(const xmlstring& element, const std::map<xmlstring, std::string>& atts) override;
  virtual void character(const std::string &ch);
  virtual void end(const xmlstring& element);

 private:
  int m_index;
  std::string m_path;
};

#endif
