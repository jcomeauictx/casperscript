/* add support for casperscript extensions */
#ifndef TEST_ZCASPER
static int zsleep(i_ctx_t *i_ctx_p);  /* implement `sleep` in casperscript */
#endif
int sleep(double seconds);  /* actual implementation of `sleep` */
// vim: tabstop=8 shiftwidth=4 expandtab softtabstop=4
