/* split shebang-supplied multiple options into individual arguments
 *
 * we could do this just by looking for spaces, or we could have a switch
 * that makes it explicit. I'm opting for -S which is what `env` use; but
 * this means I'll need to modify gs options parsing which already accepts
 * -S<string> as a synonym for -s<string>, defining a string variable.
 * the 2nd arg, argv[1], will be the combined args, if such exist.
 */
#include <stdio.h>
#include <string.h>
#include "splitargs.h"

const char signal[] = "-S ";
const size_t offset = sizeof(signal) - 1;
const char delimiters[] = " \t";  /* space or tab only ones expected */

int splitargs(int argc, char **argv, char **argp) {
    char *substring;  /* for string which may be split */
    int i = 1, j = 1;
    /* store argv[0], the program name, first */
    argp[0] = argv[0];
    if (argc > 1 && strncmp(signal, argv[i], offset) == 0) {
        substring = &argv[i++][offset];
        while((argp[j] = strtok(substring, delimiters)) != NULL) {
            j++; substring = NULL;
        }
    }
    for (; i < argc; i++, j++) argp[j] = argv[i];
    return j;
}
#ifdef TEST_SPLITARGS
int main(int argc, char **argv) {
    char *argp[1024];  /* likely overkill but still well under 1 MB */
    size_t size;
    int i;
    size = splitargs(argc, argv, argp);
    fprintf(stderr, "dumping argp\n");
    for (i = 0; i < size; i++) fprintf(stderr, "argp[%d]: %s\n", i, argp[i]);
    return size;
}
#endif
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
