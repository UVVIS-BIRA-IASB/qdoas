/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA
*/


#ifndef _CUSAMPCONFIGHANDLER_H_GUARD
#define _CUSAMPCONFIGHANDLER_H_GUARD

#include <string>

#include "CConfigHandler.h"

#include "mediate_usamp.h"

//------------------------------------------------------------------

class CUsampConfigHandler : public CConfigHandler
{
 public:
  CUsampConfigHandler();

  const mediate_usamp_t* properties(void) const;

protected:
  virtual void start_subhandler(const xmlstring& name,
                                const std::map<xmlstring, std::string>& attributes) override;

 private:
  mediate_usamp_t m_properties;
};

inline const mediate_usamp_t* CUsampConfigHandler::properties(void) const { return &m_properties; }

//------------------------------------------------------------------

class CUsampGeneralSubHandler : public CConfigSubHandler
{
 public:
  CUsampGeneralSubHandler(CConfigHandler *master, mediate_usamp_t *d);

  virtual void start(const std::map<xmlstring, std::string>& attributes) override;
  virtual void start(const xmlstring& element, const std::map<xmlstring, std::string>& attributes) override;

 private:
  mediate_usamp_t *m_d;
};

#endif
