/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef WIN_H_
#define WIN_H_

#if !defined (_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif

#if defined (__cplusplus)
#undef NULL
#endif
#define INITGUID
#include <windows.h>
#undef INITGUID

#define	WIN_TEN     64
#define	WIN_EIGHTP1 63
#define	WIN_EIGHT   62
#define	WIN_SEVEN   61
#define	WIN_VISTA   60
#define	WIN_XP      51

#define exit_thread(value) return

#endif /* WIN_H_ */
