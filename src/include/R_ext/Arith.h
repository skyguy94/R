/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--1999  The R Development Core Team.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef R_EXT_ARITH_H_
#define R_EXT_ARITH_H_
#include <math.h>

extern double	R_NaN;		/* IEEE NaN or -DBL_MAX */
extern double	R_PosInf;	/* IEEE Inf or DBL_MAX */
extern double	R_NegInf;	/* IEEE -Inf or -DBL_MAX */
extern int	R_NaInt;	/* NA_INTEGER etc */
extern double	R_NaReal;	/* NA_REAL */

#define NA_LOGICAL	R_NaInt
#define NA_INTEGER	R_NaInt
#define NA_FACTOR	R_NaInt
#define NA_REAL		R_NaReal
/* NA_STRING is a SEXP, so defined in Rinternals.h */

int R_IsNA(double);		/* True for Real NA only */
int R_IsNaN(double);		/* True for NaN, *not* for NA */
int R_IsNaNorNA(double);        /* True for both */
int R_finite(double);

#define ISNA(x)		R_IsNA(x)
#define ISNAN(x)	R_IsNaNorNA(x)
#define R_FINITE(x)	R_finite(x)

#endif
