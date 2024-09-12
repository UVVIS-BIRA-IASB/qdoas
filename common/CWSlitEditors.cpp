
/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QStackedLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include "CWSlitEditors.h"
#include "CValidator.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

//--------------------------------------------------------

static const int cSuggestedColumnZeroWidth = 115; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cStandardEditWidth         = 70;


//--------------------------------------------------------

CWSlitFileBase::CWSlitFileBase(QWidget *parent) :
  QFrame(parent)
{
}

QLineEdit *CWSlitFileBase::helperConstructFileEdit(QGridLayout *gridLayout, int row, const char *labelText, const char *fileName, int len) {

  QLabel *label = new QLabel(labelText);
  label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label->setMinimumSize(cSuggestedColumnZeroWidth, 0);
  gridLayout->addWidget(label, row, 0, Qt::AlignRight);

  QLineEdit *fileEdit = new QLineEdit(this);
  fileEdit->setMaxLength(len);
  fileEdit->setText(fileName);
  gridLayout->addWidget(fileEdit, row, 1);

  QPushButton *browseButton = new QPushButton("Browse");
  gridLayout->addWidget(browseButton, row, 2);
  // store a pointer to the fileEdit in browseButton's properties, so
  // we can find the fileEdit when the button is clicked
  QVariant v_fileEdit;
  v_fileEdit.setValue(fileEdit);
  browseButton->setProperty("lineEdit", v_fileEdit);
  connect(browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseFile()));

  return fileEdit;
}

void CWSlitFileBase::slotBrowseFile()
{
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  QLineEdit *fileEdit = qobject_cast<QLineEdit *>(qvariant_cast<QObject *>(button->property("lineEdit")));

  CPreferences *pref = CPreferences::instance();
  QString filename = QFileDialog::getOpenFileName(this, "Select Slit Function File",
                          pref->directoryName("Slit"),
                          "Slit Function File (*.slf);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Slit", filename);
    fileEdit->setText(filename);
  }
}

void CWSlitFileBase::toggleVisible(int state) {
  m_toggleWavelengthStack->setCurrentIndex((state)?1:0);
}

void CWSlitFileBase::slotToggleWavelength(int state)
 {
   toggleVisible(state);
 }

CWSlitNoneEdit::CWSlitNoneEdit(const struct slit_file *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
}

void CWSlitNoneEdit::reset(const struct slit_file *d)
{
}

void CWSlitNoneEdit::apply(struct slit_file *d) const
{
}

void CWSlitFileEdit::toggleVisible(int state) {

  for(int row=0; row<m_wvlDependentGrid->rowCount(); ++row) {
    for(int col=0; col<m_wvlDependentGrid->columnCount(); ++col) {
      QLayoutItem *item = m_wvlDependentGrid->itemAtPosition(row,col);
      item->widget()->setVisible(m_wavelengthDependent->isChecked());
    }
  }

}

//--------------------------------------------------------

CWSlitFileEdit::CWSlitFileEdit(const struct slit_file *d, QWidget *parent) :
  CWSlitFileBase(parent) {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Wavelength dependent
  m_wavelengthDependent = new QCheckBox("Wavelength dependent stretch factor", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect (m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QGridLayout *slitFileGrid = new QGridLayout;
  mainLayout->addLayout(slitFileGrid);
  m_slitFileEdit = helperConstructFileEdit(slitFileGrid, 0, "Slit Function File", d->filename, sizeof(d->filename)-1);

  m_wvlDependentGrid = new QGridLayout;
  mainLayout->addLayout(m_wvlDependentGrid);
  m_stretchEdit = helperConstructFileEdit(m_wvlDependentGrid, 0, "Stretch on wavelength", d->filename2, sizeof(d->filename2)-1);

  reset(d);

  mainLayout->addStretch(1);
}

void CWSlitFileEdit::reset(const struct slit_file *d)
{
  m_slitFileEdit->setText(d->filename);

  if (d->wveDptFlag)
      m_stretchEdit->setText(d->filename2);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  this->toggleVisible(m_wavelengthDependent->checkState());

}

void CWSlitFileEdit::apply(struct slit_file *d) const
{
  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_slitFileEdit->text().toLocal8Bit().data());

  if (d->wveDptFlag)
   strcpy(d->filename2, m_stretchEdit->text().toLocal8Bit().data());
}

//--------------------------------------------------------

CWSlitGaussianEdit::CWSlitGaussianEdit(const struct slit_gaussian *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Wavelength dependent
  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *fwhmFrameLayout = new QHBoxLayout(fwhmFrame);
  QLabel *labelfwhm = new QLabel("FWHM (nm)", fwhmFrame);
  labelfwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelfwhm->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  fwhmFrameLayout->addWidget(labelfwhm);
  fwhmFrameLayout->addWidget(m_fwhmEdit, 1);
  fwhmFrameLayout->addStretch(0);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);

  m_slitFileEdit = helperConstructFileEdit(fileFrameLayout, 0, "FWHM (nm)",d->filename, sizeof(d->filename)-1);

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(m_toggleWavelengthStack);

  // initialise
  reset(d);

  mainLayout->addStretch(1);
}

void CWSlitGaussianEdit::reset(const struct slit_gaussian *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);

  m_slitFileEdit->setText(d->filename);

}

void CWSlitGaussianEdit::apply(struct slit_gaussian *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_slitFileEdit->text().toLocal8Bit().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitLorentzEdit::CWSlitLorentzEdit(const struct slit_lorentz *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *fwhmFrameLayout = new QHBoxLayout(fwhmFrame);

  // width

  QLabel *labelWidth = new QLabel("Width (nm)", fwhmFrame);
  labelWidth->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelWidth->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));

  fwhmFrameLayout->addWidget(labelWidth);
  fwhmFrameLayout->addWidget(m_widthEdit, 1);
  fwhmFrameLayout->addStretch(0);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);

  m_slitFileEdit = helperConstructFileEdit(fileFrameLayout, 0, "Width (nm)",d->filename, sizeof(d->filename));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  // order
  QGridLayout *orderLayout = new QGridLayout;

  QLabel *labelOrder = new QLabel("Order", fwhmFrame);
  labelOrder->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelOrder->setMinimumSize(cSuggestedColumnZeroWidth, 0);
  orderLayout->addWidget(labelOrder, 0, 0, Qt::AlignRight);

  m_orderSpin = new QSpinBox(this);
  m_orderSpin->setRange(1,10);
  m_orderSpin->setFixedWidth(cStandardEditWidth);
  orderLayout->addWidget(m_orderSpin, 0, 1, Qt::AlignLeft);

  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addLayout(orderLayout);

  // initialise
  reset(d);

  orderLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  orderLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  orderLayout->setColumnStretch(1, 1);

  mainLayout->addStretch(1);
}

void CWSlitLorentzEdit::reset(const struct slit_lorentz *d)
{
  QString tmpStr;
  m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  m_widthEdit->setText(tmpStr);
  m_orderSpin->setValue(d->order);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_slitFileEdit->setText(d->filename);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitLorentzEdit::apply(struct slit_lorentz *d) const
{
  d->width = m_widthEdit->text().toDouble();
  d->order = m_orderSpin->value();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_slitFileEdit->text().toLocal8Bit().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitVoigtEdit::CWSlitVoigtEdit(const struct slit_voigt *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);

  // left FWHM
  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelFwhm->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_fwhmLeftEdit = new QLineEdit(this);
  m_fwhmLeftEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmLeftEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmLeftEdit));

  QLabel *labelRatio = new QLabel("Lorentz/Gaussian ratio", fwhmFrame);
  labelRatio->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelRatio->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_ratioLeftEdit = new QLineEdit(this);
  m_ratioLeftEdit->setFixedWidth(cStandardEditWidth);
  m_ratioLeftEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 10.0, 3, m_ratioLeftEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmLeftEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelRatio,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_ratioLeftEdit,1, 1,Qt::AlignLeft);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);

  m_fwhmFileEdit = helperConstructFileEdit(fileFrameLayout, 0, "Gaussian FWHM File",d->filename, sizeof(d->filename)-1);
  m_ratioFileEdit = helperConstructFileEdit(fileFrameLayout, 1, "Lorentz/Gaussian ratio",d->filename2, sizeof(d->filename2)-1);

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(m_toggleWavelengthStack);

  // initialise
  reset(d);

  fwhmFrameLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  fwhmFrameLayout->setColumnStretch(1, 1);

  mainLayout->addStretch(1);
}

void CWSlitVoigtEdit::reset(const struct slit_voigt *d)
{
  QString tmpStr;
  m_fwhmLeftEdit->validator()->fixup(tmpStr.setNum(d->fwhmL));
  m_fwhmLeftEdit->setText(tmpStr);
  m_ratioLeftEdit->validator()->fixup(tmpStr.setNum(d->glRatioL));
  m_ratioLeftEdit->setText(tmpStr);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_fwhmFileEdit->setText(d->filename);
  m_ratioFileEdit->setText(d->filename2);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitVoigtEdit::apply(struct slit_voigt *d) const
{
  d->fwhmL = m_fwhmLeftEdit->text().toDouble();
  d->glRatioL = m_ratioLeftEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_fwhmFileEdit->text().toLocal8Bit().data());
  strcpy(d->filename2, m_ratioFileEdit->text().toLocal8Bit().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitErrorEdit::CWSlitErrorEdit(const struct slit_error *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);

  // fwhm

  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelFwhm->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  // width

  QLabel *labelWidth = new QLabel("Boxcar width", fwhmFrame);
  labelWidth->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelWidth->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelWidth,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_widthEdit,1, 1,Qt::AlignLeft);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);

  m_fwhmFileEdit = helperConstructFileEdit(fileFrameLayout, 0, "Gaussian FWHM File",d->filename, sizeof(d->filename)-1);
  m_ratioFileEdit = helperConstructFileEdit(fileFrameLayout, 1, "Boxcar width File",d->filename2, sizeof(d->filename2)-1);

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addStretch(1);

  // initialise
  reset(d);

  fwhmFrameLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  fwhmFrameLayout->setColumnStretch(1, 1);
}

void CWSlitErrorEdit::reset(const struct slit_error *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
  m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  m_widthEdit->setText(tmpStr);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_fwhmFileEdit->setText(d->filename);
  m_ratioFileEdit->setText(d->filename2);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitErrorEdit::apply(struct slit_error *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->width = m_widthEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_fwhmFileEdit->text().toLocal8Bit().data());
  strcpy(d->filename2, m_ratioFileEdit->text().toLocal8Bit().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitApodEdit::CWSlitApodEdit(const struct slit_apod *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // resolution
  gridLayout->addWidget(new QLabel("Resolution (nm)"), row, 0, Qt::AlignRight);
  m_resolutionEdit = new QLineEdit(this);
  m_resolutionEdit->setFixedWidth(cStandardEditWidth);
  m_resolutionEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_resolutionEdit));
  gridLayout->addWidget(m_resolutionEdit, row, 1, Qt::AlignLeft);
  ++row;
  // phase
  gridLayout->addWidget(new QLabel("Phase"), row, 0, Qt::AlignRight);
  m_phaseEdit = new QLineEdit(this);
  m_phaseEdit->setFixedWidth(cStandardEditWidth);
  m_phaseEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 360.0, 3, m_phaseEdit));
  gridLayout->addWidget(m_phaseEdit, row, 1, Qt::AlignLeft);
  ++row;

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

void CWSlitApodEdit::reset(const struct slit_apod *d)
{
  QString tmpStr;
  m_resolutionEdit->validator()->fixup(tmpStr.setNum(d->resolution));
  m_resolutionEdit->setText(tmpStr);
  m_phaseEdit->validator()->fixup(tmpStr.setNum(d->phase));
  m_phaseEdit->setText(tmpStr);
}

void CWSlitApodEdit::apply(struct slit_apod *d) const
{
  d->resolution = m_resolutionEdit->text().toDouble();
  d->phase = m_phaseEdit->text().toDouble();
}

CWSlitAGaussEdit::CWSlitAGaussEdit(const struct slit_agauss *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);

  // fwhm

  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelFwhm->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  QLabel *labelAsym = new QLabel("Asymmetry factor", fwhmFrame);
  labelAsym->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelAsym->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  // asymmetry factor

  m_asymEdit = new QLineEdit(this);
  m_asymEdit->setFixedWidth(cStandardEditWidth);
  m_asymEdit->setValidator(new CDoubleFixedFmtValidator(-50., 50.0, 3, m_asymEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelAsym,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_asymEdit,1, 1,Qt::AlignLeft);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);

  m_fwhmFileEdit = helperConstructFileEdit(fileFrameLayout, 0, "Gaussian FWHM File",d->filename, sizeof(d->filename)-1);
  m_asymFileEdit = helperConstructFileEdit(fileFrameLayout, 1, "Asymmetry factor File",d->filename2, sizeof(d->filename2)-1);

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addStretch(1);

  // initialise
  reset(d);

  fwhmFrameLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  fwhmFrameLayout->setColumnStretch(1, 1);
}

void CWSlitAGaussEdit::reset(const struct slit_agauss *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
  m_asymEdit->validator()->fixup(tmpStr.setNum(d->asym));
  m_asymEdit->setText(tmpStr);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_fwhmFileEdit->setText(d->filename);
  m_asymFileEdit->setText(d->filename2);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitAGaussEdit::apply(struct slit_agauss *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->asym = m_asymEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_fwhmFileEdit->text().toLocal8Bit().data());
  strcpy(d->filename2, m_asymFileEdit->text().toLocal8Bit().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

CWSlitSuperGaussEdit::CWSlitSuperGaussEdit(const struct slit_supergauss *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  mainLayout->addWidget(m_wavelengthDependent, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);

  // fwhm

  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelFwhm->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  // exponential term

  QLabel *labelExp = new QLabel(    "Exponential term", fwhmFrame);
  labelExp->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelExp->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  m_expEdit = new QLineEdit(this);
  m_expEdit->setFixedWidth(cStandardEditWidth);
  m_expEdit->setValidator(new CDoubleFixedFmtValidator(-50., 50.0, 3, m_expEdit));

  // Asymmetry factor

  QLabel *labelAsym = new QLabel("Asymmetry factor", fwhmFrame);
  labelAsym->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  labelAsym->setMinimumSize(cSuggestedColumnZeroWidth, 0);

  // asymmetry factor

  m_asymEdit = new QLineEdit(this);
  m_asymEdit->setFixedWidth(cStandardEditWidth);
  m_asymEdit->setValidator(new CDoubleFixedFmtValidator(-50., 50.0, 3, m_asymEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelExp,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_expEdit,1, 1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelAsym,2,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_asymEdit,2, 1,Qt::AlignLeft);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);

  m_fwhmFileEdit = helperConstructFileEdit(fileFrameLayout, 0, "Gaussian FWHM File",d->filename, sizeof(d->filename)-1);
  m_expFileEdit = helperConstructFileEdit(fileFrameLayout, 1, "Exponential term File",d->filename2, sizeof(d->filename2)-1);
  m_asymFileEdit = helperConstructFileEdit(fileFrameLayout, 2, "Asymmetry factor File",d->filename3, sizeof(d->filename3)-1);

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addStretch(1);

  // initialise
  reset(d);

  fwhmFrameLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  fwhmFrameLayout->setColumnStretch(1, 1);
}

void CWSlitSuperGaussEdit::reset(const struct slit_supergauss *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
  m_expEdit->validator()->fixup(tmpStr.setNum(d->exponential));
  m_expEdit->setText(tmpStr);
  m_asymEdit->validator()->fixup(tmpStr.setNum(d->asym));
  m_asymEdit->setText(tmpStr);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_fwhmFileEdit->setText(d->filename);
  m_expFileEdit->setText(d->filename2);
  m_asymFileEdit->setText(d->filename3);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitSuperGaussEdit::apply(struct slit_supergauss *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->exponential = m_expEdit->text().toDouble();
  d->asym= m_asymEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_fwhmFileEdit->text().toLocal8Bit().data());
  strcpy(d->filename2, m_expFileEdit->text().toLocal8Bit().data());
  strcpy(d->filename3, m_asymFileEdit->text().toLocal8Bit().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------



CWSlitSelector::CWSlitSelector(const mediate_slit_function_t *slit, const QString &title, bool noneFlag,QWidget *parent) :
  QGroupBox(title, parent),
  m_noneFlag(noneFlag)
{

  QGridLayout *mainLayout = new QGridLayout(this);

  // slit type
  m_slitCombo = new QComboBox(this);
  m_slitStack = new QStackedWidget(this);
  // insert widgets into the stack and items into the combo in lock-step.

  if (m_noneFlag)
   {
    m_noneEdit = new CWSlitNoneEdit(&(slit->file));
    m_slitStack->addWidget(m_noneEdit);
    m_slitCombo->addItem("None", QVariant(SLIT_TYPE_NONE));
   }

  m_fileEdit = new CWSlitFileEdit(&(slit->file));
  m_slitStack->addWidget(m_fileEdit);
  m_slitCombo->addItem("File", QVariant(SLIT_TYPE_FILE));

  m_gaussianEdit = new CWSlitGaussianEdit(&(slit->gaussian));
  m_slitStack->addWidget(m_gaussianEdit);
  m_slitCombo->addItem("Gaussian", QVariant(SLIT_TYPE_GAUSS));

  m_lorentzEdit = new CWSlitLorentzEdit(&(slit->lorentz));
  m_slitStack->addWidget(m_lorentzEdit);
  m_slitCombo->addItem("2n-Lorentz", QVariant(SLIT_TYPE_INVPOLY));

  m_voigtEdit = new CWSlitVoigtEdit(&(slit->voigt));
  m_slitStack->addWidget(m_voigtEdit);
  m_slitCombo->addItem("Voigt", QVariant(SLIT_TYPE_VOIGT));

  m_errorEdit = new CWSlitErrorEdit(&(slit->error));
  m_slitStack->addWidget(m_errorEdit);
  m_slitCombo->addItem("Error Function", QVariant(SLIT_TYPE_ERF));

  m_agaussEdit = new CWSlitAGaussEdit(&(slit->agauss));
  m_slitStack->addWidget(m_agaussEdit);
  m_slitCombo->addItem("Asymmetric Gaussian", QVariant(SLIT_TYPE_AGAUSS));

  m_supergaussEdit = new CWSlitSuperGaussEdit(&(slit->supergauss));
  m_slitStack->addWidget(m_supergaussEdit);
  m_slitCombo->addItem("Super Gaussian", QVariant(SLIT_TYPE_SUPERGAUSS));

  m_boxcarApodEdit = new CWSlitApodEdit(&(slit->boxcarapod));
  m_slitStack->addWidget(m_boxcarApodEdit);
  m_slitCombo->addItem("Boxcar (FTS)", QVariant(SLIT_TYPE_APOD));

  m_nbsApodEdit = new CWSlitApodEdit(&(slit->nbsapod));
  m_slitStack->addWidget(m_nbsApodEdit);
  m_slitCombo->addItem("Norton Beer Strong (FTS)", QVariant(SLIT_TYPE_APODNBS));

  mainLayout->addWidget(new QLabel("Slit Function Type", this), 0, 0);
  mainLayout->addWidget(m_slitCombo, 0, 1);

  mainLayout->addWidget(m_slitStack, 1, 0, 1, 2);
  mainLayout->setRowStretch(2, 1);

  // initialize set the current slit - stack will follow
  int index = m_slitCombo->findData(QVariant(slit->type));
  if (index != -1) {
    m_slitCombo->setCurrentIndex(index);
    m_slitStack->setCurrentIndex(index);
  }

  // connections
  connect(m_slitCombo, SIGNAL(currentIndexChanged(int)), m_slitStack, SLOT(setCurrentIndex(int)));
}

void CWSlitSelector::reset(const mediate_slit_function_t *slit)
{
  int index = m_slitCombo->findData(QVariant(slit->type));
  if (index != -1) {
    m_slitCombo->setCurrentIndex(index);
    // stack will follow ...
  }

  if (m_noneFlag)
   m_noneEdit->reset(NULL);

  m_fileEdit->reset(&(slit->file));
  m_gaussianEdit->reset(&(slit->gaussian));
  m_lorentzEdit->reset(&(slit->lorentz));
  m_voigtEdit->reset(&(slit->voigt));
  m_errorEdit->reset(&(slit->error));
  m_agaussEdit->reset(&(slit->agauss));
  m_supergaussEdit->reset(&(slit->supergauss));
  m_boxcarApodEdit->reset(&(slit->boxcarapod));
  m_nbsApodEdit->reset(&(slit->nbsapod));
}

void CWSlitSelector::apply(mediate_slit_function_t *slit) const
{
  // set values for ALL slits ... and the selected slit type

  slit->type = m_slitCombo->itemData(m_slitCombo->currentIndex()).toInt();

  if (m_noneFlag)
   m_noneEdit->apply(NULL);

  m_fileEdit->apply(&(slit->file));
  m_gaussianEdit->apply(&(slit->gaussian));
  m_lorentzEdit->apply(&(slit->lorentz));
  m_voigtEdit->apply(&(slit->voigt));
  m_errorEdit->apply(&(slit->error));
  m_agaussEdit->apply(&(slit->agauss));
  m_supergaussEdit->apply(&(slit->supergauss));
  m_boxcarApodEdit->apply(&(slit->boxcarapod));
  m_nbsApodEdit->apply(&(slit->nbsapod));
}
