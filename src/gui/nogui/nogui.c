/*
 * nogui.c
 *
 *  Created on: 23/lug/2011
 *      Author: fhorse
 */

#include "nogui.h"
#include "gfx.h"
#include "snd.h"
#include "opengl.h"
#include "palette.h"
#include "clock.h"
#include "controller.h"
#include "cfg_file.h"
#include "icon.h"

SDBWORD guiPosX, guiPosY;

BYTE guiCreate(void) {
	guiPosX = guiPosY = 100;
	return (EXIT_OK);
}
void guiBeforeSet(void) {
	WORD i;
	SDL_Color colors[256];
	SDL_Surface *icon;

	snd.wmMs = SNDNOSYNC;

	/* setto l'icona della finestra */
	for (i = 0; i < 256; i++) {
		colors[i].r = applicationIcon.colors[i].r;
		colors[i].g = applicationIcon.colors[i].g;
		colors[i].b = applicationIcon.colors[i].b;
	}

	/* l'icona deve essere in formato bmp a 256 colori */
	icon = SDL_CreateRGBSurfaceFrom((void*) applicationIcon.pixelData,
			applicationIcon.width, applicationIcon.height, 8
					* applicationIcon.bytesPerPixel, applicationIcon.width
					* applicationIcon.bytesPerPixel, 0, 0, 0, 0);

	icon->format->palette->colors = (void *) colors;

	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, applicationIcon.trasparentColor);

	SDL_WM_SetIcon(icon, NULL);

	return;
}
void guiAfterSet(void) {
	/*
	 * questo con l'X11 mi serve per sapere la posizione
	 * della finestra, mentre con Windows per intercettare
	 * lo spostamento della finestra.
	 */
	SDL_VERSION(&gui.pInfo.version);

	if (!SDL_GetWMInfo(&gui.pInfo)) {
		fprintf(stderr, "SDL GetWMInfo failed\n");
    }

#if defined (MINGW32) || defined (MINGW64)
	/* abilito l'intercettazzioni di eventi particolari */
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	/*
	 * bloccare il rilevamento di errori di sync
	 * audio per un tot di millisecondi.
	 */
	//sndWmEvent(2000);

}
void guiStart(void) {
	emu_loop();
}
void guiEvent(void) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			info.stop = TRUE;
			break;
#if defined (MINGW32) || defined (MINGW64)
		case SDL_SYSWMEVENT:
			switch (event.syswm.msg->msg) {
			case WM_CAPTURECHANGED:
			case WM_MOVE:
				//sndWmEvent(500);
				break;
			default:
				break;
			}
			break;
#else
		/*
		 * con questo evento intercetto qualsiasi modifica
		 * avvenga alla finestra (spostamento, ridimensionamento,
		 * o qualsiasi altra cosa (credo).
		 */
		case SDL_VIDEOEXPOSE:
			//sndWmEvent(600);
			break;
#endif
#if defined (OPENGL)
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				opengl.mouseLeftButton = TRUE;
				opengl.x_diff =  event.motion.x - (opengl.y_rotate * slow_factor);
				opengl.y_diff = -event.motion.y + (opengl.x_rotate * slow_factor);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT) {
				opengl.mouseLeftButton = FALSE;
			}
			break;
		case SDL_MOUSEMOTION:
			if (opengl.mouseLeftButton && opengl.rotation) {
				opengl.x_rotate = (event.motion.y + opengl.y_diff) / slow_factor;
				opengl.y_rotate = (event.motion.x - opengl.x_diff) / slow_factor;
			}
			break;
#endif
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_F8:
				if (machine.type == NTSC) {
					machine = machinePAL;
					if (gfx.palette == PALETTENTSC) {
						gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTEPAL, FALSE);
					}
				} else {
					machine = machineNTSC;
					if (gfx.palette == PALETTEPAL) {
						gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTENTSC, FALSE);
					}
				}
				if (emu_reset(CHANGE_MODE)) { emu_quit(EXIT_FAILURE); }
				break;
			case SDLK_F11:
				if (emu_reset(HARD)) { emu_quit(EXIT_FAILURE); }
				break;
			case SDLK_F12:
				if (emu_reset(RESET)) { emu_quit(EXIT_FAILURE); }
				break;
			case SDLK_ESCAPE:
				info.stop = TRUE;
				break;
			case SDLK_1:
				/* 1x */
				gfx_set_screen(X1, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
				break;
			case SDLK_2:
				/* 2x */
				gfx_set_screen(X2, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
				break;
			case SDLK_3:
				/* 3x */
				gfx_set_screen(X3, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
				break;
			case SDLK_4:
				/* 4x */
				gfx_set_screen(X4, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
				break;
			case SDLK_5:
				/* no filter */
				gfx_set_screen(NO_CHANGE, NO_FILTER, NO_CHANGE, NO_CHANGE, FALSE);
				break;
			case SDLK_6:
				/* filtro scale2x */
				gfx_set_screen(NO_CHANGE, SCALE2X, NO_CHANGE, NO_CHANGE, FALSE);
				break;
			case SDLK_7:
				/* filtro ntsc */
				if (gfx.filter == RGBNTSC) {
					if (++gfx.ntscFormat > RGBMODE) {
						gfx.ntscFormat = COMPOSITE;
					}
					ntsc_set(gfx.ntscFormat, 0, 0, (BYTE *) palette_RGB, 0);
					gui_update();
				} else {
					gfx_set_screen(NO_CHANGE, RGBNTSC, NO_CHANGE, NO_CHANGE, FALSE);
				}
				break;
			case SDLK_a:
				/* B */
				controller.dev1[BUT_B] = 0x41;
				break;
			case SDLK_s:
				/* A */
				controller.dev1[BUT_A] = 0x41;
				break;
			case SDLK_z:
				/* Select */
				controller.dev1[SELECT] = 0x41;
				break;
			case SDLK_x:
				/* Start */
				controller.dev1[START] = 0x41;
				break;
			case SDLK_w:
				cfg_file_save();
				break;
#if defined (OPENGL)
			case SDLK_f:
				/* Fullscreen */
				if (gfx.fullscreen == NO_FULLSCR) {
					if (!opengl.rotation) {
						SDL_ShowCursor(SDL_DISABLE);
					}
					/*
					 * imposto il fattore di scale a quello predefinito per il
					 * fullscreen lo abilito.
					 */
					gfx_set_screen(SCALEFLSCR, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE);
				} else {
					/* riabilito la visualizzazione del puntatore */
					SDL_ShowCursor(SDL_ENABLE);
					/* ripristino i valori di scale ed esco dal fullscreen */
					gfx_set_screen(gfx.scaleBeforeFullscreen, NO_CHANGE,
							NO_FULLSCR, NO_CHANGE, FALSE);
				}
				break;
			case SDLK_o:
				/* switch opengl/software render */
				gfx.opengl = !gfx.opengl;

				if (gfx.opengl) {
					opengl.rotation = FALSE;
				}

				gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
				break;
			case SDLK_p:
				/* switch stretch ma solo in fullscreen */
				if (gfx.fullscreen) {
					opengl.aspectRatio = !opengl.aspectRatio;
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
				}
				break;
			case SDLK_r:
				/* switch stretch ma solo in fullscreen */
				opengl.rotation = !opengl.rotation;
				if (opengl.rotation) {
					opengl.factor_distance = opengl.x_rotate = opengl.y_rotate = 0;
					if (gfx.fullscreen == FULLSCR) {
						SDL_ShowCursor(SDL_ENABLE);
					}
				} else {
					if (gfx.fullscreen == FULLSCR) {
						SDL_ShowCursor(SDL_DISABLE);
					}
				}
				gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
				break;
#endif
			case SDLK_UP:
				/* Up */
				controller.dev1[UP] = 0x41;
				/* non posono essere premuti contemporaneamente */
				controller.dev1[DOWN] = 0x40;
				break;
			case SDLK_DOWN:
				/* Down */
				controller.dev1[DOWN] = 0x41;
				/* non posono essere premuti contemporaneamente */
				controller.dev1[UP] = 0x40;
				break;
			case SDLK_LEFT:
				/* Left */
				controller.dev1[LEFT] = 0x41;
				/* non posono essere premuti contemporaneamente */
				controller.dev1[RIGHT] = 0x40;
				break;
			case SDLK_RIGHT:
				/* Right */
				controller.dev1[RIGHT] = 0x41;
				/* non posono essere premuti contemporaneamente */
				controller.dev1[LEFT] = 0x40;
				break;
			case SDLK_COMMA:
				/* , */
			{
				BYTE i = gfx.palette;

				if (--i > PALETTEGREEN) {
					i = PALETTEGREEN;
				}
				gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, i, FALSE);
			}
				break;
			case SDLK_PERIOD:
				/* . */
			{
				BYTE i = gfx.palette;

				if (++i > PALETTEGREEN) {
					i = PALETTEPAL;
				}
				gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, i, FALSE);
			}
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_a:
				/* A */
				controller.dev1[BUT_B] = 0x40;
				break;
			case SDLK_s:
				/* B */
				controller.dev1[BUT_A] = 0x40;
				break;
			case SDLK_z:
				/* Select */
				controller.dev1[SELECT] = 0x40;
				break;
			case SDLK_x:
				/* Start */
				controller.dev1[START] = 0x40;
				break;
			case SDLK_UP:
				/* Up */
				controller.dev1[UP] = 0x40;
				break;
			case SDLK_DOWN:
				/* Down */
				controller.dev1[DOWN] = 0x40;
				break;
			case SDLK_LEFT:
				/* Left */
				controller.dev1[LEFT] = 0x40;
				break;
			case SDLK_RIGHT:
				/* Right */
				controller.dev1[RIGHT] = 0x40;
				break;
			default:
				break;
			}
			break;
		}
	}
}
int guiWindowID(void) {
	return(0);
}
void gui_update(void) {
	char title[255];

	emu_set_title(title);
	SDL_WM_SetCaption(title, NULL);
}
void guiSetPosition(char *buffer) {
	sprintf(buffer, "SDL_VIDEO_WINDOW_POS=%d,%d",
			guiPosX, guiPosY);
}
#if defined (OPENGL)
void guiFullscreen(void) {
	return;
}
#endif

#if defined (MINGW32) || defined (MINGW64)
void guiInit(int argc, char **argv) {
	OSVERSIONINFO winInfo;

	info.gui = FALSE;

	ZeroMemory(&winInfo, sizeof(OSVERSIONINFO));
	winInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&winInfo);

	gui.versionOS = ((winInfo.dwMajorVersion * 10) | winInfo.dwMinorVersion);

	guiFindHOME();
}
void guiGetPosition(void) {
	static SDL_SysWMinfo pInfo;
	POINT client;

	SDL_VERSION(&pInfo.version);
	SDL_GetWMInfo(&pInfo);

	/*
	 * prendo le cordinate iniziali della client area
	 * e le trasformo in coordinate del desktop.
	 */
	client.x = client.y = 0;
	ClientToScreen(pInfo.window, &client);
	guiPosX = client.x;
	guiPosY = client.y;
}
void guiGetResolution(void) {
	gfx.w = GetSystemMetrics(SM_CXSCREEN);
	gfx.h = GetSystemMetrics(SM_CYSCREEN);
}
void guiFindHOME(void) {
	TCHAR szPath[MAX_PATH];
	HRESULT hr;

	switch (gui.versionOS) {
	case SEVEN:
	case VISTA:
		// FIXME : non compila sotto mingw
		//hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL,
		//		&gui.home);
		//break;
	case WINXP:
	case WINXP64:
		hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath);
		gui.home = szPath;
		break;
	}
}
#else
void guiInit(int argc, char **argv) {
	info.gui = FALSE;
	guiFindHOME();
}
void guiGetPosition(void) {
	Window dummy;
	XWindowAttributes attribute, borders;

	/* locko la finestra */
	gui.pInfo.info.x11.lock_func();
	/*
	 * the XSync function flushes the output buffer
	 * and then waits until all requests have been
	 * received and processed by the X server.
	 * P.s. chiaro no?
	 */
	XSync(gui.pInfo.info.x11.display, FALSE);
	/* ottengo gli attributi della finestra */
	XGetWindowAttributes(gui.pInfo.info.x11.display, gui.pInfo.info.x11.window,
			&attribute);
	/*
	 * if XTranslateCoordinates returns True, it takes
	 * the src_x and src_y coordinates relative to the
	 * source window's origin and returns these coordinates
	 * to dest_x_return and dest_y_return relative to
	 * the destination window's origin.
	 */
	XTranslateCoordinates(gui.pInfo.info.x11.display, gui.pInfo.info.x11.window,
			attribute.root, 0, 0, &attribute.x, &attribute.y, &dummy);
	/*
	 * ho bisogno anche degli attributi delle finestra
	 * del window manager per ottenere  le dimensioni
	 * dei bordi decorativi, infatti la wmwindow inizia
	 * esattamante dove finiscono le decorazioni (ergo
	 * posso considerare x e y come loro dimensioni).
	 */
	XGetWindowAttributes(gui.pInfo.info.x11.display, gui.pInfo.info.x11.wmwindow,
			&borders);
	/* unlock della finestra */
	gui.pInfo.info.x11.unlock_func();
	/* calcolo esattamente la posizione nel desktop */
	guiPosX = attribute.x - borders.x;
	guiPosY = attribute.y - borders.y;
}
void guiGetResolution(void) {
	Display *display;
	Screen *screen;

	display = XOpenDisplay(NULL);
	screen = DefaultScreenOfDisplay(display);
	gfx.w = WidthOfScreen(screen);
	gfx.h = HeightOfScreen(screen);
}
void guiFindHOME(void) {
	gui.home = SDL_getenv("HOME");
}

#endif

