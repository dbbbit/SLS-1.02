/*
 * xdm - display manager daemon
 *
 * $XConsortium: dpylist.c,v 1.17 89/12/06 19:38:23 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * a simple linked list of known displays
 */

# include "dm.h"

struct display	*displays;

extern void	free ();

AnyDisplaysLeft ()
{
	return displays != (struct display *) 0;
}

ForEachDisplay (f)
	void	(*f)();
{
	struct display	*d, *next;

	for (d = displays; d; d = next) {
		next = d->next;
		(*f) (d);
	}
}

struct display *
FindDisplayByName (name)
char	*name;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (!strcmp (name, d->name))
			return d;
	return 0;
}

struct display *
FindDisplayByPid (pid)
int	pid;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (pid == d->pid)
			return d;
	return 0;
}

struct display *
FindDisplayByServerPid (serverPid)
int	serverPid;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (serverPid == d->serverPid)
			return d;
	return 0;
}

struct display *
FindDisplayBySessionID (sessionID)
    CARD32  sessionID;
{
    struct display	*d;

    for (d = displays; d; d = d->next)
	if (sessionID == d->sessionID)
	    return d;
    return 0;
}

struct display *
FindDisplayByAddress (addr, addrlen, displayNumber)
    struct sockaddr *addr;
    int		    addrlen;
    CARD16	    displayNumber;
{
    struct display  *d;

    for (d = displays; d; d = d->next)
	if (d->displayType.origin == FromXDMCP &&
	    d->displayNumber == displayNumber &&
	    addressEqual (d->from, d->fromlen, addr, addrlen))
	{
	    return d;
	}
    return 0;
}

#define IfFree(x)  if (x) free ((char *) x)
    
RemoveDisplay (old)
struct display	*old;
{
    struct display	*d, *p;
    char		**x;

    p = 0;
    for (d = displays; d; d = d->next) {
	if (d == old) {
	    if (p)
		p->next = d->next;
	    else
		displays = d->next;
	    IfFree (d->name);
	    IfFree (d->class);
	    for (x = d->argv; x && *x; x++)
		IfFree (*x);
	    IfFree (d->argv);
	    IfFree (d->resources);
	    IfFree (d->xrdb);
	    IfFree (d->cpp);
	    IfFree (d->startup);
	    IfFree (d->reset);
	    IfFree (d->session);
	    IfFree (d->userPath);
	    IfFree (d->systemPath);
	    IfFree (d->systemShell);
	    IfFree (d->failsafeClient);
	    if (d->authorization)
		XauDisposeAuth (d->authorization);
	    if (d->authFile)
		(void) unlink (d->authFile);
	    IfFree (d->authFile);
	    IfFree (d->userAuthDir);
	    IfFree (d->authName);
	    IfFree (d->peer);
	    IfFree (d->from);
	    free ((char *) d);
	    break;
	}
	p = d;
    }
}

struct display *
NewDisplay (name, class)
char		*name;
char		*class;
{
    struct display	*d;

    d = (struct display *) malloc (sizeof (struct display));
    if (!d) {
	LogOutOfMem ("NewDisplay");
	return 0;
    }
    d->next = displays;
    d->name = malloc ((unsigned) (strlen (name) + 1));
    if (!d->name) {
	LogOutOfMem ("NewDisplay");
	free ((char *) d);
	return 0;
    }
    strcpy (d->name, name);
    if (class)
    {
	d->class = malloc ((unsigned) (strlen (class) + 1));
	if (!d->class) {
	    LogOutOfMem ("NewDisplay");
	    free (d->name);
	    free ((char *) d);
	    return 0;
	}
	strcpy (d->class, class);
    }
    else
    {
	d->class = (char *) 0;
    }
    /* initialize every field to avoid possible problems */
    d->argv = 0;
    d->status = notRunning;
    d->pid = -1;
    d->serverPid = -1;
    d->state = NewEntry;
    d->resources = NULL;
    d->xrdb = NULL;
    d->cpp = NULL;
    d->startup = NULL;
    d->reset = NULL;
    d->session = NULL;
    d->userPath = NULL;
    d->systemPath = NULL;
    d->systemShell = NULL;
    d->failsafeClient = NULL;
    d->authorize = FALSE;
    d->authorization = NULL;
    d->authFile = NULL;
    d->userAuthDir = NULL;
    d->authName = NULL;
    d->authNameLen = 0;
    d->openDelay = 0;
    d->openRepeat = 0;
    d->openTimeout = 0;
    d->startAttempts = 0;
    d->startTries = 0;
    d->terminateServer = 0;
    d->grabTimeout = 0;
    d->sessionID = 0;
    d->peer = 0;
    d->peerlen = 0;
    d->from = 0;
    d->fromlen = 0;
    d->displayNumber = 0;
    displays = d;
    return d;
}
