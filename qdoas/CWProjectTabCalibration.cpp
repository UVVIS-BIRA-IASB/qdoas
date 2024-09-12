/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QFileDialog>

#include "CWProjectTabCalibration.h"
#include "CPreferences.h"
#include "CValidator.h"
#include "PolynomialTab.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabCalibration::CWProjectTabCalibration(const mediate_project_calibration_t *properties, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QLabel *methodLabel,*refLabel;

  QGridLayout *topLayout = new QGridLayout;

  mainLayout->addSpacing(5);

  // ref file
  topLayout->addWidget((refLabel=new QLabel("Solar Ref. File", this)), 0, 0);
  m_refFileEdit  = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile) - 1); // limit test length to buffer size
  m_refBrowseBtn = new QPushButton("Browse", this);
  topLayout->addWidget(m_refFileEdit, 0, 1, 1, 2);
  topLayout->addWidget(m_refBrowseBtn, 0, 3);

  // methodType
  topLayout->addWidget((methodLabel=new QLabel("Analysis Method", this)), 1, 0);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical Density Fitting", QVariant(OPTICAL_DENSITY_FIT));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)", QVariant(INTENSITY_FIT));
  topLayout->addWidget(m_methodCombo, 1, 1, 1, 3); // spans 3 columns

  // line shape
  topLayout->addWidget(new QLabel("Line Shape (SFP)", this), 2, 0);
  m_lineShapeCombo = new QComboBox(this);
  m_lineShapeCombo->addItem("Don't Fit", QVariant(PRJCT_CALIB_FWHM_TYPE_NONE));
  m_lineShapeCombo->addItem("File", QVariant(PRJCT_CALIB_FWHM_TYPE_FILE));
  m_lineShapeCombo->addItem("Gaussian", QVariant(PRJCT_CALIB_FWHM_TYPE_GAUSS));
  m_lineShapeCombo->addItem("Error Function", QVariant(PRJCT_CALIB_FWHM_TYPE_ERF));
  m_lineShapeCombo->addItem("2n-Lorentz", QVariant(PRJCT_CALIB_FWHM_TYPE_INVPOLY));
  m_lineShapeCombo->addItem("Voigt", QVariant(PRJCT_CALIB_FWHM_TYPE_VOIGT));
  m_lineShapeCombo->addItem("Asymmetric Gaussian", QVariant(PRJCT_CALIB_FWHM_TYPE_AGAUSS));
  m_lineShapeCombo->addItem("Super Gaussian", QVariant(PRJCT_CALIB_FWHM_TYPE_SUPERGAUSS));
  topLayout->addWidget(m_lineShapeCombo, 2, 1);

  // File
  m_slfFileEdit  = new QLineEdit(this);
  QPushButton *fileBrowseBtn = new QPushButton("Browse", this);
  m_fileWidget = new QFrame(this);
  m_fileWidget->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *fileLayout = new QHBoxLayout(m_fileWidget);
  fileLayout->addWidget(m_slfFileEdit);
  fileLayout->addWidget(fileBrowseBtn);

  topLayout->addWidget(m_fileWidget, 2, 2, 1, 2);
  m_fileWidget->hide(); // show when lineshape combo is File

  // order of 2n-Lorentz
  m_orderWidget = new QFrame(this);
  m_orderWidget->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *orderLayout = new QHBoxLayout(m_orderWidget);
  orderLayout->setAlignment(Qt::AlignLeft);
  orderLayout->addWidget(new QLabel("Order", this));
  m_orderSpinBox = new QSpinBox(this);
  m_orderSpinBox->setRange(1, 10);
  orderLayout->addWidget(m_orderSpinBox);
  topLayout->addWidget(m_orderWidget, 2, 2, 1, 2);
  m_orderWidget->hide(); // show when lineshape combo is cSpectralLineShapeLorentz

  topLayout->setColumnStretch(0, 0);
  topLayout->setColumnStretch(1, 0);
  topLayout->setColumnStretch(2, 1);
  topLayout->setColumnStretch(3, 0);

  // force some sizes to prevent 'jumpy' display.
  m_orderSpinBox->setFixedHeight(m_lineShapeCombo->sizeHint().height());
  m_slfFileEdit->setFixedHeight(m_lineShapeCombo->sizeHint().height());

  mainLayout->addLayout(topLayout);

  // middle
  QHBoxLayout *groupLayout = new QHBoxLayout;
  groupLayout->setSpacing(5);

  // display
  QGroupBox *displayGroup = new QGroupBox("Display", this);
  QGridLayout *displayLayout = new QGridLayout(displayGroup);
  displayLayout->setSpacing(0);

  m_spectraCheck = new QCheckBox("Spectra", displayGroup);
  displayLayout->addWidget(m_spectraCheck, 0, 0);
  m_fitsCheck = new QCheckBox("Fits", displayGroup);
  displayLayout->addWidget(m_fitsCheck, 1, 0);
  m_residualCheck = new QCheckBox("Residual", displayGroup);
  displayLayout->addWidget(m_residualCheck, 2, 0);
  m_shiftSfpCheck = new QCheckBox("Shift/SFP", displayGroup);
  displayLayout->addWidget(m_shiftSfpCheck, 3, 0);

  groupLayout->addWidget(displayGroup);

  // Polynomial Degree
  QGroupBox *polyGroup = new QGroupBox("Polynomial Degree", this);
  QGridLayout *polyLayout = new QGridLayout(polyGroup);
  polyLayout->setAlignment(Qt::AlignLeft);

  polyLayout->addWidget(new QLabel("Shift", polyGroup), 0, 0, Qt::AlignRight);
  m_shiftDegreeSpinBox = new QSpinBox(polyGroup);
  m_shiftDegreeSpinBox->setRange(0,5);
  m_shiftDegreeSpinBox->setFixedWidth(50);
  polyLayout->addWidget(m_shiftDegreeSpinBox, 0, 1, Qt::AlignLeft);

  polyLayout->addWidget(new QLabel("SFP", polyGroup), 1, 0, Qt::AlignRight);
  m_sfpDegreeSpinBox = new QSpinBox(polyGroup);
  m_sfpDegreeSpinBox->setRange(0,5);
  m_sfpDegreeSpinBox->setFixedWidth(50);
  polyLayout->addWidget(m_sfpDegreeSpinBox, 1, 1, Qt::AlignLeft);

  groupLayout->addWidget(polyGroup);

  // window limits
  QGroupBox *intervalGroup = new QGroupBox("Calib. Interval (nm)", this);
  QGridLayout *intervalLayout = new QGridLayout(intervalGroup);
  intervalLayout->setAlignment(Qt::AlignLeft);

  intervalLayout->addWidget(new QLabel("Min", intervalGroup), 0, 0, Qt::AlignRight);
  m_lambdaMinEdit = new QLineEdit(intervalGroup);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 2000.0, 2, m_lambdaMinEdit));
  m_lambdaMinEdit->setFixedWidth(100);
  intervalLayout->addWidget(m_lambdaMinEdit, 0, 1, Qt::AlignLeft);

  intervalLayout->addWidget(new QLabel("Max", intervalGroup), 1, 0, Qt::AlignRight);
  m_lambdaMaxEdit = new QLineEdit(intervalGroup);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 2000.0, 2, m_lambdaMaxEdit));
  m_lambdaMaxEdit->setFixedWidth(100);
  intervalLayout->addWidget(m_lambdaMaxEdit, 1, 1, Qt::AlignLeft);

  groupLayout->addWidget(intervalGroup);

  // Calibration interval subdivision

  QGroupBox *windowGroup = new QGroupBox("Calibration windows", this);
  QGridLayout *windowLayout = new QGridLayout(windowGroup);
  windowLayout->setAlignment(Qt::AlignLeft);

  windowLayout->addWidget(new QLabel("Subdivision", this), 0, 0);
  m_subwindowsCombo = new QComboBox(this);
  m_subwindowsCombo->addItem("Contiguous", QVariant(PRJCT_CALIB_WINDOWS_CONTIGUOUS));
  m_subwindowsCombo->addItem("Sliding", QVariant(PRJCT_CALIB_WINDOWS_SLIDING));
  m_subwindowsCombo->addItem("Custom", QVariant(PRJCT_CALIB_WINDOWS_CUSTOM));
  windowLayout->addWidget(m_subwindowsCombo, 0, 1);

  windowLayout->addWidget(new QLabel("Number", windowGroup), 1, 0, Qt::AlignRight);
  m_subWindowsSpinBox = new QSpinBox(this);
  m_subWindowsSpinBox->setFixedWidth(50);
  m_subWindowsSpinBox->setRange(1, MAX_CALIB_WINDOWS);
  windowLayout->addWidget(m_subWindowsSpinBox, 1, 1, Qt::AlignLeft);

  m_subWindowsSizeLabel=new QLabel("Size (nm)", windowGroup);
  m_subWindowsSizeEdit=new QLineEdit(windowGroup);
  m_subWindowsSizeEdit->setFixedWidth(60);
  m_subWindowsSizeEdit->setValidator(new CDoubleFixedFmtValidator(5., 500.0, 2, m_subWindowsSizeEdit));

  m_subWindowsCustomButton = new QPushButton("Define windows", this);

  windowLayout->addWidget(m_subWindowsSizeLabel,2,0,Qt::AlignRight);
  windowLayout->addWidget(m_subWindowsSizeEdit,2,1,Qt::AlignLeft);
  windowLayout->addWidget(m_subWindowsCustomButton,2,0,1,2,Qt::AlignHCenter);

  groupLayout->addWidget(windowGroup);

  // Preshift

  QGroupBox   *preshiftGroup = new QGroupBox("Preshift (nm)", this);
  QGridLayout *preshiftLayout = new QGridLayout(preshiftGroup);

  m_preshiftCheck = new QCheckBox("Calculate preshift", preshiftGroup);
  preshiftLayout->addWidget(m_preshiftCheck, 0, 0,1,0);
  m_preshiftMinLabel = new QLabel("Min preshift", preshiftGroup);
  preshiftLayout->addWidget(m_preshiftMinLabel, 1, 0, Qt::AlignRight);
  m_preshiftMinEdit = new QLineEdit(preshiftGroup);
  m_preshiftMinEdit->setFixedWidth(60);
  preshiftLayout->addWidget(m_preshiftMinEdit, 1, 1, Qt::AlignRight);
  m_preshiftMaxLabel = new QLabel("Max preshift", preshiftGroup);
  preshiftLayout->addWidget(m_preshiftMaxLabel, 2, 0, Qt::AlignRight);
  m_preshiftMaxEdit = new QLineEdit(preshiftGroup);
  m_preshiftMaxEdit->setFixedWidth(60);
  preshiftLayout->addWidget(m_preshiftMaxEdit, 2, 1, Qt::AlignRight);

  groupLayout->addWidget(preshiftGroup);
  mainLayout->addLayout(groupLayout);

  // fit paramters tables
  m_tabs = new QTabWidget(this);

  m_moleculesTab = new CWMoleculesDoasTable("Molecules", 120);
  m_tabs->addTab(m_moleculesTab, "Molecules");
  m_linearTab = new PolynomialTab;
  m_tabs->addTab(m_linearTab, "Linear Parameters");
  m_sfpTab = new CWSfpParametersDoasTable("SFP Parameters");
  m_tabs->addTab(m_sfpTab, "SFP Parameters");
  m_shiftAndStretchTab = new CWShiftAndStretchDoasTable("Cross sections and spectrum");
  m_tabs->addTab(m_shiftAndStretchTab, "Shift and Stretch");
  m_gapTab = new CWGapDoasTable("Gaps");
  m_tabs->addTab(m_gapTab, "Gaps");
  m_outputTab = new CWOutputDoasTable("Output");
  m_tabs->addTab(m_outputTab, "Output");

  mainLayout->addWidget(m_tabs, 1);

  // tabel interconnections - MUST be connected before populating the tables ....
  connect(m_fitsCheck, SIGNAL(stateChanged(int)),  m_moleculesTab, SLOT(slotFitColumnCheckable(int)));
  connect(m_shiftAndStretchTab, SIGNAL(signalLockSymbol(const QString &, const QObject *)),
      m_moleculesTab, SLOT(slotLockSymbol(const QString &, const QObject *)));
  connect(m_shiftAndStretchTab, SIGNAL(signalUnlockSymbol(const QString &, const QObject *)),
      m_moleculesTab, SLOT(slotUnlockSymbol(const QString &, const QObject *)));
  connect(m_moleculesTab, SIGNAL(signalSymbolListChanged(const QStringList&)),
      m_shiftAndStretchTab, SLOT(slotSymbolListChanged(const QStringList&)));
  connect(m_moleculesTab, SIGNAL(signalSymbolListChanged(const QStringList&)),
      m_outputTab, SLOT(slotSymbolListChanged(const QStringList&)));

  // set the current values
  m_refFileEdit->setText(QString(properties->solarRefFile));
  m_slfFileEdit->setText(QString(properties->slfFile));

  index = m_methodCombo->findData(QVariant(properties->methodType));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  m_orderSpinBox->setValue(properties->lorentzOrder);

  index = m_lineShapeCombo->findData(QVariant(properties->lineShape));
  if (index != -1) {
    m_lineShapeCombo->setCurrentIndex(index); // no signals yet
    slotLineShapeSelectionChanged(index);
  }

  m_spectraCheck->setChecked(properties->requireSpectra != 0);
  m_fitsCheck->setChecked(properties->requireFits != 0);
  m_residualCheck->setChecked(properties->requireResidual != 0);
  m_shiftSfpCheck->setChecked(properties->requireShiftSfp != 0);

  m_shiftDegreeSpinBox->setValue(properties->shiftDegree);
  m_sfpDegreeSpinBox->setValue(properties->sfpDegree);

  m_lambdaMinEdit->validator()->fixup(tmpStr.setNum(properties->wavelengthMin));
  m_lambdaMinEdit->setText(tmpStr);
  m_lambdaMaxEdit->validator()->fixup(tmpStr.setNum(properties->wavelengthMax));
  m_lambdaMaxEdit->setText(tmpStr);

  index = m_subwindowsCombo->findData(QVariant(properties->divisionMode));
  if (index != -1)
   {
    m_subwindowsCombo->setCurrentIndex(index);
    slotDivisionModeChanged(index);
   }

  m_subWindowsSpinBox->setValue(properties->subWindows);
  m_subWindowsSizeEdit->validator()->fixup(tmpStr.setNum(properties->windowSize));
  m_subWindowsSizeEdit->setText(tmpStr);

  memcpy(m_customLambdaMin,properties->customLambdaMin,sizeof(double)*MAX_CALIB_WINDOWS);
  memcpy(m_customLambdaMax,properties->customLambdaMax,sizeof(double)*MAX_CALIB_WINDOWS);

  m_preshiftCheck->setChecked(properties->preshiftFlag!= 0);

  m_preshiftMinEdit->setValidator(new CDoubleFixedFmtValidator(-20., 20., 2, m_preshiftMinEdit));
  m_preshiftMinEdit->validator()->fixup(tmpStr.setNum(properties->preshiftMin));
  m_preshiftMinEdit->setText(tmpStr);
  m_preshiftMaxEdit->setValidator(new CDoubleFixedFmtValidator(-20., 20., 2, m_preshiftMaxEdit));
  m_preshiftMaxEdit->validator()->fixup(tmpStr.setNum(properties->preshiftMax));
  m_preshiftMaxEdit->setText(tmpStr);

  m_moleculesTab->populate(&(properties->crossSectionList));
  m_linearTab->populate(&(properties->linear));
  m_sfpTab->populate(&(properties->sfp[0]));
  m_shiftAndStretchTab->populate(&(properties->shiftStretchList));
  m_gapTab->populate(&(properties->gapList));
  m_outputTab->populate(&(properties->outputList));

  m_moleculesTab->slotFitColumnCheckable(m_fitsCheck->checkState());
  // disable the AMF tab
  m_moleculesTab->setColumnEnabled(2, false);

  // no AMF, residual or vertical output
  m_outputTab->setColumnEnabled(0, false);
  m_outputTab->setColumnEnabled(1, false);
  m_outputTab->setColumnEnabled(5, false);
  m_outputTab->setColumnEnabled(6, false);
  m_outputTab->setColumnEnabled(7, false);

  // connections

  connect(m_lineShapeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLineShapeSelectionChanged(int)));
  connect(m_subwindowsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDivisionModeChanged(int)));
  connect(m_subWindowsCustomButton,SIGNAL(clicked()), this, SLOT(slotDefineCustomWindows()));
  connect(m_refBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarRefFile()));
  connect(fileBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSlfFile()));
}

void CWProjectTabCalibration::apply(mediate_project_calibration_t *properties) const
{
     // a safe text length is assured.
  strcpy(properties->solarRefFile, m_refFileEdit->text().toLocal8Bit().data());
  strcpy(properties->slfFile, m_slfFileEdit->text().toLocal8Bit().data());

  properties->methodType = m_methodCombo->itemData(m_methodCombo->currentIndex()).toInt();

  properties->lineShape = m_lineShapeCombo->itemData(m_lineShapeCombo->currentIndex()).toInt();
  properties->lorentzOrder = m_orderSpinBox->value();

  properties->requireSpectra = (m_spectraCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->requireFits = (m_fitsCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->requireResidual = (m_residualCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->requireShiftSfp = (m_shiftSfpCheck->checkState() == Qt::Checked) ? 1 : 0;

  properties->shiftDegree = m_shiftDegreeSpinBox->value();
  properties->sfpDegree = m_sfpDegreeSpinBox->value();

  properties->wavelengthMin = m_lambdaMinEdit->text().toDouble();
  properties->wavelengthMax = m_lambdaMaxEdit->text().toDouble();
  properties->divisionMode = m_subwindowsCombo->itemData(m_subwindowsCombo->currentIndex()).toInt();
  properties->subWindows = m_subWindowsSpinBox->value();
  properties->windowSize = m_subWindowsSizeEdit->text().toDouble();

  properties->preshiftFlag = (m_preshiftCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->preshiftMin = m_preshiftMinEdit->text().toDouble();
  properties->preshiftMax = m_preshiftMaxEdit->text().toDouble();

  CheckCustomWindows((double *)m_customLambdaMin,(double *)m_customLambdaMax,properties->subWindows);  // Check custom windows

  memcpy(properties->customLambdaMin,m_customLambdaMin,sizeof(double)*MAX_CALIB_WINDOWS);
  memcpy(properties->customLambdaMax,m_customLambdaMax,sizeof(double)*MAX_CALIB_WINDOWS);

  m_moleculesTab->apply(&(properties->crossSectionList));
  m_linearTab->apply(&(properties->linear));
  m_sfpTab->apply(&(properties->sfp[0]));
  m_shiftAndStretchTab->apply(&(properties->shiftStretchList));
  m_gapTab->apply(&(properties->gapList));
  m_outputTab->apply(&(properties->outputList));
}

void CWProjectTabCalibration::slotLineShapeSelectionChanged(int index)
{
  int tmp = m_lineShapeCombo->itemData(index).toInt();

  if (tmp == PRJCT_CALIB_FWHM_TYPE_FILE)
   m_fileWidget->show();
  else
    m_fileWidget->hide();

  if (tmp == PRJCT_CALIB_FWHM_TYPE_INVPOLY)
    m_orderWidget->show();
  else
    m_orderWidget->hide();

  if (tmp==PRJCT_CALIB_FWHM_TYPE_NONE) {
    m_refFileEdit->setEnabled(false);
    m_refBrowseBtn->setEnabled(false);
  } else {
    m_refFileEdit->setEnabled(true);
    m_refBrowseBtn->setEnabled(true);
  }

  m_sfpDegreeSpinBox->setEnabled(tmp != PRJCT_CALIB_FWHM_TYPE_NONE);

}

CWProjectTabCalibrationCustomWindowsDialog::CWProjectTabCalibrationCustomWindowsDialog(const double *lambdaMin, const double *lambdaMax, const int nWindows,
                         QWidget *parent) :
  QDialog(parent)
{
  setWindowTitle("Define calibration subwindows");

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QGridLayout *gridLayout=new QGridLayout(this);
  char str[80];
  QString tmpStr;
  double calibIntervalMin,calibIntervalMax;

  m_nWindows=nWindows;

  calibIntervalMin=lambdaMin[0];
  calibIntervalMax=lambdaMax[nWindows-1];

  gridLayout->addWidget(new QLabel("Lambda Min"),0,1, Qt::AlignRight);
  gridLayout->addWidget(new QLabel("Lambda Max"),0,2, Qt::AlignRight);

  for (int i=0;i<nWindows;i++) {
    sprintf(str,"Window %d",i+1);
    gridLayout->addWidget(new QLabel(str, this),i+1,0, Qt::AlignRight);

    m_customLambdaMinEdit[i]=new QLineEdit(this);
    m_customLambdaMinEdit[i]->setValidator(new CDoubleFixedFmtValidator(calibIntervalMin, calibIntervalMax, 2, m_customLambdaMinEdit[i]));
    m_customLambdaMinEdit[i]->setFixedWidth(60);
    m_customLambdaMinEdit[i]->validator()->fixup(tmpStr.setNum(lambdaMin[i]));
    m_customLambdaMinEdit[i]->setText(tmpStr);

    m_customLambdaMaxEdit[i]=new QLineEdit(this);
    m_customLambdaMaxEdit[i]->setValidator(new CDoubleFixedFmtValidator(calibIntervalMin, calibIntervalMax, 2, m_customLambdaMaxEdit[i]));
    m_customLambdaMaxEdit[i]->setFixedWidth(60);
    m_customLambdaMaxEdit[i]->validator()->fixup(tmpStr.setNum(lambdaMax[i]));
    m_customLambdaMaxEdit[i]->setText(tmpStr);

    gridLayout->addWidget(m_customLambdaMinEdit[i],i+1,1,Qt::AlignRight);
    gridLayout->addWidget(m_customLambdaMaxEdit[i],i+1,2,Qt::AlignRight);
  }

  m_customLambdaMinEdit[0]->setEnabled(false);
  m_customLambdaMaxEdit[nWindows-1]->setEnabled(false);

  gridLayout->addWidget(buttonBox,nWindows+2,0,1,3,Qt::AlignCenter);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void CWProjectTabCalibrationCustomWindowsDialog::GetLambdaMin(double *lambdaMin)
 {
     for (int i=0;i<m_nWindows;i++)
      lambdaMin[i]=m_customLambdaMinEdit[i]->text().toDouble();
 }

void CWProjectTabCalibrationCustomWindowsDialog::GetLambdaMax(double *lambdaMax)
 {
     for (int i=0;i<m_nWindows;i++)
      lambdaMax[i]=m_customLambdaMaxEdit[i]->text().toDouble();
 }

void CWProjectTabCalibration::slotDefineCustomWindows(void)
 {
     int nWindows=m_subWindowsSpinBox->value();

  // Check windows before entering dialog box

     CheckCustomWindows((double *)m_customLambdaMin,(double *)m_customLambdaMax,nWindows);

  CWProjectTabCalibrationCustomWindowsDialog dialog((const double *)m_customLambdaMin,(const double *)m_customLambdaMax,nWindows, this);

  if (dialog.exec() == QDialog::Accepted)
   {
    dialog.GetLambdaMin((double *)m_customLambdaMin);
    dialog.GetLambdaMax((double *)m_customLambdaMax);

    // Check windows when exiting dialog box

    CheckCustomWindows((double *)m_customLambdaMin,(double *)m_customLambdaMax,nWindows);
   }
 }

void CWProjectTabCalibration::CheckCustomWindows(double *customLambdaMin,double *customLambdaMax,int nWindows) const
 {
     double calibIntervalMin=m_lambdaMinEdit->text().toDouble();
  double calibIntervalMax = m_lambdaMaxEdit->text().toDouble();
  double windowSize;

  customLambdaMin[0]=calibIntervalMin;
  customLambdaMax[nWindows-1]=calibIntervalMax;

  windowSize=(calibIntervalMax-calibIntervalMin)/nWindows;

  for (int i=0;i<nWindows-1;i++)
   {
       if (customLambdaMax[i]<customLambdaMin[i])
        customLambdaMax[i]=customLambdaMin[i]+windowSize;
       if (customLambdaMax[i]>calibIntervalMax)
        customLambdaMax[i]=calibIntervalMax;
       if (customLambdaMin[i+1]<calibIntervalMin)
        customLambdaMin[i+1]=customLambdaMax[i];
   }
 }

void CWProjectTabCalibration::slotDivisionModeChanged(int index)
{
  int tmp = m_subwindowsCombo->itemData(index).toInt();

  if (tmp == PRJCT_CALIB_WINDOWS_SLIDING)
   {
    m_subWindowsSizeLabel->show();
    m_subWindowsSizeEdit->show();
    m_subWindowsCustomButton->hide();
   }
  else if (tmp == PRJCT_CALIB_WINDOWS_CUSTOM)
   {
    m_subWindowsSizeLabel->hide();
    m_subWindowsSizeEdit->hide();
    m_subWindowsCustomButton->show();

    CheckCustomWindows((double *)m_customLambdaMin,(double *)m_customLambdaMax,m_subWindowsSpinBox->value());
   }
  else
   {
       m_subWindowsSizeLabel->hide();
    m_subWindowsSizeEdit->hide();
    m_subWindowsCustomButton->hide();
   }
}

void CWProjectTabCalibration::slotBrowseSolarRefFile()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Open Solar Reference File",
                          pref->directoryName("Ref"),
                                                  "Kurucz File (*.ktz);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);

    m_refFileEdit->setText(filename);
  }
}

void CWProjectTabCalibration::slotBrowseSlfFile()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Open Slit Function File",
                          pref->directoryName("Slf"),
                                                  "Slit function File (*.slf);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Slf", filename);

    m_slfFileEdit->setText(filename);
  }
}

void CWProjectTabCalibration::slotOutputCalibration(bool enabled)
{
  m_outputTab->setEnabled(enabled);
}

