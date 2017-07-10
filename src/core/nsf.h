/*  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#ifndef NSF_H_
#define NSF_H_

#include "common.h"

#define nsf_prg_rom_rd(address) nsf.prg.rom_4k[(address >> 12) & 0x07][address & 0x0FFF]
#define nsf_prg_rom_rd_6xxx(address) nsf.prg.rom_4k_6xxx[(address >> 12) & 0x01][address & 0x0FFF]

enum nsf_states {
	NSF_STOP = 0x01,
	NSF_PLAY = 0x02,
	NSF_PAUSE = 0x04,
	NSF_CHANGE_SONG = 0x10,
	NSF_NEXT = 0x100,
	NSF_PREV = 0x200,
};
enum nsf_routine_bytes {
	NSF_R_SONG = 1,
	NSF_R_TYPE = 3,
	NSF_R_INIT_LO = 5,
	NSF_R_INIT_HI = 6,
	NSF_R_PLAY_LO = 12,
	NSF_R_PLAY_HI = 13,
	NSF_R_JMP_PLAY = 15,
	NSF_R_PLAY = 0x0B,
	NSF_R_LOOP = 0x0E,
	NSF_R_PLAY_INST = 0x250B,
	NSF_R_START = 0x2500,
	NSF_R_END = 0x2510,
	NSF_R_MASK = 0x001F,
};

struct _nsf {
	BYTE enabled;
	BYTE version;
	BYTE type;
	BYTE state;
	BYTE made_tick;
	DBWORD frames;

	struct _nsf_rate {
		DBWORD count;
		DBWORD reload;
	} rate;
	struct _nsf_songs {
		BYTE total;
		BYTE starting;
		BYTE current;

		BYTE started;
	} song;
	struct _nsf_address {
		WORD load;
		WORD init;
		WORD play;
	} adr;
	struct _nsf_info {
		char name[32];
		char artist[32];
		char copyright[32];
	} info;
	struct _nsf_play_speed {
		WORD ntsc;
		WORD pal;
	} play_speed;
	struct _nsf_bankswitch {
		BYTE enabled;
		BYTE banks[8];
	} bankswitch;
	struct _nsf_extra_sound_chips {
		BYTE vrc6;
		BYTE vrc7;
		BYTE fds;
		BYTE mmc5;
		BYTE namco163;
		BYTE sunsoft5b;
	} sound_chips;
	struct _nsf_prg {
		WORD banks_4k;
		BYTE *rom_4k[8];
		BYTE *rom_4k_6xxx[2];
	} prg;
	struct _nsf_routine {
		BYTE prg[17];
		BYTE INT_NMI;
		BYTE INT_RESET;
	} routine;
	struct _nsf_timers {
		double buttons[24];
		double total_rom;
		double song;
		double last_tick;
		double silence;
		double last_silence;
	} timers;
} nsf;

void nsf_init(void);
void nsf_quit(void);
BYTE nsf_load_rom(void);
void nsf_init_tune(void);
void nsf_tick(WORD cycles_cpu);
void nsf_reset_prg(void);
void nsf_reset_timers(void);

void nsf_main_screen(void);
void nsf_main_screen_event(void);

int nsf_controls_mouse_in_buttons(int x_mouse, int y_mouse);

void nsf_effect(void);

#endif /* NSF_H_ */
