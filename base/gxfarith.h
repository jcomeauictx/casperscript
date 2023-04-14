/* Copyright (C) 2001-2023 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  39 Mesa Street, Suite 108A, San Francisco,
   CA 94129, USA, for further information.
*/


/* Floating point arithmetic macros for Ghostscript library */

#ifndef gxfarith_INCLUDED
#  define gxfarith_INCLUDED

#include "stdpre.h"
#include "gxarith.h"

/*
 * Define sine and cosine functions that take angles in degrees rather than
 * radians, hit exact values at multiples of 90 degrees, and are implemented
 * efficiently on machines with slow (or no) floating point.
 */
double gs_sin_degrees(double angle);
double gs_cos_degrees(double angle);
typedef struct gs_sincos_s {
    double sin, cos;
    bool orthogonal;		/* angle is multiple of 90 degrees */
} gs_sincos_t;
void gs_sincos_degrees(double angle, gs_sincos_t * psincos);

/*
 * Define an atan2 function that returns an angle in degrees and uses
 * the PostScript quadrant rules.  Note that it may return
 * gs_error_undefinedresult.
 */
int gs_atan2_degrees(double y, double x, double *pangle);

#endif /* gxfarith_INCLUDED */
