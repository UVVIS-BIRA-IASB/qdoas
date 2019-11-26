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

  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  data_select_list_t *m_selectList;
};

//-------------------------------------------------------------------

class CProjectDisplaySubHandler : public CSelectorSubHandler
{
 public:
  CProjectDisplaySubHandler(CConfigHandler *master, mediate_project_display_t *display);

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_project_display_t *m_display;
};

//-------------------------------------------------------------------

class CProjectSelectionSubHandler : public CConfigSubHandler
{
 public:
  CProjectSelectionSubHandler(CConfigHandler *master, mediate_project_selection_t *selection);

  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_project_selection_t *m_selection;
};

//-------------------------------------------------------------------

class CProjectAnalysisSubHandler : public CConfigSubHandler
{
 public:
  CProjectAnalysisSubHandler(CConfigHandler *master,
			    mediate_project_analysis_t *analysis);

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_project_analysis_t *m_analysis;
};

//-------------------------------------------------------------------

class CProjectRawSpectraSubHandler : public CConfigSubHandler
{
 public:
  CProjectRawSpectraSubHandler(CConfigHandler *master,
			      CProjectConfigTreeNode *node);

  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  CProjectConfigTreeNode *m_node;
};

//-------------------------------------------------------------------

class CProjectCalibrationSubHandler : public CConfigSubHandler
{
 public:
  CProjectCalibrationSubHandler(CConfigHandler *master,
			   mediate_project_calibration_t *calibration);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_project_calibration_t *m_calibration;
};

//-------------------------------------------------------------------

class CProjectUndersamplingSubHandler : public CConfigSubHandler
{
 public:
  CProjectUndersamplingSubHandler(CConfigHandler *master,
			   mediate_project_undersampling_t *undersampling);

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_project_undersampling_t *m_undersampling;
};

//-------------------------------------------------------------------

class CProjectInstrumentalSubHandler : public CConfigSubHandler
{
 public:
  CProjectInstrumentalSubHandler(CConfigHandler *master,
			   mediate_project_instrumental_t *instrumental);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 protected:
  bool helperLoadLogger(const QXmlAttributes &atts, struct instrumental_logger *d);
  bool helperLoadSaoz(const QXmlAttributes &atts, struct instrumental_saoz *d);
  bool helperLoadMinimum(const QXmlAttributes &atts, struct instrumental_minimum *d);
  bool helperLoadCcd(const QXmlAttributes &atts, struct instrumental_ccd *d);
  bool helperLoadGdp(const QXmlAttributes &atts, struct instrumental_gdp *d);
  bool helperLoadGome2(const QXmlAttributes &atts, struct instrumental_gome2 *d);
  bool helperLoadScia(const QXmlAttributes &atts, struct instrumental_scia *d);


 private:
  mediate_project_instrumental_t *m_instrumental;
};

//-------------------------------------------------------------------

class CProjectSlitSubHandler : public CConfigSubHandler
{
 public:
  CProjectSlitSubHandler(CConfigHandler *master,
			 mediate_project_slit_t *slit);

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  mediate_project_slit_t *m_slit;
};

//-------------------------------------------------------------------

class CProjectOutputSubHandler : public CSelectorSubHandler
{
 public:
  CProjectOutputSubHandler(CConfigHandler *master,
			   mediate_project_output_t *output);

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_project_output_t *m_output;
};

//-------------------------------------------------------------------

class CProjectExportSubHandler : public CSelectorSubHandler
{
 public:
  CProjectExportSubHandler(CConfigHandler *master,
			   mediate_project_export_t *exportSpectra);

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_project_export_t *m_export;
};

//-------------------------------------------------------------------



#endif

