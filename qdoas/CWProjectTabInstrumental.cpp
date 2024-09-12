/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFontMetrics>
#include <QFileDialog>

#include <stdexcept>

#include "CWProjectTabInstrumental.h"
#include "CValidator.h"
#include "CWSiteListCombo.h"
#include "CWorkSpace.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

StrayLightConfig::StrayLightConfig(Qt::Orientation orientation, QWidget *parent) : QGroupBox("Correct straylight bias", parent),
                                                                                   m_lambdaMinEdit(new QLineEdit(this) ),
                                                                                   m_lambdaMaxEdit(new QLineEdit(this) ) {
  this->setCheckable(true);
  m_lambdaMinEdit->setFixedWidth(50);
  CDoubleFixedFmtValidator *validator = new CDoubleFixedFmtValidator(0.0, 900.0, 2, this);
  m_lambdaMinEdit->setValidator(validator);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(validator);

  QLabel *minLabel = new QLabel("Wavelength min", this);
  QLabel *maxLabel = new QLabel("Wavelength max", this);

  if (orientation == Qt::Vertical) {
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(minLabel, 0, 0, Qt::AlignRight);
    layout->addWidget(m_lambdaMinEdit, 0, 1, Qt::AlignLeft);
    layout->addWidget(maxLabel, 1, 0, Qt::AlignRight);
    layout->addWidget(m_lambdaMaxEdit, 1, 1, Qt::AlignLeft);
  } else {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(minLabel);
    layout->addWidget(m_lambdaMinEdit);
    layout->addWidget(maxLabel);
    layout->addWidget(m_lambdaMaxEdit);
    layout->addStretch(1);
  }
}

static void setDouble(QLineEdit &lineEdit, double lambda) {
  QString tmp;
  tmp.setNum(lambda);
  lineEdit.validator()->fixup(tmp);
  lineEdit.setText(tmp);
}

void StrayLightConfig::setLambdaMin(double lambda) {
  setDouble(*m_lambdaMinEdit, lambda);
};

void StrayLightConfig::setLambdaMax(double lambda) {
  setDouble(*m_lambdaMaxEdit, lambda);
}

double StrayLightConfig::getLambdaMax() const {
  return m_lambdaMaxEdit->text().toDouble();
}

double StrayLightConfig::getLambdaMin() const {
  return m_lambdaMinEdit->text().toDouble();
}

CWProjectTabInstrumental::CWProjectTabInstrumental(const mediate_project_instrumental_t *instr, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(25);
  
  // QGridLayout *topLayout = new QGridLayout;

  m_formatStack = new QStackedWidget(this);
  // insert widgets into the stack, and store their index in the map - keyed by the instrument format

  // ascii
  m_asciiEdit = new CWInstrAsciiEdit(&(instr->ascii));
  index = m_formatStack->addWidget(m_asciiEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_ASCII, index));

  #ifdef PRJCT_INSTR_FORMAT_OLD

  // logger
  m_loggerEdit = new CWInstrLoggerEdit(&(instr->logger));
  index = m_formatStack->addWidget(m_loggerEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_LOGGER, index));

  // acton
  m_actonEdit = new CWInstrActonEdit(&(instr->acton));
  index = m_formatStack->addWidget(m_actonEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_ACTON, index));

  // pdaegg
  m_pdaEggEdit = new CWInstrLoggerEdit(&(instr->pdaegg));
  index = m_formatStack->addWidget(m_pdaEggEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_PDAEGG, index));

  // pdaeggold
  m_pdaEggOldEdit = new CWInstrLoggerEdit(&(instr->pdaeggold));
  index = m_formatStack->addWidget(m_pdaEggOldEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_PDAEGG_OLD, index));

  // ccdohp96
  m_ccdOhp96Edit = new CWInstrCcdEdit(&(instr->ccdohp96));
  index = m_formatStack->addWidget(m_ccdOhp96Edit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_CCD_OHP_96, index));

  // ccdha94
  m_ccdHa94Edit = new CWInstrCcdEdit(&(instr->ccdha94));
  index = m_formatStack->addWidget(m_ccdHa94Edit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_CCD_HA_94, index));

  #endif

  // saozvis
  m_saozVisEdit = new CWInstrSaozEdit(&(instr->saozvis));
  index = m_formatStack->addWidget(m_saozVisEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_SAOZ_VIS, index));

  // saozefm
  m_saozEfmEdit = new CWInstrMinimumEdit(&(instr->saozefm));
  index = m_formatStack->addWidget(m_saozEfmEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_SAOZ_EFM, index));

  // mfc
  m_mfcEdit = new CWInstrMfcEdit(&(instr->mfc));
  index = m_formatStack->addWidget(m_mfcEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MFC, index));

  // mfcstd
  m_mfcStdEdit = new CWInstrMfcStdEdit(&(instr->mfcstd));
  index = m_formatStack->addWidget(m_mfcStdEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MFC_STD, index));

  // mfcbira
  m_mfcbiraEdit = new CWInstrMfcbiraEdit(&(instr->mfcbira));
  index = m_formatStack->addWidget(m_mfcbiraEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MFC_BIRA, index));

  #ifdef PRJCT_INSTR_FORMAT_OLD
  // rasas
  m_rasasEdit = new CWInstrMinimumEdit(&(instr->rasas));
  index = m_formatStack->addWidget(m_rasasEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_RASAS, index));

   // pdasieasoe
  m_pdasiEasoeEdit = new CWInstrMinimumEdit(&(instr->pdasieasoe));
  index = m_formatStack->addWidget(m_pdasiEasoeEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_PDASI_EASOE, index));
  #endif

  // ccdeev
  m_ccdEevEdit = new CWInstrCcdEevEdit(&(instr->ccdeev));
  index = m_formatStack->addWidget(m_ccdEevEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_CCD_EEV, index));

  // gdpnetcdf
  m_gdpNetcdfEdit = new CWInstrGome1Edit(&(instr->gdpnetcdf));
  index = m_formatStack->addWidget(m_gdpNetcdfEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GOME1_NETCDF, index));

  // gdpbin
  m_gdpBinEdit = new CWInstrGdpEdit(&(instr->gdpbin));
  index = m_formatStack->addWidget(m_gdpBinEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GDP_BIN, index));

  // sciapds
  m_sciaPdsEdit = new CWInstrSciaEdit(&(instr->sciapds));
  index = m_formatStack->addWidget(m_sciaPdsEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_SCIA_PDS, index));

  // uoft
  m_uoftEdit = new CWInstrMinimumEdit(&(instr->uoft));
  index = m_formatStack->addWidget(m_uoftEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_UOFT, index));

  // noaa
  m_noaaEdit = new CWInstrMinimumEdit(&(instr->noaa));
  index = m_formatStack->addWidget(m_noaaEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_NOAA, index));

  // omi
  m_omiEdit = new CWInstrOmiEdit(&(instr->omi));
  index = m_formatStack->addWidget(m_omiEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_OMI, index));

  // omiv4
  m_omiV4Edit = new CWInstrOmiV4Edit(&(instr->omi));
  index = m_formatStack->addWidget(m_omiV4Edit);
  m_instrumentToStackIndexMap[PRJCT_INSTR_FORMAT_OMIV4] = index;

  // omps
  m_ompsEdit = new CWInstrOmpsEdit();
  index = m_formatStack->addWidget(m_ompsEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_OMPS, index));

  // tropomi
  m_tropomiEdit = new CWInstrTropomiEdit(&(instr->tropomi));
  index = m_formatStack->addWidget(m_tropomiEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_TROPOMI, index));

  // gome2
  m_gome2Edit = new CWInstrGome2Edit(&(instr->gome2));
  index = m_formatStack->addWidget(m_gome2Edit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GOME2, index));

  // mkzy pack
  m_mkzyEdit = new CWInstrMinimumEdit(&(instr->mkzy));
  index = m_formatStack->addWidget(m_mkzyEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MKZY, index));

  // bira airborne
  m_biraairborneEdit = new CWInstrAvantesEdit(&(instr->biraairborne));
  index = m_formatStack->addWidget(m_biraairborneEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_BIRA_AIRBORNE, index));

  // bira mobile
  m_biramobileEdit = new CWInstrAvantesEdit(&(instr->biramobile));
  index = m_formatStack->addWidget(m_biramobileEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_BIRA_MOBILE, index));

  // apex
  m_apexEdit = new CWInstrApexEdit(&(instr->apex));
  index = m_formatStack->addWidget(m_apexEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_APEX, index));

  // GEMS
  m_gemsEdit = new CWInstrGemsEdit(&(instr->gems));
  index = m_formatStack->addWidget(m_gemsEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GEMS, index));

  // ocean optics
  m_oceanOpticsEdit = new CWInstrOceanOpticsEdit(&(instr->oceanoptics));
  index = m_formatStack->addWidget(m_oceanOpticsEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_OCEAN_OPTICS    , index));

  // frm4doas
  m_frm4doasEdit = new CWInstrFrm4doasEdit(&(instr->frm4doas));
  index = m_formatStack->addWidget(m_frm4doasEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF, index));
  
  // Site

  m_siteFrame = new QFrame(this);
  m_siteFrame->setFrameStyle(QFrame::NoFrame);

  QHBoxLayout *siteLayout=new(QHBoxLayout),
              *saaLayout=new(QHBoxLayout);
  QVBoxLayout *topLayout = new QVBoxLayout(m_siteFrame);

  QLabel *siteLabel,*saaLabel;

  m_siteCombo = new CWSiteListCombo(m_siteFrame); // automatically populated

  siteLabel=new QLabel("Site", m_siteFrame);
  siteLabel->setFixedWidth(100);

  siteLayout->addWidget(siteLabel);
  siteLayout->addWidget(m_siteCombo);

  saaLabel=new QLabel("SAA convention", m_siteFrame);
  saaLabel->setFixedWidth(100);

  m_saaSRadioButton = new QRadioButton("-180..180, 0 degree South", m_siteFrame);
  m_saaNRadioButton = new QRadioButton("0-360, 0 degree North", m_siteFrame);

  saaLayout->addWidget(saaLabel);
  saaLayout->addWidget(m_saaSRadioButton);
  saaLayout->addWidget(m_saaNRadioButton);
  saaLayout->addStretch(1);

  topLayout->addLayout(siteLayout);
  topLayout->addLayout(saaLayout);

  // topLayout->setColumnMinimumWidth(0, 90);
  // topLayout->setColumnStretch(1, 1);

  mainLayout->addWidget(m_siteFrame);
  //mainLayout->addLayout(topLayout);
  mainLayout->addWidget(m_formatStack);
  mainLayout->addStretch(1);

  // set the current format - stack will follow
  slotInstrumentChanged(instr->format);

  index = m_siteCombo->findText(QString(instr->siteName));
  if (index != -1)
    m_siteCombo->setCurrentIndex(index);

  if (instr->saaConvention == PRJCT_INSTR_SAA_SOUTH)
    m_saaSRadioButton->setChecked(true);
  else if (instr->saaConvention == PRJCT_INSTR_SAA_NORTH)
    m_saaNRadioButton->setChecked(true);
 }

void CWProjectTabInstrumental::apply(mediate_project_instrumental_t *instr) const
{
  // set values for ALL instruments ... the selected mode is handled elsewhere (by the parent).

  QString siteName = m_siteCombo->currentText();
//  if (siteName != "No Site Specified" && siteName.length() < (int)sizeof(instr->siteName))
  strcpy(instr->siteName, siteName.toLocal8Bit().data());
  if (m_saaSRadioButton->isChecked()) instr->saaConvention = PRJCT_INSTR_SAA_SOUTH;
  if (m_saaNRadioButton->isChecked()) instr->saaConvention = PRJCT_INSTR_SAA_NORTH;

  m_asciiEdit->apply(&(instr->ascii));
  #ifdef PRJCT_INSTR_FORMAT_OLD
  m_loggerEdit->apply(&(instr->logger));
  m_actonEdit->apply(&(instr->acton));
  m_pdaEggEdit->apply(&(instr->pdaegg));
  m_pdaEggOldEdit->apply(&(instr->pdaeggold));
  m_ccdOhp96Edit->apply(&(instr->ccdohp96));
  m_ccdHa94Edit->apply(&(instr->ccdha94));
  #endif
  m_saozVisEdit->apply(&(instr->saozvis));
  m_saozEfmEdit->apply(&(instr->saozefm));
  m_mfcEdit->apply(&(instr->mfc));
  m_mfcStdEdit->apply(&(instr->mfcstd));
  m_mfcbiraEdit->apply(&(instr->mfcbira));
  #ifdef PRJCT_INSTR_FORMAT_OLD
  m_rasasEdit->apply(&(instr->rasas));
  m_pdasiEasoeEdit->apply(&(instr->pdasieasoe));
  #endif
  m_ccdEevEdit->apply(&(instr->ccdeev));
  m_gdpNetcdfEdit->apply(&(instr->gdpnetcdf));
  m_gdpBinEdit->apply(&(instr->gdpbin));
  m_sciaPdsEdit->apply(&(instr->sciapds));
  m_uoftEdit->apply(&(instr->uoft));
  m_noaaEdit->apply(&(instr->noaa));
  m_omiEdit->apply(&(instr->omi));
  m_omiV4Edit->apply(&(instr->omi));
  m_ompsEdit->apply();
  m_tropomiEdit->apply(&(instr->tropomi));
  m_apexEdit->apply(&(instr->apex));
  m_gome2Edit->apply(&(instr->gome2));
  m_mkzyEdit->apply(&(instr->mkzy));
  m_biraairborneEdit->apply(&(instr->biraairborne));
  m_biramobileEdit->apply(&(instr->biramobile));
  m_oceanOpticsEdit->apply(&(instr->oceanoptics));
  m_frm4doasEdit->apply(&(instr->frm4doas));
  m_gemsEdit->apply(&(instr->gems));
}

void CWProjectTabInstrumental::slotInstrumentChanged(int instrument)
{
  std::map<int,int>::const_iterator it = m_instrumentToStackIndexMap.find(instrument);
  if (it != m_instrumentToStackIndexMap.end()) {

    m_formatStack->setCurrentIndex(it->second);
  }
}

void CWProjectTabInstrumental::slotInstrumentTypeChanged(int instrumentType)
{
 if (instrumentType==PRJCT_INSTR_TYPE_SATELLITE)
  m_siteFrame->hide();
 else
  m_siteFrame->show();
 // m_siteFrame->setEnabled((instrumentType==PRJCT_INSTR_TYPE_SATELLITE)?0:1);
 // m_siteCombo->setEnabled((instrumentType==PRJCT_INSTR_TYPE_SATELLITE)?0:1);
}

//--------------------------------------------------------
// Specific Instrument Editors...

static const int cSuggestedColumnZeroWidth = 10; //120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 10; //100; // consistent
static const int cStandardEditWidth         = 70;

//--------------------------------------------------------

CWCalibInstrEdit::CWCalibInstrEdit(QWidget *parent) :
  QFrame(parent) {}

void CWCalibInstrEdit::slotCalibOneBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Calibration File",
                          pref->directoryName("Calib"),
                          "Calibration File (*.clb);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Calib", filename);

    m_fileOneEdit->setText(filename);
  }
}

void CWCalibInstrEdit::slotInstrTwoBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Instrument Function File",
                          pref->directoryName("Instr"),
                          "Instrument Function File (*.ins);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Instr", filename);

    m_fileTwoEdit->setText(filename);
  }
}

void CWCalibInstrEdit::slotOffsetTwoBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Offset File",
                          pref->directoryName("Offset"),
                          "Offset File (*.txt);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Offset", filename);

    m_fileTwoEdit->setText(filename);
  }
}

void CWCalibInstrEdit::helperConstructFileWidget(QLineEdit **fileEdit, QGridLayout *gridLayout, int &row,
                            const char *str, int len,
                            const char *label, const char *slot)
{
  // Helper that constructs the fileOneEdit editing widget and place it in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  gridLayout->addWidget(new QLabel(label, this), row, 0);
  QLineEdit *edit = new QLineEdit(this);
  edit->setMaxLength(len-1);
  gridLayout->addWidget(edit, row, 1);
  QPushButton *browseBtn = new QPushButton("Browse", this);
  gridLayout->addWidget(browseBtn, row, 2);
  ++row;

  // initialise the value
  edit->setText(QString(str));

  // connections
  connect(browseBtn, SIGNAL(clicked()), this, slot);

  *fileEdit = edit;
}

void CWCalibInstrEdit::helperConstructCalInsFileWidgets(QGridLayout *gridLayout, int &row,
                            const char *calib, int lenCalib,
                            const char *instr, int lenInstr)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  helperConstructFileWidget(&m_fileOneEdit, gridLayout, row, calib, lenCalib,
                "Calibration File", SLOT(slotCalibOneBrowse()));

  helperConstructFileWidget(&m_fileTwoEdit, gridLayout, row, instr, lenInstr,
                "Transmission file", SLOT(slotInstrTwoBrowse()));
}

//--------------------------------------------------------

CWAllFilesEdit::CWAllFilesEdit(QWidget *parent) :
  CWCalibInstrEdit(parent)
{
}

void CWAllFilesEdit::slotInterPixelVariabilityThreeBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Inter-Pixel Variability File",
                          pref->directoryName("IntPixVar"),
                          "Inter-Pixel Variability File (*.ipv);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("IntPixVar", filename);

    m_fileThreeEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotDarkCurrentThreeBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Dark Current File",
                          pref->directoryName("Dark"),
                          "Dark Current File (*.drk);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Dark", filename);

    m_fileThreeEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotStraylightCorrectionThreeBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Stray-Light File",
                          pref->directoryName("Stray"),
                          "Stray-Light File (*.str);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Stray", filename);

    m_fileThreeEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotDetectorNonLinearityFourBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Detector Non-Linearity File",
                          pref->directoryName("DetNonLin"),
                          "Detector Non-Linearity File (*.dnl);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("DetNonLin", filename);

    m_fileFourEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotOffsetFourBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Offset File",
                          pref->directoryName("Offset"),
                          "Offset File (*.txt);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Offset", filename);

    m_fileFourEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotImagePathFiveBrowse()                                  // !!!!!
{
 QString dir = CPreferences::instance()->directoryName("ImageDir", ".");

 // modal dialog
 dir = QFileDialog::getExistingDirectory(0, "Select the root path for camera pictures if any", dir);

 if (!dir.isEmpty()) {
   CPreferences::instance()->setDirectoryName("ImageDir", dir);
   m_fileFiveEdit->setText(dir);
 }
}

void CWAllFilesEdit::helperConstructIpvDnlFileWidgets(QGridLayout *gridLayout, int &row,
                              const char *ipv, int lenIpv,
                              const char *dnl, int lenDnl)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row, ipv, lenIpv,
                "Interpixel Variability", SLOT(slotInterPixelVariabilityThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row, dnl, lenDnl,
                "Det. Non-Linearity", SLOT(slotDetectorNonLinearityFourBrowse()));
}

void CWAllFilesEdit::helperConstructFileWidgets(QGridLayout *gridLayout, int &row,
                        const char *calib, int lenCalib,
                        const char *instr, int lenInstr,
                        const char *ipv, int lenIpv,
                        const char *dnl, int lenDnl)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  helperConstructCalInsFileWidgets(gridLayout, row, calib, lenCalib, instr, lenInstr);
  helperConstructIpvDnlFileWidgets(gridLayout, row, ipv, lenIpv, dnl, lenDnl);
}

//--------------------------------------------------------

CWInstrAsciiEdit::CWInstrAsciiEdit(const struct instrumental_ascii *d, QWidget *parent) :
  CWCalibInstrEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Vertical, this) )
{
  int row;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Format and Flags group
  QHBoxLayout *groupLayout = new QHBoxLayout;

  // Format
  QGroupBox *formatGroup = new QGroupBox("Format", this);
  QVBoxLayout *formatLayout = new QVBoxLayout(formatGroup);

  m_lineRadioButton = new QRadioButton("line", formatGroup);
  formatLayout->addWidget(m_lineRadioButton);
  m_columnRadioButton = new QRadioButton("column", formatGroup);
  formatLayout->addWidget(m_columnRadioButton);
  m_columnExtendedRadioButton = new QRadioButton("column extended", formatGroup);
  formatLayout->addWidget(m_columnExtendedRadioButton);
  formatLayout->addStretch(1);

  groupLayout->addWidget(formatGroup);

  // Flags
  m_flagsGroup = new QGroupBox("Read from file", this);
  QGridLayout *flagsLayout = new QGridLayout(m_flagsGroup);

  m_zenCheck = new QCheckBox("Solar Zenith Angle", m_flagsGroup);
  flagsLayout->addWidget(m_zenCheck, 0, 0);
  m_dateCheck = new QCheckBox("DD/MM/YYYY", m_flagsGroup);
  flagsLayout->addWidget(m_dateCheck, 0, 1);

  m_aziCheck = new QCheckBox("Azimuth Viewing Angle", m_flagsGroup);
  flagsLayout->addWidget(m_aziCheck, 1, 0);
  m_timeCheck = new QCheckBox("Decimal Time", m_flagsGroup);
  flagsLayout->addWidget(m_timeCheck, 1, 1);

  m_eleCheck = new QCheckBox("Elevation Viewing Angle", m_flagsGroup);
  flagsLayout->addWidget(m_eleCheck, 2, 0);
  m_lambdaCheck = new QCheckBox("Lambda", m_flagsGroup);
  flagsLayout->addWidget(m_lambdaCheck, 2, 1);

  groupLayout->addWidget(m_flagsGroup);

  row=0;

  groupLayout->addWidget(m_strayLightConfig);
  mainLayout->addLayout(groupLayout);

  // bottom layout - det. size and files
  row = 0;
  QGridLayout *bottomLayout = new QGridLayout;

  bottomLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  bottomLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(bottomLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  bottomLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  bottomLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(bottomLayout);
  mainLayout->addStretch(1);

  // initial values
  QString tmpStr;

  // detector size

  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // format
  if (d->format == PRJCT_INSTR_ASCII_FORMAT_LINE)
    m_lineRadioButton->setChecked(true);
  else if (d->format == PRJCT_INSTR_ASCII_FORMAT_COLUMN)
    m_columnRadioButton->setChecked(true);
  else if (d->format == PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED)
    m_columnExtendedRadioButton->setChecked(true);

  // flags
  m_zenCheck->setCheckState(d->flagZenithAngle ? Qt::Checked : Qt::Unchecked);
  m_aziCheck->setCheckState(d->flagAzimuthAngle ? Qt::Checked : Qt::Unchecked);
  m_eleCheck->setCheckState(d->flagElevationAngle ? Qt::Checked : Qt::Unchecked);
  m_dateCheck->setCheckState(d->flagDate ? Qt::Checked : Qt::Unchecked);
  m_timeCheck->setCheckState(d->flagTime ? Qt::Checked : Qt::Unchecked);
  m_lambdaCheck->setCheckState(d->flagWavelength ? Qt::Checked : Qt::Unchecked);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);

  connect(m_lineRadioButton, SIGNAL(toggled(bool)), this, SLOT(slotAsciiFormatChanged(bool)));
  connect(m_columnRadioButton, SIGNAL(toggled(bool)), this, SLOT(slotAsciiFormatChanged(bool)));
  connect(m_columnExtendedRadioButton, SIGNAL(toggled(bool)), this, SLOT(slotAsciiExtendedFormatChanged(bool)));

  setflagsEnabled((m_columnExtendedRadioButton->isChecked())?false:true);
}

void CWInstrAsciiEdit::apply(struct instrumental_ascii *d) const
{
  // detected size

  d->detectorSize = m_detSizeEdit->text().toInt();

  // format
  if (m_lineRadioButton->isChecked()) d->format = PRJCT_INSTR_ASCII_FORMAT_LINE;
  if (m_columnRadioButton->isChecked()) d->format = PRJCT_INSTR_ASCII_FORMAT_COLUMN;
  if (m_columnExtendedRadioButton->isChecked()) d->format = PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED;

  // flags
  d->flagZenithAngle = m_zenCheck->isChecked() ? 1 : 0;
  d->flagAzimuthAngle = m_aziCheck->isChecked() ? 1 : 0;
  d->flagElevationAngle = m_eleCheck->isChecked() ? 1 : 0;
  d->flagDate = m_dateCheck->isChecked() ? 1 : 0;
  d->flagTime = m_timeCheck->isChecked() ? 1 : 0;
  d->flagWavelength = m_lambdaCheck->isChecked() ? 1 : 0;

  // straylight

  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

void CWInstrAsciiEdit::setflagsEnabled(bool enableFlag)
{
 if (enableFlag)
  {
   m_zenCheck->show();
   m_aziCheck->show();
   m_eleCheck->show();
   m_dateCheck->show();
   m_timeCheck->show();
   m_lambdaCheck->show();
   m_flagsGroup->show();
  }
 else
  {
   m_zenCheck->hide();
   m_aziCheck->hide();
   m_eleCheck->hide();
   m_dateCheck->hide();
   m_timeCheck->hide();
   m_lambdaCheck->hide();
   m_flagsGroup->hide();
  }
}

void CWInstrAsciiEdit::slotAsciiFormatChanged(bool state)
{
  setflagsEnabled(state);
}

void CWInstrAsciiEdit::slotAsciiExtendedFormatChanged(bool state)
{
  setflagsEnabled(!state);
}

//--------------------------------------------------------
#ifdef PRJCT_INSTR_FORMAT_OLD

CWInstrLoggerEdit::CWInstrLoggerEdit(const struct instrumental_logger *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("All", QVariant(PRJCT_INSTR_IASB_TYPE_ALL));
  m_spectralTypeCombo->addItem("Zenithal", QVariant(PRJCT_INSTR_IASB_TYPE_ZENITHAL));
  m_spectralTypeCombo->addItem("Off-Axis", QVariant(PRJCT_INSTR_IASB_TYPE_OFFAXIS));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // Azimuth
  m_aziCheck = new QCheckBox("Format with azimuth angle", this);
  gridLayout->addWidget(m_aziCheck, row, 1, 1, 2);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral type
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // flag
  m_aziCheck->setCheckState(d->flagAzimuthAngle ? Qt::Checked : Qt::Unchecked);
}

void CWInstrLoggerEdit::apply(struct instrumental_logger *d) const
{
  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // flags
  d->flagAzimuthAngle = m_aziCheck->isChecked() ? 1 : 0;

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------

CWInstrActonEdit::CWInstrActonEdit(const struct instrumental_acton *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // NILU Type
  gridLayout->addWidget(new QLabel("NILU Format Type", this), row, 0);
  m_niluTypeCombo = new QComboBox(this);
  m_niluTypeCombo->addItem("Old format", QVariant(PRJCT_INSTR_NILU_FORMAT_OLD));
  m_niluTypeCombo->addItem("New format", QVariant(PRJCT_INSTR_NILU_FORMAT_NEW));
  gridLayout->addWidget(m_niluTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // nilu type
  int index = m_niluTypeCombo->findData(QVariant(d->niluType));
  if (index != -1)
    m_niluTypeCombo->setCurrentIndex(index);

}

void CWInstrActonEdit::apply(struct instrumental_acton *d) const
{
  // nilu type
  d->niluType = m_niluTypeCombo->itemData(m_niluTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}
#endif
//--------------------------------------------------------

CWInstrSaozEdit::CWInstrSaozEdit(const struct instrumental_saoz *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("Zenithal", QVariant(PRJCT_INSTR_SAOZ_TYPE_ZENITHAL));
  m_spectralTypeCombo->addItem("Pointed", QVariant(PRJCT_INSTR_SAOZ_TYPE_POINTED));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                                   d->calibrationFile, sizeof(d->calibrationFile),
                                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral type
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

}

void CWInstrSaozEdit::apply(struct instrumental_saoz *d) const
{
  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------------------------

CWInstrMfcEdit::CWInstrMfcEdit(const struct instrumental_mfc *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Vertical, this) )
{
  QString tmpStr;
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // masks and check boxes
  QHBoxLayout *middleLayout = new QHBoxLayout;
  // check boxes

  QGroupBox *formatGroup = new QGroupBox("Format", this);
  QGridLayout *checkLayout = new QGridLayout(formatGroup);

  m_revertCheck = new QCheckBox("Revert", formatGroup);
  checkLayout->addWidget(m_revertCheck,0,0);

  checkLayout->addWidget(new QLabel("Detector Size", formatGroup), 1, 0);
  m_detSizeEdit = new QLineEdit(formatGroup);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  checkLayout->addWidget(m_detSizeEdit, 1, 1, Qt::AlignLeft);
  checkLayout->addWidget(new QLabel("1st Wavelength (nm)", formatGroup), 2, 0);
  m_firstLambdaEdit = new QLineEdit(formatGroup);
  m_firstLambdaEdit->setFixedWidth(cStandardEditWidth);
  m_firstLambdaEdit->setValidator(new QIntValidator(100, 999, m_firstLambdaEdit));
  checkLayout->addWidget(m_firstLambdaEdit, 2,1, Qt::AlignLeft);


  // mask group
  QGroupBox *maskGroup = new QGroupBox("Masks", this);
  QGridLayout *maskLayout = new QGridLayout(maskGroup);
  // offset
  maskLayout->addWidget(new QLabel("Offset", maskGroup), 0, 0);
  m_offsetMaskEdit = new QLineEdit(maskGroup);
  m_offsetMaskEdit->setFixedWidth(cStandardEditWidth);
  m_offsetMaskEdit->setValidator(new QIntValidator(0, 65535, m_offsetMaskEdit));
  maskLayout->addWidget(m_offsetMaskEdit, 0, 1);
  // instr fctn
  maskLayout->addWidget(new QLabel("Instr. ftcn", maskGroup), 0, 2);
  m_instrMaskEdit = new QLineEdit(maskGroup);
  m_instrMaskEdit->setFixedWidth(cStandardEditWidth);
  m_instrMaskEdit->setValidator(new QIntValidator(0, 65535, m_instrMaskEdit));
  maskLayout->addWidget(m_instrMaskEdit, 0, 3);
  // dark
  maskLayout->addWidget(new QLabel("Dark Current", maskGroup), 1, 0);
  m_darkMaskEdit = new QLineEdit(maskGroup);
  m_darkMaskEdit->setFixedWidth(cStandardEditWidth);
  m_darkMaskEdit->setValidator(new QIntValidator(0, 65535, m_darkMaskEdit));
  maskLayout->addWidget(m_darkMaskEdit, 1, 1);
  // spectra
  maskLayout->addWidget(new QLabel("Spectra", maskGroup), 1, 2);
  m_spectraMaskEdit = new QLineEdit(maskGroup);
  m_spectraMaskEdit->setFixedWidth(cStandardEditWidth);
  m_spectraMaskEdit->setValidator(new QIntValidator(0, 65535, m_spectraMaskEdit));
  maskLayout->addWidget(m_spectraMaskEdit, 1, 3);

  middleLayout->addWidget(formatGroup);
  middleLayout->addWidget(maskGroup);
  middleLayout->addWidget(m_strayLightConfig);
  mainLayout->addLayout(middleLayout);

  QGridLayout *gridLayout = new QGridLayout;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  // non-standard files...
  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row,
                d->darkCurrentFile, sizeof(d->darkCurrentFile),
                "Dark Current", SLOT(slotDarkCurrentThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row,
                d->offsetFile, sizeof(d->offsetFile),
                "Offset", SLOT(slotOffsetFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // first wavelength
  tmpStr.setNum(d->firstWavelength);
  m_firstLambdaEdit->validator()->fixup(tmpStr);
  m_firstLambdaEdit->setText(tmpStr);

  // check boxes
  m_revertCheck->setCheckState(d->revert ? Qt::Checked : Qt::Unchecked);
  // m_autoCheck->setCheckState(d->autoFileSelect ? Qt::Checked : Qt::Unchecked);

  // masks
  tmpStr.setNum(d->offsetMask);
  m_offsetMaskEdit->validator()->fixup(tmpStr);
  m_offsetMaskEdit->setText(tmpStr);

  tmpStr.setNum(d->instrFctnMask);
  m_instrMaskEdit->validator()->fixup(tmpStr);
  m_instrMaskEdit->setText(tmpStr);

  tmpStr.setNum(d->darkCurrentMask);
  m_darkMaskEdit->validator()->fixup(tmpStr);
  m_darkMaskEdit->setText(tmpStr);

  tmpStr.setNum(d->spectraMask);
  m_spectraMaskEdit->validator()->fixup(tmpStr);
  m_spectraMaskEdit->setText(tmpStr);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrMfcEdit::apply(struct instrumental_mfc *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // first wavlength
  d->firstWavelength = m_firstLambdaEdit->text().toInt();

  // checkboxes
  d->revert = (m_revertCheck->checkState() == Qt::Checked) ? 1 : 0;
  // d->autoFileSelect = (m_autoCheck->checkState() == Qt::Checked) ? 1 : 0;

  // masks
  d->offsetMask = m_offsetMaskEdit->text().toUInt();
  d->instrFctnMask = m_instrMaskEdit->text().toUInt();
  d->darkCurrentMask = m_darkMaskEdit->text().toUInt();
  d->spectraMask = m_spectraMaskEdit->text().toUInt();

  // straylight

  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  strcpy(d->darkCurrentFile, m_fileThreeEdit->text().toLocal8Bit().data());
  strcpy(d->offsetFile, m_fileFourEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------------------------

CWInstrMfcStdEdit::CWInstrMfcStdEdit(const struct instrumental_mfcstd *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Vertical, this) )
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *groupLayout = new QHBoxLayout;
  QString tmpStr;



  QGridLayout *gridLayout = new QGridLayout;

  QGroupBox *formatGroup = new QGroupBox("Format", this);
  QGridLayout *formatLayout = new QGridLayout(formatGroup);

  int row = 0;

  m_revertCheck = new QCheckBox("Revert spectra", formatGroup);                        // revert spectra check box
  formatLayout->addWidget(m_revertCheck, row, 0);

  ++row;

  formatLayout->addWidget(new QLabel("Detector Size", formatGroup), row, 0);             // detector size label
  m_detSizeEdit = new QLineEdit(formatGroup);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  formatLayout->addWidget(m_detSizeEdit, row, 1, Qt::AlignLeft);

  ++row;

  formatLayout->addWidget(new QLabel("Date Format", formatGroup), row, 0);               // date format label
  m_dateFormatEdit = new QLineEdit(formatGroup);
  m_dateFormatEdit->setMaxLength(sizeof(d->dateFormat)-1);
  formatLayout->addWidget(m_dateFormatEdit, row, 1, Qt::AlignLeft);

  groupLayout->addWidget(formatGroup);

  groupLayout->addWidget(m_strayLightConfig);
  mainLayout->addLayout(groupLayout);

  row=0;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  // non-standard files...
  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row,
                d->darkCurrentFile, sizeof(d->darkCurrentFile),
                "Dark Current", SLOT(slotDarkCurrentThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row,
                d->offsetFile, sizeof(d->offsetFile),
                "Offset", SLOT(slotOffsetFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  m_dateFormatEdit->setText(QString(d->dateFormat));

  // revert
  m_revertCheck->setCheckState(d->revert ? Qt::Checked : Qt::Unchecked);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrMfcStdEdit::apply(struct instrumental_mfcstd *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // revert
  d->revert = (m_revertCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();

  strcpy(d->dateFormat,m_dateFormatEdit->text().toLocal8Bit().data());

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  strcpy(d->darkCurrentFile, m_fileThreeEdit->text().toLocal8Bit().data());
  strcpy(d->offsetFile, m_fileFourEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------------------------

CWInstrMfcbiraEdit::CWInstrMfcbiraEdit(const struct instrumental_mfcbira *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Horizontal, this))
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  //  QHBoxLayout *groupLayout = new QHBoxLayout;
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addWidget(m_strayLightConfig);

  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0, Qt::AlignRight);             // detector size label
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1, Qt::AlignLeft);

  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrMfcbiraEdit::apply(struct instrumental_mfcbira *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());

  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();
}

//--------------------------------------------------------------------------

CWInstrFrm4doasEdit::CWInstrFrm4doasEdit(const struct instrumental_frm4doas *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Horizontal, this))
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *groupLayout = new QHBoxLayout;
  QGridLayout *gridLayout = new QGridLayout;

  m_imagersGroup=new QGroupBox("Imagers", this);
  m_imagersGroup->setCheckable(false);
  
  QHBoxLayout *imagersLayout=new QHBoxLayout(m_imagersGroup);
  m_imagersAverageCheck=new QCheckBox("Average rows", m_imagersGroup);
  imagersLayout->addWidget(m_imagersAverageCheck);
  
  groupLayout->addWidget(m_strayLightConfig);
  groupLayout->addWidget(m_imagersGroup);
  groupLayout->addStretch(1);
  
  mainLayout->addLayout(groupLayout);
  
  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0, Qt::AlignRight);             // detector size label
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1, Qt::AlignLeft);
  
  ++row;
  
  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("All", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_NONE));
  m_spectralTypeCombo->addItem("Zenith only", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_ZENITH));
  m_spectralTypeCombo->addItem("Off-axis", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS));
  m_spectralTypeCombo->addItem("Direct sun", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN));
  m_spectralTypeCombo->addItem("Almucantar", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  m_imagersAverageCheck->setChecked(d->averageRows);

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // spectral type

  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrFrm4doasEdit::apply(struct instrumental_frm4doas *d) const
{
  d->averageRows=m_imagersAverageCheck->isChecked() ? 1 : 0;

  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());

  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();
}

//--------------------------------------------------------------------------

CWInstrAvantesEdit::CWInstrAvantesEdit(const struct instrumental_avantes *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Horizontal, this))
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  //  QHBoxLayout *groupLayout = new QHBoxLayout;
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addWidget(m_strayLightConfig);

  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0, Qt::AlignRight);             // detector size label
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1, Qt::AlignLeft);

  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrAvantesEdit::apply(struct instrumental_avantes *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());

  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();
}

//--------------------------------------------------------

CWInstrMinimumEdit::CWInstrMinimumEdit(const struct instrumental_minimum *d, QWidget *parent) :
  CWCalibInstrEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Horizontal, this))
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addWidget(m_strayLightConfig);

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrMinimumEdit::apply(struct instrumental_minimum *d) const
{
  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());

  // straylight bias
  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();
}

//--------------------------------------------------------
#ifdef PRJCT_INSTR_FORMAT_OLD
CWInstrCcdEdit::CWInstrCcdEdit(const struct instrumental_ccd *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // files
  helperConstructFileWidgets(gridLayout, row,
                 d->calibrationFile, sizeof(d->calibrationFile),
                 d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile),
                 d->interPixelVariabilityFile, sizeof(d->interPixelVariabilityFile),
                 d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

void CWInstrCcdEdit::apply(struct instrumental_ccd *d) const
{
  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  strcpy(d->interPixelVariabilityFile, m_fileThreeEdit->text().toLocal8Bit().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toLocal8Bit().data());
}
#endif
//--------------------------------------------------------

CWInstrCcdEevEdit::CWInstrCcdEevEdit(const struct instrumental_ccdeev *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Horizontal, this))
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addWidget(m_strayLightConfig);

  // detector size
  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("All", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_NONE));
  m_spectralTypeCombo->addItem("Zenith only", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_ZENITH));
  m_spectralTypeCombo->addItem("Off-axis", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS));
  m_spectralTypeCombo->addItem("Direct sun", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN));
  m_spectralTypeCombo->addItem("Almucantar", QVariant(PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  // non-standard files...

  helperConstructFileWidget(&m_fileFiveEdit, gridLayout, row,                   // !!!!
                d->imagePath, sizeof(d->imagePath),
                "Camera pictures", SLOT(slotImagePathFiveBrowse()));

  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row,
                d->straylightCorrectionFile, sizeof(d->straylightCorrectionFile),
                "Stray-Light Correction", SLOT(slotStraylightCorrectionThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row,
                d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile),
                "Det. Non-Linearity", SLOT(slotDetectorNonLinearityFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // spectral type

  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrCcdEevEdit::apply(struct instrumental_ccdeev *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  strcpy(d->straylightCorrectionFile, m_fileThreeEdit->text().toLocal8Bit().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toLocal8Bit().data());
  strcpy(d->imagePath, m_fileFiveEdit->text().toLocal8Bit().data());

  // straylight bias
  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();
}

//--------------------------------------------------------

CWInstrGdpEdit::CWInstrGdpEdit(const struct instrumental_gdp *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // band
  gridLayout->addWidget(new QLabel("Band Type", this), row, 0);
  m_bandTypeCombo = new QComboBox(this);
  m_bandTypeCombo->addItem("Band 1a", QVariant(PRJCT_INSTR_GDP_BAND_1A));
  m_bandTypeCombo->addItem("Band 1b", QVariant(PRJCT_INSTR_GDP_BAND_1B));
  m_bandTypeCombo->addItem("Band 2a", QVariant(PRJCT_INSTR_GDP_BAND_2A));
  m_bandTypeCombo->addItem("Band 2b", QVariant(PRJCT_INSTR_GDP_BAND_2B));
  m_bandTypeCombo->addItem("Band 3", QVariant(PRJCT_INSTR_GDP_BAND_3));
  m_bandTypeCombo->addItem("Band 4", QVariant(PRJCT_INSTR_GDP_BAND_4));
  gridLayout->addWidget(m_bandTypeCombo, row, 1);
  ++row;

  // pixel type

  gridLayout->addWidget(new QLabel("Pixel Type", this), row, 0);
  m_pixelTypeCombo = new QComboBox(this);
  m_pixelTypeCombo->addItem("All", QVariant(PRJCT_INSTR_GDP_PIXEL_ALL));
  m_pixelTypeCombo->addItem("East", QVariant(PRJCT_INSTR_GDP_PIXEL_EAST));
  m_pixelTypeCombo->addItem("Center", QVariant(PRJCT_INSTR_GDP_PIXEL_CENTER));
  m_pixelTypeCombo->addItem("West", QVariant(PRJCT_INSTR_GDP_PIXEL_WEST));
  m_pixelTypeCombo->addItem("Backscan", QVariant(PRJCT_INSTR_GDP_PIXEL_BACKSCAN));
  gridLayout->addWidget(m_pixelTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // band
  int index = m_bandTypeCombo->findData(QVariant(d->bandType));
  if (index != -1)
    m_bandTypeCombo->setCurrentIndex(index);

  // pixel type
  index = m_pixelTypeCombo->findData(QVariant(d->pixelType));
  if (index != -1)
    m_pixelTypeCombo->setCurrentIndex(index);
}

void CWInstrGdpEdit::apply(struct instrumental_gdp *d) const
{
  // band
  d->bandType = m_bandTypeCombo->itemData(m_bandTypeCombo->currentIndex()).toInt();

  // pixel type
  d->pixelType = m_pixelTypeCombo->itemData(m_pixelTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------

CWInstrGome1Edit::CWInstrGome1Edit(const struct instrumental_gdp *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // band
  gridLayout->addWidget(new QLabel("Band Type", this), row, 0);
  m_bandTypeCombo = new QComboBox(this);
  m_bandTypeCombo->addItem("Band 1a", QVariant(PRJCT_INSTR_GDP_BAND_1A));
  m_bandTypeCombo->addItem("Band 1b", QVariant(PRJCT_INSTR_GDP_BAND_1B));
  m_bandTypeCombo->addItem("Band 2a", QVariant(PRJCT_INSTR_GDP_BAND_2A));
  m_bandTypeCombo->addItem("Band 2b", QVariant(PRJCT_INSTR_GDP_BAND_2B));
  m_bandTypeCombo->addItem("Band 3", QVariant(PRJCT_INSTR_GDP_BAND_3));
  m_bandTypeCombo->addItem("Band 4", QVariant(PRJCT_INSTR_GDP_BAND_4));
  gridLayout->addWidget(m_bandTypeCombo, row, 1);
  ++row;

  // pixel type

  gridLayout->addWidget(new QLabel("Pixel Type", this), row, 0);
  m_pixelTypeCombo = new QComboBox(this);
  m_pixelTypeCombo->addItem("All", QVariant(PRJCT_INSTR_GOME1_PIXEL_ALL));
  m_pixelTypeCombo->addItem("Ground pixels only", QVariant(PRJCT_INSTR_GOME1_PIXEL_GROUND));
  m_pixelTypeCombo->addItem("Backscans only", QVariant(PRJCT_INSTR_GOME1_PIXEL_BACKSCAN));
  gridLayout->addWidget(m_pixelTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // band
  int index = m_bandTypeCombo->findData(QVariant(d->bandType));
  if (index != -1)
    m_bandTypeCombo->setCurrentIndex(index);

  // pixel type
  index = m_pixelTypeCombo->findData(QVariant(d->pixelType));
  if (index != -1)
    m_pixelTypeCombo->setCurrentIndex(index);
}

void CWInstrGome1Edit::apply(struct instrumental_gdp *d) const
{
  // band
  d->bandType = m_bandTypeCombo->itemData(m_bandTypeCombo->currentIndex()).toInt();

  // pixel type
  d->pixelType = m_pixelTypeCombo->itemData(m_pixelTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------------------------

CWInstrGome2Edit::CWInstrGome2Edit(const struct instrumental_gome2 *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // band
  gridLayout->addWidget(new QLabel("Band Type", this), row, 0);
  m_bandTypeCombo = new QComboBox(this);
  m_bandTypeCombo->addItem("Band 1a", QVariant(PRJCT_INSTR_GDP_BAND_1A));
  m_bandTypeCombo->addItem("Band 1b", QVariant(PRJCT_INSTR_GDP_BAND_1B));
  m_bandTypeCombo->addItem("Band 2a", QVariant(PRJCT_INSTR_GDP_BAND_2A));
  m_bandTypeCombo->addItem("Band 2b", QVariant(PRJCT_INSTR_GDP_BAND_2B));
  m_bandTypeCombo->addItem("Band 3", QVariant(PRJCT_INSTR_GDP_BAND_3));
  m_bandTypeCombo->addItem("Band 4", QVariant(PRJCT_INSTR_GDP_BAND_4));
  gridLayout->addWidget(m_bandTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // band
  int index = m_bandTypeCombo->findData(QVariant(d->bandType));
  if (index != -1)
    m_bandTypeCombo->setCurrentIndex(index);
}

void CWInstrGome2Edit::apply(struct instrumental_gome2 *d) const
{
  // band
  d->bandType = m_bandTypeCombo->itemData(m_bandTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------------------------

CWInstrSciaEdit::CWInstrSciaEdit(const struct instrumental_scia *d, QWidget *parent) :
  CWAllFilesEdit(parent),
  m_clusterOffset(0)
{
  QString tmpStr;
  int i;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // channel
  gridLayout->addWidget(new QLabel("Channel", this), row, 0);
  m_channelCombo = new QComboBox(this);
  m_channelCombo->addItem("Channel 1", QVariant(PRJCT_INSTR_SCIA_CHANNEL_1));
  m_channelCombo->addItem("Channel 2", QVariant(PRJCT_INSTR_SCIA_CHANNEL_2));
  m_channelCombo->addItem("Channel 3", QVariant(PRJCT_INSTR_SCIA_CHANNEL_3));
  m_channelCombo->addItem("Channel 4", QVariant(PRJCT_INSTR_SCIA_CHANNEL_4));
  gridLayout->addWidget(m_channelCombo, row, 1);
  ++row;

  // clusters
  gridLayout->addWidget(new QLabel("Clusters", this), row, 0);
  QHBoxLayout *checkLayout = new QHBoxLayout;
  for (i=0; i<6; ++i) {
    m_clusterCheck[i] = new QCheckBox(this);
    //m_clusterCheck[i]->setFixedWidth(40);
    checkLayout->addWidget(m_clusterCheck[i]);
  }
  checkLayout->addStretch(1);
  gridLayout->addLayout(checkLayout, row, 1, 1, 2, Qt::AlignLeft);
  ++row;

  // sun reference
  gridLayout->addWidget(new QLabel("Reference", this), row, 0);
  m_referenceEdit = new QLineEdit(this);
  m_referenceEdit->setFixedWidth(50);
  m_referenceEdit->setMaxLength(2);
  gridLayout->addWidget(m_referenceEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row, d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile),
                "Det. Non-Linearity", SLOT(slotDetectorNonLinearityFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // cluster state
  memcpy(m_clusterState, d->clusters, sizeof(m_clusterState));

  // channel
  int index = m_channelCombo->findData(QVariant(d->channel));
  if (index != -1) {
    m_channelCombo->setCurrentIndex(index);
    slotChannelChanged(index);
  }

  m_referenceEdit->setText(QString(d->sunReference));

  // connections
  connect(m_channelCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChannelChanged(int)));

  connect(m_clusterCheck[0], SIGNAL(stateChanged(int)), this, SLOT(slotCluster0Changed(int)));
  connect(m_clusterCheck[1], SIGNAL(stateChanged(int)), this, SLOT(slotCluster1Changed(int)));
  connect(m_clusterCheck[2], SIGNAL(stateChanged(int)), this, SLOT(slotCluster2Changed(int)));
  connect(m_clusterCheck[3], SIGNAL(stateChanged(int)), this, SLOT(slotCluster3Changed(int)));
  connect(m_clusterCheck[4], SIGNAL(stateChanged(int)), this, SLOT(slotCluster4Changed(int)));
  connect(m_clusterCheck[5], SIGNAL(stateChanged(int)), this, SLOT(slotCluster5Changed(int)));
}

void CWInstrSciaEdit::apply(struct instrumental_scia *d) const
{
  // channel
  d->channel = m_channelCombo->itemData(m_channelCombo->currentIndex()).toInt();

  // cluster state
  memcpy(d->clusters, m_clusterState, sizeof(d->clusters));

  // sun reference
  strcpy(d->sunReference, m_referenceEdit->text().toLocal8Bit().data());

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toLocal8Bit().data());
}

void CWInstrSciaEdit::slotChannelChanged(int index)
{
  int i;
  QString str;

  // the index defines the clusters that should be displyed
  switch (index) {
  case 0:
    m_clusterOffset = 2;
    for (i=0; i<4; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    for (i=4; i<6; ++i)
      m_clusterCheck[i]->hide();

    break;
  case 1:
    m_clusterOffset = 8;
    for (i=0; i<3; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    for (i=3; i<6; ++i)
      m_clusterCheck[i]->hide();

    break;
  case 2:
    m_clusterOffset = 13;
    for (i=0; i<6; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    break;
  case 3:
    m_clusterOffset = 22;
    for (i=0; i<6; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    break;
  }
}

void CWInstrSciaEdit::slotCluster0Changed(int state)
{
  m_clusterState[m_clusterOffset] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster1Changed(int state)
{
  m_clusterState[m_clusterOffset+1] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster2Changed(int state)
{
  m_clusterState[m_clusterOffset+2] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster3Changed(int state)
{
  m_clusterState[m_clusterOffset+3] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster4Changed(int state)
{
  m_clusterState[m_clusterOffset+4] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster5Changed(int state)
{
  m_clusterState[m_clusterOffset+5] = (state == Qt::Checked) ? 1 : 0;
}

//--------------------------------------------------------

CWInstrOmiEdit::CWInstrOmiEdit(const struct instrumental_omi *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(5);

  QHBoxLayout *qualityFlagsLayout = new QHBoxLayout;
  mainLayout->addLayout(qualityFlagsLayout);

  // Pixel quality flags
  m_pixelQFGroup = new QGroupBox("Use Pixel Quality Flags", this);
  qualityFlagsLayout->addWidget(m_pixelQFGroup);
  m_pixelQFGroup->setCheckable(true);

  QGridLayout *pixelQFLayout = new QGridLayout(m_pixelQFGroup);
  pixelQFLayout->addWidget(new QLabel("Mask for pixel rejection", m_pixelQFGroup), 1, 0, Qt::AlignRight);
  m_pixelQFMaskEdit = new QLineEdit(m_pixelQFGroup);
  m_pixelQFMaskEdit->setFixedWidth(50);
  pixelQFLayout->addWidget(m_pixelQFMaskEdit, 1, 1, Qt::AlignLeft);
  pixelQFLayout->addWidget(new QLabel("Maximum number of gaps", m_pixelQFGroup), 2, 0, Qt::AlignRight);
  m_pixelQFMaxGapsEdit = new QLineEdit(m_pixelQFGroup);
  m_pixelQFMaxGapsEdit->setFixedWidth(50);
  m_pixelQFMaxGapsEdit->setValidator(new QIntValidator(0, 20, m_pixelQFMaxGapsEdit));
  pixelQFLayout->addWidget(m_pixelQFMaxGapsEdit, 2, 1, Qt::AlignLeft);


  // XTrack quality flags
  m_xtrackQFBox = new QGroupBox("Use Cross-track Quality Flags", this);
  qualityFlagsLayout->addWidget(m_xtrackQFBox);
  m_xtrackQFBox->setCheckable(true);

  QVBoxLayout *xtrackQFBoxLayout = new QVBoxLayout(m_xtrackQFBox);
  xtrackQFBoxLayout->setAlignment(Qt::AlignCenter);
  m_nonstrictXTrackQF = new QRadioButton("Exclude bad pixels");
  xtrackQFBoxLayout->addWidget(m_nonstrictXTrackQF);
  m_strictXTrackQF = new QRadioButton("Exclude all affected pixels");
  xtrackQFBoxLayout->addWidget(m_strictXTrackQF);

  QGridLayout *gridLayout = new QGridLayout;

  // spectral
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("UV-1", QVariant(PRJCT_INSTR_OMI_TYPE_UV1));
  m_spectralTypeCombo->addItem("UV-2", QVariant(PRJCT_INSTR_OMI_TYPE_UV2));
  m_spectralTypeCombo->addItem("Visible", QVariant(PRJCT_INSTR_OMI_TYPE_VIS));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // Track selection
  gridLayout->addWidget(new QLabel("Row selection", this), row, 0);
  m_trackSelection = new QLineEdit(this);
  gridLayout->addWidget(m_trackSelection, row, 1);
  ++row;

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  m_trackSelection->setText(QString(d->trackSelection));

  // pixel quality flag

  m_pixelQFGroup->setChecked(d->pixelQFRejectionFlag);

  QString tmpStr;
  char str[80];
  tmpStr.setNum(d->pixelQFMaxGaps);
  m_pixelQFMaxGapsEdit->validator()->fixup(tmpStr);
  m_pixelQFMaxGapsEdit->setText(tmpStr);

  sprintf(str,"%02X",d->pixelQFMask);
  m_pixelQFMaskEdit->setText(QString(str));

  // xtrack quality flag
  switch(d->xtrack_mode) {
  case XTRACKQF_IGNORE:
    m_xtrackQFBox->setChecked(false);
    break;
  case XTRACKQF_NONSTRICT:
    m_xtrackQFBox->setChecked(true);
    m_nonstrictXTrackQF->setChecked(true);
    break;
  case XTRACKQF_STRICT:
    m_xtrackQFBox->setChecked(true);
    m_strictXTrackQF->setChecked(true);
    break;
  }

}

void CWInstrOmiEdit::apply(struct instrumental_omi *d) const
{
  // spectral
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // track selection

  d->pixelQFRejectionFlag= m_pixelQFGroup->isChecked() ? 1 : 0;
  d->pixelQFMaxGaps=m_pixelQFMaxGapsEdit->text().toInt();

  sscanf(m_pixelQFMaskEdit->text().toLocal8Bit().data(),"%02X",&d->pixelQFMask);

  strcpy(d->trackSelection, m_trackSelection->text().toLocal8Bit().data());

  // XTrack Quality Flags:
  if(m_xtrackQFBox->isChecked() ) {
    if (m_strictXTrackQF->isChecked() )
      d->xtrack_mode = XTRACKQF_STRICT;
    else if (m_nonstrictXTrackQF->isChecked() )
      d->xtrack_mode = XTRACKQF_NONSTRICT;
  } else {
    d->xtrack_mode = XTRACKQF_IGNORE;
  }
}

//--------------------------------------------------------

CWInstrOmiV4Edit::CWInstrOmiV4Edit(const struct instrumental_omi *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(5);

  QHBoxLayout *qualityFlagsLayout = new QHBoxLayout;
  mainLayout->addLayout(qualityFlagsLayout);

  // XTrack quality flags
  QGroupBox *xtrackQFBox = new QGroupBox("Cross-track Quality Flags", this);
  qualityFlagsLayout->addWidget(xtrackQFBox);
  //m_xtrackQFBox->setCheckable(true);

  QVBoxLayout *xtrackQFBoxLayout = new QVBoxLayout(xtrackQFBox);
  xtrackQFBoxLayout->setAlignment(Qt::AlignCenter);
  m_ignoreXTrackQF = new QRadioButton("Ignore quality flags");
  xtrackQFBoxLayout->addWidget(m_ignoreXTrackQF);
  m_nonstrictXTrackQF = new QRadioButton("Exclude uncorrected affected pixels");
  xtrackQFBoxLayout->addWidget(m_nonstrictXTrackQF);
  m_strictXTrackQF = new QRadioButton("Exclude all affected pixels");
  xtrackQFBoxLayout->addWidget(m_strictXTrackQF);

  QGridLayout *gridLayout = new QGridLayout;

  int row = 0;

  // spectral
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("UV-1", QVariant(PRJCT_INSTR_OMI_TYPE_UV1));
  m_spectralTypeCombo->addItem("UV-2", QVariant(PRJCT_INSTR_OMI_TYPE_UV2));
  m_spectralTypeCombo->addItem("Visible", QVariant(PRJCT_INSTR_OMI_TYPE_VIS));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // xtrack quality flag
  switch(d->xtrack_mode) {
  case XTRACKQF_IGNORE:
    m_ignoreXTrackQF->setChecked(true);
    break;
  case XTRACKQF_NONSTRICT:
    m_nonstrictXTrackQF->setChecked(true);
    break;
  case XTRACKQF_STRICT:
    m_strictXTrackQF->setChecked(true);
    break;
  }
}

void CWInstrOmiV4Edit::apply(struct instrumental_omi *d) const
{
  // spectral
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // XTrack Quality Flags:
  if (m_ignoreXTrackQF->isChecked() ) {
    d->xtrack_mode = XTRACKQF_IGNORE;
  } else if (m_strictXTrackQF->isChecked() ) {
    d->xtrack_mode = XTRACKQF_STRICT;
  } else if (m_nonstrictXTrackQF->isChecked() ) {
    d->xtrack_mode = XTRACKQF_NONSTRICT;
  }

}

//--------------------------------------------------------


void CWInstrTropomiEdit::slot_browse_reference_directory()
{
  QString directory  = QFileDialog::getExistingDirectory(this, "Select directory for radiance reference spectra", QDir::currentPath());

  if (!directory.isEmpty()) {
    m_reference_directory_edit->setText(directory);
  }
}

CWInstrTropomiEdit::CWInstrTropomiEdit(const struct instrumental_tropomi *pInstrTropomi, QWidget *parent) : CWCalibInstrEdit(parent) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addLayout(gridLayout);

  // spectral band
  int row = 0;
  gridLayout->addWidget(new QLabel("Spectral Band", this), row, 0);
  m_spectralBandCombo = new QComboBox(this);
#define EXPAND(BAND, LABEL) m_spectralBandCombo->addItem(#BAND, QVariant(BAND));
  TROPOMI_BANDS
#undef EXPAND

  int index = m_spectralBandCombo->findData(QVariant(pInstrTropomi->spectralBand));
  if (index != -1)
    m_spectralBandCombo->setCurrentIndex(index);

  gridLayout->addWidget(m_spectralBandCombo, row, 1);

  ++row;
  helperConstructFileWidget(&m_reference_directory_edit,gridLayout,row, pInstrTropomi->reference_orbit_dir,
                            sizeof(pInstrTropomi->reference_orbit_dir), "Reference orbit directory" , SLOT(slot_browse_reference_directory()));
  ++row;

  // Track selection
  gridLayout->addWidget(new QLabel("Row selection", this), row, 0);
  m_trackSelection = new QLineEdit(this);
  gridLayout->addWidget(m_trackSelection, row, 1);
  ++row;

  helperConstructCalInsFileWidgets(gridLayout, row,
                   pInstrTropomi->calibrationFile, sizeof(pInstrTropomi->calibrationFile),
                   pInstrTropomi->instrFunctionFile, sizeof(pInstrTropomi->instrFunctionFile));

        m_trackSelection->setText(QString(pInstrTropomi->trackSelection));
}

void CWInstrTropomiEdit::apply(struct instrumental_tropomi *pInstrTropomi) const
{
  strcpy(pInstrTropomi->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(pInstrTropomi->instrFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  // spectral band
  pInstrTropomi->spectralBand = static_cast<tropomiSpectralBand>(m_spectralBandCombo->itemData(m_spectralBandCombo->currentIndex()).toInt());
  strcpy(pInstrTropomi->trackSelection, m_trackSelection->text().toLocal8Bit().data());

  // radiance reference directory
  if(m_reference_directory_edit->text().size() < sizeof(pInstrTropomi->reference_orbit_dir))
     strcpy(pInstrTropomi->reference_orbit_dir,m_reference_directory_edit->text().toLocal8Bit().data());
  else
    throw(std::runtime_error("Filename too long: " + m_reference_directory_edit->text().toStdString()));
}

//--------------------------------------------------------

CWInstrApexEdit::CWInstrApexEdit(const struct instrumental_apex *pInstrApex, QWidget *parent) : CWCalibInstrEdit(parent) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addLayout(gridLayout);

  int row = 0;

  // Track selection
  gridLayout->addWidget(new QLabel("Row selection", this), row, 0);
  m_trackSelection = new QLineEdit(this);
  gridLayout->addWidget(m_trackSelection, row, 1);
  ++row;

  helperConstructCalInsFileWidgets(gridLayout, row,
                   pInstrApex->calibrationFile, sizeof(pInstrApex->calibrationFile),
                   pInstrApex->transmissionFunctionFile, sizeof(pInstrApex->transmissionFunctionFile));

        m_trackSelection->setText(QString(pInstrApex->trackSelection));
}

void CWInstrApexEdit::apply(struct instrumental_apex *pInstrApex) const
{
  strcpy(pInstrApex->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(pInstrApex->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());

  // Track selection

  strcpy(pInstrApex->trackSelection, m_trackSelection->text().toLocal8Bit().data());
}

//--------------------------------------------------------

CWInstrOmpsEdit::CWInstrOmpsEdit(QWidget *parent) :
  CWCalibInstrEdit(parent) { }

void CWInstrOmpsEdit::apply(void) { }


//--------------------------------------------------------

CWInstrOceanOpticsEdit::CWInstrOceanOpticsEdit(const struct instrumental_oceanoptics *d, QWidget *parent) :
  CWCalibInstrEdit(parent),
  m_strayLightConfig(new StrayLightConfig(Qt::Horizontal, this) )
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  int row = 0;
  QGridLayout *bottomLayout = new QGridLayout;

  mainLayout->addWidget(m_strayLightConfig);

  bottomLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  bottomLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(bottomLayout, row,
                   d->calibrationFile, sizeof(d->calibrationFile),
                   d->transmissionFunctionFile, sizeof(d->transmissionFunctionFile));

  bottomLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  bottomLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(bottomLayout);
  mainLayout->addStretch(1);

  // initial values
  QString tmpStr;

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // straylight bias
  m_strayLightConfig->setChecked(d->straylight ? true : false);
  m_strayLightConfig->setLambdaMin(d->lambdaMin);
  m_strayLightConfig->setLambdaMax(d->lambdaMax);
}

void CWInstrOceanOpticsEdit::apply(struct instrumental_oceanoptics *d) const
{
  // straylight bias
  d->straylight = m_strayLightConfig->isChecked() ? 1 : 0;
  d->lambdaMin = m_strayLightConfig->getLambdaMin();
  d->lambdaMax = m_strayLightConfig->getLambdaMax();

  // detected size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(d->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------

CWInstrGemsEdit::CWInstrGemsEdit(const struct instrumental_gems *pInstrGems, QWidget *parent) : CWCalibInstrEdit(parent) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  mainLayout->addLayout(gridLayout);

  int row = 0;

//   gridLayout->addWidget(new QLabel("Binning", this), row, 0);
//   m_binning = new QSpinBox(this);
//   m_binning->setRange(1, 2048);
//   m_binning->setFixedWidth(cStandardEditWidth);
//   gridLayout->addWidget(m_binning, row, 1);
//
//   ++row;

  // Track selection
  gridLayout->addWidget(new QLabel("Row selection", this), row, 0);
  m_trackSelection = new QLineEdit(this);
  gridLayout->addWidget(m_trackSelection, row, 1);
  ++row;

  helperConstructCalInsFileWidgets(gridLayout, row,
  pInstrGems->calibrationFile, sizeof(pInstrGems->calibrationFile),
  pInstrGems->transmissionFunctionFile, sizeof(pInstrGems->transmissionFunctionFile));

  m_trackSelection->setText(QString(pInstrGems->trackSelection));
}

void CWInstrGemsEdit::apply(struct instrumental_gems *pInstrGems) const
{
  strcpy(pInstrGems->calibrationFile, m_fileOneEdit->text().toLocal8Bit().data());
  strcpy(pInstrGems->transmissionFunctionFile, m_fileTwoEdit->text().toLocal8Bit().data());
  strcpy(pInstrGems->trackSelection, m_trackSelection->text().toLocal8Bit().data());

//  pInstrGems->binning = m_binning->value();
}

//--------------------------------------------------------
