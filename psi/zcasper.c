/* add support for casperscript extensions */
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "zcasper.h"
#include "syslog.h"

int zsleep(double seconds) {  /* implement `sleep` in casperscript */
    int iseconds = (int)seconds;
    double fractional = seconds - iseconds;
    long nanoseconds = roundl(fractional * 1000000000L);
    syslog(LOG_USER | LOG_DEBUG, "%d s, %ld ns", iseconds, nanoseconds);
    return 0;
}

#ifdef TEST_ZCASPER
int main(int argc, char **argv) {
    return zsleep(atof(argv[1]));
}
#endif

// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
