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

#ifndef CHEAT_H_
#define CHEAT_H_

#include <stdio.h>
#include "common.h"
#include "mappers/mapper_GameGenie.h"

#define GG_CHEATS 3
#define CL_CHEATS 100

enum cheat_modes { NOCHEAT_MODE, GAMEGENIE_MODE, CHEATSLIST_MODE };
enum GG_DATA {
	GG_ADDRESS_HIGH,
	GG_ADDRESS_LOW,
	GG_COMPARE,
	GG_REPLACE
};
enum GG_PHASE {
	GG_LOAD_ROM = 1,
	GG_EXECUTE,
	GG_FINISH,
	GG_LOAD_GAMEGENIE
};

typedef struct _cheat {
	BYTE disabled;
	BYTE enabled_compare;
	WORD address;
	BYTE replace;
	BYTE compare;
} _cheat;
typedef struct _type_cheat {
	int counter;
	_cheat cheat[CL_CHEATS];
} _type_cheat;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _gamegenie {
	BYTE phase;
	BYTE rom_present;
	BYTE value;
	BYTE counter;
	_cheat cheat[GG_CHEATS];
} gamegenie;
EXTERNC struct _cheats_list {
	_type_cheat rom;
	_type_cheat ram;
} cheats_list;

EXTERNC void gamegenie_init(void);
EXTERNC void gamegenie_reset(void);
EXTERNC void gamegenie_check_rom_present(BYTE print_message);
EXTERNC FILE *gamegenie_load_rom(FILE *fp);

EXTERNC void cheatslist_init(void);
EXTERNC void cheatslist_read_game_cheats(void);
EXTERNC void cheatslist_save_game_cheats(void);
EXTERNC void cheatslist_blank(void);
EXTERNC void cheatslist_quit(void);

#undef EXTERNC

#endif /* CHEAT_H_ */
