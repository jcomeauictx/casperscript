/* Copyright (C) 2001-2021 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  1305 Grant Avenue - Suite 200, Novato,
   CA 94945, U.S.A., +1(415)492-9861, for further information.
*/


/* Default, stream-based readline implementation */
#include "std.h"
#include "gstypes.h"
#include "gsmemory.h"
#include "gp.h"
#ifdef USE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include "gssyslog.h"
#endif

int
gp_readline_init(void **preadline_data, gs_memory_t * mem)
{
    return 0;
}

int
gp_readline(stream *s_in, stream *s_out, void *readline_data,
            gs_const_string *prompt, gs_string * buf,
            gs_memory_t * bufmem, uint * pcount, bool *pin_eol,
            bool (*is_stdin)(const stream *))
{
    /* gp_readline is only called, via zreadline_from, when stream s_in is
     * stdin. */
#ifndef USE_LIBREADLINE
    return sreadline(s_in, s_out, readline_data, prompt, buf, bufmem, pcount,
                     pin_eol, is_stdin);
#else
#define MAXPROMPT 32
#define sane(x, max) (x > 0 && x < max)
    /* //eli.thegreenplace.net/2016/basics-of-using-the-readline-library/ */
    char *buffer, promptstring[MAXPROMPT] = "";
    int promptsize, count = *pcount, code = EOF;
    uint nsize;
    byte *ndata;

    /* disable auto-complete with filenames */
    DISCARD(rl_bind_key('\t', rl_insert));
    if (prompt && prompt->size > (MAXPROMPT - 1)) {
        code = 1;
    } else {
        if (!count && s_out && prompt) {
            DISCARD(strncpy(promptstring, (char *)prompt->data, prompt->size));
            promptstring[prompt->size] = '\0';
        } else {
            rl_already_prompted = (int)true;
            /* ugly hack to get prompt hidden in supposedly-empty buffer */
            promptsize = strlen((char *)buf->data);
            if (sane(promptsize, MAXPROMPT) && !buf->size && !count && s_out) {
                DISCARD(strncpy(promptstring, (char *)buf->data, promptsize));
                promptstring[promptsize] = '\0';
            }
        }
        while ((buffer = readline(promptstring)) != NULL) {
            count = strlen(buffer);
            if (count > 0) {
                if (count >= buf->size) {	/* filled the string */
                    /* copy what fits now, in case no more memory available */
                    DISCARD(strncpy((char *)(buf->data + *pcount), buffer,
                            buf->size - *pcount));
                    if (!bufmem) {code = 1; break;}
                    nsize = count + 1;
                    ndata = gs_resize_string(bufmem, buf->data, buf->size,
                                             nsize, "sreadline(buffer)");
                    if (ndata == 0) {code = ERRC; break;} /* no better choice */
                    buf->data = ndata;
                    buf->size = nsize;
                }
                add_history(buffer);
                syslog(LOG_USER | LOG_DEBUG,
                       "count: %d, buffer: %*s", count, count, buffer);
                DISCARD(strncpy((char *)(buf->data + *pcount), buffer,
                                count - *pcount));
                *pcount = count;
                code = 0;
                break;  /* for count > 0 */
            }
            break;  /* for count == 0 */
        }
        /* OK to have free() after while() because it only runs once,
         * and it's needed here because of the `break` statements above */
        free(buffer);
    }
    return code;
#endif
}

void
gp_readline_finit(void *readline_data)
{
}
/* vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
 */
