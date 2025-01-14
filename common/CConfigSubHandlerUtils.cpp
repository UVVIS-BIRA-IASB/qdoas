
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

using std::map;
using std::string;

//------------------------------------------------------------------------
// handler for <lowpass_filter> and <highpass_filter>

CFilteringSubHandler::CFilteringSubHandler(CConfigHandler *master,
                       mediate_filter_t *filter) :
  CConfigSubHandler(master),
  m_filter(filter)
{
}

void CFilteringSubHandler::start(const map<Glib::ustring, string>& atts)
{
  // selected filter

  string str = value(atts, "selected");

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

void CFilteringSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, string>& atts)
{
  // sub element of lowpass_filter or highpass_filter

  if (element == "kaiser") {
    m_filter->kaiser.cutoffFrequency = stod(value(atts, "cutoff"));
    m_filter->kaiser.tolerance = stod(value(atts, "tolerance"));
    m_filter->kaiser.passband = stod(value(atts, "passband"));
    m_filter->kaiser.iterations = stoi(value(atts, "iterations"));
    m_filter->kaiser.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->kaiser.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->kaiser.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "boxcar") {

    m_filter->boxcar.width = stoi(value(atts, "width"));
    m_filter->boxcar.iterations = stoi(value(atts, "iterations"));
    m_filter->boxcar.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->boxcar.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->boxcar.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "gaussian") {

    m_filter->gaussian.fwhm = stod(value(atts, "fwhm"));
    m_filter->gaussian.iterations = stoi(value(atts, "iterations"));
    m_filter->gaussian.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->gaussian.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->gaussian.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "triangular") {

    m_filter->triangular.width = stoi(value(atts, "width"));
    m_filter->triangular.iterations = stoi(value(atts, "iterations"));
    m_filter->triangular.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->triangular.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->triangular.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "savitzky_golay") {

    m_filter->savitzky.width = stoi(value(atts, "width"));
    m_filter->savitzky.order = stoi(value(atts, "order"));
    m_filter->savitzky.iterations = stoi(value(atts, "iterations"));
    m_filter->savitzky.usage.calibrationFlag = (value(atts, "cal") == "true") ? 1 : 0;
    m_filter->savitzky.usage.fittingFlag = (value(atts, "fit") == "true") ? 1 : 0;
    m_filter->savitzky.usage.divide = (value(atts, "div") == "true") ? 1 : 0;
  }
  else if (element == "binomial") {

    m_filter->binomial.width = stoi(value(atts, "width"));
    m_filter->binomial.iterations = stoi(value(atts, "iterations"));
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

void CSlitFunctionSubHandler::start(const map<Glib::ustring, string>& atts)
{
  string str = value(atts, "type");

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

void CSlitFunctionSubHandler::start(const Glib::ustring& element, const map<Glib::ustring, string>& atts)
{
  if (element == "file") {

    string str = value(atts, "file");
    string str2 = value(atts, "file2");

    m_function->file.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->file.filename))
    strcpy(m_function->file.filename, str.c_str());
      else
    throw std::runtime_error("Slit Function Filename too long");
    }

    if (!str2.empty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->file.filename2))
          strcpy(m_function->file.filename2, str2.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->file.filename2 ,"\0");
  }
  else if (element == "gaussian") {

    string str = value(atts, "file");

    m_function->gaussian.fwhm = stod(value(atts, "fwhm"));
    m_function->gaussian.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

   if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussian.filename))
          strcpy(m_function->gaussian.filename, str.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->gaussian.filename ,"\0");
  }
  else if (element == "lorentz") {

    string str = value(atts, "file");

    m_function->lorentz.width = stod(value(atts, "width"));

    if (value(atts, "order") != "") {
      m_function->lorentz.order = stoi(value(atts, "order"));
    } else { // in older versions, 2n-lorentz was specified by the value "2n"
      m_function->lorentz.order = stoi(value(atts, "degree"));
    }

    m_function->lorentz.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;
    if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentz.filename))
          strcpy(m_function->lorentz.filename, str.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->lorentz.filename ,"\0");
  }
  else if (element == "voigt") {

    string str = value(atts, "file");
    string str2 = value(atts, "file2");

    m_function->voigt.fwhmL = stod(value(atts, "fwhmleft"));
    m_function->voigt.fwhmR = stod(value(atts, "fwhmright"));
    m_function->voigt.glRatioL = stod(value(atts, "glrleft"));
    m_function->voigt.glRatioR = stod(value(atts, "glrright"));

    m_function->voigt.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->voigt.filename))
          strcpy(m_function->voigt.filename, str.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->voigt.filename ,"\0");

    if (!str2.empty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->voigt.filename2))
          strcpy(m_function->voigt.filename2, str2.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->voigt.filename2 ,"\0");
  }
  else if (element == "error") {

    string str = value(atts, "file");
    string str2 = value(atts, "file2");

    m_function->error.fwhm = stod(value(atts, "fwhm"));
    m_function->error.width = stod(value(atts, "width"));

    m_function->error.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->error.filename))
          strcpy(m_function->error.filename, str.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->error.filename ,"\0");

    if (!str2.empty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->error.filename2))
          strcpy(m_function->error.filename2, str2.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->error.filename2,"\0");
  }
  else if (element == "agauss") {

    string str = value(atts, "file");
    string str2 = value(atts, "file2");

    m_function->agauss.fwhm = stod(value(atts, "fwhm"));
    m_function->agauss.asym = stod(value(atts, "asym"));

    m_function->agauss.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->agauss.filename))
          strcpy(m_function->agauss.filename, str.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->agauss.filename ,"\0");

    if (!str2.empty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->agauss.filename2))
          strcpy(m_function->agauss.filename2, str2.c_str());
      else
          throw std::runtime_error("Slit Function Filename too long");
        }
       else
        strcpy(m_function->agauss.filename2 ,"\0");
  }

  else if (element == "supergauss") {

    string str = value(atts, "file");
    string str2 = value(atts, "file2");
    string str3 = value(atts, "file3");

    m_function->supergauss.fwhm = stod(value(atts, "fwhm"));
    m_function->supergauss.exponential = stod(value(atts, "expTerm"));
    m_function->supergauss.asym = stod(value(atts, "asym"));

    m_function->supergauss.wveDptFlag=(value(atts, "wveDptFlag") == "true") ? 1 : 0;

    if (!str.empty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->supergauss.filename))
          strcpy(m_function->supergauss.filename, str.c_str());
      else
          throw std::runtime_error("Slit Function Filename (FWHM) too long");
        }
       else
        strcpy(m_function->supergauss.filename ,"\0");

    if (!str2.empty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->supergauss.filename2))
          strcpy(m_function->supergauss.filename2, str2.c_str());
      else
          throw std::runtime_error("Slit Function Filename (Exponential term) too long");
        }
       else
        strcpy(m_function->supergauss.filename2 ,"\0");

    if (!str3.empty())
     {
      str3 = m_master->pathExpand(str3);
      if (str3.length() < (int)sizeof(m_function->supergauss.filename3))
          strcpy(m_function->supergauss.filename3, str3.c_str());
      else
          throw std::runtime_error("Slit Function Filename (Asymmetry factor) too long");
        }
       else
        strcpy(m_function->supergauss.filename3 ,"\0");
  }
  else if (element == "boxcarapod") {

    m_function->boxcarapod.resolution = stod(value(atts, "resolution"));
    m_function->boxcarapod.phase = stod(value(atts, "phase"));
  }
  else if (element == "nbsapod") {

    m_function->nbsapod.resolution = stod(value(atts, "resolution"));
    m_function->nbsapod.phase = stod(value(atts, "phase"));
  }
  else if (element == "gaussianfile") {

    string str = value(atts, "file");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussian.filename))
       {
           strcpy(m_function->gaussian.filename, str.c_str());
           m_function->gaussian.wveDptFlag=1;
          }
      else
    throw std::runtime_error("Slit Function Filename too long");
    }
  }
  else if (element == "lorentzfile") {

    string str = value(atts, "file");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentz.filename))
       {
           strcpy(m_function->lorentz.filename, str.c_str());
           m_function->lorentz.wveDptFlag=1;
          }

      else
    throw std::runtime_error("Slit Function Filename too long");
    }

    if (value(atts, "order") != "") {
      m_function->lorentz.order = stoi(value(atts, "order"));
    } else {
      m_function->lorentz.order = stoi(value(atts, "degree"));
    }
  }
  else if (element == "errorfile") {

    string str = value(atts, "file");
    if (!str.empty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->error.filename))
       {
           m_function->error.wveDptFlag=1;
           strcpy(m_function->error.filename, str.c_str());
          }
      else
    throw std::runtime_error("Slit Function Filename too long");
    }

    string str2 = value(atts, "file2");
    if (!str2.empty()) {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->error.filename2))
    strcpy(m_function->error.filename2, str2.c_str());
      else
    throw std::runtime_error("Slit Function Filename too long");
    }
   else
    strcpy(m_function->error.filename2, str.c_str());
  }

}

