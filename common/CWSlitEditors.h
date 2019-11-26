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


#ifndef _CWSLITEDITORS_H_GUARD
#define _CWSLITEDITORS_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>

#include "mediate_general.h"

//--------------------------------------------------------------------------
// base for specifc editor that require a slit function file

class CWSlitFileBase : public QFrame
{
Q_OBJECT
 public:
  CWSlitFileBase(QWidget *parent = 0);

  virtual void toggleVisible(int state);

 protected:
  QLineEdit *helperConstructFileEdit(QGridLayout *gridLayout, int row, const char *label,const char *str, int len);

 public slots:
  void slotBrowseFile();
  void slotToggleWavelength(int);

 protected:
  QCheckBox *m_wavelengthDependent;
  QStackedLayout *m_toggleWavelengthStack;
};

//--------------------------------------------------------------------------

class CWSlitNoneEdit : public CWSlitFileBase
{
 public:
  CWSlitNoneEdit(const struct slit_file *d, QWidget *parent = 0);

  void reset(const struct slit_file *d);
  void apply(struct slit_file *d) const;

};

class CWSlitFileEdit : public CWSlitFileBase
{
 public:
  CWSlitFileEdit(const struct slit_file *d, QWidget *parent = 0);

  void reset(const struct slit_file *d);
  void apply(struct slit_file *d) const;

  virtual void toggleVisible(int state);

private:
  QLineEdit *m_slitFileEdit, *m_stretchEdit;
  QGridLayout *m_wvlDependentGrid;
};

class CWSlitGaussianEdit : public CWSlitFileBase
{
 public:
  CWSlitGaussianEdit(const struct slit_gaussian *d, QWidget *parent = 0);

  void reset(const struct slit_gaussian *d);
  void apply(struct slit_gaussian *d) const;

 private:
  QLineEdit *m_slitFileEdit, *m_fwhmEdit;
};

class CWSlitLorentzEdit : public CWSlitFileBase
{
 public:
  CWSlitLorentzEdit(const struct slit_lorentz *d, QWidget *parent = 0);

  void reset(const struct slit_lorentz *d);
  void apply(struct slit_lorentz *d) const;

 private:
  QLineEdit *m_slitFileEdit, *m_widthEdit;
  QSpinBox *m_orderSpin;
};

class CWSlitVoigtEdit : public CWSlitFileBase
{
 public:
  CWSlitVoigtEdit(const struct slit_voigt *d, QWidget *parent = 0);

  void reset(const struct slit_voigt *d);
  void apply(struct slit_voigt *d) const;

 private:
  QLineEdit *m_fwhmFileEdit, *m_ratioFileEdit;
  QLineEdit *m_fwhmLeftEdit, *m_fwhmRightEdit;
  QLineEdit *m_ratioLeftEdit, *m_ratioRightEdit;
};

class CWSlitErrorEdit : public CWSlitFileBase
{
 public:
  CWSlitErrorEdit(const struct slit_error *d, QWidget *parent = 0);

  void reset(const struct slit_error *d);
  void apply(struct slit_error *d) const;

 private:
  QLineEdit *m_fwhmFileEdit, *m_ratioFileEdit;
  QLineEdit *m_fwhmEdit;
  QLineEdit *m_widthEdit;
};

class CWSlitAGaussEdit : public CWSlitFileBase
{
 public:
  CWSlitAGaussEdit(const struct slit_agauss *d, QWidget *parent = 0);

  void reset(const struct slit_agauss *d);
  void apply(struct slit_agauss *d) const;

 private:
  QLineEdit *m_fwhmFileEdit, *m_asymFileEdit;
  QLineEdit *m_fwhmEdit;
  QLineEdit *m_asymEdit;
};

class CWSlitSuperGaussEdit : public CWSlitFileBase
{
 public:
  CWSlitSuperGaussEdit(const struct slit_supergauss *d, QWidget *parent = 0);

  void reset(const struct slit_supergauss *d);
  void apply(struct slit_supergauss *d) const;

 private:
  QLineEdit *m_fwhmFileEdit, *m_expFileEdit,*m_asymFileEdit;
  QLineEdit *m_fwhmEdit;
  QLineEdit *m_expEdit;
  QLineEdit *m_asymEdit;
};

class CWSlitApodEdit : public QFrame
{
 public:
  CWSlitApodEdit(const struct slit_apod *d, QWidget *parent = 0);

  void reset(const struct slit_apod *d);
  void apply(struct slit_apod *d) const;

 private:
  QLineEdit *m_resolutionEdit;
  QLineEdit *m_phaseEdit;
};

//-----------------------------------------------

class CWSlitSelector : public QGroupBox
{
 public:
  CWSlitSelector(const mediate_slit_function_t *slit, const QString &title, bool noneFlag,QWidget *parent = 0);

  void reset(const mediate_slit_function_t *slit);
  void apply(mediate_slit_function_t *slit) const;

 private:

  bool m_noneFlag;

  QComboBox *m_slitCombo;
  QStackedWidget *m_slitStack;
  // widgets for the configuration of each slit
  CWSlitNoneEdit *m_noneEdit;
  CWSlitFileEdit *m_fileEdit;
  CWSlitGaussianEdit *m_gaussianEdit;
  CWSlitLorentzEdit *m_lorentzEdit;
  CWSlitVoigtEdit *m_voigtEdit;
  CWSlitErrorEdit *m_errorEdit;
  CWSlitAGaussEdit *m_agaussEdit;
  CWSlitSuperGaussEdit *m_supergaussEdit;
  CWSlitApodEdit *m_boxcarApodEdit, *m_nbsApodEdit;
};

#endif
