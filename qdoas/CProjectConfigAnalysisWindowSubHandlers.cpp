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

  if (!m_item->setName(atts.at("name")))
    throw std::runtime_error("Analysis window name too long.");

  m_item->setEnabled(atts.at("disable") != "true");

  QString str = atts.at("kurucz");
  if (str == "ref")
    d->kuruczMode = ANLYS_KURUCZ_REF;
  else if (str == "spec")
    d->kuruczMode = ANLYS_KURUCZ_SPEC;
  else if (str == "ref+spec")
    d->kuruczMode = ANLYS_KURUCZ_REF_AND_SPEC;
  else
    d->kuruczMode = ANLYS_KURUCZ_NONE;

  d->refSpectrumSelection = (atts.at("refsel") == "auto") ? ANLYS_REF_SELECTION_MODE_AUTOMATIC :  ANLYS_REF_SELECTION_MODE_FILE;

  d->fitMinWavelength = atts.at("min").toDouble();
  d->fitMaxWavelength = atts.at("max").toDouble();
  d->resolFwhm=(!atts.at("resol_fwhm").isEmpty())?atts.at("resol_fwhm").toDouble():0.5;
  d->lambda0=(!atts.at("lambda0").isEmpty())?atts.at("lambda0").toDouble():0.5*(d->fitMinWavelength+d->fitMaxWavelength);

  // MUST have a valid name
  if (m_item->name().isEmpty()) {
    throw std::runtime_error("Analysis window must have valid name.");
  };
}

void CAnalysisWindowSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (element == "display") {
    d->requireSpectrum = (atts.at("spectrum") == "true") ? 1 : 0;
    d->requirePolynomial = (atts.at("poly") == "true") ? 1 : 0;
    d->requireFit = (atts.at("fits") == "true") ? 1 : 0;
    d->requireResidual = (atts.at("residual") == "true") ? 1 : 0;
    d->requirePredefined = (atts.at("predef") == "true") ? 1 : 0;
    d->requireRefRatio = (atts.at("ratio") == "true") ? 1 : 0;

  }
  else if (element == "files") {
    QString str;

    str = atts.at("refone");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refOneFile))
    strcpy(d->refOneFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Reference 1 Filename too long");
    }

    str = atts.at("reftwo");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refTwoFile))
    strcpy(d->refTwoFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Reference 2 Filename too long");
    }

    str = atts.at("residual");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->residualFile))
    strcpy(d->residualFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Residual Filename too long");
    }

    d->saveResidualsFlag = (atts.at("saveresiduals") == "true") ? 1 : 0;

    d->refMinLongitude = atts.at("minlon").toDouble();
    d->refMaxLongitude = atts.at("maxlon").toDouble();
    d->refMinLatitude = atts.at("minlat").toDouble();
    d->refMaxLatitude = atts.at("maxlat").toDouble();
    d->refNs = atts.at("refns").toInt();

    if (d->refNs<=0)
     d->refNs=1;
    else if (d->refNs>50)
     d->refNs=50;

    d->cloudFractionMin = atts.at("cloudfmin").toDouble();
    d->cloudFractionMax = atts.at("cloudfmax").toDouble();

    d->refMaxdoasSelection = (atts.at("maxdoasrefmode") == "scan") ? ANLYS_MAXDOAS_REF_SCAN :  ANLYS_MAXDOAS_REF_SZA;

    str = atts.at("scanmode");
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
      d->refSzaCenter = atts.at("maxdoasszacenter").toDouble();
      d->refSzaDelta = atts.at("maxdoasszadelta").toDouble();
     }
    else
     {
      d->refSzaCenter = atts.at("szacenter").toDouble();
      d->refSzaDelta = atts.at("szadelta").toDouble();
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

    str = atts.at("sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toLocal8Bit().data());
    else
      throw std::runtime_error("missing symbol (or name too long)");

    str = atts.at("ortho");

    if (str.isEmpty())
     strcpy(d->orthogonal,"");
    else if (str.length() < (int)sizeof(d->orthogonal))
      strcpy(d->orthogonal, str.toLocal8Bit().data());
    else
       throw std::runtime_error("ortho name too long");

    str = atts.at("subtract");

    if (str.isEmpty())
     strcpy(d->subtract,"");
    else if (str.length() < (int)sizeof(d->subtract))
      strcpy(d->subtract, str.toLocal8Bit().data());
    else
      throw std::runtime_error("subtract name too long");

    d->subtractFlag=(atts.at("subtract_flag") == "true") ? 1 : 0;

    str = atts.at("cstype");
    if (str == "interp") d->crossType = ANLYS_CROSS_ACTION_INTERPOLATE;
    else if (str == "std") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE;
    else if (str == "io") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_I0;
    else if (str == "ring") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_RING;
    else d->crossType = ANLYS_CROSS_ACTION_NOTHING;

    str = atts.at("amftype");
    if (str == "sza") d->amfType = ANLYS_AMF_TYPE_SZA;
    else if (str == "climate") d->amfType = ANLYS_AMF_TYPE_CLIMATOLOGY;
    else if ((str == "wave") ||
             (str == "wave1") || (str == "wave2") || (str == "wave3"))          // for compatibility with previous versions (wavelength 1, 2, 3)
     d->amfType = ANLYS_AMF_TYPE_WAVELENGTH;
    else d->amfType = ANLYS_AMF_TYPE_NONE;

    str = atts.at("corrtype");
    if (str == "slope") d->correctionType = ANLYS_CORRECTION_TYPE_SLOPE;
    else if (str == "pukite") d->correctionType = ANLYS_CORRECTION_TYPE_PUKITE;
    else if (str == "molecular_ring") d->correctionType = ANLYS_CORRECTION_TYPE_MOLECULAR_RING;
    else if (str == "molecular_ring_slope") d->correctionType = ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE;
    else d->correctionType = ANLYS_CORRECTION_TYPE_NONE;

    str = atts.at("molecular_xs");
    strcpy(d->molecularRing,str.toLocal8Bit().data());

    d->requireFit = (atts.at("fit") == "true") ? 1 : 0;
    d->requireFilter = (atts.at("filter") == "true") ? 1 : 0;
    d->constrainedCc = (atts.at("cstrncc") == "true") ? 1 : 0;
    d->requireCcFit = (atts.at("ccfit") == "true") ? 1 : 0;
    d->initialCc = atts.at("icc").toDouble();
    d->deltaCc = atts.at("dcc").toDouble();
    d->ccIo = atts.at("ccio").toDouble();
    d->ccMin = atts.at("ccmin").toDouble();
    d->ccMax = atts.at("ccmax").toDouble();

    if (fabs(d->deltaCc)<(double)EPSILON)
     d->deltaCc=(double)1.e-3;

    str = atts.at("csfile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->crossSectionFile))
    strcpy(d->crossSectionFile, str.toLocal8Bit().data());
      else
    throw std::runtime_error("Cross Section filename too long");
    }
    else
      throw std::runtime_error("Missing cross section file");

    str = atts.at("amffile");
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
  m_d->xPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.at("xpoly"));
  m_d->xBaseOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.at("xbase"));
  m_d->xFlagFitStore = (atts.at("xfit") == "true") ? 1 : 0;
  m_d->xFlagErrStore = (atts.at("xerr") == "true") ? 1 : 0;

  m_d->offsetPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.at("offpoly"));
  m_d->offsetFlagFitStore = (atts.at("offfit") == "true") ? 1 : 0;
  m_d->offsetFlagErrStore = (atts.at("offerr") == "true") ? 1 : 0;

  m_d->offsetI0 = (atts.at("offizero") == "true") ?  1 : 0;

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

  m_d->solFlagFit = (atts.at("solfit") == "true") ? 1 : 0;
  m_d->solInitial = atts.at("solinit").toDouble();
  m_d->solDelta = atts.at("soldelt").toDouble();
  m_d->solFlagFitStore = (atts.at("solfstr") == "true") ? 1 : 0;
  m_d->solFlagErrStore = (atts.at("solestr") == "true") ? 1 : 0;

  if (fabs(m_d->solDelta)<(double)EPSILON)
   m_d->solDelta=(double)1.e-3;

  m_d->off0FlagFit = (atts.at("o0fit") == "true") ? 1 : 0;
  m_d->off0Initial = atts.at("o0init").toDouble();
  m_d->off0Delta = atts.at("o0delt").toDouble();
  m_d->off0FlagFitStore = (atts.at("o0fstr") == "true") ? 1 : 0;
  m_d->off0FlagErrStore = (atts.at("o0estr") == "true") ? 1 : 0;

  if (fabs(m_d->off0Delta)<(double)EPSILON)
   m_d->off0Delta=(double)1.e-3;

  m_d->off1FlagFit = (atts.at("o1fit") == "true") ? 1 : 0;
  m_d->off1Initial = atts.at("o1init").toDouble();
  m_d->off1Delta = atts.at("o1delt").toDouble();
  m_d->off1FlagFitStore = (atts.at("o1fstr") == "true") ? 1 : 0;
  m_d->off1FlagErrStore = (atts.at("o1estr") == "true") ? 1 : 0;

  if (fabs(m_d->off1Delta)<(double)EPSILON)
   m_d->off1Delta=(double)1.e-3;

  m_d->off2FlagFit = (atts.at("o2fit") == "true") ? 1 : 0;
  m_d->off2Initial = atts.at("o2init").toDouble();
  m_d->off2Delta = atts.at("o2delt").toDouble();
  m_d->off2FlagFitStore = (atts.at("o2fstr") == "true") ? 1 : 0;
  m_d->off2FlagErrStore = (atts.at("o2estr") == "true") ? 1 : 0;

  if (fabs(m_d->off2Delta)<(double)EPSILON)
   m_d->off2Delta=(double)1.e-3;

  m_d->comFlagFit = (atts.at("comfit") == "true") ? 1 : 0;
  m_d->comInitial = atts.at("cominit").toDouble();
  m_d->comDelta = atts.at("comdelt").toDouble();
  m_d->comFlagFitStore = (atts.at("comfstr") == "true") ? 1 : 0;
  m_d->comFlagErrStore = (atts.at("comestr") == "true") ? 1 : 0;

  if (fabs(m_d->comDelta)<(double)EPSILON)
   m_d->comDelta=(double)1.e-3;

  m_d->usamp1FlagFit = (atts.at("u1fit") == "true") ? 1 : 0;
  m_d->usamp1Initial = atts.at("u1init").toDouble();
  m_d->usamp1Delta = atts.at("u1delt").toDouble();
  m_d->usamp1FlagFitStore = (atts.at("u1str") == "true") ? 1 : 0;
  m_d->usamp1FlagErrStore = (atts.at("u1estr") == "true") ? 1 : 0;

  if (fabs(m_d->usamp1Delta)<(double)EPSILON)
   m_d->usamp1Delta=(double)1.e-3;

  m_d->usamp2FlagFit = (atts.at("u2fit") == "true") ? 1 : 0;
  m_d->usamp2Initial = atts.at("u2init").toDouble();
  m_d->usamp2Delta = atts.at("u2delt").toDouble();
  m_d->usamp2FlagFitStore = (atts.at("u2str") == "true") ? 1 : 0;
  m_d->usamp2FlagErrStore = (atts.at("u2estr") == "true") ? 1 : 0;

  if (fabs(m_d->usamp2Delta)<(double)EPSILON)
   m_d->usamp2Delta=(double)1.e-3;

  m_d->resolFlagFit = (atts.at("resolfit") == "true") ? 1 : 0;
  m_d->resolInitial = atts.at("resolinit").toDouble();
  m_d->resolDelta = atts.at("resoldelt").toDouble();
  m_d->resolFlagFitStore = (atts.at("resolstr") == "true") ? 1 : 0;
  m_d->resolFlagErrStore = (atts.at("resolestr") == "true") ? 1 : 0;

  if (fabs(m_d->resolDelta)<(double)EPSILON)
   m_d->resolDelta=(double)1.e-3;

  str = atts.at("comfile");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->comFile))
      strcpy(m_d->comFile, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Com filename too long");
  }

  str = atts.at("u1file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp1File))
      strcpy(m_d->usamp1File, str.toLocal8Bit().data());
    else
      throw std::runtime_error("Usamp1 filename too long");
  }

  str = atts.at("u2file");
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

    str=atts.at("shfit");

    if ((str=="true") || (str=="nonlinear")) d->shFit=ANLYS_SHIFT_TYPE_NONLINEAR;
  //  else if (str=="linear") d->shFit=ANLYS_SHIFT_TYPE_LINEAR;
    else d->shFit=ANLYS_SHIFT_TYPE_NONE;

    str = atts.at("stfit");
    if (str == "1st") d->stFit = ANLYS_STRETCH_TYPE_FIRST_ORDER;
    else if (str == "2nd") d->stFit = ANLYS_STRETCH_TYPE_SECOND_ORDER;
    else d->stFit = ANLYS_STRETCH_TYPE_NONE;

    d->shStore = (atts.at("shstr") == "true") ? 1 : 0;
    d->stStore = (atts.at("ststr") == "true") ? 1 : 0;
    d->errStore = (atts.at("errstr") == "true") ? 1 : 0;

    d->shInit = atts.at("shini").toDouble();
    d->stInit = atts.at("stini").toDouble();
    d->stInit2 = atts.at("stini2").toDouble();

    if (fabs((double)(d->shDelta = atts.at("shdel").toDouble()  ))<EPSILON) d->shDelta  =(double)1.e-3;
    if (fabs((double)(d->stDelta = atts.at("stdel").toDouble()  ))<EPSILON) d->stDelta  =(double)1.e-3;
    if (fabs((double)(d->stDelta2 = atts.at("stdel2").toDouble()))<EPSILON) d->stDelta2 =(double)1.e-3;

    d->shMin = atts.at("shmin").toDouble();
    d->shMax = atts.at("shmax").toDouble();

    return;
  }

  throw std::runtime_error("Too many cross sections in analysis window");
}

void CAnalysisWindowShiftStretchSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, QString> &atts)
{
  if (element == "symbol" && m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);

    if (d->nSymbol < MAX_AW_SHIFT_STRETCH) {

      QString str = atts.at("name");
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
    m_d->gap[m_d->nGap].minimum = atts.at("min").toDouble();
    m_d->gap[m_d->nGap].maximum = atts.at("max").toDouble();
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

    str = atts.at("sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toLocal8Bit().data());
    else
      throw std::runtime_error("missing symbol (or name too long)");

    d->amf = (atts.at("amf") == "true") ? 1 : 0;
    d->resCol = atts.at("rescol").toDouble();
    d->slantCol = (atts.at("scol") == "true") ? 1 : 0;
    d->slantErr = (atts.at("serr") == "true") ? 1 : 0;
    d->slantFactor = atts.at("sfact").toDouble();
    d->vertCol = (atts.at("vcol") == "true") ? 1 : 0;
    d->vertErr = (atts.at("verr") == "true") ? 1 : 0;
    d->vertFactor = atts.at("vfact").toDouble();

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
  int index = atts.at("index").toInt();

  if (index > 0 && index <= 4) {
    struct calibration_sfp *p = (m_d + index - 1);

    p->fitFlag = (atts.at("fit") == "true") ? 1 : 0;
    p->initialValue = atts.at("init").toDouble();
    p->deltaValue = atts.at("delta").toDouble();
    p->fitStore = (atts.at("fstr") == "true") ? 1 : 0;
    p->errStore = (atts.at("estr") == "true") ? 1 : 0;

    if (fabs(p->deltaValue)<EPSILON)
     p->deltaValue=(double)1.e-3;

    return;
  }

  throw std::runtime_error("Invalid SFP index");
}
