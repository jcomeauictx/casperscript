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
#include "gssyslog.h"  /* for syslog debugging */
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

char * memdump(char *buffer, void *location, int count);
int get_int(ref object);
double get_real(ref object);

#define BUFFERSIZE 1024
#define SHORTBUFFERSIZE 7

#define NEXT_ARG \
  do { \
    code = array_get(imemory, &args, argindex++, &arg); \
    if (code < 0) return code;  /* aborts safely */ \
  } while (0)

#define COPY_INT \
  do { \
    NEXT_ARG; \
    const int value = arg.value.intval; \
    char buf[32]; \
    /*errprintf("COPY_INT called with value %d\n", value);*/ \
    ptr++; /* Go past the asterisk.  */ \
    *sptr = '\0'; /* NULL terminate sptr.  */ \
    sprintf(buf, "%d", value); \
    strcat(sptr, buf); \
    /*errprintf("COPY_INT result: %s\n", sptr);*/ \
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
    int result; \
    NEXT_ARG; \
    syslog(LOG_USER | LOG_DEBUG, \
        "value at PRINT_TYPE: 0x%x (%d), float: %f", VALUE, VALUE, VALUE); \
    *sptr++ = *ptr++; /* Copy the type specifier.  */ \
    *sptr = '\0'; /* NULL terminate sptr.  */ \
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

int gsprintf(i_ctx_t *i_ctx_p)
{
  char buffer[MAX_STR + 1];
  char formatstring[MAX_STR + 1];
  char argstring[MAX_STR + 1];
  const char * ptr = formatstring;
  char specifier[128];
  int total_printed = 0, argindex;
  long longvalue;
  double doublevalue;
  ref formatted, format, args, arg;
  
  os_ptr op = osp;
  formatted = *(op - 2);
  format = *(op - 1);
  args = *op;
  /* trust that no gs-supplied string will be longer than MAX_STR */
  /* this, combined with strncpy spec, will ensure trailing \0 */
  DISCARD(strncpy(buffer, formatted->value.bytes, r_size(formatted) + 1));
  DISCARD(strncpy(formatstring, format->value.bytes, r_size(format) + 1));
  while (*ptr != '\0')
    {
      if (*ptr != '%') /* While we have regular characters, print them.  */
	PRINT_CHAR(*ptr);
      else /* We got a format specifier! */
	{
	  char * sptr = specifier;
	  int wide_width = 0, short_width = 0;
	  
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
	    {
	      switch (*ptr)
		{
		case 'h':
		  short_width = 1;
		  break;
		case 'l':
		  wide_width++;
		  break;
		case 'L':
		  wide_width = 2;
		  break;
		default:
		  abort();
		}
	      *sptr++ = *ptr++;
	    }

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
                NEXT_ARG;
                longvalue = (DEFAULT_INT_TYPE) arg.value.intval;
                syslog(LOG_USER | LOG_DEBUG,
                    "longvalue: 0x%x (%d)", longvalue, longvalue);
		PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
              }
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      {
                NEXT_ARG;
                doublevalue = (DEFAULT_TYPE) arg.value.realval;
                syslog(LOG_USER | LOG_DEBUG,
                  "doublevalue: %f", doublevalue);
		PRINT_TYPE(double, doublevalue);
	      }
	      break;
	    case 's':
              NEXT_ARG;
              longvalue = (DEFAULT_INT_TYPE) arg.value.intval;
	      PRINT_TYPE(char *, longvalue);
	      break;
	    case 'p':
              NEXT_ARG;
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

  return total_printed;
}

char * memdump(char *buffer, void *location, int count) {
  unsigned char uchar, *ptr;
  char *saved = buffer;
  int i;
  buffer += sprintf(buffer, "%p: ", location);
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
/* vim: tabstop=8 shiftwidth=2 expandtab softtabstop=2
 */
