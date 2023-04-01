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

static int zsprintf(i_ctx_t *i_ctx_p);  /* `sprintf` for casperscript */
    /* <stringbuffer> <formatstring> <args_array> sprintf <formatted> <fit>*/
static int zsprintf(i_ctx_t *i_ctx_p) {
    os_ptr op = osp;
    size_t buffersize, written;
    char *format, *formatted;
    void **args;
    bool arg_is_string = false;
    format = ref_to_string(op - 1, imemory, "zsprintf format");
    formatted = ref_to_string(op - 2, imemory, "zsprintf formatted");
    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
            break;
        case t_real:
            args = (void * []){(void *)&op->value.realval};
            break;
        case t_integer:
            args = (void * []){(void *)op->value.intval};
            break;
        case t_string:
            args = (void * []){(void *)ref_to_string(
                    op, imemory, "zsprintf arg")};
            arg_is_string = true;
    }
    syslog(LOG_USER | LOG_DEBUG,
           "format: \"%s\", formatted: \"%s\"", format, formatted);
    buffersize = strlen(formatted) + 1;
    /* FIXME: retrieve the actual array values */
    written = gsprintf(formatted, buffersize, format, args);
    if (formatted != NULL)
        gs_free_string(imemory, (byte *) formatted, buffersize,
                       "zsprintf formatted");
    if (format != NULL)
        gs_free_string(imemory, (byte *) format,
                       strlen(format) + 1, "zsprintf format");
    if (arg_is_string && (char *)args[0] != NULL)
        gs_free_string(imemory, (byte *) format,
                       strlen(args[0]), "zsprintf arg");
    /* ideally we should only pop(2) and modify the value of stringbuffer */
    pop(3);
    push(2);
    make_string(op - 1, a_all | icurrent_space,
                written >= buffersize ? buffersize - 1: written, formatted);
    make_bool(op, written < buffersize);
    return 0;
}

/* ------ Initialization procedure ------ */
const op_def zcasper_op_defs[] =
{
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
