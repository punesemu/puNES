/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#ifndef TIMELINE_H_
#define TIMELINE_H_

#include "common.h"

enum timeline_misc {
	TL_NORMAL,
	TL_SAVE_SLOT,
	TL_SNAP_SEC = 5,
	TL_SNAPS_TOT = 14,
	TL_SNAP_FREE = TL_SNAPS_TOT - 1,
	TL_SNAPS = TL_SNAPS_TOT - 1
};

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _timeline {
	BYTE *start;
	BYTE *snaps[TL_SNAPS_TOT];
	SWORD snap;
	SWORD snaps_fill;
	DBWORD snap_size;
	DBWORD preview;
	DBWORD frames;
	DBWORD frames_snap;
	BYTE update;
	BYTE button;
	BYTE key;
} tl;

EXTERNC BYTE timeline_init(void);
EXTERNC void timeline_snap(BYTE mode);
EXTERNC void timeline_preview(BYTE snap);
EXTERNC void timeline_back(BYTE mode, BYTE snap);
EXTERNC void timeline_quit(void);

#undef EXTERNC

#endif /* TIMELINE_H_ */
