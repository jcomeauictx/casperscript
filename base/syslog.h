#ifdef SYSLOG_DEBUGGING
#include <syslog.h>
#else
#define syslog(...)
#endif
