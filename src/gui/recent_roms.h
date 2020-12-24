/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef RECENT_ROMS_H_
#define RECENT_ROMS_H_

#include "common.h"

enum recent_roms_misc {
	RECENT_ROMS_MAX = 15,
};

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void recent_roms_init(void);
EXTERNC void recent_roms_add(uTCHAR *file);
EXTERNC void recent_roms_parse(void);
EXTERNC void recent_roms_save(void);
EXTERNC int recent_roms_count(void);
EXTERNC const char *recent_roms_item(int index);
EXTERNC int recent_roms_item_size(int index);
EXTERNC const char *recent_roms_current(void);
EXTERNC int recent_roms_current_size(void);

#undef EXTERNC

#endif /* RECENT_ROMS_H_ */
