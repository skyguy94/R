/*
 *  R : A Computer Langage for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*	Spline Interpolation
 *	--------------------
 *	C code to perform spline fitting and interpolation.
 *	There is code here for:
 *
 *	1. Natural splines.
 *
 *	2. Periodic splines
 *
 *	3. Splines with end-conditions determined by fitting
 *	   cubics in the start and end intervals (Forsythe et al).
 *
 *
 *	Computational Techniques
 *	------------------------
 *	A special LU decomposition for symmetric tridiagonal matrices
 *	is used for all computations, except for periodic splines where
 *	Choleski is more efficient.
 */

#include "Mathlib.h"
#include <stdio.h>
#include <errno.h>

/*
 *	Natural Splines
 *	---------------
 *	Here the end-conditions are determined by setting the second
 *	derivative of the spline at the end-points to equal to zero.
 *
 *	There are n-2 unknowns (y[i]'' at x[2], ..., x[n-1]) and n-2
 *	equations to determine them.  Either Choleski or Gaussian
 *	elimination could be used.
 */

void natural_spline(int n, double *x, double *y, double *b, double *c, double *d)
{
	int nm1, i;
	double t;

	x--; y--; b--; c--; d--;

	if(n < 2) {
		errno = EDOM;
		return;
	}

	if(n < 3) {
		t = (y[2] - y[1]);
		b[1] = t / (x[2]-x[1]);
		b[2] = b[1];
		c[1] = c[2] = d[1] = d[2] = 0.0;
		return;
	}

	nm1 = n - 1;

	/* Set up the tridiagonal system */
	/* b = diagonal, d = offdiagonal, c = right hand side */

	d[1] = x[2] - x[1];
	c[2] = (y[2] - y[1])/d[1];
	for( i=2 ; i<n ; i++) {
		d[i] = x[i+1] - x[i];
		b[i] = 2.0 * (d[i-1] + d[i]);
		c[i+1] = (y[i+1] - y[i])/d[i];
		c[i] = c[i+1] - c[i];
	}

	/* Gaussian elimination */

	for(i=3 ; i<n ; i++) {
		t = d[i-1]/b[i-1];
		b[i] = b[i] - t*d[i-1];
		c[i] = c[i] - t*c[i-1];
	}

	/* Backward substitution */
	
	c[n-1] = c[n-1]/b[n-1];
	for(i=n-2 ; i>1 ; i--)
		c[i] = (c[i]-d[i]*c[i+1])/b[i];

	/* End conditions */

	c[1] = c[n] = 0.0;

	/* Get cubic coefficients */
	
	b[1] = (y[2] - y[1])/d[1] - d[i] * c[2];
	c[1] = 0.0;
	d[1] = c[2]/d[1];
	for(i=2 ; i<n ; i++) {
		b[i] = (y[i+1]-y[i])/d[i] - d[i]*(c[i+1]+2.0*c[i]);
		d[i] = (c[i+1]-c[i])/d[i];
		c[i] = 3.0*c[i];
	}
	b[n] = (y[n] - y[n-1])/d[n-1] + 2.0 * d[n-1] * c[n-1];
	c[n] = 0.0;
	d[n] = d[n-1];

	return;
}

/*
 *	Splines a la Forsythe Malcolm and Moler
 *	---------------------------------------
 *	In this case the end-conditions are determined by fitting
 *	cubic polynomials to the first and last 4 points and matching
 *	the third derivitives of the spline at the end-points to the
 *	third derivatives of these cubics at the end-points.
 */

void fmm_spline(int n, double *x, double *y, double *b, double *c, double *d)
{
	int nm1, i;
	double t;
	
	/* Adjustment for 1-based arrays */

	x--; y--; b--; c--; d--;
	
	if(n < 2) {
		errno = EDOM;
		return;
	}
	
	if(n < 3) {
		t = (y[2] - y[1]);
		b[1] = t / (x[2]-x[1]);
		b[2] = b[1];
		c[1] = c[2] = d[1] = d[2] = 0.0;
		return;
	}

	nm1 = n - 1;
	
	/* Set up tridiagonal system */
	/* b = diagonal, d = offdiagonal, c = right hand side */
	
	d[1] = (x[2] - x[1]);
	c[2] = (y[2] - y[1])/d[1];
	for(i=2 ; i<n ; i++) {
		d[i] = x[i+1] - x[i];
		b[i] = 2.0 * (d[i-1] + d[i]);
		c[i+1] = (y[i+1] - y[i])/d[i];
		c[i] = c[i+1] - c[i];
	}
	
	/* End conditions. */
	/* Third derivatives at x[0] and x[n-1] obtained */
	/* from divided differences */
	
	b[1] = -d[1];
	b[n] = -d[n-1];
	c[1] = c[n] = 0.0;
	if(n > 3) {
		c[1] = c[3]/(x[4]-x[2]) - c[2]/(x[3]-x[1]);
		c[n] = c[n-1]/(x[n] - x[n-2]) - c[n-2]/(x[n-1]-x[n-3]);
		c[1] = c[1]*d[1]*d[1]/(x[4]-x[1]);
		c[n] = -c[n]*d[n-1]*d[n-1]/(x[n]-x[n-3]);
	}

	/* Gaussian elimination */

	for(i=2 ; i<=n ; i++) {
		t = d[i-1]/b[i-1];
		b[i] = b[i] - t*d[i-1];
		c[i] = c[i] - t*c[i-1];
	}
	
	/* Backward substitution */
	
	c[n] = c[n]/b[n];
	for(i=n-1 ; i>=1 ; i--)
		c[i] = (c[i]-d[i]*c[i+1])/b[i];
		
	/* c[i] is now the sigma[i-1] of the text */
	/* Compute polynomial coefficients */
	
	for(i=1 ; i<=nm1 ; i++) {
		b[i] = (y[i+1]-y[i])/d[i] - d[i]*(c[i+1]+2.0*c[i]);
		d[i] = (c[i+1]-c[i])/d[i];
		c[i] = 3.0*c[i];
	}
	b[n] = (y[n] - y[n-1])/d[n-1] + d[n-1]*(c[n-1]+ 2.0*c[n]);
	c[n] = 3.0*c[n];
	d[n] = d[n-1];
	return;
}


/*
 *	Periodic Spline
 *	---------------
 *	The end conditions here match spline (and its derivatives)
 *	at x[1] and x[n].
 *
 *	Note: There is an explicit check that the user has supplied
 *	data with y[1] equal to y[n].
 */

void periodic_spline(int n, double *x, double *y, double *b, double *c, double *d, double *e)
{
	double s;
	int i, nm1;
	
	/* Adjustment for 1-based arrays */

	x--; y--; b--; c--; d--; e--;

	if(n < 2 || y[1] != y[n]) {
		errno = EDOM;
		return;
	}

	nm1 = n-1;
	
	/* Set up the matrix system */
	/* A = diagonal  B = off-diagonal  C = rhs */

#define A	b
#define B	d
#define C	c
	
	B[1] = x[2] - x[1];
	B[n-1] = (x[n] - x[n-1]);
	A[1] = 2.0 * (B[1] + (x[n-1] - x[n-2]));
	C[1] = (y[2] - y[1])/B[1] - (y[n] - y[n-1])/B[n-1];
	
	for(i=2 ; i<n ; i++) {
		B[i] = x[i+1] - x[i];
		A[i] = 2.0 * (B[i] + B[i-1]);
		C[i] = (y[i+1] - y[i])/B[i] - (y[i] - y[i-1])/B[i-1];
	}

	/* Choleski decomposition */
		
#define L	b
#define M	d
#define E	e

	L[1] = sqrt(A[1]);
	E[1] = (x[n] - x[nm1])/L[1];
	s = 0.0;
	for(i=1 ; i<=nm1-2; i++) {
		M[i] = B[i]/L[i];
		if(i != 1) E[i] = -E[i-1] * M[i-1] / L[i];
		L[i+1] = sqrt(A[i+1]-M[i]*M[i]);
		s = s + E[i]*E[i];
	}
	M[nm1-1] = (B[nm1-1] - E[nm1-2] * M[nm1-2])/L[nm1-1];
	L[nm1] = sqrt(A[nm1] - M[nm1-1]*M[nm1-1] - s);

		/* Forward Elimination */

#define Y	c
#define D	c
	
	Y[1] = D[1]/L[1];
	s = 0.0;
	for(i=2 ; i<=nm1-1 ; i++) {
		Y[i] = (D[i] - M[i-1]*Y[i-1])/L[i];
		s = s + E[i-1] * Y[i-1];
	}
	Y[nm1] = (D[nm1] - M[nm1-1] * Y[nm1-1] - s) / L[nm1];
	
#define X	c

	/*
	X[nm1] = -Y[nm1]/L[nm1];
	X[nm1-1] = -(Y[nm1-1] + M[nm1-1] * X[nm1])/L[nm1-1];
	for(i=nm1-2 ; i>=1 ; i--)
		X[i] = -(Y[i] + M[i] * X[i+1] + E[i] * X[nm1])/L[i];
	*/

	X[nm1] = Y[nm1]/L[nm1];
	X[nm1-1] = (Y[nm1-1] - M[nm1-1] * X[nm1])/L[nm1-1];
	for(i=nm1-2 ; i>=1 ; i--)
		X[i] = (Y[i] - M[i] * X[i+1] - E[i] * X[nm1])/L[i];
	
	/* Compute polynomial coefficients */ 
	
	for(i=1 ; i<=nm1 ; i++) {
		s = x[i+1] - x[i];
		b[i] = (y[i+1]-y[i])/s - s*(c[i+1]+2.0*c[i]);
		d[i] = (c[i+1]-c[i])/s;
		c[i] = 3.0*c[i];
	}
	b[n] = b[1];
	c[n] = c[1];
	d[n] = d[1];
	return;
}

int spline_coef(int *method, int *n, double *x, double *y,
double *b, double *c, double *d, double *e)
{
	switch(*method) {
		case 1:
			periodic_spline(*n, x, y, b, c, d, e);
			break;

		case 2:
			natural_spline(*n, x, y, b, c, d);
			break;

		case 3:
			fmm_spline(*n, x, y, b, c, d);
			break;
	}
	return 0;
}

int spline_eval(int *method, int *nu, double *u, double *v,
	int *n, double *x, double *y, double *b, double *c, double *d)
{
	int i, j, k, l;
	double ul, dx;

	u--; v--;
	x--; y--;
	b--; c--; d--;

	if(*method == 1) {
		dx = x[*n] - x[1];
		for( l=1 ; l<=*nu ; l++) { 
			v[l] = fmod(u[l]-x[1], dx);
			if(v[l] < 0.0) v[l] += dx;
			v[l] = v[l] + x[1];
		}
	}
	else {
		for( l=1 ; l<=*nu ; l++)
			v[l] = u[l];
	}

	i = 1;
	for( l=1 ; l<=*nu ; l++) {
		ul = v[l];
		if(ul < x[i] || x[i+1] < ul) {
			i = 1;
			j = *n + 1;
			do {
				k = (i+j)/2;
				if(ul < x[k]) j = k;
				else i = k;
			}
			while(j > i+1);
		}
		dx = ul - x[i];
		v[l] = y[i] + dx*(b[i] + dx*(c[i] + dx*d[i]));
	}
	return 0;
}
