
#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

//------------------------------------------------------------------------
// handler for <lowpass_filter> and <highpass_filter>

CFilteringSubHandler::CFilteringSubHandler(CConfigHandler *master,
					   mediate_filter_t *filter) :
  CConfigSubHandler(master),
  m_filter(filter)
{
}

bool CFilteringSubHandler::start(const QXmlAttributes &atts)
{
  // selected filter

  QString str = atts.value("selected");

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
    return postErrorMessage("Invalid filter method");

  return true;
}

bool CFilteringSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // sub element of lowpass_filter or highpass_filter

  if (element == "kaiser") {

    m_filter->kaiser.cutoffFrequency = atts.value("cutoff").toDouble();
    m_filter->kaiser.tolerance = atts.value("tolerance").toDouble();
    m_filter->kaiser.passband = atts.value("passband").toDouble();
    m_filter->kaiser.iterations = atts.value("iterations").toInt();
    m_filter->kaiser.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->kaiser.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->kaiser.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "boxcar") {

    m_filter->boxcar.width = atts.value("width").toInt();
    m_filter->boxcar.iterations = atts.value("iterations").toInt();
    m_filter->boxcar.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->boxcar.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->boxcar.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "gaussian") {

    m_filter->gaussian.fwhm = atts.value("fwhm").toDouble();
    m_filter->gaussian.iterations = atts.value("iterations").toInt();
    m_filter->gaussian.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->gaussian.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->gaussian.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "triangular") {

    m_filter->triangular.width = atts.value("width").toInt();
    m_filter->triangular.iterations = atts.value("iterations").toInt();
    m_filter->triangular.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->triangular.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->triangular.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "savitzky_golay") {

    m_filter->savitzky.width = atts.value("width").toInt();
    m_filter->savitzky.order = atts.value("order").toInt();
    m_filter->savitzky.iterations = atts.value("iterations").toInt();
    m_filter->savitzky.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->savitzky.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->savitzky.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "binomial") {

    m_filter->binomial.width = atts.value("width").toInt();
    m_filter->binomial.iterations = atts.value("iterations").toInt();
    m_filter->binomial.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->binomial.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->binomial.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <slit_func>

CSlitFunctionSubHandler::CSlitFunctionSubHandler(CConfigHandler *master,
						 mediate_slit_function_t *function) :
  CConfigSubHandler(master),
  m_function(function)
{
}

bool CSlitFunctionSubHandler::start(const QXmlAttributes &atts)
{
  QString str = atts.value("type");

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
    return postErrorMessage("Invalid slit type");

  return true;
}

bool CSlitFunctionSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "file") {

    QString str = atts.value("file");
    QString str2 = atts.value("file2");

    m_function->file.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->file.filename))
	strcpy(m_function->file.filename, str.toLatin1().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->file.filename2))
	      strcpy(m_function->file.filename2, str2.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->file.filename2 ,"\0");
  }
  else if (element == "gaussian") {

  	 QString str = atts.value("file");

    m_function->gaussian.fwhm = atts.value("fwhm").toDouble();
    m_function->gaussian.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;

   if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussian.filename))
	      strcpy(m_function->gaussian.filename, str.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->gaussian.filename ,"\0");
  }
  else if (element == "lorentz") {

    QString str = atts.value("file");

    m_function->lorentz.width = atts.value("width").toDouble();

    if (atts.value("order") != "") {
      m_function->lorentz.order = atts.value("order").toInt();
    } else { // in older versions, 2n-lorentz was specified by the value "2n"
      m_function->lorentz.order = atts.value("degree").toInt() / 2;
    }

    m_function->lorentz.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;
    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentz.filename))
	      strcpy(m_function->lorentz.filename, str.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->lorentz.filename ,"\0");
  }
  else if (element == "voigt") {

    QString str = atts.value("file");
    QString str2 = atts.value("file2");

    m_function->voigt.fwhmL = atts.value("fwhmleft").toDouble();
    m_function->voigt.fwhmR = atts.value("fwhmright").toDouble();
    m_function->voigt.glRatioL = atts.value("glrleft").toDouble();
    m_function->voigt.glRatioR = atts.value("glrright").toDouble();

    m_function->voigt.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->voigt.filename))
	      strcpy(m_function->voigt.filename, str.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->voigt.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->voigt.filename2))
	      strcpy(m_function->voigt.filename2, str2.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->voigt.filename2 ,"\0");
  }
  else if (element == "error") {

    QString str = atts.value("file");
    QString str2 = atts.value("file2");

    m_function->error.fwhm = atts.value("fwhm").toDouble();
    m_function->error.width = atts.value("width").toDouble();

    m_function->error.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->error.filename))
	      strcpy(m_function->error.filename, str.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->error.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->error.filename2))
	      strcpy(m_function->error.filename2, str2.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->error.filename2,"\0");
  }
  else if (element == "agauss") {

    QString str = atts.value("file");
    QString str2 = atts.value("file2");

    m_function->agauss.fwhm = atts.value("fwhm").toDouble();
    m_function->agauss.asym = atts.value("asym").toDouble();

    m_function->agauss.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->agauss.filename))
	      strcpy(m_function->agauss.filename, str.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->agauss.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->agauss.filename2))
	      strcpy(m_function->agauss.filename2, str2.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename too long");
	    }
	   else
	    strcpy(m_function->agauss.filename2 ,"\0");
  }

  else if (element == "supergauss") {

    QString str = atts.value("file");
    QString str2 = atts.value("file2");
    QString str3 = atts.value("file3");

    m_function->supergauss.fwhm = atts.value("fwhm").toDouble();
    m_function->supergauss.exponential = atts.value("expTerm").toDouble();
    m_function->supergauss.asym = atts.value("asym").toDouble();

    m_function->supergauss.wveDptFlag=(atts.value("wveDptFlag") == "true") ? 1 : 0;

    if (!str.isEmpty())
     {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->supergauss.filename))
	      strcpy(m_function->supergauss.filename, str.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename (FWHM) too long");
	    }
	   else
	    strcpy(m_function->supergauss.filename ,"\0");

    if (!str2.isEmpty())
     {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->supergauss.filename2))
	      strcpy(m_function->supergauss.filename2, str2.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename (Exponential term) too long");
	    }
	   else
	    strcpy(m_function->supergauss.filename2 ,"\0");

    if (!str3.isEmpty())
     {
      str3 = m_master->pathExpand(str3);
      if (str3.length() < (int)sizeof(m_function->supergauss.filename3))
	      strcpy(m_function->supergauss.filename3, str3.toLatin1().data());
      else
	      return postErrorMessage("Slit Function Filename (Asymmetry factor) too long");
	    }
	   else
	    strcpy(m_function->supergauss.filename3 ,"\0");
  }
  else if (element == "boxcarapod") {

    m_function->boxcarapod.resolution = atts.value("resolution").toDouble();
    m_function->boxcarapod.phase = atts.value("phase").toDouble();
  }
  else if (element == "nbsapod") {

    m_function->nbsapod.resolution = atts.value("resolution").toDouble();
    m_function->nbsapod.phase = atts.value("phase").toDouble();
  }
  else if (element == "gaussianfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussian.filename))
       {
	       strcpy(m_function->gaussian.filename, str.toLatin1().data());
	       m_function->gaussian.wveDptFlag=1;
	      }
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "lorentzfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentz.filename))
       {
	       strcpy(m_function->lorentz.filename, str.toLatin1().data());
	       m_function->lorentz.wveDptFlag=1;
	      }

      else
	return postErrorMessage("Slit Function Filename too long");
    }

    if (atts.value("order") != "") {
      m_function->lorentz.order = atts.value("order").toInt();
    } else {
      m_function->lorentz.order = atts.value("degree").toInt() / 2;
    }
  }
  else if (element == "errorfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->error.filename))
       {
       	m_function->error.wveDptFlag=1;
	       strcpy(m_function->error.filename, str.toLatin1().data());
	      }
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    QString str2 = atts.value("file2");
    if (!str2.isEmpty()) {
      str2 = m_master->pathExpand(str2);
      if (str2.length() < (int)sizeof(m_function->error.filename2))
	strcpy(m_function->error.filename2, str2.toLatin1().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
   else
    strcpy(m_function->error.filename2, str.toLatin1().data());
  }

  return true;
}

