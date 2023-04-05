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
#ifdef SYSLOG_DEBUGGING
#include <syslog.h>  /* for debugging */
#else
#define syslog(...)
#endif

#define ISDIGIT(c) (c <= '9' && c >= '0')
/* assuming that any architecture that has long longs has long doubles */
#if !defined(DISABLE_LONG_LONGS)
#if defined(__GNUC__) || defined (HAVE_LONG_LONG)
#define USE_LONG_LONGS
#endif  /* HAVE_LONG_LONG */
#endif /* DISABLE_LONG_LONG */

#ifdef USE_LONG_LONGS
#define DEFAULT_TYPE long double
#define DEFAULT_INT_TYPE long long
#else
#define DEFAULT_TYPE double
#define DEFAULT_INT_TYPE long
#endif
#define FORCE_CAST (DEFAULT_TYPE)(DEFAULT_INT_TYPE)  /* for pointers */
#define CAST_ARGS (DEFAULT_TYPE [])
#define CAST_ARG (DEFAULT_TYPE)
/* shorthand for building compound literals */
#define F CAST_ARG
#define I (DEFAULT_INT_TYPE)
#define P FORCE_CAST

int errprintf(const char *format, ...);
char * memdump(char *buffer, void *location, int count);
int precheckit(int buffersize, const char *format, DEFAULT_TYPE *args);
int checkit(const char * format, DEFAULT_TYPE *args);
int testsnprintf(int size, const char *format, ...);

#define BUFFERSIZE 1024
#define SHORTBUFFERSIZE 7

#define COPY_INT \
  do { \
	 const int value = (int)(*args++); \
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
     *(formatted + total_printed++) = *ptr++; \
   else {total_printed++; ptr++;} \
  } while (0)

#define PRINT_TYPE(TYPE, VALUE) \
  do { \
    int result; \
    syslog(LOG_USER | LOG_DEBUG, \
        "value at PRINT_TYPE: 0x%x (%d), float: %f", VALUE, VALUE, VALUE); \
    *sptr++ = *ptr++; /* Copy the type specifier.  */ \
    *sptr = '\0'; /* NULL terminate sptr.  */ \
    result = snprintf(formatted + total_printed, \
      maxlength - total_printed, specifier, (TYPE) VALUE); \
    if (result == -1) \
      return -1; \
    else \
      { \
        total_printed += result; \
        continue; \
      } \
  } while (0)

int gsprintf (char *formatted, int maxlength, const char *format,
              DEFAULT_TYPE *args);

int gsprintf (char *formatted, int maxlength, const char *format,
              DEFAULT_TYPE *args)
  /* NOTE that `maxlength` should always be at least 1 less than the size
   * of the `formatted` buffer */
{
  const char * ptr = format;
  char specifier[128];
  int total_printed = 0;
#ifdef USE_LONG_LONGS
  long long longvalue;
  long double doublevalue;
#else
  long longvalue;
  double doublevalue;
#endif
  
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
                longvalue = (DEFAULT_INT_TYPE) *args++;
                syslog(LOG_USER | LOG_DEBUG,
                    "longvalue: 0x%x (%d)", longvalue, longvalue);
		/* Short values are promoted to int, so just copy it
                   as an int and trust the C library printf to cast it
                   to the right width.  */
		if (short_width)
		  PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
		else
		  {
		    switch (wide_width)
		      {
		      case 0:
			PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
			break;
		      case 1:
			PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
			break;
		      case 2:
		      default:
#ifdef USE_LONG_LONGS
			PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
#else
                        /* Fake it, hope for the best.  */
			PRINT_TYPE(DEFAULT_INT_TYPE, longvalue);
#endif
			break;
		      } /* End of switch (wide_width) */
		  } /* End of else statement */
	      } /* End of integer case */
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      {
                doublevalue = (DEFAULT_TYPE) *args++;
                syslog(LOG_USER | LOG_DEBUG,
                  "doublevalue: %Lf (%f)", doublevalue, (double)doublevalue);
		if (wide_width == 0)
		  PRINT_TYPE(double, doublevalue);
		else
		  {
#ifdef USE_LONG_LONGS
		    PRINT_TYPE(DEFAULT_TYPE, doublevalue);
#else
                    /* Fake it, hope for the best.  */
		    PRINT_TYPE(DEFAULT_TYPE, doublevalue);
#endif
		  }
	      }
	      break;
	    case 's':
              longvalue = (DEFAULT_INT_TYPE) *args++;
	      PRINT_TYPE(char *, longvalue);
	      break;
	    case 'p':
              longvalue = (DEFAULT_INT_TYPE) *args++;
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

#ifdef TEST

#include <math.h>
#ifndef M_PI
#define M_PI (3.1415926535897932385)
#endif

#define RESULTFORMAT "printed %d characters\n"
#define RESULT(x) do \
{ \
    int i = (x); \
    if (strstr(#x, "checkit") != NULL) \
      checkit(RESULTFORMAT, CAST_ARGS{i}); \
    else printf(RESULTFORMAT, i); \
    fflush(stdin); \
} while (0)

int testsnprintf(int size, const char *format, ...)
/* for checking for buffer overruns from macros */
{
  int result;
  char buffer[BUFFERSIZE];
  va_list args;
  va_start(args, format);
  result = snprintf(buffer, size, format, args);
  va_end(args);
  printf("%s", buffer);
  if (buffer[strlen(buffer) - 1] != '\n') printf("\n");
  return result;
}

int precheckit(int buffersize, const char *format, DEFAULT_TYPE *args)
{
  int result;
  char formatted[BUFFERSIZE] = "";  /* can't use runtime-supplied size */
  /* allocate a safety zone in case program error causes buffer overrun */
  char safety[BUFFERSIZE];
  result = gsprintf (formatted, buffersize - 1, format, args);
  fprintf(stderr, "%s", formatted);  /* avoid double newline */
  if (formatted[strlen(formatted) - 1] != '\n') fprintf(stderr, "\n");
  return result;
}

int checkit(const char* format, DEFAULT_TYPE *args)
{
  return precheckit(BUFFERSIZE, format, args);
}

int errprintf(const char *format, ...)
{
  int result;
  va_list args;
  va_start(args, format);
  result = vfprintf(stderr, format, args);
  va_end(args);
  return result;
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

int
main (void)
{
  /* constants needed for some tests below */

  RESULT(checkit ("<%d>\n", CAST_ARGS {0x12345678}));
  RESULT(printf ("<%d>\n", 0x12345678));

  RESULT(checkit ("<%200d>\n", CAST_ARGS {5}));
  RESULT(printf ("<%200d>\n", 5));

  RESULT(checkit ("<%.300d>\n", CAST_ARGS {6}));
  RESULT(printf ("<%.300d>\n", 6));

  RESULT(checkit ("<%100.150d>\n", CAST_ARGS {7}));
  RESULT(printf ("<%100.150d>\n", 7));

  RESULT(checkit ("<%s>\n", CAST_ARGS {
    FORCE_CAST"jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
    777777777777777777333333333333366666666666622222222222777777777777733333"
  }));
  RESULT(printf ("<%s>\n",
    "jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
    777777777777777777333333333333366666666666622222222222777777777777733333"
  ));

  RESULT(checkit ("<%f><%0+#f>%s%d%s>\n", CAST_ARGS {
		  1.0, 1.0, FORCE_CAST"foo", 77,
		  FORCE_CAST"asdjffffffffffffffiiiiiiiiiiixxxxx"}));
  RESULT(printf ("<%f><%0+#f>%s%d%s>\n",
		 1.0, 1.0, "foo", 77, "asdjffffffffffffffiiiiiiiiiiixxxxx"));

  RESULT(checkit ("<%4f><%.4f><%%><%4.4f>\n",
		  CAST_ARGS {M_PI, M_PI, M_PI}));
  RESULT(printf ("<%4f><%.4f><%%><%4.4f>\n", M_PI, M_PI, M_PI));

  RESULT(checkit ("<%*f><%.*f><%%><%*.*f>\n",
		  CAST_ARGS {3, M_PI, 3, M_PI, 3, 3, M_PI}));
  RESULT(printf ("<%*f><%.*f><%%><%*.*f>\n", 3, M_PI, 3, M_PI, 3, 3, M_PI));

  RESULT(checkit ("<%d><%i><%o><%u><%x><%X><%c>\n",
		  CAST_ARGS {75, 75, 75, 75, 75, 75, 75}));
  RESULT(printf ("<%d><%i><%o><%u><%x><%X><%c>\n",
		 75, 75, 75, 75, 75, 75, 75));

  RESULT(checkit ("Testing (hd) short: <%d><%ld><%hd><%hd><%d>\n",
                  CAST_ARGS {123, (long)234, 345, 123456789, 456}));
  RESULT(printf ("Testing (hd) short: <%d><%ld><%hd><%hd><%d>\n", 123, (long)234, 345, 123456789, 456));

#ifdef USE_LONG_LONGS
  RESULT(checkit ("Testing (lld) long long: <%d><%lld><%d>\n", CAST_ARGS 
        {123, 234234234234234234LL, 345}));
  RESULT(printf ("Testing (lld) long long: <%d><%lld><%d>\n", 123, 234234234234234234LL, 345));
  RESULT(checkit ("Testing (Ld) long long: <%d><%Ld><%d>\n", CAST_ARGS
        {123, 234234234234234234LL, 345}));
  RESULT(printf ("Testing (Ld) long long: <%d><%Ld><%d>\n", 123, 234234234234234234LL, 345));

  RESULT(checkit ("Testing (Lf) long double: <%.20f><%.20Lf><%0+#.20f>\n",
		 CAST_ARGS {1.23456, 1.234567890123456789L, 1.23456}));
  RESULT(printf ("Testing (Lf) long double: <%.20f><%.20Lf><%0+#.20f>\n",
		 1.23456, 1.234567890123456789L, 1.23456));
#endif  /* USE_LONG_LONGS */

  /* now let's test buffer overruns for the various macros used */
  /* first, PRINT_CHAR */
  RESULT(precheckit(SHORTBUFFERSIZE, "abcdefghijklmn", NULL));
  RESULT(testsnprintf(SHORTBUFFERSIZE, "abcdefghijklmn"));

  return 0;
}
#endif /* TEST */
/* vim: tabstop=8 shiftwidth=2 expandtab softtabstop=2
 */
