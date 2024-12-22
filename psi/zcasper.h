#include "iref.h"
#define startswith(NAME, PART) strncmp(NAME, PART, strlen(PART)) == 0
#define endswith(NAME, PART) strcmp(NAME + strlen(NAME) - strlen(PART), PART) == 0
/* add support for casperscript extensions */
int cs_sleep(double seconds);  /* actual implementation of `sleep` */
int zcasperinit(i_ctx_t *i_ctx_p);
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
