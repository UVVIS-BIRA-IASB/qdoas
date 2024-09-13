/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <math.h>
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CProjectConfigItem.h"

#include "constants.h"

using std::map;

CAnalysisWindowSubHandler::CAnalysisWindowSubHandler(CConfigHandler *master,
                             CAnalysisWindowConfigItem *item) :
  CConfigSubHandler(master),
  m_item(item)
{
}

void CAnalysisWindowSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (!m_item->setName(value(atts, "name")))
    throw std::runtime_error("Analysis window name too long.");

  m_item->setEnabled(value(atts, "disable") != "true");

  QString str = value(atts, "kurucz");
  if (str == "ref")
    d->kuruczMode = ANLYS_KURUCZ_REF;
  else if (str == "spec")
    d->kuruczMode = ANLYS_KURUCZ_SPEC;
  else if (str == "ref+spec")
    d->kuruczMode = ANLYS_KURUCZ_REF_AND_SPEC;
  else
    d->kuruczMode = ANLYS_KURUCZ_NONE;

  d->refSpectrumSelection = (value(atts, "refsel") == "auto") ? ANLYS_REF_SELECTION_MODE_AUTOMATIC :  ANLYS_REF_SELECTION_MODE_FILE;

  d->fitMinWavelength = value(atts, "min").toDouble();
  d->fitMaxWavelength = value(atts, "max").toDouble();
  d->resolFwhm=(!value(atts, "resol_fwhm").isEmpty())?value(atts, "resol_fwhm").toDouble():0.5;
  d->lambda0=(!value(atts, "lambda0").isEmpty())?value(atts, "lambda0").toDouble():0.5*(d->fitMinWavelength+d->fitMaxWavelength);

  // MUST have a valid name
  if (m_item->name().isEmpty()) {
    throw std::runtime_error("Analysis window must have valid name.");
  };
}

void CAnalysisWindowSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (element == "display") {
    d->requireSpectrum = (value(atts, "spectrum") == "true") ? 1 : 0;
    d->requirePolynomial = (value(atts, "poly") == "true") ? 1 : 0;
    d->requireFit = (value(atts, "fits") == "true") ? 1 : 0;
    d->requireResidual = (value(atts, "residual") == "true") ? 1 : 0;
    d->requirePredefined = (value(atts, "predef") == "true") ? 1 : 0;
    d->requireRefRatio = (value(atts, "ratio") == "true") ? 1 : 0;

  }
  else if (element == "files") {
    QString str;

    str = value(atts, "refone");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refOneFile))
    strcpy(d->refOneFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Reference 1 Filename too long");
    }

    str = value(atts, "reftwo");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refTwoFile))
    strcpy(d->refTwoFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Reference 2 Filename too long");
    }

    str = value(atts, "residual");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->residualFile))
    strcpy(d->residualFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Residual Filename too long");
    }

    d->saveResidualsFlag = (value(atts, "saveresiduals") == "true") ? 1 : 0;

    d->refMinLongitude = value(atts, "minlon").toDouble();
    d->refMaxLongitude = value(atts, "maxlon").toDouble();
    d->refMinLatitude = value(atts, "minlat").toDouble();
    d->refMaxLatitude = value(atts, "maxlat").toDouble();
    d->refNs = value(atts, "refns").toInt();

    if (d->refNs<=0)
     d->refNs=1;
    else if (d->refNs>50)
     d->refNs=50;

    d->cloudFractionMin = value(atts, "cloudfmin").toDouble();
    d->cloudFractionMax = value(atts, "cloudfmax").toDouble();

    d->refMaxdoasSelection = (value(atts, "maxdoasrefmode") == "scan") ? ANLYS_MAXDOAS_REF_SCAN :  ANLYS_MAXDOAS_REF_SZA;

    str = value(atts, "scanmode");
    if (str == "before")
     d->refSpectrumSelectionScanMode = ANLYS_MAXDOAS_REF_SCAN_BEFORE;
    else if (str == "average")
     d->refSpectrumSelectionScanMode = ANLYS_MAXDOAS_REF_SCAN_AVERAGE;
    else if (str == "interpolate")
     d->refSpectrumSelectionScanMode = ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE;
    else
     d->refSpectrumSelectionScanMode = ANLYS_MAXDOAS_REF_SCAN_AFTER;

    if (d->refMaxdoasSelection==ANLYS_MAXDOAS_REF_SCAN)
     {
      d->refSzaCenter = value(atts, "maxdoasszacenter").toDouble();
      d->refSzaDelta = value(atts, "maxdoasszadelta").toDouble();
     }
    else
     {
      d->refSzaCenter = value(atts, "szacenter").toDouble();
      d->refSzaDelta = value(atts, "szadelta").toDouble();
     }
  }
  else if (element == "cross_section") {
    m_master->install_subhandler(new CAnalysisWindowCrossSectionSubHandler(m_master, &(d->crossSectionList)), atts);
  }
  else if (element == "linear") {
    m_master->install_subhandler(new CAnalysisWindowLinearSubHandler(m_master, &(d->linear)), atts);
  }
  else if (element == "nonlinear") {
    m_master->install_subhandler(new CAnalysisWindowNonLinearSubHandler(m_master, &(d->nonlinear)), atts);
  }
  else if (element == "shift_stretch") {
    m_master->install_subhandler(new CAnalysisWindowShiftStretchSubHandler(m_master, &(d->shiftStretchList)), atts);
  }
  else if (element == "gap") {
    m_master->install_subhandler(new CAnalysisWindowGapSubHandler(m_master, &(d->gapList)), atts);
  }
  else if (element == "output") {
    m_master->install_subhandler(new CAnalysisWindowOutputSubHandler(m_master, &(d->outputList)), atts);
  }
}

int CAnalysisWindowSubHandler::mapToPolyType(const QString &str)
{
  int result = ANLYS_POLY_TYPE_NONE;

  if (str == "0") result = ANLYS_POLY_TYPE_0;
  else if (str == "1") result = ANLYS_POLY_TYPE_1;
  else if (str == "2") result = ANLYS_POLY_TYPE_2;
  else if (str == "3") result = ANLYS_POLY_TYPE_3;
  else if (str == "4") result = ANLYS_POLY_TYPE_4;
  else if (str == "5") result = ANLYS_POLY_TYPE_5;
  else if (str == "6") result = ANLYS_POLY_TYPE_6;
  else if (str == "7") result = ANLYS_POLY_TYPE_7;
  else if (str == "8") result = ANLYS_POLY_TYPE_8;

  return result;
}

CAnalysisWindowCrossSectionSubHandler::CAnalysisWindowCrossSectionSubHandler(CConfigHandler *master,
                                         cross_section_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowCrossSectionSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  if (m_d->nCrossSection < MAX_AW_CROSS_SECTION) {

    QString str;
    struct anlyswin_cross_section *d = &(m_d->crossSection[m_d->nCrossSection]);

    str = value(atts, "sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toLocal8Bit().data());
    else
      throw std::runtime_error("missing symbol (or name too long)");

    str = value(atts, "ortho");

    if (str.isEmpty())
     strcpy(d->orthogonal,"");
    else if (str.length() < (int)sizeof(d->orthogonal))
      strcpy(d->orthogonal, str.toLocal8Bit().data());
    else
       throw std::runtime_error("ortho name too long");

    str = value(atts, "subtract");

    if (str.isEmpty())
     strcpy(d->subtract,"");
    else if (str.length() < (int)sizeof(d->subtract))
      strcpy(d->subtract, str.toLocal8Bit().data());
    else
      throw std::runtime_error("subtract name too long");

    d->subtractFlag=(value(atts, "subtract_flag") == "true") ? 1 : 0;

    str = value(atts, "cstype");
    if (str == "interp") d->crossType = ANLYS_CROSS_ACTION_INTERPOLATE;
    else if (str == "std") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE;
    else if (str == "io") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_I0;
    else if (str == "ring") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_RING;
    else d->crossType = ANLYS_CROSS_ACTION_NOTHING;

    str = value(atts, "amftype");
    if (str == "sza") d->amfType = ANLYS_AMF_TYPE_SZA;
    else if (str == "climate") d->amfType = ANLYS_AMF_TYPE_CLIMATOLOGY;
    else if ((str == "wave") ||
             (str == "wave1") || (str == "wave2") || (str == "wave3"))          // for compatibility with previous versions (wavelength 1, 2, 3)
     d->amfType = ANLYS_AMF_TYPE_WAVELENGTH;
    else d->amfType = ANLYS_AMF_TYPE_NONE;

    str = value(atts, "corrtype");
    if (str == "slope") d->correctionType = ANLYS_CORRECTION_TYPE_SLOPE;
    else if (str == "pukite") d->correctionType = ANLYS_CORRECTION_TYPE_PUKITE;
    else if (str == "molecular_ring") d->correctionType = ANLYS_CORRECTION_TYPE_MOLECULAR_RING;
    else if (str == "molecular_ring_slope") d->correctionType = ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE;
    else d->correctionType = ANLYS_CORRECTION_TYPE_NONE;

    str = value(atts, "molecular_xs");
    strcpy(d->molecularRing,str.toLocal8Bit().data());

    d->requireFit = (value(atts, "fit") == "true") ? 1 : 0;
    d->requireFilter = (value(atts, "filter") == "true") ? 1 : 0;
    d->constrainedCc = (value(atts, "cstrncc") == "true") ? 1 : 0;
    d->requireCcFit = (value(atts, "ccfit") == "true") ? 1 : 0;
    d->initialCc = value(atts, "icc").toDouble();
    d->deltaCc = value(atts, "dcc").toDouble();
    d->ccIo = value(atts, "ccio").toDouble();
    d->ccMin = value(atts, "ccmin").toDouble();
    d->ccMax = value(atts, "ccmax").toDouble();

    if (fabs(d->deltaCc)<(double)EPSILON)
     d->deltaCc=(double)1.e-3;

    str = value(atts, "csfile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->crossSectionFile))
    strcpy(d->crossSectionFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Cross Section filename too long");
    }
    else
      throw std::runtime_error("Missing cross section file");

    str = value(atts, "amffile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->amfFile))
    strcpy(d->amfFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("AMF filename too long");
    }

    // All OK
    ++(m_d->nCrossSection);

    return;
  }

  throw std::runtime_error("Too many cross sections in analysis window");
}

//------------------------------------------------------------

CAnalysisWindowLinearSubHandler::CAnalysisWindowLinearSubHandler(CConfigHandler *master,
                                 struct anlyswin_linear *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowLinearSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  m_d->xPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(value(atts, "xpoly"));
  m_d->xBaseOrder = CAnalysisWindowSubHandler::mapToPolyType(value(atts, "xbase"));
  m_d->xFlagFitStore = (value(atts, "xfit") == "true") ? 1 : 0;
  m_d->xFlagErrStore = (value(atts, "xerr") == "true") ? 1 : 0;

  m_d->offsetPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(value(atts, "offpoly"));
  m_d->offsetFlagFitStore = (value(atts, "offfit") == "true") ? 1 : 0;
  m_d->offsetFlagErrStore = (value(atts, "offerr") == "true") ? 1 : 0;

  m_d->offsetI0 = (value(atts, "offizero") == "true") ?  1 : 0;

}

//------------------------------------------------------------

CAnalysisWindowNonLinearSubHandler::CAnalysisWindowNonLinearSubHandler(CConfigHandler *master,
                                       struct anlyswin_nonlinear *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowNonLinearSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  QString str;

  m_d->solFlagFit = (value(atts, "solfit") == "true") ? 1 : 0;
  m_d->solInitial = value(atts, "solinit").toDouble();
  m_d->solDelta = value(atts, "soldelt").toDouble();
  m_d->solFlagFitStore = (value(atts, "solfstr") == "true") ? 1 : 0;
  m_d->solFlagErrStore = (value(atts, "solestr") == "true") ? 1 : 0;

  if (fabs(m_d->solDelta)<(double)EPSILON)
   m_d->solDelta=(double)1.e-3;

  m_d->off0FlagFit = (value(atts, "o0fit") == "true") ? 1 : 0;
  m_d->off0Initial = value(atts, "o0init").toDouble();
  m_d->off0Delta = value(atts, "o0delt").toDouble();
  m_d->off0FlagFitStore = (value(atts, "o0fstr") == "true") ? 1 : 0;
  m_d->off0FlagErrStore = (value(atts, "o0estr") == "true") ? 1 : 0;

  if (fabs(m_d->off0Delta)<(double)EPSILON)
   m_d->off0Delta=(double)1.e-3;

  m_d->off1FlagFit = (value(atts, "o1fit") == "true") ? 1 : 0;
  m_d->off1Initial = value(atts, "o1init").toDouble();
  m_d->off1Delta = value(atts, "o1delt").toDouble();
  m_d->off1FlagFitStore = (value(atts, "o1fstr") == "true") ? 1 : 0;
  m_d->off1FlagErrStore = (value(atts, "o1estr") == "true") ? 1 : 0;

  if (fabs(m_d->off1Delta)<(double)EPSILON)
   m_d->off1Delta=(double)1.e-3;

  m_d->off2FlagFit = (value(atts, "o2fit") == "true") ? 1 : 0;
  m_d->off2Initial = value(atts, "o2init").toDouble();
  m_d->off2Delta = value(atts, "o2delt").toDouble();
  m_d->off2FlagFitStore = (value(atts, "o2fstr") == "true") ? 1 : 0;
  m_d->off2FlagErrStore = (value(atts, "o2estr") == "true") ? 1 : 0;

  if (fabs(m_d->off2Delta)<(double)EPSILON)
   m_d->off2Delta=(double)1.e-3;

  m_d->comFlagFit = (value(atts, "comfit") == "true") ? 1 : 0;
  m_d->comInitial = value(atts, "cominit").toDouble();
  m_d->comDelta = value(atts, "comdelt").toDouble();
  m_d->comFlagFitStore = (value(atts, "comfstr") == "true") ? 1 : 0;
  m_d->comFlagErrStore = (value(atts, "comestr") == "true") ? 1 : 0;

  if (fabs(m_d->comDelta)<(double)EPSILON)
   m_d->comDelta=(double)1.e-3;

  m_d->usamp1FlagFit = (value(atts, "u1fit") == "true") ? 1 : 0;
  m_d->usamp1Initial = value(atts, "u1init").toDouble();
  m_d->usamp1Delta = value(atts, "u1delt").toDouble();
  m_d->usamp1FlagFitStore = (value(atts, "u1str") == "true") ? 1 : 0;
  m_d->usamp1FlagErrStore = (value(atts, "u1estr") == "true") ? 1 : 0;

  if (fabs(m_d->usamp1Delta)<(double)EPSILON)
   m_d->usamp1Delta=(double)1.e-3;

  m_d->usamp2FlagFit = (value(atts, "u2fit") == "true") ? 1 : 0;
  m_d->usamp2Initial = value(atts, "u2init").toDouble();
  m_d->usamp2Delta = value(atts, "u2delt").toDouble();
  m_d->usamp2FlagFitStore = (value(atts, "u2str") == "true") ? 1 : 0;
  m_d->usamp2FlagErrStore = (value(atts, "u2estr") == "true") ? 1 : 0;

  if (fabs(m_d->usamp2Delta)<(double)EPSILON)
   m_d->usamp2Delta=(double)1.e-3;

  m_d->resolFlagFit = (value(atts, "resolfit") == "true") ? 1 : 0;
  m_d->resolInitial = value(atts, "resolinit").toDouble();
  m_d->resolDelta = value(atts, "resoldelt").toDouble();
  m_d->resolFlagFitStore = (value(atts, "resolstr") == "true") ? 1 : 0;
  m_d->resolFlagErrStore = (value(atts, "resolestr") == "true") ? 1 : 0;

  if (fabs(m_d->resolDelta)<(double)EPSILON)
   m_d->resolDelta=(double)1.e-3;

  str = value(atts, "comfile");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->comFile))
      strcpy(m_d->comFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Com filename too long");
  }

  str = value(atts, "u1file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp1File))
      strcpy(m_d->usamp1File, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Usamp1 filename too long");
  }

  str = value(atts, "u2file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp2File))
      strcpy(m_d->usamp2File, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Usamp2 filename too long");
  }

}

//------------------------------------------------------------

CAnalysisWindowShiftStretchSubHandler::CAnalysisWindowShiftStretchSubHandler(CConfigHandler *master,
                                         shift_stretch_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowShiftStretchSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  if (m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    QString str;
    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);

    str=value(atts, "shfit");

    if ((str=="true") || (str=="nonlinear")) d->shFit=ANLYS_SHIFT_TYPE_NONLINEAR;
  //  else if (str=="linear") d->shFit=ANLYS_SHIFT_TYPE_LINEAR;
    else d->shFit=ANLYS_SHIFT_TYPE_NONE;

    str = value(atts, "stfit");
    if (str == "1st") d->stFit = ANLYS_STRETCH_TYPE_FIRST_ORDER;
    else if (str == "2nd") d->stFit = ANLYS_STRETCH_TYPE_SECOND_ORDER;
    else d->stFit = ANLYS_STRETCH_TYPE_NONE;

    d->shStore = (value(atts, "shstr") == "true") ? 1 : 0;
    d->stStore = (value(atts, "ststr") == "true") ? 1 : 0;
    d->errStore = (value(atts, "errstr") == "true") ? 1 : 0;

    d->shInit = value(atts, "shini").toDouble();
    d->stInit = value(atts, "stini").toDouble();
    d->stInit2 = value(atts, "stini2").toDouble();

    if (fabs((double)(d->shDelta = value(atts, "shdel").toDouble()  ))<EPSILON) d->shDelta  =(double)1.e-3;
    if (fabs((double)(d->stDelta = value(atts, "stdel").toDouble()  ))<EPSILON) d->stDelta  =(double)1.e-3;
    if (fabs((double)(d->stDelta2 = value(atts, "stdel2").toDouble()))<EPSILON) d->stDelta2 =(double)1.e-3;

    d->shMin = value(atts, "shmin").toDouble();
    d->shMax = value(atts, "shmax").toDouble();

    return;
  }

  throw std::runtime_error("Too many cross sections in analysis window");
}

void CAnalysisWindowShiftStretchSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  if (element == "symbol" && m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);

    if (d->nSymbol < MAX_AW_SHIFT_STRETCH) {

      QString str = value(atts, "name");
      if (!str.isEmpty() && str.length() < (int)SYMBOL_NAME_BUFFER_LENGTH) {
        strcpy(&(d->symbol[d->nSymbol][0]), str.toLocal8Bit().data());
        ++(d->nSymbol);
        return;
      }
    }
  }
  throw std::runtime_error("Failed parsing shift/stretch settings.");
}

void CAnalysisWindowShiftStretchSubHandler::end()
{
  if (m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH)
    ++(m_d->nShiftStretch);
}

//------------------------------------------------------------

CAnalysisWindowGapSubHandler::CAnalysisWindowGapSubHandler(CConfigHandler *master,
                               gap_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowGapSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  if (m_d->nGap < MAX_AW_GAP) {
    m_d->gap[m_d->nGap].minimum = value(atts, "min").toDouble();
    m_d->gap[m_d->nGap].maximum = value(atts, "max").toDouble();
    if (m_d->gap[m_d->nGap].minimum < m_d->gap[m_d->nGap].maximum) {
      ++(m_d->nGap);
      return;
    }
    else
      throw std::runtime_error("Invalid gap definition");
  }

  throw std::runtime_error("Too many gaps defined");
}

//------------------------------------------------------------

CAnalysisWindowOutputSubHandler::CAnalysisWindowOutputSubHandler(CConfigHandler *master,
                                 output_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowOutputSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  if (m_d->nOutput < MAX_AW_CROSS_SECTION) {

    QString str;
    struct anlyswin_output *d = &(m_d->output[m_d->nOutput]);

    str = value(atts, "sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toLocal8Bit().data());
    else
      throw std::runtime_error("missing symbol (or name too long)");

    d->amf = (value(atts, "amf") == "true") ? 1 : 0;
    d->resCol = value(atts, "rescol").toDouble();
    d->slantCol = (value(atts, "scol") == "true") ? 1 : 0;
    d->slantErr = (value(atts, "serr") == "true") ? 1 : 0;
    d->slantFactor = value(atts, "sfact").toDouble();
    d->vertCol = (value(atts, "vcol") == "true") ? 1 : 0;
    d->vertErr = (value(atts, "verr") == "true") ? 1 : 0;
    d->vertFactor = value(atts, "vfact").toDouble();

    // All OK
    ++(m_d->nOutput);

    return;
  }

  throw std::runtime_error("Too many outputs in analysis window");
}

//------------------------------------------------------------

CAnalysisWindowSfpSubHandler::CAnalysisWindowSfpSubHandler(CConfigHandler *master,
                               struct calibration_sfp *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

void CAnalysisWindowSfpSubHandler::start(const map<Glib::ustring, QString> &atts)
{
  int index = value(atts, "index").toInt();

  if (index > 0 && index <= 4) {
    struct calibration_sfp *p = (m_d + index - 1);

    p->fitFlag = (value(atts, "fit") == "true") ? 1 : 0;
    p->initialValue = value(atts, "init").toDouble();
    p->deltaValue = value(atts, "delta").toDouble();
    p->fitStore = (value(atts, "fstr") == "true") ? 1 : 0;
    p->errStore = (value(atts, "estr") == "true") ? 1 : 0;

    if (fabs(p->deltaValue)<EPSILON)
     p->deltaValue=(double)1.e-3;

    return;
  }

  throw std::runtime_error("Invalid SFP index");
}
