#  File src/library/base/R/all.equal.R
#  Part of the R package, http://www.R-project.org
#
#  Copyright (C) 1995-2014 The R Core Team
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  A copy of the GNU General Public License is available at
#  http://www.r-project.org/Licenses/

all.equal <- function(target, current, ...) UseMethod("all.equal")

all.equal.default <-
    function(target, current, ...)
{
    ## Really a dispatcher given mode() of args :
    ## use data.class as unlike class it does not give "integer"
    if(is.language(target) || is.function(target))
	return(all.equal.language(target, current, ...))
    if(is.environment(target) || is.environment(current))# both: unclass() fails on env.
	return(all.equal.environment(target, current, ...))
    if(is.recursive(target))
	return(all.equal.list(target, current, ...))
    msg <- switch (mode(target),
                   integer = ,
                   complex = ,
                   numeric = all.equal.numeric(target, current, ...),
                   character = all.equal.character(target, current, ...),
                   logical = ,
                   raw = all.equal.raw(target, current, ...),
		   ## assumes that slots are implemented as attributes :
		   S4 = attr.all.equal(target, current, ...),
                   if(data.class(target) != data.class(current)) {
                       gettextf("target is %s, current is %s",
                                data.class(target), data.class(current))
                   } else NULL)
    if(is.null(msg)) TRUE else msg
}

all.equal.numeric <-
    function(target, current, tolerance = .Machine$double.eps ^ .5,
             scale = NULL, ..., check.attributes = TRUE)
{
    if (!is.numeric(tolerance))
        stop("'tolerance' should be numeric")
    if (!is.numeric(scale) && !is.null(scale))
        stop("'scale' should be numeric or NULL")
    if (!is.logical(check.attributes))
        stop(gettextf("'%s' must be logical", "check.attributes"), domain = NA)
    msg <- if(check.attributes)
	attr.all.equal(target, current, tolerance = tolerance, scale = scale,
                       ...)
    if(data.class(target) != data.class(current)) {
	msg <- c(msg, paste0("target is ", data.class(target), ", current is ",
                             data.class(current)))
	return(msg)
    }

    lt <- length(target)
    lc <- length(current)
    cplx <- is.complex(target) # and so current must be too.
    if(lt != lc) {
	## *replace* the 'Lengths' msg[] from attr.all.equal():
	if(!is.null(msg)) msg <- msg[- grep("\\bLengths\\b", msg)]
	msg <- c(msg, paste0(if(cplx) "Complex" else "Numeric",
                             ": lengths (", lt, ", ", lc, ") differ"))
	return(msg)
    }
    ## remove atttributes (remember these are both numeric or complex vectors)
    ## one place this is needed is to unclass Surv objects in the rpart test suite.
    target <- as.vector(target)
    current <- as.vector(current)
    out <- is.na(target)
    if(any(out != is.na(current))) {
	msg <- c(msg, paste("'is.NA' value mismatch:", sum(is.na(current)),
			    "in current", sum(out), "in target"))
	return(msg)
    }
    out <- out | target == current
    if(all(out)) { if (is.null(msg)) return(TRUE) else return(msg) }

    target <- target[!out]
    current <- current[!out]
    if(is.integer(target) && is.integer(current)) target <- as.double(target)
    xy <- mean((if(cplx) Mod else abs)(target - current))
    what <-
	if(is.null(scale)) {
	    xn <- mean(abs(target))
	    if(is.finite(xn) && xn > tolerance) {
		xy <- xy/xn
		"relative"
	    } else "absolute"
	} else {
	    xy <- xy/scale
	    if(scale == 1) "absolute" else "scaled"
	}

    if (cplx) what <- paste(what, "Mod") # PR#10575
    if(is.na(xy) || xy > tolerance)
        msg <- c(msg, paste("Mean", what, "difference:", format(xy)))

    if(is.null(msg)) TRUE else msg
}

all.equal.character <-
    function(target, current, ..., check.attributes = TRUE)
{
    if (!is.logical(check.attributes))
        stop(gettextf("'%s' must be logical", "check.attributes"), domain = NA)
    msg <-  if(check.attributes) attr.all.equal(target, current, ...)
    if(data.class(target) != data.class(current)) {
	msg <- c(msg, paste0("target is ", data.class(target), ", current is ",
                             data.class(current)))
	return(msg)
    }
    lt <- length(target)
    lc <- length(current)
    if(lt != lc) {
	if(!is.null(msg)) msg <- msg[- grep("\\bLengths\\b", msg)]
	msg <- c(msg,
                 paste0("Lengths (", lt, ", ", lc,
                        ") differ (string compare on first ",
                        ll <- min(lt, lc), ")"))
	ll <- seq_len(ll)
	target <- target[ll]
	current <- current[ll]
    }
    nas <- is.na(target); nasc <- is.na(current)
    if (any(nas != nasc)) {
	msg <- c(msg, paste("'is.NA' value mismatch:", sum(nasc),
                            "in current", sum(nas), "in target"))
	return(msg)
    }
    ne <- !nas & (target != current)
    if(!any(ne) && is.null(msg)) TRUE
    else if(sum(ne) == 1L) c(msg, paste("1 string mismatch"))
    else if(sum(ne) > 1L) c(msg, paste(sum(ne), "string mismatches"))
    else msg
}

## In 'base' these are all visible, so need to test both args:

all.equal.envRefClass <- function (target, current, all.names=NA, ...) {
    if(!is (target, "envRefClass")) return("'target' is not an envRefClass")
    if(!is(current, "envRefClass")) return("'current' is not an envRefClass")
    if(!isTRUE(ae <- all.equal(class(target), class(current), ...)) ||
       !identical(cld <- target$getClass(), current$getClass()))
	return(sprintf("Classes differ%s",
		       if(!isTRUE(ae)) paste(":", ae, collapse=" ")else ""))
    flds <- names(cld@fieldClasses)
    asL <- function(O) sapply(flds, function(ch) O[[ch]], simplify = FALSE)
    ## ## ?setRefClass explicitly says users should not use ".<foo>" fields:
    ## if(is.na(all.names)) all.names <- FALSE
    ## ## try preventing infinite recursion by not looking at  .self :
    ## T <- function(ls) ls[is.na(match(names(ls), c(".self", methods:::.envRefMethods)))]
    ## asL <- function(E) T(as.list(as.environment(E), all.names=all.names, sorted=TRUE))
    n <- all.equal.list(asL(target), asL(current), ...)
    ## Can have slots (apart from '.xData'), though not recommended; check these:
    sns <- names(cld@slots); sns <- sns[sns != ".xData"]
    msg <- if(length(sns)) {
	L <- lapply(sns, function(sn)
	    all.equal(slot(target, sn), slot(current, sn), ...))
	unlist(L[vapply(L, is.character, NA)])
    }
    if(is.character(n)) msg <- c(msg, n)
    if(is.null(msg)) TRUE else msg
}

all.equal.environment <- function (target, current, all.names=TRUE, ...) {
    if(!is.environment (target)) return( "'target' is not an environment")
    if(!is.environment(current)) return("'current' is not an environment")
    ae.run <- dynGet("__all.eq.E__", NULL)
    if(is.null(ae.run))
	"__all.eq.E__" <- environment() # -> 5 visible + 6 ".<..>" objects
    else { ## ae.run contains previous target, current, ..

	## If we exactly match one of these, we return TRUE here,
	## otherwise, divert to all.equal(as.list(.), ...) below

	## needs recursive function -- a loop with  em <- em$mm	 destroys the env!
	do1 <- function(em) {
	    if(identical(target, em$target) && identical(current, em$current))
		TRUE
	    else if(!is.null(em$ mm)) ## recurse
		do1(em$ mm)
	    else {
		## add the new (target, current) pair, and return FALSE
		e <- new.env(parent = emptyenv())
		e$target  <- target
		e$current <- current
		em$ mm <- e
		FALSE
	    }
	}

	if(do1(ae.run)) return(TRUE)
	## else, continue:
    }
    all.equal.list(as.list.environment(target , all.names=all.names, sorted=TRUE),
		   as.list.environment(current, all.names=all.names, sorted=TRUE), ...)
}

all.equal.factor <- function(target, current, ..., check.attributes = TRUE)
{
    if(!inherits(target, "factor"))
	return("'target' is not a factor")
    if(!inherits(current, "factor"))
	return("'current' is not a factor")
    msg <-  if(check.attributes) attr.all.equal(target, current, ...)
    n <- all.equal(as.character(target), as.character(current),
                   check.attributes = check.attributes, ...)
    if(is.character(n)) msg <- c(msg, n)
    if(is.null(msg)) TRUE else msg
}

all.equal.formula <- function(target, current, ...)
{
    ## NB: this assumes the default method for class formula, not
    ## the misquided one in package Formula
    if(length(target) != length(current))
	return(paste("target, current differ in having response: ",
		     length(target) == 3L, ", ",
                     length(current) == 3L, sep=""))
    ## <NOTE>
    ## This takes same-length formulas as all equal if they deparse
    ## identically.  As of 2010-02-24, deparsing strips attributes; if
    ## this is changed, the all equal behavior will change unless the
    ## test is changed.
    ## </NOTE>
    if(!identical(deparse(target), deparse(current)))
	"formulas differ in contents"
    else TRUE
}

all.equal.language <- function(target, current, ...)
{
    mt <- mode(target)
    mc <- mode(current)
    if(mt == "expression" && mc == "expression")
	return(all.equal.list(target, current, ...))
    ttxt <- paste(deparse(target), collapse = "\n")
    ctxt <- paste(deparse(current), collapse = "\n")
    msg <- c(if(mt != mc)
	     paste0("Modes of target, current: ", mt, ", ", mc),
	     if(ttxt != ctxt) {
		 if(pmatch(ttxt, ctxt, 0L))
		     "target is a subset of current"
		 else if(pmatch(ctxt, ttxt, 0L))
		     "current is a subset of target"
		 else "target, current do not match when deparsed"
	     })
    if(is.null(msg)) TRUE else msg
}

## use.names is new in 3.1.0: avoid partial/positional matching
all.equal.list <- function(target, current, ...,
                           check.attributes = TRUE, use.names = TRUE)
{
    if (!is.logical(check.attributes))
        stop(gettextf("'%s' must be logical", "check.attributes"),
             domain = NA)
    if (!is.logical(use.names))
        stop(gettextf("'%s' must be logical", "use.names"), domain = NA)
    msg <- if(check.attributes) attr.all.equal(target, current, ...)
    ## Unclass to ensure we get the low-level components
    target <- unclass(target) # "list"
    current <- unclass(current)# ??
    ## Comparing the data.class() is not ok, as a list matrix is 'matrix' not 'list'
    if(!is.list(target) && !is.vector(target))
	return(c(msg, "target is not list-like"))
    if(!is.list(current) && !is.vector(current))
	return(c(msg, "current is not list-like"))
    if((n <- length(target)) != length(current)) {
	if(!is.null(msg)) msg <- msg[- grep("\\bLengths\\b", msg)]
	n <- min(n, length(current))
	msg <- c(msg, paste("Length mismatch: comparison on first",
			    n, "components"))
    }
    iseq <- seq_len(n)
    if(use.names)
	use.names <- (length(nt <- names(target )[iseq]) == n &&
		      length(nc <- names(current)[iseq]) == n)
    for(i in iseq) {
	mi <- all.equal(target[[i]], current[[i]],
			check.attributes=check.attributes, use.names=use.names, ...)
	if(is.character(mi))
	    msg <- c(msg, paste0("Component ",
				 if(use.names && nt[i] == nc[i]) dQuote(nt[i]) else i,
				 ": ", mi))
    }
    if(is.null(msg)) TRUE else msg
}

## also used for logical
all.equal.raw <-
    function(target, current, ..., check.attributes = TRUE)
{
    if (!is.logical(check.attributes))
        stop(gettextf("'%s' must be logical", "check.attributes"), domain = NA)
    msg <-  if(check.attributes) attr.all.equal(target, current, ...)
    if(data.class(target) != data.class(current)) {
	msg <- c(msg, paste0("target is ", data.class(target), ", current is ",
                             data.class(current)))
	return(msg)
    }
    lt <- length(target)
    lc <- length(current)
    if(lt != lc) {
	if(!is.null(msg)) msg <- msg[- grep("\\bLengths\\b", msg)]
	msg <- c(msg, paste0("Lengths (", lt, ", ", lc,
                             ") differ (comparison on first ",
                             ll <- min(lt, lc), " components)"))
	ll <- seq_len(ll)
	target <- target[ll]
	current <- current[ll]
    }
    # raws do not have NAs, but logicals do
    nas <- is.na(target); nasc <- is.na(current)
    if (any(nas != nasc)) {
	msg <- c(msg, paste("'is.NA' value mismatch:", sum(nasc),
                            "in current", sum(nas), "in target"))
	return(msg)
    }
    ne <- !nas & (target != current)
    if(!any(ne) && is.null(msg)) TRUE
    else if(sum(ne) == 1L) c(msg, paste("1 element mismatch"))
    else if(sum(ne) > 1L) c(msg, paste(sum(ne), "element mismatches"))
    else msg
}


## attributes are a pairlist, so never 'long'
attr.all.equal <- function(target, current, ...,
                           check.attributes = TRUE, check.names = TRUE)
{
    ##--- "all.equal(.)" for attributes ---
    ##---  Auxiliary in all.equal(.) methods --- return NULL or character()
    if (!is.logical(check.attributes))
        stop(gettextf("'%s' must be logical", "check.attributes"), domain = NA)
    if (!is.logical(check.names))
        stop(gettextf("'%s' must be logical", "check.names"), domain = NA)
    msg <- NULL
    if(mode(target) != mode(current))
	msg <- paste0("Modes: ", mode(target), ", ", mode(current))
    if(length(target) != length(current))
	msg <- c(msg,
                 paste0("Lengths: ", length(target), ", ", length(current)))
    ax <- attributes(target)
    ay <- attributes(current)
    if(check.names) {
        nx <- names(target)
        ny <- names(current)
        if((lx <- length(nx)) | (ly <- length(ny))) {
            ## names() treated now; hence NOT with attributes()
            ax$names <- ay$names <- NULL
            if(lx && ly) {
                if(is.character(m <- all.equal.character(nx, ny, check.attributes = check.attributes)))
                    msg <- c(msg, paste("Names:", m))
            } else if(lx)
                msg <- c(msg, "names for target but not for current")
            else msg <- c(msg, "names for current but not for target")
        }
    } else {
	ax[["names"]] <- NULL
	ay[["names"]] <- NULL
    }

    if(check.attributes && (length(ax) || length(ay))) {# some (more) attributes
	## order by names before comparison:
	nx <- names(ax)
	ny <- names(ay)
	if(length(nx)) ax <- ax[order(nx)]
	if(length(ny)) ay <- ay[order(ny)]
	tt <- all.equal(ax, ay, ..., check.attributes = check.attributes)
	if(is.character(tt)) msg <- c(msg, paste("Attributes: <", tt, ">"))
    }
    msg # NULL or character
}

## formerly in datetime.R
## force absolute comparisons
all.equal.POSIXt <- function(target, current, ..., tolerance = 1e-3, scale)
{
    target <- as.POSIXct(target); current <- as.POSIXct(current)
    check_tzones(target, current)
    attr(target, "tzone") <- attr(current, "tzone") <- NULL
    all.equal.numeric(target, current, ..., tolerance = tolerance, scale = 1)
}
