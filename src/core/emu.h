/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include "common.h"

#define emu_irand(x) ((unsigned int)((x) * emu_drand()))

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void emu_quit(void);
EXTERNC void emu_frame(void);
EXTERNC void emu_frame_debugger(void);
EXTERNC BYTE emu_make_dir(const uTCHAR *fmt, ...);
EXTERNC BYTE emu_file_exist(const uTCHAR *file);
EXTERNC char *emu_file2string(const uTCHAR *path);
EXTERNC BYTE emu_load_rom(void);
EXTERNC void emu_set_title(uTCHAR *title, int len);
EXTERNC BYTE emu_turn_on(void);
EXTERNC void emu_pause(BYTE mode);
EXTERNC BYTE emu_reset(BYTE type);
EXTERNC WORD emu_round_WORD(WORD number, WORD round);
EXTERNC unsigned int emu_power_of_two(unsigned int base);
EXTERNC int emu_strcicmp(char const *a, char const *b);
EXTERNC double emu_drand(void);
EXTERNC uTCHAR *emu_ustrncpy(uTCHAR *dst, uTCHAR *src);
EXTERNC uTCHAR *emu_rand_str(void);
EXTERNC void emu_ctrl_doublebuffer(void);
EXTERNC void emu_frame_input_and_rewind(void);
EXTERNC void emu_info_rom(void);
EXTERNC void emu_initial_ram(BYTE *ram, unsigned int length);
EXTERNC void emu_save_header_info(void);
EXTERNC BYTE emu_active_nidx(void);
EXTERNC void emu_invert_bytes(BYTE *b0, BYTE *b1);
EXTERNC double emu_ms_to_cpu_cycles(double ms);
EXTERNC uTCHAR *emu_ctrl_rom_ext(uTCHAR *file);

#undef EXTERNC

#endif /* EMU_H_ */

