/* add support for casperscript extensions */
#include <time.h>  /* for nanosleep(), ... */
#include <math.h>  /* for roundl(), ... */
#include <stdlib.h>  /* for abs(), atof(), ... */
#include <stdarg.h>  /* for vsnprintf(), ... */
#include "gssyslog.h"
#include "zcasper.h"
#include "gsprintf.h"  /* for gsprintf and thus zsprintf */
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

#define MAX_ARRAY 128
static int zsprintf(i_ctx_t *i_ctx_p);  /* `sprintf` for casperscript */
    /* <stringbuffer> <formatstring> <args_array> sprintf <formatted> <fit>*/
static int zsprintf(i_ctx_t *i_ctx_p) {
    os_ptr op = osp;
    unsigned int buffersize, written;
    char *format, *formatted;
    unsigned int arraysize = r_type(op) == t_array ? r_size(op) : 1;
    /* can't use arraysize for dimension, not known until runtime */
    double args [MAX_ARRAY];
    int offset = 0, code;
    if (arraysize > MAX_ARRAY) return_error(gs_error_rangecheck);
    else zsprintf_load_args(i_ctx_p, op, args, offset);
    format = ref_to_string(op - 1, imemory, "zsprintf format");
    formatted = (op - 2)->value.bytes;
    buffersize = r_size(op - 2);
    syslog(LOG_USER | LOG_DEBUG,
           "format: \"%s\", buffersize: %d", format, buffersize);
    written = gsprintf(formatted, buffersize, format, args);
    pop(3); op = osp;  /* pop macro only decrements osp */
    if (format != NULL)
        gs_free_string(imemory, (byte *) format,
                       strlen(format) + 1, "zsprintf format");
    push(2);
    make_bool(op, written < buffersize);
    make_string(op - 1, a_all | icurrent_space, strlen(formatted), formatted);
    return 0;
}

int zsprintf_load_args(i_ctx_t *i_ctx_p, ref *op, double *args, int offset) {
    int code = 0;
    ref temp;
    switch (r_type(op)) {
        default:
            syslog(LOG_USER | LOG_DEBUG, "unexpected type at zsprintf");
            return_op_typecheck(op);
            break;
        case t_real:
            args[offset] = op->value.realval;
            syslog(LOG_USER | LOG_DEBUG, "zsprintf float value %f",
                   args[offset]);
            break;
        case t_integer:
            args[offset] = (double)op->value.intval;
            syslog(LOG_USER | LOG_DEBUG, "zsprintf int value %f",
                   args[offset]);
            break;
        case t_string:
            /* must have trailing \0 until we find a better way */
            args[offset] = (double)(long)op->value.bytes;
            syslog(LOG_USER | LOG_DEBUG, "zsprintf string at %p",
                   args[offset]);
        case t_array:
            syslog(LOG_USER | LOG_DEBUG, "zsprintf starting array");
            for (int i = 0; i < r_size(op); i++) {
                code = array_get(imemory, op, i, &temp);
                if (code < 0) break;  /* aborts safely */
                zsprintf_load_args(i_ctx_p, &temp, args, i);
            }
    }
    return code;
}


/* ------ Initialization procedure ------ */
const op_def zcasper_op_defs[] =
{
    /* FIXME: make these dotted, and `casper` aliases to undotted versions */
    {"1sleep", zsleep},
    {"3sprintf", zsprintf},
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
