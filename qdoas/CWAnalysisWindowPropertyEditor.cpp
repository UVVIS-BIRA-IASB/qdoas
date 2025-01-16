/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QIntValidator>
#include <QFileDialog>

#include "CWorkSpace.h"
#include "CWAnalysisWindowPropertyEditor.h"
#include "CPreferences.h"
#include "CValidator.h"

#include "PolynomialTab.h"

#include "CHelpSystem.h"

#include "constants.h"
#include "doas.h"
#include "debugutil.h"

CWAnalysisWindowPropertyEditor::CWAnalysisWindowPropertyEditor(const QString &projectName,
                                   const QString &analysisWindowName,
                                   QWidget *parent) :
  CWEditor(parent),
  CProjectObserver(),
  m_projectName(projectName),
  m_analysisWindowName(analysisWindowName),
  m_autoSelection(false),
  m_scanSelection(false)
{
  const int cDoubleEditWidth = 70;

  mediate_analysis_window_t *d = CWorkSpace::instance()->findAnalysisWindow(m_projectName.toStdString(), m_analysisWindowName.toStdString());
  mediate_project_t *p = CWorkSpace::instance()->findProject(m_projectName.toStdString());

  assert(d != NULL);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(5);

  QHBoxLayout *topLayout = new QHBoxLayout;

  // calibration combo ... group
  QGroupBox *calibrationGroup = new QGroupBox("Wavelength Calibration", this);
  QVBoxLayout *calibrationLayout = new QVBoxLayout(calibrationGroup);
  calibrationLayout->setSpacing(0);
  m_calibrationCombo = new QComboBox(calibrationGroup);
  m_calibrationCombo->addItem("None", QVariant(ANLYS_KURUCZ_NONE));
  m_calibrationCombo->addItem("Ref Only", QVariant(ANLYS_KURUCZ_REF));
  m_calibrationCombo->addItem("Spectra Only", QVariant(ANLYS_KURUCZ_SPEC));
  m_calibrationCombo->addItem("Ref + Spectra", QVariant(ANLYS_KURUCZ_REF_AND_SPEC));
  calibrationLayout->addWidget(m_calibrationCombo);
  calibrationLayout->addStretch(1);

  topLayout->addWidget(calibrationGroup);

  // radio button - ref selection
  QGroupBox *refSelectGroup = new QGroupBox("Ref. Selection", this);
  QVBoxLayout *refSelectLayout = new QVBoxLayout(refSelectGroup);
  refSelectLayout->setSpacing(0);
  QRadioButton *autoButton = new QRadioButton("Automatic",  refSelectGroup);
  refSelectLayout->addWidget(autoButton);
  QRadioButton *fileButton = new QRadioButton("File",  refSelectGroup);
  refSelectLayout->addWidget(fileButton);

  topLayout->addWidget(refSelectGroup);

  // fitting interval
  QGroupBox *fitIntervalGroup = new QGroupBox("Fitting Interval", this);
  QGridLayout *fitIntervalLayout = new QGridLayout(fitIntervalGroup);
  fitIntervalLayout->setSpacing(0);
  fitIntervalLayout->addWidget(new QLabel("Min ", fitIntervalGroup), 0, 0);
  m_fitMinEdit = new QLineEdit(fitIntervalGroup);
  m_fitMinEdit->setFixedWidth(cDoubleEditWidth);
  m_fitMinEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 2000.0, 3, m_fitMinEdit));
  fitIntervalLayout->addWidget(m_fitMinEdit, 0, 1);
  fitIntervalLayout->addWidget(new QLabel("Max ", fitIntervalGroup), 1, 0);
  m_fitMaxEdit = new QLineEdit(fitIntervalGroup);
  m_fitMaxEdit->setFixedWidth(cDoubleEditWidth);
  m_fitMaxEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 2000.0, 3, m_fitMaxEdit));
  fitIntervalLayout->addWidget(m_fitMaxEdit, 1, 1);

  topLayout->addWidget(fitIntervalGroup);

  // display
  QGroupBox *displayGroup = new QGroupBox("Display", this);
  QGridLayout *displayLayout = new QGridLayout(displayGroup);
  displayLayout->setSpacing(0);
  m_spectrumCheck = new QCheckBox("Spectrum and reference", displayGroup);
  displayLayout->addWidget(m_spectrumCheck, 0, 0);
  m_polyCheck = new QCheckBox("Polynomial", displayGroup);
  displayLayout->addWidget(m_polyCheck, 0, 1);
  m_ratioCheck = new QCheckBox("Ref1/Ref2 Alignment", displayGroup);
  displayLayout->addWidget(m_ratioCheck, 1, 0);
  m_residualCheck = new QCheckBox("Residual", displayGroup);
  displayLayout->addWidget(m_residualCheck, 1, 1);
  m_predefCheck = new QCheckBox("Predefined parameters", displayGroup);
  displayLayout->addWidget(m_predefCheck, 2, 0);
  m_fitsCheck = new QCheckBox("Fits", displayGroup);
  displayLayout->addWidget(m_fitsCheck, 2, 1);

  topLayout->addWidget(displayGroup);

  // Other options

  QGroupBox *optionsGroup = new QGroupBox("Other options", this);
  QGridLayout *optionsLayout=new QGridLayout(optionsGroup);
  optionsLayout->setSpacing(0);
  optionsLayout->addWidget(new QLabel("Ref. Resol FWHM (nm) ", optionsGroup), 0, 0);
  m_resolEdit = new QLineEdit(optionsGroup);
  m_resolEdit->setFixedWidth(cDoubleEditWidth);
  m_resolEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 999.0, 3, m_resolEdit));
  optionsLayout->addWidget(m_resolEdit, 0, 1);
  optionsLayout->addWidget(new QLabel("Lambda0", optionsGroup), 1, 0);
  m_lambda0Edit = new QLineEdit(optionsGroup);
  m_lambda0Edit->setFixedWidth(cDoubleEditWidth);
  m_lambda0Edit->setValidator(new CDoubleFixedFmtValidator(0.0, 999.0, 3, m_lambda0Edit));
  optionsLayout->addWidget(m_lambda0Edit, 1, 1);

  topLayout->addWidget(optionsGroup);

  topLayout->addStretch(1);

  mainLayout->addLayout(topLayout);

  // files
  QGroupBox *filesGroup = new QGroupBox("Files", this);
  QVBoxLayout *filesLayout = new QVBoxLayout(filesGroup);
  filesLayout->setSpacing(0);

  // row 0 - Ref1
  m_refOneFrame = new QFrame(filesGroup);
  m_refOneFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *refOneFrameLayout = new QHBoxLayout(m_refOneFrame);
  QLabel *labelOne = new QLabel(" Reference 1 ", m_refOneFrame);
  labelOne->setFixedWidth(85);
  m_refOneEdit = new QLineEdit(m_refOneFrame);
  m_refOneEdit->setMaxLength(sizeof(d->refOneFile)-1);
  QPushButton *refOneBrowseBtn = new QPushButton("Browse", m_refOneFrame);
  refOneBrowseBtn->setFixedWidth(70);
  refOneFrameLayout->addWidget(labelOne);
  refOneFrameLayout->addWidget(m_refOneEdit, 1);
  refOneFrameLayout->addWidget(refOneBrowseBtn);

  filesLayout->addWidget(m_refOneFrame);

  // frame for MAXDOAS measurements

  m_maxdoasFrame=new QFrame(filesGroup);
  m_maxdoasFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *maxdoasLayout = new QHBoxLayout(m_maxdoasFrame);
  QLabel *labelMaxdoas = new QLabel(" Reference 2 ", m_maxdoasFrame);
  labelMaxdoas->setFixedWidth(85);

  m_maxdoasSzaFrame=new QFrame(m_maxdoasFrame);
  m_maxdoasSzaFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *maxdoasSzaLayout = new QHBoxLayout(m_maxdoasSzaFrame);

  m_maxdoasSzaCenterEdit = new QLineEdit(m_maxdoasSzaFrame);
  m_maxdoasSzaCenterEdit->setFixedWidth(cDoubleEditWidth);
  m_maxdoasSzaCenterEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_maxdoasSzaCenterEdit));
  m_maxdoasSzaDeltaEdit = new QLineEdit(m_maxdoasSzaFrame);
  m_maxdoasSzaDeltaEdit->setFixedWidth(cDoubleEditWidth);
  m_maxdoasSzaDeltaEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_maxdoasSzaDeltaEdit));

  maxdoasSzaLayout->addWidget(m_maxdoasSzaCenterEdit);
  maxdoasSzaLayout->addWidget(new QLabel("+/-", m_maxdoasSzaFrame));
  maxdoasSzaLayout->addWidget(m_maxdoasSzaDeltaEdit);

  // Option for scan

  m_maxdoasScanFrame=new QFrame(m_maxdoasFrame);
  m_maxdoasScanFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *maxdoasScanLayout = new QHBoxLayout(m_maxdoasScanFrame);

  m_scanCombo = new QComboBox(m_maxdoasScanFrame);
  m_scanCombo->addItem("Reference before", QVariant(ANLYS_MAXDOAS_REF_SCAN_BEFORE));
  m_scanCombo->addItem("Reference after", QVariant(ANLYS_MAXDOAS_REF_SCAN_AFTER));
  m_scanCombo->addItem("Average references after and before", QVariant(ANLYS_MAXDOAS_REF_SCAN_AVERAGE));
  m_scanCombo->addItem("Interpolate references after and before", QVariant(ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE));
  maxdoasScanLayout->addWidget(m_scanCombo);
  maxdoasScanLayout->addStretch(1);

  QRadioButton *refScan = new QRadioButton("Scans",  m_maxdoasFrame);
  QRadioButton *refSza = new QRadioButton("SZA",  m_maxdoasFrame);

  refScan->setFixedWidth(60);
  refSza->setFixedWidth(60);

  maxdoasLayout->addWidget(labelMaxdoas);
  maxdoasLayout->addWidget(refScan);
  maxdoasLayout->addWidget(refSza);
  maxdoasLayout->addWidget(m_maxdoasSzaFrame);
  maxdoasLayout->addWidget(m_maxdoasScanFrame);

  maxdoasLayout->addSpacing(10);
  maxdoasLayout->addStretch(1);

  int maxdoasRefScan = d->refMaxdoasSelection;

  if (maxdoasRefScan==ANLYS_MAXDOAS_REF_SZA)
    refSza->setChecked(true);
  else
    refScan->setChecked(true);
  slotMaxdoasSelectionChanged(maxdoasRefScan);

  // row 1 - Ref2
  // Option for SZA

  m_refTwoSzaFrame = new QFrame(filesGroup);
  m_refTwoSzaFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *szaLayout = new QHBoxLayout(m_refTwoSzaFrame);
  QLabel *labelTwoSza = new QLabel(" Reference 2 ", m_refTwoSzaFrame);
  labelTwoSza->setFixedWidth(85);
  m_szaCenterEdit = new QLineEdit(m_refTwoSzaFrame);
  m_szaCenterEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 1, m_szaCenterEdit));
  m_szaCenterEdit->setAlignment(Qt::AlignRight);
  m_szaDeltaEdit = new QLineEdit(m_refTwoSzaFrame);
  const int sza_width = m_szaCenterEdit->fontMetrics().horizontalAdvance("0090.0");
  m_szaCenterEdit->setFixedWidth(sza_width);
  m_szaDeltaEdit->setFixedWidth(sza_width);
  m_szaDeltaEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 1, m_szaDeltaEdit));
  m_szaDeltaEdit->setAlignment(Qt::AlignRight);
  szaLayout->addWidget(labelTwoSza);
  szaLayout->addWidget(new QLabel("SZA ", m_refTwoSzaFrame));
  szaLayout->addWidget(m_szaCenterEdit);
  szaLayout->addWidget(new QLabel("+/-", m_refTwoSzaFrame));
  szaLayout->addWidget(m_szaDeltaEdit);
  szaLayout->addSpacing(10);

  // frame for satellite selection...
  m_satelliteFrame = new QFrame(m_refTwoSzaFrame);
  m_satelliteFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *satelliteLayout = new QHBoxLayout(m_satelliteFrame);

  // longitude (min/max)
  satelliteLayout->addWidget(new QLabel("Lon.", m_satelliteFrame));
  m_refTwoLonMinEdit = new QLineEdit(m_satelliteFrame);
  m_refTwoLonMinEdit->setValidator(new CDoubleFixedFmtValidator(-180, 360.0, 1, m_refTwoLonMinEdit));
  m_refTwoLonMinEdit->setAlignment(Qt::AlignRight);
  const int lon_width = m_refTwoLonMinEdit->fontMetrics().horizontalAdvance("-00360.0");
  m_refTwoLonMinEdit->setFixedWidth(lon_width);
  satelliteLayout->addWidget(m_refTwoLonMinEdit);
  m_refTwoLonMaxEdit = new QLineEdit(m_satelliteFrame);
  m_refTwoLonMaxEdit->setValidator(new CDoubleFixedFmtValidator(-180.0, 360.0, 1, m_refTwoLonMaxEdit));
  m_refTwoLonMaxEdit->setAlignment(Qt::AlignRight);
  m_refTwoLonMaxEdit->setFixedWidth(lon_width);
  satelliteLayout->addWidget(m_refTwoLonMaxEdit);
  // latitude (min/max)
  satelliteLayout->addWidget(new QLabel("Lat.", m_satelliteFrame));
  m_refTwoLatMinEdit = new QLineEdit(m_satelliteFrame);
  const int lat_width = m_refTwoLatMinEdit->fontMetrics().horizontalAdvance("0090.0");
  m_refTwoLatMinEdit->setFixedWidth(lat_width);
  m_refTwoLatMinEdit->setValidator(new CDoubleFixedFmtValidator(-90.0, 90.0, 1, m_refTwoLatMinEdit));
  m_refTwoLatMinEdit->setAlignment(Qt::AlignRight);
  satelliteLayout->addWidget(m_refTwoLatMinEdit);
  m_refTwoLatMaxEdit = new QLineEdit(m_satelliteFrame);
  m_refTwoLatMaxEdit->setFixedWidth(lat_width);
  m_refTwoLatMaxEdit->setValidator(new CDoubleFixedFmtValidator(-90.0, 90.0, 1, m_refTwoLatMaxEdit));
  m_refTwoLatMaxEdit->setAlignment(Qt::AlignRight);
  satelliteLayout->addWidget(m_refTwoLatMaxEdit);
  // cloud fraction (min/max)
  satelliteLayout->addWidget(new QLabel("Cloud fraction", m_satelliteFrame));
  m_cloudFractionMinEdit = new QLineEdit(m_satelliteFrame);
  const int cloud_frac_width = m_cloudFractionMinEdit->fontMetrics().horizontalAdvance("001.0");
  m_cloudFractionMinEdit->setFixedWidth(cloud_frac_width);
  m_cloudFractionMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 1.0, 1, m_cloudFractionMinEdit));
  m_cloudFractionMinEdit->setAlignment(Qt::AlignRight);
  m_cloudFractionMinEdit->setEnabled(false);
  satelliteLayout->addWidget(m_cloudFractionMinEdit);
  m_cloudFractionMaxEdit = new QLineEdit(m_satelliteFrame);
  m_cloudFractionMaxEdit->setFixedWidth(cloud_frac_width);
  m_cloudFractionMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 1.0, 1, m_cloudFractionMaxEdit));
  m_cloudFractionMaxEdit->setAlignment(Qt::AlignRight);
  m_cloudFractionMaxEdit->setEnabled(false);
  satelliteLayout->addWidget(m_cloudFractionMaxEdit);
  satelliteLayout->addSpacing(10);

  szaLayout->addWidget(m_satelliteFrame);
  szaLayout->addStretch(1);

  // Option for Files
  m_refTwoEditFrame = new QFrame(filesGroup);
  m_refTwoEditFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *refTwoEditFrameLayout = new QHBoxLayout(m_refTwoEditFrame);
  QLabel *labelTwoEdit = new QLabel(" Reference 2 ", m_refTwoEditFrame);
  labelTwoEdit->setFixedWidth(85);
  m_refTwoEdit = new QLineEdit(m_refTwoEditFrame);
  m_refTwoEdit->setMaxLength(sizeof(d->refTwoFile)-1);
  QPushButton *refTwoBrowseBtn = new QPushButton("Browse", m_refTwoEditFrame);
  refTwoBrowseBtn->setFixedWidth(70);
  refTwoEditFrameLayout->addWidget(labelTwoEdit);
  refTwoEditFrameLayout->addWidget(m_refTwoEdit, 1);
  refTwoEditFrameLayout->addWidget(refTwoBrowseBtn);

  // stack for ref 2 switching ...
  m_refTwoStack = new QStackedLayout;

  if (((p->instrumental.format!=PRJCT_INSTR_FORMAT_ASCII) && is_maxdoas((_prjctInstrFormat)p->instrumental.format)) ||
      ((p->instrumental.format==PRJCT_INSTR_FORMAT_ASCII) && ((p->instrumental.ascii.flagElevationAngle) || (p->instrumental.ascii.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED))))

   m_refTwoStack->addWidget(m_maxdoasFrame);  // automatic - takes index 0
  else
   m_refTwoStack->addWidget(m_refTwoSzaFrame);  // automatic - takes index 0

  m_refTwoStack->addWidget(m_refTwoEditFrame); // file      - takes index 1
  filesLayout->addLayout(m_refTwoStack);

  // row 2 - residual

  QFrame *residualFrame = new QFrame(filesGroup);
  residualFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *residualFrameLayout = new QHBoxLayout(residualFrame);

  if  (p->output.file_format==NETCDF)
  // (p->instrumental.format==PRJCT_INSTR_FORMAT_GEMS)
   {
    m_saveResidualCheck = new QCheckBox("Save residuals", residualFrame);
    displayLayout->addWidget(m_saveResidualCheck, 0, 0);   
    m_saveResidualCheck->setChecked(d->saveResidualsFlag ? Qt::Checked : Qt::Unchecked);
    residualFrameLayout->addWidget(m_saveResidualCheck);
   }
  else
   {
    QLabel *labelRes = new QLabel(" Residual ", residualFrame);
    labelRes->setFixedWidth(85);    
    m_residualEdit = new QLineEdit(residualFrame);
    m_residualEdit->setMaxLength(sizeof(d->residualFile)-1);
    QPushButton *residualBrowseBtn = new QPushButton("Browse", residualFrame);
    m_residualEdit->setText(d->residualFile);
    residualBrowseBtn->setFixedWidth(70);
    residualFrameLayout->addWidget(labelRes);
    residualFrameLayout->addWidget(m_residualEdit, 1);
    residualFrameLayout->addWidget(residualBrowseBtn);
    connect(residualBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseResidual()));
   }

  filesLayout->addWidget(residualFrame);

  mainLayout->addWidget(filesGroup);

  // table tabs. NOTE that the *DoasTables are dependent of the list of
  // user symbol list and the ASCII output setting of the project properties...

  m_tabs = new QTabWidget(this);

  m_moleculesTab = new CWMoleculesDoasTable("Molecules", 120);
  m_tabs->addTab(m_moleculesTab, "Molecules");
  m_linearTab = new PolynomialTab();
  m_tabs->addTab(m_linearTab, "Polynomials");
  m_nonLinearTab = new CWNonLinearParametersDoasTable("NL Parameters");
  m_tabs->addTab(m_nonLinearTab, "Predefined Parameters");
  m_shiftAndStretchTab = new CWShiftAndStretchDoasTable("Cross sections and spectrum");
  m_tabs->addTab(m_shiftAndStretchTab, "Shift and Stretch");
  m_gapTab = new CWGapDoasTable("Gaps");
  m_tabs->addTab(m_gapTab, "Gaps");
  m_outputTab = new CWOutputDoasTable("Output");
  m_tabs->addTab(m_outputTab, "Output");

  connect(m_fitsCheck, SIGNAL(stateChanged(int)),  m_moleculesTab, SLOT(slotFitColumnCheckable(int)));
  connect(m_shiftAndStretchTab, SIGNAL(signalLockSymbol(const QString &, const QObject *)),
      m_moleculesTab, SLOT(slotLockSymbol(const QString &, const QObject *)));
  connect(m_shiftAndStretchTab, SIGNAL(signalUnlockSymbol(const QString &, const QObject *)),
      m_moleculesTab, SLOT(slotUnlockSymbol(const QString &, const QObject *)));
  connect(m_moleculesTab, SIGNAL(signalSymbolListChanged(const QStringList&)),
      m_shiftAndStretchTab, SLOT(slotSymbolListChanged(const QStringList&)));
  connect(m_moleculesTab, SIGNAL(signalSymbolListChanged(const QStringList&)),
      m_outputTab, SLOT(slotSymbolListChanged(const QStringList&)));

  mainLayout->addWidget(m_tabs, 1);

  // caption string and context tag
  m_captionStr = "Properties of Analysis Window : ";
  m_captionStr += m_projectName;
  m_captionStr += " / ";
  m_captionStr += m_analysisWindowName;

  m_contextTag = m_projectName;
  m_contextTag += ":";
  m_contextTag += m_analysisWindowName;
  m_contextTag += " Prop";

  // set the current values
  int index;
  QString tmpStr;

  bool refAuto = (d->refSpectrumSelection == ANLYS_REF_SELECTION_MODE_AUTOMATIC);
  if (refAuto)
    autoButton->setChecked(true);
  else
    fileButton->setChecked(true);
  slotRefSelectionChanged(refAuto);

  index = m_calibrationCombo->findData(d->kuruczMode);
  if (index != -1) {
    m_calibrationCombo->setCurrentIndex(index);
    slotWavelengthCalibrationChanged(index);
  }

  m_fitMinEdit->setText(tmpStr.setNum(d->fitMinWavelength));
  m_fitMaxEdit->setText(tmpStr.setNum(d->fitMaxWavelength));
  m_resolEdit->setText(tmpStr.setNum(d->resolFwhm));
  m_lambda0Edit->setText(tmpStr.setNum(d->lambda0));

  m_spectrumCheck->setChecked(d->requireSpectrum ? Qt::Checked : Qt::Unchecked);
  m_polyCheck->setChecked(d->requirePolynomial ? Qt::Checked : Qt::Unchecked);
  m_fitsCheck->setChecked(d->requireFit ? Qt::Checked : Qt::Unchecked);
  m_residualCheck->setChecked(d->requireResidual ? Qt::Checked : Qt::Unchecked);
  m_predefCheck->setChecked(d->requirePredefined ? Qt::Checked : Qt::Unchecked);
  m_ratioCheck->setChecked(d->requireRefRatio ? Qt::Checked : Qt::Unchecked);

  m_refOneEdit->setText(d->refOneFile);
  m_refTwoEdit->setText(d->refTwoFile);

  m_maxdoasSzaCenterEdit->validator()->fixup(tmpStr.setNum(d->refSzaCenter));
  m_maxdoasSzaCenterEdit->setText(tmpStr);
  m_maxdoasSzaDeltaEdit->validator()->fixup(tmpStr.setNum(d->refSzaDelta));
  m_maxdoasSzaDeltaEdit->setText(tmpStr);
  m_szaCenterEdit->validator()->fixup(tmpStr.setNum(d->refSzaCenter));
  m_szaCenterEdit->setText(tmpStr);
  m_szaDeltaEdit->validator()->fixup(tmpStr.setNum(d->refSzaDelta));
  m_szaDeltaEdit->setText(tmpStr);
  m_scanCombo->setCurrentIndex(d->refSpectrumSelectionScanMode);
  m_refTwoLonMinEdit->validator()->fixup(tmpStr.setNum(d->refMinLongitude));
  m_refTwoLonMinEdit->setText(tmpStr);
  m_refTwoLonMaxEdit->validator()->fixup(tmpStr.setNum(d->refMaxLongitude));
  m_refTwoLonMaxEdit->setText(tmpStr);
  m_refTwoLatMinEdit->validator()->fixup(tmpStr.setNum(d->refMinLatitude));
  m_refTwoLatMinEdit->setText(tmpStr);
  m_refTwoLatMaxEdit->validator()->fixup(tmpStr.setNum(d->refMaxLatitude));
  m_refTwoLatMaxEdit->setText(tmpStr);
  m_cloudFractionMinEdit->validator()->fixup(tmpStr.setNum(d->cloudFractionMin));
  m_cloudFractionMinEdit->setText(tmpStr);
  m_cloudFractionMaxEdit->validator()->fixup(tmpStr.setNum(d->cloudFractionMax));
  m_cloudFractionMaxEdit->setText(tmpStr);

  m_moleculesTab->populate(&(d->crossSectionList));
  m_linearTab->populate(&(d->linear));
  m_nonLinearTab->populate(&(d->nonlinear));
  m_shiftAndStretchTab->populate(&(d->shiftStretchList));
  m_gapTab->populate(&(d->gapList));
  m_outputTab->populate(&(d->outputList));

  m_moleculesTab->slotFitColumnCheckable(m_fitsCheck->checkState());

  projectPropertiesChanged();

  // slot on automatic button is sufficient
  connect(autoButton, SIGNAL(toggled(bool)), this, SLOT(slotRefSelectionChanged(bool)));
  connect(refScan, SIGNAL(toggled(bool)), this, SLOT(slotMaxdoasSelectionChanged(bool)));
  connect(m_calibrationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWavelengthCalibrationChanged(int)));
  connect(refOneBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseRefOne()));
  connect(refTwoBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseRefTwo()));

  connect(m_tabs,SIGNAL(currentChanged(int)),this,SLOT(slotPageChanged(int)));

  notifyAcceptActionOk(true);
}

bool CWAnalysisWindowPropertyEditor::actionOk(void)
{

  // NOTE: can perform and validation checks here .. if NOT ok, return false, otherwise proceed to set data ...


  const mediate_project_t *p = CWorkSpace::instance()->findProject(m_projectName.toStdString());
  mediate_analysis_window_t *d = CWorkSpace::instance()->findAnalysisWindow(m_projectName.toStdString(), m_analysisWindowName.toStdString());
  bool brc=true;

  if (d) {

    d->kuruczMode = m_calibrationCombo->itemData(m_calibrationCombo->currentIndex()).toInt();
    d->refSpectrumSelection = m_refTwoStack->currentIndex() ? ANLYS_REF_SELECTION_MODE_FILE : ANLYS_REF_SELECTION_MODE_AUTOMATIC;
    d->refSpectrumSelectionScanMode=m_scanCombo->itemData(m_scanCombo->currentIndex()).toInt();
    d->refMaxdoasSelection = (m_scanSelection) ? ANLYS_MAXDOAS_REF_SCAN : ANLYS_MAXDOAS_REF_SZA;

    d->fitMinWavelength = m_fitMinEdit->text().toDouble();
    d->fitMaxWavelength = m_fitMaxEdit->text().toDouble();

    d->resolFwhm = m_resolEdit->text().toDouble();
    d->lambda0 = m_lambda0Edit->text().toDouble();

    d->requireSpectrum = (m_spectrumCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requirePolynomial = (m_polyCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requireFit = (m_fitsCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requireResidual = (m_residualCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requirePredefined = (m_predefCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requireRefRatio = (m_ratioCheck->checkState() == Qt::Checked) ? 1 : 0;

    strcpy(d->refOneFile, m_refOneEdit->text().toLocal8Bit().data());
    strcpy(d->refTwoFile, m_refTwoEdit->text().toLocal8Bit().data());
    
    if  (p->output.file_format==NETCDF)
     // (p->instrumental.format==PRJCT_INSTR_FORMAT_GEMS)
     d->saveResidualsFlag = (m_saveResidualCheck->checkState() == Qt::Checked) ? 1 : 0;
    else
     strcpy(d->residualFile, m_residualEdit->text().toLocal8Bit().data());

    if (((p->instrumental.format!=PRJCT_INSTR_FORMAT_ASCII) && is_maxdoas((_prjctInstrFormat)p->instrumental.format)) ||
        ((p->instrumental.format==PRJCT_INSTR_FORMAT_ASCII) && ((p->instrumental.ascii.flagElevationAngle) || (p->instrumental.ascii.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED))))
     {
      d->refSzaCenter = m_maxdoasSzaCenterEdit->text().toDouble();
      d->refSzaDelta = m_maxdoasSzaDeltaEdit->text().toDouble();
     }
    else
     {
      d->refSzaCenter = m_szaCenterEdit->text().toDouble();
      d->refSzaDelta = m_szaDeltaEdit->text().toDouble();
     }

    d->refMinLongitude = m_refTwoLonMinEdit->text().toDouble();
    d->refMaxLongitude = m_refTwoLonMaxEdit->text().toDouble();
    d->refMinLatitude = m_refTwoLatMinEdit->text().toDouble();
    d->refMaxLatitude = m_refTwoLatMaxEdit->text().toDouble();

    d->cloudFractionMin = m_cloudFractionMinEdit->text().toDouble();
    d->cloudFractionMax = m_cloudFractionMaxEdit->text().toDouble();

    // call apply for all tabs ...

    // consider changes to the symbols used by calibration
    CWorkSpace *ws = CWorkSpace::instance();

    for (int i=0; i < d->crossSectionList.nCrossSection; ++i)
      ws->decrementUseCount(d->crossSectionList.crossSection[i].symbol);

    if (d->lambda0<(double)1e-3)
     {
      char str[300];
      QString tmpStr;
      d->lambda0=(d->fitMinWavelength+d->fitMaxWavelength)*0.5;
      m_lambda0Edit->setText(tmpStr.setNum(d->lambda0));
      sprintf(str,"lambda0 automatically initialized at wavelength of the center of the spectral window %g",d->lambda0);
      QMessageBox::warning((QWidget*)this, "lambda0", str);
     }
    else if ((d->lambda0<d->fitMinWavelength) || (d->lambda0>d->fitMaxWavelength))
     {
      QMessageBox::warning((QWidget*)this, "lambda0", "lambda0 out of the spectral window range");
      brc=false;
     }

    if (brc)
     brc=m_moleculesTab->apply(&(d->crossSectionList));

    for (int i=0; i < d->crossSectionList.nCrossSection; ++i)
      ws->incrementUseCount(d->crossSectionList.crossSection[i].symbol);

    m_linearTab->apply(&(d->linear));
    m_nonLinearTab->apply(&(d->nonlinear));
    m_shiftAndStretchTab->apply(&(d->shiftStretchList));
    m_gapTab->apply(&(d->gapList));
    m_outputTab->apply(&(d->outputList));

    // this is a change to the project properties (indirectly).
    QString updateBlock = m_projectName;
    m_projectName.clear();
    ws->modifiedProjectProperties(updateBlock.toStdString());
    m_projectName = updateBlock;
  }
 else
  brc=false;


  // Project not found ... TODO

  return brc;
}

void CWAnalysisWindowPropertyEditor::actionHelp(void)
{
    const char *analysisPages[]={"Analysis_Molecules",
                           "Analysis_Polynomial",
                           "Analysis_Predefined",
                           "Analysis_Shift",
                           "Analysis_Gaps",
                           "Analysis_Output"};

 CHelpSystem::showHelpTopic("Analysis",((m_selectedPage>=0) && (m_selectedPage<6))?analysisPages[m_selectedPage]:"Analysis");
}

void CWAnalysisWindowPropertyEditor::updateModifyProject(const std::string &projectName)
{
  if (projectName == m_projectName.toStdString())
    projectPropertiesChanged();
}

void CWAnalysisWindowPropertyEditor::projectPropertiesChanged()
{
  const mediate_project_t *d = CWorkSpace::instance()->findProject(m_projectName.toStdString());

  if (d) {
    // enable/disable filtering
    m_moleculesTab->setColumnEnabled(5, (d->lowpass.mode != PRJCT_FILTER_TYPE_NONE));

    bool analysisEnabled = (d->output.analysisFlag != 0);

    // non-linear ... TODO

    // enable/disable shift+stretch store columns
                                                                                // Check also for calibFlag !!!!
    // m_linearTab->setColumnEnabled(1, analysisEnabled);                       // polynomial class not a CDoasTable ???
    // m_linearTab->setColumnEnabled(2, analysisEnabled);

    m_nonLinearTab->setColumnEnabled(3, analysisEnabled);
    m_nonLinearTab->setColumnEnabled(4, analysisEnabled);


    m_shiftAndStretchTab->setColumnEnabled(2, analysisEnabled);                 // !!! DANGEROUS TO USE NUMERICAL INDEX FOR COLUMNS
    m_shiftAndStretchTab->setColumnEnabled(3, analysisEnabled);
    m_shiftAndStretchTab->setColumnEnabled(4, analysisEnabled);

    // enable/disable the output table
    m_outputTab->setEnabled(analysisEnabled);

    if (m_autoSelection)
     {
      // MAXDOAS measurements

      if (((d->instrumental.format!=PRJCT_INSTR_FORMAT_ASCII) && is_maxdoas((_prjctInstrFormat)d->instrumental.format)) ||
          ((d->instrumental.format==PRJCT_INSTR_FORMAT_ASCII) && ((d->instrumental.ascii.flagElevationAngle) || (d->instrumental.ascii.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED))))
       {
        m_maxdoasFrame->show();
        m_refTwoSzaFrame->hide();
       }
      else
       {
        m_maxdoasFrame->hide();
        m_refTwoSzaFrame->show();
       }
     }
    else
     {
         m_maxdoasFrame->hide();
         m_refTwoSzaFrame->hide();
     }

    // satellite instruments

    switch (d->instrumental.format)  {
    // -------------------------------------------------------------------------
       case PRJCT_INSTR_FORMAT_GDP_BIN:
       case PRJCT_INSTR_FORMAT_GOME1_NETCDF:
       case PRJCT_INSTR_FORMAT_GOME2:
         // GOME & GOME-2 have cloud fraction info in L1 data:
         m_cloudFractionMinEdit->setEnabled(true);
         m_cloudFractionMaxEdit->setEnabled(true);
         //FALL-THROUGH
       case PRJCT_INSTR_FORMAT_SCIA_PDS:
       case PRJCT_INSTR_FORMAT_OMI:
       case PRJCT_INSTR_FORMAT_OMIV4:
       case PRJCT_INSTR_FORMAT_OMPS:
       case PRJCT_INSTR_FORMAT_TROPOMI:
          m_satelliteFrame->show();
       break;
    // -------------------------------------------------------------------------
       default:
        {
            m_satelliteFrame->hide();
        }
       break;
    // -------------------------------------------------------------------------
    }

    m_refTwoStack->setCurrentIndex((m_autoSelection)?0:1);
  }
}

void CWAnalysisWindowPropertyEditor::slotMaxdoasSelectionChanged(bool checked)
{
  // checked is true for scan selection
  m_scanSelection = checked;

  if (m_scanSelection)
   {
    m_maxdoasSzaFrame->hide();
    m_maxdoasScanFrame->show();
   }
  else
   {
    m_maxdoasSzaFrame->show();
    m_maxdoasScanFrame->hide();
   }
}

void CWAnalysisWindowPropertyEditor::slotRefSelectionChanged(bool checked)
{
  // checked is true for auto selection
  m_autoSelection = checked;

  m_refTwoStack->setCurrentIndex(m_autoSelection ? 0 : 1);
}

void CWAnalysisWindowPropertyEditor::slotWavelengthCalibrationChanged(int index)
{
  int mode = m_calibrationCombo->itemData(index).toInt();

  switch (mode) {
  case ANLYS_KURUCZ_NONE:
  case ANLYS_KURUCZ_REF:
    {
      m_refOneFrame->setEnabled(true);
      m_refTwoEditFrame->setEnabled(true);
      m_refTwoSzaFrame->setEnabled(true);
      m_maxdoasFrame->setEnabled(true);
    }
    break;
  case ANLYS_KURUCZ_SPEC:
    {
      m_refOneFrame->setEnabled(false);
      m_refTwoEditFrame->setEnabled(false);
      m_refTwoSzaFrame->setEnabled(false);
      m_maxdoasFrame->setEnabled(false);
    }
    break;
  case ANLYS_KURUCZ_REF_AND_SPEC:
    {
      m_refOneFrame->setEnabled(false);
      m_refTwoEditFrame->setEnabled(true);
      m_refTwoSzaFrame->setEnabled(true);
      m_maxdoasFrame->setEnabled(true);
    }
    break;
  }
}

void CWAnalysisWindowPropertyEditor::slotBrowseRefOne()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Reference 1",
                          pref->directoryName("Ref"),
                          "Reference File (*.ref *.he4 *.he5 *.nc);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);

    m_refOneEdit->setText(filename);
  }
}

void CWAnalysisWindowPropertyEditor::slotBrowseRefTwo()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Reference 2",
                          pref->directoryName("Ref"),
                          "Reference File (*.ref *.he4 *.he5 *.nc);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);

    m_refTwoEdit->setText(filename);
  }
}

void CWAnalysisWindowPropertyEditor::slotBrowseResidual()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getSaveFileName(this, "Select Residual",
                          pref->directoryName("Residual"),
                          "All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Residual", filename);

    m_residualEdit->setText(filename);
  }
}

void CWAnalysisWindowPropertyEditor::slotPageChanged(int index)
{
 m_selectedPage=index;
}
