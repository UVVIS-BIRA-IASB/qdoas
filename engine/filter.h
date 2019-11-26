#ifndef FILTER_H
#define FILTER_H

#include "comdefs.h"

void realft(double *source,double *buffer,int nn,int sens);

RC   FILTER_OddEvenCorrection(double *lambdaData,double *specData,double *output,int vectorSize);
RC   FILTER_Vector(PRJCT_FILTER *pFilter,double *Input,double *Output,double *tmpVector,int Size,int outputType);
RC   FILTER_Build(PRJCT_FILTER *pFilter,double param1,double param2,double param3);
RC   FILTER_LoadFilter(PRJCT_FILTER *pFilter);

#endif
