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

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cheat.h"
#include "gui.h"
#include "emu.h"
#include "info.h"
#include "text.h"

#define GGFILE "gamegenie.rom"

void gamegenie_init(void) {
	gamegenie.rom_present = FALSE;
	gamegenie_reset();
}
void gamegenie_reset(void) {
	BYTE i;

	gamegenie.counter = 0;
	gamegenie.phase = GG_LOAD_GAMEGENIE;
	gamegenie.value = 0x71;

	for (i = 0; i < LENGTH(gamegenie.cheat); i++) {
		_cheat *ch = &gamegenie.cheat[i];

		ch->disabled = TRUE;
		ch->address = 0xFFFF;
		ch->compare = 0xFF;
		ch->replace = 0xFF;
	}
}
void gamegenie_check_rom_present(BYTE print_message) {
	uTCHAR gg_rom[LENGTH_FILE_NAME_LONG];

	usnprintf(gg_rom, usizeof(gg_rom), uL("" uPERCENTs BIOS_FOLDER "/" GGFILE), info.base_folder);

	gamegenie.rom_present = FALSE;

	if (emu_file_exist(gg_rom) == EXIT_OK) {
		gamegenie.rom_present = TRUE;
	}

	if (print_message && gamegenie.rom_present == FALSE) {
		text_add_line_info(1, "[red]'bios/gamegenie.rom' not found");
		fprintf(stderr, "Game Genie rom 'bios/gamegenie.rom' not found\n");
	}
}
FILE *gamegenie_load_rom(FILE *fp) {
	FILE *fp_gg, *fp_rom = fp;

	gamegenie_check_rom_present(FALSE);

	if ((gamegenie.phase == GG_LOAD_ROM) || !gamegenie.rom_present) {
		return (fp_rom);
	}

	ustrncpy(info.load_rom_file, info.rom_file, usizeof(info.load_rom_file));
	usnprintf(info.rom_file, usizeof(info.rom_file), uL("" uPERCENTs BIOS_FOLDER "/" GGFILE),
			info.base_folder);

	if (!(fp_gg = ufopen(info.rom_file, uL("rb")))) {
		text_add_line_info(1, "[red]error loading Game Genie rom");
		fprintf(stderr, "error loading Game Genie rom\n");

		ustrncpy(info.rom_file, info.load_rom_file, usizeof(info.rom_file));

		umemset(info.load_rom_file, 0, usizeof(info.load_rom_file));
		return (fp_rom);
	}

	fclose(fp_rom);

	gamegenie.phase = GG_EXECUTE;

	return (fp_gg);
}

void cheatslist_init(void) {
	gui_cheat_init();
	memset (&cheats_list, 0x00, sizeof(cheats_list));
}
void cheatslist_read_game_cheats(void) {
	cheatslist_blank();
	gui_cheat_read_game_cheats();
}
void cheatslist_save_game_cheats(void) {
	gui_cheat_save_game_cheats();
}
void cheatslist_blank(void) {
	if (cheats_list.rom.counter > 0) {
		memset(&cheats_list.rom, 0x00, sizeof(_type_cheat));
	}
	if (cheats_list.ram.counter > 0) {
		memset(&cheats_list.ram, 0x00, sizeof(_type_cheat));
	}
}
void cheatslist_quit(void) {
	cheatslist_save_game_cheats();
	cheatslist_blank();
}
