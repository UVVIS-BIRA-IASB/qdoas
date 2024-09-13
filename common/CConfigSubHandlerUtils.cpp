
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

using std::map;

//------------------------------------------------------------------------
// handler for <lowpass_filter> and <highpass_filter>

CFilteringSubHandler::CFilteringSubHandler(CConfigHandler *master,
                       mediate_filter_t *filter) :
  CConfigSubHandler(master),
  m_filter(filter)
{
}

void CFilteringSubHandler::start(const map<Glib::ustring, QString>& atts)
{
  // selected filter

  QString str = value(atts, "selected");

  if (str == "none")
    m_filter->mode = PRJCT_FILTER_TYPE_NONE;
  else if (str == "kaiser")
    m_filter->mode = PRJCT_FILTER_TYPE_KAISER;
  else if (str == "boxcar")
    m_filter->mode = PRJCT_FILTER_TYPE_BOXCAR;
  else if (str == "gaussian")
    m_filter->mode = PRJCT_FILTER_TYPE_GAUSSIAN;
  else if (str == "triangular")
    m_filter->mode = PRJCT_FILTER_TYPE_TRIANGLE;
  else if ((str == "savitzky_golay") || (str == "savitzky"))
    m_filter->mode = PRJCT_FILTER_TYPE_SG;
  else if (str == "oddeven")
    m_filter->mode = PRJCT_FILTER_TYPE_ODDEVEN;
  else if (str == "binomial")
    m_filter->mode = PRJCT_FILTER_TYPE_BINOMIAL;
  else
    throw std::runtime_error("Invalid filter method");
}

void CFilteringSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, QString>& atts)
{
  // sub element of lowpass_filter or highpass_filter

  if (element == "kaiser") {

    m_filter->kaiser.cutoffFrequency = value(atts, "cutoff").toDouble();
    m_filter->kaiser.tolerance = value(atts, "tolerance").toDouble();
    m_filter->kaiser.passband = value(atts, "passband").toDouble();
    m_filter->kaiser.iterations = value(atts, "iterations").toInt();
    m_filter->kaiser.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->kaiser.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->kaiser.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "boxcar") {

    m_filter->boxcar.width = value(atts, "width").toInt();
    m_filter->boxcar.iterations = value(atts, "iterations").toInt();
    m_filter->boxcar.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->boxcar.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->boxcar.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "gaussian") {

    m_filter->gaussian.fwhm = value(atts, "fwhm").toDouble();
    m_filter->gaussian.iterations = value(atts, "iterations").toInt();
    m_filter->gaussian.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->gaussian.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->gaussian.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "triangular") {

    m_filter->triangular.width = value(atts, "width").toInt();
    m_filter->triangular.iterations = value(atts, "iterations").toInt();
    m_filter->triangular.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->triangular.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->triangular.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "savitzky_golay") {

    m_filter->savitzky.width = value(atts, "width").toInt();
    m_filter->savitzky.order = value(atts, "order").toInt();
    m_filter->savitzky.iterations = value(atts, "iterations").toInt();
    m_filter->savitzky.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->savitzky.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->savitzky.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "binomial") {

    m_filter->binomial.width = value(atts, "width").toInt();
    m_filter->binomial.iterations = value(atts, "iterations").toInt();
    m_filter->binomial.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->binomial.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->binomial.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
}

//------------------------------------------------------------------------
// handler for <slit_func>

CSlitFunctionSubHandler::CSlitFunctionSubHandler(CConfigHandler *master,
                         mediate_slit_function_t *function) :
  CConfigSubHandler(master),
  m_function(function)
{
}

void CSlitFunctionSubHandler::start(const map<Glib::ustring, QString>& atts)
{
  QString str = value(atts, "type");

  if (str == "none")
    m_function->type = SLIT_TYPE_NONE;
  else if (str == "file")
    m_function->type = SLIT_TYPE_FILE;
  else if (str == "gaussian")
    m_function->type = SLIT_TYPE_GAUSS;
  else if (str == "lorentz")
    m_function->type = SLIT_TYPE_INVPOLY;
  else if (str == "voigt")
    m_function->type = SLIT_TYPE_VOIGT;
  else if (str == "error")
    m_function->type = SLIT_TYPE_ERF;
  else if (str == "agauss")
    m_function->type = SLIT_TYPE_AGAUSS;
  else if (str == "supergauss")
    m_function->type = SLIT_TYPE_SUPERGAUSS;
  else if (str == "boxcarapod")
    m_function->type = SLIT_TYPE_APOD;
  else if (str == "nbsapod")
    m_function->type = SLIT_TYPE_APODNBS;
  else if (str == "gaussianfile")
    m_function->type = SLIT_TYPE_GAUSS;
  else if (str == "lorentzfile")
    m_function->type = SLIT_TYPE_INVPOLY;
  else if (str == "errorfile")
    m_function->type = SLIT_TYPE_ERF;
  else
    throw std::runtime_error("Invalid slit type");

}

void CSlitFunctionSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, QString>& atts)
{
  if (element == "file") {

    QString str = value(atts, "file");
    QString str2 = value(atts, "file2");

    m_function->file.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->file.filename))
    strcpy(m_function->file.filename, str.toLatin1().data());
      else
    throw std::runtime_error("Slit Function Filename too long");
    }

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->file.filename2))
          strcpy(m_function->file.filename2, str2.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->file.filename2 ,"\0");
  }
  else if (element == "gaussian") {

    QString str = value(atts, "file");

    m_function->gaussian.fwhm = value(atts, "fwhm").toDouble();
    m_function->gaussian.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

   if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussian.filename))
          strcpy(m_function->gaussian.filename, str.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->gaussian.filename ,"\0");
  }
  else if (element == "lorentz") {

    QString str = value(atts, "file");

    m_function->lorentz.width = value(atts, "width").toDouble();

    if (value(atts, "order") != "") {
      m_function->lorentz.order = value(atts, "order").toInt();
    } else { // in older versions, 2n-lorentz was specified by the value "2n"
      m_function->lorentz.order = value(atts, "degree").toInt() / 2;
    }

    m_function->lorentz.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;
    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentz.filename))
          strcpy(m_function->lorentz.filename, str.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->lorentz.filename ,"\0");
  }
  else if (element == "voigt") {

    QString str = value(atts, "file");
    QString str2 = value(atts, "file2");

    m_function->voigt.fwhmL = value(atts, "fwhmleft").toDouble();
    m_function->voigt.fwhmR = value(atts, "fwhmright").toDouble();
    m_function->voigt.glRatioL = value(atts, "glrleft").toDouble();
    m_function->voigt.glRatioR = value(atts, "glrright").toDouble();

    m_function->voigt.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->voigt.filename))
          strcpy(m_function->voigt.filename, str.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->voigt.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->voigt.filename2))
          strcpy(m_function->voigt.filename2, str2.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->voigt.filename2 ,"\0");
  }
  else if (element == "error") {

    QString str = value(atts, "file");
    QString str2 = value(atts, "file2");

    m_function->error.fwhm = value(atts, "fwhm").toDouble();
    m_function->error.width = value(atts, "width").toDouble();

    m_function->error.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->error.filename))
          strcpy(m_function->error.filename, str.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->error.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->error.filename2))
          strcpy(m_function->error.filename2, str2.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->error.filename2,"\0");
  }
  else if (element == "agauss") {

    QString str = value(atts, "file");
    QString str2 = value(atts, "file2");

    m_function->agauss.fwhm = value(atts, "fwhm").toDouble();
    m_function->agauss.asym = value(atts, "asym").toDouble();

    m_function->agauss.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->agauss.filename))
          strcpy(m_function->agauss.filename, str.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->agauss.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->agauss.filename2))
          strcpy(m_function->agauss.filename2, str2.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->agauss.filename2 ,"\0");
  }

  else if (element == "supergauss") {

    QString str = value(atts, "file");
    QString str2 = value(atts, "file2");
    QString str3 = value(atts, "file3");

    m_function->supergauss.fwhm = value(atts, "fwhm").toDouble();
    m_function->supergauss.exponential = value(atts, "expTerm").toDouble();
    m_function->supergauss.asym = value(atts, "asym").toDouble();

    m_function->supergauss.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->supergauss.filename))
          strcpy(m_function->supergauss.filename, str.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename (FWHM) too long");
        }
       else
        strcpy(m_function->supergauss.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->supergauss.filename2))
          strcpy(m_function->supergauss.filename2, str2.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename (Exponential term) too long");
        }
       else
        strcpy(m_function->supergauss.filename2 ,"\0");

    if (!str3.isEmpty())
     {
      str3 = m_master->pathExpand(str3);
      if (str3.length() < (int)sizeof(m_function->supergauss.filename3))
          strcpy(m_function->supergauss.filename3, str3.toLatin1().data());
      else
          throw std::runtime_error("Slit Function Filename (Asymmetry factor) too long");
        }
       else
        strcpy(m_function->supergauss.filename3 ,"\0");
  }
  else if (element == "boxcarapod") {

    m_function->boxcarapod.resolution = value(atts, "resolution").toDouble();
    m_function->boxcarapod.phase = value(atts, "phase").toDouble();
  }
  else if (element == "nbsapod") {

    m_function->nbsapod.resolution = value(atts, "resolution").toDouble();
    m_function->nbsapod.phase = value(atts, "phase").toDouble();
  }
  else if (element == "gaussianfile") {

    QString str = value(atts, "file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussian.filename))
       {
           strcpy(m_function->gaussian.filename, str.toLatin1().data());
           m_function->gaussian.wveDptFlag=1;
          }
      else
    throw std::runtime_error("Slit Function Filename too long");
    }
  }
  else if (element == "lorentzfile") {

    QString str = value(atts, "file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentz.filename))
       {
           strcpy(m_function->lorentz.filename, str.toLatin1().data());
           m_function->lorentz.wveDptFlag=1;
          }

      else
    throw std::runtime_error("Slit Function Filename too long");
    }

    if (value(atts, "order") != "") {
      m_function->lorentz.order = value(atts, "order").toInt();
    } else {
      m_function->lorentz.order = value(atts, "degree").toInt() / 2;
    }
  }
  else if (element == "errorfile") {

    QString str = value(atts, "file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->error.filename))
       {
           m_function->error.wveDptFlag=1;
           strcpy(m_function->error.filename, str.toLatin1().data());
          }
      else
    throw std::runtime_error("Slit Function Filename too long");
    }

    QString str2 = value(atts, "file2");
    if (!str2.isEmpty()) {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->error.filename2))
    strcpy(m_function->error.filename2, str2.toLatin1().data());
      else
    throw std::runtime_error("Slit Function Filename too long");
    }
   else
    strcpy(m_function->error.filename2, str.toLatin1().data());
  }

}

