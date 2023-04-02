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

int errprintf(const char *format, ...);
char * memdump(char *buffer, void *location, int count);

#define ISDIGIT(c) (c <= '9' && c >= '0')

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
	 *(formatted + total_printed++) = *ptr++; \
     } while (0)

#define PRINT_TYPE(TYPE) \
  do { \
	int result; TYPE value; \
        if (strstr(#TYPE, "double") != NULL) value = *(TYPE *)*args++; \
        else value = *(TYPE *)args++; \
	*sptr++ = *ptr++; /* Copy the type specifier.  */ \
	*sptr = '\0'; /* NULL terminate sptr.  */ \
	result = snprintf(formatted + total_printed, \
	  maxlength - total_printed, specifier, value); \
	if (result == -1) \
	  return -1; \
	else \
	  { \
	    total_printed += result; \
	    continue; \
	  } \
      } while (0)

int gsprintf (char *formatted, size_t maxlength, const char *format,
              void **args)
{
  const char * ptr = format;
  char specifier[128];
  int total_printed = 0;
  
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
		/* Short values are promoted to int, so just copy it
                   as an int and trust the C library printf to cast it
                   to the right width.  */
		if (short_width)
		  PRINT_TYPE(int);
		else
		  {
		    switch (wide_width)
		      {
		      case 0:
			PRINT_TYPE(int);
			break;
		      case 1:
			PRINT_TYPE(long);
			break;
		      case 2:
		      default:
#if defined(__GNUC__) || defined(HAVE_LONG_LONG)
			PRINT_TYPE(long long);
#else
			PRINT_TYPE(long); /* Fake it, hope for the best.  */
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
		if (wide_width == 0)
		  PRINT_TYPE(double);
		else
		  {
#if defined(__GNUC__) || defined(HAVE_LONG_DOUBLE)
		    PRINT_TYPE(long double);
#else
		    PRINT_TYPE(double); /* Hope for the best.  */
#endif
		  }
	      }
	      break;
	    case 's':
	      PRINT_TYPE(char *);
	      break;
	    case 'p':
	      PRINT_TYPE(void *);
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

#define RESULT(x, ...) do \
{ \
    int i = x __VA_ARGS__; const char * format = "printed %d characters\n"; \
    if (strcmp("checkit", #x) == 0) x (format, (void * []){(void *)(long)i}); \
    else x (format, i); \
    fflush(stdin); \
} while (0)

static int checkit (const char * format, void **args);

static int
checkit (const char* format, void **args)
{
  int result;
  char formatted[1024] = "";
  result = gsprintf (formatted, 1024, format, args);
  printf("%s", formatted);  /* avoid double newline */
  return result;
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
  unsigned char uchar, *ptr, *saved = buffer;
  int i;
  buffer += sprintf(buffer, "%p: ", location);
  buffer--; /* so we can increment at start of each hex digit */
  ptr = location;
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
  const double PI = M_PI;
  unsigned char *pi = (unsigned char *)&PI;
  const double ONE = 1.0;
  unsigned char *one = (unsigned char *)&ONE;
  const double SEQ_SHORT = 1.23456;
  unsigned char *seq_short = (unsigned char *)&SEQ_SHORT;
  const long double SEQ_LONG = 1.234567890123456789L;
  unsigned char *seq_long = (unsigned char *)&SEQ_LONG;

  RESULT(checkit, ("<%d>\n", (void * []) {0x12345678}));
  RESULT(errprintf, ("<%d>\n", 0x12345678));

  RESULT(checkit, ("<%200d>\n", (void * []) {5}));
  RESULT(errprintf, ("<%200d>\n", 5));

  RESULT(checkit, ("<%.300d>\n", (void * []) {6}));
  RESULT(errprintf, ("<%.300d>\n", 6));

  RESULT(checkit, ("<%100.150d>\n", (void * []) {7}));
  RESULT(errprintf, ("<%100.150d>\n", 7));

  RESULT(checkit, ("<%s>\n", (void * [])
		  {(void *)
		  "jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
777777777777777777333333333333366666666666622222222222777777777777733333"}));
  RESULT(errprintf, ("<%s>\n",
		 "jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
777777777777777777333333333333366666666666622222222222777777777777733333"));

  RESULT(checkit, ("<%f><%0+#f>%s%d%s>\n", (void * []) {
		  one, one, (void *) "foo", 77,
		  (void *) "asdjffffffffffffffiiiiiiiiiiixxxxx"}));
  RESULT(errprintf, ("<%f><%0+#f>%s%d%s>\n",
		 1.0, 1.0, "foo", 77, "asdjffffffffffffffiiiiiiiiiiixxxxx"));

  RESULT(checkit, ("<%4f><%.4f><%%><%4.4f>\n",
		  (void * []) {pi, pi, pi}));
  RESULT(errprintf, ("<%4f><%.4f><%%><%4.4f>\n", M_PI, M_PI, M_PI));

  RESULT(checkit, ("<%*f><%.*f><%%><%*.*f>\n",
		  (void * []) {3, pi, 3, pi, 3, 3, pi}));
  RESULT(errprintf, ("<%*f><%.*f><%%><%*.*f>\n", 3, M_PI, 3, M_PI, 3, 3, M_PI));

  RESULT(checkit, ("<%d><%i><%o><%u><%x><%X><%c>\n",
		  (void * []) {75, 75, 75, 75, 75, 75, 75}));
  RESULT(errprintf, ("<%d><%i><%o><%u><%x><%X><%c>\n",
		 75, 75, 75, 75, 75, 75, 75));

  RESULT(checkit, ("<%d><%i><%o><%u><%x><%X><%c>\n",
		  (void * []) {75, 75, 75, 75, 75, 75, 75}));
  RESULT(errprintf, ("<%d><%i><%o><%u><%x><%X><%c>\n",
		 75, 75, 75, 75, 75, 75, 75));

  RESULT(checkit, ("Testing (hd) short: <%d><%ld><%hd><%hd><%d>\n",
                  (void * []) {123, (long)234, 345, 123456789, 456}));
  RESULT(errprintf, ("Testing (hd) short: <%d><%ld><%hd><%hd><%d>\n", 123, (long)234, 345, 123456789, 456));

#if defined(__GNUC__) || defined (HAVE_LONG_LONG)
  RESULT(checkit, ("Testing (lld) long long: <%d><%lld><%d>\n", (void * [])
        {123, 234234234234234234LL, 345}));
  RESULT(errprintf, ("Testing (lld) long long: <%d><%lld><%d>\n", 123, 234234234234234234LL, 345));
  RESULT(checkit, ("Testing (Ld) long long: <%d><%Ld><%d>\n", (void * [])
        {123, 234234234234234234LL, 345}));
  RESULT(errprintf, ("Testing (Ld) long long: <%d><%Ld><%d>\n", 123, 234234234234234234LL, 345));
#endif

#if defined(__GNUC__) || defined (HAVE_LONG_DOUBLE)
  RESULT(checkit, ("Testing (Lf) long double: <%.20f><%.20Lf><%0+#.20f>\n",
		 (void * []) {seq_short, seq_long, seq_short}));
  RESULT(errprintf, ("Testing (Lf) long double: <%.20f><%.20Lf><%0+#.20f>\n",
		 1.23456, 1.234567890123456789L, 1.23456));
#endif
#ifdef HIDE_FOR_NOW
#endif  /* HIDE_FOR_NOW */

  return 0;
}
#endif /* TEST */
/* vim: tabstop=8 shiftwidth=2 expandtab softtabstop=2
 */
