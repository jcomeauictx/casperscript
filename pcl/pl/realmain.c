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

#include <stdlib.h>
#include "string_.h"
#include "plapi.h"
#include "gserrors.h"

#define PJL_UEL "\033%-12345X"

int
main(int argc, char *argv[])
{
    int code, code1;
    void *minst = NULL;
    size_t uel_len = strlen(PJL_UEL);
    int dummy;

    code = gsapi_new_instance(&minst, (void *)0);
    if (code < 0)
        return EXIT_FAILURE;

    code = gsapi_init_with_args(minst, argc, argv);
    if (code >= 0)
        code = gsapi_run_string_begin(minst, 0, &dummy);
    if (code >= 0)
        code = gsapi_run_string_continue(minst, PJL_UEL, uel_len, 0, &dummy);
    if (code >= 0)
        code = gsapi_run_string_end(minst, 0, &dummy);
    if (code == gs_error_InterpreterExit)
        code = 0;

    code1 = gsapi_exit(minst);
    if ((code == 0) || (code == gs_error_Quit))
        code = code1;

    gsapi_delete_instance(minst);

    if ((code == 0) || (code == gs_error_Quit))
        return EXIT_SUCCESS;
    return EXIT_FAILURE;
}
