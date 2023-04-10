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
#ifndef USE_LIBREADLINE
    return sreadline(s_in, s_out, readline_data, prompt, buf, bufmem, pcount,
                     pin_eol, is_stdin);
#else
#define MAXPROMPTSIZE 32
    /* //eli.thegreenplace.net/2016/basics-of-using-the-readline-library/ */
    char *buffer, promptstring[MAXPROMPTSIZE] = "";
    int count;
    uint nsize;
    byte *ndata;

    if (prompt->size > MAXPROMPTSIZE - 1) {
        return 1;
    } else {
        strncpy(promptstring, (char *)prompt->data, prompt->size);
        promptstring[prompt->size] = '\0';
    }
    while ((buffer = readline(promptstring)) != NULL) {
        count = strlen(buffer);
        if (count > 0) {
            if (count >= buf->size) {	/* filled the string */
                if (!bufmem) return 1;
                nsize = count + max(count, 20);
                ndata = gs_resize_string(bufmem, buf->data, buf->size,
                                          nsize, "sreadline(buffer)");
                if (ndata == 0)
                    return ERRC; /* no better choice */
                buf->data = ndata;
                buf->size = nsize;
            }
            add_history(buffer);
            strncpy((char *)buf->data, buffer, count);
            free(buffer);
            break;
        }
    }
    return 0;
#endif
}

void
gp_readline_finit(void *readline_data)
{
}
/* vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
 */
