/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CRINGCONFIGHANDLER_H_GUARD
#define _CRINGCONFIGHANDLER_H_GUARD

#include "CConfigHandler.h"

#include "mediate_ring.h"

//------------------------------------------------------------------

class CRingConfigHandler : public CConfigHandler
{
 public:
  CRingConfigHandler();

  const mediate_ring_t* properties(void) const;

protected:
  virtual void start_subhandler(const xmlstring& name,
                                const std::map<xmlstring, std::string>& attributes) override;

 private:
  mediate_ring_t m_properties;
};

inline const mediate_ring_t* CRingConfigHandler::properties(void) const { return &m_properties; }

//------------------------------------------------------------------

class CRingGeneralSubHandler : public CConfigSubHandler
{
 public:
  CRingGeneralSubHandler(CConfigHandler *master, mediate_ring_t *d);

  virtual void start(const std::map<xmlstring, std::string>& attributes) override;
  virtual void start(const xmlstring& element, const std::map<xmlstring, std::string>& attributes) override;

 private:
  mediate_ring_t *m_d;
};

#endif
