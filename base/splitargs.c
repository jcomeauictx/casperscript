/* split shebang-supplied multiple options into individual arguments */
#include <stdio.h>
int main(int argc, char **argv)
    {
        char *argp[1024];  /* likely overkill but still only 1 MB */
        int argn = 1, i;
        /* store argv[0], the program name, first */
        argp[0] = argv[0];
        for (i = 1; i < argc; i++) {argp[i] = argv[i]; argn++;}
        fprintf(stderr, "dumping argp\n");
        for (i = 1; i < argn; i++) fprintf(stderr, "%s\n", argp[i]);
    }
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
