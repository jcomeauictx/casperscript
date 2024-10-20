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
#define MAXPROMPT 8192  /* largest conceivable character terminal width */
#define MAXREPLY 12
#define MINREPLY 5  /* 5 is absolute minimum reply: <CSI>1;1R */
#define QUERY "\033[6n"
#define CHA "\033[%dG"  /* Cursor Horizontal Absolute */
    const byte *query = (byte *)QUERY;
    /* uint querysize = strlen((char *)query); */ /* no need using outprintf */
    /* "\033[{ROW};{COLUMN}R", minimum 5 bytes with CSI, 6 with <ESC>[
     * NOTE: use low-level calls here, bypassing any stream buffering
     *
     * NOTE also: have to assume possibility that cruft exists on the command
     * line, output without \n, that moves the prompt well into the line. so
     * we need to read the entire column number, not just the final digit
     * assumed on earlier attempt. */
    char reply[MAXREPLY] = "";
    uint replysize = 0;
    char *buffer, promptstring[MAXPROMPT] = "";
    int promptsize = 0, count = *pcount, code = EOF;
    int digit = 'R', offset = 2, multiplier = 1;
    uint nsize;
    byte *ndata;
    struct termios settings[2];

    /* disable auto-complete with filenames */
    DISCARD(rl_bind_key('\t', rl_insert));
    syslog(LOG_USER | LOG_DEBUG, "gp_readline called with count %d, buf %.*s",
            count, min(count, 64), (char *)buf->data);
    if (prompt && prompt->size > (MAXPROMPT - 1)) {
        code = ERRC;
    } else {
        if (!count && s_out && prompt) {
            DISCARD(strncpy(promptstring, (char *)prompt->data, prompt->size));
            promptstring[prompt->size] = '\0';
        } else {
            rl_already_prompted = (int)true;
            /* attempt to get cursor position using <ESC>[6n which replies
             * with <ESC>[{ROW};{COLUMN}R
             */
            if (isatty(CS_STDIN)) {
                int columnlength = 0, cha_promptsize = strlen(CHA);
                /*FIXME: this needs to be wrapped in timeout and ^C trap */
                tcgetattr(CS_STDIN, &settings[0]);  /* for reference */
                settings[1] = settings[0];  /* let gcc generate the code */
                settings[1].c_lflag &= ~ECHO;  /* don't echo reply to screen */
                settings[1].c_lflag &= ~ICANON;  /* stop buffering */
                tcsetattr(CS_STDIN, TCSANOW, &settings[1]);
                outprintf(bufmem, (const char *)query);
                syslog(LOG_USER | LOG_DEBUG, "sent query, waiting for reply");
                replysize += read(CS_STDIN, reply, MINREPLY);
                while (replysize < MAXREPLY - 1) {  /* leave a byte for \0 */
                    if (reply[replysize - 1] == 'R') break;
                    replysize += read(CS_STDIN, &reply[replysize], 1);
                }
                syslog(LOG_USER | LOG_DEBUG, "gp_readline reply: <ESC>%.*s",
                        replysize - 1, reply + 1);
                while (replysize - offset > 0) {
                    digit = reply[replysize - offset++];
                    if (digit < '0' || digit > '9') break;
                    promptsize += ((digit - '0') * multiplier);
                    multiplier *= 10;
                    columnlength += 1;
                }
#ifndef USE_CHA_PROMPT
                promptsize -= 1;  /* column returned is one *past* prompt */
                memset(promptstring, '.', promptsize);
                promptstring[promptsize] = '\0';
#else
                /* the `%d' will be replaced by the actual column number */
                /* but add 1 for final "\0" in `size` arg to snprintf */
                syslog(LOG_USER | LOG_DEBUG, "columnlength: %d", columnlength);
                cha_promptsize += (columnlength - 2) + 1;
                DISCARD(snprintf(promptstring, cha_promptsize, CHA, promptsize)); 
#endif
                syslog(LOG_USER | LOG_DEBUG, "prompt now: \"%s\"",
                        promptstring);
                tcsetattr(CS_STDIN, TCSANOW, &settings[0]);  /* restore term */
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
                DISCARD(strncpy((char *)(buf->data + *pcount), buffer, count));
                *pcount += count;
                code = 0;
                break;  /* for count > 0 */
            }
            /* for count == 0 */
            code = 0;
            break;
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
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
