/*
 *  R : A Computer Language for Statistical Data Analysis

 *  Copyright (C) 1999        The R Development Core Team
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

#include "Arith.h"
#include <math.h>

#ifndef min
#define min(a, b) ((a < b)?(a):(b))
#define max(a, b) ((a < b)?(b):(a))
#endif

#define my_isok(x) (!ISNA(x) & !ISNAN(x))

void
filter1(double *x, int *n, double *filter, int *nfilt, int *sides, 
	int *circular, double *out)
{
    int i, ii, j, nf=*nfilt, nn = *n, nshift;
    double z, tmp;

    if(*sides == 2) nshift = nf /2; else nshift = 0;
    if(!*circular) {
	for(i = 0; i < nn; i++) {
	    z = 0;
	    if(i + nshift - (nf - 1) < 0 || i + nshift >= nn) {
		out[i] = NA_REAL;
		continue;
	    }
	    for(j = max(0, nshift + i - nn); j < min(nf, i + nshift + 1) ; j++) {
		tmp = x[i + nshift - j];
		if(my_isok(tmp)) z += filter[j] * tmp;
		else { out[i] = NA_REAL; goto bad; }
	    }
	    out[i] = z;
	bad:
	}
    } else { /* circular */
	for(i = 0; i < nn; i++)
	{
	    z = 0;
	    for(j = 0; j < nf; j++) {
		ii = i + nshift - j;
		if(ii < 0) ii += nn;
		if(ii >= nn) ii -= nn;
		tmp = x[ii];
		if(my_isok(tmp)) z += filter[j] * tmp;
		else { out[i] = NA_REAL; goto bad2; }
	    }
	    out[i] = z;
	bad2:
	}	
    }
}

void
filter2(double *x, int *n, double *filter, int *nfilt, double *out)
{
    int i, j, nf = *nfilt;
    double sum, tmp;

    for(i = 0; i < *n; i++) {
	sum = x[i];
	for (j = 0; j < nf; j++) {
	    tmp = out[nf + i - j - 1];
	    if(my_isok(tmp)) sum += tmp * filter[j];
	    else { out[i] = NA_REAL; goto bad3; }
	}
	out[nf + i] = sum;
    bad3:    
    }
}

void
acf(double *x, int *n, int *nser, int *nlag, int *correlation, double *acf)
{
    int i, u, v, lag, nl = *nlag, nn=*n, ns = *nser, d1 = nl+1, d2 = ns*d1;
    double sum, se[nn];
    
    for(u = 0; u < ns; u++)
	for(v = 0; v < ns; v++)
	    for(lag = 0; lag <= nl; lag++) {
		sum = 0.0;
		for(i = 0; i < nn-lag; i++)
		    sum += x[i + nn*u] * x[i + lag + nn*v];
		acf[lag + d1*u + d2*v] = sum/nn;
	    }
    if(*correlation) {
	for(u = 0; u < ns; u++)
	    se[u] = sqrt(acf[0 + d1*u + d2*u]);
	for(u = 0; u < ns; u++)
	    for(v = 0; v < ns; v++)
		for(lag = 0; lag <= nl; lag++)
		    acf[lag + d1*u + d2*v] /= se[u]*se[v];
    }
}
