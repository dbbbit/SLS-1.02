/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fnmatch.c	5.3 (Berkeley) 6/23/90";
#endif /* LIBC_SCCS and not lint */

/*
 * Function fnmatch() as proposed in Posix 1003.2 B.6 (rev. 9).
 * Compares a filename or pathname to a pattern.
 */

#include <unistd.h>
#include <string.h>

#ifndef	FNM_PATHNAME
/* fnmatch function */
#define	FNM_PATHNAME	0x01	/* match pathnames, not filenames */
#ifndef _POSIX_SOURCE
#endif
#endif
#define	FNM_QUOTE	0x02	/* escape special chars with \ */

#define	EOS	'\0'

static char *
rangematch(pattern, test)
	register char *pattern, test;
{
	register char c, c2;
	int negate, ok;

	if (negate = (*pattern == '!'))
		++pattern;

	/*
	 * TO DO: quoting
	 */

	for (ok = 0; (c = *pattern++) != ']';) {
		if (c == EOS)
			return(NULL);		/* illegal pattern */
		if (*pattern == '-' && (c2 = pattern[1]) != EOS && c2 != ']') {
			if (c <= test && test <= c2)
				ok = 1;
			pattern += 2;
		}
		else if (c == test)
			ok = 1;
	}
	return(ok == negate ? NULL : pattern);
}

fnmatch(pattern, string, flags)
	register char *pattern, *string;
	int flags;
{
	register char c;
	char test, *rangematch();

	for (;;)
		switch (c = *pattern++) {
		case EOS:
			return(*string == EOS);
		case '?':
			if ((test = *string++) == EOS ||
			    test == '/' && flags & FNM_PATHNAME)
				return(0);
			break;
		case '*':
			c = *pattern;
			/* collapse multiple stars */
			while (c == '*')
				c = *++pattern;

			/* optimize for pattern with * at end or before / */
			if (c == EOS)
				if (flags & FNM_PATHNAME)
					return(!index(string, '/'));
				else
					return(1);
			else if (c == '/' && flags & FNM_PATHNAME) {
				if ((string = index(string, '/')) == (char *) NULL)
					return(0);
				break;
			}

			/* general case, use recursion */
			while ((test = *string) != EOS) {
				if (fnmatch(pattern, string, flags))
					return(1);
				if (test == '/' && flags & FNM_PATHNAME)
					break;
				++string;
			}
			return(0);
		case '[':
			if ((test = *string++) == EOS ||
			    test == '/' && flags & FNM_PATHNAME)
				return(0);
			if ((pattern = rangematch(pattern, test)) == NULL)
				return(0);
			break;
		case '\\':
			if (flags & FNM_QUOTE) {
				if ((c = *pattern++) == EOS) {
					c = '\\';
					--pattern;
				}
				if (c != *string++)
					return(0);
				break;
			}
			/* FALLTHROUGH */
		default:
			if (c != *string++)
				return(0);
			break;
		}
}
