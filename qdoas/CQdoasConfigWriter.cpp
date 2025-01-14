/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QTextStream>

#include "CPathMgr.h"
#include "CWorkSpace.h"
#include "CWProjectTree.h"
#include "CQdoasConfigWriter.h"
#include "ConfigWriterUtils.h"

#include "constants.h"

#include "debugutil.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

QString CQdoasConfigWriter::write(const QString &fileName)
{
  QString msg;
  FILE *fp = fopen(fileName.toLocal8Bit().constData(), "w");

  if (fp == NULL) {
    QTextStream stream(&msg);

    stream << "Failed to open file '" << fileName << "' for writing.";
    return msg;
  }

  // fp is open for writing ...
  int i, n;
  CPathMgr *pathMgr = CPathMgr::instance();
  CWorkSpace *ws = CWorkSpace::instance();

  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<qdoas>\n");

  // paths
  fprintf(fp, "  <paths>\n"
      "    <!-- upto 10 paths can be specified (index 0 to 9). Any file or directory name in the  -->\n"
      "    <!-- raw_spectra tree that begins with %%? (where ? is a single digit) is expanded with -->\n"
      "    <!-- the correponding path.                                                            -->\n\n");

  for (i=0; i<10; ++i) {
    QString path = pathMgr->path(i);
    if (!path.isNull()) {
      fprintf(fp, "    <path index=\"%d\">%s</path>\n", i, path.toUtf8().constData());
    }
  }
  fprintf(fp, "  </paths>\n");

  // sites
  const mediate_site_t *siteList = ws->siteList(n);
  if (siteList != NULL) {
    fprintf(fp, "  <sites>\n");
    for (i=0; i<n; ++i) {
      fprintf(fp, "    <site name=\"%s\" abbrev=\"%s\" long=\"%.3f\" lat=\"%.3f\" alt=\"%.3f\" />\n",
          siteList[i].name, siteList[i].abbreviation, siteList[i].longitude,
          siteList[i].latitude, siteList[i].altitude);
    }
    fprintf(fp, "  </sites>\n");
    delete [] siteList;
  }

  // symbols
  const mediate_symbol_t *symbolList = ws->symbolList(n);
  if (symbolList != NULL) {
    fprintf(fp, "  <symbols>\n");
    for (i=0; i<n; ++i) {
      fprintf(fp, "    <symbol name=\"%s\" descr=\"%s\" />\n",
          symbolList[i].name, symbolList[i].description);
    }
    fprintf(fp, "  </symbols>\n");
    delete [] symbolList;
  }


  writeProjects(fp);

  fprintf(fp, "</qdoas>\n");

  if (fclose(fp)) {
    QTextStream stream(&msg);

    stream << "Error writing to the project file '" << fileName << "'";
  }

  return msg;
}

void CQdoasConfigWriter::writeProjects(FILE *fp)
{
  const QTreeWidgetItem *item;
  const CProjectItem *projItem;
  const mediate_project_t *properties;
  QString projName;

  int n = m_projectTree->topLevelItemCount();
  int i = 0;

  while (i < n) {
    projItem = dynamic_cast<const CProjectItem*>(m_projectTree->topLevelItem(i));
    if (projItem != NULL && projItem->childCount() == 2) {
      // Should always be a project item ... with two children
      projName = projItem->text(0);

      properties = CWorkSpace::instance()->findProject(projName.toStdString());
      if (properties != NULL) {
    // write the project data
    fprintf(fp, "  <project name=\"%s\" disable=\"%s\">\n", projName.toUtf8().constData(),
        (projItem->isEnabled() ? sFalse : sTrue));
    writeProperties(fp, properties);

    // Analysis Windows ...
    item = projItem->child(1); // Analysis Windows Branch
    writeAnalysisWindows(fp, projName, item);

    // Raw spectra ...
    item = projItem->child(0); // Raw Spectra Branch
    writeRawSpectraTree(fp, item);

    fprintf(fp, "  </project>\n");
      }
    }
    ++i;
  }
}

void CQdoasConfigWriter::writeProperties(FILE *fp, const mediate_project_t *d)
{
  writePropertiesDisplay(fp, &(d->display));
  writePropertiesSelection(fp, &(d->selection));
  writePropertiesAnalysis(fp, &(d->analysis));
  writeFilter(fp, 4, "low", &(d->lowpass));
  writeFilter(fp, 4, "high", &(d->highpass));
  writePropertiesCalibration(fp, &(d->calibration));
  writePropertiesUndersampling(fp, &(d->undersampling));
  writePropertiesInstrumental(fp, &(d->instrumental));
  writePropertiesSlit(fp, &(d->slit));
  writePropertiesOutput(fp, &(d->output));
  writePropertiesExport(fp, &(d->export_spectra));
}

void CQdoasConfigWriter::writePropertiesDisplay(FILE *fp, const mediate_project_display_t *d)
{
  fprintf(fp, "    <display spectra=\"%s\" data=\"%s\" calib=\"%s\" fits=\"%s\">\n",
      (d->requireSpectra ? sTrue : sFalse), (d->requireData ? sTrue : sFalse), (d->requireCalib ? sTrue : sFalse), (d->requireFits ? sTrue : sFalse));

  writeDataSelectList(fp, &(d->selection));

  fprintf(fp, "    </display>\n");

}

void CQdoasConfigWriter::writePropertiesSelection(FILE *fp, const mediate_project_selection_t *d)
{
  fprintf(fp, "    <selection>\n");
  fprintf(fp, "      <sza min=\"%.3f\" max=\"%.3f\" delta=\"%.3f\" />\n", d->szaMinimum, d->szaMaximum, d->szaDelta);
  fprintf(fp, "      <elevation min=\"%.3f\" max=\"%.3f\" tol=\"%.3f\" />\n", d->elevationMinimum, d->elevationMaximum,d->elevationTolerance);
  fprintf(fp, "      <reference angle=\"%.3f\" tol=\"%.3f\" />\n", d->refAngle, d->refTolerance);
  fprintf(fp, "      <record min=\"%d\" max=\"%d\" />\n", d->recordNumberMinimum, d->recordNumberMaximum);
  fprintf(fp, "      <cloud min=\"%.3f\" max=\"%.3f\" />\n", d->cloudFractionMinimum, d->cloudFractionMaximum);
  switch (d->geo.mode) {
  case PRJCT_SPECTRA_MODES_CIRCLE:
    fprintf(fp, "      <geolocation selected=\"circle\">\n");
    break;
  case PRJCT_SPECTRA_MODES_RECTANGLE:
    fprintf(fp, "      <geolocation selected=\"rectangle\">\n");
    break;
  case PRJCT_SPECTRA_MODES_OBSLIST:
    fprintf(fp, "      <geolocation selected=\"sites\">\n");
    break;
  default:
     fprintf(fp, "      <geolocation selected=\"none\">\n");
  }
  fprintf(fp, "        <circle radius=\"%.3f\" long=\"%.3f\" lat=\"%.3f\" />\n",
      d->geo.circle.radius, d->geo.circle.centerLongitude, d->geo.circle.centerLatitude);
  fprintf(fp, "        <rectangle west=\"%.3f\" east=\"%.3f\" south=\"%.3f\" north=\"%.3f\" />\n",
      d->geo.rectangle.westernLongitude, d->geo.rectangle.easternLongitude,
      d->geo.rectangle.southernLatitude, d->geo.rectangle.northernLatitude);
  fprintf(fp, "        <sites radius=\"%.3f\" />\n", d->geo.sites.radius);
  fprintf(fp, "      </geolocation>\n    </selection>\n");

}

void CQdoasConfigWriter::writePropertiesAnalysis(FILE *fp, const mediate_project_analysis_t *d)
{
  fprintf(fp, "    <analysis method=");
  switch (d->methodType) {
  case OPTICAL_DENSITY_FIT:
    fprintf(fp, "\"ODF\"");
    break;
  case INTENSITY_FIT:
    fprintf(fp, "\"ML+SVD\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " fit=");
  switch (d->fitType) {
  case PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL:
    fprintf(fp, "\"instr\"");
    break;
  default:
    fprintf(fp, "\"none\"");
  }

  fprintf(fp, " unit=\"nm\"");         // not used anymore but forced for versions compatibility
  fprintf(fp, " interpolation=");
  switch (d->interpolationType) {
  case PRJCT_ANLYS_INTERPOL_LINEAR:
    fprintf(fp, "\"linear\"");
    break;
  case PRJCT_ANLYS_INTERPOL_SPLINE:
    fprintf(fp, "\"spline\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " gap=\"%d\" converge=\"%g\" max_iterations=\"%d\" spike_tolerance=\"%g\" >\n",
      d->interpolationSecurityGap,
      d->convergenceCriterion,
      d->maxIterations,
      d->spike_tolerance);
  fprintf(fp,
      "      <!-- method        : ODF ML+SVD -->\n"
      "      <!-- fit           : none instr -->\n"
      "      <!-- unit          : pixel nm -->\n"
      "      <!-- interpolation : linear spline -->\n"
      "    </analysis>\n");
}

void CQdoasConfigWriter::writePropertiesCalibration(FILE *fp, const mediate_project_calibration_t *d)
{
  fprintf(fp, "    <calibration ref=\"%s\" method=", d->solarRefFile);
  switch (d->methodType) {
  case OPTICAL_DENSITY_FIT:
    fprintf(fp, "\"ODF\"");
    break;
  case INTENSITY_FIT:
    fprintf(fp, "\"ML+SVD\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, ">\n      <line shape=");
  switch (d->lineShape) {
  case PRJCT_CALIB_FWHM_TYPE_FILE:
    fprintf(fp, "\"file\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_GAUSS:
    fprintf(fp, "\"gauss\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_ERF:
    fprintf(fp, "\"error\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_AGAUSS:
    fprintf(fp, "\"agauss\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_SUPERGAUSS:
    fprintf(fp, "\"supergauss\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_INVPOLY:
    fprintf(fp, "\"lorentz\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_VOIGT:
    fprintf(fp, "\"voigt\"");
    break;
  default:
    fprintf(fp, "\"none\"");
  }
  fprintf(fp, " lorentzorder=\"%d\"", d->lorentzOrder);
  fprintf(fp, " slfFile=\"%s\" />\n", d->slfFile);

  fprintf(fp, "      <display spectra=\"%s\" fits=\"%s\" residual=\"%s\" shiftsfp=\"%s\" />\n",
      (d->requireSpectra ? sTrue : sFalse), (d->requireFits ? sTrue : sFalse),
      (d->requireResidual ? sTrue : sFalse), (d->requireShiftSfp ? sTrue : sFalse));
  fprintf(fp, "      <polynomial shift=\"%d\" sfp=\"%d\" />\n", d->shiftDegree, d->sfpDegree);
  fprintf(fp, "      <window min=\"%.1f\" max=\"%.1f\" intervals=\"%d\" custom_windows=\"",
      d->wavelengthMin, d->wavelengthMax, d->subWindows);

     for (int i=0;i<d->subWindows;i++)
      fprintf(fp,"%.2lf-%.2lf,",d->customLambdaMin[i],d->customLambdaMax[i]);

  fprintf(fp,"\" division=");

  switch (d->divisionMode) {
  case PRJCT_CALIB_WINDOWS_CONTIGUOUS:
    fprintf(fp, "\"contiguous\"");
    break;
  case PRJCT_CALIB_WINDOWS_SLIDING:
    fprintf(fp, "\"sliding\"");
    break;
  case PRJCT_CALIB_WINDOWS_CUSTOM:
    fprintf(fp, "\"custom\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
     fprintf(fp," size=\"%.2f\" />\n",d->windowSize);


     fprintf(fp, "      <preshift calculate=\"%s\" min=\"%.1f\" max=\"%.1f\" />\n",(d->preshiftFlag ? sTrue : sFalse),d->preshiftMin, d->preshiftMax);

  writeCrossSectionList(fp, &(d->crossSectionList));
  writeSfps(fp, &(d->sfp[0]));
  writeLinear(fp, &(d->linear));
  writeShiftStretchList(fp, &(d->shiftStretchList));
  writeGapList(fp, &(d->gapList));
  writeOutputList(fp, &(d->outputList));

  fprintf(fp, "    </calibration>\n");
}

void CQdoasConfigWriter::writePropertiesUndersampling(FILE *fp, const mediate_project_undersampling_t *d)
{
  fprintf(fp, "    <undersampling ref=\"%s\" method=", d->solarRefFile);
  switch (d->method) {
  case PRJCT_USAMP_FILE:
    fprintf(fp, "\"file\"");
    break;
  case PRJCT_USAMP_FIXED:
    fprintf(fp, "\"fixed\"");
    break;
  case PRJCT_USAMP_AUTOMATIC:
    fprintf(fp, "\"auto\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " shift=\"%f\" />\n", d->shift);
}

void CQdoasConfigWriter::writePropertiesInstrumental(FILE *fp, const mediate_project_instrumental_t *d)
{
  int i;
  QString tmpStr;
  CPathMgr *pathMgr = CPathMgr::instance();

  fprintf(fp, "    <instrumental format=");
  switch (d->format) {
  case PRJCT_INSTR_FORMAT_ASCII:
    fprintf(fp, "\"ascii\"");
    break;
#ifdef PRJCT_INSTR_FORMAT_OLD
  case PRJCT_INSTR_FORMAT_LOGGER:
    fprintf(fp, "\"logger\"");
    break;
  case PRJCT_INSTR_FORMAT_ACTON:
    fprintf(fp, "\"acton\"");
    break;
  case PRJCT_INSTR_FORMAT_PDAEGG:
    fprintf(fp, "\"pdaegg\"");
    break;
  case PRJCT_INSTR_FORMAT_PDAEGG_OLD:
    fprintf(fp, "\"pdaeggold\"");
    break;
  case PRJCT_INSTR_FORMAT_CCD_OHP_96:
    fprintf(fp, "\"ccdohp96\"");
    break;
  case PRJCT_INSTR_FORMAT_CCD_HA_94:
    fprintf(fp, "\"ccdha94\"");
    break;
#endif
  case PRJCT_INSTR_FORMAT_SAOZ_VIS:
    fprintf(fp, "\"saozvis\"");
    break;
  case PRJCT_INSTR_FORMAT_SAOZ_EFM:
    fprintf(fp, "\"saozefm\"");
    break;
  case PRJCT_INSTR_FORMAT_MFC:
    fprintf(fp, "\"mfc\"");
    break;
  case PRJCT_INSTR_FORMAT_MFC_STD:
    fprintf(fp, "\"mfcstd\"");
    break;
  case PRJCT_INSTR_FORMAT_MFC_BIRA:
    fprintf(fp, "\"mfcbira\"");
    break;
#ifdef PRJCT_INSTR_FORMAT_OLD
  case PRJCT_INSTR_FORMAT_RASAS:
    fprintf(fp, "\"rasas\"");
    break;
  case PRJCT_INSTR_FORMAT_PDASI_EASOE:
    fprintf(fp, "\"pdasieasoe\"");
    break;
#endif
  case PRJCT_INSTR_FORMAT_CCD_EEV:
    fprintf(fp, "\"ccdeev\"");
    break;
  case PRJCT_INSTR_FORMAT_GDP_BIN:
    fprintf(fp, "\"gdpbin\"");
    break;
  case PRJCT_INSTR_FORMAT_SCIA_PDS:
    fprintf(fp, "\"sciapds\"");
    break;
  case PRJCT_INSTR_FORMAT_UOFT:
    fprintf(fp, "\"uoft\"");
    break;
  case PRJCT_INSTR_FORMAT_NOAA:
    fprintf(fp, "\"noaa\"");
    break;
  case PRJCT_INSTR_FORMAT_OMI:
    fprintf(fp, "\"omi\"");
    break;
  case PRJCT_INSTR_FORMAT_OMIV4:
    fprintf(fp, "\"omiv4\"");
    break;
  case PRJCT_INSTR_FORMAT_OMPS:
    fprintf(fp, "\"omps\"");
    break;
  case PRJCT_INSTR_FORMAT_TROPOMI:
    fprintf(fp, "\"tropomi\"");
    break;
  case PRJCT_INSTR_FORMAT_GOME2:
    fprintf(fp, "\"gome2\"");
    break;
  case PRJCT_INSTR_FORMAT_MKZY:
    fprintf(fp, "\"mkzy\"");
    break;
  case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE:
    fprintf(fp, "\"biraairborne\"");
    break;
  case PRJCT_INSTR_FORMAT_BIRA_MOBILE:
    fprintf(fp,"\"biramobile\"");
    break;
  case PRJCT_INSTR_FORMAT_APEX:
    fprintf(fp, "\"apex\"");
    break;
  case PRJCT_INSTR_FORMAT_GEMS:
    fprintf(fp, "\"gems\"");
    break;
  case PRJCT_INSTR_FORMAT_OCEAN_OPTICS:
    fprintf(fp, "\"oceanoptics\"");
    break;
  case PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF:
    fprintf(fp, "\"frm4doas\"");
    break;
  case PRJCT_INSTR_FORMAT_GOME1_NETCDF:
    fprintf(fp, "\"gdpnetcdf\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " site=\"%s\" saa_convention=\"%s\" >\n", d->siteName,(d->saaConvention)?"0-north":"0-south");

  // ascii
  fprintf(fp, "      <ascii size=\"%d\" format=", d->ascii.detectorSize);
  switch (d->ascii.format) {
  case PRJCT_INSTR_ASCII_FORMAT_LINE:
    fprintf(fp, "\"line\"");
    break;
  case PRJCT_INSTR_ASCII_FORMAT_COLUMN:
    fprintf(fp, "\"column\"");
    break;
  case PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED:
    fprintf(fp, "\"column_extended\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " zen=\"%s\" azi=\"%s\" ele=\"%s\" date=\"%s\" time=\"%s\" lambda=\"%s\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",
      (d->ascii.flagZenithAngle ? sTrue : sFalse), (d->ascii.flagAzimuthAngle ? sTrue : sFalse),
      (d->ascii.flagElevationAngle ? sTrue : sFalse), (d->ascii.flagDate ? sTrue : sFalse),
      (d->ascii.flagTime ? sTrue : sFalse), (d->ascii.flagWavelength ? sTrue : sFalse),
      (d->ascii.straylight ? sTrue : sFalse),d->ascii.lambdaMin,d->ascii.lambdaMax);

  tmpStr = pathMgr->simplifyPath(QString(d->ascii.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ascii.transmissionFunctionFile));
  fprintf(fp, " transmission=\"%s\" />\n", tmpStr.toUtf8().constData());

#ifdef PRJCT_INSTR_FORMAT_OLD

  // logger
  fprintf(fp, "      <logger type=");
  switch (d->logger.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->logger.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->logger.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->logger.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // acton
  fprintf(fp, "      <acton type=");
  switch (d->acton.niluType) {
  case PRJCT_INSTR_NILU_FORMAT_OLD:
    fprintf(fp, "\"old\"");
    break;
  case PRJCT_INSTR_NILU_FORMAT_NEW:
    fprintf(fp, "\"new\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = pathMgr->simplifyPath(QString(d->acton.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->acton.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // pdaegg
  fprintf(fp, "      <pdaegg type=");
  switch (d->pdaegg.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdaegg.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->pdaegg.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->pdaegg.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // pdaeggold
  fprintf(fp, "      <pdaeggold type=");
  switch (d->pdaeggold.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdaeggold.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->pdaeggold.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->pdaeggold.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // pdaeggold
  fprintf(fp, "      <pdaeggold type=");
  switch (d->pdaeggold.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdaeggold.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->pdaeggold.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->pdaeggold.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // ccdohp96
  tmpStr = pathMgr->simplifyPath(QString(d->ccdohp96.calibrationFile));
  fprintf(fp, "      <ccdohp96 calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdohp96.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdohp96.interPixelVariabilityFile));
  fprintf(fp, " ipv=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdohp96.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toUtf8().constData());

  // ccdha94
  tmpStr = pathMgr->simplifyPath(QString(d->ccdha94.calibrationFile));
  fprintf(fp, "      <ccdha94 calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdha94.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdha94.interPixelVariabilityFile));
  fprintf(fp, " ipv=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdha94.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toUtf8().constData());
#endif

  // saozvis

  fprintf(fp, "      <saozvis type=\"%s\"",(d->saozvis.spectralType==PRJCT_INSTR_SAOZ_TYPE_ZENITHAL)?"zenithal":"pointed");

  tmpStr = pathMgr->simplifyPath(QString(d->saozvis.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->saozvis.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  fprintf(fp, "      <saozefm straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",(d->saozefm.straylight ? sTrue : sFalse),d->saozefm.lambdaMin,d->saozefm.lambdaMax);

  // saozefm
  tmpStr = pathMgr->simplifyPath(QString(d->saozefm.calibrationFile));
  fprintf(fp, "      calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->saozefm.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // mfc
  fprintf(fp, "      <mfc size=\"%d\" first=\"%d\" revert=\"%s\" auto=\"%s\" omask=\"%d\" imask=\"%d\" dmask=\"%d\" smask=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",
      d->mfc.detectorSize, d->mfc.firstWavelength,
      (d->mfc.revert ? sTrue: sFalse), (d->mfc.autoFileSelect ? sTrue : sFalse),
      d->mfc.offsetMask, d->mfc.instrFctnMask, d->mfc.darkCurrentMask, d->mfc.spectraMask,(d->mfc.straylight ? sTrue : sFalse),d->mfc.lambdaMin,d->mfc.lambdaMax);

  tmpStr = pathMgr->simplifyPath(QString(d->mfc.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mfc.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mfc.darkCurrentFile));
  fprintf(fp, " dark=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mfc.offsetFile));
  fprintf(fp, " offset=\"%s\" />\n", tmpStr.toUtf8().constData());

  // mfcbira
  fprintf(fp, "      <mfcbira size=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",d->mfcbira.detectorSize,
  (d->mfcbira.straylight ? sTrue : sFalse),d->mfcbira.lambdaMin,d->mfcbira.lambdaMax);

  tmpStr = pathMgr->simplifyPath(QString(d->mfcbira.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());
  tmpStr = pathMgr->simplifyPath(QString(d->mfcbira.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // mfcstd
  fprintf(fp, "      <mfcstd size=\"%d\" revert=\"%s\" straylight=\"%s\" date=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",
    d->mfcstd.detectorSize, (d->mfcstd.revert ? sTrue : sFalse),
   (d->mfcstd.straylight ? sTrue : sFalse),d->mfcstd.dateFormat,d->mfcstd.lambdaMin,d->mfcstd.lambdaMax);

  tmpStr = pathMgr->simplifyPath(QString(d->mfcstd.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mfcstd.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mfcstd.darkCurrentFile));
  fprintf(fp, " dark=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mfcstd.offsetFile));
  fprintf(fp, " offset=\"%s\" />\n", tmpStr.toUtf8().constData());
#ifdef PRJCT_INSTR_FORMAT_OLD
  // rasas

  fprintf(fp, "      <rasas straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",(d->rasas.straylight ? sTrue : sFalse),d->rasas.lambdaMin,d->rasas.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->rasas.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->rasas.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // pdasieasoe

  fprintf(fp, "      <pdasieasoe straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",(d->pdasieasoe.straylight ? sTrue : sFalse),d->pdasieasoe.lambdaMin,d->pdasieasoe.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->pdasieasoe.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->pdasieasoe.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());
#endif
  // ccdeev
  fprintf(fp, "      <ccdeev size=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"", d->ccdeev.detectorSize,(d->ccdeev.straylight ? sTrue : sFalse),d->ccdeev.lambdaMin,d->ccdeev.lambdaMax);

  tmpStr = pathMgr->simplifyPath(QString(d->ccdeev.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdeev.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdeev.imagePath));
  fprintf(fp, " image=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdeev.straylightCorrectionFile));
  fprintf(fp, " stray=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->ccdeev.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" ", tmpStr.toUtf8().constData());

  fprintf(fp, " type=");
  switch (d->ccdeev.spectralType) {
  case PRJCT_INSTR_MAXDOAS_TYPE_NONE:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN:
    fprintf(fp, "\"direct-sun\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR:
    fprintf(fp, "\"almucantar\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_ZENITH:
    fprintf(fp, "\"zenith-only\"");
    break;    
  default:
    fprintf(fp, "\"invalid\"");
  }

  fprintf(fp," />\n");

  // gdpascii
  fprintf(fp, "      <gdpnetcdf type=");
  switch (d->gdpnetcdf.bandType) {
  case PRJCT_INSTR_GDP_BAND_1A:
    fprintf(fp, "\"1a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_1B:
    fprintf(fp, "\"1b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2A:
    fprintf(fp, "\"2a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2B:
    fprintf(fp, "\"2b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_GDP_BAND_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  fprintf(fp," pixel=");
  switch(d->gdpnetcdf.pixelType)
   {
       case PRJCT_INSTR_GDP_PIXEL_ALL :
        fprintf(fp,"\"all\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_EAST :
        fprintf(fp,"\"east\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_CENTER :
        fprintf(fp,"\"center\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_WEST :
        fprintf(fp,"\"west\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_BACKSCAN :
        fprintf(fp,"\"backscan\"");
       break;

       default:
        fprintf(fp,"\"invalid\"");
       break;
   };

  tmpStr = pathMgr->simplifyPath(QString(d->gdpnetcdf.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->gdpnetcdf.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // gdpbin
  fprintf(fp, "      <gdpbin type=");
  switch (d->gdpbin.bandType) {
  case PRJCT_INSTR_GDP_BAND_1A:
    fprintf(fp, "\"1a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_1B:
    fprintf(fp, "\"1b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2A:
    fprintf(fp, "\"2a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2B:
    fprintf(fp, "\"2b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_GDP_BAND_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  fprintf(fp," pixel=");
  switch(d->gdpbin.pixelType)
   {
       case PRJCT_INSTR_GDP_PIXEL_ALL :
        fprintf(fp,"\"all\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_EAST :
        fprintf(fp,"\"east\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_CENTER :
        fprintf(fp,"\"center\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_WEST :
        fprintf(fp,"\"west\"");
       break;

       case PRJCT_INSTR_GDP_PIXEL_BACKSCAN :
        fprintf(fp,"\"backscan\"");
       break;

       default:
        fprintf(fp,"\"invalid\"");
       break;
   };

  tmpStr = pathMgr->simplifyPath(QString(d->gdpbin.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->gdpbin.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // sciapds
  fprintf(fp, "      <sciapds channel=");
  switch (d->sciapds.channel) {
  case PRJCT_INSTR_SCIA_CHANNEL_1:
    fprintf(fp, "\"1\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_2:
    fprintf(fp, "\"2\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " sunref=\"%s\"", d->sciapds.sunReference);
  for (i=2; i<=5; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=8; i<=10; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=13; i<=18; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=22; i<=27; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }

  tmpStr = pathMgr->simplifyPath(QString(d->sciapds.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->sciapds.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->sciapds.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toUtf8().constData());

  // uoft

  fprintf(fp, "      <uoft straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",(d->uoft.straylight ? sTrue : sFalse),d->uoft.lambdaMin,d->uoft.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->uoft.calibrationFile));
  fprintf(fp, "      calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->uoft.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // noaa
  fprintf(fp, "      <noaa straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",(d->noaa.straylight ? sTrue : sFalse),d->noaa.lambdaMin,d->noaa.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->noaa.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->noaa.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // omi
  fprintf(fp, "      <omi type=");
  switch (d->omi.spectralType) {
  case PRJCT_INSTR_OMI_TYPE_UV1:
    fprintf(fp, "\"uv1\"");
    break;
  case PRJCT_INSTR_OMI_TYPE_UV2:
    fprintf(fp, "\"uv2\"");
    break;
  case PRJCT_INSTR_OMI_TYPE_VIS:
    fprintf(fp, "\"vis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  const char *this_xtrack_mode;
  switch(d->omi.xtrack_mode) {
  case XTRACKQF_STRICT:
    this_xtrack_mode = STR_STRICT;
    break;
  case XTRACKQF_NONSTRICT:
    this_xtrack_mode = STR_NONSTRICT;
    break;
  default:
    this_xtrack_mode = STR_IGNORE;
    break;
  }

  fprintf(fp, " min=\"%.1f\" max=\"%.1f\" ave=\"%s\" trackSelection=\"%s\" pixelQF_rejectionFlag=\"%s\" pixelQF_maxGaps=\"%d\" pixelQF_mask=\"%d\" xTrackMode=\"%s\" />\n",
      d->omi.minimumWavelength, d->omi.maximumWavelength, (d->omi.flagAverage ? sTrue : sFalse),d->omi.trackSelection,
      (d->omi.pixelQFRejectionFlag ? sTrue : sFalse),d->omi.pixelQFMaxGaps,d->omi.pixelQFMask,
          this_xtrack_mode);

  // tropomi
  const char *tropomiSpectralBand;
  switch(d->tropomi.spectralBand) {
    // use macro to generate expression for every value of the
    // tropomiSpectralBand enum:
#define EXPAND(BAND, LABEL)                    \
    case BAND:                                 \
      tropomiSpectralBand = LABEL;             \
      break;
    TROPOMI_BANDS
#undef EXPAND
      }
  fprintf(fp, "      <tropomi band=\"%s\"", tropomiSpectralBand);
  fprintf(fp, " reference_orbit_dir=\"%s\"", pathMgr->simplifyPath(QString(d->tropomi.reference_orbit_dir)).toUtf8().constData());
  fprintf(fp," trackSelection=\"%s\"",d->tropomi.trackSelection);

  tmpStr = pathMgr->simplifyPath(QString(d->tropomi.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->tropomi.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // apex
  fprintf(fp, "      <apex trackSelection=\"%s\"",d->apex.trackSelection);
  tmpStr = pathMgr->simplifyPath(QString(d->apex.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->apex.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // gems
  fprintf(fp, "      <gems trackSelection=\"%s\" binning=\"%d\"",d->gems.trackSelection,d->gems.binning);

  tmpStr = pathMgr->simplifyPath(QString(d->gems.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->gems.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // gome2
  fprintf(fp, "      <gome2 type=");
  switch (d->gome2.bandType) {
  case PRJCT_INSTR_GDP_BAND_1A:
    fprintf(fp, "\"1a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_1B:
    fprintf(fp, "\"1b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2A:
    fprintf(fp, "\"2a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2B:
    fprintf(fp, "\"2b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_GDP_BAND_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = pathMgr->simplifyPath(QString(d->gome2.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->gome2.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // mkzy
  fprintf(fp, "      <mkzy straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",(d->mkzy.straylight ? sTrue : sFalse),d->mkzy.lambdaMin,d->mkzy.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->mkzy.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->mkzy.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // biraairborne
  fprintf(fp, "      <biraairborne size=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",d->biraairborne.detectorSize,
    (d->biraairborne.straylight ? sTrue : sFalse),d->biraairborne.lambdaMin,d->biraairborne.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->biraairborne.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->biraairborne.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // biramobile
  fprintf(fp, "      <biramobile size=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"",d->biramobile.detectorSize,
    (d->biramobile.straylight ? sTrue : sFalse),d->biramobile.lambdaMin,d->biramobile.lambdaMax);
  tmpStr = pathMgr->simplifyPath(QString(d->biramobile.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->biramobile.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // ocean optics

  fprintf(fp, "      <oceanoptics size=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\"", d->oceanoptics.detectorSize,(d->oceanoptics.straylight ? sTrue : sFalse),d->oceanoptics.lambdaMin,d->oceanoptics.lambdaMax);

  tmpStr = pathMgr->simplifyPath(QString(d->oceanoptics.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());

  tmpStr = pathMgr->simplifyPath(QString(d->oceanoptics.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  // frm4doas

  fprintf(fp, "      <frm4doas size=\"%d\" straylight=\"%s\" lambda_min=\"%g\" lambda_max=\"%g\" average_rows=\"%s\"",d->frm4doas.detectorSize,
  (d->frm4doas.straylight ? sTrue : sFalse),d->frm4doas.lambdaMin,d->frm4doas.lambdaMax,(d->frm4doas.averageRows ? sTrue : sFalse));
  tmpStr = pathMgr->simplifyPath(QString(d->frm4doas.calibrationFile));

  fprintf(fp, " type=");
  switch (d->frm4doas.spectralType) {
  case PRJCT_INSTR_MAXDOAS_TYPE_NONE:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_ZENITH:
    fprintf(fp, "\"zenith\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN:
    fprintf(fp, "\"direct-sun\"");
    break;
  case PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR:
    fprintf(fp, "\"almucantar\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  fprintf(fp, " calib=\"%s\"", tmpStr.toUtf8().constData());
  tmpStr = pathMgr->simplifyPath(QString(d->frm4doas.transmissionFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toUtf8().constData());

  fprintf(fp, "    </instrumental>\n");
}

void CQdoasConfigWriter::writePropertiesSlit(FILE *fp, const mediate_project_slit_t *d)
{
  QString tmpStr = CPathMgr::instance()->simplifyPath(QString(d->solarRefFile));

  fprintf(fp, "    <slit ref=\"%s\" fwhmcor=\"%s\">\n", tmpStr.toUtf8().constData(),
      (d->applyFwhmCorrection ? sTrue : sFalse));

  writeSlitFunction(fp, 6, &(d->function));

  fprintf(fp, "    </slit>\n");
}

void CQdoasConfigWriter::writePropertiesOutput(FILE *fp, const mediate_project_output_t *d)
{
  QString tmpStr = CPathMgr::instance()->simplifyPath(QString(d->path));
                                                                 // newcalib=\"%s\"
  fprintf(fp, "    <output path=\"%s\" anlys=\"%s\" calib=\"%s\" ref=\"%s\" conf=\"%s\" dirs=\"%s\" file=\"%s\" success=\"%s\" flux=\"%s\" cic=\" \" bandWidth=\"%f\" swathName=\"%s\" fileFormat=\"%s\">\n",
          tmpStr.toUtf8().constData(), (d->analysisFlag ? sTrue : sFalse),
          (d->calibrationFlag ? sTrue : sFalse),
          // (d->newcalibFlag ? sTrue : sFalse),
          (d->referenceFlag ? sTrue : sFalse),
          (d->configurationFlag ? sTrue : sFalse),
          (d->directoryFlag ? sTrue : sFalse),
          (d->filenameFlag ? sTrue : sFalse),
          (d->successFlag ? sTrue : sFalse),
          d->flux, d->bandWidth,
          d->swath_name,
          output_file_extensions[d->file_format]);

  writeDataSelectList(fp, &(d->selection));

  fprintf(fp, "    </output>\n");
}

void CQdoasConfigWriter::writePropertiesExport(FILE *fp, const mediate_project_export_t *d)
{
  QString tmpStr = CPathMgr::instance()->simplifyPath(QString(d->path));

  fprintf(fp, "    <export path=\"%s\" titles=\"%s\" dir=\"%s\" >\n",
          tmpStr.toUtf8().constData(),
          (d->titlesFlag ? sTrue : sFalse),(d->directoryFlag ? sTrue : sFalse));

  writeDataSelectList(fp, &(d->selection));

  fprintf(fp, "    </export>\n");
}

void CQdoasConfigWriter::writeRawSpectraTree(FILE *fp, const QTreeWidgetItem *rawSpectraItem)
{
  fprintf(fp, "    <raw_spectra>\n"
      "      <!-- Disable file, folder and directory items with the disable set equal to \"true\". -->\n"
      "      <!--  The default is enabled.                                                       -->\n");

  QTreeWidgetItem *item;
  int nChildren = rawSpectraItem->childCount();
  int i = 0;

  while (i < nChildren) {
    item = rawSpectraItem->child(i);
    writeSpectraTreeNode(fp, item, 3);
    ++i;
  }

  fprintf(fp, "    </raw_spectra>\n");
}

void CQdoasConfigWriter::writeSpectraTreeNode(FILE *fp, const QTreeWidgetItem *item, int depth)
{
  int i;

  switch (item->type()) {
  case cSpectraFolderItemType:
    {
      const CSpectraFolderItem *folderItem = static_cast<const CSpectraFolderItem*>(item);
      int nChildren = item->childCount();

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "<folder name=\"%s\"", item->text(0).toUtf8().constData());
      if (folderItem->isEnabled())
    fprintf(fp, ">\n");
      else
    fprintf(fp, " disable=\"true\">\n");

      // now all children ...
      i = 0;
      while (i < nChildren) {
    writeSpectraTreeNode(fp, item->child(i), depth + 1);
    ++i;
      }

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "</folder>\n");
    }
    break;
  case cSpectraDirectoryItemType:
    {
      const CSpectraDirectoryItem *dirItem = static_cast<const CSpectraDirectoryItem*>(item);
      QString dirName = CPathMgr::instance()->simplifyPath(dirItem->directoryName());

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "<directory name=\"%s\" filters=\"%s\" recursive=\"%s\"",
          dirName.toUtf8().constData(),
          dirItem->fileFilters().toUtf8().constData(),
          (dirItem->isRecursive() ? sTrue : sFalse));
      if (dirItem->isEnabled())
    fprintf(fp, " />\n");
      else
    fprintf(fp, " disable=\"true\" />\n");
    }
    break;
  case cSpectraFileItemType:
    {
      const CSpectraFileItem *fileItem = static_cast<const CSpectraFileItem*>(item);
      QString fileName = CPathMgr::instance()->simplifyPath(fileItem->file().filePath());

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "<file name=\"%s\"", fileName.toUtf8().constData());
      if (fileItem->isEnabled())
    fprintf(fp, " />\n");
      else
    fprintf(fp, " disable=\"true\" />\n");
    }
    break;
  }
}

void CQdoasConfigWriter::writeAnalysisWindows(FILE *fp, const QString &projectName, const QTreeWidgetItem *item)
{
  const CAnalysisWindowItem *awItem;
  const mediate_analysis_window_t *properties;
  QString awName, tmpStr;

  CPathMgr *pathMgr = CPathMgr::instance();

  int n = item->childCount();
  int i = 0;

  while (i < n) {
    awItem = dynamic_cast<const CAnalysisWindowItem*>(item->child(i));
    if (awItem != NULL) {
      awName = awItem->text(0);

      properties = CWorkSpace::instance()->findAnalysisWindow(projectName.toStdString(), awName.toStdString());
      if (properties != NULL) {

 fprintf(fp, "    <analysis_window name=\"%s\" disable=\"%s\" kurucz=", awName.toUtf8().constData(),
  (awItem->isEnabled() ? sFalse : sTrue));

 switch (properties->kuruczMode) {
 case ANLYS_KURUCZ_REF:
   fprintf(fp, "\"ref\""); break;
 case ANLYS_KURUCZ_SPEC:
   fprintf(fp, "\"spec\""); break;
 case ANLYS_KURUCZ_REF_AND_SPEC:
   fprintf(fp, "\"ref+spec\""); break;
 default:
   fprintf(fp, "\"none\"");
 }

 if (properties->refSpectrumSelection == ANLYS_REF_SELECTION_MODE_AUTOMATIC)
   fprintf(fp, " refsel=\"auto\"");
 else
   fprintf(fp, " refsel=\"file\"");

 fprintf(fp, " min=\"%.3f\" max=\"%.3f\" resol_fwhm=\"%.3f\" lambda0=\"%.3f\" >\n", properties->fitMinWavelength, properties->fitMaxWavelength, properties->resolFwhm,properties->lambda0);

 fprintf(fp, "      <display spectrum=\"%s\" poly=\"%s\" fits=\"%s\" residual=\"%s\" predef=\"%s\" ratio=\"%s\" />\n",
  (properties->requireSpectrum ? sTrue : sFalse),
  (properties->requirePolynomial ? sTrue : sFalse),
  (properties->requireFit ? sTrue : sFalse),
  (properties->requireResidual ? sTrue : sFalse),
  (properties->requirePredefined ? sTrue : sFalse),
  (properties->requireRefRatio ? sTrue : sFalse));

 tmpStr = pathMgr->simplifyPath(QString(properties->refOneFile));
 fprintf(fp, "      <files refone=\"%s\"\n", tmpStr.toUtf8().constData());
 tmpStr = pathMgr->simplifyPath(QString(properties->refTwoFile));
 fprintf(fp, "             reftwo=\"%s\"\n", tmpStr.toUtf8().constData());
 tmpStr = pathMgr->simplifyPath(QString(properties->residualFile));

 fprintf(fp, "             residual=\"%s\"\nsaveresiduals=\"%s\" szacenter=\"%.3f\" szadelta=\"%.3f\" scanmode=",tmpStr.toUtf8().constData(),(properties->saveResidualsFlag ? sTrue : sFalse),properties->refSzaCenter , properties->refSzaDelta);

 switch(properties->refSpectrumSelectionScanMode)
  {
   case ANLYS_MAXDOAS_REF_SCAN_BEFORE :
    fprintf(fp,"\"before\" ");
   break;

   case ANLYS_MAXDOAS_REF_SCAN_AVERAGE :
    fprintf(fp,"\"average\" ");
   break;

   case ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE :
    fprintf(fp,"\"interpolate\" ");
   break;

   default :
    fprintf(fp,"\"after\" ");
   break;
  }

        fprintf(fp,"minlon=\"%.3f\" maxlon=\"%.3f\" minlat=\"%.3f\" maxlat=\"%.3f\" refns=\"%d\" cloudfmin=\"%.3f\" cloudfmax=\"%.3f\" \n",
                properties->refMinLongitude, properties->refMaxLongitude,
                properties->refMinLatitude, properties->refMaxLatitude, properties->refNs,
                properties->cloudFractionMin,properties->cloudFractionMax);

        fprintf(fp,"              maxdoasrefmode=\"%s\" \n",
                (properties->refMaxdoasSelection==ANLYS_MAXDOAS_REF_SCAN)?"scan":"sza");
        // for backwards compatibility, we still write the GOME pixeltype selection configuration, defaulting to "true" for all types.
        fprintf(fp, "             east=\"true\" center=\"true\" west=\"true\" backscan=\"true\" />\n");

    // cross sections ....
    writeCrossSectionList(fp, &(properties->crossSectionList));

    // linear
    writeLinear(fp, &(properties->linear));

    // nonlinear
    writeNonLinear(fp, &(properties->nonlinear));

    // shift and stretch
    writeShiftStretchList(fp, &(properties->shiftStretchList));

    // gaps...
    writeGapList(fp, &(properties->gapList));

    // output...
    writeOutputList(fp, &(properties->outputList));

    fprintf(fp, "    </analysis_window>\n");
      }
    }

    ++i;
  }
}

void CQdoasConfigWriter::writePolyType(FILE *fp, const char *attr, int type)
{
  switch (type) {
  case ANLYS_POLY_TYPE_0: fprintf(fp, " %s=\"0\"", attr); break;
  case ANLYS_POLY_TYPE_1: fprintf(fp, " %s=\"1\"", attr); break;
  case ANLYS_POLY_TYPE_2: fprintf(fp, " %s=\"2\"", attr); break;
  case ANLYS_POLY_TYPE_3: fprintf(fp, " %s=\"3\"", attr); break;
  case ANLYS_POLY_TYPE_4: fprintf(fp, " %s=\"4\"", attr); break;
  case ANLYS_POLY_TYPE_5: fprintf(fp, " %s=\"5\"", attr); break;
  case ANLYS_POLY_TYPE_6: fprintf(fp, " %s=\"6\"", attr); break;
  case ANLYS_POLY_TYPE_7: fprintf(fp, " %s=\"7\"", attr); break;
  case ANLYS_POLY_TYPE_8: fprintf(fp, " %s=\"8\"", attr); break;
  default: fprintf(fp, " %s=\"none\"", attr);
  }
}

void CQdoasConfigWriter::writeCrossSectionList(FILE *fp, const cross_section_list_t *data)
{
  QString tmpStr;
  CPathMgr *pathMgr = CPathMgr::instance();
  const struct anlyswin_cross_section *d = &(data->crossSection[0]);
  int j = 0;

  fprintf(fp, "      <cross_sections>\n");

  while (j < data->nCrossSection) {
    fprintf(fp, "        <cross_section sym=\"%s\" ortho=\"%s\" subtract=\"%s\" subtract_flag=\"%s\" cstype=",
        d->symbol,
        d->orthogonal,
        d->subtract,
       (d->subtractFlag)?"true":"false");

    switch (d->crossType) {
    case ANLYS_CROSS_ACTION_INTERPOLATE:
      fprintf(fp, "\"interp\""); break;
    case ANLYS_CROSS_ACTION_CONVOLUTE:
      fprintf(fp, "\"std\""); break;
    case ANLYS_CROSS_ACTION_CONVOLUTE_I0:
      fprintf(fp, "\"io\""); break;
    case ANLYS_CROSS_ACTION_CONVOLUTE_RING:
      fprintf(fp, "\"ring\""); break;
    default:
      fprintf(fp, "\"none\"");
    }
    fprintf(fp, " amftype=");
    switch (d->amfType) {
    case ANLYS_AMF_TYPE_SZA:
      fprintf(fp, "\"sza\""); break;
    case ANLYS_AMF_TYPE_CLIMATOLOGY:
      fprintf(fp, "\"climate\""); break;
    case ANLYS_AMF_TYPE_WAVELENGTH:
      fprintf(fp, "\"wave\""); break;
    default:
      fprintf(fp, "\"none\"");
    }

    fprintf(fp, " corrtype=");
    switch (d->correctionType) {
    case ANLYS_CORRECTION_TYPE_SLOPE:
      fprintf(fp, "\"slope\""); break;
    case ANLYS_CORRECTION_TYPE_PUKITE:
      fprintf(fp, "\"pukite\""); break;
    case ANLYS_CORRECTION_TYPE_MOLECULAR_RING:
      fprintf(fp, "\"molecular_ring\""); break;
    case ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE:
      fprintf(fp, "\"molecular_ring_slope\""); break;
    default:
      fprintf(fp, "\"none\"");
    }

    fprintf(fp, " molecular_xs=\"%s\"",d->molecularRing);

    fprintf(fp, " fit=\"%s\" filter=\"%s\" cstrncc=\"%s\" ccfit=\"%s\" icc=\"%.3f\" dcc=\"%.3f\" ccio=\"%.3e\" ccmin=\"%.3e\" ccmax=\"%.3e\"",
        (d->requireFit ? sTrue : sFalse),
        (d->requireFilter ? sTrue : sFalse),
        (d->constrainedCc ? sTrue : sFalse),
        (d->requireCcFit ? sTrue : sFalse),
        d->initialCc, d->deltaCc, d->ccIo,d->ccMin,d->ccMax);
    tmpStr = pathMgr->simplifyPath(QString(d->crossSectionFile)); // TODO: use fromUtf8() here...
    fprintf(fp, " csfile=\"%s\"", tmpStr.toUtf8().constData());
    tmpStr = pathMgr->simplifyPath(QString(d->amfFile));
    fprintf(fp, " amffile=\"%s\" />\n", tmpStr.toUtf8().constData());

    ++d;
    ++j;
  }
  fprintf(fp, "      </cross_sections>\n");
}

void CQdoasConfigWriter::writeLinear(FILE *fp, const struct anlyswin_linear *d)
{
  fprintf(fp, "      <linear");
  writePolyType(fp, "xpoly", d->xPolyOrder);
  writePolyType(fp, "xbase", d->xBaseOrder);
  fprintf(fp, " xfit=\"%s\" xerr=\"%s\"",
      (d->xFlagFitStore ? sTrue : sFalse),
      (d->xFlagErrStore ? sTrue : sFalse));
  writePolyType(fp, "offpoly", d->offsetPolyOrder);
  fprintf(fp, " offfit=\"%s\" offerr=\"%s\" offizero=\"%s\"",
      (d->offsetFlagFitStore ? sTrue : sFalse),
      (d->offsetFlagErrStore ? sTrue : sFalse),
          (d->offsetI0 ? sTrue : sFalse) );
  fprintf(fp, " />\n");
}

void CQdoasConfigWriter::writeNonLinear(FILE *fp, const struct anlyswin_nonlinear *d)
{
  QString tmpStr;
  CPathMgr *pathMgr = CPathMgr::instance();

  fprintf(fp, "      <nonlinear solfit=\"%s\" solinit=\"%.3f\" soldelt=\"%.3f\" solfstr=\"%s\" solestr=\"%s\"\n",
      (d->solFlagFit ? sTrue : sFalse), d->solInitial, d->solDelta,
      (d->solFlagFitStore ? sTrue : sFalse), (d->solFlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 o0fit=\"%s\" o0init=\"%.3f\" o0delt=\"%.3f\" o0fstr=\"%s\" o0estr=\"%s\"\n",
      (d->off0FlagFit ? sTrue : sFalse), d->off0Initial, d->off0Delta,
      (d->off0FlagFitStore ? sTrue : sFalse), (d->off0FlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 o1fit=\"%s\" o1init=\"%.3f\" o1delt=\"%.3f\" o1fstr=\"%s\" o1estr=\"%s\"\n",
      (d->off1FlagFit ? sTrue : sFalse), d->off1Initial, d->off1Delta,
      (d->off1FlagFitStore ? sTrue : sFalse), (d->off1FlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 o2fit=\"%s\" o2init=\"%.3f\" o2delt=\"%.3f\" o2fstr=\"%s\" o2estr=\"%s\"\n",
      (d->off2FlagFit ? sTrue : sFalse), d->off2Initial, d->off2Delta,
      (d->off2FlagFitStore ? sTrue : sFalse), (d->off2FlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 comfit=\"%s\" cominit=\"%.3f\" comdelt=\"%.3f\" comstr=\"%s\" comestr=\"%s\"\n",
      (d->comFlagFit ? sTrue : sFalse), d->comInitial, d->comDelta,
      (d->comFlagFitStore ? sTrue : sFalse), (d->comFlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 u1fit=\"%s\" u1init=\"%.3f\" u1delt=\"%.3f\" u1str=\"%s\" u1estr=\"%s\"\n",
      (d->usamp1FlagFit ? sTrue : sFalse), d->usamp1Initial, d->usamp1Delta,
      (d->usamp1FlagFitStore ? sTrue : sFalse), (d->usamp1FlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 u2fit=\"%s\" u2init=\"%.3f\" u2delt=\"%.3f\" u2str=\"%s\" u2estr=\"%s\"\n",
      (d->usamp2FlagFit ? sTrue : sFalse), d->usamp2Initial, d->usamp2Delta,
      (d->usamp2FlagFitStore ? sTrue : sFalse), (d->usamp2FlagErrStore ? sTrue : sFalse));

  fprintf(fp, "                 resolfit=\"%s\" resolinit=\"%.3f\" resoldelt=\"%.3f\" resolstr=\"%s\" resolestr=\"%s\"\n",
      (d->resolFlagFit ? sTrue : sFalse), d->resolInitial, d->resolDelta,
      (d->resolFlagFitStore ? sTrue : sFalse), (d->resolFlagErrStore ? sTrue : sFalse));

  tmpStr = pathMgr->simplifyPath(QString(d->comFile));
  fprintf(fp, "                 comfile=\"%s\"\n", tmpStr.toUtf8().constData());
  tmpStr = pathMgr->simplifyPath(QString(d->usamp1File));
  fprintf(fp, "                 u1file=\"%s\"\n", tmpStr.toUtf8().constData());
  tmpStr = pathMgr->simplifyPath(QString(d->usamp2File));
  fprintf(fp, "                 u2file=\"%s\"\n", tmpStr.toUtf8().constData());
  tmpStr = pathMgr->simplifyPath(QString(d->ramanFile));
  fprintf(fp, "                 ramfile=\"%s\" />\n", tmpStr.toUtf8().constData());

}

void CQdoasConfigWriter::writeShiftStretchList(FILE *fp, const shift_stretch_list_t *data)
{
  int k, j;
  const struct anlyswin_shift_stretch *d = &(data->shiftStretch[0]);

  fprintf(fp, "      <shift_stretches>\n");

  j = 0;
  while (j < data->nShiftStretch) {
    fprintf(fp, "        <shift_stretch shfit=\"%s\" ",
            ((d->shFit==ANLYS_SHIFT_TYPE_NONLINEAR) /* ||
             (d->shFit==ANLYS_SHIFT_TYPE_LINEAR) */ ) ? sTrue : sFalse);

    fprintf(fp," stfit=");
    switch (d->stFit) {
    case ANLYS_STRETCH_TYPE_FIRST_ORDER: fprintf(fp, "\"1st\""); break;
    case ANLYS_STRETCH_TYPE_SECOND_ORDER: fprintf(fp, "\"2nd\""); break;
    default: fprintf(fp, "\"none\"");
    }
    fprintf(fp, " shstr=\"%s\" ststr=\"%s\" errstr=\"%s\"",
        (d->shStore ? sTrue : sFalse),
        (d->stStore ? sTrue : sFalse),
        (d->errStore ? sTrue : sFalse));
    fprintf(fp, " shini=\"%.3e\" stini=\"%.3e\" stini2=\"%.3e\"",
        d->shInit,
        d->stInit, d->stInit2);
    fprintf(fp, " shdel=\"%.4e\" stdel=\"%.4e\" stdel2=\"%.4e\"",
        d->shDelta,
        d->stDelta, d->stDelta2);
    fprintf(fp, " shmin=\"%.3e\" shmax=\"%.3e\" >\n",
        d->shMin, d->shMax);

    k = 0;
    while (k < d->nSymbol) {
      fprintf(fp, "          <symbol name=\"%s\" />\n", d->symbol[k]);
      ++k;
    }

    fprintf(fp, "        </shift_stretch>\n");

    ++d;
    ++j;
  }

  fprintf(fp, "      </shift_stretches>\n");
}

void CQdoasConfigWriter::writeGapList(FILE *fp, const gap_list_t *d)
{
  int j = 0;

  fprintf(fp, "      <gaps>\n");

  while (j < d->nGap) {
    fprintf(fp, "        <gap min=\"%.2f\" max=\"%.2f\" />\n",
        d->gap[j].minimum, d->gap[j].maximum);
    ++j;
  }
  fprintf(fp, "      </gaps>\n");
}

void CQdoasConfigWriter::writeOutputList(FILE *fp, const output_list_t *d)
{
  int j = 0;

  fprintf(fp, "      <outputs>\n");

  while (j < d->nOutput) {
    fprintf(fp, "        <output sym=\"%s\" amf=\"%s\" scol=\"%s\" serr=\"%s\" sfact=\"%.3f\"",
        d->output[j].symbol,
        (d->output[j].amf ? sTrue : sFalse),
        (d->output[j].slantCol ? sTrue : sFalse),
        (d->output[j].slantErr ? sTrue : sFalse),
        d->output[j].slantFactor);
    fprintf(fp, " rescol=\"%.6le\" vcol=\"%s\" verr=\"%s\" vfact=\"%.3f\" />\n",
     d->output[j].resCol,
        (d->output[j].vertCol ? sTrue : sFalse),
        (d->output[j].vertErr ? sTrue : sFalse),
        d->output[j].vertFactor);

    ++j;
  }
  fprintf(fp, "      </outputs>\n");
}

void CQdoasConfigWriter::writeSfps(FILE *fp, const struct calibration_sfp *d)
{
  fprintf(fp, "      <sfps>\n");

  for (int i=0; i<NSFP; ++i) {
    fprintf(fp, "        <sfp index=\"%d\" fit=\"%s\" init=\"%.3f\" delta=\"%.3f\" fstr=\"%s\" estr=\"%s\" />\n",
        i+1, (d->fitFlag ? sTrue : sFalse),
        d->initialValue, d->deltaValue,
        (d->fitStore ? sTrue : sFalse), (d->errStore ? sTrue : sFalse));
    ++d;
  }

  fprintf(fp, "      </sfps>\n");
}


void CQdoasConfigWriter::writeDataSelectList(FILE *fp, const data_select_list_t *d)
{
  for (int i=0; i<d->nSelected; ++i) {

    const char *config_string = NULL;

    switch (d->selected[i]) {
    case PRJCT_RESULTS_SPECNO:           config_string = "specno"; break;
    case PRJCT_RESULTS_NAME:             config_string = "name"; break;
    case PRJCT_RESULTS_DATE_TIME:        config_string = "date_time"; break;
    case PRJCT_RESULTS_DATE:             config_string = "date"; break;
    case PRJCT_RESULTS_TIME:             config_string = "time"; break;
    case PRJCT_RESULTS_YEAR:             config_string = "year"; break;
    case PRJCT_RESULTS_JULIAN:           config_string = "julian"; break;
    case PRJCT_RESULTS_JDFRAC:           config_string = "jdfrac"; break;
    case PRJCT_RESULTS_TIFRAC:           config_string = "tifrac"; break;
    case PRJCT_RESULTS_SCANS:            config_string = "scans"; break;
    case PRJCT_RESULTS_NREJ:             config_string = "rejected"; break;
    case PRJCT_RESULTS_TINT:             config_string = "tint"; break;
    case PRJCT_RESULTS_SZA:              config_string = "sza"; break;
    case PRJCT_RESULTS_CHI:              config_string = "chi"; break;
    case PRJCT_RESULTS_RMS:              config_string = "rms"; break;
    case PRJCT_RESULTS_AZIM:             config_string = "azim"; break;
    case PRJCT_RESULTS_TDET:             config_string = "tdet"; break;
    case PRJCT_RESULTS_SKY:              config_string = "sky"; break;
    case PRJCT_RESULTS_BESTSHIFT:        config_string = "bestshift"; break;
    case PRJCT_RESULTS_REFZM:            config_string = "refzm"; break;
    case PRJCT_RESULTS_REFNUMBER:        config_string = "refnumber"; break;
    case PRJCT_RESULTS_REFSHIFT:         config_string = "refshift"; break;
    case PRJCT_RESULTS_PIXEL:            config_string = "pixel"; break;
    case PRJCT_RESULTS_PIXEL_TYPE:       config_string = "pixel_type"; break;
    case PRJCT_RESULTS_ORBIT:            config_string = "orbit"; break;
    case PRJCT_RESULTS_LON_CORNERS: config_string = "corner_longitudes"; break;
    case PRJCT_RESULTS_LAT_CORNERS: config_string = "corner_latitudes"; break;
    case PRJCT_RESULTS_LONGIT:           config_string = "longit"; break;
    case PRJCT_RESULTS_LATIT:            config_string = "latit"; break;
    case PRJCT_RESULTS_ALTIT:            config_string = "altit"; break;
    case PRJCT_RESULTS_COVAR:            config_string = "covar"; break;
    case PRJCT_RESULTS_CORR:             config_string = "corr"; break;
    case PRJCT_RESULTS_CLOUD:            config_string = "cloud"; break;
    case PRJCT_RESULTS_O3:               config_string = "o3"; break;
    case PRJCT_RESULTS_NO2:              config_string = "no2"; break;
    case PRJCT_RESULTS_CLOUDTOPP:        config_string = "cloudtopp"; break;
    case PRJCT_RESULTS_LOS_ZA:           config_string = "los_za"; break;
    case PRJCT_RESULTS_LOS_AZIMUTH:      config_string = "los_azimuth"; break;
    case PRJCT_RESULTS_SAT_HEIGHT:       config_string = "sat_height"; break;
    case PRJCT_RESULTS_SAT_LON:          config_string = "sat_latitude"; break;
    case PRJCT_RESULTS_SAT_LAT:          config_string = "sat_longitude"; break;
    case PRJCT_RESULTS_SAT_SAA:          config_string = "sat_saa"; break;
    case PRJCT_RESULTS_SAT_SZA:          config_string = "sat_sza"; break;
    case PRJCT_RESULTS_SAT_VZA:          config_string = "sat_vza"; break;
    case PRJCT_RESULTS_EARTH_RADIUS:     config_string = "earth_radius"; break;
    case PRJCT_RESULTS_VIEW_ELEVATION:   config_string = "view_elevation"; break;
    case PRJCT_RESULTS_VIEW_ZENITH:      config_string = "view_zenith"; break;
    case PRJCT_RESULTS_VIEW_AZIMUTH:     config_string = "view_azimuth"; break;
    case PRJCT_RESULTS_SCIA_QUALITY:     config_string = "scia_quality"; break;
    case PRJCT_RESULTS_SCIA_STATE_INDEX: config_string = "scia_state_index"; break;
    case PRJCT_RESULTS_SCIA_STATE_ID:    config_string = "scia_state_id"; break;
    case PRJCT_RESULTS_STARTDATE:        config_string = "startdate"; break;
    case PRJCT_RESULTS_ENDDATE:          config_string = "enddate"; break;
    case PRJCT_RESULTS_STARTTIME:        config_string = "starttime"; break;
    case PRJCT_RESULTS_ENDTIME:          config_string = "endtime"; break;

    case PRJCT_RESULTS_LAMBDA_CENTER : config_string = "center_wavelength"; break;

    case PRJCT_RESULTS_SCANNING            :      config_string = "scanning_angle"; break;
    case PRJCT_RESULTS_FILTERNUMBER        :      config_string = "filterNumber"; break;
    case PRJCT_RESULTS_MEASTYPE            :      config_string = "measType"; break;
    case PRJCT_RESULTS_CCD_HEADTEMPERATURE :      config_string = "ccd_headTemp"; break;

    case PRJCT_RESULTS_COOLING_STATUS      :      config_string = "cooler_status"; break;
    case PRJCT_RESULTS_MIRROR_ERROR        :      config_string = "mirror_status"; break;
    case PRJCT_RESULTS_COMPASS             :      config_string = "compass_angle"; break;
    case PRJCT_RESULTS_PITCH               :      config_string = "pitch_angle"; break;
    case PRJCT_RESULTS_ROLL                :      config_string = "roll_angle"; break;
    case PRJCT_RESULTS_ITER                :      config_string = "iter_number"; break;
    case PRJCT_RESULTS_ERROR_FLAG          :      config_string = "error_flag"; break;
    case PRJCT_RESULTS_NUM_BANDS           :      config_string = "num_bands"; break;

    case PRJCT_RESULTS_GOME2_MDR_NUMBER: config_string = "gome2_mdr_number"; break;
    case PRJCT_RESULTS_GOME2_OBSERVATION_INDEX: config_string = "gome2_observation_index"; break;
    case PRJCT_RESULTS_GOME2_SCANDIRECTION      :      config_string = "scan_direction"; break;
    case PRJCT_RESULTS_GOME2_OBSERVATION_MODE   :      config_string = "gome2_observation_mode"; break;
    case PRJCT_RESULTS_GOME2_SAA                :      config_string = "saa_flag"; break;
    case PRJCT_RESULTS_GOME2_SUNGLINT_RISK      :      config_string = "sunglint_danger_flag"; break;
    case PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK  :      config_string = "sunglint_highdanger_flag"; break;
    case PRJCT_RESULTS_GOME2_RAINBOW            :      config_string = "rainbow_flag"; break;

    case PRJCT_RESULTS_CCD_DIODES : config_string = "diodes"; break;
    case PRJCT_RESULTS_CCD_TARGETAZIMUTH : config_string = "target_azimuth"; break;
    case PRJCT_RESULTS_CCD_TARGETELEVATION : config_string = "target_elevation"; break;
    case PRJCT_RESULTS_SATURATED : config_string = "saturated"; break;
    case PRJCT_RESULTS_INDEX_ALONGTRACK : config_string = "index_alongtrack"; break;
    case PRJCT_RESULTS_INDEX_CROSSTRACK : config_string = "index_crosstrack"; break;
    case PRJCT_RESULTS_GROUNDP_QF : config_string = "groundp_qf"; break;
    case PRJCT_RESULTS_XTRACK_QF : config_string = "xtrack_qf"; break;
    case PRJCT_RESULTS_PIXELS_QF : config_string = "pixels_qf"; break;
    case PRJCT_RESULTS_OMI_CONFIGURATION_ID : config_string = "omi_configuration_id"; break;
    case PRJCT_RESULTS_SPIKES: config_string = "spike_pixels"; break;
    case PRJCT_RESULTS_UAV_SERVO_BYTE_SENT : config_string = "servo_byte_sent"; break;
    case PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED : config_string = "servo_byte_received"; break;
    case PRJCT_RESULTS_UAV_INSIDE_TEMP : config_string = "uav_inside_temp"; break;
    case PRJCT_RESULTS_UAV_OUTSIDE_TEMP : config_string = "uav_outside_temp"; break;
    case PRJCT_RESULTS_UAV_PRESSURE : config_string = "uav_pressure"; break;
    case PRJCT_RESULTS_UAV_HUMIDITY : config_string = "uav_humidity"; break;
    case PRJCT_RESULTS_UAV_DEWPOINT : config_string = "uav_dewpoint"; break;
    case PRJCT_RESULTS_UAV_PITCH: config_string = "uav_pitch"; break;
    case PRJCT_RESULTS_UAV_ROLL: config_string = "uav_roll"; break;
    case PRJCT_RESULTS_UAV_HEADING: config_string = "uav_heading"; break;
    case PRJCT_RESULTS_STARTGPSTIME : config_string = "gps_starttime"; break;
    case PRJCT_RESULTS_ENDGPSTIME : config_string = "gps_endtime"; break;
    case PRJCT_RESULTS_LONGITEND : config_string = "longitude_end"; break;
    case PRJCT_RESULTS_LATITEND : config_string = "latitude_end"; break;
    case PRJCT_RESULTS_ALTITEND : config_string = "altitude_end"; break;
    case PRJCT_RESULTS_TOTALEXPTIME : config_string = "total_exp_time";break;
    case PRJCT_RESULTS_TOTALACQTIME : config_string = "total_acq_time";break;
    case PRJCT_RESULTS_LAMBDA : config_string = "lambda"; break;
    case PRJCT_RESULTS_SPECTRA : config_string = "spectra"; break;
    case PRJCT_RESULTS_FILENAME : config_string = "filename"; break;
    case PRJCT_RESULTS_SCANINDEX : config_string = "scan_index"; break;
    case PRJCT_RESULTS_ZENITH_BEFORE : config_string = "zenith_before_index"; break;
    case PRJCT_RESULTS_ZENITH_AFTER : config_string = "zenith_after_index"; break;
    case PRJCT_RESULTS_PRECALCULATED_FLUXES : config_string = "precalculated_fluxes"; break;
    case PRJCT_RESULTS_RC : config_string = "rc"; break;
    case PRJCT_RESULTS_RESIDUAL_SPECTRUM : config_string = "residual_spectrum"; break;

    default: puts("ERROR: no configuration string defined for output field. This is a bug, please contact Qdoas developers.");
    }

    if (config_string) {
      fprintf(fp, "      <field name=\"%s\" />\n", config_string);
    }
  }
}
