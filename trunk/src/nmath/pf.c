/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
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
 *
 *  SYNOPSIS
 *
 *    #include "Mathlib.h"
 *    double pf(double x, double n1, double n2);
 *
 *  DESCRIPTION
 *
 *    The distribution function of the F distribution.
 */

#include "Mathlib.h"

double pf(double x, double n1, double n2)
{
    if (
#ifdef IEEE_754
	isnan(x) || !finite(n2) || !finite(n2) ||
#endif
	n1 <= 0.0 || n2 <= 0.0) {
	ML_ERROR(ME_DOMAIN);
	return ML_NAN;
    }
    if (x <= 0.0)
	return 0.0;
    x = 1.0 - pbeta(n2 / (n2 + n1 * x), n2 / 2.0, n1 / 2.0);
    return ML_VALID(x) ? x : ML_NAN;
}
