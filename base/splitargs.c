/* split shebang-supplied multiple options into individual arguments
 *
 * we could do this just by looking for spaces, or we could have a switch
 * that makes it explicit. I'm opting for -S which is what `env` uses; but
 * this means I'll need to modify gs options parsing which already accepts
 * -S<string> as a synonym for -s<string>, defining a string variable.
 * the 2nd arg, argv[1], will be the combined args, if such exist.
 */
#include <stdio.h>
#include <string.h>
#include "gssyslog.h"
#include "splitargs.h"

#ifdef TEST_SPLITARGS
/* you can, e.g., `make XCFLAGS="-DDEBUG_SPLITARGS=1" remake` */
#define DEBUG_SPLITARGS 1
#endif

#ifndef DEBUG_SPLITARGS
// nullify fprintf if not debugging
#define fprintf(...) {}
#endif

const char signal[] = "-S ";
const size_t offset = sizeof(signal) - 1;
const char delimiters[] = " \t";  /* space or tab only ones expected */

int splitargs(int argc, char **argv, char **argp) {
    char *substring;  /* for string which may be split */
    int i = 1, j = 1;
    syslog(LOG_USER | LOG_DEBUG, "starting splitargs()");
    /* store argv[0], the program name, first */
    argp[0] = argv[0];
    if (argc > 1 && strncmp(signal, argv[i], offset) == 0) {
        substring = &argv[i++][offset];
        while((argp[j] = strtok(substring, delimiters)) != NULL) {
            j++; substring = NULL;
        }
    }
    for (; i < argc; i++, j++) argp[j] = argv[i];
    syslog(LOG_USER | LOG_DEBUG, "ending splitargs()");
    return j;
}

int prependopts(int argc, char **argv, char **argp, char **prepend, int new) {
    /* unlike appendopts, "prepend" has to treat argv[0] specially, and not
     * move it up to another position. also, since argv and argp may be
     * the *same pointer*, things must be done so as to avoid overwriting */
    int i, j;
    /* simplest case: argc is 1, argv is [(bin/ccs)], new is [(-dARG)] */
    argp[0] = argv[0];
    /* argp is now [(bin/ccs)] */
    /* any existing args must first be moved to their previous position + new */
    for (i = argc + new, j = argc; j > 1;) argp[--i] = argv[--j];
    syslog(LOG_USER | LOG_DEBUG, "prepending %d new options", new);
    for (i = 0; i < new; i++) argp[i + 1] = prepend[i];
    /* argp is now [(bin/ccs) (-dARG)] */
    return argc + new;
}

int appendopts(int argc, char **argv, char **argp, char **append, int new) {
    /* this needs to take arg "--" specially; no options should come after it
     * also, don't allow adding 2nd "--" */
    int i, j, end_of_options = 0;
    fprintf(stderr, "appending %d new options to existing %d\n", new, argc);
    for (i = 0, j = 0; i < argc; i++, j++) {
        fprintf(stderr, "arg %d is \"%s\"\n", i, argv[i]);
        if (strcmp(argv[i], "--") == 0 && end_of_options == 0 && new > 0) {
            fprintf(stderr, "found end of options, moving from %d to %d\n",
                    j, j + new);
            if (strcmp(append[new - 1], "--") == 0) {  // don't add 2nd "--"
                fprintf(stderr, "oops, make that last number %d\n",
                        j + new - 1);
                new -= 1;
            }
            end_of_options = j;
            j += new;
            continue;
        }
        argp[j] = argv[i];
    }
    if (end_of_options > 0) {
        fprintf(stderr,
                "moving end of options \"%s\" from argv[%d] to argp[%d]\n",
                argv[end_of_options], end_of_options, end_of_options + new
               );
        argp[end_of_options + new] = argv[end_of_options];
    } else {
        fprintf(stderr, "setting end_of_options from 0 to %d\n", argc);
        end_of_options = argc;
    }
    fprintf(stderr, "appending new options starting offset %d\n",
            end_of_options);
    for (i = 0; i < new; i++) {
        j = i + end_of_options;
        fprintf(stderr, "argp[%d] (\"%s\") = append[%d] (\"%s\")\n",
                j, argp[j], i, append[i]);
        argp[j] = append[i];
    }
    return argc + new;
}
#ifdef TEST_SPLITARGS
int main(int argc, char **argv) {
    char *argp[1024];  /* likely overkill but still well under 1 MB */
    char *prepend[] = {(char *)"-pre0", (char *)"-pre1"};
    int prepended = sizeof(prepend) / sizeof(char *);
    char *append[] = {(char *)"-app0", (char *)"-app1", (char *)"--"};
    int appended = sizeof(append) / sizeof(char *);
    int i;
    fprintf(stderr, "dumping original argv (argc %d)\n", argc);
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    argc = splitargs(argc, argv, argp); argv = argp;
    fprintf(stderr, "dumping new but unchanged argv (argc %d)\n", argc);
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    argc = prependopts(argc, argv, argp, prepend, prepended); argv = argp;
    fprintf(stderr, "dumping new prepended argv (argc %d)\n", argc);
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    argc = appendopts(argc, argv, argp, append, appended); argv = argp;
    fprintf(stderr, "dumping new appended argv (argc %d)\n", argc);
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    return 0;
}
#endif
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
