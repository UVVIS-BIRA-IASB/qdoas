/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CBATCHENGINECONTROLLER_H_GUARD
#define _CBATCHENGINECONTROLLER_H_GUARD

#include "CEngineController.h"
#include "mediate_project.h"


class CBatchEngineController : public CEngineController
{
 public:
  CBatchEngineController();

  // query interface
  bool active(void) const;

  // notify interface is for use by response classes
  virtual void notifyReadyToNavigateRecords(const std::string &filename, int numberOfRecords);
  virtual void notifyEndOfRecords(void);

  virtual void notifyErrorMessages(int highestErrorLevel, const std::vector<CEngineError> &errorMessages);

 private:
  bool    m_active;
};

inline bool CBatchEngineController::active(void) const { return m_active; }

#endif
