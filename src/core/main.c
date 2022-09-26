/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "compilation_unit_orphan.h"
#include "../gui/cmd_line.h"
#include "emu.h"
#include "emu_thread.h"
#include "settings.h"
#include "video/gfx.h"
#include "version.h"
#include "cheat.h"
#include "recent_roms.h"
#include "patcher.h"
#include "ppu.h"
#include "mappers.h"
#include "tape_data_recorder.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif
#if defined (FULLSCREEN_RESFREQ)
#include "video/gfx_monitor.h"
#endif

#if defined (_WIN32)
int WINAPI WinMain(UNUSED(HINSTANCE hInstance), UNUSED(HINSTANCE hPrevInstance), UNUSED(PSTR szCmdLine), UNUSED(int iCmdShow)) {
	int argc = 0;
	uTCHAR **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
#else
int main(int argc, char **argv) {
#endif
	setlocale(LC_CTYPE, "");

	memset(&debugger, 0x00, sizeof(debugger));
	memset(&info, 0x00, sizeof(info));
	memset(&jstick, 0x00, sizeof(jstick));
	memset(&chinaersan2, 0x00, sizeof(chinaersan2));
	memset(&tape_data_recorder, 0x00, sizeof(tape_data_recorder));

	info.no_rom = TRUE;
	info.doublebuffer = TRUE;
	info.machine[HEADER] = info.machine[DATABASE] = DEFAULT;

	if (cmd_line_check_portable(argc, argv) == TRUE) {
		info.portable = TRUE;
	} else {
		info.portable = FALSE;
	}

	if (gui_init(&argc, (char **)argv) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	patcher_init();

	gamegenie_init();

	gui_overlay_info_init();
	gui_overlay_info_emulator();

	// tratto il file di configurazione ed i parametri passati dalla riga di comando.
	settings_init();
	if (cmd_line_parse(argc, argv) == EXIT_ERROR) {
		emu_quit();
		return (EXIT_SUCCESS);
	}

	if (gui_control_instance() == EXIT_ERROR) {
		emu_quit();
		return (EXIT_SUCCESS);
	}

	ufprintf(stderr, uL("INFO: config folder \"" uPs("") "\"\n"), gui_config_folder());
	ufprintf(stderr, uL("INFO: data folder   \"" uPs("") "\"\n"), gui_data_folder());
	ufprintf(stderr, uL("INFO: tmp folder    \"" uPs("") "\"\n"), gui_temp_folder());

#if defined (__NETPLAY__)
	netplay_init();
#endif

#if defined (FULLSCREEN_RESFREQ)
	gfx_monitor_init();
#endif

	recent_roms_init();
	recent_roms_parse();

#if defined (WITH_FFMPEG)
	recording_init();
#endif

	uncompress_init();

	ppu_init();

	if (emu_turn_on()) {
		emu_quit();
		return (EXIT_FAILURE);
	}

	emu_frame_input_and_rewind();

	if (emu_thread_init()) {
		emu_quit();
		return (EXIT_FAILURE);
	}

	js_scan_thread_init();

	gui_start();

	js_scan_thread_quit();
	emu_thread_quit();
	emu_quit();

	return (EXIT_SUCCESS);
}
