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


#ifndef pgfdata_h_INCLUDED
#define pgfdata_h_INCLUDED

/* pgfdata.h */
/* Interface to HP-GL/2 stick and arc font data */

/* font type - stick or arc */
typedef enum
{
    HPGL_ARC_FONT,
    HPGL_STICK_FONT
} hpgl_font_type_t;

/* Enumerate the segments of a stick/arc character. */
int hpgl_stick_arc_segments(const gs_memory_t * mem,
                            void *data, gs_char char_index,
                            hpgl_font_type_t font_type);

/* Get the unscaled width of a stick/arc character. */
int hpgl_stick_arc_width(gs_char char_index, hpgl_font_type_t font_type);

/* We also process the The DL (download font) format in this module,
   but it should be separated out */
int hpgl_531_segments(const gs_memory_t * mem, void *data, void *cdata);

#endif
