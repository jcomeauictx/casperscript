/* Copyright (C) 2001-2023 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  39 Mesa Street, Suite 108A, San Francisco,
   CA 94129, USA, for further information.
*/


/* XPS interpreter - unicode text functions */

#include "ghostxps.h"

/*
 * http://tools.ietf.org/html/rfc3629
 */

int
xps_utf8_to_ucs(int *p, const char *ss, int n)
{
    unsigned char *s = (unsigned char *)ss;

    if (s == NULL)
        goto bad;

    if ((s[0] & 0x80) == 0)
    {
        *p = (s[0] & 0x7f);
        return 1;
    }

    if ((s[0] & 0xe0) == 0xc0)
    {
        if (n < 2 || s[1] < 0x80)
            goto bad;
        *p = (s[0] & 0x1f) << 6;
        *p |= (s[1] & 0x3f);
        return 2;
    }

    if ((s[0] & 0xf0) == 0xe0)
    {
        if (n < 3 || s[1] < 0x80 || s[2] < 0x80)
            goto bad;
        *p = (s[0] & 0x0f) << 12;
        *p |= (s[1] & 0x3f) << 6;
        *p |= (s[2] & 0x3f);
        return 3;
    }

    if ((s[0] & 0xf8) == 0xf0)
    {
        if (n < 4 || s[1] < 0x80 || s[2] < 0x80 || s[3] < 0x80)
            goto bad;
        *p = (s[0] & 0x07) << 18;
        *p |= (s[1] & 0x3f) << 12;
        *p |= (s[2] & 0x3f) << 6;
        *p |= (s[3] & 0x3f);
        return 4;
    }

bad:
    *p = 0x80;
    return 1;
}
