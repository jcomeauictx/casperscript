/* chop extension in mutable string by replacing final dot (if any) with \0.
 *
 * must use basename *first*, otherwise will overwrite a dot anywhere in path.
 */
#include <stdio.h>
#include <string.h>
#include "gssyslog.h"
#include "chop_extension.h"

int delimiter = '.';

char *chop_extension(char *filename) {
    char *p;
    syslog(LOG_USER | LOG_DEBUG, "starting chop_extension(\"%s\")", filename);
    if ((p = strrchr(filename, delimiter)) != NULL) *p = '\0';
    syslog(LOG_USER | LOG_DEBUG, "ending chop_extension()");
    return filename;
}
#ifdef TEST_CHOP_EXTENSION
int main(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        printf("%s becomes ", argv[i]);
        printf("%s\n", chop_extension(argv[i]));
    }
}
#endif
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
