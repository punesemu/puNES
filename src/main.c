/*
 * main.c
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#include <string.h>
#include "main.h"
#include "emu.h"
#include "memmap.h"
#include "sdlgfx.h"
#include "sdltext.h"
#include "cfg_file.h"
#include "cmd_line.h"
#include "timeline.h"
#include "version.h"
#include "gui.h"

#include "gamegenie.h"

#if defined MINGW32 || defined MINGW64
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		PSTR szCmdLine, int iCmdShow) {
	int argc = __argc;
	char **argv = (char **)__argv;
#if !defined SDL
	gui.mainhInstance = hInstance;
#endif
#else
int main(int argc, char **argv) {
#endif
	BYTE optind;

	memset(&info, 0x00, sizeof(info));
	info.machine = info.machine_db = DEFAULT;

	{
#if defined MINGW32 || defined MINGW64
		if (!(strncmp(argv[0] + (strlen(argv[0]) - 6), "_p", 2))) {
#else
		if (!(strcmp(argv[0] + (strlen(argv[0]) - 2), "_p"))) {
#endif
			info.portable = TRUE;
		} else {
			info.portable = FALSE;
		}
	}

	guiInit(argc, argv);

#define controlDir(fld, txt)\
{\
	char file[512];\
	sprintf(file, fld, info.base_folder);\
	if (emu_make_dir(file)) {\
		fprintf(stderr, txt);\
		return (EXIT_ERROR);\
	}\
}

	/* controllo l'esistenza della directory principale */
	if (emu_make_dir(info.base_folder)) {
		fprintf(stderr, "error on create puNES folder\n");
		return (EXIT_ERROR);
	}
	/* creo le sottocartelle */
	controlDir("%s" SAVE_FOLDER, "error on create save folder\n")
	controlDir("%s" PERGAME_FOLDER, "error on create psg folder\n")
	controlDir("%s" BIOS_FOLDER, "error on create bios folder\n")
	controlDir("%s" DIFF_FOLDER, "error on create diff folder\n")

#ifdef __NETPLAY__
	netplay_init();
#endif

	gamegenie_init();

	textInit();

	if (!info.portable) {
		textAddLineInfo(1, "[yellow]p[red]u[green]N[cyan]E[brown]S[normal]"
		" [font8](by [cyan]FHorse[normal]) [font12]%s", VERSION);
	} else {
		textAddLineInfo(1,"[font8][cyan]Portable[normal] "
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

	if (emu_turn_on()) {
		emu_quit(EXIT_FAILURE);
	}

	guiStart();

	emu_quit(EXIT_SUCCESS);

	return (EXIT_SUCCESS);
}
