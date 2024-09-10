
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

  QString str = atts.at("selected");

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

    m_filter->kaiser.cutoffFrequency = atts.at("cutoff").toDouble();
    m_filter->kaiser.tolerance = atts.at("tolerance").toDouble();
    m_filter->kaiser.passband = atts.at("passband").toDouble();
    m_filter->kaiser.iterations = atts.at("iterations").toInt();
    m_filter->kaiser.usage.calibrationFlag = (atts.at("cal") == "true") ? 1 : 0;
    m_filter->kaiser.usage.fittingFlag = (atts.at("fit") == "true") ? 1 : 0;
    m_filter->kaiser.usage.divide = (atts.at("div") == "true") ? 1 : 0;
  }
  else if (element == "boxcar") {

    m_filter->boxcar.width = atts.at("width").toInt();
    m_filter->boxcar.iterations = atts.at("iterations").toInt();
    m_filter->boxcar.usage.calibrationFlag = (atts.at("cal") == "true") ? 1 : 0;
    m_filter->boxcar.usage.fittingFlag = (atts.at("fit") == "true") ? 1 : 0;
    m_filter->boxcar.usage.divide = (atts.at("div") == "true") ? 1 : 0;
  }
  else if (element == "gaussian") {

    m_filter->gaussian.fwhm = atts.at("fwhm").toDouble();
    m_filter->gaussian.iterations = atts.at("iterations").toInt();
    m_filter->gaussian.usage.calibrationFlag = (atts.at("cal") == "true") ? 1 : 0;
    m_filter->gaussian.usage.fittingFlag = (atts.at("fit") == "true") ? 1 : 0;
    m_filter->gaussian.usage.divide = (atts.at("div") == "true") ? 1 : 0;
  }
  else if (element == "triangular") {

    m_filter->triangular.width = atts.at("width").toInt();
    m_filter->triangular.iterations = atts.at("iterations").toInt();
    m_filter->triangular.usage.calibrationFlag = (atts.at("cal") == "true") ? 1 : 0;
    m_filter->triangular.usage.fittingFlag = (atts.at("fit") == "true") ? 1 : 0;
    m_filter->triangular.usage.divide = (atts.at("div") == "true") ? 1 : 0;
  }
  else if (element == "savitzky_golay") {

    m_filter->savitzky.width = atts.at("width").toInt();
    m_filter->savitzky.order = atts.at("order").toInt();
    m_filter->savitzky.iterations = atts.at("iterations").toInt();
    m_filter->savitzky.usage.calibrationFlag = (atts.at("cal") == "true") ? 1 : 0;
    m_filter->savitzky.usage.fittingFlag = (atts.at("fit") == "true") ? 1 : 0;
    m_filter->savitzky.usage.divide = (atts.at("div") == "true") ? 1 : 0;
  }
  else if (element == "binomial") {

    m_filter->binomial.width = atts.at("width").toInt();
    m_filter->binomial.iterations = atts.at("iterations").toInt();
    m_filter->binomial.usage.calibrationFlag = (atts.at("cal") == "true") ? 1 : 0;
    m_filter->binomial.usage.fittingFlag = (atts.at("fit") == "true") ? 1 : 0;
    m_filter->binomial.usage.divide = (atts.at("div") == "true") ? 1 : 0;
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
  QString str = atts.at("type");

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

    QString str = atts.at("file");
    QString str2 = atts.at("file2");

    m_function->file.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;

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

    QString str = atts.at("file");

    m_function->gaussian.fwhm = atts.at("fwhm").toDouble();
    m_function->gaussian.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;

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

    QString str = atts.at("file");

    m_function->lorentz.width = atts.at("width").toDouble();

    if (atts.at("order") != "") {
      m_function->lorentz.order = atts.at("order").toInt();
    } else { // in older versions, 2n-lorentz was specified by the value "2n"
      m_function->lorentz.order = atts.at("degree").toInt() / 2;
    }

    m_function->lorentz.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;
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

    QString str = atts.at("file");
    QString str2 = atts.at("file2");

    m_function->voigt.fwhmL = atts.at("fwhmleft").toDouble();
    m_function->voigt.fwhmR = atts.at("fwhmright").toDouble();
    m_function->voigt.glRatioL = atts.at("glrleft").toDouble();
    m_function->voigt.glRatioR = atts.at("glrright").toDouble();

    m_function->voigt.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;

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

    QString str = atts.at("file");
    QString str2 = atts.at("file2");

    m_function->error.fwhm = atts.at("fwhm").toDouble();
    m_function->error.width = atts.at("width").toDouble();

    m_function->error.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;

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

    QString str = atts.at("file");
    QString str2 = atts.at("file2");

    m_function->agauss.fwhm = atts.at("fwhm").toDouble();
    m_function->agauss.asym = atts.at("asym").toDouble();

    m_function->agauss.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;

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

    QString str = atts.at("file");
    QString str2 = atts.at("file2");
    QString str3 = atts.at("file3");

    m_function->supergauss.fwhm = atts.at("fwhm").toDouble();
    m_function->supergauss.exponential = atts.at("expTerm").toDouble();
    m_function->supergauss.asym = atts.at("asym").toDouble();

    m_function->supergauss.wveDptFlag=(atts.at("wveDptFlag") == "true") ? 1 : 0;

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

    m_function->boxcarapod.resolution = atts.at("resolution").toDouble();
    m_function->boxcarapod.phase = atts.at("phase").toDouble();
  }
  else if (element == "nbsapod") {

    m_function->nbsapod.resolution = atts.at("resolution").toDouble();
    m_function->nbsapod.phase = atts.at("phase").toDouble();
  }
  else if (element == "gaussianfile") {

    QString str = atts.at("file");
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

    QString str = atts.at("file");
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

    if (atts.at("order") != "") {
      m_function->lorentz.order = atts.at("order").toInt();
    } else {
      m_function->lorentz.order = atts.at("degree").toInt() / 2;
    }
  }
  else if (element == "errorfile") {

    QString str = atts.at("file");
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

    QString str2 = atts.at("file2");
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

