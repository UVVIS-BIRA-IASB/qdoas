/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CPROJECTCONFIGSUBHANDLERS_H_GUARD
#define _CPROJECTCONFIGSUBHANDLERS_H_GUARD

#include "CConfigHandler.h"
#include "mediate_project.h"

class CProjectConfigTreeNode;

//-------------------------------------------------------------------
// base class for display and output handlers

class CSelectorSubHandler : public CConfigSubHandler
{
 public:
  CSelectorSubHandler(CConfigHandler *master, data_select_list_t *selectList);

  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString> &atts);

 private:
  data_select_list_t *m_selectList;
};

//-------------------------------------------------------------------

class CProjectDisplaySubHandler : public CSelectorSubHandler
{
 public:
  CProjectDisplaySubHandler(CConfigHandler *master, mediate_project_display_t *display);

  virtual void start(const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_display_t *m_display;
};

//-------------------------------------------------------------------

class CProjectSelectionSubHandler : public CConfigSubHandler
{
 public:
  CProjectSelectionSubHandler(CConfigHandler *master, mediate_project_selection_t *selection);

  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_selection_t *m_selection;
};

//-------------------------------------------------------------------

class CProjectAnalysisSubHandler : public CConfigSubHandler
{
 public:
  CProjectAnalysisSubHandler(CConfigHandler *master,
                mediate_project_analysis_t *analysis);

  virtual void start(const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_analysis_t *m_analysis;
};

//-------------------------------------------------------------------

class CProjectRawSpectraSubHandler : public CConfigSubHandler
{
 public:
  CProjectRawSpectraSubHandler(CConfigHandler *master,
                  CProjectConfigTreeNode *node);

  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString> &atts);

 private:
  CProjectConfigTreeNode *m_node;
};

//-------------------------------------------------------------------

class CProjectCalibrationSubHandler : public CConfigSubHandler
{
 public:
  CProjectCalibrationSubHandler(CConfigHandler *master,
               mediate_project_calibration_t *calibration);

  virtual void start(const std::map<Glib::ustring, QString> &atts);
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_calibration_t *m_calibration;
};

//-------------------------------------------------------------------

class CProjectUndersamplingSubHandler : public CConfigSubHandler
{
 public:
  CProjectUndersamplingSubHandler(CConfigHandler *master,
               mediate_project_undersampling_t *undersampling);

  virtual void start(const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_undersampling_t *m_undersampling;
};

//-------------------------------------------------------------------

class CProjectInstrumentalSubHandler : public CConfigSubHandler
{
 public:
  CProjectInstrumentalSubHandler(CConfigHandler *master,
               mediate_project_instrumental_t *instrumental);

  virtual void start(const std::map<Glib::ustring, QString> &atts);
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString> &atts);

 protected:
  void helperLoadLogger(const std::map<Glib::ustring, QString> &atts, struct instrumental_logger *d);
  void helperLoadSaoz(const std::map<Glib::ustring, QString> &atts, struct instrumental_saoz *d);
  void helperLoadMinimum(const std::map<Glib::ustring, QString> &atts, struct instrumental_minimum *d);
  void helperLoadCcd(const std::map<Glib::ustring, QString> &atts, struct instrumental_ccd *d);
  void helperLoadGdp(const std::map<Glib::ustring, QString> &atts, struct instrumental_gdp *d);
  void helperLoadGome2(const std::map<Glib::ustring, QString> &atts, struct instrumental_gome2 *d);
  void helperLoadScia(const std::map<Glib::ustring, QString> &atts, struct instrumental_scia *d);


 private:
  mediate_project_instrumental_t *m_instrumental;
};

//-------------------------------------------------------------------

class CProjectSlitSubHandler : public CConfigSubHandler
{
 public:
  CProjectSlitSubHandler(CConfigHandler *master,
             mediate_project_slit_t *slit);

  virtual void start(const std::map<Glib::ustring, QString> &atts);
  virtual void start(const Glib::ustring& element, const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_slit_t *m_slit;
};

//-------------------------------------------------------------------

class CProjectOutputSubHandler : public CSelectorSubHandler
{
 public:
  CProjectOutputSubHandler(CConfigHandler *master,
               mediate_project_output_t *output);

  virtual void start(const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_output_t *m_output;
};

//-------------------------------------------------------------------

class CProjectExportSubHandler : public CSelectorSubHandler
{
 public:
  CProjectExportSubHandler(CConfigHandler *master,
               mediate_project_export_t *exportSpectra);

  virtual void start(const std::map<Glib::ustring, QString> &atts);

 private:
  mediate_project_export_t *m_export;
};

//-------------------------------------------------------------------



#endif

