/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef PIC16C5X_H_
#define PIC16C5X_H_

#include <stdio.h>
#include "common.h"

typedef	uint8_t (*pic16c5x_rd_funct)(int);
typedef	void (*pic16c5x_wr_funct)(int, int);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void pic16c5x_init(BYTE *rom, pic16c5x_rd_funct rd, pic16c5x_wr_funct wr);
EXTERNC void pic16c5x_quit(void);
EXTERNC void pic16c5x_reset(BYTE type);
EXTERNC void pic16c5x_run(void);
EXTERNC BYTE pic16c5x_save_mapper(BYTE mode, BYTE slot, FILE *fp);

#undef EXTERNC

#endif /* PIC16C5X_H_ */
