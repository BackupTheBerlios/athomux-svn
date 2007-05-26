/*
 * Copyright (c) 1994-1996, 1998 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

int isascii(int c)
{
	return ((c & ~0x7f) == 0);
}

int iscntrl(int c)
{
	return ((c) < ' ') || ((c) > 126);
}

int isdigit(int c)
{
	return ((c) >= '0') && ((c) <= '9');
}

int isgraph(int c)
{
	return ((c) > ' ') && ((c) <= 126);
}

int islower(int c)
{
	return (c >= 'a') && (c <= 'z');
}

int isprint(int c)
{
	return ((c) >= ' ') && ((c) <= 126);
}

int isspace(int c)
{
	return ((c) == ' ') || ((c) == '\f')
		|| ((c) == '\n') || ((c) == '\r')
		|| ((c) == '\t') || ((c) == '\v');
}

int isupper(int c)
{
	return (c >= 'A') && (c <= 'Z');
}

int isxdigit(int c)
{
	return isdigit(c) ||
		((c >= 'A') && (c <= 'F')) ||
		((c >= 'a') && (c <= 'f'));
}

int isalpha(int c)
{
	return islower(c) || isupper(c);
}

int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

int toascii(int c)
{
	return ((c) & 0x7f);
}

int toupper(int c)
{
	return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : c;
}

int tolower(int c)
{
	return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c;
}


