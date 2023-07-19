
//  ----------------------------------------------------------------------------
//! \addtogroup DOAS
//! Mathematical functions of the DOAS engine
//! @{
//!
//! \file      linear_system.c
//! \brief     Solves the linear part of the Beer-Lambert's law
//! \details   During a long time, SVD (singular value decomposition) was the
//!            solution used to solve the linear part of the Beer-Lambert equation.
//!            Now, both solutions are implemented : SVD and QR decomposition.
//! \authors   \n
//! \date
//! \copyright QDOAS is distributed under GNU General Public License
//!
//! @}
//  ----------------------------------------------------------------------------
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB
//      Belgian Institute for Space Aeronomy
//      Avenue Circulaire, 3
//      1180     UCCLE
//      BELGIUM
//      qdoas@aeronomie.be
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------

extern "C" {
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "gsl/gsl_matrix.h"
#include "gsl/gsl_linalg.h"

#include "linear_system.h"
#include "comdefs.h"
#include "svd.h"
#include "vector.h"
}

#include <Eigen/Dense>

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Matrix2d;
typedef Eigen::ColPivHouseholderQR<Matrix2d> MatrixQR;

#define EPS 2.2204e-016

struct qr {
  gsl_matrix *A;
  gsl_vector *tau;
};

struct eigen_qr {
  Matrix2d *A;
  MatrixQR *QR;
};

// linear system of m equations and n unknowns
struct linear_system {
  int m, n;
  double *norms; // colums of matrix are normalized to avoid numerical issues.
  enum linear_fit_mode mode;
  union {
    struct qr qr;
    struct svd svd;
    struct eigen_qr qr_eigen;
  } decomposition;
};

struct linear_system*LINEAR_alloc(int m, int n, enum linear_fit_mode mode) {
 
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin((char *)__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif
  
  struct linear_system *s = new linear_system();
  s->norms = new double[n];
  s->m = m;
  s->n = n;
  s->mode = mode;

  switch (mode) {
  case DECOMP_SVD:
    s->decomposition.svd.U=MEMORY_AllocDMatrix(__func__,"U",1,m,1,n);
    s->decomposition.svd.V=MEMORY_AllocDMatrix(__func__,"V",1,n,1,n);
    s->decomposition.svd.W=MEMORY_AllocDVector(__func__,"W",1,n);
    for (int j=0; j<n; ++j) {
      for(int i=0; i<m; ++i) {
        s->decomposition.svd.U[1+j][1+i]=0.;
      }
      for (int i=0; i<n; ++i) {
        s->decomposition.svd.V[1+j][1+i]=0.;
      }
      s->decomposition.svd.W[1+j]=0.;
    }
    break;
  case DECOMP_QR:
    s->decomposition.qr.A = gsl_matrix_alloc(m, n);
    s->decomposition.qr.tau = gsl_vector_alloc(n);
    break;
  case DECOMP_EIGEN_QR:
    s->decomposition.qr_eigen.A = new Matrix2d(m, n);
    s->decomposition.qr_eigen.QR = new MatrixQR(m, n);
    break;
  }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop((char *)__func__,0);
  #endif
  
  return s;
}

// linear system of m equations in n variables
struct linear_system *LINEAR_from_matrix(const double *const *a, int m, int n, enum linear_fit_mode mode) {
  struct linear_system *s = LINEAR_alloc(m, n, mode);
  switch (mode) {
  case DECOMP_SVD:
    // copy linear system to SVD matrix U
    for (int i=1; i <= s->n; ++i){
      for (int j=1; j <= s->m; ++j) {
        s->decomposition.svd.U[i][j] = a[i][j];
      }
    }
    break;
  case DECOMP_QR:
    for (int i=0; i<s->m; ++i) {
      for (int j=0; j<s->n; ++j) {
        gsl_matrix_set(s->decomposition.qr.A, i, j, a[1+j][1+i]);
      }
    }
    break;
  case DECOMP_EIGEN_QR:
    for (int i=0; i<s->m; ++i) {
      for (int j=0; j<s->n; ++j) {
	(*s->decomposition.qr_eigen.A)(i, j) = a[1+j][1+i];
      }
    }
    break;
  }
  return s;
}

void LINEAR_free(struct linear_system *s) {
  if (s == NULL)
    return;
  
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin((char *)__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  switch(s->mode) {
  case DECOMP_SVD:
    MEMORY_ReleaseDMatrix(__func__,"U",s->decomposition.svd.U,1,1);
    MEMORY_ReleaseDMatrix(__func__,"V",s->decomposition.svd.V,1,1);
    MEMORY_ReleaseDVector(__func__,"W",s->decomposition.svd.W,1);
    break;
  case DECOMP_QR:
    gsl_matrix_free(s->decomposition.qr.A);
    gsl_vector_free(s->decomposition.qr.tau);
    break;
  case DECOMP_EIGEN_QR:
    delete s->decomposition.qr_eigen.A;
  }

  delete[] s->norms;
  delete s;
  
  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop((char *)__func__,0);
  #endif
}

void LINEAR_set_column(struct linear_system *s, int n, const double *values) {
  switch (s->mode) {
  case DECOMP_SVD:
    for (int i=0; i<s->m; ++i) {
      s->decomposition.svd.U[n][1+i] = values[1+i];
    }
    break;
  case DECOMP_QR:
    for (int i=0; i<s->m; ++i) {
      gsl_matrix_set(s->decomposition.qr.A,i, n-1, values[1+i]);
    }
    break;
  case DECOMP_EIGEN_QR:
    for (int i=0; i<s->m; ++i) {
      (*s->decomposition.qr_eigen.A)(i, n-1) = values[1+i];
    }
    break;
  }
}

void LINEAR_set_weight(struct linear_system *s, const double *sigma) {
  if (sigma == NULL)
    return;

  switch (s->mode) {
  case DECOMP_SVD:
    for (int i=1; i<= s->n; ++i) {
      for (int j=1; j <= s->m; ++j) {
        s->decomposition.svd.U[i][j] /= sigma[j-1]; // sigma index runs from 0
      }
    }
    break;
  case DECOMP_QR:
    for (int i=0; i< s->m; ++i) {
      gsl_vector_view rowi = gsl_matrix_row(s->decomposition.qr.A, i);
      gsl_vector_scale(&rowi.vector, 1.0/sigma[i]);
    }
    break;
  case DECOMP_EIGEN_QR:
    for (int i=0; i< s->m; ++i) {
      s->decomposition.qr_eigen.A->row(i) /= sigma[i];
    }
    break;
  }
}

int LINEAR_decompose(struct linear_system *s, double *sigmasquare, double **covar) {
  int rc=ERROR_ID_NO;

  // decomposition depending on solution method
  switch (s->mode) {
  case DECOMP_SVD:
    // normalization:
    for (int i=0; i< s->n; ++i) {
      rc = VECTOR_NormalizeVector(s->decomposition.svd.U[1+i],s->m,&s->norms[i],__func__);
      if (rc)
        return rc;
    }
    rc = SVD_Dcmp(&s->decomposition.svd, s->m, s->n, sigmasquare, covar);
    // rescale sigmasquare & covariance using norm:
    for (int j=0; j<s->n; ++j) {
      if (covar != NULL) {
        for (int i=0; i<s->n; ++i) {
          covar[1+j][1+i] /= s->norms[i]*s->norms[j];
        }
      }
      if (sigmasquare !=NULL) {
        sigmasquare[1+j] /= s->norms[j]*s->norms[j];
      }
    }
    break;
  case DECOMP_QR: {
    // normalisation
    for (int j=0; j<s->n; ++j) {
      gsl_vector_view colj = gsl_matrix_column(s->decomposition.qr.A, j);
      s->norms[j]=0.;
      for (int i=0; i<s->m; ++i) {
        
        s->norms[j] += gsl_vector_get(&colj.vector, i)*gsl_vector_get(&colj.vector, i);
//        if (i>=s->m-6) printf("j:%d  s->norms[%d]=%lf\n",j,i,s->norms[j]);
      }
      if (s->norms[j] == 0.)
        return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_NORMALIZE);
      s->norms[j] = sqrt(s->norms[j]);
      gsl_vector_scale(&colj.vector, 1.0/s->norms[j]);
    }
    int rc_gsl = gsl_linalg_QR_decomp(s->decomposition.qr.A, s->decomposition.qr.tau);
    if (rc_gsl)
      rc = ERROR_ID_SVD_ILLCONDITIONED; // TODO: specific error conditions for QR

    // calculate covariance (A' * A)^-1 = (R' * R)^-1 using cholesky inversion method
    gsl_matrix *cholesky = gsl_matrix_alloc(s->n, s->n);
    for(int i=0; i<s->n; ++i) {
      // after gsl_linalg_QR_decomp(), the diagonal & upper triangle of A contain the matrix R.
      for (int j=i; j<s->n; ++j) { // diagonal & upper triangle: copy from A:
        gsl_matrix_set(cholesky, i, j, gsl_matrix_get(s->decomposition.qr.A,i, j));
      }
      for (int j=0;j<i; ++j) { // lower triangle part: copy from previous upper triangle:
        gsl_matrix_set(cholesky, i, j, gsl_matrix_get(cholesky, j, i));
      }
    }
    rc_gsl = gsl_linalg_cholesky_invert(cholesky);
    if (rc_gsl) {
      rc = ERROR_ID_SVD_ILLCONDITIONED; // TODO: specific error conditions for QR
      goto cleanup_qr;
    }

    if (covar != NULL) {
      for (int i=0; i<s->n; ++i) {
        for (int j=0; j<s->n; ++j) {
          covar[1+i][1+j]=gsl_matrix_get(cholesky, i, j) / (s->norms[i]*s->norms[j]);
        }
      }
    }
    if (sigmasquare != NULL) {
      for (int i=0; i<s->n; ++i) {
        sigmasquare[1+i] = gsl_matrix_get(cholesky, i, i) / (s->norms[i]*s->norms[i]);
      }
    }
    cleanup_qr:
    gsl_matrix_free(cholesky);
  }
    break;
  case DECOMP_EIGEN_QR: {
    // normalisation:
    for (int j=0; j<s->n; ++j) {
      auto col_j = s->decomposition.qr_eigen.A->col(j);
      s->norms[j] = col_j.squaredNorm();
      if (s->norms[j] == 0.)
	return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_NORMALIZE);
      s->norms[j] = sqrt(s->norms[j]);
      col_j /= s->norms[j];
    }
    s->decomposition.qr_eigen.QR->compute(*s->decomposition.qr_eigen.A);

    // Compute covariance.
    // The covariance matrix is given by the inverse of A' * A.  Unfortunately, Eigen does not seem to provide a way to
    // reuse the matrix R from our QR decomposition, which is also the Cholesky factor of A' * A.  Therefore, we compute
    // the inverse of A' * A again, using Cholesky decomposition.
    const auto& matrix_A = *s->decomposition.qr_eigen.A;
    Matrix2d matrix_covar = (matrix_A.transpose() * matrix_A).llt().solve(Eigen::MatrixXd::Identity(s->n, s->n));
    if (covar != NULL) {
      for (int i=0; i<s->n; ++i) {
	for (int j=0; j<s->n; ++j) {
	  covar[1+i][1+j] = matrix_covar(i, j) / (s->norms[i]*s->norms[j]);
        }
      }
    }
    if (sigmasquare != NULL) {
      for (int i=0; i<s->n; ++i) {
        sigmasquare[1+i] = matrix_covar(i, i) / (s->norms[i]*s->norms[i]);
      }
    }
  }
    break;
  }
  return rc;
}

int LINEAR_solve(const struct linear_system *s, const double *b, double *x) {
  int rc=ERROR_ID_NO;
  switch (s->mode) {
  case DECOMP_SVD:
    rc=SVD_Bksb(&s->decomposition.svd, s->m, s->n, b, x);
    break;
  case DECOMP_QR: {
    gsl_vector *vx = gsl_vector_alloc(s->n);
    gsl_vector *vb = gsl_vector_alloc(s->m);
    gsl_vector *residual = gsl_vector_alloc(s->m);
    for (int i=0; i<s->m; ++i) {
      gsl_vector_set(vb, i, b[1+i]);
    }
    int rc_gsl = gsl_linalg_QR_lssolve(s->decomposition.qr.A, s->decomposition.qr.tau, vb, vx, residual);
    if (rc_gsl)
      rc = ERROR_ID_SVD_ILLCONDITIONED; // TODO: specific error conditions for QR
    for (int i=0; i<s->n; ++i) {
      x[1+i] = gsl_vector_get(vx, i);
    }
    gsl_vector_free(vb);
    gsl_vector_free(vx);
    gsl_vector_free(residual);
  }
    break;
  case DECOMP_EIGEN_QR: {
    // Define Eigen vectors mapping the existing buffers b and x (b and x index starts at 1):
    Eigen::Map<const Eigen::VectorXd> vb(b+1, s->m);
    Eigen::Map<Eigen::VectorXd> vx(x+1, s->n);
    vx = s->decomposition.qr_eigen.QR->solve(vb);
  }
    break;
  }
  // divide solution by normalization factor
  for (int i=0; i< s->n; ++i) {
    x[1+i]/=s->norms[i];
  }
  return rc;
}

// for a linear system with matrix a[n][m], calculate the pseudoinverse pinv[m][n]
//
// preconditions:
//
//  - linear system must be decomposed,
//
//  - matrix pinv is allocated with correct dimensions
void LINEAR_pinv(const struct linear_system *s, double **pinv) {
  assert(s->mode == DECOMP_SVD); // currently only implemented for SVD decomposition

  // SVD decomposition: A = U * W * V',
  // pinv(A) = V * W^{-1} * U'
  const struct svd *psvd = &s->decomposition.svd;

  double tolerance = std::max(s->m,s->n)*psvd->W[1]*EPS;
  // singular values less than tolerance are treated as zero
  int r;
  for (r=1; r<=s->n && psvd->W[r]>tolerance; ++r);

  // Back substitution
  for (int i=1;i<=s->m; ++i)
    for (int j=1;j<=s->n ;++j)
      pinv[i][j]=0.;

  for (int i=1;i<=s->m; ++i)
    for (int j=1;j<r;++j) // if r > 1..
      for (int k=1;k<r;++k)
        pinv[i][j]+=psvd->V[k][j]*psvd->U[k][i]/psvd->W[k];
}

int LINEAR_fit_poly(int num_eqs, int poly_order, const double *a, const double *sigma, const double *b, double *x) {

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin((char *)__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif
  
  const int num_unknowns=1+poly_order;
  struct linear_system *linsys = NULL; //allocated later
  double **A = MEMORY_AllocDMatrix(__func__, "A",1, num_eqs, 1, 1+poly_order);
  if (A == NULL) {
    return ERROR_ID_ALLOC;
  }

  int rc=ERROR_ID_NO;

  // Fill linear system matrix

  // first column: a_j ^0 = 1.0
  for (int j=1; j<=num_eqs; ++j) {
    A[1][j] = 1.0;
  }
  for (int i=2; i<=num_unknowns; ++i) {
    for (int j=1; j<=num_eqs; ++j) {
      A[i][j] = a[j]*A[i-1][j];
    }
  }

  // Solution of the system weighted by errors
  double *b_sigma = NULL;
  if (sigma!=NULL) {
    b_sigma = new double[num_eqs];
    if (b_sigma == NULL) {
      rc = ERROR_ID_ALLOC;
      goto cleanup;
    }
    b_sigma -= 1; // linear fit routines expect b indices to run from 1..num_eqs
    for (int j=1; j<=num_eqs; ++j) {
      b_sigma[j]/=b[j]/sigma[j];
    }
  }

  linsys = LINEAR_from_matrix((const double *const *)A, num_eqs, num_unknowns, DECOMP_QR);
  LINEAR_set_weight(linsys, sigma);
  // decomposition and solution
  rc = LINEAR_decompose(linsys, NULL, NULL);
  if (rc != ERROR_ID_NO)
    goto cleanup;
  rc = LINEAR_solve(linsys, sigma != NULL? b_sigma : b, x);

 cleanup:
  MEMORY_ReleaseDMatrix(__func__, "A", A, 1, 1);
  LINEAR_free(linsys);
  delete[] b_sigma;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop((char *)__func__,rc);
  #endif
  
  return rc;
}

double LINEAR_GetNorm(const struct linear_system *s,int indexNorm)
 {
  return s->norms[indexNorm];
 }
