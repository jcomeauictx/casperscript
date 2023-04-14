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


/* Client interface to LanguageLevel 3 color facilities */

#ifndef gscolor3_INCLUDED
#  define gscolor3_INCLUDED

#include "gsgstate.h"
#include "gsshade.h"

/* Smooth shading */
int gs_setsmoothness(gs_gstate *, double);
float gs_currentsmoothness(const gs_gstate *);
int gs_shfill(gs_gstate *, const gs_shading_t *);

#endif /* gscolor3_INCLUDED */
