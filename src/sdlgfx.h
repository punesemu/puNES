/*
 * sdl.h
 *
 *  Created on: 07/apr/2010
 *      Author: fhorse
 */

#ifndef SDLGFX_H_
#define SDLGFX_H_

#include "common.h"
#include "filters/video/scale.h"
#include "filters/video/scale2x.h"
#include "filters/video/hqx.h"
#include "filters/video/bilinear.h"
#include "filters/video/ntsc.h"

enum { RENDER_SOFTWARE, RENDER_OPENGL, RENDER_GLSL };
enum { X1 = 1, X2, X3, X4 };
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
enum { OSCAN_OFF, OSCAN_ON, OSCAN_DEFAULT, OSCAN_DEFAULT_OFF, OSCAN_DEFAULT_ON };
enum { CURRENT, NOOVERSCAN, MONITOR, VIDEOMODE };

#define NOCHANGE 255

struct _gfx {
	BYTE scale_before_fscreen;
	BYTE opengl;
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
void gfxSetRender(BYTE render);
void gfxSetScreen(BYTE newScale, BYTE newFilter, BYTE newFullscreen, BYTE newPalette,
        BYTE forceScale);
void gfxDrawScreen(BYTE forced);
void gfxResetVideo(void);
void gfxQuit(void);

SDL_Surface *gfxCreateRGBSurface(SDL_Surface *src, uint32_t width, uint32_t height);

double sdlGetMs(void);
void sdlNOP(double ms);

/* funzioni virtuali */
#define GFX_EFFECT_ROUTINE\
	void (*effect)(WORD *screen, WORD **screen_index, Uint32 *palette, SDL_Surface *dst, WORD rows,\
				WORD lines, BYTE factor)

GFX_EFFECT_ROUTINE;
int (*flip)(SDL_Surface *surface);

#endif /* SDLGFX_H_ */
