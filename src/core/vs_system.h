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

#ifndef VS_SYSTEM_H_
#define VS_SYSTEM_H_

enum vs_system_ppu {
	RP2C03B,     // (bog standard RGB palette)
	RP2C03G,     // (similar pallete to above, might have 1 changed colour)
	RP2C04,      // 0001 (scrambled palette + new colours)
	RP2C04_0002, // (same as above, different scrambling)
	RP2C04_0003, // (similar to above)
	RP2C04_0004, // (similar to above)
	RC2C03B,     // (bog standard palette, seems identical to RP2C03B)
	RC2C03C,     // (similar to above, but with 1 changed colour or so)
	RC2C05_01,   // (all five of these have the normal palette...
	RC2C05_02,   // (...but with different bits returned on 2002)
	RC2C05_03,
	RC2C05_04,
	RC2C05_05
};
enum vs_system_special_mode {
	VS_SM_Normal,        // no special mode(s)
	VS_SM_RBI_Baseball,  // (protection hardware at port 5E0xh)
	VS_SM_TKO_Boxing,    // (other protection hardware at port 5E0xh)
	VS_SM_Super_Xevious, // (protection hardware at port 5xxxh)
};

#define VSONTIME 28000
#define VSOFFTIME 38000

#define vs_system_wd_next() (machine.cpu_cycles_frame * machine.fps) + \
	(((machine.cpu_cycles_frame * machine.fps) / 1000.0) * (emu_irand(100) + 214));
#define vs_system_cn_next() ((machine.cpu_cycles_frame * machine.fps) / 1000.0) * \
	(emu_irand(28) + 50);
#define vs_system_r4020_clock(type, val)\
	if (vs_system.r4020.type.actual.value != (val & 0x01)) {\
		vs_system.r4020.type.old.value = vs_system.r4020.type.actual.value;\
		vs_system.r4020.type.old.timer = vs_system.r4020.type.actual.timer;\
		vs_system.r4020.type.actual.value = val & 0x01;\
		vs_system.r4020.type.actual.timer = 0;\
	}
#define vs_system_r4020_timer(type)\
	++vs_system.r4020.type.actual.timer;\
	if (vs_system.r4020.type.old.value && !vs_system.r4020.type.actual.value) {\
		if ((vs_system.r4020.type.old.timer > VSONTIME) && \
			(vs_system.r4020.type.actual.timer > VSOFFTIME)) {\
			vs_system.coins.counter++;\
			gui_vs_system_update_dialog();\
			vs_system.r4020.type.old.value = 0;\
			vs_system.r4020.type.old.timer = 0;\
		}\
	}

typedef struct _r4020_base {
	DBWORD timer;
	BYTE value;
} _r4020_base;
typedef struct _r4020_type {
	_r4020_base old;
	_r4020_base actual;
} _r4020_type;
typedef struct _vs_system {
	BYTE enabled;
	BYTE ppu;
	BYTE shared_mem;
	struct _special_mode {
		BYTE type;
		BYTE *r5e0x;
		BYTE index;
	} special_mode;
	struct _rc2c05 {
		BYTE enabled;
		BYTE r2002;
	} rc2c05;
	struct _r4020 {
		_r4020_type rd;
		_r4020_type wr;
	} r4020;
	struct _watchdog {
		DBWORD timer;
		DBWORD next;
		BYTE reset;
	} watchdog;
	struct _coins {
		DBWORD counter;
		DBWORD service;
		DBWORD left;
		DBWORD right;
	} coins;
} _vs_system;

static const BYTE vs_protection_data[2][32] = {
	{
		0xFF, 0xBF, 0xB7, 0x97,
		0x97, 0x17, 0x57, 0x4F,
		0x6F, 0x6B, 0xEB, 0xA9,
		0xB1, 0x90, 0x94, 0x14,
		0x56, 0x4E, 0x6F, 0x6B,
		0xEB, 0xA9, 0xB1, 0x90,
		0xD4, 0x5C, 0x3E, 0x26,
		0x87, 0x83, 0x13, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x00,
		0xB4, 0x00, 0x00, 0x00,
		0x00, 0x6F, 0x00, 0x00,
		0x00, 0x00, 0x94, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	}
};

extern _vs_system vs_system;

#endif /* VS_SYSTEM_H_ */
