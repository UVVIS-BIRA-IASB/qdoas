
/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include "CWOutputSelector.h"

#include "doas.h"
#include "constants.h"

#include "debugutil.h"

static void getValidFieldFlags(int *validFlags, int instrument,int selectorOrigin);

CWOutputSelector::CWOutputSelector(const data_select_list_t *d, QWidget *parent) :
  QFrame(parent)
{
  QGridLayout *mainLayout = new QGridLayout(this);

  mainLayout->addWidget(new QLabel("Available Fields"), 0, 0, Qt::AlignCenter);
  mainLayout->addWidget(new QLabel("Selected Fields"), 0, 2, Qt::AlignCenter);

  m_availableList = new QListWidget(this);
  m_chosenList = new QListWidget(this);

  QPushButton *toChosenBtn = new QPushButton(QIcon(":/icons/to_arrow.png"), QString(), this);
  QPushButton *toAvailableBtn = new QPushButton(QIcon(":/icons/from_arrow.png"), QString(), this);

  mainLayout->addWidget(m_availableList, 1, 0, 4, 1);
  mainLayout->addWidget(toChosenBtn, 2, 1);
  mainLayout->addWidget(toAvailableBtn, 3, 1);
  mainLayout->addWidget(m_chosenList, 1, 2, 4, 1);

  mainLayout->setRowMinimumHeight(1, 20);
  mainLayout->setRowMinimumHeight(4, 20);
  mainLayout->setRowStretch(1, 1);
  mainLayout->setRowStretch(4, 1);

  // multi selection
  m_availableList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_chosenList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // populate the with all possible fields initially.
  // These CAN be added in any desired order (the display order in the availble list)

  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SPECNO,                 "Spec No"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_NAME,                   "Name"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_DATE_TIME,              "Date & time (YYYYMMDDhhmmss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_DATE,                   "Date (DD/MM/YYYY)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_TIME,                   "Time (hh:mm:ss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_YEAR,                   "Year"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_JULIAN,                 "Day number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_JDFRAC,                 "Fractional day"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_TIFRAC,                 "Fractional time"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SCANS,                  "Scans"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_NREJ,                   "Rejected"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_TINT,                   "Tint"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SZA,                    "SZA"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CHI,                    "Chi Square"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_RMS,                    "RMS"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_AZIM,                   "Solar Azimuth angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_TDET,                   "Tdet"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SKY,                    "Sky Obs"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_BESTSHIFT,              "Best shift"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_REFZM,                  "Ref SZA"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_REFNUMBER,              "Ref number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_REFSHIFT,               "Ref2/Ref1 shift"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_PIXEL,                  "Pixel number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_PIXEL_TYPE,             "Pixel type"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ORBIT,                  "Orbit number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LONGIT,                 "Longitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LATIT,                  "Latitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LON_CORNERS,            "Pixel corner longitudes"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LAT_CORNERS,            "Pixel corner latitudes"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ALTIT,                  "Altitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_COVAR,                  "Covariances"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CORR,                   "Correlations"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CLOUD,                  "Cloud fraction"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_O3,                     "GDP O3 VCD"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_NO2,                    "GDP NO2 VCD"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CLOUDTOPP,              "Cloud Top Pressure"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LOS_ZA,                 "LoS ZA"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LOS_AZIMUTH,            "LoS Azimuth"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SAT_HEIGHT,             "Satellite height"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SAT_LAT,                "Satellite latitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SAT_LON,                "Satellite longitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SAT_SZA,                "Solar zenith angle at satellite"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SAT_SAA,                "Solar azimuth angle at satellite"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SAT_VZA,                "Viewing zenith angle at satellite"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_EARTH_RADIUS,           "Earth radius"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_VIEW_ELEVATION,         "Elev. viewing angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_VIEW_AZIMUTH,           "Azim. viewing angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_VIEW_ZENITH,            "Zenith viewing angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SCIA_QUALITY,           "SCIAMACHY Quality Flag"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SCIA_STATE_INDEX,       "SCIAMACHY State Index"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SCIA_STATE_ID,          "SCIAMACHY State Id"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_STARTDATE,              "Start Date"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ENDDATE,                "End Date"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_STARTTIME,              "Start Time (hhmmss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ENDTIME,                "Stop Time (hhmmss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SCANNING,               "Scanning angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_FILTERNUMBER,           "Filter number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_MEASTYPE,               "Measurement type"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CCD_HEADTEMPERATURE,    "Head temperature"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_COOLING_STATUS,         "Cooler status"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_MIRROR_ERROR,           "Mirror status"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_COMPASS,                "Compass angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_PITCH,                  "Pitch angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ROLL,                   "Roll angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ITER,                   "Iteration number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ERROR_FLAG,             "Processing error flag"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_NUM_BANDS,              "Number of wavelength bands used"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LAMBDA_CENTER, "Central pixel wavelength"));

  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_MDR_NUMBER,    "MDR number"         ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_OBSERVATION_INDEX,    "Index of observation within MDR"         ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_SCANDIRECTION,    "Scan direction"         ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_OBSERVATION_MODE, "Observation mode"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_SAA,              "SAA flag"               ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_SUNGLINT_RISK,    "Sunglint risk flag"     ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK,"Sunglint high risk flag"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GOME2_RAINBOW,          "Rainbow flag"           ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CCD_DIODES,             "Diodes"                 ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CCD_TARGETAZIMUTH,      "Target azimuth"           ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CCD_TARGETELEVATION,    "Target elevation"          ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SATURATED,              "Saturated flag"           ));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_INDEX_ALONGTRACK,       "Along-track index"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_INDEX_CROSSTRACK,       "Cross-track index"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GROUNDP_QF,             "Ground pixel quality flags"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_XTRACK_QF,              "Xtrack quality flags"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_PIXELS_QF,              "Rejected pixels based on quality flags"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_OMI_CONFIGURATION_ID,   "Instrument configuration id"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SPIKES,                 "Pixels with spikes in residual"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_SERVO_BYTE_SENT,    "Servo position byte sent"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED,"Servo position byte received"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_INSIDE_TEMP,        "Inside Temperature (deg C)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_OUTSIDE_TEMP,       "Outside Temperature (deg C)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_PRESSURE,           "Pressure (hPa)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_HUMIDITY,           "Humidity"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_DEWPOINT,           "Dewpoint"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_PITCH,              "Pitch"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_ROLL,               "Roll"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_UAV_HEADING,            "Heading"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_STARTGPSTIME,           "Start Time (hhmmss.ms)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ENDGPSTIME,             "End Time (hhmmss.ms)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LONGITEND,              "Longitude End"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LATITEND,               "Latitude End"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ALTITEND,               "Altitude End"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_TOTALEXPTIME,           "Total Measurement Time"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_TOTALACQTIME,           "Total Acquisition Time"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_LAMBDA,                 "Lambda"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SPECTRA,                "Spectra"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_FILENAME,               "File name"));

  // if (selectorOrigin==TAB_SELECTOR_OUTPUT)
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_SCANINDEX,              "Scan index"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ZENITH_BEFORE,          "Zenith before index"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ZENITH_AFTER,           "Zenith after index"));

  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_PRECALCULATED_FLUXES,   "Precalculated fluxes"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_RC,                     "Return code"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_RESIDUAL_SPECTRUM ,     "Residual spectrum"));

  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_CLOUDMASK ,             "Cloud mask"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_RED ,                   "Red color image"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_GREEN ,                 "Green color image"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_BLUE ,                  "Blue color image"));
  // populate the selected list by key-reference to the available list ...

  int key;
  int i = 0;
  while (i < d->nSelected) {
    key = d->selected[i];

    // locate corresponding item, hide it, and add matching item, in the chosen list
    QListWidgetItem *avItem = locateItemByKey(m_availableList, key);
    if (avItem) {
      m_chosenList->addItem(new CWOutputFieldItem(key, avItem->text()));
      avItem->setHidden(true);
    }
    ++i;
  }

  connect(toChosenBtn, SIGNAL(clicked()), this, SLOT(slotToChosenList()));
  connect(toAvailableBtn, SIGNAL(clicked()), this, SLOT(slotToAvailableList()));
}

void CWOutputSelector::apply(data_select_list_t *d)
{
  int n = 0;
  int row = 0;
  while (row < m_chosenList->count()) {
    QListWidgetItem *item = m_chosenList->item(row);
     if (item && !item->isHidden() ) {
      d->selected[n] = item->data(Qt::UserRole).toInt();
      ++n;
    }
    ++row;
  }
  d->nSelected = n;
}

void CWOutputSelector::setInstrument(int instrument,int selectorOrigin)
{
  int validFlags[PRJCT_RESULTS_MAX];

  // get this information from somewhere ...
  getValidFieldFlags(validFlags, instrument,selectorOrigin);

  for (int key=0; key<PRJCT_RESULTS_MAX; ++key) {
    bool hideItems = (validFlags[key] == 0);
    QListWidgetItem *avItem = locateItemByKey(m_availableList, key);
    QListWidgetItem *chItem = locateItemByKey(m_chosenList, key);
    if (chItem)
      chItem->setHidden(hideItems);
    if (avItem)
      avItem->setHidden(hideItems || chItem != NULL);
  }
}

void CWOutputSelector::slotToChosenList()
{
  int key;
  QList<QListWidgetItem*> items = m_availableList->selectedItems();
  QList<QListWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    if (!(*it)->isHidden()) {
      // hide these items and add to the chosen list
      key = (*it)->data(Qt::UserRole).toInt();
      m_chosenList->addItem(new CWOutputFieldItem(key, (*it)->text()));
      (*it)->setHidden(true);
    }
    ++it;
  }
  m_availableList->selectionModel()->clearSelection();
}

void CWOutputSelector::slotToAvailableList()
{
  int key;
  QList<QListWidgetItem*> items = m_chosenList->selectedItems();
  QList<QListWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    key = (*it)->data(Qt::UserRole).toInt();
    // find the corresponding item in the available list and make it 'un-hidden'
    QListWidgetItem *avItem = locateItemByKey(m_availableList, key);
    if (avItem)
      avItem->setHidden((*it)->isHidden());
    // row of the selected item
    key = m_chosenList->row(*it);
    ++it;
    delete m_chosenList->takeItem(key); // discard the selected item
  }
}

QListWidgetItem* CWOutputSelector::locateItemByKey(QListWidget *listWidget, int key)
{
  QListWidgetItem *item;
  int row = 0;

  while (row < listWidget->count()) {
    item = listWidget->item(row);
    if (item && key == item->data(Qt::UserRole).toInt())
      return item;

    ++row;
  }
  // not found
  return NULL;
}

//------------------------------------------------------

CWOutputFieldItem::CWOutputFieldItem(int key, const QString &text, QListWidget * parent) :
  QListWidgetItem(text, parent),
  m_key(key)
{
}

QVariant CWOutputFieldItem::data(int role) const
{
  if (role == Qt::UserRole)
    return QVariant(m_key);

  return QListWidgetItem::data(role);
}

//------------------------------------------------------

void getValidFieldFlags(int *validFlags, int instrument,int selectorOrigin)
 {
   int satelliteFlag = is_satellite(static_cast<enum _prjctInstrFormat>(instrument));
   int maxdoasFlag = is_maxdoas(static_cast<enum _prjctInstrFormat>(instrument));

  // validFlags is indexed by the PRJCT_RESULTS_* enumerated values. A non-zero
  // value means that the corresponding field is valid for the instrument. The value of
  // instrument should correspond to a PRJCT_INSTR_* enumerated value.
  // The validFlags array MUST have a length of at least PRJCT_RESULTS_MAX.

  // zero all flags
  memset(validFlags, 0, PRJCT_RESULTS_MAX * sizeof(int));

  // Fields that are common to all formats (note that date and time fields could be absent from ASCII file)

  validFlags[PRJCT_RESULTS_SPECNO]=                                       // record number
  validFlags[PRJCT_RESULTS_DATE_TIME]=                                    // date and time
  validFlags[PRJCT_RESULTS_DATE]=                                         // date
  validFlags[PRJCT_RESULTS_TIME]=                                         // time
  validFlags[PRJCT_RESULTS_YEAR]=                                         // year
  validFlags[PRJCT_RESULTS_JULIAN]=                                       // julian day
  validFlags[PRJCT_RESULTS_JDFRAC]=                                       // fractional julian day
  validFlags[PRJCT_RESULTS_TIFRAC]=                                       // fractional time
  validFlags[PRJCT_RESULTS_SZA]=                                          // solar zenith angle (can be calculated if date, time and observation site specified)
  validFlags[PRJCT_RESULTS_AZIM]=                                         // solar azimuth angle (can be calculated if date, time and observation site specified)
  validFlags[PRJCT_RESULTS_TINT]=1;                                       // the integration time

  validFlags[PRJCT_RESULTS_COVAR]=(selectorOrigin!=TAB_SELECTOR_EXPORT)?1:0;
  validFlags[PRJCT_RESULTS_CORR]=(selectorOrigin!=TAB_SELECTOR_EXPORT)?1:0;
  validFlags[PRJCT_RESULTS_RC]=((selectorOrigin==TAB_SELECTOR_OUTPUT) && (instrument==PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF))?1:0;
  validFlags[PRJCT_RESULTS_RESIDUAL_SPECTRUM]=(selectorOrigin==TAB_SELECTOR_OUTPUT)?1:0;

  // Output fields related to overall analysis (or run calibration) results (per analysis window)

  if (selectorOrigin!=TAB_SELECTOR_EXPORT)
   {
     validFlags[PRJCT_RESULTS_REFZM]=(satelliteFlag)?0:1;                    // in automatic reference selection, the solar zenith angle of the reference spectrum
     validFlags[PRJCT_RESULTS_REFNUMBER]=(satelliteFlag)?0:1;                // in automatic reference selection, the index of the reference spectrum
     validFlags[PRJCT_RESULTS_CHI]=                                          // chi square
       validFlags[PRJCT_RESULTS_RMS]=                                          // RMS
       validFlags[PRJCT_RESULTS_REFSHIFT]=                                     // in automatic reference selection, shift of the reference spectrum
       validFlags[PRJCT_RESULTS_ITER]=
       validFlags[PRJCT_RESULTS_ERROR_FLAG]=
       validFlags[PRJCT_RESULTS_NUM_BANDS]=
       validFlags[PRJCT_RESULTS_LAMBDA_CENTER] =
       validFlags[PRJCT_RESULTS_SPIKES]=1;
   }

  validFlags[PRJCT_RESULTS_SKY]=0;                                        // information on the sky (never used except EASOE campaign, 1991 !)                                      // for satellite measurements, several spectra are averaged

  validFlags[PRJCT_RESULTS_LONGIT]=satelliteFlag;
  validFlags[PRJCT_RESULTS_LATIT]=satelliteFlag;
  validFlags[PRJCT_RESULTS_SAT_HEIGHT]=satelliteFlag;
  validFlags[PRJCT_RESULTS_EARTH_RADIUS]=satelliteFlag;
  validFlags[PRJCT_RESULTS_ORBIT]=satelliteFlag;

  validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=0;
  validFlags[PRJCT_RESULTS_VIEW_ZENITH]=satelliteFlag;
  validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=satelliteFlag;
  validFlags[PRJCT_RESULTS_LOS_ZA]=satelliteFlag;
  validFlags[PRJCT_RESULTS_LOS_AZIMUTH]=satelliteFlag;

  validFlags[PRJCT_RESULTS_TOTALACQTIME]=
  validFlags[PRJCT_RESULTS_TOTALEXPTIME]=!satelliteFlag;
  validFlags[PRJCT_RESULTS_MEASTYPE]=maxdoasFlag;

  // set the appropriate flags

  switch (instrument)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ASCII:
     {
      validFlags[PRJCT_RESULTS_DATE]=1;
      validFlags[PRJCT_RESULTS_TIME]=1;
      validFlags[PRJCT_RESULTS_YEAR]=1;
      validFlags[PRJCT_RESULTS_JULIAN]=1;
      validFlags[PRJCT_RESULTS_JDFRAC]=1;
      validFlags[PRJCT_RESULTS_TIFRAC]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_TINT]=1;
      validFlags[PRJCT_RESULTS_SZA]=1;
      validFlags[PRJCT_RESULTS_AZIM]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_ALTIT]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_STARTDATE]=1;
      validFlags[PRJCT_RESULTS_ENDDATE]=1;
      validFlags[PRJCT_RESULTS_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ENDTIME]=1;
     }
    break;
 // ----------------------------------------------------------------------------
#ifdef PRJCT_INSTR_FORMAT_OLD    
    case PRJCT_INSTR_FORMAT_ACTON:
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER:
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;                         // absent for Harestua]=1; present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;                           // absent for Harestua]=1; present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_COOLING_STATUS]=1;
      validFlags[PRJCT_RESULTS_MIRROR_ERROR]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG :
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;                         // absent for Harestua]=1; present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;                           // absent for Harestua]=1; present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG_OLD :                                        // Before spring 94]=1; no viewing angles
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
     }
    break;
#endif    
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_VIS :                                          // SAOZ PCD/NMOS UV-Visible
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_BESTSHIFT]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_ALTIT]=1;
      validFlags[PRJCT_RESULTS_UAV_SERVO_BYTE_SENT]=1;
      validFlags[PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED]=1;
      validFlags[PRJCT_RESULTS_UAV_INSIDE_TEMP]=1;
      validFlags[PRJCT_RESULTS_UAV_OUTSIDE_TEMP]=1;
      validFlags[PRJCT_RESULTS_UAV_PRESSURE]=1;
      validFlags[PRJCT_RESULTS_UAV_HUMIDITY]=1;
      validFlags[PRJCT_RESULTS_UAV_DEWPOINT]=1;
      validFlags[PRJCT_RESULTS_UAV_PITCH]=1;
      validFlags[PRJCT_RESULTS_UAV_ROLL]=1;
      validFlags[PRJCT_RESULTS_UAV_HEADING]=1;
      validFlags[PRJCT_RESULTS_LONGITEND]=1;
      validFlags[PRJCT_RESULTS_LATITEND]=1;
      validFlags[PRJCT_RESULTS_ALTITEND]=1;
      validFlags[PRJCT_RESULTS_STARTGPSTIME]=1;
      validFlags[PRJCT_RESULTS_ENDGPSTIME]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_MEASTYPE]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_MOBILE :
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_ALTIT]=1;
      validFlags[PRJCT_RESULTS_LONGITEND]=1;
      validFlags[PRJCT_RESULTS_LATITEND]=1;
      validFlags[PRJCT_RESULTS_ALTITEND]=1;
      validFlags[PRJCT_RESULTS_STARTGPSTIME]=1;
      validFlags[PRJCT_RESULTS_ENDGPSTIME]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_UAV_INSIDE_TEMP]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :
#ifdef PRJCT_INSTR_FORMAT_OLD     
    case PRJCT_INSTR_FORMAT_RASAS :
#endif     
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_ALTIT]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;                                 // not present in all measurements]=1; but could be in the next future
      validFlags[PRJCT_RESULTS_SCANNING]=1;
      validFlags[PRJCT_RESULTS_FILTERNUMBER]=1;
      validFlags[PRJCT_RESULTS_CCD_HEADTEMPERATURE]=1;
      validFlags[PRJCT_RESULTS_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_COMPASS]=1;                                      // TODO : separate CCD_EEV ground-based and airborne
      validFlags[PRJCT_RESULTS_PITCH]=1;
      validFlags[PRJCT_RESULTS_ROLL]=1;
      validFlags[PRJCT_RESULTS_CCD_DIODES]=1;
      validFlags[PRJCT_RESULTS_CCD_TARGETAZIMUTH]=1;
      validFlags[PRJCT_RESULTS_CCD_TARGETELEVATION]=1;
      validFlags[PRJCT_RESULTS_SATURATED]=1;
      validFlags[PRJCT_RESULTS_PRECALCULATED_FLUXES]=1;
     }
    break;
 // ----------------------------------------------------------------------------
#ifdef PRJCT_INSTR_FORMAT_OLD    
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
     }
    break;
#endif    
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
      validFlags[PRJCT_RESULTS_STARTDATE]=1;
      validFlags[PRJCT_RESULTS_ENDDATE]=1;
      validFlags[PRJCT_RESULTS_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_FILTERNUMBER]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :
    case PRJCT_INSTR_FORMAT_MFC_STD :
    case PRJCT_INSTR_FORMAT_MFC_BIRA :
     {
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;

      if (instrument!=PRJCT_INSTR_FORMAT_MFC_BIRA)
       validFlags[PRJCT_RESULTS_FILENAME]=1;

     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_BIN :
    case PRJCT_INSTR_FORMAT_GOME1_NETCDF :
     {
      validFlags[PRJCT_RESULTS_PIXEL]=1;
      validFlags[PRJCT_RESULTS_PIXEL_TYPE]=1;
      validFlags[PRJCT_RESULTS_LOS_ZA]=0;
      validFlags[PRJCT_RESULTS_LOS_AZIMUTH]=0;
      validFlags[PRJCT_RESULTS_TINT]=0;
      validFlags[PRJCT_RESULTS_CLOUD]=1;
      validFlags[PRJCT_RESULTS_CLOUDTOPP]=1;
      validFlags[PRJCT_RESULTS_LON_CORNERS]=1;
      validFlags[PRJCT_RESULTS_LAT_CORNERS]=1;

      if (instrument==PRJCT_INSTR_FORMAT_GOME1_NETCDF)
       {
        validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
        validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
        validFlags[PRJCT_RESULTS_SAT_SAA]=1;
        validFlags[PRJCT_RESULTS_SAT_SZA]=1;
        validFlags[PRJCT_RESULTS_SAT_VZA]=1;
       }
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SCIA_PDS :
     {
      validFlags[PRJCT_RESULTS_SCIA_STATE_INDEX]=1;
      validFlags[PRJCT_RESULTS_SCIA_STATE_ID]=1;
      validFlags[PRJCT_RESULTS_SCIA_QUALITY]=1;
      validFlags[PRJCT_RESULTS_SAT_LAT]=1;
      validFlags[PRJCT_RESULTS_SAT_LON]=1;
      validFlags[PRJCT_RESULTS_LON_CORNERS]=1;
      validFlags[PRJCT_RESULTS_LAT_CORNERS]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GOME2 :
     {
      validFlags[PRJCT_RESULTS_CLOUD]=1;
      validFlags[PRJCT_RESULTS_CLOUDTOPP]=1;

      validFlags[PRJCT_RESULTS_GOME2_MDR_NUMBER]=1;
      validFlags[PRJCT_RESULTS_GOME2_OBSERVATION_INDEX]=1;
      validFlags[PRJCT_RESULTS_GOME2_SCANDIRECTION]=1;
      validFlags[PRJCT_RESULTS_GOME2_OBSERVATION_MODE]=1;
      validFlags[PRJCT_RESULTS_GOME2_SAA]=1;
      validFlags[PRJCT_RESULTS_GOME2_SUNGLINT_RISK]=1;
      validFlags[PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK]=1;
      validFlags[PRJCT_RESULTS_GOME2_RAINBOW]=1;
      validFlags[PRJCT_RESULTS_LON_CORNERS]=1;
      validFlags[PRJCT_RESULTS_LAT_CORNERS]=1;
      validFlags[PRJCT_RESULTS_SAT_LAT]=1;
      validFlags[PRJCT_RESULTS_SAT_LON]=1;
      validFlags[PRJCT_RESULTS_SAT_SAA]=1;
      validFlags[PRJCT_RESULTS_SAT_SZA]=1;
      validFlags[PRJCT_RESULTS_SAT_VZA]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MKZY :
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_NREJ]=1;
      validFlags[PRJCT_RESULTS_NAME]=1;
      validFlags[PRJCT_RESULTS_SCANNING]=1;
      validFlags[PRJCT_RESULTS_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_ALTIT]=1;
      validFlags[PRJCT_RESULTS_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OCEAN_OPTICS :
     {
      validFlags[PRJCT_RESULTS_SCANS]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMI :
     {
      validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
      validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
      validFlags[PRJCT_RESULTS_GROUNDP_QF]=1;
      validFlags[PRJCT_RESULTS_XTRACK_QF]=1;
      validFlags[PRJCT_RESULTS_OMI_CONFIGURATION_ID]=1;

      if (selectorOrigin!=TAB_SELECTOR_EXPORT)
       validFlags[PRJCT_RESULTS_PIXELS_QF]=1;

      validFlags[PRJCT_RESULTS_SAT_LAT]=1;
      validFlags[PRJCT_RESULTS_SAT_LON]=1;
     }
    break;
 // ----------------------------------------------------------------------------
   case PRJCT_INSTR_FORMAT_OMIV4:
     {
      validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
      validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
      validFlags[PRJCT_RESULTS_XTRACK_QF]=1;
      validFlags[PRJCT_RESULTS_GROUNDP_QF]=1;

      validFlags[PRJCT_RESULTS_SAT_LAT]=1;
      validFlags[PRJCT_RESULTS_SAT_LON]=1;
      validFlags[PRJCT_RESULTS_LON_CORNERS]=1;
      validFlags[PRJCT_RESULTS_LAT_CORNERS]=1;
     }
     break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMPS :
     {
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_VIEW_ZENITH]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_LOS_ZA]=1;
      validFlags[PRJCT_RESULTS_LOS_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
      validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
     }
    break;

 // ----------------------------------------------------------------------------

   case PRJCT_INSTR_FORMAT_TROPOMI:
     {
       validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
       validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
       validFlags[PRJCT_RESULTS_LON_CORNERS]=1;
       validFlags[PRJCT_RESULTS_LAT_CORNERS]=1;
       validFlags[PRJCT_RESULTS_SAT_LAT]=1;
       validFlags[PRJCT_RESULTS_SAT_LON]=1;
       validFlags[PRJCT_RESULTS_GROUNDP_QF]=1;
     }

 // ----------------------------------------------------------------------------

   case PRJCT_INSTR_FORMAT_APEX :
     {
       validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
       validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
       validFlags[PRJCT_RESULTS_STARTDATE]=1;
       validFlags[PRJCT_RESULTS_ENDDATE]=1;
       validFlags[PRJCT_RESULTS_STARTTIME]=1;
       validFlags[PRJCT_RESULTS_ENDTIME]=1;
       validFlags[PRJCT_RESULTS_LONGIT]=1;
       validFlags[PRJCT_RESULTS_LATIT]=1;
       validFlags[PRJCT_RESULTS_VIEW_ZENITH]=1;
       validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
       validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
       validFlags[PRJCT_RESULTS_SCANS]=1;
       validFlags[PRJCT_RESULTS_NREJ]=1;
     }
     break;

 // ----------------------------------------------------------------------------

   case PRJCT_INSTR_FORMAT_GEMS :
     {
      validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
      validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_VIEW_ZENITH]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_GROUNDP_QF]=1;
      validFlags[PRJCT_RESULTS_XTRACK_QF]=1;
      validFlags[PRJCT_RESULTS_SAT_HEIGHT]=1;
      validFlags[PRJCT_RESULTS_SAT_LAT]=1;
      validFlags[PRJCT_RESULTS_SAT_LON]=1;
     }
     break;

   case PRJCT_INSTR_FORMAT_TEMPO :
     {
      validFlags[PRJCT_RESULTS_INDEX_ALONGTRACK]=1;
      validFlags[PRJCT_RESULTS_INDEX_CROSSTRACK]=1;
      validFlags[PRJCT_RESULTS_LON_CORNERS]=1;
      validFlags[PRJCT_RESULTS_LAT_CORNERS]=1;

      validFlags[PRJCT_RESULTS_GROUNDP_QF]=1;
      validFlags[PRJCT_RESULTS_CLOUDMASK]=1;
      validFlags[PRJCT_RESULTS_RED]=1;
      validFlags[PRJCT_RESULTS_GREEN]=1;
      validFlags[PRJCT_RESULTS_BLUE]=1;
     }
     break;

 // ----------------------------------------------------------------------------

   case PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF :
     {
      validFlags[PRJCT_RESULTS_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_VIEW_AZIMUTH]=1;                                 // not present in all measurements]=1; but could be in the next future
      validFlags[PRJCT_RESULTS_STARTDATE]=1;
      validFlags[PRJCT_RESULTS_ENDDATE]=1;
      validFlags[PRJCT_RESULTS_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_LONGIT]=1;
      validFlags[PRJCT_RESULTS_LATIT]=1;
      validFlags[PRJCT_RESULTS_ALTIT]=1;
      validFlags[PRJCT_RESULTS_SCANINDEX]=1;
      validFlags[PRJCT_RESULTS_SCANS]=1;
      validFlags[PRJCT_RESULTS_ZENITH_BEFORE]=1;
      validFlags[PRJCT_RESULTS_ZENITH_AFTER]=1;
     }
     break;

 // ----------------------------------------------------------------------------

   default:
     break;
 // ----------------------------------------------------------------------------
   }

  if (selectorOrigin==TAB_SELECTOR_EXPORT)
   {
       validFlags[PRJCT_RESULTS_LAMBDA]=1;
       validFlags[PRJCT_RESULTS_SPECTRA]=1;
   }
 }
