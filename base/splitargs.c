/* split shebang-supplied multiple options into individual arguments */
#include <syslog.h>
int main(int argc, char **argv)
    {
        char *splitargs[1024];  /* likely overkill but still only 1 MB */
        int argn = 0, i;
        for (i = 0; i < argc; i++) {splitargs[i] = argv[i]; argn++;}
        for (i = 0; i < argn; i++) syslog(LOG_USER | LOG_DEBUG, splitargs[i]);
    }
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
