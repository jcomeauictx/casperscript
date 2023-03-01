/* split shebang-supplied multiple options into individual arguments
 *
 * we could do this just by looking for spaces, or we could have a switch
 * that makes it explicit. I'm opting for -S which is what `env` use; but
 * this means I'll need to modify gs options parsing which already accepts
 * -S<string> as a synonym for -s<string>, defining a string variable.
 */
#include <stdio.h>
#include <string.h>
char *signal = "-S ";
size_t offset = strlen(signal);
int main(int argc, char **argv)
    {
        char *argp[1024];  /* likely overkill but still well under 1 MB */
        char *substring;  /* for string which may be split */
        int i = 1, j = 1, argn;
        /* store argv[0], the program name, first */
        argp[0] = argv[0];
        if (strncmp(signal, argv[i], offset) == 0) {
            substring = &argv[i][offset]
            while(argp[j] = strtok(substring, " \t") != NULL) {
                j++; substring = NULL;
            }
        for (; i < argc; i++, j++) argp[j] = argv[i];
        fprintf(stderr, "dumping argp\n");
        for (i = 1; i < argn; i++) fprintf(stderr, "%s\n", argp[i]);
    }
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
