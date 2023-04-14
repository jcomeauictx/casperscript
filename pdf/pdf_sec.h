/* Copyright (C) 2020-2023 Artifex Software, Inc.
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

#ifndef PDF_SECURITY
#define PDF_SECURITY

int pdfi_compute_objkey(pdf_context *ctx, pdf_obj *obj, pdf_string **Key);
int pdfi_decrypt_string(pdf_context *ctx, pdf_string *string);
int pdfi_initialise_Decryption(pdf_context *ctx);

#endif
