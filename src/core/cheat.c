/*
 * cheat.c
 *
 *  Created on: 09/mar/2015
 *      Author: fhorse
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
	char gg_rom[LENGTH_FILE_NAME_MID];

	sprintf(gg_rom, "%s" BIOS_FOLDER "/%s", info.base_folder, GGFILE);

	gamegenie.rom_present = FALSE;

	if (emu_file_exist(gg_rom) == EXIT_OK) {
		gamegenie.rom_present = TRUE;
	}

	if (print_message && gamegenie.rom_present == FALSE) {
		text_add_line_info(1, "[red]Game Genie rom not found");
		fprintf(stderr, "Game Genie rom not found\n");
	}
}
FILE *gamegenie_load_rom(FILE *fp) {
	FILE *fp_gg, *fp_rom = fp;

	gamegenie_check_rom_present(FALSE);

	if ((gamegenie.phase == GG_LOAD_ROM) || !gamegenie.rom_present) {
		return (fp_rom);
	}

	strncpy(info.load_rom_file, info.rom_file, sizeof(info.load_rom_file));

	sprintf(info.rom_file, "%s" BIOS_FOLDER "/%s", info.base_folder, GGFILE);

	if (!(fp_gg = fopen(info.rom_file, "rb"))) {
		text_add_line_info(1, "[red]error loading Game Genie rom");
		fprintf(stderr, "error loading Game Genie rom\n");

		strncpy(info.rom_file, info.load_rom_file, sizeof(info.rom_file));

		memset(info.load_rom_file, 0, sizeof(info.load_rom_file));
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
void cheatslist_read(void) {
	cheatslist_quit();
	gui_cheat_read_game_cheats();
}
void cheatslist_blank(void) {
	if (cheats_list.gg.counter > 0) {
		memset(&cheats_list.gg, 0x00, sizeof(_type_cheat));
	}
	if (cheats_list.ram.counter > 0) {
		memset(&cheats_list.ram, 0x00, sizeof(_type_cheat));
	}
}
void cheatslist_quit(void) {
	gui_cheat_quit();
	cheatslist_blank();
}
