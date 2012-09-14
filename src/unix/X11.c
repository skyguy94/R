/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1999-2012 The R Core Team
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
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>

#include <Rconnections.h>
#include <Rdynpriv.h>

#ifdef HAVE_X11

#include <Rmodules/RX11.h>   /* typedefs for the module routine types */

static R_X11Routines routines, *ptr = &routines;

static int initialized = 0;

R_X11Routines * R_setX11Routines(R_X11Routines *routines)
{
    R_X11Routines *tmp;
    tmp = ptr;
    ptr = routines;
    return tmp;
}

int attribute_hidden R_X11_Init(void)
{
    int res;

    if(initialized) return initialized;

    initialized = -1;
    if(strcmp(R_GUIType, "none") == 0) {
	warning(_("X11 module is not available under this GUI"));
	return initialized;
    }
    res = R_moduleCdynload("R_X11", 1, 1);
    if(!res) return initialized;
    if(!ptr->access)
	error(_("X11 routines cannot be accessed in module"));
    initialized = 1;
    return initialized;
}

Rboolean attribute_hidden R_access_X11(void)
{
    R_X11_Init();
    return (initialized > 0) ? (*ptr->access)() > 0 : FALSE;
}

SEXP do_X11(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    R_X11_Init();
    if(initialized > 0)
	return (*ptr->X11)(call, op, args, rho);
    else {
	error(_("X11 module cannot be loaded"));
	return R_NilValue;
    }
}

SEXP do_saveplot(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    R_X11_Init();
    if(initialized > 0)
	return (*ptr->saveplot)(call, op, args, rho);
    else {
	error(_("X11 module cannot be loaded"));
	return R_NilValue;
    }
}

Rboolean R_GetX11Image(int d, void *pximage, int *pwidth, int *pheight)
{
    R_X11_Init();
    if(initialized > 0)
	return (*ptr->image)(d, pximage, pwidth, pheight);
    else {
	error(_("X11 module cannot be loaded"));
	return FALSE;
    }
}

Rboolean attribute_hidden R_ReadClipboard(Rclpconn clpcon, char *type)
{
    R_X11_Init();
    if(initialized > 0)
	return (*ptr->readclp)(clpcon, type);
    else {
	error(_("X11 module cannot be loaded"));
	return FALSE;
    }
}

static R_deRoutines de_routines, *de_ptr = &de_routines;

R_deRoutines * R_setdeRoutines(R_deRoutines *routines)
{
    R_deRoutines *tmp;
    tmp = de_ptr;
    de_ptr = routines;
    return tmp;
}

static void R_de_Init(void)
{
    static int de_init = 0;

    if(de_init > 0) return;
    if(de_init < 0) error(_("X11 module cannot be loaded"));

    de_init = -1;
    if(strcmp(R_GUIType, "none") == 0) {
	warning(_("X11 module is not available under this GUI"));
	return;
    }
    int res = R_moduleCdynload("R_de", 1, 1);
    if(!res) error(_("X11 module cannot be loaded"));
    de_init = 1;
    return;
}

SEXP X11_do_dataentry(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    R_de_Init();
    return (*de_ptr->de)(call, op, args, rho);
}

SEXP X11_do_dataviewer(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    R_de_Init();
    return (*de_ptr->dv)(call, op, args, rho);
}
#else /* No HAVE_X11 */

Rboolean attribute_hidden R_access_X11(void)
{
    return FALSE;
}

SEXP attribute_hidden do_X11(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    error(_("X11 is not available"));
    return R_NilValue;
}

SEXP attribute_hidden do_cairo(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    error(_("X11 is not available"));
    return R_NilValue;
}

SEXP attribute_hidden do_saveplot(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    error(_("X11 is not available"));
    return R_NilValue;
}

Rboolean R_GetX11Image(int d, void *pximage, int *pwidth, int *pheight)
{
    error(_("X11 is not available"));
    return FALSE;
}

Rboolean attribute_hidden R_ReadClipboard(Rclpconn con, char *type)
{
    error(_("X11 is not available"));
    return FALSE;
}

SEXP attribute_hidden X11_do_dataentry(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    error(_("X11 is not available"));
    return R_NilValue;
}

SEXP X11_do_dataviewer(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    error(_("X11 is not available"));
    return R_NilValue;
}
#endif
