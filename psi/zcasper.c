/* add support for casperscript extensions */
#include <time.h>  /* for nanosleep(), ... */
#include <math.h>  /* for roundl(), ... */
#include <stdlib.h>  /* for abs(), atof(), ... */
#include <stdarg.h>  /* for vsnprintf(), ... */
#include "gssyslog.h"
#include "zcasper.h"
#ifdef TEST_ZCASPER
#include <stdio.h>   /* for fprintf(), ... */
#else
#include "ghost.h"
#include "gserrors.h"
#include "oper.h"
#include "store.h"
#include "estack.h"
#include "std.h"
#endif
#include "gsprintf.h"  /* for gsprintf and thus zsprintf */
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
static int push_argv0(i_ctx_t *i_ctx_p);
static int push_argv0(i_ctx_t *i_ctx_p) {
    /* FIXME: surely there's a better way to place strings in systemdict */
    os_ptr op = osp;
    push(1);
    make_string(op, a_all | icurrent_space, strlen(argv0), (byte *)argv0);
    return 0;
}
static int push_programname(i_ctx_t *i_ctx_p);
static int push_programname(i_ctx_t *i_ctx_p) {
    os_ptr op = osp;
    push(1);
    make_string(op, a_all | icurrent_space, strlen(programname),
            (byte *)programname);
    return 0;
}

/* ------ Initialization procedure ------ */
const op_def zcasper_op_defs[] =
{
    /* FIXME: relocate these from systemdict to casperdict on startup */
    {"1sleep", zsleep},
    {"3sprintf", zsprintf},
    {"argv0", push_argv0},
    {"programname", push_programname},
    op_def_end(0)
};
#endif

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
#endif

// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
