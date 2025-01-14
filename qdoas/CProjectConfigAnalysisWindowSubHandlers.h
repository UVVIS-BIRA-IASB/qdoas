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

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, std::string>& attributes) override;

  static int mapToPolyType(const std::string &str);

 private:
  CAnalysisWindowConfigItem *m_item; // does not own this item
};

class CAnalysisWindowCrossSectionSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowCrossSectionSubHandler(CConfigHandler *master,
                    cross_section_list_t *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  cross_section_list_t *m_d;
};

class CAnalysisWindowLinearSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowLinearSubHandler(CConfigHandler *master,
                  struct anlyswin_linear *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  struct anlyswin_linear *m_d;
};

class CAnalysisWindowNonLinearSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowNonLinearSubHandler(CConfigHandler *master,
                     struct anlyswin_nonlinear *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  struct anlyswin_nonlinear *m_d;
};

class CAnalysisWindowShiftStretchSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowShiftStretchSubHandler(CConfigHandler *master,
                    shift_stretch_list_t *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, std::string>& attributes) override;
  virtual void end();

 private:
  shift_stretch_list_t *m_d;
};

class CAnalysisWindowGapSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowGapSubHandler(CConfigHandler *master,
                   gap_list_t *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  gap_list_t *m_d;
};

class CAnalysisWindowOutputSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowOutputSubHandler(CConfigHandler *master,
                  output_list_t *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  output_list_t *m_d;
};

class CAnalysisWindowSfpSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowSfpSubHandler(CConfigHandler *master,
                   struct calibration_sfp *d);

  virtual void start(const std::map<Glib::ustring, std::string>& attributes) override;

 private:
  struct calibration_sfp *m_d;
};


#endif
