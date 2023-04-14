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

/* IBM 3852 JetPrinter color ink jet driver for Ghostscript */

/*
This driver program created by Kevin M. Gift <kgift@draper.com> in Sept. 1992.
Modified 3/93 to correct bug in cnt_2prn size.
Modified 3/93 to dimension page back to 8.5, which seems to
   work better than the actual page width of 7.6, ie. it uses
        the full printing width of the printer.
        It was modeled after the V2.4.1 HP Paintjet driver (gdevpjet.c)
Modified by L. Peter Deutsch <ghost@aladdin.com> 1999-01-10 to remove _ss
    modifiers inappropriately copied from other code.
 */

#include "gdevprn.h"
#include "gdevpcl.h"

/* X_DPI and Y_DPI must be the same - use the maximum graphics resolution */
/*   for this printer  */
#define X_DPI 84
#define Y_DPI 84

/* We round up LINE_SIZE to a multiple of 8 bytes */
/* because that's the unit of transposition from pixels to planes. */
/* Should = 96 (KMG) */
#define LINE_SIZE ((X_DPI * 86 / 10 + 63) / 64 * 8)

static void
jetp3852_initialize_device_procs(gx_device *dev)
{
    gdev_prn_initialize_device_procs(dev);

    set_dev_proc(dev, output_page, gdev_prn_bg_output_page);
    set_dev_proc(dev, map_rgb_color, gdev_pcl_3bit_map_rgb_color);
    set_dev_proc(dev, map_color_rgb, gdev_pcl_3bit_map_color_rgb);
    set_dev_proc(dev, encode_color, gdev_pcl_3bit_map_rgb_color);
    set_dev_proc(dev, decode_color, gdev_pcl_3bit_map_color_rgb);
}

/* The device descriptor */
static dev_proc_print_page(jetp3852_print_page);
/* Since the 'print_page' does not change the device, this device can print in the background */
const gx_device_printer far_data gs_jetp3852_device =
  prn_device(jetp3852_initialize_device_procs, "jetp3852",
        86,				/* width_10ths, 8.6" (?) */
        110,				/* height_10ths, 11" */
        X_DPI, Y_DPI,
        0.0, 0, 0.0, 0,		/* left, bottom, right, top margins */
        3, jetp3852_print_page);

/* ------ Internal routines ------ */

/* Send the page to the printer.  */
static int
jetp3852_print_page(gx_device_printer *pdev, gp_file *prn_stream)
{
#define DATA_SIZE (LINE_SIZE * 8)

    unsigned int cnt_2prn;
    unsigned int count,tempcnt;
    unsigned char vtp,cntc1,cntc2;
    int line_size_color_plane;
    int code = 0;

    byte data[DATA_SIZE];
    byte plane_data[LINE_SIZE * 3];

    /* Initialise data to zeros, otherwise later on, uninitialised bytes in
    dp[] can be greater than 7, which breaks spr8[dp[]]. */
    memset(data, 0x00, DATA_SIZE);

    /* Set initial condition for printer */
    gp_fputs("\033@",prn_stream);

    /* Send each scan line in turn */
    {
        int lnum;
        int line_size = gdev_mem_bytes_per_scan_line((gx_device *)pdev);
        int num_blank_lines = 0;

        if (line_size > DATA_SIZE) {
            emprintf2(pdev->memory, "invalid resolution and/or width gives line_size = %d, max. is %d\n",
                      line_size, DATA_SIZE);
            return_error(gs_error_rangecheck);
        }

        for ( lnum = 0; lnum < pdev->height; lnum++ ) {
            byte *end_data = data + line_size;
            code = gdev_prn_copy_scan_lines(pdev, lnum,
                                     (byte *)data, line_size);
            if (code < 0)
                break;
            /* Remove trailing 0s. */
            while ( end_data > data && end_data[-1] == 0 )
                end_data--;
            if ( end_data == data ) {
                /* Blank line */
                num_blank_lines++;
            } else {
                int i;
                byte *odp;
                byte *row;

                /* Transpose the data to get pixel planes. */
                for ( i = 0, odp = plane_data; i < DATA_SIZE;
                      i += 8, odp++
                    ) { /* The following is for 16-bit machines */
#define spread3(c)\
 { 0, c, c*0x100, c*0x101, c*0x10000L, c*0x10001L, c*0x10100L, c*0x10101L }
                    static ulong spr40[8] = spread3(0x40);
                    static ulong spr8[8] = spread3(8);
                    static ulong spr2[8] = spread3(2);
                    register byte *dp = data + i;
                    register ulong pword =
                                     (spr40[dp[0]] << 1) +
                                     (spr40[dp[1]]) +
                                     (spr40[dp[2]] >> 1) +
                                     (spr8[dp[3]] << 1) +
                                     (spr8[dp[4]]) +
                                     (spr8[dp[5]] >> 1) +
                                     (spr2[dp[6]]) +
                                     (spr2[dp[7]] >> 1);
                    odp[0] = (byte)(pword >> 16);
                    odp[LINE_SIZE] = (byte)(pword >> 8);
                    odp[LINE_SIZE*2] = (byte)(pword);
                }
                /* Skip blank lines if any */
                if ( num_blank_lines > 0 ) {
                    /* Do "dot skips" */
                    while(num_blank_lines > 255) {
                        gp_fputs("\033e\377",prn_stream);
                        num_blank_lines -= 255;
                    }
                    vtp = num_blank_lines;
                    gp_fprintf(prn_stream,"\033e%c",vtp);
                    num_blank_lines = 0;
                }

                /* Transfer raster graphics in the order R, G, B. */
                /* Apparently it is stored in B, G, R */
                /* Calculate the amount of data to send by what */
                /* Ghostscript tells us the scan line_size in (bytes) */

                count = line_size / 3;
                line_size_color_plane = count / 3;
                cnt_2prn = line_size_color_plane * 3 + 5;
                tempcnt = cnt_2prn;
                cntc1 = (tempcnt & 0xFF00) >> 8;
                cntc2 = (tempcnt & 0x00FF);
                gp_fprintf(prn_stream, "\033[O%c%c\200\037",cntc2,cntc1);
                gp_fputc('\000',prn_stream);
                gp_fputs("\124\124",prn_stream);

                for ( row = plane_data + LINE_SIZE * 2, i = 0;
                      i < 3; row -= LINE_SIZE, i++ ) {
                    int jj;
                    byte ctemp;
                    odp = row;
                    /* Complement bytes */
                    for (jj=0; jj< line_size_color_plane; jj++) {
                        ctemp = *odp;
                        *odp++ = ~ctemp;
                    }
                    gp_fwrite(row, sizeof(byte),
                              line_size_color_plane, prn_stream);
                }
            }
        }
    }

    /* eject page */
    gp_fputs("\014", prn_stream);

    return code;;
}
