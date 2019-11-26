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

#ifndef _CWFILTERINGEDITOR_H_GUARD
#define _CWFILTERINGEDITOR_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>

#include "mediate_general.h"

class CWKaiserEdit;
class CWBoxcarTriangularBinomialEdit;
class CWGaussianEdit;
class CWSavitzkyGolayEdit;

//--------------------------------------------------------------------------
class CWFilteringEditor : public QFrame
{
 public:
  enum UsageType { None, CalFitCheck, SubDivSwitch };

  CWFilteringEditor(const mediate_filter_t *lowpass,
		    const mediate_filter_t *highpass,
		    enum UsageType highPassUsage,
		    QWidget *parent = 0);

  void reset(const mediate_filter_t *lowpass, const mediate_filter_t *highpass);
  void apply(mediate_filter_t *lowpass, mediate_filter_t *highpass) const;

 private:
  QComboBox *m_lowCombo, *m_highCombo;
  QStackedWidget *m_lowStack, *m_highStack;
  // widgets for the configuration of each filter (low and high).

  CWKaiserEdit *m_lowKaiser, *m_highKaiser;
  CWBoxcarTriangularBinomialEdit *m_lowBoxcar, *m_highBoxcar;
  CWGaussianEdit *m_lowGaussian, *m_highGaussian;
  CWBoxcarTriangularBinomialEdit *m_lowTriangular, *m_highTriangular;
  CWSavitzkyGolayEdit *m_lowSavitzky, *m_highSavitzky;
  CWBoxcarTriangularBinomialEdit *m_lowBinomial, *m_highBinomial;
};

//--------------------------------------------------------------------------

class CWFilterUsageEdit : public QFrame
{
Q_OBJECT
 public:
  CWFilterUsageEdit(const struct filter_usage *d, CWFilteringEditor::UsageType type,
		  QWidget *parent = 0);

  void reset(const struct filter_usage *d);
  void apply(struct filter_usage *d) const;

  public slots:
    void slotCalibrationStateChanged(int state);
    void slotFittingStateChanged(int state);
    void slotSubtractToggled(bool checked);
    void slotDivideToggled(bool checked);

 private:
    struct filter_usage m_state;
    QRadioButton *m_subBtn;
    QRadioButton *m_divBtn;
};

//--------------------------------------------------------------------------

class CWKaiserEdit : public QFrame
{
 public:
  CWKaiserEdit(const struct filter_kaiser *d, CWFilteringEditor::UsageType type,
	       QWidget *parent = 0);

  void reset(const struct filter_kaiser *d);
  void apply(struct filter_kaiser *d) const;

 private:
  QLineEdit *m_cutoffEdit, *m_toleranceEdit, *m_passbandEdit;
  QSpinBox *m_iterationsSpinBox;
  CWFilterUsageEdit *m_usageEdit;
};

//--------------------------------------------------------------------------

class CWBoxcarTriangularBinomialEdit : public QFrame
{
 public:
  CWBoxcarTriangularBinomialEdit(const struct filter_boxcar *d, CWFilteringEditor::UsageType type,
				 QWidget *parent = 0);
  CWBoxcarTriangularBinomialEdit(const struct filter_triangular *d, CWFilteringEditor::UsageType type,
				 QWidget *parent = 0);
  CWBoxcarTriangularBinomialEdit(const struct filter_binomial *d, CWFilteringEditor::UsageType type,
				 QWidget *parent = 0);

  void reset(const struct filter_boxcar *d);
  void apply(struct filter_boxcar *d) const;
  void reset(const struct filter_triangular *d);
  void apply(struct filter_triangular *d) const;
  void reset(const struct filter_binomial *d);
  void apply(struct filter_binomial *d) const;

 protected:
  void init(int filterWidth, int nIterations, const struct filter_usage *d, CWFilteringEditor::UsageType type);

 private:
  QSpinBox *m_widthSpinBox;
  QSpinBox *m_iterationsSpinBox;
  CWFilterUsageEdit *m_usageEdit;
};

//--------------------------------------------------------------------------

class CWGaussianEdit : public QFrame
{
 public:
  CWGaussianEdit(const struct filter_gaussian *d, CWFilteringEditor::UsageType type,
		 QWidget *parent = 0);

  void reset(const struct filter_gaussian *d);
  void apply(struct filter_gaussian *d) const;

 private:
  QSpinBox *m_fwhmEdit;
  QSpinBox *m_iterationsSpinBox;
  CWFilterUsageEdit *m_usageEdit;
};

//--------------------------------------------------------------------------

class CWSavitzkyGolayEdit : public QFrame
{
 public:
  CWSavitzkyGolayEdit(const struct filter_savitzky_golay *d, CWFilteringEditor::UsageType type,
		      QWidget *parent = 0);

  void reset(const struct filter_savitzky_golay *d);
  void apply(struct filter_savitzky_golay *d) const;

 private:
  QSpinBox *m_widthSpinBox;
  QSpinBox *m_orderSpinBox;
  QSpinBox *m_iterationsSpinBox;
  CWFilterUsageEdit *m_usageEdit;
};

#endif

