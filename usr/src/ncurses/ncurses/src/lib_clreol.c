
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
**	lib_clreol.c
**
**	The routine wclrtoeol().
**
*/

#include "curses.h"
#include "curses.priv.h"


int  wclrtoeol(WINDOW *win)
{
	chtype	*maxx, *ptr, *end;
	int	y, x, minx;
	chtype	blank = ' ' | win->_attrs;

#ifdef TRACE
	if (_tracing)
	    _tracef("wclrtoeol(%x) called", win);
#endif

	y = win->_cury;
	x = win->_curx;

	end = &win->_line[y][win->_maxx];
	minx = _NOCHANGE;
	maxx = &win->_line[y][x];

	for (ptr = maxx; ptr < end; ptr++)
	{
	    if (*ptr != blank)
	    {
		maxx = ptr;
		if (minx == _NOCHANGE)
		    minx = ptr - win->_line[y];
		*ptr = blank;
	    }
	}

	if (minx != _NOCHANGE)
	{
	    if (win->_firstchar[y] > minx || win->_firstchar[y] == _NOCHANGE)
		win->_firstchar[y] = minx;

	    if (win->_lastchar[y] < maxx - win->_line[y])
		win->_lastchar[y] = maxx - win->_line[y];
	}
}
