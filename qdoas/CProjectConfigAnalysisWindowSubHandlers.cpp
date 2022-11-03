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


#include <math.h>
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CProjectConfigItem.h"

#include "constants.h"

CAnalysisWindowSubHandler::CAnalysisWindowSubHandler(CConfigHandler *master,
                             CAnalysisWindowConfigItem *item) :
  CConfigSubHandler(master),
  m_item(item)
{
}

bool CAnalysisWindowSubHandler::start(const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (!m_item->setName(atts.value("name")))
    return postErrorMessage("Analysis window name too long.");

  m_item->setEnabled(atts.value("disable") != "true");

  QString str = atts.value("kurucz");
  if (str == "ref")
    d->kuruczMode = ANLYS_KURUCZ_REF;
  else if (str == "spec")
    d->kuruczMode = ANLYS_KURUCZ_SPEC;
  else if (str == "ref+spec")
    d->kuruczMode = ANLYS_KURUCZ_REF_AND_SPEC;
  else
    d->kuruczMode = ANLYS_KURUCZ_NONE;

  d->refSpectrumSelection = (atts.value("refsel") == "auto") ? ANLYS_REF_SELECTION_MODE_AUTOMATIC :  ANLYS_REF_SELECTION_MODE_FILE;

  d->fitMinWavelength = atts.value("min").toDouble();
  d->fitMaxWavelength = atts.value("max").toDouble();
  d->resolFwhm=(!atts.value("resol_fwhm").isEmpty())?atts.value("resol_fwhm").toDouble():0.5;
  d->lambda0=(!atts.value("lambda0").isEmpty())?atts.value("lambda0").toDouble():0.5*(d->fitMinWavelength+d->fitMaxWavelength);

  // MUST have a valid name
  return !m_item->name().isEmpty();
}

bool CAnalysisWindowSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (element == "display") {
    d->requireSpectrum = (atts.value("spectrum") == "true") ? 1 : 0;
    d->requirePolynomial = (atts.value("poly") == "true") ? 1 : 0;
    d->requireFit = (atts.value("fits") == "true") ? 1 : 0;
    d->requireResidual = (atts.value("residual") == "true") ? 1 : 0;
    d->requirePredefined = (atts.value("predef") == "true") ? 1 : 0;
    d->requireRefRatio = (atts.value("ratio") == "true") ? 1 : 0;

  }
  else if (element == "files") {
    QString str;

    str = atts.value("refone");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refOneFile))
    strcpy(d->refOneFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Reference 1 Filename too long");
    }

    str = atts.value("reftwo");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refTwoFile))
    strcpy(d->refTwoFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Reference 2 Filename too long");
    }

    str = atts.value("residual");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->residualFile))
    strcpy(d->residualFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Residual Filename too long");
    }

    d->saveResidualsFlag = (atts.value("saveresiduals") == "true") ? 1 : 0;

    d->refMinLongitude = atts.value("minlon").toDouble();
    d->refMaxLongitude = atts.value("maxlon").toDouble();
    d->refMinLatitude = atts.value("minlat").toDouble();
    d->refMaxLatitude = atts.value("maxlat").toDouble();
    d->refNs = atts.value("refns").toInt();

    if (d->refNs<=0)
     d->refNs=1;
    else if (d->refNs>50)
     d->refNs=50;

    d->cloudFractionMin = atts.value("cloudfmin").toDouble();
    d->cloudFractionMax = atts.value("cloudfmax").toDouble();

    d->refMaxdoasSelection = (atts.value("maxdoasrefmode") == "scan") ? ANLYS_MAXDOAS_REF_SCAN :  ANLYS_MAXDOAS_REF_SZA;

    str = atts.value("scanmode");
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
      d->refSzaCenter = atts.value("maxdoasszacenter").toDouble();
      d->refSzaDelta = atts.value("maxdoasszadelta").toDouble();
     }
    else
     {
      d->refSzaCenter = atts.value("szacenter").toDouble();
      d->refSzaDelta = atts.value("szadelta").toDouble();
     }
  }
  else if (element == "cross_section") {
    return m_master->installSubHandler(new CAnalysisWindowCrossSectionSubHandler(m_master, &(d->crossSectionList)), atts);
  }
  else if (element == "linear") {
    return m_master->installSubHandler(new CAnalysisWindowLinearSubHandler(m_master, &(d->linear)), atts);
  }
  else if (element == "nonlinear") {
    return m_master->installSubHandler(new CAnalysisWindowNonLinearSubHandler(m_master, &(d->nonlinear)), atts);
  }
  else if (element == "shift_stretch") {
    return m_master->installSubHandler(new CAnalysisWindowShiftStretchSubHandler(m_master, &(d->shiftStretchList)), atts);
  }
  else if (element == "gap") {
    return m_master->installSubHandler(new CAnalysisWindowGapSubHandler(m_master, &(d->gapList)), atts);
  }
  else if (element == "output") {
    return m_master->installSubHandler(new CAnalysisWindowOutputSubHandler(m_master, &(d->outputList)), atts);
  }

  return true;
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

bool CAnalysisWindowCrossSectionSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nCrossSection < MAX_AW_CROSS_SECTION) {

    QString str;
    struct anlyswin_cross_section *d = &(m_d->crossSection[m_d->nCrossSection]);

    str = atts.value("sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toLocal8Bit().data());
    else
      return postErrorMessage("missing symbol (or name too long)");

    str = atts.value("ortho");

    if (str.isEmpty())
     strcpy(d->orthogonal,"");
    else if (str.length() < (int)sizeof(d->orthogonal))
      strcpy(d->orthogonal, str.toLocal8Bit().data());
    else
       return postErrorMessage("ortho name too long");

    str = atts.value("subtract");

    if (str.isEmpty())
     strcpy(d->subtract,"");
    else if (str.length() < (int)sizeof(d->subtract))
      strcpy(d->subtract, str.toLocal8Bit().data());
    else
      return postErrorMessage("subtract name too long");

    d->subtractFlag=(atts.value("subtract_flag") == "true") ? 1 : 0;

    str = atts.value("cstype");
    if (str == "interp") d->crossType = ANLYS_CROSS_ACTION_INTERPOLATE;
    else if (str == "std") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE;
    else if (str == "io") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_I0;
    else if (str == "ring") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_RING;
    else d->crossType = ANLYS_CROSS_ACTION_NOTHING;

    str = atts.value("amftype");
    if (str == "sza") d->amfType = ANLYS_AMF_TYPE_SZA;
    else if (str == "climate") d->amfType = ANLYS_AMF_TYPE_CLIMATOLOGY;
    else if ((str == "wave") ||
             (str == "wave1") || (str == "wave2") || (str == "wave3"))          // for compatibility with previous versions (wavelength 1, 2, 3)
     d->amfType = ANLYS_AMF_TYPE_WAVELENGTH;
    else d->amfType = ANLYS_AMF_TYPE_NONE;

    str = atts.value("corrtype");
    if (str == "slope") d->correctionType = ANLYS_CORRECTION_TYPE_SLOPE;
    else if (str == "pukite") d->correctionType = ANLYS_CORRECTION_TYPE_PUKITE;
    else if (str == "molecular_ring") d->correctionType = ANLYS_CORRECTION_TYPE_MOLECULAR_RING;
    else if (str == "molecular_ring_slope") d->correctionType = ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE;
    else d->correctionType = ANLYS_CORRECTION_TYPE_NONE;

    str = atts.value("molecular_xs");
    strcpy(d->molecularRing,str.toLocal8Bit().data());

    d->requireFit = (atts.value("fit") == "true") ? 1 : 0;
    d->requireFilter = (atts.value("filter") == "true") ? 1 : 0;
    d->constrainedCc = (atts.value("cstrncc") == "true") ? 1 : 0;
    d->requireCcFit = (atts.value("ccfit") == "true") ? 1 : 0;
    d->initialCc = atts.value("icc").toDouble();
    d->deltaCc = atts.value("dcc").toDouble();
    d->ccIo = atts.value("ccio").toDouble();
    d->ccMin = atts.value("ccmin").toDouble();
    d->ccMax = atts.value("ccmax").toDouble();

    if (fabs(d->deltaCc)<(double)EPSILON)
     d->deltaCc=(double)1.e-3;

    str = atts.value("csfile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->crossSectionFile))
    strcpy(d->crossSectionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Cross Section filename too long");
    }
    else
      return postErrorMessage("Missing cross section file");

    str = atts.value("amffile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->amfFile))
    strcpy(d->amfFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("AMF filename too long");
    }

    // All OK
    ++(m_d->nCrossSection);

    return true;
  }

  return postErrorMessage("Too many cross sections in analysis window");
}

//------------------------------------------------------------

CAnalysisWindowLinearSubHandler::CAnalysisWindowLinearSubHandler(CConfigHandler *master,
                                 struct anlyswin_linear *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CAnalysisWindowLinearSubHandler::start(const QXmlAttributes &atts)
{
  m_d->xPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("xpoly"));
  m_d->xBaseOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("xbase"));
  m_d->xFlagFitStore = (atts.value("xfit") == "true") ? 1 : 0;
  m_d->xFlagErrStore = (atts.value("xerr") == "true") ? 1 : 0;

  m_d->offsetPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("offpoly"));
  m_d->offsetFlagFitStore = (atts.value("offfit") == "true") ? 1 : 0;
  m_d->offsetFlagErrStore = (atts.value("offerr") == "true") ? 1 : 0;

  m_d->offsetI0 = (atts.value("offizero") == "true") ?  1 : 0;

  return true;
}

//------------------------------------------------------------

CAnalysisWindowNonLinearSubHandler::CAnalysisWindowNonLinearSubHandler(CConfigHandler *master,
                                       struct anlyswin_nonlinear *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CAnalysisWindowNonLinearSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  m_d->solFlagFit = (atts.value("solfit") == "true") ? 1 : 0;
  m_d->solInitial = atts.value("solinit").toDouble();
  m_d->solDelta = atts.value("soldelt").toDouble();
  m_d->solFlagFitStore = (atts.value("solfstr") == "true") ? 1 : 0;
  m_d->solFlagErrStore = (atts.value("solestr") == "true") ? 1 : 0;

  if (fabs(m_d->solDelta)<(double)EPSILON)
   m_d->solDelta=(double)1.e-3;

  m_d->off0FlagFit = (atts.value("o0fit") == "true") ? 1 : 0;
  m_d->off0Initial = atts.value("o0init").toDouble();
  m_d->off0Delta = atts.value("o0delt").toDouble();
  m_d->off0FlagFitStore = (atts.value("o0fstr") == "true") ? 1 : 0;
  m_d->off0FlagErrStore = (atts.value("o0estr") == "true") ? 1 : 0;

  if (fabs(m_d->off0Delta)<(double)EPSILON)
   m_d->off0Delta=(double)1.e-3;

  m_d->off1FlagFit = (atts.value("o1fit") == "true") ? 1 : 0;
  m_d->off1Initial = atts.value("o1init").toDouble();
  m_d->off1Delta = atts.value("o1delt").toDouble();
  m_d->off1FlagFitStore = (atts.value("o1fstr") == "true") ? 1 : 0;
  m_d->off1FlagErrStore = (atts.value("o1estr") == "true") ? 1 : 0;

  if (fabs(m_d->off1Delta)<(double)EPSILON)
   m_d->off1Delta=(double)1.e-3;

  m_d->off2FlagFit = (atts.value("o2fit") == "true") ? 1 : 0;
  m_d->off2Initial = atts.value("o2init").toDouble();
  m_d->off2Delta = atts.value("o2delt").toDouble();
  m_d->off2FlagFitStore = (atts.value("o2fstr") == "true") ? 1 : 0;
  m_d->off2FlagErrStore = (atts.value("o2estr") == "true") ? 1 : 0;

  if (fabs(m_d->off2Delta)<(double)EPSILON)
   m_d->off2Delta=(double)1.e-3;

  m_d->comFlagFit = (atts.value("comfit") == "true") ? 1 : 0;
  m_d->comInitial = atts.value("cominit").toDouble();
  m_d->comDelta = atts.value("comdelt").toDouble();
  m_d->comFlagFitStore = (atts.value("comfstr") == "true") ? 1 : 0;
  m_d->comFlagErrStore = (atts.value("comestr") == "true") ? 1 : 0;

  if (fabs(m_d->comDelta)<(double)EPSILON)
   m_d->comDelta=(double)1.e-3;

  m_d->usamp1FlagFit = (atts.value("u1fit") == "true") ? 1 : 0;
  m_d->usamp1Initial = atts.value("u1init").toDouble();
  m_d->usamp1Delta = atts.value("u1delt").toDouble();
  m_d->usamp1FlagFitStore = (atts.value("u1str") == "true") ? 1 : 0;
  m_d->usamp1FlagErrStore = (atts.value("u1estr") == "true") ? 1 : 0;

  if (fabs(m_d->usamp1Delta)<(double)EPSILON)
   m_d->usamp1Delta=(double)1.e-3;

  m_d->usamp2FlagFit = (atts.value("u2fit") == "true") ? 1 : 0;
  m_d->usamp2Initial = atts.value("u2init").toDouble();
  m_d->usamp2Delta = atts.value("u2delt").toDouble();
  m_d->usamp2FlagFitStore = (atts.value("u2str") == "true") ? 1 : 0;
  m_d->usamp2FlagErrStore = (atts.value("u2estr") == "true") ? 1 : 0;

  if (fabs(m_d->usamp2Delta)<(double)EPSILON)
   m_d->usamp2Delta=(double)1.e-3;

  m_d->resolFlagFit = (atts.value("resolfit") == "true") ? 1 : 0;
  m_d->resolInitial = atts.value("resolinit").toDouble();
  m_d->resolDelta = atts.value("resoldelt").toDouble();
  m_d->resolFlagFitStore = (atts.value("resolstr") == "true") ? 1 : 0;
  m_d->resolFlagErrStore = (atts.value("resolestr") == "true") ? 1 : 0;

  if (fabs(m_d->resolDelta)<(double)EPSILON)
   m_d->resolDelta=(double)1.e-3;

  str = atts.value("comfile");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->comFile))
      strcpy(m_d->comFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Com filename too long");
  }

  str = atts.value("u1file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp1File))
      strcpy(m_d->usamp1File, str.toLocal8Bit().data());
    else
      return postErrorMessage("Usamp1 filename too long");
  }

  str = atts.value("u2file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp2File))
      strcpy(m_d->usamp2File, str.toLocal8Bit().data());
    else
      return postErrorMessage("Usamp2 filename too long");
  }

  return true;
}

//------------------------------------------------------------

CAnalysisWindowShiftStretchSubHandler::CAnalysisWindowShiftStretchSubHandler(CConfigHandler *master,
                                         shift_stretch_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CAnalysisWindowShiftStretchSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    QString str;
    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);

    str=atts.value("shfit");

    if ((str=="true") || (str=="nonlinear")) d->shFit=ANLYS_SHIFT_TYPE_NONLINEAR;
  //  else if (str=="linear") d->shFit=ANLYS_SHIFT_TYPE_LINEAR;
    else d->shFit=ANLYS_SHIFT_TYPE_NONE;

    str = atts.value("stfit");
    if (str == "1st") d->stFit = ANLYS_STRETCH_TYPE_FIRST_ORDER;
    else if (str == "2nd") d->stFit = ANLYS_STRETCH_TYPE_SECOND_ORDER;
    else d->stFit = ANLYS_STRETCH_TYPE_NONE;

    d->shStore = (atts.value("shstr") == "true") ? 1 : 0;
    d->stStore = (atts.value("ststr") == "true") ? 1 : 0;
    d->errStore = (atts.value("errstr") == "true") ? 1 : 0;

    d->shInit = atts.value("shini").toDouble();
    d->stInit = atts.value("stini").toDouble();
    d->stInit2 = atts.value("stini2").toDouble();

    if (fabs((double)(d->shDelta = atts.value("shdel").toDouble()  ))<EPSILON) d->shDelta  =(double)1.e-3;
    if (fabs((double)(d->stDelta = atts.value("stdel").toDouble()  ))<EPSILON) d->stDelta  =(double)1.e-3;
    if (fabs((double)(d->stDelta2 = atts.value("stdel2").toDouble()))<EPSILON) d->stDelta2 =(double)1.e-3;

    d->shMin = atts.value("shmin").toDouble();
    d->shMax = atts.value("shmax").toDouble();

    return true;
  }

  return postErrorMessage("Too many cross sections in analysis window");
}

bool CAnalysisWindowShiftStretchSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "symbol" && m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);

    if (d->nSymbol < MAX_AW_SHIFT_STRETCH) {

      QString str = atts.value("name");
      if (!str.isEmpty() && str.length() < (int)SYMBOL_NAME_BUFFER_LENGTH) {
    strcpy(&(d->symbol[d->nSymbol][0]), str.toLocal8Bit().data());
    ++(d->nSymbol);

    return true;
      }
    }
  }

  return false;
}

bool CAnalysisWindowShiftStretchSubHandler::end(void)
{
  if (m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH)
    ++(m_d->nShiftStretch);

  return true;
}

//------------------------------------------------------------

CAnalysisWindowGapSubHandler::CAnalysisWindowGapSubHandler(CConfigHandler *master,
                               gap_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CAnalysisWindowGapSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nGap < MAX_AW_GAP) {
    m_d->gap[m_d->nGap].minimum = atts.value("min").toDouble();
    m_d->gap[m_d->nGap].maximum = atts.value("max").toDouble();
    if (m_d->gap[m_d->nGap].minimum < m_d->gap[m_d->nGap].maximum) {
      ++(m_d->nGap);
      return true;
    }
    else
      return postErrorMessage("Invalid gap definition");
  }

  return postErrorMessage("Too many gaps defined");
}

//------------------------------------------------------------

CAnalysisWindowOutputSubHandler::CAnalysisWindowOutputSubHandler(CConfigHandler *master,
                                 output_list_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CAnalysisWindowOutputSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nOutput < MAX_AW_CROSS_SECTION) {

    QString str;
    struct anlyswin_output *d = &(m_d->output[m_d->nOutput]);

    str = atts.value("sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toLocal8Bit().data());
    else
      return postErrorMessage("missing symbol (or name too long)");

    d->amf = (atts.value("amf") == "true") ? 1 : 0;
    d->resCol = atts.value("rescol").toDouble();
    d->slantCol = (atts.value("scol") == "true") ? 1 : 0;
    d->slantErr = (atts.value("serr") == "true") ? 1 : 0;
    d->slantFactor = atts.value("sfact").toDouble();
    d->vertCol = (atts.value("vcol") == "true") ? 1 : 0;
    d->vertErr = (atts.value("verr") == "true") ? 1 : 0;
    d->vertFactor = atts.value("vfact").toDouble();

    // All OK
    ++(m_d->nOutput);

    return true;
  }

  return postErrorMessage("Too many outputs in analysis window");
}

//------------------------------------------------------------

CAnalysisWindowSfpSubHandler::CAnalysisWindowSfpSubHandler(CConfigHandler *master,
                               struct calibration_sfp *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

bool CAnalysisWindowSfpSubHandler::start(const QXmlAttributes &atts)
{
  int index = atts.value("index").toInt();

  if (index > 0 && index <= 4) {
    struct calibration_sfp *p = (m_d + index - 1);

    p->fitFlag = (atts.value("fit") == "true") ? 1 : 0;
    p->initialValue = atts.value("init").toDouble();
    p->deltaValue = atts.value("delta").toDouble();
    p->fitStore = (atts.value("fstr") == "true") ? 1 : 0;
    p->errStore = (atts.value("estr") == "true") ? 1 : 0;

    if (fabs(p->deltaValue)<EPSILON)
     p->deltaValue=(double)1.e-3;

    return true;
  }

  return postErrorMessage("Invalid SFP index");
}
