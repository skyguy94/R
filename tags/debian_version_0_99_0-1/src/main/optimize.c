/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2000  Robert Gentleman, Ross Ihaka and the
 *                            R Development Core Team
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Defn.h"
#include "Print.h"		/* for printRealVector() */
#include "Mathlib.h"
#include "Applic.h"
#include "RS.h"			/* for Memcpy */


/* One Dimensional Minimization --- just wrapper code for Brent's "fmin" */

struct callinfo {
  SEXP R_fcall;
  SEXP R_env;
} ;

/*static SEXP R_fcall1;
  static SEXP R_env1; */

static double fcn1(double x, struct callinfo *info)
{
    SEXP s;
    REAL(CADR(info->R_fcall))[0] = x;
    s = eval(info->R_fcall, info->R_env);
    switch(TYPEOF(s)) {
    case INTSXP:
	if (length(s) != 1) goto badvalue;
	if (INTEGER(s)[0] == NA_INTEGER) {
	    warning("NA replaced by maximum positive value");
	    return DBL_MAX;
	}
	else return INTEGER(s)[0];
	break;
    case REALSXP:
	if (length(s) != 1) goto badvalue;
	if (!R_FINITE(REAL(s)[0])) {
	    warning("NA/Inf replaced by maximum positive value");
	    return DBL_MAX;
	}
	else return REAL(s)[0];
	break;
    default:
	goto badvalue;
    }
 badvalue:
    error("invalid function value in 'optimize'");
    return 0;/* for -Wall */
}

/* fmin(f, xmin, xmax tol) */
SEXP do_fmin(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    double xmin, xmax, tol;
    SEXP v, res;
    struct callinfo info;

    checkArity(op, args);
    PrintDefaults(rho);

    /* the function to be minimized */

    v = CAR(args);
    if (!isFunction(v))
	errorcall(call, "attempt to minimize non-function");
    args = CDR(args);

    /* xmin */

    xmin = asReal(CAR(args));
    if (!R_FINITE(xmin))
	errorcall(call, "invalid xmin value");
    args = CDR(args);

    /* xmax */

    xmax = asReal(CAR(args));
    if (!R_FINITE(xmax))
	errorcall(call, "invalid xmax value");
    if (xmin >= xmax)
	errorcall(call, "xmin not less than xmax");
    args = CDR(args);

    /* tol */

    tol = asReal(CAR(args));
    if (!R_FINITE(tol) || tol <= 0.0)
	errorcall(call, "invalid tol value");

    info.R_env = rho;
    PROTECT(info.R_fcall = lang2(v, R_NilValue));
    PROTECT(res = allocVector(REALSXP, 1));
    CADR(info.R_fcall) = allocVector(REALSXP, 1);
    REAL(res)[0] = Brent_fmin(xmin, xmax, 
			      (double (*)(double, void*)) fcn1, &info, tol);
    UNPROTECT(2);
    return res;
}



/* One Dimensional Root Finding --  just wrapper code for Brent's "zeroin" */


static double fcn2(double x, struct callinfo *info)
{
    SEXP s;
    REAL(CADR(info->R_fcall))[0] = x;
    s = eval(info->R_fcall, info->R_env);
    switch(TYPEOF(s)) {
    case INTSXP:
	if (length(s) != 1) goto badvalue;
	if (INTEGER(s)[0] == NA_INTEGER) {
	    warning("NA replaced by maximum positive value");
	    return	DBL_MAX;
	}
	else return INTEGER(s)[0];
	break;
    case REALSXP:
	if (length(s) != 1) goto badvalue;
	if (!R_FINITE(REAL(s)[0])) {
	    warning("NA/Inf replaced by maximum positive value");
	    return DBL_MAX;
	}
	else return REAL(s)[0];
	break;
    default:
	goto badvalue;
    }
 badvalue:
    error("invalid function value in 'zeroin'");
    return 0;/* for -Wall */

}

/* zeroin(f, xmin, xmax, tol, maxiter) */
SEXP do_zeroin(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    double xmin, xmax, tol;
    int iter;
    SEXP v, res;
    struct callinfo info;

    checkArity(op, args);
    PrintDefaults(rho);

    /* the function to be minimized */

    v = CAR(args);
    if (!isFunction(v))
	errorcall(call,"attempt to minimize non-function");
    args = CDR(args);

    /* xmin */

    xmin = asReal(CAR(args));
    if (!R_FINITE(xmin))
	errorcall(call, "invalid xmin value");
    args = CDR(args);

    /* xmax */

    xmax = asReal(CAR(args));
    if (!R_FINITE(xmax))
	errorcall(call, "invalid xmax value");
    if (xmin >= xmax)
	errorcall(call, "xmin not less than xmax");
    args = CDR(args);

    /* tol */

    tol = asReal(CAR(args));
    if (!R_FINITE(tol) || tol <= 0.0)
	errorcall(call, "invalid tol value");
    args = CDR(args);

    /* maxiter */
    iter = asInteger(CAR(args));
    if (iter <= 0)
	errorcall(call, "maxiter must be positive");

    info.R_env = rho;
    PROTECT(info.R_fcall = lang2(v, R_NilValue)); /* the info used in fcn2() */
    CADR(info.R_fcall) = allocVector(REALSXP, 1);
    PROTECT(res = allocVector(REALSXP, 3));
    REAL(res)[0] =
	R_zeroin(xmin, xmax,   (double (*)(double, void*)) fcn2,
		 (void *) &info, &tol, &iter);
    REAL(res)[1] = (double)iter;
    REAL(res)[2] = tol;
    UNPROTECT(2);
    return res;
}



/* General Nonlinear Optimization */

#define FT_SIZE 5		/* default size of table to store computed
				   function values */

typedef struct {
  double   fval;
  double  *x;
  double  *grad;
  double  *hess;
} ftable;

typedef struct {
  SEXP R_fcall;       /* unevaluated call to R function */
  SEXP R_env;         /* where to evaluate the calls */
  int have_gradient;
  int have_hessian;
/*  int n;	      -* length of the parameter (x) vector */
  int FT_size;        /* size of table to store computed
			 function values */
  int FT_last;        /* Newest entry in the table */
  ftable *Ftable;
} function_info;

/* Initialize the storage in the table of computed function values */

static void FT_init(int n, int FT_size, function_info *state)
{
    int i, j;
    int have_gradient, have_hessian;
    ftable *Ftable;

    have_gradient = state->have_gradient;
    have_hessian = state->have_hessian;

    Ftable = (ftable *)R_alloc(FT_size, sizeof(ftable));

    for (i = 0; i < FT_size; i++) {
	Ftable[i].x = (double *)R_alloc(n, sizeof(double));
				/* initialize to unlikely parameter values */
	for (j = 0; j < n; j++) {
	    Ftable[i].x[j] = DBL_MAX;
	}
	if (have_gradient) {
	    Ftable[i].grad = (double *)R_alloc(n, sizeof(double));
	    if (have_hessian) {
		Ftable[i].hess = (double *)R_alloc(n * n, sizeof(double));
	    }
	}
    }
    state->Ftable = Ftable;
    state->FT_size = FT_size;
    state->FT_last = -1;
}

/* Store an entry in the table of computed function values */

static void FT_store(int n, const double f, const double *x, const double *grad,
		     const double *hess, function_info *state)
{
    int ind;

    ind = (++(state->FT_last)) % (state->FT_size);
    state->Ftable[ind].fval = f;
    Memcpy(state->Ftable[ind].x, x, n);
    if (grad) {
        Memcpy(state->Ftable[ind].grad, grad, n);
	if (hess) {
	    Memcpy(state->Ftable[ind].hess, hess, n * n);
	}
    }
}

/* Check for stored values in the table of computed function values.
   Returns the index in the table or -1 for failure */

static int FT_lookup(int n, const double *x, function_info *state)
{
    double *ftx;
    int i, j, ind, matched;
    int FT_size, FT_last;
    ftable *Ftable;
    
    FT_last = state->FT_last;
    FT_size = state->FT_size;
    Ftable = state->Ftable;

    for (i = 0; i < FT_size; i++) {
	ind = (FT_last - i) % FT_size;
				/* why can't they define modulus correctly */
	if (ind < 0) ind += FT_size;
	ftx = Ftable[ind].x;
	if (ftx) {
	    matched = 1;
	    for (j = 0; j < n; j++) {
	        if (x[j] != ftx[j]) {
	            matched = 0;
		    break;
	        }
	    }
	    if (matched) return ind;
        }
    }
    return -1;
}

/* This how the optimizer sees them */

static void fcn(int n, const double x[], double *f, function_info
		*state)
{
    SEXP s, R_fcall;
    ftable *Ftable;
    double *g = (double *) 0, *h = (double *) 0;
    int i;

    R_fcall = state->R_fcall;
    Ftable = state->Ftable;
    if ((i = FT_lookup(n, x, state)) >= 0) {
	*f = Ftable[i].fval;
	return;
    }
				/* calculate for a new value of x */
    s = CADR(R_fcall);
    for (i = 0; i < n; i++)
	REAL(s)[i] = x[i];
    s = eval(state->R_fcall, state->R_env);
    switch(TYPEOF(s)) {
    case INTSXP:
	if (length(s) != 1) goto badvalue;
	if (INTEGER(s)[0] == NA_INTEGER) {
	    warning("NA replaced by maximum positive value");
	    *f = DBL_MAX;
	}
	else *f = INTEGER(s)[0];
	break;
    case REALSXP:
	if (length(s) != 1) goto badvalue;
	if (!R_FINITE(REAL(s)[0])) {
	    warning("NA/Inf replaced by maximum positive value");
	    *f = DBL_MAX;
	}
	else *f = REAL(s)[0];
	break;
    default:
	goto badvalue;
    }
    if (state->have_gradient) {
	g = REAL(coerceVector(getAttrib(s, install("gradient")), REALSXP));
	if (state->have_hessian) {
	    h = REAL(coerceVector(getAttrib(s, install("hessian")), REALSXP));
	}
    }
    FT_store(n, *f, x, g, h, state);
    return;

 badvalue:
    error("invalid function value in 'nlm' optimizer");
}


static void Cd1fcn(int n, const double x[], double *g, function_info *state)
{
    int ind;

    if ((ind = FT_lookup(n, x, state)) < 0) {	/* shouldn't happen */
	fcn(n, x, g, state);
	if ((ind = FT_lookup(n, x, state)) < 0) {
	    error("function value caching for optimization is seriously confused.\n");
	}
    }
    Memcpy(g, state->Ftable[ind].grad, n);
}


static void Cd2fcn(int nr, int n, const double x[], double *h,
		   function_info *state)
{
    int j, ind;

    if ((ind = FT_lookup(n, x, state)) < 0) {	/* shouldn't happen */
	fcn(n, x, h, state);
	if ((ind = FT_lookup(n, x, state)) < 0) {
	    error("function value caching for optimization is seriously confused.\n");
	}
    }
    for (j = 0; j < n; j++) {  /* fill in lower triangle only */
        Memcpy( h + j*(n + 1), state->Ftable[ind].hess + j*(n + 1), n - j);
    }
}


static double *fixparam(SEXP p, int *n, SEXP call)
{
    double *x;
    int i;

    if (!isNumeric(p))
	errorcall(call, "numeric parameter expected");

    if (*n) {
	if (LENGTH(p) != *n)
	    errorcall(call, "conflicting parameter lengths");
    }
    else {
	if (LENGTH(p) <= 0)
	    errorcall(call, "invalid parameter length");
	*n = LENGTH(p);
    }

    x = (double*)R_alloc(*n, sizeof(double));
    switch(TYPEOF(p)) {
    case LGLSXP:
    case INTSXP:
	for (i = 0; i < *n; i++) {
	    if (INTEGER(p)[i] == NA_INTEGER)
		errorcall(call, "missing value in parameter");
	    x[i] = INTEGER(p)[i];
	}
	break;
    case REALSXP:
	for (i = 0; i < *n; i++) {
	    if (!R_FINITE(REAL(p)[i]))
		errorcall(call, "missing value in parameter");
	    x[i] = REAL(p)[i];
	}
	break;
    default:
	errorcall(call, "invalid parameter type");
    }
    return x;
}


static void invalid_na(SEXP call)
{
    errorcall(call, "invalid NA value in parameter");
}


	/* Fatal errors - we don't deliver an answer */

static void opterror(int nerr)
{
    switch(nerr) {
    case -1:
	error("non-positive number of parameters in nlm");
    case -2:
	error("nlm is inefficient for 1-d problems");
    case -3:
	error("illegal gradient tolerance in nlm");
    case -4:
	error("illegal iteration limit in nlm");
    case -5:
	error("minimization function has no good digits in nlm");
    case -6:
	error("no analytic gradient to check in nlm!");
    case -7:
	error("no analytic Hessian to check in nlm!");
    case -21:
	error("probable coding error in analytic gradient");
    case -22:
	error("probable coding error in analytic Hessian");
    default:
	error("*** unknown error message (msg = %d) in nlm()\n*** should not happen!", nerr);
    }
}


	/* Warnings - we return a value, but print a warning */

static void optcode(int code)
{
    switch(code) {
    case 1:
	Rprintf("Relative gradient close to zero.\n");
	Rprintf("Current iterate is probably solution.\n");
	break;
    case 2:
	Rprintf("Successive iterates within tolerance.\n");
	Rprintf("Current iterate is probably solution.\n");
	break;
    case 3:
	Rprintf("Last global step failed to locate a point lower than x.\n");
	Rprintf("Either x is an approximate local minimum of the function,\n");
	Rprintf("the function is too non-linear for this algorithm,\n");
	Rprintf("or steptol is too large.\n");
	break;
    case 4:
	Rprintf("Iteration limit exceeded.  Algorithm failed.\n");
	break;
    case 5:
	Rprintf("Maximum step size exceeded 5 consecutive times.\n");
	Rprintf("Either the function is unbounded below,\n");
	Rprintf("becomes asymptotic to a finite value\n");
	Rprintf("from above in some direction,\n");
	Rprintf("or stepmx is too small.\n");
	break;
    }
    Rprintf("\n");
}

SEXP do_nlm(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP value, names, v, R_gradientSymbol, R_hessianSymbol;

    double *x, *typsiz, fscale, gradtl, stepmx,
	steptol, *xpls, *gpls, fpls, *a, *wrk, dlt;

    int code, i, j, k, itnlim, method, iexp, omsg, msg,
	n, ndigit, iagflg, iahflg, want_hessian, itncnt;

    char *vmax;

    function_info *state;

    checkArity(op, args);
    PrintDefaults(rho);
    vmax = vmaxget();

    state = (function_info *) R_alloc(1, sizeof(function_info));

    /* the function to be minimized */

    state->R_env = rho;
    v = CAR(args);
    if (!isFunction(v))
	error("attempt to minimize non-function");
    PROTECT(state->R_fcall = lang2(v, R_NilValue));
    args = CDR(args);

    /* inital parameter value */

    n = 0;
    x = fixparam(CAR(args), &n, call);
    args = CDR(args);

    /* hessian required? */

    want_hessian = asLogical(CAR(args));
    if (want_hessian == NA_LOGICAL) want_hessian = 0;
    args = CDR(args);

    /* typical size of parameter elements */

    typsiz = fixparam(CAR(args), &n, call);
    args = CDR(args);

    /* expected function size */

    fscale = asReal(CAR(args));
    if (ISNA(fscale)) invalid_na(call);
    args = CDR(args);

    omsg = msg = asInteger(CAR(args));
    if (msg == NA_INTEGER) invalid_na(call);
    args = CDR(args);

    ndigit = asInteger(CAR(args));
    if (ndigit == NA_INTEGER) invalid_na(call);
    args = CDR(args);

    gradtl = asReal(CAR(args));
    if (ISNA(gradtl)) invalid_na(call);
    args = CDR(args);

    stepmx = asReal(CAR(args));
    if (ISNA(stepmx)) invalid_na(call);
    args = CDR(args);

    steptol = asReal(CAR(args));
    if (ISNA(steptol)) invalid_na(call);
    args = CDR(args);

    itnlim = asInteger(CAR(args));
    if (itnlim == NA_INTEGER) invalid_na(call);
    args = CDR(args);

    /* force one evaluation to check for the gradient and hessian */
    iagflg = 0;			/* No analytic gradient */
    iahflg = 0;			/* No analytic hessian */
    state->have_gradient = 0;
    state->have_hessian = 0;
    R_gradientSymbol = install("gradient");
    R_hessianSymbol = install("hessian");

    v = allocVector(REALSXP, n);
    for (i = 0; i < n; i++) {
	REAL(v)[i] = x[i];
    }
    CADR(state->R_fcall) = v;
    value = eval(state->R_fcall, state->R_env);

    v = getAttrib(value, R_gradientSymbol);
    if (v != R_NilValue && LENGTH(v) == n && (isReal(v) || isInteger(v))) {
      iagflg = 1;
      state->have_gradient = 1;
       v = getAttrib(value, R_hessianSymbol);
       if (v != R_NilValue && LENGTH(v) == (n * n) &&
 	  (isReal(v) || isInteger(v))) {
 	iahflg = 1;
 	state->have_hessian = 1;
       }
    }
    if (((msg/4) % 2) && !iahflg) { /* skip check of analytic Hessian */
      msg -= 4;
    }
    if (((msg/2) % 2) && !iagflg) { /* skip check of analytic gradient */
      msg -= 2;
    }
    FT_init(n, FT_SIZE, state);
    /* Plug in the call to the optimizer here */

    method = 1;	/* Line Search */
    iexp = iahflg ? 0 : 1; /* Function calls are expensive */
    dlt = 1.0;

    xpls = (double*)R_alloc(n, sizeof(double));
    gpls = (double*)R_alloc(n, sizeof(double));
    a = (double*)R_alloc(n*n, sizeof(double));
    wrk = (double*)R_alloc(8*n, sizeof(double));

    /*
     *	 Dennis + Schnabel Minimizer
     *
     *	  SUBROUTINE OPTIF9(NR,N,X,FCN,D1FCN,D2FCN,TYPSIZ,FSCALE,
     *	 +	   METHOD,IEXP,MSG,NDIGIT,ITNLIM,IAGFLG,IAHFLG,IPR,
     *	 +	   DLT,GRADTL,STEPMX,STEPTOL,
     *	 +	   XPLS,FPLS,GPLS,ITRMCD,A,WRK)
     *
     *
     *	 Note: I have figured out what msg does.
     *	 It is actually a sum of bit flags as follows
     *	   1 = don't check/warn for 1-d problems
     *	   2 = don't check analytic gradients
     *	   4 = don't check analytic hessians
     *	   8 = don't print start and end info
     *	  16 = print at every iteration
     *	 Using msg=9 is absolutely minimal
     *	 I think we always check gradients and hessians
     */

    optif9(n, n, x, (fcn_p) fcn, (fcn_p) Cd1fcn, (d2fcn_p) Cd2fcn,
	   state, typsiz, fscale, method, iexp, &msg, ndigit, itnlim,
	   iagflg, iahflg, dlt, gradtl, stepmx, steptol, xpls, &fpls,
	   gpls, &code, a, wrk, &itncnt);

    if (msg < 0)
	opterror(msg);
    if (code != 0 && (omsg&8) == 0)
	optcode(code);

    if (want_hessian) {
	PROTECT(value = allocVector(VECSXP, 6));
	PROTECT(names = allocVector(STRSXP, 6));
	fdhess(n, xpls, fpls, (fcn_p) fcn, state, a, n, &wrk[0], &wrk[n],
	       ndigit, typsiz);
	for (i = 0; i < n; i++)
	    for (j = 0; j < i; j++)
		a[i + j * n] = a[j + i * n];
    }
    else {
	PROTECT(value = allocVector(VECSXP, 5));
	PROTECT(names = allocVector(STRSXP, 5));
    }
    k = 0;

    STRING(names)[k] = mkChar("minimum");
    VECTOR(value)[k] = allocVector(REALSXP, 1);
    REAL(VECTOR(value)[k])[0] = fpls;
    k++;

    STRING(names)[k] = mkChar("estimate");
    VECTOR(value)[k] = allocVector(REALSXP, n);
    for (i = 0; i < n; i++)
	REAL(VECTOR(value)[k])[i] = xpls[i];
    k++;

    STRING(names)[k] = mkChar("gradient");
    VECTOR(value)[k] = allocVector(REALSXP, n);
    for (i = 0; i < n; i++)
	REAL(VECTOR(value)[k])[i] = gpls[i];
    k++;

    if (want_hessian) {
	STRING(names)[k] = mkChar("hessian");
	VECTOR(value)[k] = allocMatrix(REALSXP, n, n);
	for (i = 0; i < n * n; i++)
	    REAL(VECTOR(value)[k])[i] = a[i];
	k++;
    }

    STRING(names)[k] = mkChar("code");
    VECTOR(value)[k] = allocVector(INTSXP, 1);
    INTEGER(VECTOR(value)[k])[0] = code;
    k++;

    /* added by Jim K Lindsey */
    STRING(names)[k] = mkChar("iterations");
    VECTOR(value)[k] = allocVector(INTSXP, 1);
    INTEGER(VECTOR(value)[k])[0] = itncnt;
    k++;

    setAttrib(value, R_NamesSymbol, names);
    vmaxset(vmax);
    UNPROTECT(3);
    return value;
}
