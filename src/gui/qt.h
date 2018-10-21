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

#ifndef QT_H_
#define QT_H_

#if defined (__WIN32__)
#include "win.h"
#else
#include <sys/time.h>
#endif
#include "common.h"
#include "emu.h"
#include "uncompress.h"
#include "jstick.h"

#define tools_stylesheet()\
	"QGroupBox {"\
	"	border-radius: 10px;"\
	"	border: 1px solid black;"\
	"	border: 2px groove gray;"\
	"	font-weight: bold;"\
	"}"\
	"QGroupBox::title {"\
	"	subcontrol-origin: margin;"\
	"	subcontrol-position: top center;"\
	"	padding: 0 0px;"\
	"}"

#define mainwin ((mainWindow *)gui_mainwindow_get_ptr())
#define dlgsettings ((dlgSettings *)gui_dlgsettings_get_ptr())
#define objcheat ((objCheat *)gui_objcheat_get_ptr())

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _gui {
#if defined (__WIN32__)
	uTCHAR home[MAX_PATH];
	DWORD version_os;
	double frequency;
	uint64_t counter_start;
#else
	const uTCHAR *home;
	struct timeval counterStart;
#endif

	uTCHAR last_open_path[LENGTH_FILE_NAME_MAX];
	uTCHAR last_open_ips_path[LENGTH_FILE_NAME_MAX];

	//int8_t cpu_cores;

	uint8_t start;
	uint8_t in_update;

	// lost focus pause
	uint8_t main_win_lfp;

	int dlg_rc;

	struct _key {
		DBWORD tl;
	} key;
} gui;
EXTERNC struct _gui_mouse {
	int x;
	int y;
	uint8_t left;
	uint8_t right;

	uint8_t hidden;

	double timer;
} gmouse;
EXTERNC struct _external_windows {
	uint8_t vs_system;
	uint8_t apu_channels;
	uint8_t ppu_hacks;
} ext_win;

EXTERNC void gui_quit(void);
EXTERNC BYTE gui_create(void);
EXTERNC void gui_start(void);

EXTERNC void gui_set_video_mode(void);

EXTERNC void gui_update(void);
EXTERNC void gui_update_gps_settings(void);

EXTERNC void gui_fullscreen(void);
EXTERNC void gui_timeline(void);
EXTERNC void gui_save_slot(BYTE slot);

EXTERNC void gui_print_usage(char *usage);
EXTERNC int gui_uncompress_selection_dialog(_uncompress_archive *archive, BYTE type);

EXTERNC void gui_control_pause_bck(WORD event);

EXTERNC void gui_active_window(void);
EXTERNC void gui_set_focus(void);

EXTERNC void *gui_objcheat_get_ptr(void);
EXTERNC void gui_objcheat_init(void);
EXTERNC void gui_objcheat_read_game_cheats(void);
EXTERNC void gui_objcheat_save_game_cheats(void);

EXTERNC void gui_cursor_init(void);
EXTERNC void gui_cursor_set(void);
EXTERNC void gui_cursor_hide(BYTE hide);
EXTERNC void gui_control_visible_cursor(void);

EXTERNC void *gui_mainwindow_get_ptr(void);
EXTERNC void gui_mainwindow_make_reset(int type);

EXTERNC void gui_screen_update(void);

EXTERNC void *gui_dlgsettings_get_ptr(void);

EXTERNC void gui_external_control_windows_show(void);
EXTERNC void gui_external_control_windows_update_pos(void);

EXTERNC void gui_vs_system_update_dialog(void);
EXTERNC void gui_vs_system_insert_coin(void);

EXTERNC void gui_apu_channels_widgets_update(void);

EXTERNC void gui_ppu_hacks_widgets_update(void);

#if defined (WITH_OPENGL)
EXTERNC void gui_wdgopengl_make_current(void);
EXTERNC unsigned int gui_wdgopengl_framebuffer_id(void);

EXTERNC void gui_screen_info(void);

EXTERNC uint32_t gui_color(BYTE a, BYTE r, BYTE g, BYTE b);
#endif

EXTERNC BYTE gui_load_lut(void *l, const uTCHAR *path);
EXTERNC void gui_save_screenshot(int w, int h, char *buffer, BYTE flip);

EXTERNC void gui_utf_printf(const uTCHAR *fmt, ...);
EXTERNC void gui_utf_dirname(uTCHAR *path, uTCHAR *dst, size_t len);
EXTERNC void gui_utf_basename(uTCHAR *path, uTCHAR *dst, size_t len);
EXTERNC int gui_utf_strcasecmp(uTCHAR *s0, uTCHAR *s1);

EXTERNC double (*gui_get_ms)(void);

EXTERNC void gui_init(int *argc, char **argv);
EXTERNC void gui_sleep(double ms);
#if defined (__WIN32__)
EXTERNC HWND gui_screen_id(void);
#else
EXTERNC int gui_screen_id(void);
#endif

//EXTERNC void gui_add_event(void *funct, void *args);
//EXTERNC void gui_set_thread_affinity(uint8_t core);

#undef EXTERNC

#endif /* QT_H_ */
