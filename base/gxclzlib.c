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


/* zlib filter initialization for RAM-based band lists */
/* Must be compiled with -I$(ZSRCDIR) */
#include "std.h"
#include "gstypes.h"
#include "gsmemory.h"
#include "gxclmem.h"
#include "szlibx.h"

/* Return the prototypes for compressing/decompressing the band list. */
const stream_template *
clist_compressor_template(void)
{
    return &s_zlibE_template;
}
const stream_template *
clist_decompressor_template(void)
{
    return &s_zlibD_template;
}
void
clist_compressor_init(stream_state *state)
{
    s_zlib_set_defaults(state);
    ((stream_zlib_state *)state)->no_wrapper = true;
    state->templat = &s_zlibE_template;
}
void
clist_decompressor_init(stream_state *state)
{
    s_zlib_set_defaults(state);
    ((stream_zlib_state *)state)->no_wrapper = true;
    state->templat = &s_zlibD_template;
}
