/*
 * nogui.h
 *
 *  Created on: 23/lug/2011
 *      Author: fhorse
 */

#ifndef NOGUI_H_
#define NOGUI_H_

#if defined (MINGW32)
#include <windows.h>
#include <shlobj.h>
#else
#include <X11/Xlib.h>
#endif

#include <SDL.h>
#include <SDL_getenv.h>
#include <SDL_syswm.h>
#include "common.h"
#include "emu.h"

#define	SEVEN		61
#define	VISTA		60
#define	WINXP64		52
#define	WINXP		51

void guiInit(int argc, char **argv);
BYTE guiCreate(void);
void guiBeforeSet(void);
void guiAfterSet(void);
void guiStart(void);
void guiEvent(void);
int guiWindowID(void);
void gui_update(void);
void guiGetPosition(void);
void guiSetPosition(char *buffer);
void guiGetResolution(void);
#if defined (OPENGL)
void guiFullscreen(void);
#endif

void guiFindHOME(void);

typedef struct {
	SDL_SysWMinfo pInfo;
	WORD versionOS;
	const char *home;
} _gui;

_gui gui;

#endif /* NOGUI_H_ */
