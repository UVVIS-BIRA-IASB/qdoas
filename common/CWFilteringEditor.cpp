/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QFontMetrics>

#include "CWFilteringEditor.h"
#include "CValidator.h"

#include "constants.h"

#include "debugutil.h"
#include <stdio.h>

CWFilteringEditor::CWFilteringEditor(const mediate_filter_t *lowpass,
                     const mediate_filter_t *highpass,
                     enum UsageType highPassUsage,
                     QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Low Pass Filter
  QGroupBox *lowGroup = new QGroupBox("Low Pass Filter", this);
  QVBoxLayout *lowLayout = new QVBoxLayout(lowGroup);
  lowLayout->addSpacing(5);
  m_lowCombo = new QComboBox;
  m_lowStack = new QStackedWidget;
  // insert widgets into the stack and items into the combo in lock-step.

  // none
  m_lowStack->addWidget(new QFrame);
  m_lowCombo->addItem("No Filter", QVariant(PRJCT_FILTER_TYPE_NONE));

  // kaiser
  m_lowKaiser = new CWKaiserEdit(&(lowpass->kaiser), CWFilteringEditor::None);
  m_lowStack->addWidget(m_lowKaiser);
  m_lowCombo->addItem("Kaiser Filter", QVariant(PRJCT_FILTER_TYPE_KAISER));

  // boxcar
  m_lowBoxcar = new CWBoxcarTriangularBinomialEdit(&(lowpass->boxcar), CWFilteringEditor::None);
  m_lowStack->addWidget(m_lowBoxcar);
  m_lowCombo->addItem("Boxcar Filter", QVariant(PRJCT_FILTER_TYPE_BOXCAR));

  // gaussian
  m_lowGaussian = new CWGaussianEdit(&(lowpass->gaussian), CWFilteringEditor::None);
  m_lowStack->addWidget(m_lowGaussian);
  m_lowCombo->addItem("Gaussian Filter", QVariant(PRJCT_FILTER_TYPE_GAUSSIAN));

  // triangular
  m_lowTriangular = new CWBoxcarTriangularBinomialEdit(&(lowpass->triangular), CWFilteringEditor::None);
  m_lowStack->addWidget(m_lowTriangular);
  m_lowCombo->addItem("Triangular Filter", QVariant(PRJCT_FILTER_TYPE_TRIANGLE));

  // savitzky-golay
  m_lowSavitzky = new CWSavitzkyGolayEdit(&(lowpass->savitzky), CWFilteringEditor::None);
  m_lowStack->addWidget(m_lowSavitzky);
  m_lowCombo->addItem("Savitzky-Golay Filter", QVariant(PRJCT_FILTER_TYPE_SG));

  // none
  m_lowStack->addWidget(new QFrame);
  m_lowCombo->addItem("Odd-Even Correction", QVariant(PRJCT_FILTER_TYPE_ODDEVEN));

  // binomial
  m_lowBinomial = new CWBoxcarTriangularBinomialEdit(&(lowpass->binomial), CWFilteringEditor::None);
  m_lowStack->addWidget(m_lowBinomial);
  m_lowCombo->addItem("Binomial Filter", QVariant(PRJCT_FILTER_TYPE_BINOMIAL));

  // low pass group organisation
  lowLayout->addWidget(m_lowCombo);
  lowLayout->addWidget(m_lowStack);
  lowLayout->addStretch(1);

  // High Pass Filter
  QGroupBox *highGroup = new QGroupBox("High Pass Filter", this);
  QVBoxLayout *highLayout = new QVBoxLayout(highGroup);
  highLayout->addSpacing(5);
  m_highCombo = new QComboBox;
  m_highStack = new QStackedWidget;
  // insert widgets into the stack and items into the combo in lock-step.

  // none
  m_highStack->addWidget(new QFrame);
  m_highCombo->addItem("No Filter", QVariant(PRJCT_FILTER_TYPE_NONE));

  // kaiser
  m_highKaiser = new CWKaiserEdit(&(highpass->kaiser), highPassUsage);
  m_highStack->addWidget(m_highKaiser);
  m_highCombo->addItem("Kaiser Filter", QVariant(PRJCT_FILTER_TYPE_KAISER));

  // boxcar
  m_highBoxcar = new CWBoxcarTriangularBinomialEdit(&(highpass->boxcar), highPassUsage);
  m_highStack->addWidget(m_highBoxcar);
  m_highCombo->addItem("Boxcar Filter", QVariant(PRJCT_FILTER_TYPE_BOXCAR));

  // gaussian
  m_highGaussian = new CWGaussianEdit(&(highpass->gaussian), highPassUsage);
  m_highStack->addWidget(m_highGaussian);
  m_highCombo->addItem("Gaussian Filter", QVariant(PRJCT_FILTER_TYPE_GAUSSIAN));

  // triangular
  m_highTriangular = new CWBoxcarTriangularBinomialEdit(&(highpass->triangular), highPassUsage);
  m_highStack->addWidget(m_highTriangular);
  m_highCombo->addItem("Triangular Filter", QVariant(PRJCT_FILTER_TYPE_TRIANGLE));

  // savitzky-golay

  m_highSavitzky = new CWSavitzkyGolayEdit(&(highpass->savitzky), highPassUsage);
  m_highStack->addWidget(m_highSavitzky);
  m_highCombo->addItem("Savitzky-Golay Filter", QVariant(PRJCT_FILTER_TYPE_SG));

  // odd-even
  m_highStack->addWidget(new QFrame);
  m_highCombo->addItem("Odd-Even Correction", QVariant(PRJCT_FILTER_TYPE_ODDEVEN));

  // binomial
   m_highBinomial = new CWBoxcarTriangularBinomialEdit(&(highpass->binomial), highPassUsage);
   m_highStack->addWidget(m_highBinomial);
   m_highCombo->addItem("Binomial Filter", QVariant(PRJCT_FILTER_TYPE_BINOMIAL));

  // high pass group organisation
  highLayout->addWidget(m_highCombo);
  highLayout->addWidget(m_highStack);

  // equal space to both groups
  mainLayout->addWidget(lowGroup, 1);
  mainLayout->addWidget(highGroup, 1);
  mainLayout->addStretch(1);

  // connections
  connect(m_lowCombo, SIGNAL(currentIndexChanged(int)), m_lowStack, SLOT(setCurrentIndex(int)));
  connect(m_highCombo, SIGNAL(currentIndexChanged(int)), m_highStack, SLOT(setCurrentIndex(int)));

  // set the current mode - stack will follow
  int index;

  index = m_lowCombo->findData(QVariant(lowpass->mode));
  if (index != -1)
    m_lowCombo->setCurrentIndex(index);

  index = m_highCombo->findData(QVariant(highpass->mode));
  if (index != -1)
    m_highCombo->setCurrentIndex(index);

 }

void CWFilteringEditor::reset(const mediate_filter_t *lowpass, const mediate_filter_t *highpass)
{
  int index;

  index = m_lowCombo->findData(QVariant(lowpass->mode));
  if (index != -1)
    m_lowCombo->setCurrentIndex(index);

  index = m_highCombo->findData(QVariant(highpass->mode));
  if (index != -1)
    m_highCombo->setCurrentIndex(index);

  m_lowKaiser->reset(&(lowpass->kaiser));
  m_highKaiser->reset(&(highpass->kaiser));

  m_lowBoxcar->reset(&(lowpass->boxcar));
  m_highBoxcar->reset(&(highpass->boxcar));

  m_lowGaussian->reset(&(lowpass->gaussian));
  m_highGaussian->reset(&(highpass->gaussian));

  m_lowTriangular->reset(&(lowpass->triangular));
  m_highTriangular->reset(&(highpass->triangular));

   m_lowBinomial->reset(&(lowpass->binomial));
   m_highBinomial->reset(&(highpass->binomial));

  m_lowSavitzky->reset(&(lowpass->savitzky));
  m_highSavitzky->reset(&(highpass->savitzky));
}

void CWFilteringEditor::apply(mediate_filter_t *lowpass, mediate_filter_t *highpass) const
{
  // set values for ALL filters ... and the selected mode

  lowpass->mode = m_lowCombo->itemData(m_lowCombo->currentIndex()).toInt();
  highpass->mode = m_highCombo->itemData(m_highCombo->currentIndex()).toInt();

  m_lowKaiser->apply(&(lowpass->kaiser));
  m_highKaiser->apply(&(highpass->kaiser));

  m_lowBoxcar->apply(&(lowpass->boxcar));
  m_highBoxcar->apply(&(highpass->boxcar));

  m_lowGaussian->apply(&(lowpass->gaussian));
  m_highGaussian->apply(&(highpass->gaussian));

  m_lowTriangular->apply(&(lowpass->triangular));
  m_highTriangular->apply(&(highpass->triangular));

  m_lowBinomial->apply(&(lowpass->binomial));
  m_highBinomial->apply(&(highpass->binomial));

  m_lowSavitzky->apply(&(lowpass->savitzky));
  m_highSavitzky->apply(&(highpass->savitzky));
}


//--------------------------------------------------------
// FilterUsage helper ...

CWFilterUsageEdit::CWFilterUsageEdit(const struct filter_usage *d, CWFilteringEditor::UsageType type,
                     QWidget *parent) :
  QFrame(parent)
{
  m_state = *d; // blot copy state

  // look and function dependes on the UsageType
  switch (type) {
  case CWFilteringEditor::CalFitCheck:
    {
      QHBoxLayout *layout = new QHBoxLayout(this);
      layout->addStretch(1); // push right
      QCheckBox *calCheck = new QCheckBox("Calibration", this);
      layout->addWidget(calCheck);
      QCheckBox *fitCheck = new QCheckBox("Fitting Window", this);
      layout->addWidget(fitCheck);
      calCheck->setCheckState(m_state.calibrationFlag ? Qt::Checked : Qt::Unchecked);
      fitCheck->setCheckState(m_state.fittingFlag ? Qt::Checked : Qt::Unchecked);
      // connections ...
      connect(calCheck, SIGNAL(stateChanged(int)), this, SLOT(slotCalibrationStateChanged(int)));
      connect(fitCheck, SIGNAL(stateChanged(int)), this, SLOT(slotFittingStateChanged(int)));
    }
    break;
  case CWFilteringEditor::SubDivSwitch:
    {
      QHBoxLayout *layout = new QHBoxLayout(this);
      layout->addStretch(1); // push right
      m_subBtn = new QRadioButton("Subtract", this);
      layout->addWidget(m_subBtn);
      m_divBtn = new QRadioButton("Divide", this);
      layout->addWidget(m_divBtn);

      // connections ...
      connect(m_subBtn, SIGNAL(toggled(bool)), this, SLOT(slotSubtractToggled(bool)));
      connect(m_divBtn, SIGNAL(toggled(bool)), this, SLOT(slotDivideToggled(bool)));

      if (d->divide)
       m_divBtn->setChecked(true);
      else
       m_subBtn->setChecked(true);
    }
    break;
  case CWFilteringEditor::None:
    break; // an empty widget ...
  }
}

void CWFilterUsageEdit::reset(const struct filter_usage *d)
{
  slotCalibrationStateChanged(d->calibrationFlag ? Qt::Checked : Qt::Unchecked);
  slotFittingStateChanged(d->fittingFlag ? Qt::Checked : Qt::Unchecked);

  bool checkDiv = (d->divide != 0);

  if (m_state.divide!=d->divide)
   m_divBtn->toggle();

  slotDivideToggled(checkDiv);
  slotSubtractToggled(!checkDiv);
}

void CWFilterUsageEdit::apply(struct filter_usage *d) const
{
  *d = m_state;
}

void CWFilterUsageEdit::slotCalibrationStateChanged(int state)
{
  m_state.calibrationFlag = (state == Qt::Checked) ? 1 : 0;
}

void CWFilterUsageEdit::slotFittingStateChanged(int state)
{
  m_state.fittingFlag = (state == Qt::Checked) ? 1 : 0;
}

void CWFilterUsageEdit::slotSubtractToggled(bool checked)
{
  if (checked)
   m_state.divide = 0;
}

void CWFilterUsageEdit::slotDivideToggled(bool checked)
{
  if (checked)
   m_state.divide = 1;
}


//--------------------------------------------------------
// Specific Filter Editors...

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cMaxIterations            =  20; // and data consistent.
static const Qt::Alignment cLabelAlign     = Qt::AlignRight;

//--------------------------------------------------------

CWKaiserEdit::CWKaiserEdit(const struct filter_kaiser *d, CWFilteringEditor::UsageType type, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("Cut-Off Freq.", this), row, 0, cLabelAlign);

  m_cutoffEdit = new QLineEdit(this);
  m_cutoffEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 100.0, 4, m_cutoffEdit));
  m_cutoffEdit->setFixedWidth(160);

  mainLayout->addWidget(m_cutoffEdit, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Tolerance (dB)", this), row, 0, cLabelAlign);

  m_toleranceEdit = new QLineEdit(this);
  m_toleranceEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 100.0, 4, m_toleranceEdit));
  m_toleranceEdit->setFixedWidth(160);

  mainLayout->addWidget(m_toleranceEdit, row, 1, Qt::AlignLeft);
  ++row;

  // row 0
  row = 0;
  mainLayout->addWidget(new QLabel("Passband", this), row, 2, cLabelAlign);

  m_passbandEdit = new QLineEdit(this);
  m_passbandEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 100.0, 4, m_passbandEdit));
  m_passbandEdit->setFixedWidth(160);

  mainLayout->addWidget(m_passbandEdit, row, 3, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Iterations", this), row, 2, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_iterationsSpinBox, row, 3, Qt::AlignLeft);
  ++row;

  m_usageEdit = new CWFilterUsageEdit(&(d->usage), type, this);
  mainLayout->addWidget(m_usageEdit, row, 0, 1, 4);

  // layout control
  mainLayout->setColumnStretch(1, 1);
  mainLayout->setColumnStretch(3, 1);

  // initialize
  reset(d);
}

void CWKaiserEdit::reset(const struct filter_kaiser *d)
{
  QString tmpStr;

  m_cutoffEdit->validator()->fixup(tmpStr.setNum(d->cutoffFrequency));
  m_cutoffEdit->setText(tmpStr);

  m_toleranceEdit->validator()->fixup(tmpStr.setNum(d->tolerance));
  m_toleranceEdit->setText(tmpStr);

  m_passbandEdit->validator()->fixup(tmpStr.setNum(d->passband));
  m_passbandEdit->setText(tmpStr);

  m_iterationsSpinBox->setValue(d->iterations);

  m_usageEdit->reset(&(d->usage));
}

void CWKaiserEdit::apply(struct filter_kaiser *d) const
{
  bool ok;
  double tmp;

  tmp = m_cutoffEdit->text().toDouble(&ok);
  d->cutoffFrequency = ok ? tmp : 0.0;
  tmp = m_toleranceEdit->text().toDouble(&ok);
  d->tolerance = ok ? tmp : 0.0;
  tmp = m_passbandEdit->text().toDouble(&ok);
  d->passband = ok ? tmp : 0.0;

  d->iterations = m_iterationsSpinBox->value();

  m_usageEdit->apply(&(d->usage));
}

//--------------------------------------------------------

CWBoxcarTriangularBinomialEdit::CWBoxcarTriangularBinomialEdit(const struct filter_boxcar *d, CWFilteringEditor::UsageType type, QWidget *parent) :
  QFrame(parent)
{
  init(d->width, d->iterations, &(d->usage), type);
}

CWBoxcarTriangularBinomialEdit::CWBoxcarTriangularBinomialEdit(const struct filter_triangular *d, CWFilteringEditor::UsageType type, QWidget *parent) :
  QFrame(parent)
{
  init(d->width, d->iterations, &(d->usage), type);
}

CWBoxcarTriangularBinomialEdit::CWBoxcarTriangularBinomialEdit(const struct filter_binomial *d, CWFilteringEditor::UsageType type, QWidget *parent) :
  QFrame(parent)
{
  init(d->width, d->iterations, &(d->usage), type);
}

void CWBoxcarTriangularBinomialEdit::init(int filterWidth, int nIterations, const struct filter_usage *d, CWFilteringEditor::UsageType type)
{
  int row = 0;
  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("Width (pixels)", this), row, 0, cLabelAlign);

  m_widthSpinBox = new QSpinBox(this);
  m_widthSpinBox->setRange(1, 999);
  m_widthSpinBox->setSingleStep(2);
  m_widthSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_widthSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Iterations", this), row, 0, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_iterationsSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  m_usageEdit = new CWFilterUsageEdit(d, type, this);
  mainLayout->addWidget(m_usageEdit, row, 0, 1, 2);

  // layout control
  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnStretch(1, 1);

  // initialize
  m_widthSpinBox->setValue(filterWidth);
  m_iterationsSpinBox->setValue(nIterations);
}

void CWBoxcarTriangularBinomialEdit::reset(const struct filter_boxcar *d)
{
  m_widthSpinBox->setValue(d->width);
  m_iterationsSpinBox->setValue(d->iterations);
  m_usageEdit->reset(&(d->usage));
}

void CWBoxcarTriangularBinomialEdit::apply(struct filter_boxcar *d) const
{
  d->width = m_widthSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
  m_usageEdit->apply(&(d->usage));
}

void CWBoxcarTriangularBinomialEdit::reset(const struct filter_triangular *d)
{
  m_widthSpinBox->setValue(d->width);
  m_iterationsSpinBox->setValue(d->iterations);
  m_usageEdit->reset(&(d->usage));
}

void CWBoxcarTriangularBinomialEdit::apply(struct filter_triangular *d) const
{
  d->width = m_widthSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
  m_usageEdit->apply(&(d->usage));
}

void CWBoxcarTriangularBinomialEdit::reset(const struct filter_binomial *d)
{
  m_widthSpinBox->setValue(d->width);
  m_iterationsSpinBox->setValue(d->iterations);
  m_usageEdit->reset(&(d->usage));
}

void CWBoxcarTriangularBinomialEdit::apply(struct filter_binomial *d) const
{
  d->width = m_widthSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
  m_usageEdit->apply(&(d->usage));
}

//--------------------------------------------------------

CWGaussianEdit::CWGaussianEdit(const struct filter_gaussian *d, CWFilteringEditor::UsageType type, QWidget *parent) :
  QFrame(parent)
{
  QString tmpStr;
  int row = 0;

  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("FWHM (pixels)", this), row, 0, cLabelAlign);

//   m_fwhmEdit = new QLineEdit(this);
//   m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(5, 999.0, 4, m_fwhmEdit));
//   m_fwhmEdit->setFixedWidth(160);

  m_fwhmEdit = new QSpinBox(this);
  m_fwhmEdit->setRange(1, 999);
  m_fwhmEdit->setSingleStep(2);
  m_fwhmEdit->setFixedWidth(160);

  mainLayout->addWidget(m_fwhmEdit, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Iterations", this), row, 0, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_iterationsSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  m_usageEdit = new CWFilterUsageEdit(&(d->usage), type, this);
  mainLayout->addWidget(m_usageEdit, row, 0, 1, 2);

  // layout control
  mainLayout->setColumnStretch(1, 1);

  // initialize
  reset(d);
}

void CWGaussianEdit::reset(const struct filter_gaussian *d)
{
  QString tmpStr;

//   m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
//   m_fwhmEdit->setText(tmpStr);
  m_fwhmEdit->setValue((int)d->fwhm);
  m_iterationsSpinBox->setValue(d->iterations);

  m_usageEdit->reset(&(d->usage));
}

void CWGaussianEdit::apply(struct filter_gaussian *d) const
{
  bool ok;
  double tmp;

  tmp = m_fwhmEdit->text().toDouble(&ok);
  d->fwhm = ok ? tmp : 0;

  d->iterations = m_iterationsSpinBox->value();
  m_usageEdit->apply(&(d->usage));
}

//--------------------------------------------------------

CWSavitzkyGolayEdit::CWSavitzkyGolayEdit(const struct filter_savitzky_golay *d, CWFilteringEditor::UsageType type, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;
  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("Width (pixels)", this), row, 0, cLabelAlign);

  m_widthSpinBox = new QSpinBox(this);
  m_widthSpinBox->setRange(1, 999);
  m_widthSpinBox->setSingleStep(2);
  m_widthSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_widthSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Order", this), row, 0, cLabelAlign);

  m_orderSpinBox = new QSpinBox(this);
  m_orderSpinBox->setRange(2, 10);
  m_orderSpinBox->setSingleStep(2);
  m_orderSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_orderSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // row 2
  mainLayout->addWidget(new QLabel("Iterations", this), row, 0, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(160);

  mainLayout->addWidget(m_iterationsSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  m_usageEdit = new CWFilterUsageEdit(&(d->usage), type, this);
  mainLayout->addWidget(m_usageEdit, row, 0, 1, 2);

  // layout control
  mainLayout->setColumnStretch(1, 1);

  // initialize
  reset(d);
}

void CWSavitzkyGolayEdit::reset(const struct filter_savitzky_golay *d)
{
  m_widthSpinBox->setValue(d->width);
  m_orderSpinBox->setValue(d->order);
  m_iterationsSpinBox->setValue(d->iterations);

  m_usageEdit->reset(&(d->usage));
}

void CWSavitzkyGolayEdit::apply(struct filter_savitzky_golay *d) const
{
  d->width = m_widthSpinBox->value();
  d->order = m_orderSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();

  m_usageEdit->apply(&(d->usage));
}

