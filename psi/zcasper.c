/* add support for casperscript extensions */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "zcasper.h"
#include "syslog.h"

int zsleep(double seconds) {  /* implement `sleep` in casperscript */
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

#ifdef TEST_ZCASPER
int main(int argc, char **argv) {
    int code;
    if (argc < 2) {
        fprintf(stderr, "Must specify sleep time\n");
        code = 1;
    }
    else code = zsleep(atof(argv[1]));
    return code;
}
#endif

// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
