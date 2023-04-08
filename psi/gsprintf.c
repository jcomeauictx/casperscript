/* Provide a version of sprintf that uses a GhostScript t_array rather
   than a va_list, and using a buffer of fixed size. Based on _doprnt.c,
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Contributed by Kaveh Ghazi  (ghazi@caip.rutgers.edu)  3/29/98
   gsprintf.c edits by John Comeau <jc@unternet.net> 2023-03-25

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "gsprintf.h"
#include "gssyslog.h"  /* for syslog debugging */

#ifndef GSPRINTF_MAIN
/* the following 6 imports are for ghostscript integration */
#include "ghost.h"
#include "gserrors.h"
#include "oper.h"
#include "store.h"
#include "estack.h"
#include "std.h"

#define ISDIGIT(c) (c <= '9' && c >= '0')
#define DEFAULT_TYPE ref
#define DEFAULT_INT_TYPE long
#define DEFAULT_REAL_TYPE double
#endif  /* GSPRINTF_MAIN */

char * memdump(char *buffer, void *location, int count);

#define GSPRINTF_MAX 4096
#define BUFFERSIZE GSPRINTF_MAX
#define SHORTBUFFERSIZE 7
#define INTBUFFERSIZE 32

#ifndef GSPRINTF_MAIN
#define NEXT_ARG(TYPE) \
  do { \
    code = array_get(NULL, args, argindex++, argptr); \
    if (code < 0) return code; \
    else if (TYPE != r_type(argptr)) return_error(gs_error_typecheck); \
  } while (0)

#define COPY_INT \
  do { \
    char buf[INTBUFFERSIZE]; \
    NEXT_ARG(t_integer); \
    longvalue = (long)arg.value.intval; \
    ptr++; /* Go past the asterisk.  */ \
    *sptr = '\0'; /* NULL terminate sptr.  */ \
    snprintf(buf, INTBUFFERSIZE - 1, "%ld", longvalue); \
    strcat(sptr, buf); \
    while (*sptr) sptr++; \
  } while (0)

#define PRINT_CHAR(CHAR) \
  do { \
   if (total_printed < maxlength) \
     *(buffer + total_printed++) = *ptr++; \
   else {total_printed++; ptr++;} \
  } while (0)

#define PRINT_TYPE(TYPE, VALUE) \
  do { \
    int result = 0; \
    *sptr++ = *ptr++; /* Copy the type specifier.  */ \
    *sptr = '\0'; /* NULL terminate sptr.  */ \
    if (total_printed < maxlength) \
      result = snprintf(buffer + total_printed, \
        maxlength - total_printed, specifier, (TYPE) VALUE); \
    if (result == -1) \
      return -1; \
    else \
      { \
        total_printed += result; \
        continue; \
      } \
  } while (0)

#define GETSTR(DESTINATION, REF, MAXSIZE) \
  do { \
    int size = r_size(REF); \
    char *src = (char *)REF->value.bytes; \
    if (size >= MAXSIZE)  /* needs to be at least one byte less */ \
      return_error(gs_error_rangecheck); \
    /* From `man strncpy`:
     * If the length of src is less than n, strncpy() writes additional null
     * bytes to dest to ensure that a total of n bytes are written.
     * HOWEVER: remember Ghostscript "strings" are *not* null terminated, so
     * you cannot COUNT on the "length of src" being detectable by C! */ \
    else {strncpy(DESTINATION, src, size); DESTINATION[size] = '\0';} \
  } while (0)
 
int gsprintf(ref *formatted, ref *format, ref *args)
{
  char *buffer;
  char formatstring[BUFFERSIZE];
  char argstring[BUFFERSIZE];
  const char * ptr = formatstring;
  char specifier[128];
  int total_printed = 0, argindex = 0, maxlength, code = 0;
  long longvalue;
  double doublevalue;
  ref arg, *argptr;
  argptr = &arg;

  syslog(LOG_USER | LOG_DEBUG, "gsprintf starting");
  buffer = (char *)(formatted->value.bytes);
  maxlength = r_size(formatted);
  if (maxlength > BUFFERSIZE) return_error(gs_error_rangecheck);
  GETSTR(formatstring, format, BUFFERSIZE);
  while (*ptr != '\0')
    {
      if (*ptr != '%') /* While we have regular characters, print them.  */
	PRINT_CHAR(*ptr);
      else /* We got a format specifier! */
	{
	  char * sptr = specifier;
	  *sptr++ = *ptr++; /* Copy the % and move forward.  */
	  while (strchr ("-+ #0", *ptr)) /* Move past flags.  */
	    *sptr++ = *ptr++;
	  if (*ptr == '*')
	    COPY_INT;
	  else
	    while (ISDIGIT(*ptr)) /* Handle explicit numeric value.  */
	      *sptr++ = *ptr++;
	  if (*ptr == '.')
	    {
	      *sptr++ = *ptr++; /* Copy and go past the period.  */
	      if (*ptr == '*')
		COPY_INT;
	      else
		while (ISDIGIT(*ptr)) /* Handle explicit numeric value.  */
		  *sptr++ = *ptr++;
	    }
	  while (strchr ("hlL", *ptr))
            /* we only have one width for ints and floats, so ignore these */
            *sptr++ = *ptr++;

	  switch (*ptr)
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      {
		/* Short values are promoted to int, so just cast everything
                   to a long and trust the C library sprintf with it */
                NEXT_ARG(t_integer);
                longvalue = (long)arg.value.intval; \
                syslog(LOG_USER | LOG_DEBUG,
                    "longvalue: 0x%lx (%ld)", longvalue, longvalue);
		PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
              }
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      {
                NEXT_ARG(t_real);
                doublevalue = (DEFAULT_REAL_TYPE) arg.value.realval;
                syslog(LOG_USER | LOG_DEBUG, "doublevalue: %f", doublevalue);
		PRINT_TYPE(double, doublevalue);
	      }
	      break;
	    case 's':
              NEXT_ARG(t_string);
              GETSTR(argstring, argptr, BUFFERSIZE);
	      PRINT_TYPE(char *, argstring);
	      break;
	    case 'p':
              NEXT_ARG(t_integer);
              longvalue = (DEFAULT_INT_TYPE) arg.value.intval;
	      PRINT_TYPE(void *, longvalue);
	      break;
	    case '%':
	      PRINT_CHAR('%');
	      break;
	    default:
	      abort();
	    } /* End of switch (*ptr) */
	} /* End of else statement */
    }
  syslog(LOG_USER | LOG_DEBUG, "gsprintf final string \"%*s\", %d bytes",
         total_printed, buffer, total_printed);
  return total_printed;
}
#endif  /* GSPRINTF_MAIN */

char * memdump(char *buffer, void *location, int count) {
  unsigned char uchar, *ptr;
  char *saved = buffer;
  int i;
  buffer += snprintf(buffer, INTBUFFERSIZE - 1, "%p: ", location);
  buffer--; /* so we can increment at start of each hex digit */
  ptr = (unsigned char *)location;
  for (i = 0; i < count; i++) {
    uchar = *ptr++;
    *++buffer = uchar >> 4; *buffer += *buffer > 9 ? 'a' - 10 : '0';
    *++buffer = uchar & 0xf; *buffer += *buffer > 9 ? 'a' - 10: '0';
  }
  *buffer = '\0';
  return saved;
}

#ifdef GSPRINTF_MAIN
int main() {
  int foo = 0xfedcba98;
  char *bar = "zyxwvuts";
  char buffer[1024];
  char memory[1024];
  syslog(LOG_USER | LOG_DEBUG, memdump(buffer, &foo, 64));
  return 0;
}
#endif
/* vim: tabstop=8 shiftwidth=2 expandtab softtabstop=2
 */
