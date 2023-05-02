#ifdef BUILD_CASPERSCRIPT
/* add support for casperscript extensions */
#include <time.h>  /* for nanosleep(), ... */
#include <math.h>  /* for roundl(), ... */
#include <stdlib.h>  /* for abs(), atof(), ... */
#include <stdarg.h>  /* for vsnprintf(), ... */
#include "gssyslog.h"
#include "zcasper.h"
#include "interp.h"  /* for i_initial_enter_name() */
#include <readline/history.h>  /* for history functionality */
#ifdef TEST_ZCASPER
#include <stdio.h>   /* for fprintf(), ... */
#else
#include "ghost.h"
#include "gserrors.h"
#include "oper.h"
#include "store.h"
#include "estack.h"
#include "std.h"
#endif  /* TEST_ZCASPER */
#include "gsprintf.h"  /* for gsprintf and thus zsprintf */
#define PATHLENGTH 1024  /* for savesession filename */
extern char *argv0;
extern char *programname;

int sleep(double seconds) {
    struct timespec requested;
    int iseconds = (int)seconds, code;
    double fractional = seconds - iseconds;
    long nanoseconds = roundl(fractional * 1000000000L);
    syslog(LOG_USER | LOG_DEBUG, "%d s, %ld ns", iseconds, nanoseconds);
    requested.tv_sec = iseconds;
    requested.tv_nsec = nanoseconds;
    code = nanosleep(&requested, NULL);
    return abs(code);
}

#ifndef TEST_ZCASPER
static int zsleep(i_ctx_t *i_ctx_p);  /* implement `sleep` in casperscript */
    /* <sleeptime> sleep */
static int zsleep(i_ctx_t *i_ctx_p) {
    os_ptr op = osp;
    double sleeptime = 0.0;
    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
            break;
        case t_real:
            sleeptime = op->value.realval;
            break;
        case t_integer:
            sleeptime = (double)op->value.intval;
            break;
    }
    if (sleeptime > 0) sleep(sleeptime);
    pop(1);
    return 0;
}

static int zsprintf(i_ctx_t *i_ctx_p);  /* `sprintf` for casperscript */
    /* <stringbuffer> <formatstring> <args_array> sprintf <formatted> <fit>*/
static int zsprintf(i_ctx_t *i_ctx_p) {
    os_ptr op = osp;
    unsigned int written;
    if (r_type(op) != t_array) return_op_typecheck(op);
    if (r_type(op - 1) != t_string) return_op_typecheck(op - 1);
    if (r_type(op - 2) != t_string) return_op_typecheck(op - 2);
    written = gsprintf(op - 2, op - 1, op);
    /* we pop the args array...
     * overwrite the format string with success flag...
     * and resize the buffer to the resultant string if necessary */
    pop(1); op = osp;  /* pop macro only decrements osp */
    if (written <= r_size(op - 1)) {
        make_true(op);
        r_dec_size(op - 1, r_size(op - 1) - written);
    } else make_false(op);
    return 0;
}

/* define constants for casperscript, and do any other initialization */
int zcasperinit(i_ctx_t *i_ctx_p) {
    int code = 0, i;
    const char *names[16] = {"argv0", "programname"};
    const char *strings[] = {argv0, programname};
    int arraysize = sizeof(strings) / sizeof(char *);
    ref objects[arraysize];
    syslog(LOG_USER | LOG_DEBUG, "making systemdict entries for %d strings",
            arraysize);
    for (i = 0; i < arraysize; i++) {
        make_const_string(&objects[i], a_readonly | avm_foreign,
                strlen(strings[i]), (byte *)strings[i]);
        syslog(LOG_USER | LOG_DEBUG, "%s: \"%.*s\"", names[i],
                objects[i].tas.rsize, objects[i].value.bytes);
        code |= i_initial_enter_name(i_ctx_p, names[i], &objects[i]);
    }
    syslog(LOG_USER | LOG_DEBUG, "initializing readline history");
    using_history();
    return code;
};

int zsavesession(i_ctx_t *i_ctx_p);
int zsavesession(i_ctx_t *i_ctx_p) {
    HISTORY_STATE *history_state = history_get_history_state();
    gp_file *historyfile = NULL;
    os_ptr op = osp;
    int i, code = 0;
    char filename[PATHLENGTH];
    if (r_type(op) != t_string) return_op_typecheck(op);
    if (r_size(op) >= PATHLENGTH) return_error(gs_error_rangecheck);
    DISCARD(strncpy(filename, (char *)op->value.bytes, r_size(op)));
    filename[r_size(op)] = '\0';
    historyfile = gp_fopen(imemory, filename, "w");
    for (i = 0; i < history_state->length; i++) {
        code |= gp_fputs(history_state->entries[i]->line, historyfile);
        code |= gp_fputc('\n', historyfile);
        if (code < 0) break;
    }
    code |= gp_fclose(historyfile);
    code = code < 0 ? -1 : 0;
    pop(1);
    return code;
}
#endif  /* TEST_ZCASPER */

/* ------ Initialization procedure ------ */
const op_def zcasper_op_defs[] =
{
    /* FIXME: relocate these from systemdict to casperdict on startup */
    {"1sleep", zsleep},
    {"3sprintf", zsprintf},
    {"1savesession", zsavesession},
    op_def_end(0)
};

#ifdef TEST_ZCASPER
int main(int argc, char **argv) {
    int code;
    if (argc < 2) {
        fprintf(stderr, "Must specify sleep time\n");
        code = 1;
    }
    else code = sleep(atof(argv[1]));
    return code;
}
#endif  /* TEST_ZCASPER */
#endif  /* BUILD_CASPERSCRIPT */
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
