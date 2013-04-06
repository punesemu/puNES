/*
 * win.h
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#ifndef WIN_H_
#define WIN_H_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#include "resources.h"
#include "keyboard.h"
#include "joystick.h"
#include "cfg_input.h"
#include "common.h"
#include "emu.h"

#define	WIN_EIGHT 62
#define	WIN_SEVEN 61
#define	WIN_VISTA 60
#define	WIN_XP64  52
#define	WIN_XP    51

#define exit_thread(value) return

struct _gui {
	HINSTANCE main_hinstance;
	WORD version_os;
	char home[MAX_PATH];
	double frequency;
	uint64_t counter_start;
	uint8_t cpu_cores;
	uint8_t start;
	int x;
	int y;
	uint8_t left_button;
	uint8_t right_button;
} gui;

void gui_init(int argc, char **argv);
void gui_quit(void);
BYTE gui_create(void);
void gui_set_video_mode(void);
void gui_start(void);
void gui_event(void);
HWND gui_emu_frame_id(void);
void gui_update(void);
void gui_fullscreen(void);
void gui_timeline(void);
void gui_save_slot(BYTE slot);
double (*gui_get_ms)(void);
void gui_sleep(double ms);
void gui_set_thread_affinity(uint8_t core);

void set_effect(void);

#endif /* WIN_H_ */
