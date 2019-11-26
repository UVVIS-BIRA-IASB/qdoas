
#include "ConfigWriterUtils.h"
#include "CPathMgr.h"

#include "constants.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

void writePaths(FILE *fp)
{
  CPathMgr *pathMgr = CPathMgr::instance();

  // paths
  fprintf(fp, "  <paths>\n"
	  "    <!-- upto 10 paths can be specified (index 0 to 9). Any file or directory name in the  -->\n"
	  "    <!-- raw_spectra tree that begins with %%? (where ? is a single digit) is expanded with -->\n"
	  "    <!-- the correponding path.                                                            -->\n\n");

  for (int i=0; i<10; ++i) {
    QString path = pathMgr->path(i);
    if (!path.isNull()) {
      fprintf(fp, "    <path index=\"%d\">%s</path>\n", i, path.toLocal8Bit().constData());
    }
  }
  fprintf(fp, "  </paths>\n");
}

void writeFilter(FILE *fp, size_t nIndent, const char *passband, const mediate_filter_t *d)
{
  size_t i;
  char buf[32]; // max index is sizeof(buf)-1

  // make a space padding string - robust against nIndex < 0.
  if (nIndent >= sizeof(buf)) nIndent = sizeof(buf) - 1;
  for (i=0; i<nIndent; ++i) buf[i] = ' ';
  buf[i] = '\0';

  fprintf(fp, "%s<%spass_filter selected=", buf, passband); // low or high
  switch (d->mode) {
  case PRJCT_FILTER_TYPE_KAISER:
    fprintf(fp, "\"kaiser\"");
    break;
  case PRJCT_FILTER_TYPE_BOXCAR:
    fprintf(fp, "\"boxcar\"");
    break;
  case PRJCT_FILTER_TYPE_GAUSSIAN:
    fprintf(fp, "\"gaussian\"");
    break;
  case PRJCT_FILTER_TYPE_TRIANGLE:
    fprintf(fp, "\"triangular\"");
    break;
  case PRJCT_FILTER_TYPE_SG:
    fprintf(fp, "\"savitzky_golay\"");
    break;
  case PRJCT_FILTER_TYPE_ODDEVEN:
    fprintf(fp, "\"oddeven\"");
    break;
  case PRJCT_FILTER_TYPE_BINOMIAL:
    fprintf(fp, "\"binomial\"");
    break;
  default:
    fprintf(fp, "\"none\"");
  }
  fprintf(fp, ">\n");

  fprintf(fp, "%s  <kaiser cutoff=\"%f\" tolerance=\"%f\" passband=\"%f\" iterations=\"%d\" cal=\"%s\" fit=\"%s\" div=\"%s\" />\n", buf,
	  d->kaiser.cutoffFrequency, d->kaiser.tolerance, d->kaiser.passband, d->kaiser.iterations,
	  (d->kaiser.usage.calibrationFlag ? sTrue : sFalse),
	  (d->kaiser.usage.fittingFlag ? sTrue : sFalse),
	  (d->kaiser.usage.divide ? sTrue : sFalse));
  fprintf(fp, "%s  <boxcar width=\"%d\" iterations=\"%d\" cal=\"%s\" fit=\"%s\" div=\"%s\" />\n", buf,
	  d->boxcar.width, d->boxcar.iterations,
	  (d->boxcar.usage.calibrationFlag ? sTrue : sFalse),
	  (d->boxcar.usage.fittingFlag ? sTrue : sFalse),
	  (d->boxcar.usage.divide ? sTrue : sFalse));
  fprintf(fp, "%s  <gaussian fwhm=\"%f\" iterations=\"%d\" cal=\"%s\" fit=\"%s\" div=\"%s\" />\n", buf,
	  d->gaussian.fwhm, d->gaussian.iterations,
	  (d->gaussian.usage.calibrationFlag ? sTrue : sFalse),
	  (d->gaussian.usage.fittingFlag ? sTrue : sFalse),
	  (d->gaussian.usage.divide ? sTrue : sFalse));
  fprintf(fp, "%s  <triangular width=\"%d\" iterations=\"%d\" cal=\"%s\" fit=\"%s\" div=\"%s\" />\n", buf,
	  d->triangular.width, d->triangular.iterations,
	  (d->triangular.usage.calibrationFlag ? sTrue : sFalse),
	  (d->triangular.usage.fittingFlag ? sTrue : sFalse),
	  (d->triangular.usage.divide ? sTrue : sFalse));
  fprintf(fp, "%s  <savitzky_golay width=\"%d\" order=\"%d\" iterations=\"%d\" cal=\"%s\" fit=\"%s\" div=\"%s\" />\n", buf,
	  d->savitzky.width, d->savitzky.order, d->savitzky.iterations,
	  (d->savitzky.usage.calibrationFlag ? sTrue : sFalse),
	  (d->savitzky.usage.fittingFlag ? sTrue : sFalse),
	  (d->savitzky.usage.divide ? sTrue : sFalse));
  fprintf(fp, "%s  <binomial width=\"%d\" iterations=\"%d\" cal=\"%s\" fit=\"%s\" div=\"%s\" />\n", buf,
	  d->binomial.width, d->binomial.iterations,
	  (d->binomial.usage.calibrationFlag ? sTrue : sFalse),
	  (d->binomial.usage.fittingFlag ? sTrue : sFalse),
	  (d->binomial.usage.divide ? sTrue : sFalse));
  fprintf(fp, "%s</%spass_filter>\n", buf, passband);
}

void writeSlitFunction(FILE *fp, size_t nIndent, const mediate_slit_function_t *d)
{
  size_t i;
  char buf[32]; // max index is sizeof(buf)-1
  QString tmpStr,tmpStr2,tmpStr3;
  CPathMgr *pathMgr = CPathMgr::instance();

  // make a space padding string - robust against nIndex < 0.
  if (nIndent >= sizeof(buf)) nIndent = sizeof(buf) - 1;
  for (i=0; i<nIndent; ++i) buf[i] = ' ';
  buf[i] = '\0';

  fprintf(fp, "%s<slit_func type=", buf);

  switch (d->type) {
  case SLIT_TYPE_NONE:
    fprintf(fp, "\"none\"");
    break;
  case SLIT_TYPE_FILE:
    fprintf(fp, "\"file\"");
    break;
  case SLIT_TYPE_GAUSS:
    fprintf(fp, "\"gaussian\"");
    break;
  case SLIT_TYPE_INVPOLY:
    fprintf(fp, "\"lorentz\"");
    break;
  case SLIT_TYPE_VOIGT:
    fprintf(fp, "\"voigt\"");
    break;
  case SLIT_TYPE_ERF:
    fprintf(fp, "\"error\"");
    break;
  case SLIT_TYPE_AGAUSS:
    fprintf(fp, "\"agauss\"");
    break;
  case SLIT_TYPE_SUPERGAUSS:
    fprintf(fp, "\"supergauss\"");
    break;
  case SLIT_TYPE_APOD:
    fprintf(fp, "\"boxcarapod\"");
    break;
  case SLIT_TYPE_APODNBS:
    fprintf(fp, "\"nbsapod\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, ">\n");

  tmpStr = pathMgr->simplifyPath(QString(d->file.filename));
  tmpStr2 = pathMgr->simplifyPath(QString(d->file.filename2));
  fprintf(fp, "%s  <file file=\"%s\" wveDptFlag=\"%s\" file2=\"%s\" />\n", buf, tmpStr.toLocal8Bit().data(),(d->file.wveDptFlag ? sTrue : sFalse),tmpStr2.toLocal8Bit().data());
  tmpStr = pathMgr->simplifyPath(QString(d->gaussian.filename));
  fprintf(fp, "%s  <gaussian fwhm=\"%.3f\" wveDptFlag=\"%s\" file=\"%s\" />\n", buf, d->gaussian.fwhm,(d->gaussian.wveDptFlag ? sTrue : sFalse),tmpStr.toLocal8Bit().data());
  tmpStr = pathMgr->simplifyPath(QString(d->lorentz.filename));
  fprintf(fp, "%s  <lorentz width=\"%.3f\" order=\"%d\" wveDptFlag=\"%s\" file=\"%s\" />\n", buf, d->lorentz.width, d->lorentz.order,(d->lorentz.wveDptFlag ? sTrue : sFalse),tmpStr.toLocal8Bit().data());
  tmpStr = pathMgr->simplifyPath(QString(d->voigt.filename));
  tmpStr2 = pathMgr->simplifyPath(QString(d->voigt.filename2));
  fprintf(fp, "%s  <voigt fwhmleft=\"%.3f\" fwhmright=\"%.3f\" glrleft=\"%.3f\" glrright=\"%.3f\" wveDptFlag=\"%s\" file=\"%s\" file2=\"%s\" />\n",
	  buf, d->voigt.fwhmL, d->voigt.fwhmR, d->voigt.glRatioL, d->voigt.glRatioR,(d->voigt.wveDptFlag ? sTrue : sFalse),tmpStr.toLocal8Bit().data(), tmpStr2.toLocal8Bit().data());
  tmpStr = pathMgr->simplifyPath(QString(d->error.filename));
  tmpStr2 = pathMgr->simplifyPath(QString(d->error.filename2));
  fprintf(fp, "%s  <error fwhm=\"%.3f\" width=\"%.3f\" wveDptFlag=\"%s\" file=\"%s\" file2=\"%s\" />\n", buf, d->error.fwhm, d->error.width,(d->error.wveDptFlag ? sTrue : sFalse),tmpStr.toLocal8Bit().data(), tmpStr2.toLocal8Bit().data());
  tmpStr = pathMgr->simplifyPath(QString(d->agauss.filename));
  tmpStr2 = pathMgr->simplifyPath(QString(d->agauss.filename2));
  fprintf(fp, "%s  <agauss fwhm=\"%.3f\" asym=\"%.3f\" wveDptFlag=\"%s\" file=\"%s\" file2=\"%s\" />\n", buf, d->agauss.fwhm, d->agauss.asym,(d->agauss.wveDptFlag ? sTrue : sFalse),tmpStr.toLocal8Bit().data(), tmpStr2.toLocal8Bit().data());
  tmpStr = pathMgr->simplifyPath(QString(d->supergauss.filename));
  tmpStr2 = pathMgr->simplifyPath(QString(d->supergauss.filename2));
  tmpStr3 = pathMgr->simplifyPath(QString(d->supergauss.filename3));
  fprintf(fp, "%s  <supergauss fwhm=\"%.3f\" expTerm=\"%.3f\" asym=\"%.3f\" wveDptFlag=\"%s\" file=\"%s\" file2=\"%s\" file3=\"%s\" />\n", buf, d->supergauss.fwhm, d->supergauss.exponential,d->supergauss.asym,(d->supergauss.wveDptFlag ? sTrue : sFalse),tmpStr.toLocal8Bit().data(), tmpStr2.toLocal8Bit().data(), tmpStr3.toLocal8Bit().data());
  fprintf(fp, "%s  <boxcarapod resolution=\"%.3f\" phase=\"%.3f\" />\n", buf,
	  d->boxcarapod.resolution, d->boxcarapod.phase);
  fprintf(fp, "%s  <nbsapod resolution=\"%.3f\" phase=\"%.3f\" />\n", buf,
	  d->nbsapod.resolution, d->nbsapod.phase);

  fprintf(fp, "%s</slit_func>\n", buf);
}

