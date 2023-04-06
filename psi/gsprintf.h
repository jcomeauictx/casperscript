#include "iref.h"  /* for i_ctx_t */
int gsprintf(i_ctx_t *i_ctx_p);
/* MAX_STR definition copied from mkromfs.c
 * FIXME: shouldn't this be something central and prominent? Like in ghost.h?
 */
#define MAX_STR 0xffff	/* longest possible PostScript string token */
