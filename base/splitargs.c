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

int prependargs(int argc, char **argv, char **argp, char **prepend, int new) {
    /* unlike appendargs, "prepend" has to treat argv[0] specially, and not
     * move it up to another position */
    int i, j;
    /* simplest case: argc is 1, argv is [(bin/ccs)], new is [(-dARG)] */
    argp[0] = argv[0];
    /* argp is now [(bin/ccs)] */
    /* any existing args must first be moved to their previous position + new */
    for (i = new + 1, j = 1; j < argc; i++, j++) argp[i] = argv[j];
    syslog(LOG_USER | LOG_DEBUG, "prepending %d new args", new);
    for (i = 0; i < new; i++) argp[i + 1] = prepend[i];
    /* argp is now [(bin/ccs) (-dARG)] */
    return argc + new;
}

int appendargs(int argc, char **argv, char **argp, char **append, int new) {
    int i;
    syslog(LOG_USER | LOG_DEBUG, "appending %d new args", new);
    for (i = 0; i < argc; i++) argp[i] = argv[i];
    for (i = argc; i < argc + new; i++) argp[i] = append[i - argc];
    return argc + new;
}
#ifdef TEST_SPLITARGS
int main(int argc, char **argv) {
    char *argp[1024];  /* likely overkill but still well under 1 MB */
    char *prepend[] = {(char *)"pre0", (char *)"pre1"};
    int prepended = sizeof(prepend) / sizeof(char *);
    char *append[] = {(char *)"app0", (char *)"app1"};
    int appended = sizeof(append) / sizeof(char *);
    int i;
    fprintf(stderr, "dumping original argv\n");
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    argc = splitargs(argc, argv, argp); argv = argp;
    fprintf(stderr, "dumping new argv\n");
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    argc = prependargs(argc, argv, argp, prepend, prepended); argv = argp;
    fprintf(stderr, "dumping new argv\n");
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    argc = appendargs(argc, argv, argp, append, appended); argv = argp;
    fprintf(stderr, "dumping new argv\n");
    for (i = 0; i < argc; i++) fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    return 0;
}
#endif
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
