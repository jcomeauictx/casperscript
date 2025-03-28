/* Copyright (C) 2001-2025 Artifex Software, Inc.
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


/* Definitions common to stream clients and implementors */

#ifndef scommon_INCLUDED
#  define scommon_INCLUDED

#include "gsmemory.h"
#include "gstypes.h"		/* for gs_string */
#include "gsstype.h"		/* for extern_st */
#include "stdint_.h"         /* for int64_t */

/*
 * There are three major structures involved in the stream package.
 *
 * A stream is an "object" that owns a buffer, which it uses to implement
 * byte-oriented sequential access in a standard way, and a set of
 * procedures that handle things like buffer refilling.  See stream.h
 * for more information about streams.
 */
typedef struct stream_s stream;

/*
 * A stream_state records the state specific to a given variety of stream.
 * The buffer processing function of a stream maintains this state.
 */
typedef struct stream_state_s stream_state;

/*
 * A stream_template provides the information needed to create a stream.
 * The client must fill in any needed setup parameters in the appropriate
 * variety of stream_state, and then call the initialization function
 * provided by the template.  See strimpl.h for more information about
 * stream_templates.
 */
typedef struct stream_template_s stream_template;

/*
 * The stream package works with bytes, not chars.
 * This is to ensure unsigned representation on all systems.
 * A stream currently can only be read or written, not both.
 * Note also that the read procedure returns an int, not a char or a byte;
 * we use negative values to indicate exceptional conditions.
 * (We cast these values to int explicitly, because some compilers
 * don't do this if the other arm of a conditional is a byte.)
 *
 * Note that when a stream reaches an exceptional condition, that condition
 * remains set until the client does something explicit to reset it.
 * (There should be a 'sclearerr' procedure to do that, but there isn't.)
 * In particular, if a read stream encounters an exceptional condition,
 * it delivers the data it has in its buffer, and then all subsequent
 * calls to read data (sgetc, sgets, etc.) will return the exceptional
 * condition without reading any more actual data.
 */
/* End of data */
#define EOFC ((int)(-1))
/* Error */
#define ERRC ((int)(-2))
/* Interrupt */
#define INTC ((int)(-3))
/****** INTC IS NOT USED YET ******/
/* Callout */
#define CALLC ((int)(-4))
#define max_stream_exception 4
/* The following hack is needed for initializing scan_char_array in iscan.c. */
#define stream_exception_repeat(x) x, x, x, x

/*
 * Define cursors for reading from or writing into a buffer.
 * We lay them out this way so that we can alias
 * the write pointer and the read limit.
 */
typedef struct stream_cursor_read_s {
    const byte *ptr;
    const byte *limit;
    byte *_skip;
} stream_cursor_read;
typedef struct stream_cursor_write_s {
    const byte *_skip;
    byte *ptr;
    byte *limit;
} stream_cursor_write;
typedef union stream_cursor_s {
    stream_cursor_read r;
    stream_cursor_write w;
} stream_cursor;

/* The following two inline functions are here to keep the nasty
 * cursor initialisation (that ptr is initialised to one byte
 * /before/ the beginning of the buffer) in the one place,
 * rather than everywhere that uses short lived streams.
 * This allows localized disabling of the "array bounds" compiler
 * warning for this one specific case.
 *
 * The equally nasty "gs_fake_?buf" stuff avoids setting a pointer
 * value of "NULL - 1" which UBSAN complains about.
 */
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif
static byte gs_fake_sbuf[1];

static inline void
stream_cursor_read_init(stream_cursor_read *r, const byte *buf, size_t length)
{
    if (buf != NULL) {
        /* starting pos for pointer is always one position back */
        r->ptr = buf - 1;
        r->limit = r->ptr + length;
    }
    else {
        r->ptr = gs_fake_sbuf - 1;
        r->limit = r->ptr;
    }
}

static inline void
stream_cursor_write_init(stream_cursor_write *w, const byte *buf, size_t length)
{
    if (buf != NULL) {
        /* starting pos for pointer is always one position back */
        w->ptr = (byte *)buf - 1;
        w->limit = (byte *)w->ptr + length;
    }
    else {
        w->ptr = gs_fake_sbuf - 1;
        w->limit = w->ptr;
    }
}
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

/*
 * Define the prototype for the procedures known to both the generic
 * stream code and the stream implementations.
 */

/* Initialize the stream state (after the client parameters are set). */
#define stream_proc_init(proc)\
  int proc(stream_state *)

/* Process a buffer.  See strimpl.h for details. */
#define stream_proc_process(proc)\
  int proc(stream_state *, stream_cursor_read *,\
    stream_cursor_write *, bool)

/* Release the stream state when closing. */
#define stream_proc_release(proc)\
  void proc(stream_state *)

/* Initialize the client parameters to default values. */
#define stream_proc_set_defaults(proc)\
  void proc(stream_state *)

/* Reinitialize any internal stream state.  Note that this does not */
/* affect buffered data.  We declare this as returning an int so that */
/* it can be the same as the init procedure; however, reinit cannot fail. */
#define stream_proc_reinit(proc)\
  int proc(stream_state *)

/* Report an error.  Note that this procedure is stored in the state, */
/* not in the main stream structure. */
#define stream_proc_report_error(proc)\
  int proc(stream_state *, const char *)
stream_proc_report_error(s_no_report_error);

/*
 * Some types of streams have the ability to read their parameters from
 * a parameter list, and to write all (or only the non-default)
 * parameters to a parameter list.  Since these are not virtual
 * procedures for the stream (they operate on stream_state structures
 * even if no actual stream has been created), we name them differently.
 */
#define stream_state_proc_get_params(proc, state_type)\
  int proc(gs_param_list *plist, const state_type *ss, bool all)
#define stream_state_proc_put_params(proc, state_type)\
  int proc(gs_param_list *plist, state_type *ss)

/*
 * Define a generic stream state.  If a processing procedure has no
 * state of its own, it can use stream_state; otherwise, it must
 * create a "subclass".  There is a hack in stream.h to allow the stream
 * itself to serve as the "state" of a couple of heavily used stream types.
 *
 * In order to simplify the structure descriptors for concrete streams,
 * we require that the generic stream state not contain any pointers
 * to garbage-collectable storage.
 */
#define STREAM_MAX_ERROR_STRING 79
#define stream_state_common\
        const stream_template *templat;\
        gs_memory_t *memory;\
        stream_proc_report_error((*report_error));\
        int min_left; /* required bytes for lookahead */ \
        char error_string[STREAM_MAX_ERROR_STRING + 1]
struct stream_state_s {
    stream_state_common;
};

extern_st(st_stream_state);
#define public_st_stream_state() /* in stream.c */\
  gs_public_st_simple(st_stream_state, stream_state, "stream_state")

#endif /* scommon_INCLUDED */
