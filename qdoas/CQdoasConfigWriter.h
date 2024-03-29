/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _CQDOASCONFIGWRITER_H_GUARD
#define _CQDOASCONFIGWRITER_H_GUARD

#include <cstdio>

#include <QTreeWidgetItem>

#include "mediate.h"

class CWProjectTree;

class CQdoasConfigWriter
{
 public:
  CQdoasConfigWriter(const CWProjectTree *projectTree) : m_projectTree(projectTree) {};

  QString write(const QString &fileName);

 private:
  void writeProjects(FILE *fp);
  void writeProperties(FILE *fp, const mediate_project_t *properties);
  void writePropertiesDisplay(FILE *fp, const mediate_project_display_t *d);
  void writePropertiesSelection(FILE *fp, const mediate_project_selection_t *d);
  void writePropertiesAnalysis(FILE *fp, const mediate_project_analysis_t *d);
  void writePropertiesCalibration(FILE *fp, const mediate_project_calibration_t *d);
  void writePropertiesUndersampling(FILE *fp, const mediate_project_undersampling_t *d);
  void writePropertiesInstrumental(FILE *fp, const mediate_project_instrumental_t *d);
  void writePropertiesSlit(FILE *fp, const mediate_project_slit_t *d);
  void writePropertiesOutput(FILE *fp, const mediate_project_output_t *d);
  void writePropertiesExport(FILE *fp, const mediate_project_export_t *d);

  void writeRawSpectraTree(FILE *fp, const QTreeWidgetItem *rawSpectraItem);
  void writeSpectraTreeNode(FILE *fp, const QTreeWidgetItem *item, int depth);

  void writeAnalysisWindows(FILE *fp, const QString &projectName, const QTreeWidgetItem *item);

  void writePolyType(FILE *fp, const char *attr, int type);

  void writeCrossSectionList(FILE *fp, const cross_section_list_t *d);
  void writeLinear(FILE *fp, const struct anlyswin_linear *d);
  void writeNonLinear(FILE *fp, const struct anlyswin_nonlinear *d);
  void writeShiftStretchList(FILE *fp, const shift_stretch_list_t *d);
  void writeGapList(FILE *fp, const gap_list_t *d);
  void writeOutputList(FILE *fp, const output_list_t *d);
  void writeSfps(FILE *fp, const struct calibration_sfp *d);
  void writeDataSelectList(FILE *fp, const data_select_list_t *d);

 private:
  const CWProjectTree *m_projectTree;
};

#endif
