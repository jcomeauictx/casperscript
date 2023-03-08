/* substring, for debugging with syslog */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "substr.h"

char *substr(char *dest, const char *src, int start, int length) {
    return strncpy(dest, src + start, length);
}

#ifdef TEST_SPLITARGS
int main(int argc, char **argv) {
    char *buffer;
    int start, length;
    if (argc > 2) {
        buffer = argv[1];
        if (argc >= 4) start = atoi(argv[3]); else start = 0;
        if (argc >= 5) length = atoi(argv[4]); else length = strlen(argv[1]);
        if (length > strlen(argv[1])) {
            fprintf(stderr, "Length will overflow buffer, shortening\n");
            length = strlen(argv[1]);
        }
        if (argc >= 6) fprintf(stderr, "Extra args ignored\n");
        printf("\"%s\"\n", substr(buffer, argv[2], start, length));
        return 0;
    } else {
        fprintf(stderr, "Must at least specify buffer and source strings\n");
    }
}
#endif
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
