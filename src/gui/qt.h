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

#ifndef QT_H_
#define QT_H_

#if defined (_WIN32)
#include "win.h"
#else
#include <sys/time.h>
#endif
#include "common.h"
#include "emu.h"
#include "uncompress.h"
#include "jstick.h"

enum _overlay_info_alignment {
	OVERLAY_INFO_LEFT,
	OVERLAY_INFO_CENTER,
	OVERLAY_INFO_RIGHT
};

//	"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #aaabae);"

#define button_stylesheet()\
	"QPushButton {"\
	"	margin: 0; padding: 2px; border: 2px groove gray;"\
	"}"\
	"QPushButton:pressed {"\
	"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #aaabae, stop: 1 #f6f7fa);"\
	"}"\
	"QPushButton:disabled {"\
	"	color: gray;"\
	"}"\
	"QPushButton:disabled:checked {"\
	"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #aaabae, stop: 1 #f6f7fa);"\
	"	color: gray;"\
	"}"\
	"QPushButton:checked {"\
	"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #aaabae, stop: 1 #f6f7fa);"\
	"	color: black;"\
	"}"
#define group_title_bold_stylesheet()\
	"QGroupBox {"\
	"	font-weight: bold;"\
	"}"
#define group_title_and_button_stylesheet()\
	group_title_bold_stylesheet() button_stylesheet()
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
#define toolbar_button_stylesheet()\
	"QPushButton {"\
	"	margin :0; padding: 2px; border: 1px groove lightgray;"\
	"}"\
	"QPushButton:hover {"\
	"	border: 1px groove darkgray;"\
	"}"\
	"QPushButton:pressed {"\
	"	margin :0; padding: 2px; border: 1px inset darkgray;"\
	"}"\
	"QPushButton:disabled {"\
	"	color: gray;"\
	"}"\
	"QPushButton:disabled:checked {"\
	"	margin :0; padding: 2px; border: 1px inset darkgray;"\
	"	color: gray;"\
	"}"\
	"QPushButton:checked {"\
	"	margin :0; padding: 2px; border: 1px inset darkgray;"\
	"	color: black;"\
	"}"
#define toolbar_toolbutton_stylesheet()\
	"QToolButton {"\
	"	margin :0; padding: 2px; border: 1px groove lightgray;"\
	"}"\
	"QToolButton:hover {"\
	"	border: 1px groove darkgray;"\
	"}"\
	"QToolButton:pressed {"\
	"	margin :0; padding: 2px; border: 1px inset darkgray;"\
	"}"\
	"QToolButton:disabled {"\
	"	color: gray;"\
	"}"\
	"QToolButton:checked:disabled {"\
	"	margin :0; padding: 2px; border: 1px inset darkgray;"\
	"	color: gray;"\
	"}"\
	"QToolButton:checked {"\
	"	margin :0; padding: 2px; border: 1px inset darkgray;"\
	"	color: black;"\
	"}"

#define mainwin ((mainWindow *)gui_mainwindow_get_ptr())
#define wdgrewind ((wdgRewind *)gui_wdgrewind_get_ptr())
#define dlgsettings ((dlgSettings *)gui_dlgsettings_get_ptr())
#define dlgjsc ((dlgJsc *)gui_dlgjsc_get_ptr())
#define dlgkeyb ((dlgKeyboard *)gui_dlgkeyboard_get_ptr())
#define objcheat ((objCheat *)gui_objcheat_get_ptr())
#define wdgoverlayui ((wdgOverlayUi *)gui_wdgoverlayui_get_ptr())

typedef struct _gui {
#if defined (_WIN32)
	uTCHAR home[MAX_PATH];
	const uTCHAR *ostmp;
	DWORD version_os;
	double frequency;
	uint64_t counter_start;
#else
	const uTCHAR *home;
	const uTCHAR *ostmp;
	struct timeval counterStart;
#endif
	uTCHAR last_open_path[LENGTH_FILE_NAME_MAX];
	uTCHAR last_open_patch_path[LENGTH_FILE_NAME_MAX];

	//int8_t cpu_cores;

	uint8_t start;
	uint8_t in_update;
	uint8_t capture_input;

	// lost focus pause
	uint8_t main_win_lfp;

	int dlg_rc;
	int dlg_tabWidget_kbd_joy_index[PORT_MAX];
} _gui;
typedef struct _gui_mouse {
	int x;
	int y;
	uint8_t left;
	uint8_t right;

	uint8_t hidden;

	double timer;
} _gui_mouse;
typedef struct _external_windows {
	uint8_t vs_system;
} _external_windows;

extern _gui gui;
extern _gui_mouse gmouse;
extern _external_windows ext_win;

extern double (*gui_get_ms)(void);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void gui_init(int *argc, char **argv);
EXTERNC void gui_quit(void);
EXTERNC BYTE gui_control_instance(void);
EXTERNC BYTE gui_create(void);
EXTERNC void gui_start(void);

EXTERNC double gui_device_pixel_ratio(void);
EXTERNC void gui_set_window_size(void);

EXTERNC void gui_state_save_slot_set(BYTE slot, BYTE on_video);
EXTERNC void gui_state_save_slot_set_tooltip(BYTE slot, char *buffer);

EXTERNC void gui_update(void);
EXTERNC void gui_update_dset(void);
EXTERNC void gui_update_gps_settings(void);
EXTERNC void gui_update_status_bar(void);

EXTERNC void gui_update_ntsc_widgets(void);
EXTERNC void gui_update_apu_channels_widgets(void);
EXTERNC void gui_update_recording_widgets(void);

EXTERNC void gui_update_ppu_hacks_lag_frames(void);

EXTERNC void gui_update_fds_menu(void);
EXTERNC void gui_update_recording_tab(void);

EXTERNC void gui_egds_set_fps(void);
EXTERNC void gui_egds_stop_unnecessary(void);
EXTERNC void gui_egds_start_pause(void);
EXTERNC void gui_egds_stop_pause(void);
EXTERNC void gui_egds_start_rwnd(void);
EXTERNC void gui_egds_stop_rwnd(void);

EXTERNC void gui_fullscreen(void);

EXTERNC int gui_uncompress_selection_dialog(_uncompress_archive *archive, BYTE type);

EXTERNC void gui_control_pause_bck(WORD event);

EXTERNC void gui_active_window(void);
EXTERNC void gui_set_focus(void);

EXTERNC void *gui_objcheat_get_ptr(void);
EXTERNC void gui_objcheat_init(void);
EXTERNC void gui_objcheat_read_game_cheats(void);

EXTERNC void gui_cursor_init(void);
EXTERNC void gui_cursor_set(void);
EXTERNC void gui_cursor_hide(BYTE hide);
EXTERNC void gui_control_visible_cursor(void);

EXTERNC void *gui_mainwindow_get_ptr(void);
EXTERNC void gui_mainwindow_coords(int *x, int *y, BYTE border);
EXTERNC void gui_mainwindow_before_set_res(void);

EXTERNC void *gui_wdgrewind_get_ptr(void);
EXTERNC void gui_wdgrewind_play(void);

EXTERNC void gui_emit_et_gg_reset(void);
EXTERNC void gui_emit_et_vs_reset(void);
EXTERNC void gui_emit_et_external_control_windows_show(void);
EXTERNC void gui_emit_et_input_update_combo(void);

EXTERNC void gui_max_speed_start(void);
EXTERNC void gui_max_speed_stop(void);

EXTERNC void gui_decode_all_input_events(void);

EXTERNC void gui_screen_update(void);

EXTERNC void *gui_wdgoverlayui_get_ptr(void);
EXTERNC void gui_overlay_update(void);
EXTERNC BYTE gui_overlay_is_updated(void);
EXTERNC void gui_overlay_enable_save_slot(BYTE mode);
EXTERNC void gui_overlay_set_size(int w, int h);
EXTERNC void gui_overlay_info_init(void);
EXTERNC void gui_overlay_info_emulator(void);
EXTERNC void gui_overlay_info_append_subtitle(uTCHAR *msg);
EXTERNC void gui_overlay_info_append_msg_precompiled(int index, void *arg1);
EXTERNC void gui_overlay_info_append_msg_precompiled_with_alignment(BYTE alignment, int index, void *arg1);
EXTERNC void gui_overlay_blit(void);
EXTERNC void gui_overlay_slot_preview(int slot, void *buffer, uTCHAR *file);

EXTERNC void *gui_dlgsettings_get_ptr(void);
EXTERNC void gui_dlgsettings_input_update_joy_combo(void);

EXTERNC void *gui_dlgjsc_get_ptr(void);
EXTERNC void gui_dlgjsc_emit_update_joy_combo(void);

EXTERNC void *gui_dlgkeyboard_get_ptr(void);

EXTERNC void gui_js_joyval_icon_desc(int index, DBWORD input, void *icon, void *desc);

EXTERNC void *gui_dlgdebugger_get_ptr(void);
EXTERNC void gui_dlgdebugger_click_step(void);

EXTERNC void gui_external_control_windows_show(void);
EXTERNC void gui_external_control_windows_update_pos(void);

EXTERNC void gui_vs_system_update_dialog(void);
EXTERNC void gui_vs_system_insert_coin(void);

EXTERNC void gui_nes_keyboard(void);
EXTERNC void gui_nes_keyboard_paste_event(void);

#if defined (WITH_OPENGL)
EXTERNC void gui_wdgopengl_make_current(void);
EXTERNC unsigned int gui_wdgopengl_framebuffer_id(void);

EXTERNC void gui_screen_info(void);

EXTERNC uint32_t gui_color(BYTE a, BYTE r, BYTE g, BYTE b);
#endif

EXTERNC BYTE gui_load_lut(void *l, const uTCHAR *path);
EXTERNC void gui_save_screenshot(int w, int h, int stride, char *buffer, BYTE flip);

EXTERNC void gui_utf_printf(const uTCHAR *fmt, ...);
EXTERNC void gui_utf_dirname(uTCHAR *path, uTCHAR *dst, size_t len);
EXTERNC void gui_utf_basename(uTCHAR *path, uTCHAR *dst, size_t len);
EXTERNC int gui_utf_strcasecmp(uTCHAR *s0, uTCHAR *s1);

EXTERNC unsigned int gui_hardware_concurrency(void);

EXTERNC void gui_init_os(void);
EXTERNC void gui_sleep(double ms);
#if defined (_WIN32)
EXTERNC HWND gui_screen_id(void);
EXTERNC HWND gui_win_id(void);
EXTERNC char *gui_dup_wchar_to_utf8(uTCHAR *w);
#else
EXTERNC int gui_screen_id(void);
EXTERNC int gui_win_id(void);
#endif

#if defined (FULLSCREEN_RESFREQ)
EXTERNC BYTE gui_monitor_enum_monitors(void);
EXTERNC void gui_monitor_set_res(void *monitor_info, void *mode_info);
EXTERNC void gui_monitor_get_current_x_y(void *monitor_info, int *x, int *y);
#endif

#undef EXTERNC

#endif /* QT_H_ */
