/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CENGINEREQUEST_H_GUARD
#define _CENGINEREQUEST_H_GUARD

#include <QString>
#include <QList>

#include "mediate_request.h"

class CEngineThread;

//------------------------------------------------------------

class CEngineRequest
{
 public:
  virtual ~CEngineRequest() {};

  virtual bool process(CEngineThread *engineThread) = 0;

 protected:
};

//------------------------------------------------------------

class CEngineRequestCompound : public CEngineRequest
{
 public:
  virtual ~CEngineRequestCompound();

  virtual bool process(CEngineThread *engineThread);

  void addRequest(CEngineRequest *req);

 private:
  QList<CEngineRequest*> m_requestList;
};

//------------------------------------------------------------

class CEngineRequestSetProject : public CEngineRequest
{
 public:
  CEngineRequestSetProject(const mediate_project_t *project, int opMode);

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_project_t m_project;
  int m_opMode;
};

//------------------------------------------------------------

class CEngineRequestSetAnalysisWindows : public CEngineRequest
{
 public:
  CEngineRequestSetAnalysisWindows(const mediate_analysis_window_t *windowList, int nWindows,int opMode);
  virtual ~CEngineRequestSetAnalysisWindows();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_analysis_window_t *m_windowList;
  int m_nWindows, m_opMode;
};

//------------------------------------------------------------

class CEngineRequestSetSymbols : public CEngineRequest
{
 public:
  CEngineRequestSetSymbols(mediate_symbol_t *symbolList, int nSymbols);
  virtual ~CEngineRequestSetSymbols();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_symbol_t *m_symbolList;
  int m_nSymbols;
};

//------------------------------------------------------------

class CEngineRequestSetSites : public CEngineRequest
{
 public:
  CEngineRequestSetSites(mediate_site_t *siteList, int nSites);
  virtual ~CEngineRequestSetSites();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_site_t *m_siteList;
  int m_nSites;
};

//------------------------------------------------------------

class CEngineRequestBeginBrowseFile : public CEngineRequest
{
 public:
  CEngineRequestBeginBrowseFile(const std::string &fileName) : m_fileName(fileName) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  std::string m_fileName;
};

//------------------------------------------------------------

class CEngineRequestBrowseNextRecord : public CEngineRequest
{
 public:
  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestBrowseSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestBrowseSpecificRecord(int recordNumber) : m_recordNumber(recordNumber) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestBeginExportFile : public CEngineRequest
{
 public:
  CEngineRequestBeginExportFile(const std::string& fileName) : m_fileName(fileName) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  std::string m_fileName;
};

//------------------------------------------------------------

class CEngineRequestExportNextRecord : public CEngineRequest
{
 public:
  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestExportSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestExportSpecificRecord(int recordNumber) : m_recordNumber(recordNumber) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestBeginAnalyseFile : public CEngineRequest
{
 public:
  CEngineRequestBeginAnalyseFile(const std::string &fileName) : m_fileName(fileName) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  std::string m_fileName;
};

//------------------------------------------------------------

class CEngineRequestAnalyseNextRecord : public CEngineRequest
{
 public:
  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestAnalyseSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestAnalyseSpecificRecord(int recordNumber) : m_recordNumber(recordNumber) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestBeginCalibrateFile : public CEngineRequest
{
 public:
  CEngineRequestBeginCalibrateFile(const std::string &fileName) : m_fileName(fileName) {};
  virtual bool process(CEngineThread *engineThread);

 private:
  std::string m_fileName;
};

//------------------------------------------------------------

class CEngineRequestCalibrateNextRecord : public CEngineRequest
{
 public:
  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestCalibrateSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestCalibrateSpecificRecord(int recordNumber) : m_recordNumber(recordNumber) {};

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestStop : public CEngineRequest
{
 public:
  virtual bool process(CEngineThread *engineThread);

};

//------------------------------------------------------------

class CEngineRequestViewCrossSections : public CEngineRequest
{
 public:
  CEngineRequestViewCrossSections(char *awName,double minWavelength, double maxWavelength,
                                  int nFiles, char **filenames);
  virtual ~CEngineRequestViewCrossSections();

  virtual bool process(CEngineThread *engineThread);

 private:
  char *m_awName;
  double m_minWavelength, m_maxWavelength;
  int m_nFiles;
  char **m_filenames;

};

//------------------------------------------------------------

#endif
