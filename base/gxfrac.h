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


/* Fraction representation for Ghostscript */

#ifndef gxfrac_INCLUDED
#  define gxfrac_INCLUDED

/*
 * Represent a fraction in [0.0..1.0].  Note that the 1.0 endpoint is
 * included.  Since undercolor removal requires a signed frac, we limit
 * fracs to 15 bits rather than 16.
 */
typedef short frac;
typedef short signed_frac;

#define arch_log2_sizeof_frac ARCH_LOG2_SIZEOF_SHORT
#define arch_sizeof_frac ARCH_SIZEOF_SHORT
#define frac_bits 15
#define frac_0 ((frac)0)

/*
 * Normally one would represent a fractional value of this kind as a short
 * integer, in [-32767..32767].  Unfortunately, this approach cannot
 * represent any of the common values like 1/2, 1/3, or 1/5 exactly, causing
 * rounding errors.  Instead, we opt for using the range [-32760..32760],
 * which allows exact representation of almost all commonly used fractions
 * (e.g., N/360 for 0<=N<=360).
 */
#define frac_1_0bits 3
#define frac_1 ((frac)0x7ff8)
#define frac_1_long ((long)frac_1)
#define frac_1_float ((float)frac_1)
/* Conversion between fracs and floats. */
#define frac2float(fr) ((fr) / frac_1_float)
#define float2frac(fl) ((frac)(((fl) + 0.5 / frac_1_float) * frac_1_float))

/*
 * Conversion between unsigned fracs and bytes (or, in general,
 * shorter integers) representing fractions. This is highly dependent
 * on the definition of frac_1 above.
 */
#define _frac2s(fr)\
  (((fr) >> (frac_bits - frac_1_0bits)) + (fr))
#define frac2bits(fr, nb)\
  ((uint)(_frac2s(fr) >> (frac_bits - (nb))))
#define frac2byte(fr) ((byte)frac2bits(fr, 8))
/* bits2frac requires frac_bits / 2 <= nb <= frac_bits. */
#define bits2frac(v, nb) ((frac)(\
  ((frac)(v) << (frac_bits - (nb))) +\
   ((v) >> ((nb) * 2 - frac_bits)) -\
   ((v) >> ((nb) - frac_1_0bits)) ))
#define byte2frac(b) bits2frac(b, 8)
/* Produce a result that is guaranteed to convert back to a frac */
/* not exceeding the original value fr. */
#define frac2bits_floor(fr, nb)\
  ((uint)((_frac2s(fr) - (_frac2s(fr) >> (nb))) >> (frac_bits - (nb))))
/*
 * Conversion between fracs and unsigned shorts.
 */
#define ushort_bits (ARCH_SIZEOF_SHORT * 8)
#define frac2ushort(fr) ((ushort)(\
  ((fr) << (ushort_bits - frac_bits)) +\
  ((fr) >> (frac_bits * 2 - ushort_bits - frac_1_0bits)) ))
#define ushort2frac(us) ((frac)(\
  ((us) >> (ushort_bits - frac_bits)) -\
  ((us) >> (ushort_bits - frac_1_0bits)) ))

/*
 * Conversion between frac31s and unsigned shorts.
 */
#define ushort_bits (ARCH_SIZEOF_SHORT * 8)
#define frac31_bits 31
#if 0
#define frac312ushort(fr) ((ushort)(\
  ((fr) >> (frac31_bits - ushort_bits)) +\
  ((fr) >> (frac31_bits - ushort_bits + frac_bits - frac_1_0bits)) ))
#define ushort2frac31(us) ((frac31)(\
  ((us) << (frac31_bits - ushort_bits)) -\
  ((us) << (frac31_bits - ushort_bits - frac_bits + frac_1_0bits)) ))
#else
#define frac312ushort(fr) ((ushort)((fr) >> (frac31_bits - ushort_bits)))
#define ushort2frac31(us) ((frac31)((us) << (frac31_bits - ushort_bits)))
#endif

/*
 * Compute the quotient Q = floor(P / frac_1),
 * where P is the (ulong) product of a uint or ushort V and a frac F.
 * See gxarith.h for the underlying algorithm.
 */
#define frac_1_quo(p)\
  ( (((p) >> frac_1_0bits) + ((p) >> frac_bits) + 1) >> (frac_bits - frac_1_0bits) )
/*
 * Compute the remainder similarly, having already computed the quotient.
 * This is, of course, P - Q * frac_1.
 */
#define frac_1_rem(p, q)\
  ((frac)( (uint)(p) - ((q) << frac_bits) + ((q) << frac_1_0bits) ))

#endif /* gxfrac_INCLUDED */
