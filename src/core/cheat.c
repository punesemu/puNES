/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cheat.h"
#include "rom_mem.h"
#include "gui.h"
#include "emu.h"
#include "info.h"
#include "text.h"
#include "patcher.h"
#include "conf.h"

#define GGFILE "gamegenie.rom"

void gamegenie_init(void) {
	memset(&gamegenie, 0x00, sizeof(gamegenie));
	gamegenie_reset();
}
void gamegenie_quit(void) {
	gamegenie_free_paths();
}
void gamegenie_reset(void) {
	unsigned int i;

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
void gamegenie_free_paths(void) {
	if (gamegenie.rom) {
		free (gamegenie.rom);
		gamegenie.rom = NULL;
	}
	if (gamegenie.patch) {
		free (gamegenie.patch);
		gamegenie.patch = NULL;
	}
}
uTCHAR *gamegenie_check_rom_present(BYTE print_message) {
	static uTCHAR gg_rom_file[LENGTH_FILE_NAME_LONG], *lastSlash;

	gamegenie.rom_present = FALSE;

	// 1) file specificato dall'utente
	usnprintf(gg_rom_file, usizeof(gg_rom_file), uL("" uPERCENTs), cfg->gg_rom_file);
	if (emu_file_exist(gg_rom_file) == EXIT_OK) {
		goto gamegenie_check_rom_present_founded;
	}

	// 2) directory di lavoro
	ustrncpy(gg_rom_file, uL("" GGFILE), usizeof(gg_rom_file));
	if (emu_file_exist(gg_rom_file) == EXIT_OK) {
		goto gamegenie_check_rom_present_founded;
	}

	// 3) directory contenente la rom nes
	ustrncpy(gg_rom_file, info.rom.file, usizeof(gg_rom_file));
	// rintraccio l'ultimo '.' nel nome
#if defined (__WIN32__)
	if ((lastSlash = ustrrchr(gg_rom_file, uL('\\')))) {
		(*(lastSlash + 1)) = 0x00;
	}
#else
	if ((lastSlash = ustrrchr(gg_rom_file, uL('/')))) {
		(*(lastSlash + 1)) = 0x00;
	}
#endif
	// aggiungo il nome del file
	ustrcat(gg_rom_file, uL("" GGFILE));
	if (emu_file_exist(gg_rom_file) == EXIT_OK) {
		goto gamegenie_check_rom_present_founded;
	}

	// 4) directory puNES/bios
	usnprintf(gg_rom_file, usizeof(gg_rom_file),
		uL("" uPERCENTs BIOS_FOLDER "/" GGFILE), info.base_folder);
	if (emu_file_exist(gg_rom_file) == EXIT_OK) {
		goto gamegenie_check_rom_present_founded;
	}

	if (print_message) {
		text_add_line_info(1, "[red]Game Genie rom not found");
		fprintf(stderr, "Game Genie rom not found\n");
	}

	return (NULL);

	gamegenie_check_rom_present_founded:
	gamegenie.rom_present = TRUE;
	return (gg_rom_file);
}
void gamegenie_load_rom(void *rom_mem) {
	_rom_mem *rom = (_rom_mem *)rom_mem;
	uTCHAR *gg_rom_file;
	BYTE *gg_rom_mem;
	size_t size;
	FILE *fp;

	gg_rom_file = gamegenie_check_rom_present(FALSE);

	if ((gamegenie.phase == GG_LOAD_ROM) || !gamegenie.rom_present) {
		return;
	}

	if (info.rom.file[0] && (gamegenie.rom = emu_ustrncpy(gamegenie.rom, info.rom.file)) == NULL) {
		return;
	}

	if (patcher.file && (gamegenie.patch = emu_ustrncpy(gamegenie.patch, patcher.file)) == NULL) {
		gamegenie_free_paths();
		return;
	}

	ustrncpy(info.rom.file, gg_rom_file, usizeof(info.rom.file));

	if (!(fp = ufopen(info.rom.file, uL("rb")))) {
		text_add_line_info(1, "[red]error loading Game Genie rom");
		fprintf(stderr, "error loading Game Genie rom\n");
		ustrncpy(info.rom.file, gamegenie.rom, usizeof(info.rom.file));
		gamegenie_free_paths();
		return;
	}

	gamegenie.phase = GG_EXECUTE;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if ((gg_rom_mem = (BYTE *) malloc(size)) == NULL) {
		fclose(fp);
		ustrncpy(info.rom.file, gamegenie.rom, usizeof(info.rom.file));
		gamegenie_free_paths();
		return;
	}

	if (fread(gg_rom_mem, 1, size, fp) != size) {
		fclose(fp);
		free(gg_rom_mem);
		ustrncpy(info.rom.file, gamegenie.rom, usizeof(info.rom.file));
		gamegenie_free_paths();
		return;
	}

	fclose(fp);
	free(rom->data);

	rom->data = gg_rom_mem;
	rom->size = size;
}

void cheatslist_init(void) {
	gui_objcheat_init();
	memset(&cheats_list, 0x00, sizeof(cheats_list));
}
void cheatslist_read_game_cheats(void) {
	cheatslist_blank();
	gui_objcheat_read_game_cheats();
}
void cheatslist_save_game_cheats(void) {
	gui_objcheat_save_game_cheats();
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
