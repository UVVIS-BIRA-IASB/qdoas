/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPROJECTCONFIGANALYSISWINDOWSUBHANDLERS_H_GUARD
#define _CPROJECTCONFIGANALYSISWINDOWSUBHANDLERS_H_GUARD

#include "CConfigHandler.h"
#include "mediate_analysis_window.h"

class CAnalysisWindowConfigItem;

class CAnalysisWindowSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowSubHandler(CConfigHandler *master,
                CAnalysisWindowConfigItem *item);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

  static int mapToPolyType(const QString &str);

 private:
  CAnalysisWindowConfigItem *m_item; // does not own this item
};

class CAnalysisWindowCrossSectionSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowCrossSectionSubHandler(CConfigHandler *master,
                    cross_section_list_t *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  cross_section_list_t *m_d;
};

class CAnalysisWindowLinearSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowLinearSubHandler(CConfigHandler *master,
                  struct anlyswin_linear *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  struct anlyswin_linear *m_d;
};

class CAnalysisWindowNonLinearSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowNonLinearSubHandler(CConfigHandler *master,
                     struct anlyswin_nonlinear *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  struct anlyswin_nonlinear *m_d;
};

class CAnalysisWindowShiftStretchSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowShiftStretchSubHandler(CConfigHandler *master,
                    shift_stretch_list_t *d);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool end(void);

 private:
  shift_stretch_list_t *m_d;
};

class CAnalysisWindowGapSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowGapSubHandler(CConfigHandler *master,
                   gap_list_t *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  gap_list_t *m_d;
};

class CAnalysisWindowOutputSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowOutputSubHandler(CConfigHandler *master,
                  output_list_t *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  output_list_t *m_d;
};

class CAnalysisWindowSfpSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowSfpSubHandler(CConfigHandler *master,
                   struct calibration_sfp *d);

  virtual bool start(const QXmlAttributes &atts);

 private:
  struct calibration_sfp *m_d;
};


#endif
