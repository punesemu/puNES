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
#include "cfginput.h"
#include "common.h"
#include "emu.h"

#define	SEVEN		61
#define	VISTA		60
#define	WINXP64		52
#define	WINXP		51

#define exit_thread(value) return

struct _gui {
	HINSTANCE mainhInstance;
	WORD versionOS;
	char home[MAX_PATH];
	double frequency;
	uint64_t counterStart;
	uint8_t cpu_cores;
	uint8_t start;
	int x;
	int y;
	uint8_t left_button;
	uint8_t right_button;
} gui;

void guiInit(int argc, char **argv);
BYTE guiCreate(void);
void guiSetVideoMode(void);
void guiStart(void);
void guiEvent(void);
HWND guiWindowID(void);
void guiUpdate(void);
void guiFullscreen(void);
void guiTimeline(void);
void guiSavestate(BYTE slot);
double (*guiGetMs)(void);
int guiSleep(double ms);
void guiSetThreadAffinity(uint8_t core);

void set_effect(void);

#endif /* WIN_H_ */
