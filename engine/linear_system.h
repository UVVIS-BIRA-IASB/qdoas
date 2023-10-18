/* Copyright (C) 2017 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef LINEAR_SYSTEM_H
#define LINEAR_SYSTEM_H

// Interface for linear least squares fitting
//
// for a system m equations and n unknowns x, represented by the
// matrix equation
//
//  A * x = b,
//
// where A is an (m x n) matrix and b and m-vectro, with m >= n,
// calculate the linear least squares solution x which minimizes
//
//  || A*x - b ||
//
// a linear system can be created from an existing matrix using
// LINEAR_from_matrix(), or it can be allocated using LINEAR_alloc()
// and LINEAR_set_column() (making sure each column is set)
//
// weights can be applied using linear_set_weight()
//
// before calling LINEAR_solve, the user must call LINEAR_decompose
//
// Order of calls is important,  an example of the full sequence of calls is the following:
//
// struct linear_sytem *s = LINEAR_alloc(....) or LINEAR_from_matrix(...);
//
// LINEAR_set_column(s,...); // for each column you need to set
//
// LINEAR_set_weight(s,...); // optional
//
// LINEAR_decompose(s,...);
//
// LINEAR_solve(s,...)
//
// LINEAR_free(s);

enum linear_fit_mode {
  DECOMP_SVD,
  DECOMP_EIGEN_QR
};

struct linear_system;

// allocate a linear fitting environment for m equations and n unknowns
// memory must be freed with LINEAR_free
struct linear_system *LINEAR_alloc(int m, int n, enum linear_fit_mode);

// create a linear fitting environment from a matix a[1..n][1..m]
// memory must be freed with LINEAR_free
struct linear_system *LINEAR_from_matrix(const double *const *a, int m, int n, enum linear_fit_mode mode);

// assign a vector to column n of the linear system.
// col[1..m]:
void LINEAR_set_column(struct linear_system *s, int n, const double *col);

// weigh the vectors of the linear system by sigma.
// sigma[0..m-1]: vector of weights
//
// TODO: store weight vector in linear system and automatically apply to right hand side when solving A*x=b?
void LINEAR_set_weight(struct linear_system *s, const double *sigma);

// release memory
void LINEAR_free(struct linear_system *s);

// perform SVD or QR decomposition
int LINEAR_decompose(struct linear_system *s, double *sigmasquare, double **covar);

// use the decomposition to fit A*x=b
// b[1..num_eqs], x[1..num_unknowns]
int LINEAR_solve(const struct linear_system *s, const double *b, double *x);

// fit a polynomial of order "poly_order" through "num_eqs" points
// (a_i, b_i), weighted by errors sigma_i
//
//   1 + x_1 * a_i + *x_2 * (a_i)^2 + ... = b_i
//
// a[1..num_eqs], b[1..num_eqs], x[1..(1+poly_order)]
int LINEAR_fit_poly(int num_eqs, int poly_order, const double *a, const double *sigma, const double *b, double *x);

// calculate Moore-Penrose pseudo-inverse
void LINEAR_pinv(const struct linear_system *s, double **m);

double LINEAR_GetNorm(const struct linear_system *s,int indexNorm);

#endif
