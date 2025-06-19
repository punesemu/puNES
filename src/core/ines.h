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

#ifndef INES_H_
#define INES_H_

#include "common.h"

enum ines_flags { FL6, FL7, FL8, FL9, FL10, FL11, FL12, FL13, FL14, FL15, TOTAL_FL };

typedef struct _ines {
	BYTE flags[TOTAL_FL];
} _ines;

extern _ines ines;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE ines_load_rom(void);

EXTERNC size_t nes20_prg_chr_size(DBWORD *reg, double divider);

#undef EXTERNC

#endif /* INES_H_ */
