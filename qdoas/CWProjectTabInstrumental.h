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


#ifndef _CWPROJECTTABINSTRUMENTAL_H_GUARD
#define _CWPROJECTTABINSTRUMENTAL_H_GUARD

#include <map>

#include <QFrame>
#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QGridLayout>

#include "mediate_project.h"

class CWInstrAsciiEdit;
#ifdef PRJCT_INSTR_FORMAT_OLD
class CWInstrLoggerEdit;
class CWInstrActonEdit;
#endif
class CWInstrSaozEdit;
class CWInstrMinimumEdit;
#ifdef PRJCT_INSTR_FORMAT_OLD
class CWInstrCcdEdit;
#endif
class CWInstrCcdEevEdit;
class CWInstrGdpEdit;
class CWInstrGome1Edit;
class CWInstrGome2Edit;
class CWInstrSciaEdit;
class CWInstrOmiEdit;
class CWInstrOmpsEdit;
class CWInstrTropomiEdit;
class CWInstrApexEdit;
class CWInstrMfcEdit;
class CWInstrMfcStdEdit;
class CWInstrMfcbiraEdit;
class CWInstrOceanOpticsEdit;
class CWInstrFrm4doasEdit;
class CWInstrAvantesEdit;
class CWInstrApexEdit;
class CWInstrGemsEdit;

//--------------------------------------------------------------------------
class CWProjectTabInstrumental : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabInstrumental(const mediate_project_instrumental_t *instr, QWidget *parent = 0);

  void apply(mediate_project_instrumental_t *instr) const;

 public slots:
  void slotInstrumentChanged(int instrument);
  void slotInstrumentTypeChanged(int instrumentType);

 private:
  QComboBox *m_siteCombo;
  QRadioButton *m_saaSRadioButton,*m_saaNRadioButton;
  QFrame *m_siteFrame;
  QStackedWidget *m_formatStack;
  // widgets for the configuration of each instrument file format
  CWInstrAsciiEdit *m_asciiEdit;
  #ifdef PRJCT_INSTR_FORMAT_OLD
  CWInstrLoggerEdit *m_loggerEdit;
  CWInstrActonEdit *m_actonEdit;
  CWInstrLoggerEdit *m_pdaEggEdit;
  CWInstrLoggerEdit *m_pdaEggOldEdit;
  CWInstrCcdEdit *m_ccdOhp96Edit;
  CWInstrCcdEdit *m_ccdHa94Edit;
  #endif
  CWInstrSaozEdit *m_saozVisEdit;
  CWInstrSaozEdit *m_saozUvEdit;
  CWInstrMfcEdit *m_mfcEdit;
  CWInstrMfcStdEdit *m_mfcStdEdit;
  CWInstrMfcbiraEdit *m_mfcbiraEdit;
  CWInstrMinimumEdit *m_saozEfmEdit;
  #ifdef PRJCT_INSTR_FORMAT_OLD
  CWInstrMinimumEdit *m_rasasEdit;
  CWInstrMinimumEdit *m_pdasiEasoeEdit;
  #endif
  CWInstrCcdEevEdit *m_ccdEevEdit;
  CWInstrGome1Edit *m_gdpNetcdfEdit;
  CWInstrGdpEdit *m_gdpBinEdit;
  CWInstrSciaEdit *m_sciaHdfEdit;
  CWInstrSciaEdit *m_sciaPdsEdit;
  CWInstrMinimumEdit *m_uoftEdit;
  CWInstrMinimumEdit *m_noaaEdit;
  CWInstrOmiEdit *m_omiEdit;
  CWInstrOmpsEdit *m_ompsEdit;
  CWInstrTropomiEdit *m_tropomiEdit;
  CWInstrApexEdit *m_apexEdit;
  CWInstrGemsEdit *m_gemsEdit;
  CWInstrGome2Edit *m_gome2Edit;
  CWInstrMinimumEdit *m_mkzyEdit;
  CWInstrOceanOpticsEdit *m_oceanOpticsEdit;
  CWInstrFrm4doasEdit *m_frm4doasEdit;
  CWInstrAvantesEdit *m_biraairborneEdit;
  CWInstrAvantesEdit *m_biramobileEdit;
  std::map<int,int> m_instrumentToStackIndexMap;
};

//--------------------------------------------------------------------------

// base class for editors with calibration and instrument files. Only provides
// the implementation for the browse slots. Instanciation of the protected
// member widgets is the responsibility of concrete derived classes.

class CWCalibInstrEdit : public QFrame
{
Q_OBJECT
 public:
  CWCalibInstrEdit(QWidget *parent = 0);

 protected:
  void helperConstructFileWidget(QLineEdit **fileEdit, QGridLayout *gridLayout, int &row,
                 const char *str, int len,
                 const char *label, const char *slot);

  void helperConstructCalInsFileWidgets(QGridLayout *gridLayout, int &row,
                    const char *calib, int lenCalib,
                    const char *instr, int lenInstr);

 public slots:
  void slotCalibOneBrowse();
  void slotInstrTwoBrowse();
  void slotOffsetTwoBrowse();

 protected:
  QLineEdit *m_fileOneEdit, *m_fileTwoEdit;
};

class StrayLightConfig : public QGroupBox {
  Q_OBJECT
  public:
  StrayLightConfig(Qt::Orientation orientation, QWidget *parent = 0);
  void setLambdaMin(double lambda);
  void setLambdaMax(double lambda);
  double getLambdaMin() const;
  double getLambdaMax() const;

  private:
  QLineEdit *m_lambdaMinEdit;
  QLineEdit *m_lambdaMaxEdit;
};

//--------------------------------------------------------------------------

// base class for editors with all instrument files. Only provides
// the implementation for the browse slots.Instanciation of the protected
// member widgets is the responsibility of concrete derived classes.
// A helper function can be used for the construction of these widgets.

class CWAllFilesEdit : public CWCalibInstrEdit
{
Q_OBJECT
 public:
  CWAllFilesEdit(QWidget *parent = 0);

 protected:
  void helperConstructIpvDnlFileWidgets(QGridLayout *gridLayout, int &row,
                    const char *ipv, int lenIpv,
                    const char *dnl, int lenDnl);

  void helperConstructFileWidgets(QGridLayout *gridLayout, int &row,
                  const char *calib, int lenCalib,
                  const char *instr, int lenInstr,
                  const char *ipv, int lenIpv,
                  const char *dnl, int lenDnl);

 public slots:
  void slotInterPixelVariabilityThreeBrowse();
  void slotDarkCurrentThreeBrowse();
  void slotStraylightCorrectionThreeBrowse();
  void slotDetectorNonLinearityFourBrowse();
  void slotOffsetFourBrowse();
  void slotImagePathFiveBrowse();

 protected:
  QLineEdit *m_fileThreeEdit, *m_fileFourEdit, *m_fileFiveEdit;
};

//--------------------------------------------------------------------------
class CWInstrAsciiEdit : public CWCalibInstrEdit
{
 Q_OBJECT
public:
  CWInstrAsciiEdit(const struct instrumental_ascii *d, QWidget *parent = 0);
  void setflagsEnabled(bool enableFlag);
  void apply(struct instrumental_ascii *d) const;

public slots :

  void slotAsciiFormatChanged (bool state);
  void slotAsciiExtendedFormatChanged(bool state);

private:
  QGroupBox *m_flagsGroup;
  QLineEdit *m_detSizeEdit;
  QRadioButton *m_lineRadioButton, *m_columnRadioButton,*m_columnExtendedRadioButton;
  QCheckBox *m_zenCheck, *m_aziCheck, *m_eleCheck, *m_dateCheck, *m_timeCheck, *m_lambdaCheck;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------
#ifdef PRJCT_INSTR_FORMAT_OLD
class CWInstrLoggerEdit : public CWCalibInstrEdit
{
 public:
  CWInstrLoggerEdit(const struct instrumental_logger *d, QWidget *parent = 0);

  void apply(struct instrumental_logger *d) const;

 private:
  QComboBox *m_spectralTypeCombo;
  QCheckBox *m_aziCheck;
};

//--------------------------------------------------------------------------

class CWInstrActonEdit : public CWCalibInstrEdit
{
 public:
  CWInstrActonEdit(const struct instrumental_acton *d, QWidget *parent = 0);

  void apply(struct instrumental_acton *d) const;

 private:
  QComboBox *m_niluTypeCombo;
};
#endif
//--------------------------------------------------------------------------

class CWInstrSaozEdit : public CWCalibInstrEdit
{
 public:
  CWInstrSaozEdit(const struct instrumental_saoz *d, QWidget *parent = 0);

  void apply(struct instrumental_saoz *d) const;

 private:
  QComboBox *m_spectralTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrMfcEdit : public CWAllFilesEdit
{
 public:
  CWInstrMfcEdit(const struct instrumental_mfc *d, QWidget *parent = 0);

  void apply(struct instrumental_mfc *d) const;

 private:
  QLineEdit *m_detSizeEdit, *m_firstLambdaEdit;
  QLineEdit *m_offsetMaskEdit, *m_instrMaskEdit, *m_darkMaskEdit, *m_spectraMaskEdit;
  QCheckBox *m_revertCheck, *m_autoCheck;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrMfcStdEdit : public CWAllFilesEdit
{
 public:
  CWInstrMfcStdEdit(const struct instrumental_mfcstd *d, QWidget *parent = 0);

  void apply(struct instrumental_mfcstd *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  QLineEdit *m_dateFormatEdit;
  QCheckBox *m_revertCheck;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrMfcbiraEdit : public CWAllFilesEdit
{
 public:

  CWInstrMfcbiraEdit(const struct instrumental_mfcbira *d, QWidget *parent = 0);

  void apply(struct instrumental_mfcbira *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrMinimumEdit : public CWCalibInstrEdit
{
 public:
  CWInstrMinimumEdit(const struct instrumental_minimum *d, QWidget *parent = 0);

  void apply(struct instrumental_minimum *d) const;

 private:
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------
#ifdef PRJCT_INSTR_FORMAT_OLD
class CWInstrCcdEdit : public CWAllFilesEdit
{
 public:
  CWInstrCcdEdit(const struct instrumental_ccd *d, QWidget *parent = 0);

  void apply(struct instrumental_ccd *d) const;
};
#endif
//--------------------------------------------------------------------------

class CWInstrCcdEevEdit : public CWAllFilesEdit
{
 public:
  CWInstrCcdEevEdit(const struct instrumental_ccdeev *d, QWidget *parent = 0);

  void apply(struct instrumental_ccdeev *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  QComboBox *m_spectralTypeCombo;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrGdpEdit : public CWCalibInstrEdit
{
 public:
  CWInstrGdpEdit(const struct instrumental_gdp *d, QWidget *parent = 0);

  void apply(struct instrumental_gdp *d) const;

 private:
  QComboBox *m_bandTypeCombo;
  QComboBox *m_pixelTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrGome1Edit : public CWCalibInstrEdit
{
 public:
  CWInstrGome1Edit(const struct instrumental_gdp *d, QWidget *parent = 0);

  void apply(struct instrumental_gdp *d) const;

 private:
  QComboBox *m_bandTypeCombo;
  QComboBox *m_pixelTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrGome2Edit : public CWCalibInstrEdit
{
 public:
  CWInstrGome2Edit(const struct instrumental_gome2 *d, QWidget *parent = 0);

  void apply(struct instrumental_gome2 *d) const;

 private:
  QComboBox *m_bandTypeCombo;
  QComboBox *m_pixelTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrSciaEdit : public CWAllFilesEdit
{
Q_OBJECT
 public:
  CWInstrSciaEdit(const struct instrumental_scia *d, QWidget *parent = 0);

  void apply(struct instrumental_scia *d) const;

 public slots:
  void slotChannelChanged(int index);
  void slotCluster0Changed(int state);
  void slotCluster1Changed(int state);
  void slotCluster2Changed(int state);
  void slotCluster3Changed(int state);
  void slotCluster4Changed(int state);
  void slotCluster5Changed(int state);

 private:
  QComboBox *m_channelCombo;
  QCheckBox *m_clusterCheck[6];
  QLineEdit *m_referenceEdit;
  unsigned char m_clusterState[32];
  int m_clusterOffset; // maps check box to cluster index
};


//--------------------------------------------------------------------------

class CWInstrOmiEdit : public CWCalibInstrEdit
{
 public:
  CWInstrOmiEdit(const struct instrumental_omi *d, QWidget *parent = 0);

  void apply(struct instrumental_omi *d) const;

 private:
  QComboBox *m_spectralTypeCombo;
  QLineEdit *m_minLambdaEdit, *m_maxLambdaEdit,*m_trackSelection;
  QCheckBox *m_averageCheck;
  QGroupBox *m_pixelQFGroup;
  QGroupBox *m_xtrackQFBox;
  QLineEdit *m_pixelQFMaskEdit;
  QLineEdit *m_pixelQFMaxGapsEdit;
  QRadioButton *m_nonstrictXTrackQF;
  QRadioButton *m_strictXTrackQF;
};

//--------------------------------------------------------------------------

class CWInstrOmpsEdit : public CWCalibInstrEdit
{
public:
  CWInstrOmpsEdit(QWidget *parent = 0);
  void apply(void);
};
//--------------------------------------------------------------------------

class CWInstrTropomiEdit : public CWCalibInstrEdit
{
  Q_OBJECT
 public:
  CWInstrTropomiEdit(const struct instrumental_tropomi *d, QWidget *parent = 0);

  void apply(struct instrumental_tropomi *d) const;

  public slots:
    void slot_browse_reference_directory();

 private:
  QComboBox *m_spectralBandCombo;
  QLineEdit *m_reference_directory_edit,*m_trackSelection;
};

//--------------------------------------------------------------------------

class CWInstrApexEdit : public CWCalibInstrEdit
{
  Q_OBJECT
 public:
  CWInstrApexEdit(const struct instrumental_apex *d, QWidget *parent = 0);

  void apply(struct instrumental_apex *d) const;

 private:
  QLineEdit *m_trackSelection;
};

//--------------------------------------------------------------------------

class CWInstrOceanOpticsEdit : public CWCalibInstrEdit
{
 public:
  CWInstrOceanOpticsEdit(const struct instrumental_oceanoptics *d, QWidget *parent = 0);

  void apply(struct instrumental_oceanoptics *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrFrm4doasEdit : public CWAllFilesEdit
{
 public:

  CWInstrFrm4doasEdit(const struct instrumental_frm4doas *d, QWidget *parent = 0);

  void apply(struct instrumental_frm4doas *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  QComboBox *m_spectralTypeCombo;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrAvantesEdit : public CWAllFilesEdit
{
 public:

  CWInstrAvantesEdit(const struct instrumental_avantes *d, QWidget *parent = 0);

  void apply(struct instrumental_avantes *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  StrayLightConfig *m_strayLightConfig;
};

//--------------------------------------------------------------------------

class CWInstrGemsEdit : public CWCalibInstrEdit
{
  Q_OBJECT
 public:
  CWInstrGemsEdit(const struct instrumental_gems *d, QWidget *parent = 0);

  void apply(struct instrumental_gems *d) const;

 private:
  QLineEdit *m_trackSelection;
  QSpinBox *m_binning;
};

//--------------------------------------------------------------------------

#endif

