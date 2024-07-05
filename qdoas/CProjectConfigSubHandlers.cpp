
#include "CProjectConfigSubHandlers.h"
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CConfigSubHandlerUtils.h"
#include "CProjectConfigTreeNode.h"

#include "constants.h"
#include "debugutil.h"

const char *STR_IGNORE = "IGNORE";
const char *STR_STRICT = "STRICT";
const char *STR_NONSTRICT = "NONSTRICT";

static const char *TRANSMISSION_FILENAME_ERR = "Transmission function filename too long";
static const char *CALIBRATION_FILENAME_ERR = "Calibration Filename too long";

enum omi_xtrack_mode str_to_mode(const char *configstr) {
  enum omi_xtrack_mode result;
  if (! strcmp(configstr,STR_STRICT))
   result = XTRACKQF_STRICT;
  else if (! strcmp(configstr,STR_NONSTRICT))
   result = XTRACKQF_NONSTRICT;
  else
   result = XTRACKQF_IGNORE;
  return result;
}

//------------------------------------------------------------------------

CSelectorSubHandler::CSelectorSubHandler(CConfigHandler *master, data_select_list_t *selectList) :
  CConfigSubHandler(master),
  m_selectList(selectList)
{
}

bool CSelectorSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element != "field")
    return postErrorMessage("Invalid XML element");

  data_select_list_t *d = m_selectList;

  if (d->nSelected >= PRJCT_RESULTS_MAX)
    return postErrorMessage("Too many output fields");

  QString str = atts.value("name");
  if ( str == "filename" )
    d->selected[d->nSelected] = PRJCT_RESULTS_FILENAME;
  else if (str == "specno")
    d->selected[d->nSelected] = PRJCT_RESULTS_SPECNO;
  else if (str == "name")
    d->selected[d->nSelected] = PRJCT_RESULTS_NAME;
  else if (str == "date_time")
    d->selected[d->nSelected] = PRJCT_RESULTS_DATE_TIME;
  else if (str == "date")
    d->selected[d->nSelected] = PRJCT_RESULTS_DATE;
  else if (str == "time")
    d->selected[d->nSelected] = PRJCT_RESULTS_TIME;
  else if (str == "year")
    d->selected[d->nSelected] = PRJCT_RESULTS_YEAR;
  else if (str == "julian")
    d->selected[d->nSelected] = PRJCT_RESULTS_JULIAN;
  else if (str == "jdfrac")
    d->selected[d->nSelected] = PRJCT_RESULTS_JDFRAC;
  else if (str == "tifrac")
    d->selected[d->nSelected] = PRJCT_RESULTS_TIFRAC;
  else if (str == "scans")
    d->selected[d->nSelected] = PRJCT_RESULTS_SCANS;
  else if (str == "rejected")
    d->selected[d->nSelected] = PRJCT_RESULTS_NREJ;
  else if (str == "tint")
    d->selected[d->nSelected] = PRJCT_RESULTS_TINT;
  else if (str == "sza")
    d->selected[d->nSelected] = PRJCT_RESULTS_SZA;
  else if (str == "chi")
    d->selected[d->nSelected] = PRJCT_RESULTS_CHI;
  else if (str == "rms")
    d->selected[d->nSelected] = PRJCT_RESULTS_RMS;
  else if (str == "azim")
    d->selected[d->nSelected] = PRJCT_RESULTS_AZIM;
  else if (str == "tdet")
    d->selected[d->nSelected] = PRJCT_RESULTS_TDET;
  else if (str == "sky")
    d->selected[d->nSelected] = PRJCT_RESULTS_SKY;
  else if (str == "bestshift")
    d->selected[d->nSelected] = PRJCT_RESULTS_BESTSHIFT;
  else if (str == "refzm")
    d->selected[d->nSelected] = PRJCT_RESULTS_REFZM;
  else if (str == "refnumber")
    d->selected[d->nSelected] = PRJCT_RESULTS_REFNUMBER;
  else if (str == "refshift")
    d->selected[d->nSelected] = PRJCT_RESULTS_REFSHIFT;
  else if (str == "pixel")
    d->selected[d->nSelected] = PRJCT_RESULTS_PIXEL;
  else if (str == "pixel_type")
    d->selected[d->nSelected] = PRJCT_RESULTS_PIXEL_TYPE;
  else if (str == "orbit")
    d->selected[d->nSelected] = PRJCT_RESULTS_ORBIT;
  else if (str == "corner_longitudes")
    d->selected[d->nSelected] = PRJCT_RESULTS_LON_CORNERS;
  else if (str == "corner_latitudes")
    d->selected[d->nSelected] = PRJCT_RESULTS_LAT_CORNERS;
  else if (str == "longit")
    d->selected[d->nSelected] = PRJCT_RESULTS_LONGIT;
  else if (str == "latit")
    d->selected[d->nSelected] = PRJCT_RESULTS_LATIT;
  else if (str == "altit")
    d->selected[d->nSelected] = PRJCT_RESULTS_ALTIT;
  else if (str == "covar")
    d->selected[d->nSelected] = PRJCT_RESULTS_COVAR;
  else if (str == "corr")
    d->selected[d->nSelected] = PRJCT_RESULTS_CORR;
  else if (str == "cloud")
    d->selected[d->nSelected] = PRJCT_RESULTS_CLOUD;
  else if (str == "o3")
    d->selected[d->nSelected] = PRJCT_RESULTS_O3;
  else if (str == "no2")
    d->selected[d->nSelected] = PRJCT_RESULTS_NO2;
  else if (str == "cloudtopp")
    d->selected[d->nSelected] = PRJCT_RESULTS_CLOUDTOPP;
  else if (str == "los_za")
    d->selected[d->nSelected] = PRJCT_RESULTS_LOS_ZA;
  else if (str == "los_azimuth")
    d->selected[d->nSelected] = PRJCT_RESULTS_LOS_AZIMUTH;
  else if (str == "sat_height")
    d->selected[d->nSelected] = PRJCT_RESULTS_SAT_HEIGHT;
  else if (str == "sat_longitude")
    d->selected[d->nSelected] = PRJCT_RESULTS_SAT_LON;
  else if (str == "sat_latitude")
    d->selected[d->nSelected] = PRJCT_RESULTS_SAT_LAT;
  else if (str == "sat_saa")
    d->selected[d->nSelected] = PRJCT_RESULTS_SAT_SAA;
  else if (str == "sat_sza")
    d->selected[d->nSelected] = PRJCT_RESULTS_SAT_SZA;
  else if (str == "sat_vza")
    d->selected[d->nSelected] = PRJCT_RESULTS_SAT_VZA;
  else if (str == "earth_radius")
    d->selected[d->nSelected] = PRJCT_RESULTS_EARTH_RADIUS;
  else if (str == "view_elevation")
    d->selected[d->nSelected] = PRJCT_RESULTS_VIEW_ELEVATION;
  else if (str == "view_zenith")
    d->selected[d->nSelected] = PRJCT_RESULTS_VIEW_ZENITH;
  else if (str == "view_azimuth")
    d->selected[d->nSelected] = PRJCT_RESULTS_VIEW_AZIMUTH;
  else if (str == "scia_quality")
    d->selected[d->nSelected] = PRJCT_RESULTS_SCIA_QUALITY;
  else if (str == "scia_state_index")
    d->selected[d->nSelected] = PRJCT_RESULTS_SCIA_STATE_INDEX;
  else if (str == "scia_state_id")
    d->selected[d->nSelected] = PRJCT_RESULTS_SCIA_STATE_ID;
  else if (str=="startdate")
    d->selected[d->nSelected] = PRJCT_RESULTS_STARTDATE;
  else if (str=="enddate")
    d->selected[d->nSelected] = PRJCT_RESULTS_ENDDATE;
  else if ((str == "mfc_starttime") || (str=="starttime"))
    d->selected[d->nSelected] = PRJCT_RESULTS_STARTTIME;
  else if ((str == "mfc_endtime") || (str=="endtime"))
    d->selected[d->nSelected] = PRJCT_RESULTS_ENDTIME;

  else if (str == "center_wavelength")
    d->selected[d->nSelected] = PRJCT_RESULTS_LAMBDA_CENTER;

  else if (str == "scanning_angle")
    d->selected[d->nSelected] = PRJCT_RESULTS_SCANNING;
  else if ((str == "ccd_filterNumber") || (str=="filterNumber"))
    d->selected[d->nSelected] = PRJCT_RESULTS_FILTERNUMBER;
  else if ((str == "ccd_measType") || (str == "measType"))
    d->selected[d->nSelected] = PRJCT_RESULTS_MEASTYPE;
  else if (str == "ccd_headTemp")
    d->selected[d->nSelected] = PRJCT_RESULTS_CCD_HEADTEMPERATURE;
  else if (str == "cooler_status")
    d->selected[d->nSelected] = PRJCT_RESULTS_COOLING_STATUS;
  else if (str == "mirror_status")
    d->selected[d->nSelected] = PRJCT_RESULTS_MIRROR_ERROR;
  else if (str == "compass_angle")
    d->selected[d->nSelected] = PRJCT_RESULTS_COMPASS;
  else if (str == "pitch_angle")
    d->selected[d->nSelected] = PRJCT_RESULTS_PITCH;
  else if (str == "roll_angle")
    d->selected[d->nSelected] = PRJCT_RESULTS_ROLL;
  else if (str == "iter_number")
    d->selected[d->nSelected] = PRJCT_RESULTS_ITER;
  else if (str == "error_flag")
    d->selected[d->nSelected] = PRJCT_RESULTS_ERROR_FLAG;
  else if (str == "num_bands")
    d->selected[d->nSelected] = PRJCT_RESULTS_NUM_BANDS;
  else if (str == "scan_direction")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_SCANDIRECTION;
  else if (str == "gome2_mdr_number")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_MDR_NUMBER;
  else if (str == "gome2_observation_index")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_OBSERVATION_INDEX;
  else if (str == "gome2_observation_mode")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_OBSERVATION_MODE;
  else if (str == "saa_flag")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_SAA;
  else if (str == "sunglint_danger_flag")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_SUNGLINT_RISK;
  else if (str == "sunglint_highdanger_flag")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK;
  else if (str == "rainbow_flag")
    d->selected[d->nSelected] = PRJCT_RESULTS_GOME2_RAINBOW;
  else if (str == "diodes")
    d->selected[d->nSelected] = PRJCT_RESULTS_CCD_DIODES;
  else if (str == "target_azimuth")
    d->selected[d->nSelected] = PRJCT_RESULTS_CCD_TARGETAZIMUTH;
  else if (str == "target_elevation")
    d->selected[d->nSelected] = PRJCT_RESULTS_CCD_TARGETELEVATION;
  else if (str == "saturated")
    d->selected[d->nSelected] = PRJCT_RESULTS_SATURATED;
  else if (str == "omi_index_swath" || str == "index_alongtrack")
    d->selected[d->nSelected] = PRJCT_RESULTS_INDEX_ALONGTRACK;
  else if (str == "omi_index_row" || str == "index_crosstrack")
    d->selected[d->nSelected] = PRJCT_RESULTS_INDEX_CROSSTRACK;
  else if ((str == "groundp_qf") || (str == "omi_groundp_qf"))
    d->selected[d->nSelected] = PRJCT_RESULTS_GROUNDP_QF;
  else if ((str == "xtrack_qf") || (str == "omi_xtrack_qf"))
    d->selected[d->nSelected] = PRJCT_RESULTS_XTRACK_QF;
  else if ((str == "pixels_qf") || (str == "omi_pixels_qf"))
    d->selected[d->nSelected] = PRJCT_RESULTS_PIXELS_QF;
  else if (str == "omi_configuration_id")
    d->selected[d->nSelected] = PRJCT_RESULTS_OMI_CONFIGURATION_ID;
  else if (str == "spike_pixels")
    d->selected[d->nSelected] = PRJCT_RESULTS_SPIKES;
  else if (str == "servo_byte_sent")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_SERVO_BYTE_SENT;
  else if (str == "servo_byte_received")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED;
  else if (str == "uav_inside_temp")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_INSIDE_TEMP;
  else if (str == "uav_outside_temp")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_OUTSIDE_TEMP;
  else if (str == "uav_pressure")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_PRESSURE;
  else if (str == "uav_humidity")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_HUMIDITY;
  else if (str == "uav_dewpoint")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_DEWPOINT;
  else if (str == "uav_pitch")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_PITCH;
  else if (str == "uav_roll")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_ROLL;
  else if (str == "uav_heading")
    d->selected[d->nSelected] = PRJCT_RESULTS_UAV_HEADING;

  else if (str == "gps_starttime")
    d->selected[d->nSelected] = PRJCT_RESULTS_STARTGPSTIME;
  else if (str == "gps_endtime")
    d->selected[d->nSelected] = PRJCT_RESULTS_ENDGPSTIME;
  else if (str == "longitude_end")
    d->selected[d->nSelected] = PRJCT_RESULTS_LONGITEND;
  else if (str == "latitude_end")
    d->selected[d->nSelected] = PRJCT_RESULTS_LATITEND;
  else if (str == "altitude_end")
    d->selected[d->nSelected] = PRJCT_RESULTS_ALTITEND;
  else if (str == "total_exp_time")
   d->selected[d->nSelected] = PRJCT_RESULTS_TOTALEXPTIME;
  else if (str == "total_acq_time")
   d->selected[d->nSelected] = PRJCT_RESULTS_TOTALACQTIME;
  else if (str == "lambda")
    d->selected[d->nSelected] = PRJCT_RESULTS_LAMBDA;
  else if (str == "spectra")
    d->selected[d->nSelected] = PRJCT_RESULTS_SPECTRA;

  else if (str == "precalculated_fluxes")
    d->selected[d->nSelected] = PRJCT_RESULTS_PRECALCULATED_FLUXES;
  else if (str == "scan_index")
    d->selected[d->nSelected] = PRJCT_RESULTS_SCANINDEX;
  else if (str == "zenith_before_index")
    d->selected[d->nSelected] = PRJCT_RESULTS_ZENITH_BEFORE;
  else if (str == "zenith_after_index")
    d->selected[d->nSelected] = PRJCT_RESULTS_ZENITH_AFTER;
  else if (str == "rc")
    d->selected[d->nSelected] = PRJCT_RESULTS_RC;
  else if (str == "residual_spectrum")
    d->selected[d->nSelected] = PRJCT_RESULTS_RESIDUAL_SPECTRUM;
  else
    return postErrorMessage("Invalid output field " + str);

  // MUST be ok ...
  ++(d->nSelected);

  return true;
}


//------------------------------------------------------------------------
// handler for <display> (child of project)

CProjectDisplaySubHandler::CProjectDisplaySubHandler(CConfigHandler *master, mediate_project_display_t *display) :
  CSelectorSubHandler(master, &(display->selection)),
  m_display(display)
{
}

bool CProjectDisplaySubHandler::start(const QXmlAttributes &atts)
{
  m_display->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
  m_display->requireData = (atts.value("data") == "true") ? 1 : 0;
  m_display->requireCalib = (atts.value("calib") == "true") ? 1 : 0;
  m_display->requireFits = (atts.value("fits") == "true") ? 1 : 0;

  return true;
}

//------------------------------------------------------------------------
// handler for <selection> (child of project)

CProjectSelectionSubHandler::CProjectSelectionSubHandler(CConfigHandler *master, mediate_project_selection_t *selection) :
  CConfigSubHandler(master),
  m_selection(selection)
{
}

bool CProjectSelectionSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "sza") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->szaMinimum = atts.value("min").toDouble();
    m_selection->szaMaximum = atts.value("max").toDouble();
    m_selection->szaDelta = atts.value("delta").toDouble();

  }
  if (element == "elevation") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->elevationMinimum = atts.value("min").toDouble();
    m_selection->elevationMaximum = atts.value("max").toDouble();
    m_selection->elevationTolerance = atts.value("tol").toDouble();

  }
  if (element == "reference") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->refAngle = atts.value("angle").toDouble();
    m_selection->refTolerance = atts.value("tol").toDouble();

  }
  else if (element == "record") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->recordNumberMinimum = atts.value("min").toInt();
    m_selection->recordNumberMaximum = atts.value("max").toInt();
  }
  else if (element == "cloud")
   {
    m_selection->cloudFractionMinimum = atts.value("min").toDouble();
    m_selection->cloudFractionMaximum = atts.value("max").toDouble();
   }
  else if (element == "circle") {

    m_selection->geo.circle.radius = atts.value("radius").toDouble();
    m_selection->geo.circle.centerLongitude = atts.value("long").toDouble();
    m_selection->geo.circle.centerLatitude = atts.value("lat").toDouble();
  }
  else if (element == "rectangle") {

    m_selection->geo.rectangle.easternLongitude = atts.value("east").toDouble();
    m_selection->geo.rectangle.westernLongitude = atts.value("west").toDouble();
    m_selection->geo.rectangle.northernLatitude = atts.value("north").toDouble();
    m_selection->geo.rectangle.southernLatitude = atts.value("south").toDouble();
  }
  else if (element == "sites") {

    m_selection->geo.sites.radius = atts.value("radius").toDouble();
  }
  else if (element == "geolocation") {

    QString selected = atts.value("selected");

    if (selected == "circle")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_CIRCLE;
    else if (selected == "rectangle")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_RECTANGLE;
    else if (selected == "sites")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_OBSLIST;
    else
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_NONE; // default and "none"
  }

  return true;
}


//------------------------------------------------------------------------
// handler for <analysis> (child of project)

CProjectAnalysisSubHandler::CProjectAnalysisSubHandler(CConfigHandler *master,
                               mediate_project_analysis_t *analysis) :
  CConfigSubHandler(master),
  m_analysis(analysis)
{
}

bool CProjectAnalysisSubHandler::start(const QXmlAttributes &atts)
{
  // all options are in the attributes of the analysis element itself

  QString str;

  str = atts.value("method");
  if (str == "ODF")
    m_analysis->methodType = OPTICAL_DENSITY_FIT;
  else if (str == "ML+SVD")
    m_analysis->methodType = INTENSITY_FIT;
  else
    return postErrorMessage("Invalid analysis method");

  str = atts.value("fit");
  if (str == "none")
    m_analysis->fitType = PRJCT_ANLYS_FIT_WEIGHTING_NONE;
  else if (str == "instr")
    m_analysis->fitType = PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL;
  else
    return postErrorMessage("Invalid analysis fit");

  str = atts.value("interpolation");
  if (str == "linear")
    m_analysis->interpolationType = PRJCT_ANLYS_INTERPOL_LINEAR;
  else if (str == "spline")
    m_analysis->interpolationType = PRJCT_ANLYS_INTERPOL_SPLINE;
  else
    return postErrorMessage("Invalid analysis interpolation");

  m_analysis->interpolationSecurityGap = atts.value("gap").toInt();
  m_analysis->maxIterations=atts.value("max_iterations").toInt();
  m_analysis->convergenceCriterion = atts.value("converge").toDouble();
  if (atts.value("spike_tolerance") != "")
    m_analysis->spike_tolerance = atts.value("spike_tolerance").toDouble();

  return true;
}


//------------------------------------------------------------------------
// handler for <raw_spectra> (child of project)

CProjectRawSpectraSubHandler::CProjectRawSpectraSubHandler(CConfigHandler *master,
                               CProjectConfigTreeNode *node) :
  CConfigSubHandler(master),
  m_node(node)
{
}

bool CProjectRawSpectraSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // <directory>, <file> or <folder>

  QString name = atts.value("name");
  bool enabled = (atts.value("disabled") != "true");

  // MUST have a name ...
  if (name.isEmpty())
    return false;

  if (element == "file") {
    // expand the name (the filename)
    name = m_master->pathExpand(name);
    m_node->addChild(new CProjectConfigFile(name, enabled));
  }
  else if (element == "directory") {

    // expand the name (the directory name)
    name = m_master->pathExpand(name);
    m_node->addChild(new CProjectConfigDirectory(name, atts.value("filters"),
                         (atts.value("recursive") == "true"), enabled));
  }
  else if (element == "folder") {
    // create an item for the folder now ...
    CProjectConfigFolder *item = new CProjectConfigFolder(name, enabled);
    m_node->addChild(item);

    // and a sub handler for child items
    return m_master->installSubHandler(new CProjectRawSpectraSubHandler(m_master, item), atts);
  }

  return true;
}


//------------------------------------------------------------------------
// handler for <calibration> (child of project)

CProjectCalibrationSubHandler::CProjectCalibrationSubHandler(CConfigHandler *master,
                                 mediate_project_calibration_t *calibration) :
  CConfigSubHandler(master),
  m_calibration(calibration)
{
}

bool CProjectCalibrationSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("method");
  if (str == "ODF")
    m_calibration->methodType = OPTICAL_DENSITY_FIT;
  else if (str == "ML+SVD")
    m_calibration->methodType = INTENSITY_FIT;
  else
    return postErrorMessage("Invalid analysis method");

  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_calibration->solarRefFile))
      strcpy(m_calibration->solarRefFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }

  return true;
}

bool CProjectCalibrationSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  QString str;
  // sub element of calibration

  if (element == "line") {
    str = atts.value("shape");
    if (str == "none")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_NONE;
    else if (str == "file")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_FILE;
    else if (str == "gauss")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_GAUSS;
    else if (str == "error")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_ERF;
    else if (str == "agauss")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_AGAUSS;
    else if (str == "supergauss")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_SUPERGAUSS;
    else if (str == "lorentz")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_INVPOLY;
    else if (str == "voigt")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_VOIGT;
    else
      postErrorMessage("Invalid line shape");

    if (atts.value("lorentzorder") != "") {
      m_calibration->lorentzOrder = atts.value("lorentzorder").toInt();
    } else {
      m_calibration->lorentzOrder = atts.value("lorentzdegree").toInt()/2;
    }

    str = atts.value("slfFile");
    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_calibration->slfFile))
        strcpy(m_calibration->slfFile, str.toLocal8Bit().data());
      else
        return postErrorMessage("Slit function Filename too long");
     }
  }
  else if (element == "display") {

    m_calibration->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
    m_calibration->requireFits = (atts.value("fits") == "true") ? 1 : 0;
    m_calibration->requireResidual = (atts.value("residual") == "true") ? 1 : 0;
    m_calibration->requireShiftSfp = (atts.value("shiftsfp") == "true") ? 1 : 0;
  }
  else if (element == "polynomial") {

    m_calibration->shiftDegree = atts.value("shift").toInt();
    m_calibration->sfpDegree = atts.value("sfp").toInt();
  }
  else if (element == "window") {

    m_calibration->wavelengthMin = atts.value("min").toDouble();
    m_calibration->wavelengthMax = atts.value("max").toDouble();
    m_calibration->windowSize = atts.value("size").toDouble();
    m_calibration->subWindows = atts.value("intervals").toInt();

    const QByteArray& ascii=atts.value("custom_windows").toLocal8Bit();
    const char *ptr = ascii.data();
    int i=0;
    double lambdaMin,lambdaMax;

    m_calibration->customLambdaMin[0]=m_calibration->wavelengthMin;
    m_calibration->customLambdaMax[m_calibration->subWindows-1]=m_calibration->wavelengthMax;

    while ((ptr!=NULL) && (i<m_calibration->subWindows)) {
      sscanf(ptr,"%lf-%lf",&lambdaMin,&lambdaMax);
      if (i>0)
        m_calibration->customLambdaMin[i]=lambdaMin;
      if (i<m_calibration->subWindows-1)
        m_calibration->customLambdaMax[i]=lambdaMax;

      if ((ptr=strchr(ptr,','))!=NULL)
        ptr++;

      i++;
    }

    str = atts.value("division");
    if (str == "sliding")
     m_calibration->divisionMode = PRJCT_CALIB_WINDOWS_SLIDING;
    else if (str == "custom")
     m_calibration->divisionMode = PRJCT_CALIB_WINDOWS_CUSTOM;
    else
     m_calibration->divisionMode = PRJCT_CALIB_WINDOWS_CONTIGUOUS;


  }
  else if (element == "preshift") {
    m_calibration->preshiftFlag = (atts.value("calculate") == "true") ? 1 : 0;
    m_calibration->preshiftMin = atts.value("min").toDouble();
    m_calibration->preshiftMax = atts.value("max").toDouble();
  }
  else if (element == "cross_section") {
    return m_master->installSubHandler(new CAnalysisWindowCrossSectionSubHandler(m_master, &(m_calibration->crossSectionList)), atts);
  }
  else if (element == "linear") {
    return m_master->installSubHandler(new CAnalysisWindowLinearSubHandler(m_master, &(m_calibration->linear)), atts);
  }
  else if (element == "sfp") {
    return m_master->installSubHandler(new CAnalysisWindowSfpSubHandler(m_master, &(m_calibration->sfp[0])), atts);
  }
  else if (element == "shift_stretch") {
    return m_master->installSubHandler(new CAnalysisWindowShiftStretchSubHandler(m_master, &(m_calibration->shiftStretchList)), atts);
  }
  else if (element == "gap") {
    return m_master->installSubHandler(new CAnalysisWindowGapSubHandler(m_master, &(m_calibration->gapList)), atts);
  }
  else if (element == "output") {
    return m_master->installSubHandler(new CAnalysisWindowOutputSubHandler(m_master, &(m_calibration->outputList)), atts);
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <undersampling> (child of project)

CProjectUndersamplingSubHandler::CProjectUndersamplingSubHandler(CConfigHandler *master,
                                 mediate_project_undersampling_t *undersampling) :
  CConfigSubHandler(master),
  m_undersampling(undersampling)
{
}

bool CProjectUndersamplingSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("method");
  if (str == "file")
    m_undersampling->method = PRJCT_USAMP_FILE;
  else if (str == "fixed")
    m_undersampling->method = PRJCT_USAMP_FIXED;
  else if (str == "auto")
    m_undersampling->method = PRJCT_USAMP_AUTOMATIC;
  else
    return postErrorMessage("Invalid analysis method");

  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_undersampling->solarRefFile))
      strcpy(m_undersampling->solarRefFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }

  m_undersampling->shift = atts.value("shift").toDouble();

  return true;
}

//------------------------------------------------------------------------
// handler for <instrumental> (child of project)

CProjectInstrumentalSubHandler::CProjectInstrumentalSubHandler(CConfigHandler *master,
                                   mediate_project_instrumental_t *instrumental) :
  CConfigSubHandler(master),
  m_instrumental(instrumental)
{
}

bool CProjectInstrumentalSubHandler::start(const QXmlAttributes &atts)
{
  // the <instrumental ...> element

  QString str;

  str = atts.value("format");
  if (str == "ascii")
    m_instrumental->format = PRJCT_INSTR_FORMAT_ASCII;
#ifdef PRJCT_INSTR_FORMAT_OLD
  else if (str == "logger")
    m_instrumental->format = PRJCT_INSTR_FORMAT_LOGGER;
  else if (str == "acton")
    m_instrumental->format = PRJCT_INSTR_FORMAT_ACTON;
  else if (str == "pdaegg")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDAEGG;
  else if (str == "pdaeggold")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDAEGG_OLD;
  else if (str == "ccdohp_96")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_OHP_96;
  else if (str == "ccdha_94")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_HA_94;
#endif
  else if (str == "saozvis")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SAOZ_VIS;
  else if (str == "saozefm")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SAOZ_EFM;
  else if (str == "biraairborne")
    m_instrumental->format = PRJCT_INSTR_FORMAT_BIRA_AIRBORNE;
  else if (str == "biramobile")
    m_instrumental->format = PRJCT_INSTR_FORMAT_BIRA_MOBILE;
  else if (str == "apex")
    m_instrumental->format = PRJCT_INSTR_FORMAT_APEX;
  else if (str == "gems")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GEMS;
  else if (str == "mfc")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MFC;
  else if (str == "mfcstd")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MFC_STD;
  else if (str == "mfcbira")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MFC_BIRA;
#ifdef PRJCT_INSTR_FORMAT_OLD
  else if (str == "rasas")
    m_instrumental->format = PRJCT_INSTR_FORMAT_RASAS;
  else if (str == "pdasieasoe")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDASI_EASOE;
#endif
  else if (str == "ccdeev")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_EEV;
  else if (str == "gdpbin")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GDP_BIN;
  else if (str == "sciapds")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SCIA_PDS;
  else if (str == "uoft")
    m_instrumental->format = PRJCT_INSTR_FORMAT_UOFT;
  else if (str == "noaa")
    m_instrumental->format = PRJCT_INSTR_FORMAT_NOAA;
  else if (str == "omi")
    m_instrumental->format = PRJCT_INSTR_FORMAT_OMI;
  else if (str == "omps")
    m_instrumental->format = PRJCT_INSTR_FORMAT_OMPS;
  else if (str == "tropomi")
    m_instrumental->format = PRJCT_INSTR_FORMAT_TROPOMI;
  else if (str == "gome2")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GOME2;
  else if (str == "mkzy")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MKZY;
  else if (str == "oceanoptics")
    m_instrumental->format = PRJCT_INSTR_FORMAT_OCEAN_OPTICS;
  else if (str == "frm4doas")
    m_instrumental->format = PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF;
  else if (str == "gdpnetcdf")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GOME1_NETCDF;
  else
    return postErrorMessage("Invalid instrumental format");

  str = atts.value("site");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_instrumental->siteName))
      strcpy(m_instrumental->siteName, str.toLocal8Bit().data());
    else
      return postErrorMessage("Instrumental Site Name too long");
  }

  str = atts.value("saa_convention");
  m_instrumental->saaConvention=(!str.isEmpty() && (str=="0-north"))?PRJCT_INSTR_SAA_NORTH:PRJCT_INSTR_SAA_SOUTH;

  return true;
}

bool CProjectInstrumentalSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // format specific children of instrumental

  if (element == "ascii") { // ASCII
    QString str;

    m_instrumental->ascii.detectorSize = atts.value("size").toInt();

    str = atts.value("format");
    if (str == "line")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_LINE;
    else if (str == "column")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_COLUMN;
    else if (str == "column_extended")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED;
    else
      return postErrorMessage("Invalid ascii format");

    m_instrumental->ascii.flagZenithAngle = (atts.value("zen") == "true") ? 1 : 0;
    m_instrumental->ascii.flagAzimuthAngle = (atts.value("azi") == "true") ? 1 : 0;
    m_instrumental->ascii.flagElevationAngle = (atts.value("ele") == "true") ? 1 : 0;
    m_instrumental->ascii.flagDate = (atts.value("date") == "true") ? 1 : 0;
    m_instrumental->ascii.flagTime = (atts.value("time") == "true") ? 1 : 0;
    m_instrumental->ascii.flagWavelength = (atts.value("lambda") == "true") ? 1 : 0;
    m_instrumental->ascii.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->ascii.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->ascii.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ascii.calibrationFile))
    strcpy(m_instrumental->ascii.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ascii.transmissionFunctionFile))
    strcpy(m_instrumental->ascii.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }

  }
#ifdef PRJCT_INSTR_FORMAT_OLD
  else if (element == "logger") { // LOGGER
    return helperLoadLogger(atts, &(m_instrumental->logger));

  }
  else if (element == "acton") { // ACTON
    QString str;

    str = atts.value("type");
    if (str == "old")
      m_instrumental->acton.niluType = PRJCT_INSTR_NILU_FORMAT_OLD;
    else if (str == "new")
      m_instrumental->acton.niluType = PRJCT_INSTR_NILU_FORMAT_NEW;
    else
      return postErrorMessage("Invalid acton Type");

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->acton.calibrationFile))
    strcpy(m_instrumental->acton.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->acton.transmissionFunctionFile))
    strcpy(m_instrumental->acton.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }

  }
  else if (element == "pdaegg") { // PDA EGG
    return helperLoadLogger(atts, &(m_instrumental->pdaegg));

  }
  else if (element == "pdaeggold") { // PDA EGG OLD
    return helperLoadLogger(atts, &(m_instrumental->pdaeggold));

  }
  else if (element == "ccdohp96") { // CCD OHP 96
    return helperLoadCcd(atts, &(m_instrumental->ccdohp96));

  }
  else if (element == "ccdha94") { // CCD HA 94
    return helperLoadCcd(atts, &(m_instrumental->ccdha94));

  }
#endif
  else if (element == "saozvis") { // SAOZ VIS
    return helperLoadSaoz(atts, &(m_instrumental->saozvis));

  }
  else if (element == "saozefm") { // SAOZ EFM
    m_instrumental->saozefm.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->saozefm.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->saozefm.lambdaMax = atts.value("lambda_max").toDouble();
    return helperLoadMinimum(atts, &(m_instrumental->saozefm));

  }
  else if (element == "mfc") { // MFC
    QString str;

    m_instrumental->mfc.detectorSize = atts.value("size").toInt();
    m_instrumental->mfc.firstWavelength = atts.value("first").toInt();

    m_instrumental->mfc.revert = (atts.value("revert") == "true") ? 1 : 0;
    m_instrumental->mfc.autoFileSelect = (atts.value("auto") == "true") ? 1 : 0;
    m_instrumental->mfc.offsetMask = atts.value("omask").toUInt();
    m_instrumental->mfc.instrFctnMask = atts.value("imask").toUInt();
    m_instrumental->mfc.darkCurrentMask = atts.value("dmask").toUInt();
    m_instrumental->mfc.spectraMask = atts.value("smask").toUInt();

    m_instrumental->mfc.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->mfc.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->mfc.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.calibrationFile))
    strcpy(m_instrumental->mfc.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.transmissionFunctionFile))
    strcpy(m_instrumental->mfc.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }
    str = atts.value("dark");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.darkCurrentFile))
    strcpy(m_instrumental->mfc.darkCurrentFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Dark Current Filename too long");
    }

    str = atts.value("offset");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.offsetFile))
    strcpy(m_instrumental->mfc.offsetFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Offset Filename too long");
    }

  }
  else if (element == "mfcstd") { // MFC STD
    QString str;

    m_instrumental->mfcstd.detectorSize = atts.value("size").toInt();
    m_instrumental->mfcstd.revert = (atts.value("revert") == "true") ? 1 : 0;
    m_instrumental->mfcstd.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->mfcstd.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->mfcstd.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("date");
    if (!str.isEmpty()) {
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.dateFormat))
        strcpy(m_instrumental->mfcstd.dateFormat, str.toLocal8Bit().data());
      else
        return postErrorMessage("Date format too long");
     }

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.calibrationFile))
    strcpy(m_instrumental->mfcstd.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.transmissionFunctionFile))
    strcpy(m_instrumental->mfcstd.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }
    str = atts.value("dark");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.darkCurrentFile))
    strcpy(m_instrumental->mfcstd.darkCurrentFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Dark Current Filename too long");
    }

    str = atts.value("offset");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.offsetFile))
    strcpy(m_instrumental->mfcstd.offsetFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Offset Filename too long");
    }
  }
  else if (element == "mfcbira") { // MFC bira
    QString str;

    m_instrumental->mfcbira.detectorSize = atts.value("size").toInt();
    m_instrumental->mfcbira.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->mfcbira.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->mfcbira.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcbira.calibrationFile))
    strcpy(m_instrumental->mfcbira.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcbira.transmissionFunctionFile))
    strcpy(m_instrumental->mfcbira.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }
  }

#ifdef PRJCT_INSTR_FORMAT_OLD
  else if (element == "rasas") { // RASAS
    m_instrumental->rasas.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->rasas.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->rasas.lambdaMax = atts.value("lambda_max").toDouble();
    return helperLoadMinimum(atts, &(m_instrumental->rasas));

  }
  else if (element == "pdasieasoe") { // PDASI EASOE
    m_instrumental->pdasieasoe.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->pdasieasoe.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->pdasieasoe.lambdaMax = atts.value("lambda_max").toDouble();
    return helperLoadMinimum(atts, &(m_instrumental->pdasieasoe));

  }
#endif
  else if (element == "ccdeev") { // CCD EEV
    QString str;

    m_instrumental->ccdeev.detectorSize = atts.value("size").toInt();
    m_instrumental->ccdeev.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->ccdeev.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->ccdeev.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.calibrationFile))
    strcpy(m_instrumental->ccdeev.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.transmissionFunctionFile))
    strcpy(m_instrumental->ccdeev.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }

    str = atts.value("image");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.imagePath))
    strcpy(m_instrumental->ccdeev.imagePath, str.toLocal8Bit().data());
      else
    return postErrorMessage("Stray Light Correction Filename too long");
    }

    str = atts.value("stray");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.straylightCorrectionFile))
    strcpy(m_instrumental->ccdeev.straylightCorrectionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Stray Light Correction Filename too long");
    }

    str = atts.value("dnl");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.detectorNonLinearityFile))
    strcpy(m_instrumental->ccdeev.detectorNonLinearityFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Detector Non-Linearity Filename too long");
    }

    str = atts.value("type");

    if (!str.isEmpty())
         {
      if (str == "all")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_NONE;
      else if (str == "off-axis")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;
      else if (str == "direct-sun")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN;
      else if (str == "almucantar")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR;
      else
        return postErrorMessage("Invalid ccdeev Type");
     }
  }
  else if (element == "gdpnetcdf") { // GDP netCDF
    helperLoadGdp(atts, &(m_instrumental->gdpnetcdf));

  }
  else if (element == "gdpbin") { // GDP BIN
    helperLoadGdp(atts, &(m_instrumental->gdpbin));

  }
  else if (element == "sciapds") { // SCIA PDS
    helperLoadScia(atts, &(m_instrumental->sciapds));

  }
  else if (element == "uoft") { // UOFT
    m_instrumental->uoft.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->uoft.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->uoft.lambdaMax = atts.value("lambda_max").toDouble();
    return helperLoadMinimum(atts, &(m_instrumental->uoft));

  }
  else if (element == "noaa") { // NOAA
    m_instrumental->noaa.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->noaa.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->noaa.lambdaMax = atts.value("lambda_max").toDouble();
    return helperLoadMinimum(atts, &(m_instrumental->noaa));

  }
  else if (element == "omi") { // OMI
    QString str;

    str = atts.value("type");
    if (str == "uv1")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV1;
    else if (str == "uv2")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV2;
    else if (str == "vis")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_VIS;
    else
      return postErrorMessage("Invalid omi Spectral Type");

    m_instrumental->omi.minimumWavelength = atts.value("min").toDouble();
    m_instrumental->omi.maximumWavelength = atts.value("max").toDouble();
    m_instrumental->omi.flagAverage = (atts.value("ave") == "true") ? 1 : 0;

    str = atts.value("trackSelection");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->omi.trackSelection))
    strcpy(m_instrumental->omi.trackSelection, str.toLocal8Bit().data());
      else
    return postErrorMessage("Track selection string too long");
    }

    str = atts.value("xTrackMode");
    m_instrumental->omi.xtrack_mode = str_to_mode(str.toLocal8Bit().constData());

    m_instrumental->omi.pixelQFRejectionFlag = (atts.value("pixelQF_rejectionFlag") == "true") ? 1 : 0;

    str = atts.value("pixelQF_maxGaps");
    m_instrumental->omi.pixelQFMaxGaps = (!str.isEmpty())?atts.value("pixelQF_maxGaps").toInt():5;
    m_instrumental->omi.pixelQFMask = atts.value("pixelQF_mask").toInt();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->omi.calibrationFile))
    strcpy(m_instrumental->omi.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(CALIBRATION_FILENAME_ERR);
    }

    str = atts.value("transmission");
    if (str.isEmpty())
      str = atts.value("instr"); // check for old config files
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->omi.transmissionFunctionFile))
    strcpy(m_instrumental->omi.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage(TRANSMISSION_FILENAME_ERR);
    }

  }
  else if (element == "tropomi") {
    QString str = atts.value("band");
#define EXPAND(BAND, LABEL)                      \
    if(str == LABEL || str == #BAND)                 \
      m_instrumental->tropomi.spectralBand = BAND;   \
    else
    TROPOMI_BANDS
#undef EXPAND
      return postErrorMessage("No Tropomi spectral band configured.");

    str = atts.value("reference_orbit_dir");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.reference_orbit_dir))
    strcpy(m_instrumental->tropomi.reference_orbit_dir, str.toLocal8Bit().data());
      else
    return postErrorMessage("Tropomi reference orbit directory name too long");
    }

    str = atts.value("trackSelection");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->tropomi.trackSelection))
    strcpy(m_instrumental->tropomi.trackSelection, str.toLocal8Bit().data());
      else
    return postErrorMessage("Track selection string too long");
    }


    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.calibrationFile))
    strcpy(m_instrumental->tropomi.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.instrFunctionFile))
    strcpy(m_instrumental->tropomi.instrFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Instrument Function  Filename too long");
    }
  }
  else if (element == "apex") {
    QString str = atts.value("trackSelection");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->apex.trackSelection))
    strcpy(m_instrumental->apex.trackSelection, str.toLocal8Bit().data());
      else
    return postErrorMessage("Track selection string too long");
    }


    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.calibrationFile))
    strcpy(m_instrumental->tropomi.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.instrFunctionFile))
    strcpy(m_instrumental->tropomi.instrFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Instrument Function  Filename too long");
    }
  }
  else if (element == "gome2") { // GOME2
    helperLoadGome2(atts, &(m_instrumental->gome2));
  }
  else if (element == "mkzy") { // MKZY
    m_instrumental->mkzy.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->mkzy.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->mkzy.lambdaMax = atts.value("lambda_max").toDouble();
    return helperLoadMinimum(atts, &(m_instrumental->mkzy));
  }
  else if (element == "biraairborne") {  // BIRA AIRBORNE
    QString str;

    m_instrumental->biraairborne.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->biraairborne.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->biraairborne.lambdaMax = atts.value("lambda_max").toDouble();

    m_instrumental->biraairborne.detectorSize = atts.value("size").toInt();

    if (!m_instrumental->biraairborne.detectorSize)
     m_instrumental->biraairborne.detectorSize=2048;

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biraairborne.calibrationFile))
          strcpy(m_instrumental->biraairborne.calibrationFile, str.toLocal8Bit().data());
      else
          return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biraairborne.transmissionFunctionFile))
          strcpy(m_instrumental->biraairborne.transmissionFunctionFile, str.toLocal8Bit().data());
      else
          return postErrorMessage("Instrument Function  Filename too long");
    }
  }
  else if (element == "biramobile") {  // BIRA MOBILE
    QString str;

    m_instrumental->biramobile.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->biramobile.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->biramobile.lambdaMax = atts.value("lambda_max").toDouble();

    m_instrumental->biramobile.detectorSize = atts.value("size").toInt();

    if (!m_instrumental->biramobile.detectorSize)
     m_instrumental->biramobile.detectorSize=2048;

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biramobile.calibrationFile))
          strcpy(m_instrumental->biramobile.calibrationFile, str.toLocal8Bit().data());
      else
          return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biramobile.transmissionFunctionFile))
          strcpy(m_instrumental->biramobile.transmissionFunctionFile, str.toLocal8Bit().data());
      else
          return postErrorMessage("Instrument Function  Filename too long");
    }
  }
  else if (element == "oceanoptics") { // OCEAN OPTICS
    QString str;

    m_instrumental->oceanoptics.detectorSize = atts.value("size").toInt();
    m_instrumental->oceanoptics.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->oceanoptics.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->oceanoptics.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->oceanoptics.calibrationFile))
    strcpy(m_instrumental->oceanoptics.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->oceanoptics.transmissionFunctionFile))
    strcpy(m_instrumental->oceanoptics.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Instrument Function  Filename too long");
    }
  }
    else if (element == "gems") {
     QString str;
    str = atts.value("trackSelection");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->gems.trackSelection))
        strcpy(m_instrumental->gems.trackSelection, str.toLocal8Bit().data());
      else
        return postErrorMessage("Track selection string too long");
    str = atts.value("binning");
    m_instrumental->gems.binning=(!str.isEmpty())?str.toInt():4;

    }


    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->gems.calibrationFile))
        strcpy(m_instrumental->gems.calibrationFile, str.toLocal8Bit().data());
      else
        return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->gems.transmissionFunctionFile))
        strcpy(m_instrumental->gems.transmissionFunctionFile, str.toLocal8Bit().data());
      else
        return postErrorMessage("Instrument Function  Filename too long");
    }
  }
  else if (element == "frm4doas") { // FRM4DOAS netCDF
    QString str;

    str = atts.value("average_rows");

    if (!str.isEmpty())
     m_instrumental->frm4doas.averageRows = (atts.value("average_rows") == "true") ? 1 : 0;
    else
     m_instrumental->frm4doas.averageRows=1;

    m_instrumental->frm4doas.detectorSize = atts.value("size").toInt();
    m_instrumental->frm4doas.straylight = (atts.value("straylight") == "true") ? 1 : 0;
    m_instrumental->frm4doas.lambdaMin = atts.value("lambda_min").toDouble();
    m_instrumental->frm4doas.lambdaMax = atts.value("lambda_max").toDouble();

    str = atts.value("type");

    if (!str.isEmpty())
      {
      if (str == "all")
        m_instrumental->frm4doas.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_NONE;
      else if (str == "zenith")
        m_instrumental->frm4doas.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;
      else if (str == "off-axis")
        m_instrumental->frm4doas.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;
      else if (str == "direct-sun")
        m_instrumental->frm4doas.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN;
      else if (str == "almucantar")
        m_instrumental->frm4doas.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR;
      else
        return postErrorMessage("Invalid ccdeev Type");
     }

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->frm4doas.calibrationFile))
    strcpy(m_instrumental->frm4doas.calibrationFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->frm4doas.transmissionFunctionFile))
    strcpy(m_instrumental->frm4doas.transmissionFunctionFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Instrument Function  Filename too long");
    }
  }

  // ... other formats ...

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadLogger(const QXmlAttributes &atts, struct instrumental_logger *d)
{
  QString str;

  // spectral type
  str = atts.value("type");
  if (str == "all")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_ALL;
  else if (str == "zenithal")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_ZENITHAL;
  else if (str == "off-axis")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_OFFAXIS;
  else
    return postErrorMessage("Invalid spectral Type");

  d->flagAzimuthAngle = (atts.value("azi") == "true") ? 1 : 0;

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadSaoz(const QXmlAttributes &atts, struct instrumental_saoz *d)
{
  QString str;

  // spectral type
  str = atts.value("type");
  if (str == "zenithal")
    d->spectralType = PRJCT_INSTR_SAOZ_TYPE_ZENITHAL;
  else if (str == "pointed")
    d->spectralType = PRJCT_INSTR_SAOZ_TYPE_POINTED;
  else
    return postErrorMessage("Invalid spectral Type");

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadMinimum(const QXmlAttributes &atts, struct instrumental_minimum *d)
{
  QString str;

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadCcd(const QXmlAttributes &atts, struct instrumental_ccd *d)
{
  QString str;

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

  str = atts.value("ipv");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->interPixelVariabilityFile))
      strcpy(d->interPixelVariabilityFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Inter Pixel Variability Filename too long");
  }

  str = atts.value("dnl");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->detectorNonLinearityFile))
      strcpy(d->detectorNonLinearityFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Detector Non-Linearity Filename too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadGdp(const QXmlAttributes &atts, struct instrumental_gdp *d)
{
  QString str;

  str = atts.value("type");
  if (str == "1a")
    d->bandType = PRJCT_INSTR_GDP_BAND_1A;
  else if (str == "1b")
    d->bandType = PRJCT_INSTR_GDP_BAND_1B;
  else if (str == "2a")
    d->bandType = PRJCT_INSTR_GDP_BAND_2A;
  else if (str == "2b")
    d->bandType = PRJCT_INSTR_GDP_BAND_2B;
  else if (str == "3")
    d->bandType = PRJCT_INSTR_GDP_BAND_3;
  else if (str == "3")
    d->bandType = PRJCT_INSTR_GDP_BAND_4;
  else
    return postErrorMessage("Invalid gdp band");

  str = atts.value("pixel");
  if (str == "all")
    d->pixelType = PRJCT_INSTR_GDP_PIXEL_ALL;
  else if (str == "east")
    d->pixelType = PRJCT_INSTR_GDP_PIXEL_EAST;
  else if (str == "center")
    d->pixelType = PRJCT_INSTR_GDP_PIXEL_CENTER;
  else if (str == "west")
    d->pixelType = PRJCT_INSTR_GDP_PIXEL_WEST;
  else if (str == "backscan")
    d->pixelType = PRJCT_INSTR_GDP_PIXEL_BACKSCAN;
  else
    return postErrorMessage("Invalid gdp pixel type");

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadGome2(const QXmlAttributes &atts, struct instrumental_gome2 *d)
{
  QString str;

  str = atts.value("type");
  if (str == "1a")
    d->bandType = PRJCT_INSTR_GDP_BAND_1A;
  else if (str == "1b")
    d->bandType = PRJCT_INSTR_GDP_BAND_1B;
  else if (str == "2a")
    d->bandType = PRJCT_INSTR_GDP_BAND_2A;
  else if (str == "2b")
    d->bandType = PRJCT_INSTR_GDP_BAND_2B;
  else if (str == "3")
    d->bandType = PRJCT_INSTR_GDP_BAND_3;
  else if (str == "3")
    d->bandType = PRJCT_INSTR_GDP_BAND_4;
  else
    return postErrorMessage("Invalid gdp band");

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadScia(const QXmlAttributes &atts, struct instrumental_scia *d)
{
  QString str;

  str = atts.value("channel");
  if (str == "1")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_1;
  else if (str == "2")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_2;
  else if (str == "3")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_3;
  else if (str == "4")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_4;
  else
    return postErrorMessage("Invalid scia channel");

  // clusters
  memset(d->clusters, 0, sizeof(d->clusters)); // zero
  d->clusters[2]  = (atts.value("c2")  == "true") ? 1 : 0;
  d->clusters[3]  = (atts.value("c3")  == "true") ? 1 : 0;
  d->clusters[4]  = (atts.value("c4")  == "true") ? 1 : 0;
  d->clusters[5]  = (atts.value("c5")  == "true") ? 1 : 0;

  d->clusters[8]  = (atts.value("c8")  == "true") ? 1 : 0;
  d->clusters[9]  = (atts.value("c9")  == "true") ? 1 : 0;
  d->clusters[10] = (atts.value("c10") == "true") ? 1 : 0;

  d->clusters[13] = (atts.value("c13") == "true") ? 1 : 0;
  d->clusters[14] = (atts.value("c14") == "true") ? 1 : 0;
  d->clusters[15] = (atts.value("c15") == "true") ? 1 : 0;
  d->clusters[16] = (atts.value("c16") == "true") ? 1 : 0;
  d->clusters[17] = (atts.value("c17") == "true") ? 1 : 0;
  d->clusters[18] = (atts.value("c18") == "true") ? 1 : 0;

  d->clusters[22] = (atts.value("c22") == "true") ? 1 : 0;
  d->clusters[23] = (atts.value("c23") == "true") ? 1 : 0;
  d->clusters[24] = (atts.value("c24") == "true") ? 1 : 0;
  d->clusters[25] = (atts.value("c25") == "true") ? 1 : 0;
  d->clusters[26] = (atts.value("c26") == "true") ? 1 : 0;
  d->clusters[27] = (atts.value("c27") == "true") ? 1 : 0;

  str = atts.value("sunref");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(d->sunReference))
      strcpy(d->sunReference, str.toLocal8Bit().data());
    else
      return postErrorMessage("Sun Reference too long");
  }

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(CALIBRATION_FILENAME_ERR);
  }

  str = atts.value("transmission");
  if (str.isEmpty())
    str = atts.value("instr"); // check for old config files
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.toLocal8Bit().data());
    else
      return postErrorMessage(TRANSMISSION_FILENAME_ERR);
  }

    str = atts.value("dnl");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->detectorNonLinearityFile))
    strcpy(d->detectorNonLinearityFile, str.toLocal8Bit().data());
      else
    return postErrorMessage("Detector Non-Linearity Filename too long");
    }

  return true;
}

//------------------------------------------------------------------------
// handler for <slit> (child of project)

CProjectSlitSubHandler::CProjectSlitSubHandler(CConfigHandler *master,
                           mediate_project_slit_t *slit) :
  CConfigSubHandler(master),
  m_slit(slit)
{
}

bool CProjectSlitSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_slit->solarRefFile))
      strcpy(m_slit->solarRefFile, str.toLocal8Bit().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }

  m_slit->applyFwhmCorrection = (atts.value("fwhmcor") == "true") ? 1 : 0;

  return true;
}

bool CProjectSlitSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "slit_func") {

    return m_master->installSubHandler(new CSlitFunctionSubHandler(m_master, &(m_slit->function)), atts);
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <output> (child of project)

CProjectOutputSubHandler::CProjectOutputSubHandler(CConfigHandler *master,
                           mediate_project_output_t *output) :
  CSelectorSubHandler(master, &(output->selection)),
  m_output(output)
{
}

bool CProjectOutputSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("path");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_output->path))
      strcpy(m_output->path, str.toLocal8Bit().data());
    else
      return postErrorMessage("Output path too long");
  }

  m_output->analysisFlag = (atts.value("anlys") == "true") ? 1 : 0;
  m_output->calibrationFlag = (atts.value("calib") == "true") ? 1 : 0;
  m_output->configurationFlag = (atts.value("conf") == "true") ? 1 : 0;
  m_output->referenceFlag = (atts.value("ref") == "true");
  // m_output->newcalibFlag = (atts.value("newcalib") == "true");
  m_output->directoryFlag = (atts.value("dirs") == "true") ? 1 : 0;
  m_output->filenameFlag = (atts.value("file") == "true") ? 1 : 0;

  str = atts.value("success");
  if (str.isEmpty())
   m_output->successFlag=1;
  else
   m_output->successFlag = (str == "true") ? 1 : 0;


  str = atts.value("flux");
  if (!str.isEmpty()) {
    if (str.length()+1 < (int)sizeof(m_output->flux))
      strcpy(m_output->flux, str.toLocal8Bit().data());
    else
      return postErrorMessage("Output flux too long");
  }

  str = atts.value("bandWidth");
  if (str.isEmpty())
   m_output->bandWidth=1.;
  else
   m_output->bandWidth = str.toDouble();

  str = atts.value("swathName");
  if (!str.isEmpty()) {
    if ((size_t) str.length()+1 < sizeof(m_output->swath_name))
      strcpy(m_output->swath_name, str.toLocal8Bit().data());
    else
      return postErrorMessage("Output file swath name too long.");
  }

  str = atts.value("fileFormat");
  if (!str.isEmpty()) {
    enum output_format format = output_get_format(str.toLocal8Bit().data());
    if ((format == ASCII)  || (format==NETCDF))
     m_output->file_format = format;
    else
     m_output->file_format=ASCII;
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <export> (child of project)

CProjectExportSubHandler::CProjectExportSubHandler(CConfigHandler *master,
                           mediate_project_export_t *exportSpectra) :
  CSelectorSubHandler(master, &(exportSpectra->selection)),
  m_export(exportSpectra)
{
}

bool CProjectExportSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("path");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_export->path))
      strcpy(m_export->path, str.toLocal8Bit().data());
    else
      return postErrorMessage("Export path too long");
  }

  m_export->titlesFlag = (atts.value("titles") == "true") ? 1 : 0;
  m_export->directoryFlag = (atts.value("dir") == "true") ? 1 : 0;

  return true;
}
