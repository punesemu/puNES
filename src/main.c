/*
 * main.c
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#include <string.h>
#include "main.h"
#include "emu.h"
#include "mem_map.h"
#include "gfx.h"
#include "text.h"
#include "cfg_file.h"
#include "cmd_line.h"
#include "timeline.h"
#include "version.h"
#define __GUI_BASE__
#include "gui.h"
#undef __GUI_BASE__
#include "gamegenie.h"
#include "recent_roms.h"
#include "uncompress.h"

#if defined MINGW32 || defined MINGW64
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	int argc = __argc;
	char **argv = (char **)__argv;

	gui.main_hinstance = hInstance;
#else
int main(int argc, char **argv) {
#endif
	BYTE optind;

	memset(&info, 0x00, sizeof(info));
	info.machine[HEADER] = info.machine[DATABASE] = DEFAULT;

	if (cmd_line_check_portable(argc, argv) == TRUE) {
		info.portable = TRUE;
	} else {
		info.portable = FALSE;
	}

	gui_init(argc, argv);

	/* controllo l'esistenza della directory principale */
	if (emu_make_dir(info.base_folder)) {
		fprintf(stderr, "error on create puNES folder\n");
		return (EXIT_ERROR);
	}
	/* creo le sottocartelle */
	if (emu_make_dir("%s" SAVE_FOLDER, info.base_folder)) {
		fprintf(stderr, "error on create save folder\n");
		return (EXIT_ERROR);
	}
	if (emu_make_dir("%s" PERGAME_FOLDER, info.base_folder)) {
		fprintf(stderr, "error on create psg folder\n");
		return (EXIT_ERROR);
	}
	if (emu_make_dir("%s" BIOS_FOLDER, info.base_folder)) {
		fprintf(stderr, "error on create bios folder\n");
		return (EXIT_ERROR);
	}
	if (emu_make_dir("%s" DIFF_FOLDER, info.base_folder)) {
		fprintf(stderr, "error on create diff folder\n");
		return (EXIT_ERROR);
	}
	if (emu_make_dir("%s" TMP_FOLDER, info.base_folder)) {
		fprintf(stderr, "error on create tmp folder\n");
		return (EXIT_ERROR);
	}

#ifdef __NETPLAY__
	netplay_init();
#endif

	gamegenie_init();

	text_init();

	if (!info.portable) {
		text_add_line_info(1, "[yellow]p[red]u[green]N[cyan]E[brown]S[normal]"
		" [font8](by [cyan]FHorse[normal]) [font12]%s", VERSION);
	} else {
		text_add_line_info(1, "[font8][cyan]Portable[normal] "
		"[font12][yellow]p[red]u[green]N[cyan]E[brown]S[normal]"
		"[font8] (by [cyan]FHorse[normal]) [font12]%s", VERSION);
	}

	/*
	 * tratto il file di configurazione ed
	 * i parametri passati dalla riga di comando.
	 */
	cfg_file_init();
	cfg_file_parse();
	optind = cmd_line_parse(argc, argv);

	if (argc == optind) {
#ifndef DEBUG
		//if(!info.gui) {
		//	strcpy(info.rom_file, "rom.nes");
		//}
#else
		//strcpy(info.rom_file, "/home/fhorse/sviluppo/personale/roms/85/Lagrange Point (J).nes");
#endif
	} else {
		strcpy(info.rom_file, argv[optind]);
	}

	fprintf(stderr, "INFO: path %s\n", info.base_folder);

	recent_roms_init();
	recent_roms_parse();

	uncomp_init();

	if (emu_turn_on()) {
		emu_quit(EXIT_FAILURE);
	}

	gui_start();

	emu_quit(EXIT_SUCCESS);

	return (EXIT_SUCCESS);
}
