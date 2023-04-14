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


/* Allocator debugging definitions and interface */
/* Requires gdebug.h (for gs_debug) */

#ifndef gsmdebug_INCLUDED
#  define gsmdebug_INCLUDED

#include "valgrind.h"

/* Define the fill patterns used for debugging the allocator. */
extern const byte
       gs_alloc_fill_alloc,	/* allocated but not initialized */
       gs_alloc_fill_block,	/* locally allocated block */
       gs_alloc_fill_collected,	/* garbage collected */
       gs_alloc_fill_deleted,	/* locally deleted block */
       gs_alloc_fill_free;	/* freed */

/* Define an alias for a specialized debugging flag */
/* that used to be a separate variable. */
#define gs_alloc_debug gs_debug['@']

/* Conditionally fill unoccupied blocks with a pattern. */
extern void gs_alloc_memset(void *, int /*byte */ , size_t);

#ifdef DEBUG
#  define gs_alloc_fill(ptr, fill, len)                              \
     BEGIN                                                           \
     if ( gs_alloc_debug ) gs_alloc_memset(ptr, fill, (size_t)(len)); \
     VALGRIND_MAKE_MEM_UNDEFINED(ptr,(ulong)(len));                  \
     END
#else
#  define gs_alloc_fill(ptr, fill, len)                              \
     BEGIN                                                           \
     VALGRIND_MAKE_MEM_UNDEFINED(ptr,(ulong)(len));                  \
     END
#endif

#endif /* gsmdebug_INCLUDED */
