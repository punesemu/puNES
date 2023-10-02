/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef DIPSWITCH_HPP_
#define DIPSWITCH_HPP_

#include "common.h"

typedef struct _dipswitch {
	BYTE used;
	BYTE show_dlg;
	int value;
	int def;
} _dipswitch;

extern _dipswitch dipswitch;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void dipswitch_reset(void);
EXTERNC void dipswitch_search(void);
EXTERNC void dipswitch_update_value(void);
EXTERNC int dipswitch_type_length(void);

#undef EXTERNC

#endif /* DIPSWITCH_HPP_ */
