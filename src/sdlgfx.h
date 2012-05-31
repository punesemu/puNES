/*
 * sdl.h
 *
 *  Created on: 07/apr/2010
 *      Author: fhorse
 */

#ifndef SDLGFX_H_
#define SDLGFX_H_

#include "common.h"
#include "filters/scale.h"
#include "filters/scale2x.h"
#include "filters/hqx.h"
#include "filters/bilinear.h"
#include "filters/ntsc.h"

enum {
	X1 = 1,
	X2,
	X3,
	X4 };
enum {
	NOFILTER,
	SCALE2X,
	SCALE3X,
	SCALE4X,
	HQ2X,
	HQ3X,
	HQ4X,
	RGBNTSC,
	BILINEAR,
	/* glsl shaders */
	POSPHOR,
	SCANLINE,
	DBL,
	CRTCURVE,
	CRTNOCURVE,
	/* glsl shaders end */
};
enum { NOFULLSCR, FULLSCR };
enum {
	OSCANOFF,
	OSCANON,
	OSCANDEF,
	OSCANDEFAULTOFF,
	OSCANDEFAULTON
};
enum { CURRENT, NOOVERSCAN, MONITOR, VIDEOMODE };

#define NOCHANGE   255

struct _gfx {
	BYTE scale;
	BYTE scaleBeforeFullscreen;
	BYTE overscan;
	BYTE overscanDefault;
	BYTE filter;
	BYTE palette;
	BYTE ntscFormat;
	BYTE onCfg;
	BYTE fullscreen;
	BYTE opengl;
	BYTE vsync;
	BYTE bitperpixel;
	WORD rows;
	WORD lines;
	SDBWORD w[4];
	SDBWORD h[4];
	float wPr;
	float hPr;
} gfx;

SDL_Surface *surfaceSDL;

BYTE gfxInit(void);
void gfxSetScreen(BYTE newScale, BYTE newFilter, BYTE newFullscreen, BYTE newPalette,
        BYTE forceScale);
void gfxDrawScreen(BYTE forced);
void gfxResetVideo(void);
void gfxQuit(void);

SDL_Surface *gfxCreateRGBSurface(SDL_Surface *src, uint32_t width, uint32_t height);

double sdlGetMs(void);
void sdlNOP(double ms);

/* funzioni virtuali */
#define GFX_EFFECT_ROUTINE \
	void (*effect)(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,\
	WORD lines, BYTE factor);

GFX_EFFECT_ROUTINE
int (*flip)(SDL_Surface *surface);

#endif /* SDLGFX_H_ */
