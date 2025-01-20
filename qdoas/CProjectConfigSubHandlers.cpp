#include "CProjectConfigSubHandlers.h"
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CConfigSubHandlerUtils.h"
#include "CProjectConfigTreeNode.h"

#include "constants.h"
#include "debugutil.h"

using std::map;
using std::string;

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

void CSelectorSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  if (element != "field")
    throw std::runtime_error("Invalid XML element");

  data_select_list_t *d = m_selectList;

  if (d->nSelected >= PRJCT_RESULTS_MAX)
    throw std::runtime_error("Too many output fields");

  string str = value(atts, "name");
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
    throw std::runtime_error("Invalid output field " + str);

  // MUST be ok ...
  ++(d->nSelected);

}


//------------------------------------------------------------------------
// handler for <display> (child of project)

CProjectDisplaySubHandler::CProjectDisplaySubHandler(CConfigHandler *master, mediate_project_display_t *display) :
  CSelectorSubHandler(master, &(display->selection)),
  m_display(display)
{
}

void CProjectDisplaySubHandler::start(const map<Glib::ustring, string> &atts)
{
  m_display->requireSpectra = (value(atts, "spectra") == "true") ? 1 : 0;
  m_display->requireData = (value(atts, "data") == "true") ? 1 : 0;
  m_display->requireCalib = (value(atts, "calib") == "true") ? 1 : 0;
  m_display->requireFits = (value(atts, "fits") == "true") ? 1 : 0;

}

//------------------------------------------------------------------------
// handler for <selection> (child of project)

CProjectSelectionSubHandler::CProjectSelectionSubHandler(CConfigHandler *master, mediate_project_selection_t *selection) :
  CConfigSubHandler(master),
  m_selection(selection)
{
}

void CProjectSelectionSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  if (element == "sza") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->szaMinimum = parse_value<double>(atts, "min");
    m_selection->szaMaximum = parse_value<double>(atts, "max");
    m_selection->szaDelta = parse_value<double>(atts, "delta");

  }
  if (element == "elevation") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->elevationMinimum = parse_value<double>(atts, "min");
    m_selection->elevationMaximum = parse_value<double>(atts, "max");
    m_selection->elevationTolerance = parse_value<double>(atts, "tol");

  }
  if (element == "reference") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->refAngle = parse_value<double>(atts, "angle");
    m_selection->refTolerance = parse_value<double>(atts, "tol");

  }
  else if (element == "record") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->recordNumberMinimum = parse_value<int>(atts, "min");
    m_selection->recordNumberMaximum = parse_value<int>(atts, "max");
  }
  else if (element == "cloud")
   {
     m_selection->cloudFractionMinimum = parse_value<double>(atts, "min");
     m_selection->cloudFractionMaximum = parse_value<double>(atts, "max");
   }
  else if (element == "circle") {

    m_selection->geo.circle.radius = parse_value<double>(atts, "radius");
    m_selection->geo.circle.centerLongitude = parse_value<double>(atts, "long");
    m_selection->geo.circle.centerLatitude = parse_value<double>(atts, "lat");
  }
  else if (element == "rectangle") {

    m_selection->geo.rectangle.easternLongitude = parse_value<double>(atts, "east");
    m_selection->geo.rectangle.westernLongitude = parse_value<double>(atts, "west");
    m_selection->geo.rectangle.northernLatitude = parse_value<double>(atts, "north");
    m_selection->geo.rectangle.southernLatitude = parse_value<double>(atts, "south");
  }
  else if (element == "sites") {

    m_selection->geo.sites.radius = parse_value<double>(atts, "radius");
  }
  else if (element == "geolocation") {

    string selected = value(atts, "selected");

    if (selected == "circle")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_CIRCLE;
    else if (selected == "rectangle")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_RECTANGLE;
    else if (selected == "sites")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_OBSLIST;
    else
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_NONE; // default and "none"
  }

}


//------------------------------------------------------------------------
// handler for <analysis> (child of project)

CProjectAnalysisSubHandler::CProjectAnalysisSubHandler(CConfigHandler *master,
                               mediate_project_analysis_t *analysis) :
  CConfigSubHandler(master),
  m_analysis(analysis)
{
}

void CProjectAnalysisSubHandler::start(const map<Glib::ustring, string> &atts)
{
  // all options are in the attributes of the analysis element itself

  string str;

  str = value(atts, "method");
  if (str == "ODF")
    m_analysis->methodType = OPTICAL_DENSITY_FIT;
  else if (str == "ML+SVD")
    m_analysis->methodType = INTENSITY_FIT;
  else
    throw std::runtime_error("Invalid analysis method");

  str = value(atts, "fit");
  if (str == "none")
    m_analysis->fitType = PRJCT_ANLYS_FIT_WEIGHTING_NONE;
  else if (str == "instr")
    m_analysis->fitType = PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL;
  else
    throw std::runtime_error("Invalid analysis fit");

  str = value(atts, "interpolation");
  if (str == "linear")
    m_analysis->interpolationType = PRJCT_ANLYS_INTERPOL_LINEAR;
  else if (str == "spline")
    m_analysis->interpolationType = PRJCT_ANLYS_INTERPOL_SPLINE;
  else
    throw std::runtime_error("Invalid analysis interpolation");

  m_analysis->interpolationSecurityGap = parse_value<int>(atts, "gap");
  m_analysis->maxIterations=parse_value<int>(atts, "max_iterations");
  m_analysis->convergenceCriterion = parse_value<double>(atts, "converge");
  if (value(atts, "spike_tolerance") != "")
    m_analysis->spike_tolerance = parse_value<double>(atts, "spike_tolerance");

}


//------------------------------------------------------------------------
// handler for <raw_spectra> (child of project)

CProjectRawSpectraSubHandler::CProjectRawSpectraSubHandler(CConfigHandler *master,
                                                           std::shared_ptr<CProjectConfigTreeNode> node) :
  CConfigSubHandler(master),
  m_node(node)
{
}

void CProjectRawSpectraSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  // <directory>, <file> or <folder>

  string name = value(atts, "name");
  bool enabled = (value(atts, "disabled") != "true");

  // MUST have a name ...
  if (name.empty())
    throw std::runtime_error("Spectrum file must have a name");

  if (element == "file") {
    // expand the name (the filename)
    name = m_master->pathExpand(name);
    m_node->addChild(std::make_shared<CProjectConfigFile>(name, enabled));
  }
  else if (element == "directory") {

    // expand the name (the directory name)
    name = m_master->pathExpand(name);
    m_node->addChild(std::make_shared<CProjectConfigDirectory>(name,
                                                               value(atts, "filters"),
                                                               value(atts, "recursive") == "true",
                                                               enabled));
  }
  else if (element == "folder") {
    // create an item for the folder now ...
    auto item = std::make_shared<CProjectConfigFolder>(name, enabled);
    m_node->addChild(item);

    // and a sub handler for child items
    return m_master->install_subhandler(new CProjectRawSpectraSubHandler(m_master, item), atts);
  }

}


//------------------------------------------------------------------------
// handler for <calibration> (child of project)

CProjectCalibrationSubHandler::CProjectCalibrationSubHandler(CConfigHandler *master,
                                 mediate_project_calibration_t *calibration) :
  CConfigSubHandler(master),
  m_calibration(calibration)
{
}

void CProjectCalibrationSubHandler::start(const map<Glib::ustring, string> &atts)
{
  string str;

  str = value(atts, "method");
  if (str == "ODF")
    m_calibration->methodType = OPTICAL_DENSITY_FIT;
  else if (str == "ML+SVD")
    m_calibration->methodType = INTENSITY_FIT;
  else
    throw std::runtime_error("Invalid analysis method");

  str = value(atts, "ref");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(m_calibration->solarRefFile))
      strcpy(m_calibration->solarRefFile, str.c_str());
    else
      throw std::runtime_error("Solar Reference Filename too long");
  }

}

void CProjectCalibrationSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  string str;
  // sub element of calibration

  if (element == "line") {
    str = value(atts, "shape");
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
      throw std::runtime_error("Invalid line shape" + str);

    if (value(atts, "lorentzorder") != "") {
      m_calibration->lorentzOrder = parse_value<int>(atts, "lorentzorder");
    } else {
      m_calibration->lorentzOrder = parse_value<int>(atts, "lorentzdegree")/2;
    }

    str = value(atts, "slfFile");
    if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_calibration->slfFile))
        strcpy(m_calibration->slfFile, str.c_str());
      else
        throw std::runtime_error("Slit function Filename too long");
     }
  }
  else if (element == "display") {

    m_calibration->requireSpectra = (value(atts, "spectra") == "true") ? 1 : 0;
    m_calibration->requireFits = (value(atts, "fits") == "true") ? 1 : 0;
    m_calibration->requireResidual = (value(atts, "residual") == "true") ? 1 : 0;
    m_calibration->requireShiftSfp = (value(atts, "shiftsfp") == "true") ? 1 : 0;
  }
  else if (element == "polynomial") {

    m_calibration->shiftDegree = parse_value<int>(atts, "shift");
    m_calibration->sfpDegree = parse_value<int>(atts, "sfp");
  }
  else if (element == "window") {

    m_calibration->wavelengthMin = parse_value<double>(atts, "min");
    m_calibration->wavelengthMax = parse_value<double>(atts, "max");
    m_calibration->windowSize = parse_value<double>(atts, "size");
    m_calibration->subWindows = parse_value<int>(atts, "intervals");

    const string& custom_windows = value(atts, "custom_windows");
    const char *ptr = custom_windows.c_str();
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

    str = value(atts, "division");
    if (str == "sliding")
     m_calibration->divisionMode = PRJCT_CALIB_WINDOWS_SLIDING;
    else if (str == "custom")
     m_calibration->divisionMode = PRJCT_CALIB_WINDOWS_CUSTOM;
    else
     m_calibration->divisionMode = PRJCT_CALIB_WINDOWS_CONTIGUOUS;


  }
  else if (element == "preshift") {
    m_calibration->preshiftFlag = (value(atts, "calculate") == "true") ? 1 : 0;
    m_calibration->preshiftMin = parse_value<double>(atts, "min");
    m_calibration->preshiftMax = parse_value<double>(atts, "max");
  }
  else if (element == "cross_section") {
    return m_master->install_subhandler(new CAnalysisWindowCrossSectionSubHandler(m_master, &(m_calibration->crossSectionList)), atts);
  }
  else if (element == "linear") {
    return m_master->install_subhandler(new CAnalysisWindowLinearSubHandler(m_master, &(m_calibration->linear)), atts);
  }
  else if (element == "sfp") {
    return m_master->install_subhandler(new CAnalysisWindowSfpSubHandler(m_master, &(m_calibration->sfp[0])), atts);
  }
  else if (element == "shift_stretch") {
    return m_master->install_subhandler(new CAnalysisWindowShiftStretchSubHandler(m_master, &(m_calibration->shiftStretchList)), atts);
  }
  else if (element == "gap") {
    return m_master->install_subhandler(new CAnalysisWindowGapSubHandler(m_master, &(m_calibration->gapList)), atts);
  }
  else if (element == "output") {
    return m_master->install_subhandler(new CAnalysisWindowOutputSubHandler(m_master, &(m_calibration->outputList)), atts);
  }

}

//------------------------------------------------------------------------
// handler for <undersampling> (child of project)

CProjectUndersamplingSubHandler::CProjectUndersamplingSubHandler(CConfigHandler *master,
                                 mediate_project_undersampling_t *undersampling) :
  CConfigSubHandler(master),
  m_undersampling(undersampling)
{
}

void CProjectUndersamplingSubHandler::start(const map<Glib::ustring, string> &atts)
{
  string str;

  str = value(atts, "method");
  if (str == "file")
    m_undersampling->method = PRJCT_USAMP_FILE;
  else if (str == "fixed")
    m_undersampling->method = PRJCT_USAMP_FIXED;
  else if (str == "auto")
    m_undersampling->method = PRJCT_USAMP_AUTOMATIC;
  else
    throw std::runtime_error("Invalid analysis method");

  str = value(atts, "ref");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(m_undersampling->solarRefFile))
      strcpy(m_undersampling->solarRefFile, str.c_str());
    else
      throw std::runtime_error("Solar Reference Filename too long");
  }

  m_undersampling->shift = parse_value<double>(atts, "shift");

}

//------------------------------------------------------------------------
// handler for <instrumental> (child of project)

CProjectInstrumentalSubHandler::CProjectInstrumentalSubHandler(CConfigHandler *master,
                                   mediate_project_instrumental_t *instrumental) :
  CConfigSubHandler(master),
  m_instrumental(instrumental)
{
}

void CProjectInstrumentalSubHandler::start(const map<Glib::ustring, string> &atts)
{
  // the <instrumental ...> element

  string str;

  str = value(atts, "format");
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
  else if (str == "omiv4")
    m_instrumental->format = PRJCT_INSTR_FORMAT_OMIV4;
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
    throw std::runtime_error("Invalid instrumental format");

  str = value(atts, "site");
  if (!str.empty()) {
    if (str.length() < sizeof(m_instrumental->siteName))
      strcpy(m_instrumental->siteName, str.c_str());
    else
      throw std::runtime_error("Instrumental Site Name too long");
  }

  str = value(atts, "saa_convention");
  m_instrumental->saaConvention=(!str.empty() && (str=="0-north"))?PRJCT_INSTR_SAA_NORTH:PRJCT_INSTR_SAA_SOUTH;

}

void CProjectInstrumentalSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  // format specific children of instrumental

  if (element == "ascii") { // ASCII
    string str;

    m_instrumental->ascii.detectorSize = parse_value<int>(atts, "size");

    str = value(atts, "format");
    if (str == "line")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_LINE;
    else if (str == "column")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_COLUMN;
    else if (str == "column_extended")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED;
    else
      throw std::runtime_error("Invalid ascii format");

    m_instrumental->ascii.flagZenithAngle = (value(atts, "zen") == "true") ? 1 : 0;
    m_instrumental->ascii.flagAzimuthAngle = (value(atts, "azi") == "true") ? 1 : 0;
    m_instrumental->ascii.flagElevationAngle = (value(atts, "ele") == "true") ? 1 : 0;
    m_instrumental->ascii.flagDate = (value(atts, "date") == "true") ? 1 : 0;
    m_instrumental->ascii.flagTime = (value(atts, "time") == "true") ? 1 : 0;
    m_instrumental->ascii.flagWavelength = (value(atts, "lambda") == "true") ? 1 : 0;
    m_instrumental->ascii.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->ascii.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->ascii.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ascii.calibrationFile))
    strcpy(m_instrumental->ascii.calibrationFile, str.c_str());
      else
    throw std::runtime_error(CALIBRATION_FILENAME_ERR);
    }

    str = value(atts, "transmission");
    if (str.empty())
      str = value(atts, "instr"); // check for old config files
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ascii.transmissionFunctionFile))
    strcpy(m_instrumental->ascii.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
    }

  }
#ifdef PRJCT_INSTR_FORMAT_OLD
  else if (element == "logger") { // LOGGER
    return helperLoadLogger(atts, &(m_instrumental->logger));

  }
  else if (element == "acton") { // ACTON
    string str;

    str = value(atts, "type");
    if (str == "old")
      m_instrumental->acton.niluType = PRJCT_INSTR_NILU_FORMAT_OLD;
    else if (str == "new")
      m_instrumental->acton.niluType = PRJCT_INSTR_NILU_FORMAT_NEW;
    else
      throw std::runtime_error("Invalid acton Type");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->acton.calibrationFile))
    strcpy(m_instrumental->acton.calibrationFile, str.c_str());
      else
    throw std::runtime_error(CALIBRATION_FILENAME_ERR);
    }

    str = value(atts, "transmission");
    if (str.empty())
      str = value(atts, "instr"); // check for old config files
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->acton.transmissionFunctionFile))
    strcpy(m_instrumental->acton.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
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
    m_instrumental->saozefm.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->saozefm.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->saozefm.lambdaMax = parse_value<double>(atts, "lambda_max");
    return helperLoadMinimum(atts, &(m_instrumental->saozefm));

  }
  else if (element == "mfc") { // MFC
    string str;

    m_instrumental->mfc.detectorSize = parse_value<int>(atts, "size");
    m_instrumental->mfc.firstWavelength = parse_value<int>(atts, "first");

    m_instrumental->mfc.revert = (value(atts, "revert") == "true") ? 1 : 0;
    m_instrumental->mfc.autoFileSelect = (value(atts, "auto") == "true") ? 1 : 0;
    m_instrumental->mfc.offsetMask = parse_value<unsigned long>(atts, "omask");
    m_instrumental->mfc.instrFctnMask = parse_value<unsigned long>(atts, "imask");
    m_instrumental->mfc.darkCurrentMask = parse_value<unsigned long>(atts, "dmask");
    m_instrumental->mfc.spectraMask = parse_value<unsigned long>(atts, "smask");

    m_instrumental->mfc.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->mfc.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->mfc.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfc.calibrationFile))
    strcpy(m_instrumental->mfc.calibrationFile, str.c_str());
      else
    throw std::runtime_error(CALIBRATION_FILENAME_ERR);
    }

    str = value(atts, "transmission");
    if (str.empty())
      str = value(atts, "instr"); // check for old config files
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfc.transmissionFunctionFile))
    strcpy(m_instrumental->mfc.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
    }
    str = value(atts, "dark");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfc.darkCurrentFile))
    strcpy(m_instrumental->mfc.darkCurrentFile, str.c_str());
      else
    throw std::runtime_error("Dark Current Filename too long");
    }

    str = value(atts, "offset");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfc.offsetFile))
    strcpy(m_instrumental->mfc.offsetFile, str.c_str());
      else
    throw std::runtime_error("Offset Filename too long");
    }

  }
  else if (element == "mfcstd") { // MFC STD
    string str;

    m_instrumental->mfcstd.detectorSize = parse_value<int>(atts, "size");
    m_instrumental->mfcstd.revert = (value(atts, "revert") == "true") ? 1 : 0;
    m_instrumental->mfcstd.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->mfcstd.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->mfcstd.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "date");
    if (!str.empty()) {
      if (str.length() < sizeof(m_instrumental->mfcstd.dateFormat))
        strcpy(m_instrumental->mfcstd.dateFormat, str.c_str());
      else
        throw std::runtime_error("Date format too long");
     }

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfcstd.calibrationFile))
    strcpy(m_instrumental->mfcstd.calibrationFile, str.c_str());
      else
    throw std::runtime_error(CALIBRATION_FILENAME_ERR);
    }

    str = value(atts, "transmission");
    if (str.empty())
      str = value(atts, "instr"); // check for old config files
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfcstd.transmissionFunctionFile))
    strcpy(m_instrumental->mfcstd.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
    }
    str = value(atts, "dark");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfcstd.darkCurrentFile))
    strcpy(m_instrumental->mfcstd.darkCurrentFile, str.c_str());
      else
    throw std::runtime_error("Dark Current Filename too long");
    }

    str = value(atts, "offset");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfcstd.offsetFile))
    strcpy(m_instrumental->mfcstd.offsetFile, str.c_str());
      else
    throw std::runtime_error("Offset Filename too long");
    }
  }
  else if (element == "mfcbira") { // MFC bira
    string str;

    m_instrumental->mfcbira.detectorSize = parse_value<int>(atts, "size");
    m_instrumental->mfcbira.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->mfcbira.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->mfcbira.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfcbira.calibrationFile))
    strcpy(m_instrumental->mfcbira.calibrationFile, str.c_str());
      else
    throw std::runtime_error(CALIBRATION_FILENAME_ERR);
    }

    str = value(atts, "transmission");
    if (str.empty())
      str = value(atts, "instr"); // check for old config files
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->mfcbira.transmissionFunctionFile))
    strcpy(m_instrumental->mfcbira.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
    }
  }

#ifdef PRJCT_INSTR_FORMAT_OLD
  else if (element == "rasas") { // RASAS
    m_instrumental->rasas.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->rasas.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->rasas.lambdaMax = parse_value<double>(atts, "lambda_max");
    return helperLoadMinimum(atts, &(m_instrumental->rasas));

  }
  else if (element == "pdasieasoe") { // PDASI EASOE
    m_instrumental->pdasieasoe.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->pdasieasoe.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->pdasieasoe.lambdaMax = parse_value<double>(atts, "lambda_max");
    return helperLoadMinimum(atts, &(m_instrumental->pdasieasoe));

  }
#endif
  else if (element == "ccdeev") { // CCD EEV
    string str;

    m_instrumental->ccdeev.detectorSize = parse_value<int>(atts, "size");
    m_instrumental->ccdeev.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->ccdeev.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->ccdeev.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ccdeev.calibrationFile))
    strcpy(m_instrumental->ccdeev.calibrationFile, str.c_str());
      else
    throw std::runtime_error(CALIBRATION_FILENAME_ERR);
    }

    str = value(atts, "transmission");
    if (str.empty())
      str = value(atts, "instr"); // check for old config files
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ccdeev.transmissionFunctionFile))
    strcpy(m_instrumental->ccdeev.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
    }

    str = value(atts, "image");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ccdeev.imagePath))
    strcpy(m_instrumental->ccdeev.imagePath, str.c_str());
      else
    throw std::runtime_error("Stray Light Correction Filename too long");
    }

    str = value(atts, "stray");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ccdeev.straylightCorrectionFile))
    strcpy(m_instrumental->ccdeev.straylightCorrectionFile, str.c_str());
      else
    throw std::runtime_error("Stray Light Correction Filename too long");
    }

    str = value(atts, "dnl");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->ccdeev.detectorNonLinearityFile))
    strcpy(m_instrumental->ccdeev.detectorNonLinearityFile, str.c_str());
      else
    throw std::runtime_error("Detector Non-Linearity Filename too long");
    }

    str = value(atts, "type");

    if (!str.empty())
         {
      if (str == "all")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_NONE;
      else if (str == "off-axis")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS;
      else if (str == "direct-sun")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN;
      else if (str == "almucantar")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR;
      else if (str == "zenith-only")
        m_instrumental->ccdeev.spectralType = PRJCT_INSTR_MAXDOAS_TYPE_ZENITH;      
      else
        throw std::runtime_error("Invalid ccdeev Type");
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
    m_instrumental->uoft.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->uoft.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->uoft.lambdaMax = parse_value<double>(atts, "lambda_max");
    return helperLoadMinimum(atts, &(m_instrumental->uoft));

  }
  else if (element == "noaa") { // NOAA
    m_instrumental->noaa.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->noaa.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->noaa.lambdaMax = parse_value<double>(atts, "lambda_max");
    return helperLoadMinimum(atts, &(m_instrumental->noaa));

  }
  else if (element == "omi") { // OMI
    string str;

    str = value(atts, "type");
    if (str == "uv1")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV1;
    else if (str == "uv2")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV2;
    else if (str == "vis")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_VIS;
    else
      throw std::runtime_error("Invalid omi Spectral Type");

    m_instrumental->omi.minimumWavelength = parse_value<double>(atts, "min");
    m_instrumental->omi.maximumWavelength = parse_value<double>(atts, "max");
    m_instrumental->omi.flagAverage = (value(atts, "ave") == "true") ? 1 : 0;

    str = value(atts, "trackSelection");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->omi.trackSelection))
    strcpy(m_instrumental->omi.trackSelection, str.c_str());
      else
    throw std::runtime_error("Track selection string too long");
    }

    str = value(atts, "xTrackMode");
    m_instrumental->omi.xtrack_mode = str_to_mode(str.c_str());

    m_instrumental->omi.pixelQFRejectionFlag = (value(atts, "pixelQF_rejectionFlag") == "true") ? 1 : 0;

    str = value(atts, "pixelQF_maxGaps");
    m_instrumental->omi.pixelQFMaxGaps = (!str.empty())?parse_value<int>(atts, "pixelQF_maxGaps"):5;
    m_instrumental->omi.pixelQFMask = parse_value<int>(atts, "pixelQF_mask");

  }
  else if (element == "omiv4") {
    string str(value(atts, "type"));
    if (str == "uv1")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV1;
    else if (str == "uv2")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV2;
    else if (str == "vis")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_VIS;
    else
      throw std::runtime_error("Invalid omi Spectral Type");
  }
  else if (element == "tropomi") {
    string str = value(atts, "band");
#define EXPAND(BAND, LABEL)                      \
    if(str == LABEL || str == #BAND)                 \
      m_instrumental->tropomi.spectralBand = BAND;   \
    else
    TROPOMI_BANDS
#undef EXPAND
      throw std::runtime_error("No Tropomi spectral band configured.");

    str = value(atts, "reference_orbit_dir");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.reference_orbit_dir))
    strcpy(m_instrumental->tropomi.reference_orbit_dir, str.c_str());
      else
    throw std::runtime_error("Tropomi reference orbit directory name too long");
    }

    str = value(atts, "trackSelection");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.trackSelection))
    strcpy(m_instrumental->tropomi.trackSelection, str.c_str());
      else
    throw std::runtime_error("Track selection string too long");
    }


    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.calibrationFile))
    strcpy(m_instrumental->tropomi.calibrationFile, str.c_str());
      else
    throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.instrFunctionFile))
    strcpy(m_instrumental->tropomi.instrFunctionFile, str.c_str());
      else
    throw std::runtime_error("Instrument Function  Filename too long");
    }
  }
  else if (element == "apex") {
    string str = value(atts, "trackSelection");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->apex.trackSelection))
    strcpy(m_instrumental->apex.trackSelection, str.c_str());
      else
    throw std::runtime_error("Track selection string too long");
    }


    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.calibrationFile))
    strcpy(m_instrumental->tropomi.calibrationFile, str.c_str());
      else
    throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->tropomi.instrFunctionFile))
    strcpy(m_instrumental->tropomi.instrFunctionFile, str.c_str());
      else
    throw std::runtime_error("Instrument Function  Filename too long");
    }
  }
  else if (element == "gome2") { // GOME2
    helperLoadGome2(atts, &(m_instrumental->gome2));
  }
  else if (element == "mkzy") { // MKZY
    m_instrumental->mkzy.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->mkzy.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->mkzy.lambdaMax = parse_value<double>(atts, "lambda_max");
    return helperLoadMinimum(atts, &(m_instrumental->mkzy));
  }
  else if (element == "biraairborne") {  // BIRA AIRBORNE
    string str;

    m_instrumental->biraairborne.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->biraairborne.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->biraairborne.lambdaMax = parse_value<double>(atts, "lambda_max");

    m_instrumental->biraairborne.detectorSize = parse_value<int>(atts, "size");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biraairborne.calibrationFile))
          strcpy(m_instrumental->biraairborne.calibrationFile, str.c_str());
      else
          throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biraairborne.transmissionFunctionFile))
          strcpy(m_instrumental->biraairborne.transmissionFunctionFile, str.c_str());
      else
          throw std::runtime_error("Instrument Function  Filename too long");
    }
  }
  else if (element == "biramobile") {  // BIRA MOBILE
    string str;

    m_instrumental->biramobile.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->biramobile.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->biramobile.lambdaMax = parse_value<double>(atts, "lambda_max");

    m_instrumental->biramobile.detectorSize = parse_value<int>(atts, "size");

    if (!m_instrumental->biramobile.detectorSize)
     m_instrumental->biramobile.detectorSize=2048;

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biramobile.calibrationFile))
          strcpy(m_instrumental->biramobile.calibrationFile, str.c_str());
      else
          throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->biramobile.transmissionFunctionFile))
          strcpy(m_instrumental->biramobile.transmissionFunctionFile, str.c_str());
      else
          throw std::runtime_error("Instrument Function  Filename too long");
    }
  }
  else if (element == "oceanoptics") { // OCEAN OPTICS
    string str;

    m_instrumental->oceanoptics.detectorSize = parse_value<int>(atts, "size");
    m_instrumental->oceanoptics.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->oceanoptics.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->oceanoptics.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->oceanoptics.calibrationFile))
    strcpy(m_instrumental->oceanoptics.calibrationFile, str.c_str());
      else
    throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->oceanoptics.transmissionFunctionFile))
    strcpy(m_instrumental->oceanoptics.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error("Instrument Function  Filename too long");
    }
  }
    else if (element == "gems") {
     string str;
    str = value(atts, "trackSelection");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->gems.trackSelection)) {
        strcpy(m_instrumental->gems.trackSelection, str.c_str());
      } else {
        throw std::runtime_error("Track selection string too long");
      }
      m_instrumental->gems.binning=parse_value<int>(atts, "binning", 4);

    }


    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->gems.calibrationFile))
        strcpy(m_instrumental->gems.calibrationFile, str.c_str());
      else
        throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->gems.transmissionFunctionFile))
        strcpy(m_instrumental->gems.transmissionFunctionFile, str.c_str());
      else
        throw std::runtime_error("Instrument Function  Filename too long");
    }
  }
  else if (element == "frm4doas") { // FRM4DOAS netCDF
    string str;

    str = value(atts, "average_rows");

    if (!str.empty())
     m_instrumental->frm4doas.averageRows = (value(atts, "average_rows") == "true") ? 1 : 0;
    else
     m_instrumental->frm4doas.averageRows=1;

    m_instrumental->frm4doas.detectorSize = parse_value<int>(atts, "size");
    m_instrumental->frm4doas.straylight = (value(atts, "straylight") == "true") ? 1 : 0;
    m_instrumental->frm4doas.lambdaMin = parse_value<double>(atts, "lambda_min");
    m_instrumental->frm4doas.lambdaMax = parse_value<double>(atts, "lambda_max");

    str = value(atts, "type");

    if (!str.empty())
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
        throw std::runtime_error("Invalid ccdeev Type");
     }

    str = value(atts, "calib");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->frm4doas.calibrationFile))
    strcpy(m_instrumental->frm4doas.calibrationFile, str.c_str());
      else
    throw std::runtime_error("Calibration Filename too long");
    }

    str = value(atts, "instr");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(m_instrumental->frm4doas.transmissionFunctionFile))
    strcpy(m_instrumental->frm4doas.transmissionFunctionFile, str.c_str());
      else
    throw std::runtime_error("Instrument Function  Filename too long");
    }
  }

  // ... other formats ...

}

void CProjectInstrumentalSubHandler::helperLoadLogger(const map<Glib::ustring, string> &atts, struct instrumental_logger *d)
{
  string str;

  // spectral type
  str = value(atts, "type");
  if (str == "all")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_ALL;
  else if (str == "zenithal")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_ZENITHAL;
  else if (str == "off-axis")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_OFFAXIS;
  else
    throw std::runtime_error("Invalid spectral Type");

  d->flagAzimuthAngle = (value(atts, "azi") == "true") ? 1 : 0;

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

}

void CProjectInstrumentalSubHandler::helperLoadSaoz(const map<Glib::ustring, string> &atts, struct instrumental_saoz *d)
{
  string str;

  // spectral type
  str = value(atts, "type");
  if (str == "zenithal")
    d->spectralType = PRJCT_INSTR_SAOZ_TYPE_ZENITHAL;
  else if (str == "pointed")
    d->spectralType = PRJCT_INSTR_SAOZ_TYPE_POINTED;
  else
    throw std::runtime_error("Invalid spectral Type");

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

}

void CProjectInstrumentalSubHandler::helperLoadMinimum(const map<Glib::ustring, string> &atts, struct instrumental_minimum *d)
{
  string str;

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

}

void CProjectInstrumentalSubHandler::helperLoadCcd(const map<Glib::ustring, string> &atts, struct instrumental_ccd *d)
{
  string str;

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

  str = value(atts, "ipv");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->interPixelVariabilityFile))
      strcpy(d->interPixelVariabilityFile, str.c_str());
    else
      throw std::runtime_error("Inter Pixel Variability Filename too long");
  }

  str = value(atts, "dnl");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->detectorNonLinearityFile))
      strcpy(d->detectorNonLinearityFile, str.c_str());
    else
      throw std::runtime_error("Detector Non-Linearity Filename too long");
  }

}

void CProjectInstrumentalSubHandler::helperLoadGdp(const map<Glib::ustring, string> &atts, struct instrumental_gdp *d)
{
  string str;

  str = value(atts, "type");
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
    throw std::runtime_error("Invalid gdp band");

  str = value(atts, "pixel");
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
    throw std::runtime_error("Invalid gdp pixel type");

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

}

void CProjectInstrumentalSubHandler::helperLoadGome2(const map<Glib::ustring, string> &atts, struct instrumental_gome2 *d)
{
  string str;

  str = value(atts, "type");
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
    throw std::runtime_error("Invalid gdp band");

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

}

void CProjectInstrumentalSubHandler::helperLoadScia(const map<Glib::ustring, string> &atts, struct instrumental_scia *d)
{
  string str;

  str = value(atts, "channel");
  if (str == "1")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_1;
  else if (str == "2")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_2;
  else if (str == "3")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_3;
  else if (str == "4")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_4;
  else
    throw std::runtime_error("Invalid scia channel");

  // clusters
  memset(d->clusters, 0, sizeof(d->clusters)); // zero
  d->clusters[2]  = (value(atts, "c2")  == "true") ? 1 : 0;
  d->clusters[3]  = (value(atts, "c3")  == "true") ? 1 : 0;
  d->clusters[4]  = (value(atts, "c4")  == "true") ? 1 : 0;
  d->clusters[5]  = (value(atts, "c5")  == "true") ? 1 : 0;

  d->clusters[8]  = (value(atts, "c8")  == "true") ? 1 : 0;
  d->clusters[9]  = (value(atts, "c9")  == "true") ? 1 : 0;
  d->clusters[10] = (value(atts, "c10") == "true") ? 1 : 0;

  d->clusters[13] = (value(atts, "c13") == "true") ? 1 : 0;
  d->clusters[14] = (value(atts, "c14") == "true") ? 1 : 0;
  d->clusters[15] = (value(atts, "c15") == "true") ? 1 : 0;
  d->clusters[16] = (value(atts, "c16") == "true") ? 1 : 0;
  d->clusters[17] = (value(atts, "c17") == "true") ? 1 : 0;
  d->clusters[18] = (value(atts, "c18") == "true") ? 1 : 0;

  d->clusters[22] = (value(atts, "c22") == "true") ? 1 : 0;
  d->clusters[23] = (value(atts, "c23") == "true") ? 1 : 0;
  d->clusters[24] = (value(atts, "c24") == "true") ? 1 : 0;
  d->clusters[25] = (value(atts, "c25") == "true") ? 1 : 0;
  d->clusters[26] = (value(atts, "c26") == "true") ? 1 : 0;
  d->clusters[27] = (value(atts, "c27") == "true") ? 1 : 0;

  str = value(atts, "sunref");
  if (!str.empty()) {
    if (str.length() < sizeof(d->sunReference))
      strcpy(d->sunReference, str.c_str());
    else
      throw std::runtime_error("Sun Reference too long");
  }

  str = value(atts, "calib");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.c_str());
    else
      throw std::runtime_error(CALIBRATION_FILENAME_ERR);
  }

  str = value(atts, "transmission");
  if (str.empty())
    str = value(atts, "instr"); // check for old config files
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(d->transmissionFunctionFile))
      strcpy(d->transmissionFunctionFile, str.c_str());
    else
      throw std::runtime_error(TRANSMISSION_FILENAME_ERR);
  }

    str = value(atts, "dnl");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < sizeof(d->detectorNonLinearityFile))
    strcpy(d->detectorNonLinearityFile, str.c_str());
      else
    throw std::runtime_error("Detector Non-Linearity Filename too long");
    }

}

//------------------------------------------------------------------------
// handler for <slit> (child of project)

CProjectSlitSubHandler::CProjectSlitSubHandler(CConfigHandler *master,
                           mediate_project_slit_t *slit) :
  CConfigSubHandler(master),
  m_slit(slit)
{
}

void CProjectSlitSubHandler::start(const map<Glib::ustring, string> &atts)
{
  string str;

  str = value(atts, "ref");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(m_slit->solarRefFile))
      strcpy(m_slit->solarRefFile, str.c_str());
    else
      throw std::runtime_error("Solar Reference Filename too long");
  }

  m_slit->applyFwhmCorrection = (value(atts, "fwhmcor") == "true") ? 1 : 0;

}

void CProjectSlitSubHandler::start(const Glib::ustring &element, const map<Glib::ustring, string> &atts)
{
  if (element == "slit_func") {

    return m_master->install_subhandler(new CSlitFunctionSubHandler(m_master, &(m_slit->function)), atts);
  }

}

//------------------------------------------------------------------------
// handler for <output> (child of project)

CProjectOutputSubHandler::CProjectOutputSubHandler(CConfigHandler *master,
                           mediate_project_output_t *output) :
  CSelectorSubHandler(master, &(output->selection)),
  m_output(output)
{
}

void CProjectOutputSubHandler::start(const map<Glib::ustring, string> &atts)
{
  string str;

  str = value(atts, "path");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(m_output->path))
      strcpy(m_output->path, str.c_str());
    else
      throw std::runtime_error("Output path too long");
  }

  m_output->analysisFlag = (value(atts, "anlys") == "true") ? 1 : 0;
  m_output->calibrationFlag = (value(atts, "calib") == "true") ? 1 : 0;
  m_output->configurationFlag = (value(atts, "conf") == "true") ? 1 : 0;
  m_output->referenceFlag = (value(atts, "ref") == "true");
  // m_output->newcalibFlag = (value(atts, "newcalib") == "true");
  m_output->directoryFlag = (value(atts, "dirs") == "true") ? 1 : 0;
  m_output->filenameFlag = (value(atts, "file") == "true") ? 1 : 0;

  str = value(atts, "success");
  if (str.empty())
   m_output->successFlag=1;
  else
   m_output->successFlag = (str == "true") ? 1 : 0;


  str = value(atts, "flux");
  if (!str.empty()) {
    if (str.length()+1 < sizeof(m_output->flux))
      strcpy(m_output->flux, str.c_str());
    else
      throw std::runtime_error("Output flux too long");
  }

  m_output->bandWidth = parse_value<double>(atts, "bandWidth", 1.);

  str = value(atts, "swathName");
  if (!str.empty()) {
    if ((size_t) str.length()+1 < sizeof(m_output->swath_name))
      strcpy(m_output->swath_name, str.c_str());
    else
      throw std::runtime_error("Output file swath name too long.");
  }

  str = value(atts, "fileFormat");
  if (!str.empty()) {
    enum output_format format = output_get_format(str.c_str());
    if ((format == ASCII)  || (format==NETCDF))
     m_output->file_format = format;
    else
     m_output->file_format=ASCII;
  }

}

//------------------------------------------------------------------------
// handler for <export> (child of project)

CProjectExportSubHandler::CProjectExportSubHandler(CConfigHandler *master,
                           mediate_project_export_t *exportSpectra) :
  CSelectorSubHandler(master, &(exportSpectra->selection)),
  m_export(exportSpectra)
{
}

void CProjectExportSubHandler::start(const map<Glib::ustring, string> &atts)
{
  string str;

  str = value(atts, "path");
  if (!str.empty()) {
    str = m_master->pathExpand(str);
    if (str.length() < sizeof(m_export->path))
      strcpy(m_export->path, str.c_str());
    else
      throw std::runtime_error("Export path too long");
  }

  m_export->titlesFlag = (value(atts, "titles") == "true") ? 1 : 0;
  m_export->directoryFlag = (value(atts, "dir") == "true") ? 1 : 0;

}
