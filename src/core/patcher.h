/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef PATCHER_H_
#define PATCHER_H_

#include "common.h"

typedef struct _patcher {
	uTCHAR *file;
	BYTE patched;
} _patcher;

extern _patcher patcher;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void patcher_init(void);
EXTERNC void patcher_quit(void);
EXTERNC BYTE patcher_ctrl_if_exist(uTCHAR *patch);
EXTERNC void patcher_apply(void *rom_mem);

#undef EXTERNC

#endif /* PATCHER_H_ */
