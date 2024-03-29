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

/*
 * C.Itoh M8510 printer driver for ghostscript.
 *
 * By Bob Smith <bob@snuffy.penfield.ny.us>
 */

#include "gdevprn.h"

/* The device descriptor */
static dev_proc_print_page(m8510_print_page);
const gx_device_printer far_data gs_m8510_device =
        /* The print_page proc is compatible with allowing bg printing */
        prn_device(gdev_prn_initialize_device_procs_mono_bg, "m8510",
                85,				/* width_10ths, 8.5" */
                110,				/* height_10ths, 11" */
                160,				/* x_dpi */
                144,				/* y_dpi */
                0,0,0.5,0,			/* left, bottom, right, and top margins */
                1, m8510_print_page);

/* ------ forward declarations ------ */

static void m8510_output_run(gx_device_printer *pdev,
        byte *out, int pass, gp_file *prn_stream);

/* ------ internal routines ------ */

/* Send the page to the printer. */
static int
m8510_print_page(gx_device_printer *pdev, gp_file *prn_stream)
{
        int line_size = gdev_mem_bytes_per_scan_line((gx_device *)pdev);
        byte *in1 = (byte *) gs_malloc(pdev->memory, 8, line_size, "m8510_print_page(in1)");
        byte *in2 = (byte *) gs_malloc(pdev->memory, 8, line_size, "m8510_print_page(in2)");
        byte *out = (byte *) gs_malloc(pdev->memory, 8, line_size, "m8510_print_page(out)");
        int lnum = 0;
        int code = 0;
        byte *inp, *in_end, *outp;
        int i;

        if (in1 == 0 || in2 == 0 || out == 0) {
                code = gs_note_error(gs_error_VMerror);
                goto out;
        }

        /*
         * Initialize the printer.
         * NLQ mode, proportional print (160x144 dpi).
         * and 16/144" linefeeds.
         */
        gp_fwrite("\033m2\033P\033T16", 1, 9, prn_stream);

        /* Transfer pixels to printer */
        while ( lnum < pdev->height ) {
                /* get a raster */
                for (i = 7; i >= 0; i--) {
                        code = gdev_prn_copy_scan_lines(pdev, lnum, &in1[i*line_size], line_size);
                        if (code < 0)
                            goto out;
                        lnum++;
                        code = gdev_prn_copy_scan_lines(pdev, lnum, &in2[i*line_size], line_size);
                        if (code < 0)
                            goto out;
                        lnum++;
                }

                /* Transpose the 1st pass of data. */
                in_end = in1 + line_size;
                for (inp = in1, outp = out; inp < in_end; inp++, outp += 8)
                        gdev_prn_transpose_8x8(inp, line_size, outp, 1);

                /* send the 1st line */
                m8510_output_run(pdev, out, 0, prn_stream);

                /* Transpose the 2nd pass of data. */
                in_end = in2 + line_size;
                for (inp = in2, outp = out; inp < in_end; inp++, outp += 8)
                        gdev_prn_transpose_8x8(inp, line_size, outp, 1);

                /* send the 2nd line */
                m8510_output_run(pdev, out, 1, prn_stream);
        }

        /* reset the printer. */
        gp_fwrite("\033c1", 1, 3, prn_stream);
        gp_fflush(prn_stream);

out:;
        if (out) gs_free(pdev->memory, (char *) out, 8, line_size, "m8510_print_page(out)");
        if (in2) gs_free(pdev->memory, (char *) in2, 8, line_size, "m8510_print_page(in2)");
        if (in1) gs_free(pdev->memory, (char *) in1, 8, line_size, "m8510_print_page(in1)");

        return code;
}

static void
m8510_output_run(gx_device_printer *pdev,
        byte *out, int pass, gp_file *prn_stream)
{
        byte *out_end = out + ((pdev->width + 7) & -8);	/* round up to multiple of 8 */
        char tmp[10];
        int count;

        /*
         * Remove trailing 0s.
         * out must be a multiple of 8 bytes.
         */
        while (out_end > out
                && out_end[-1] == 0
                && out_end[-2] == 0
                && out_end[-3] == 0
                && out_end[-4] == 0
                && out_end[-5] == 0
                && out_end[-6] == 0
                && out_end[-7] == 0
                && out_end[-8] == 0)
                        out_end -= 8;

        /* Transfer the line of data. */
        count = out_end - out;
        if (count > 0) {
                gs_snprintf(tmp, sizeof(tmp), "\033g%03d", count/8);
                gp_fwrite(tmp, 1, 5, prn_stream);
                gp_fwrite(out, 1, count, prn_stream);
                gp_fwrite("\r", 1, 1, prn_stream);
        }

        if (pass) gp_fwrite("\n", 1, 1, prn_stream);
}
