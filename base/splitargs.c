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
/* you can, e.g., `XCFLAGS=-DDEBUG_SPLITARGS=1 make remake` */
#define DEBUG_SPLITARGS 1
#else
#endif

#ifdef DEBUG_SPLITARGS
#else
// turn fprintf into syslog if not debugging
#define stderr (LOG_USER | LOG_DEBUG)
#define fprintf(...) syslog(...)
#define dump(...)
#endif

const char signal[] = "-S ";
const size_t offset = sizeof(signal) - 1;
const char delimiters[] = " \t";  /* space or tab only ones expected */

int splitargs(int argc, char **argv, char **argp) {
    char *substring;  /* for string which may be split */
    int i = 1, j = 1;
    fprintf(stderr, "starting splitargs()\n");
    /* store argv[0], the program name, first */
    argp[0] = argv[0];
    if (argc > 1 && strncmp(signal, argv[i], offset) == 0) {
        substring = &argv[i++][offset];
        while((argp[j] = strtok(substring, delimiters)) != NULL) {
            j++; substring = NULL;
        }
    }
    for (; i < argc; i++, j++) argp[j] = argv[i];
    fprintf(stderr, "dumping new but unchanged argv (argc %d)\n", argc);
    dump(j, argp);
    fprintf(stderr, "ending splitargs()\n");
    return j;
}

int prependopts(int argc, char **argv, char **argp, char **prepend, int new) {
    /* unlike appendopts, "prepend" has to treat argv[0] specially, and not
     * move it up to another position. also, since argv and argp may be
     * the *same pointer*, things must be done so as to avoid overwriting */
    int i, j;
    fprintf(stderr, "prependopts called with %d args\n", new);
    /* simplest case: argc is 1, argv is [(bin/ccs)], new is [(-dARG)] */
    if (new > 0) {
        argp[0] = argv[0];
        /* argp is now [(bin/ccs)] */
        /* any existing args must first be moved to their
         * previous position + new */
        for (i = argc + new, j = argc; j > 1;) argp[--i] = argv[--j];
        fprintf(stderr, "prepending %d new options\n", new);
        for (i = 0; i < new; i++) argp[i + 1] = prepend[i];
        /* argp is now [(bin/ccs) (-dARG)] */
        argc += new;
        fprintf(stderr, "dumping revised options after prependopts\n");
        dump(argc, argp);
    }
    return argc;
}

int find_end_of_options(int argc, char **argv) {
    int i = 0, found = 0;
    for (; i < argc; i++) {
        fprintf(stderr, "arg %d is \"%s\"\n", i, argv[i]);
        if (strcmp(argv[i], "--") == 0) {
            fprintf(stderr, "found end of options at index %d\n", i);
            found = 1;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "no end of options found\n");
        i = -1;
    }
    return i;
}
int appendopts(int argc, char **argv, char **argp, char **append, int new) {
    /* this needs to take arg "--" specially; no options should come after it
     * also, don't allow adding 2nd "--" */
    int i, j, end = 0;
    fprintf(stderr, "appendopts called with %d args\n", new);
    if (new == 0) {
        fprintf(stderr, "nothing to append to existing %d args\n", argc);
        return argc;
    }
    fprintf(stderr, "appending %d new options to existing %d\n", new, argc);
    if ((end = find_end_of_options(argc, argv)) > -1) {
        if (strcmp(append[new - 1], "--") == 0) {
            fprintf(stderr, "dropping 2nd `==`\n");
            new -= 1;
        }
    } else {
        end = argc;
    }
    for (i = 0, j = 0; i < end; i++, j++) {
        fprintf(stderr, "arg %d is \"%s\"\n", i, argv[i]);
        argp[j] = argv[i];
    }
    /* at this point, we have forward-moved everything up to but not
     * including the `--`, if there was one. anything left to move will
     * need to be done far-to-near, to avoid overwriting options or args */
    if (end < argc) {
        for (i = argc - 1, j = argc - 1 + new; i >= end; i--, j--) {
            fprintf(stderr, "moving argc[%d] (\"%s\") to index %d (\"%s\")\n",
                    i, argv[i], j, argp[j]);
            argp[j] = argv[i];
        }
    }
    fprintf(stderr, "appending new options starting offset %d\n", end);
    for (i = 0; i < new; i++) {
        j = i + end;
        fprintf(stderr, "argp[%d] (\"%s\") = append[%d] (\"%s\")\n",
                j, argp[j], i, append[i]);
        argp[j] = append[i];
    }
    argc += new;
    fprintf(stderr, "dumping revised options after appendopts\n");
    dump(argc, argp);
    return argc;
}
#ifdef TEST_SPLITARGS
int main(int argc, char **argv) {
    char *argp[1024];  /* likely overkill but still well under 1 MB */
    char *prepend[] = {(char *)"-pre0", (char *)"-pre1"};
    int prepended = sizeof(prepend) / sizeof(char *);
    char *append[] = {(char *)"-app0", (char *)"-app1", (char *)"--"};
    int appended = sizeof(append) / sizeof(char *);
    fprintf(stderr, "dumping original argv (argc %d)\n", argc);
    dump(argc, argv);
    argc = splitargs(argc, argv, argp);
    argv = argp;  // NOTE: we're equating the two pointers, could be problems!
    argc = prependopts(argc, argv, argp, prepend, prepended);
    argv = argp;
    argc = appendopts(argc, argv, argp, append, appended);
    argv = argp;
    return 0;
}
#endif
#ifdef DEBUG_SPLITARGS
int dump(int argc, char **argv) {
    int i;
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    return 0;
}
#endif
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
