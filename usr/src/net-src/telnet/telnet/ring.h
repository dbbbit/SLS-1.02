/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
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
 *
 *	@(#)ring.h	5.2 (Berkeley) 3/1/91
 */

#if defined(P)
# undef P
#endif

#if defined(__STDC__) || defined(LINT_ARGS)
# define	P(x)	x
#else
# define	P(x)	()
#endif

/*
 * This defines a structure for a ring buffer.
 *
 * The circular buffer has two parts:
 *(((
 *	full:	[consume, supply)
 *	empty:	[supply, consume)
 *]]]
 *
 */
typedef struct {
    unsigned char	*consume,	/* where data comes out of */
			*supply,	/* where data comes in to */
			*bottom,	/* lowest address in buffer */
			*top,		/* highest address+1 in buffer */
			*mark;		/* marker (user defined) */
#if	defined(ENCRYPT)
    unsigned char	*clearto;	/* Data to this point is clear text */
    unsigned char	*encryyptedto;	/* Data is encrypted to here */
#endif
    int		size;		/* size in bytes of buffer */
    u_long	consumetime,	/* help us keep straight full, empty, etc. */
		supplytime;
} Ring;

/* Here are some functions and macros to deal with the ring buffer */

/* Initialization routine */
extern int
	ring_init P((Ring *ring, unsigned char *buffer, int count));

/* Data movement routines */
extern void
	ring_supply_data P((Ring *ring, unsigned char *buffer, int count));
#ifdef notdef
extern void
	ring_consume_data P((Ring *ring, unsigned char *buffer, int count));
#endif

/* Buffer state transition routines */
extern void
	ring_supplied P((Ring *ring, int count)),
	ring_consumed P((Ring *ring, int count));

/* Buffer state query routines */
extern int
	ring_empty_count P((Ring *ring)),
	ring_empty_consecutive P((Ring *ring)),
	ring_full_count P((Ring *ring)),
	ring_full_consecutive P((Ring *ring));

#if	defined(ENCRYPT)
extern void
	ring_encrypt P((Ring *ring, void (*func)())),
	ring_clearto P((Ring *ring));
#endif

extern void
    ring_clear_mark(),
    ring_mark();
