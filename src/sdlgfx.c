/*
 * sdlgfx.c
 *
 *  Created on: 07/apr/2010
 *      Author: fhorse
 */

#include "emu.h"
#include "cpu6502.h"
#include "sdlgfx.h"
#include "overscan.h"
#include "clock.h"
#include "input.h"
#include "ppu.h"
#include "version.h"
#include "gui.h"
#include "sdltext.h"
#define __STATICPAL__
#include "palette.h"
#ifdef OPENGL
#include "opengl.h"
#endif

#if defined MINGW64
#define sdlWid()\
	if (info.gui) {\
		char SDL_windowhack[50];\
		sprintf(SDL_windowhack,"SDL_WINDOWID=%I64u", (uint64_t) guiWindowID());\
		SDL_putenv(SDL_windowhack);\
	}
#else
#define sdlWid()\
	if (info.gui) {\
		char SDL_windowhack[50];\
		sprintf(SDL_windowhack,"SDL_WINDOWID=%i", (int) guiWindowID());\
		SDL_putenv(SDL_windowhack);\
	}
#endif
#define NTSCWIDTH(wdt, a, flag)\
{\
	wdt = 0;\
	if (newFilter == RGBNTSC) {\
		wdt = NES_NTSC_OUT_WIDTH(gfx.rows, a);\
		if (overscan.enabled) {\
			wdt -= (a - nes_ntsc_in_chunk);\
		}\
		if (flag) {\
			gfx.w[CURRENT] = wdt;\
			gfx.w[NOOVERSCAN] = (NES_NTSC_OUT_WIDTH(SCRROWS, a));\
		}\
	}\
}
#define CHANGECOLOR(index, color, operation)\
	tmp = paletteRGB[index].color + operation;\
	paletteRGB[index].color = (tmp < 0 ? 0 : (tmp > 0xFF ? 0xFF : tmp))
#define RGBMODIFIER(red, green, blue)\
	/* prima ottengo la paletta monocromatica */\
	ntscSet(gfx.ntscFormat, PALETTEMONO, 0, 0, (BYTE *) paletteRGB);\
	/* quindi la modifico */\
	{\
		WORD i;\
		SWORD tmp;\
		for (i = 0; i < NCOLORS; i++) {\
			/* rosso */\
			CHANGECOLOR(i, r, red);\
			/* green */\
			CHANGECOLOR(i, g, green);\
			/* blue */\
			CHANGECOLOR(i, b, blue);\
		}\
	}\
	/* ed infine utilizzo la nuova */\
	ntscSet(gfx.ntscFormat, FALSE, 0, (BYTE *) paletteRGB,(BYTE *) paletteRGB)

SDL_Surface *framebuffer;
uint32_t *paletteWindow, flagsSoftware;
static BYTE ntsc_width_pixel[5] = {0, 0, 7, 10, 14};

BYTE gfxInit(void) {
	const SDL_VideoInfo *vInfo;

	overscan.left = 8;
	overscan.right = 9;
	overscan.up = 8;
	overscan.down = 8;

	if (guiCreate()) {
		fprintf(stderr, "gui initialization failed\n");
		return (EXIT_ERROR);
	}

	sdlWid();

	/* inizializzazione SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	vInfo = SDL_GetVideoInfo();

	/*
	 * modalita' video con profondita' di colore
	 * inferiori a 15 bits non sono supportate.
	 */
	if (vInfo->vfmt->BitsPerPixel < 15) {
		fprintf(stderr, "Sorry but video mode at 256 color are not supported\n");
		return (EXIT_ERROR);
	}

	/* il filtro hqx supporta solo i 32 bit di profondita' di colore */
	if (((gfx.filter >= HQ2X) || (gfx.filter <= HQ4X)) && (vInfo->vfmt->BitsPerPixel < 32)) {
		gfx.filter = NOFILTER;
	}

	/* controllo se e' disponibile l'accelerazione hardware */
	if (vInfo->hw_available) {
		flagsSoftware = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT;
	} else {
		flagsSoftware = SDL_SWSURFACE | SDL_ASYNCBLIT;
	}

#ifdef OPENGL
	sdlInitGL();
#endif

	/*
	 * inizializzo l'ntsc che utilizzero' non solo
	 * come filtro ma anche nel gfxSetScreen() per
	 * generare la paletta dei colori.
	 */
	if (ntscInit(0, 0, 0, 0, 0)) {
		return (EXIT_ERROR);
	}

	/*
	 * mi alloco una zona di memoria dove conservare la
	 * paletta nel formato di visualizzazione.
	 */
	paletteWindow = malloc(NCOLORS * sizeof(uint32_t));
	if (!paletteWindow) {
		fprintf(stderr, "Out of memory");
		return (EXIT_ERROR);
	}

	if (gfx.fullscreen) {
		gfxSetScreen(gfx.scale, gfx.filter, NOFULLSCR, gfx.palette, FALSE);
		gfx.fullscreen = NOFULLSCR;
		gfx.scale = gfx.scaleBeforeFullscreen;
		guiFullscreen();
	} else {
		gfxSetScreen(gfx.scale, gfx.filter, NOFULLSCR, gfx.palette, FALSE);
	}

	if (!surfaceSDL) {
		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void gfxSetScreen(BYTE newScale, BYTE newFilter, BYTE newFullscreen, BYTE newPalette,
		BYTE forceScale) {
	BYTE setMode = FALSE;
	WORD width = 0, height = 0;
	WORD wForPr = 0, hForPr = 0;

	/*
	 * l'ordine dei vari controlli non deve essere cambiato:
	 * 0) overscan
	 * 1) filtro
	 * 2) fullscreen
	 * 3) fattore di scala
	 * 4) tipo di paletta (IMPORTANTE: dopo il SDL_SetVideoMode)
	 */

	/* overscan */
	{
		overscan.enabled = gfx.overscan;

		gfx.rows = SCRROWS;
		gfx.lines = SCRLINES;

		if (overscan.enabled == OSCANDEF) {
			overscan.enabled = gfx.overscanDefault;
		}

		if (overscan.enabled) {
			gfx.rows -= (overscan.left + overscan.right);
			gfx.lines -= (overscan.up + overscan.down);
		}
	}

	/* filtro */
	if (newFilter == NOCHANGE) {
		newFilter = gfx.filter;
	}
	if (newFilter != gfx.filter || gfx.onCfg) {
		switch (newFilter) {
			case NOFILTER:
				effect = scaleSurface;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (gfx.filter == RGBNTSC) {
					/* devo reimpostare la larghezza del video mode */
					newScale = gfx.scale;
					/* forzo il controllo del fattore di scale */
					forceScale = TRUE;
					/* indico che devo cambiare il video mode */
					setMode = TRUE;
				}
				break;
			case BILINEAR:
				effect = bilinear;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (gfx.filter == RGBNTSC) {
					/* forzo il controllo del fattore di scale */
					forceScale = TRUE;
					/* indico che devo cambiare il video mode */
					setMode = TRUE;
				}
				break;
			case SCALE2X:
			case SCALE3X:
			case SCALE4X:
				effect = scaleNx;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (gfx.filter == RGBNTSC) {
					/* forzo il controllo del fattore di scale */
					forceScale = TRUE;
					/* indico che devo cambiare il video mode */
					setMode = TRUE;
				}
				break;
			case HQ2X:
			case HQ3X:
			case HQ4X:
				effect = hqNx;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (gfx.filter == RGBNTSC) {
					/* forzo il controllo del fattore di scale */
					forceScale = TRUE;
					/* indico che devo cambiare il video mode */
					setMode = TRUE;
				}
				break;
			case RGBNTSC:
				effect = ntscSurface;
				/*
				 * il fattore di scala deve essere gia' stato
				 * inizializzato almeno una volta.
				 */
				if (gfx.scale != NOCHANGE) {
					/* devo reimpostare la larghezza del video mode */
					newScale = gfx.scale;
				} else if (newScale == NOCHANGE) {
					/*
					 * se scale e newScale sono uguali a NOCHANGE,
					 * imposto un default.
					 */
					newScale = X2;
				}
				/* forzo il controllo del fattore di scale */
				forceScale = TRUE;
				/* indico che devo cambiare il video mode */
				setMode = TRUE;
				break;
		}
	}

	/* fullscreen */
	if (newFullscreen == NOCHANGE) {
		newFullscreen = gfx.fullscreen;
	}
	if (newFullscreen != gfx.fullscreen || gfx.onCfg) {
		/* forzo il controllo del fattore di scale */
		forceScale = TRUE;
		/* indico che devo cambiare il video mode */
		setMode = TRUE;
	}

	/* fattore di scala */
	if (newScale == NOCHANGE) {
		newScale = gfx.scale;
	}
	if (newScale != gfx.scale || gfx.onCfg || forceScale) {

		#define ctrlFilterScale(scalexf, hqxf)\
			if ((newFilter >= SCALE2X) && (newFilter <= SCALE4X)) {\
				newFilter = scalexf;\
			} else  if ((newFilter >= HQ2X) && (newFilter <= HQ4X)) {\
				newFilter = hqxf;\
			}

		switch (newScale) {
			case X1:
				/*
				 * il fattore di scala a 1 e' possibile
				 * solo senza filtro.
				 */
				if (newFilter != NOFILTER) {
					/*
					 * con un fatto redi scala X1 effect deve essere
					 * sempre impostato su scaleSurface.
					 */
					effect = scaleSurface;
					return;
				}
				setMode = TRUE;
				break;
			case X2:
				ctrlFilterScale(SCALE2X, HQ2X)
				NTSCWIDTH(width, ntsc_width_pixel[newScale], TRUE);
				setMode = TRUE;
				break;
			case X3:
				ctrlFilterScale(SCALE3X, HQ3X)
				NTSCWIDTH(width, ntsc_width_pixel[newScale], TRUE);
				setMode = TRUE;
				break;
			case X4:
				ctrlFilterScale(SCALE4X, HQ4X)
				NTSCWIDTH(width, ntsc_width_pixel[newScale], TRUE);
				setMode = TRUE;
				break;
		}
		if (!width) {
			width = gfx.rows * newScale;
			gfx.w[CURRENT] = width;
			gfx.w[NOOVERSCAN] = SCRROWS * newScale;
		}
		height = gfx.lines * newScale;
		gfx.h[CURRENT] = height;
		gfx.h[NOOVERSCAN] = SCRLINES * newScale;
	}

#ifdef OPENGL
	/*
	 * se sono a schermo intero in modalita' opengl, non
	 * e' necessario fare un SDL_SetMode visto che
	 * uso una texture ridimensionabile.
	 */
	if (gfx.opengl && setMode) {
		if ((gfx.fullscreen == FULLSCR) && (newFullscreen == gfx.fullscreen)) {
			/* se e' il primo avvio, non devo mai disabilitare il setMode */
			if (!gfx.onCfg) {
				setMode = FALSE;
			}
		}
	}
#endif

	/*
	 * gfx.scale e gfx.filter posso aggiornarli prima
	 * del setMode, mentre gfx.fullscreen e gfx.palette
	 * devo farlo necessariamente dopo.
	 */
	/* salvo il nuovo fattore di scala */
	gfx.scale = newScale;
	/* salvo ill nuovo filtro */
	gfx.filter = newFilter;

	/* devo eseguire un SDL_SetVideoMode? */
	if (setMode) {
		uint32_t flags = flagsSoftware;

		gfx.w[VIDEOMODE] = width;
		gfx.h[VIDEOMODE] = height;

#ifdef OPENGL
		if (gfx.opengl) {
			flags = opengl.flagsOpengl;

			//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
			//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
			/* abilito il doublebuffering */
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);
			/* abilito il vsync se e' necessario */
			SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, gfx.vsync);

			if (newFullscreen) {
				gfx.w[VIDEOMODE] = gfx.w[MONITOR];
				gfx.h[VIDEOMODE] = gfx.h[MONITOR];
			}
		}
#endif

		/* faccio quello che serve prima del setvideo */
		guiSetVideoMode();

		/*
		 * nella versione a 32 bit (GTK) dopo un gfxResetVideo,
		 * se non lo faccio anche qui, crasha tutto.
		 */
		sdlWid();

		/* inizializzo la superfice video */
		surfaceSDL = SDL_SetVideoMode(gfx.w[VIDEOMODE], gfx.h[VIDEOMODE], 0, flags);

		/* in caso di errore */
		if (!surfaceSDL) {
			fprintf(stderr, "SDL_SetVideoMode failed : %s\n", SDL_GetError());
			return;
		}

		gfx.bitperpixel = surfaceSDL->format->BitsPerPixel;
	}

	/* paletta */
	if (newPalette == NOCHANGE) {
		newPalette = gfx.palette;
	}
	if (newPalette != gfx.palette || gfx.onCfg) {
		switch (newPalette) {
			case PALETTEPAL:
				ntscSet(gfx.ntscFormat, FALSE, (BYTE *) paletteBasePAL, 0, (BYTE *) paletteRGB);
				break;
			case PALETTENTSC:
				ntscSet(gfx.ntscFormat, FALSE, 0, 0, (BYTE *) paletteRGB);
				break;
			case PALETTEGREEN:
				RGBMODIFIER(-0x20, 0x20, -0x20);
				break;
			default:
				ntscSet(gfx.ntscFormat, newPalette, 0, 0, (BYTE *) paletteRGB);
				break;
		}

		/* inizializzo in ogni caso la tabella YUV dell'hqx */
		hqxInit();

		/*
		 * memorizzo i colori della paletta nel
		 * formato di visualizzazione.
		 */
		{
			WORD i;

			for (i = 0; i < NCOLORS; i++) {
				paletteWindow[i] = SDL_MapRGBA(surfaceSDL->format, paletteRGB[i].r,
						paletteRGB[i].g, paletteRGB[i].b, 255);
			}
		}
	}

	/* salvo il nuovo stato del fullscreen */
	gfx.fullscreen = newFullscreen;
	/* salvo il nuovo tipo di paletta */
	gfx.palette = newPalette;

	/* software rendering */
	framebuffer = surfaceSDL;
	flip = SDL_Flip;

	wForPr = gfx.w[VIDEOMODE];
	hForPr = gfx.h[VIDEOMODE];

#ifdef OPENGL
	if (gfx.opengl) {

		if (!opengl.glew){
			GLenum err;

			if ((err = glewInit()) != GLEW_OK) {
				fprintf(stderr, "INFO: %s\n", glewGetErrorString(err));
			} else {
				opengl.glew = TRUE;

				if (opengl.glew && GLEW_VERSION_2_0) {
#ifndef RELEASE
					fprintf(stderr, "INFO: OpenGL 2.0 supported. Glsl enabled.\n");
#endif
					opengl.glsl = TRUE;
				}
			}
		}

		opengl.scale = gfx.scale;
		opengl.factor = 1;
		opengl.shader = SHADER_NONE;
		opengl.effect = effect;

		if (opengl.glsl) {
			switch (gfx.filter) {
				case NOFILTER:
					opengl.scale = X1;
					opengl.factor = gfx.scale;
					opengl.shader = SHADER_NOFILTER;
					opengl.effect = scaleSurface;
					break;
				case SCALE2X:
					opengl.scale = X1;
					opengl.factor = gfx.scale;
					opengl.shader = SHADER_SCALE2X;
					opengl.effect = scaleSurface;
					break;
				case SCALE3X:
					opengl.scale = X1;
					opengl.factor = gfx.scale;
					opengl.shader = SHADER_SCALE3X;
					opengl.effect = scaleSurface;
					break;
				case SCALE4X:
					opengl.scale = X2;
					opengl.factor = 2;
					opengl.shader = SHADER_SCALE2X;
					opengl.effect = scaleNx;
					break;
			}
		}

		/* creo la superficie che utilizzero' come texture */
		sdlCreateSurfaceGL(surfaceSDL, gfx.w[CURRENT], gfx.h[CURRENT], gfx.fullscreen);

		/* opengl rendering */
		framebuffer = opengl.surfaceGL;
		flip = opengl_flip;

		wForPr = opengl.xTexture2 - opengl.xTexture1;
		hForPr = opengl.yTexture2 - opengl.yTexture1;
	}
#endif

	textReset(framebuffer);

	/*
	 * calcolo le proporzioni tra il disegnato a video (overscan e schermo
	 * con le dimensioni per il filtro NTSC compresi) e quello che dovrebbe
	 * essere (256 x 240). Mi serve per calcolarmi la posizione del puntatore
	 * dello zapper.
	 */
	gfx.wPr = ((float) wForPr / gfx.w[CURRENT]) * ((float) gfx.w[NOOVERSCAN] / SCRROWS);
	gfx.hPr = ((float) hForPr / gfx.h[CURRENT]) * ((float) gfx.h[NOOVERSCAN] / SCRLINES);

	/* setto il titolo della finestra */
	guiUpdate();

	if (gfx.onCfg == TRUE) {
		gfx.onCfg = FALSE;
	}
}
void gfxDrawScreen(BYTE forced) {
	if (!forced && (info.no_rom || info.pause)) {
		if (++info.pause_frames_drawscreen == 4) {
			info.pause_frames_drawscreen = 0;
			forced = TRUE;
		} else {
			textRendering(FALSE, framebuffer);
			return;
		}
	}

	/* se il frameskip me lo permette (o se forzato), disegno lo screen */
	if (forced || !ppu.skipDraw) {
		/* applico l'effetto desiderato */
		if (gfx.opengl) {
			opengl.effect(screen.data, screen.line, paletteWindow, framebuffer, gfx.rows,
					gfx.lines, opengl.scale);

			textRendering(TRUE, opengl.surface_text);
			//textRendering(TRUE, framebuffer);

			opengl_draw_scene(framebuffer);
		} else {
			effect(screen.data, screen.line, paletteWindow, framebuffer, gfx.rows, gfx.lines,
					gfx.scale);

			textRendering(TRUE, surfaceSDL);
		}

		/* disegno a video */
		flip(framebuffer);
	}
}
void gfxResetVideo(void) {
	SDL_FreeSurface(surfaceSDL);
	surfaceSDL = framebuffer = NULL;

#ifdef OPENGL
	if (opengl.surfaceGL) {
		SDL_FreeSurface(opengl.surfaceGL);
	}
	if (opengl.texture.data) {
		glDeleteTextures(1, &opengl.texture.data);
	}
	opengl.surfaceGL = NULL;
#endif

	sdlWid();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
}
void gfxQuit(void) {
	if (paletteWindow) {
		free(paletteWindow);
	}
#ifdef OPENGL
	sdlQuitGL();
#endif

	ntscQuit();
	textQuit();
	SDL_Quit();
}

SDL_Surface *gfxCreateRGBSurface(SDL_Surface *src, uint32_t width, uint32_t height) {
	SDL_Surface *new_surface;

	new_surface = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(src->flags, width, height,
			src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask,
			src->format->Bmask, src->format->Amask));

	return (new_surface);
}

double sdlGetMs(void) {
	return (SDL_GetTicks());
}
void sdlNOP(double ms) {
	SDL_Delay(ms);
}
