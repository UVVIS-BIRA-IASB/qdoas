/* Copyright (C) 2007-2017 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 *
 */

#ifndef SVD_H
#define SVD_H

struct svd {
  double **U,**V,*W; // SVD matrices
};

int SVD_Bksb(const struct svd *svd, int m, int n, const double *b, double *x);
int SVD_Dcmp(struct svd *svd, int m, int n, double *SigmaSqr,double **covar);

#endif
