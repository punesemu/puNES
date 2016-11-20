/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef EMU_H_
#define EMU_H_

#include <stdio.h>
#include "common.h"

#define emu_irand(x) ((unsigned int)((x) * emu_drand()))

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE emu_frame(void);
EXTERNC BYTE emu_make_dir(const uTCHAR *fmt, ...);
EXTERNC BYTE emu_file_exist(const uTCHAR *file);
EXTERNC char *emu_file2string(const uTCHAR *path);
EXTERNC BYTE emu_load_rom(void);
EXTERNC BYTE emu_search_in_database(FILE *fp);
EXTERNC void emu_set_title(uTCHAR *title, int len);
EXTERNC BYTE emu_turn_on(void);
EXTERNC void emu_pause(BYTE mode);
EXTERNC BYTE emu_reset(BYTE type);
EXTERNC WORD emu_round_WORD(WORD number, WORD round);
EXTERNC int emu_power_of_two(int base);
EXTERNC double emu_drand(void);
EXTERNC void emu_quit(BYTE exit_code);

#undef EXTERNC

#endif /* EMU_H_ */

