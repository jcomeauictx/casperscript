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
#include "stream.h"
#ifdef USE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <termios.h>  /* for turning off and on echoing */
#include <unistd.h>  /* for read() and write() */
#include "gssyslog.h"
#include "files.h"
#endif

extern int rl_tty_set_echoing PARAMS((int));

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
/* //eli.thegreenplace.net/2016/basics-of-using-the-readline-library/ */
/* //stackoverflow.com/a/59923166/493161 */
#define CS_STDIN 0
#define CS_STDOUT 1
#define MAXPROMPT 32
#define MAXREPLY 12
#define MINREPLY 5  /* 5 is absolute minimum reply: <CSI>1;1R */
#define QUERY "\033[6n"
#define PROMPT "..............................."
    const byte *query = (byte *)QUERY;
    /* uint querysize = strlen((char *)query); */ /* no need using outprintf */
    /* "\033[{ROW};{COLUMN}R", minimum 5 bytes with CSI, 6 with <ESC>[
     * we're going to assume that 4 will be the minimum (1-based) column,
     * and that a stack size of >999 is highly unlikely. this will mean
     * only having to check one digit of the reply, that before the 'R'.
     * a value less than 4 can be taken to mean: 0=10, ... 3=13.
     * NOTE: use low-level calls here, bypassing any stream buffering */
    char reply[MAXREPLY] = "";
    uint replysize = 0;
    char *buffer, promptstring[MAXPROMPT] = "";
    int promptsize, count = *pcount, code = EOF;
    uint nsize;
    byte *ndata;
    struct termios settings[2];

    /* disable auto-complete with filenames */
    DISCARD(rl_bind_key('\t', rl_insert));
    syslog(LOG_USER | LOG_DEBUG, "gp_readline called with count %d, buf %.*s",
            count, count + MAXPROMPT, (char *)buf->data);
    if (prompt && prompt->size > (MAXPROMPT - 1)) {
        code = 1;  /* FIXME: this code means buffer full, find a better one */
    } else {
        if (!count && s_out && prompt) {
            DISCARD(strncpy(promptstring, (char *)prompt->data, prompt->size));
            promptstring[prompt->size] = '\0';
        } else {
            rl_already_prompted = (int)true;
            /* attempt to get cursor position using <ESC>[6n which replies
             * with <ESC>[{ROW};{COLUMN}R
             */
            /*FIXME: this needs to be wrapped in timeout and ^C trap */
            tcgetattr(CS_STDIN, &settings[0]);  /* for reference */
            tcgetattr(CS_STDIN, &settings[1]);  /* FIXME: use memmove ? */
            settings[1].c_lflag &= ~ECHO;  /* don't echo reply to screen */
            settings[1].c_lflag &= ~ICANON;  /* stop buffering */
            tcsetattr(CS_STDIN, TCSANOW, &settings[1]);
            outprintf(bufmem, (const char *)query);
            syslog(LOG_USER | LOG_DEBUG, "sent query, waiting for reply");
            replysize += read(CS_STDIN, reply, MINREPLY);  
            while (replysize < MAXREPLY - 1) {  /* leave a byte for \0 */
                if (reply[replysize - 1] == 'R') break;
                replysize += read(CS_STDIN, &reply[replysize], 1);
                syslog(LOG_USER | LOG_DEBUG, "got <ESC>%.*s so far",
                        replysize - 1, reply + 1);
            }
            syslog(LOG_USER | LOG_DEBUG, "gp_readline query reply: <ESC>%.*s",
                    replysize - 1, reply + 1);
            promptsize = reply[replysize - 2] - '0';  /* convert digit to int */
            if (promptsize < 4) promptsize += 10;
            DISCARD(strncpy(promptstring, PROMPT, promptsize));
            promptstring[promptsize - 1] = '>';
            promptstring[promptsize] = '\0';
            tcsetattr(CS_STDIN, TCSANOW, &settings[0]);  /* restore termios */
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
                DISCARD(strncpy((char *)(buf->data + *pcount), buffer, count));
                *pcount += count;
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
