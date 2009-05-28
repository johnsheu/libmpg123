/*
	icy: support for SHOUTcast ICY meta info, an attempt to keep it organized

	copyright 2006-7 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis and modelled after patch by Honza
*/
#ifndef MPG123_ICY_H
#define MPG123_ICY_H

#ifndef NO_ICY

#include "compat.h"
#include "mpg123.h"

struct icy_meta
{
	TCHAR* data;
	off_t interval;
	off_t next;
};

void init_icy(struct icy_meta *);
void clear_icy(struct icy_meta *);
void reset_icy(struct icy_meta *);

#else

#define init_icy(a)
#define clear_icy(a)
#define reset_icy(a)

#endif /* NO_ICY */

#endif
