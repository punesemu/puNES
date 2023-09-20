/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *
 *  Originally taken from Fuse - the Free Unix Spectrum Emulator
 *  Copyright (C) 2003-2015 Fredrick Meunier, Philip Kendall, Gergely Szasz
 *
 *  Originally taken from ScummVM - Scumm Interpreter
 *  Copyright (C) 2001  Ludvig Strigeus
 *  Copyright (C) 2001-2003 The ScummVM project
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

#include <stdint.h>
#include "video/gfx.h"
#include "ppu.h"

#define SCALER_PALTV2X_SCANLINE
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#endif
#define RGB_TO_Y(r, g, b) ((2449L * (r) + 4809L * (g) +  934L * (b) + 1024) >> 11)
#define RGB_TO_U(r, g, b) ((4096L * (b) - 1383L * (r) - 2713L * (g) + 1024) >> 11)
#define RGB_TO_V(r, g, b) ((4096L * (r) - 3430L * (g) -  666L * (b) + 1024) >> 11)
#define YUV_TO_R(y, u, v) (MIN(ABS((8192L * (y)                + 11485L * (v) + 16384) >> 15), 255))
#define YUV_TO_G(y, u, v) (MIN(ABS((8192L * (y) -  2819L * (u) -  5850L * (v) + 16384) >> 15), 255))
#define YUV_TO_B(y, u, v) (MIN(ABS((8192L * (y) + 14516L * (u)                + 16384) >> 15), 255))
//#define RGB_TO_PIXEL_555(r,g,b) (((r * 125) >> 10) + (((g * 125) >> 5) & greenMask) + ((b * 125) & blueMask))
//#define RGB_TO_PIXEL_565(r,g,b) (((r * 125) >> 10) + (((g * 253) >> 5) & greenMask) + ((b * 249) & blueMask))
//#define R_TO_R(r) ((((r) & redMask) * 8424) >> 10)
//#define G_TO_G(g) (green6bit ? ((((g) & greenMask ) >> 5) * 4145) >> 10 : ((((g) & greenMask ) >> 5) * 8424) >> 10)
//#define B_TO_B(b) (green6bit ? (((b) & blueMask) >> 11) * 8424) >> 10 : ((((b) & blueMask) >> 10) * 8424) >> 10)

INLINE static void scaler_2xsai(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_dotmatrix(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_paltv1x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_paltv2x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_paltv3x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_paltv4x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_super2xsai(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_supereagle(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_tv2x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_tv3x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);
INLINE static void scaler_tv4x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette);

INLINE static int GetResult(uint32_t A, uint32_t B, uint32_t C, uint32_t D);
INLINE static uint32_t INTERPOLATE(uint32_t A, uint32_t B);
INLINE static uint32_t Q_INTERPOLATE(uint32_t A, uint32_t B, uint32_t C, uint32_t D);
INLINE static uint32_t DOT_16(uint32_t c, int j, int i);

static const uint32_t srcPitch = SCR_COLUMNS * sizeof(WORD);
static const uint32_t colorMask = 0x00FEFEFE;
static const uint32_t lowPixelMask = 0x00010101;
static const uint32_t qcolorMask = 0x00FCFCFC;
static const uint32_t qlowpixelMask = 0x00030303;
static const uint32_t redblueMask = 0x00FF00FF;
//static const uint32_t redblue8_Mask = 0x07F807F8;
//static const uint32_t redblue16_Mask = 0x0FF00FF0;
static const uint32_t redMask =   0x000000FF;
static const uint32_t greenMask = 0x0000FF00;
static const uint32_t blueMask =  0x00FF0000;
//static const uint32_t green8_Mask = 0x0007F800;
//static const uint32_t green16_Mask = 0x000FF000;
static const uint32_t dotmatrix[16] = {
	0x00003F00, 0x003F0000, 0x0000003F, 0x00000000,
	0x003F3F3F, 0x00000000, 0x003F3F3F, 0x00000000,
	0x0000003F, 0x00000000, 0x00003F00, 0x003F0000,
	0x003F3F3F, 0x00000000, 0x003F3F3F, 0x00000000
};

void scale_2xsai(void) {
	scaler_2xsai((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_dotmatrix(void) {
	scaler_dotmatrix((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_paltv1x(void) {
	scaler_paltv1x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_paltv2x(void) {
	scaler_paltv2x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_paltv3x(void) {
	scaler_paltv3x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_paltv4x(void) {
	scaler_paltv4x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_super2xsai(void) {
	scaler_super2xsai((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_supereagle(void) {
	scaler_supereagle((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_tv2x(void) {
	scaler_tv2x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_tv3x(void) {
	scaler_tv3x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}
void scale_tv4x(void) {
	scaler_tv4x((BYTE *)nes.p.ppu_screen.rd->data, (BYTE *)gfx.filter.data.pix, gfx.filter.data.pitch, SCR_COLUMNS, SCR_ROWS,
		(uint32_t *)gfx.filter.data.palette);
}

INLINE static void scaler_2xsai(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP;
	int i;

	while (height--) {
		bP = (const WORD *)srcPtr;
		dP = (uint32_t *)dstPtr;

		for (i = 0; i < width; ++i) {
			uint32_t colorA, colorB;
			uint32_t colorC, colorD, colorE, colorF, colorG, colorH, colorI, colorJ, colorK, colorL, colorM, colorN, colorO;
			uint32_t product, product1, product2;

			colorI = palette[(*(bP - nextlineSrc - 1))];
			colorE = palette[(*(bP - nextlineSrc))];
			colorF = palette[(*(bP - nextlineSrc + 1))];
			colorJ = palette[(*(bP - nextlineSrc + 2))];

			colorG = palette[(*(bP - 1))];
			colorA = palette[(*(bP))];
			colorB = palette[(*(bP + 1))];
			colorK = palette[(*(bP + 2))];

			colorH = palette[(*(bP + nextlineSrc - 1))];
			colorC = palette[(*(bP + nextlineSrc))];
			colorD = palette[(*(bP + nextlineSrc + 1))];
			colorL = palette[(*(bP + nextlineSrc + 2))];

			colorM = palette[(*(bP + 2 * nextlineSrc - 1))];
			colorN = palette[(*(bP + 2 * nextlineSrc))];
			colorO = palette[(*(bP + 2 * nextlineSrc + 1))];

			if ((colorA == colorD) && (colorB != colorC)) {
				if (((colorA == colorE) && (colorB == colorL)) ||
					((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))) {
					product = colorA;
				} else {
					product = INTERPOLATE(colorA, colorB);
				}

				if (((colorA == colorG) && (colorC == colorO)) ||
					((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))) {
					product1 = colorA;
				} else {
					product1 = INTERPOLATE(colorA, colorC);
				}
				product2 = colorA;
			} else if ((colorB == colorC) && (colorA != colorD)) {
				if (((colorB == colorF) && (colorA == colorH)) ||
					((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))) {
					product = colorB;
				} else {
					product = INTERPOLATE(colorA, colorB);
				}

				if (((colorC == colorH) && (colorA == colorF)) ||
					((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))) {
					product1 = colorC;
				} else {
					product1 = INTERPOLATE(colorA, colorC);
				}
				product2 = colorB;
			} else if ((colorA == colorD) && (colorB == colorC)) {
				if (colorA == colorB) {
					product = colorA;
					product1 = colorA;
					product2 = colorA;
				} else {
					int r = 0;

					product1 = INTERPOLATE(colorA, colorC);
					product = INTERPOLATE(colorA, colorB);

					r += GetResult(colorA, colorB, colorG, colorE);
					r -= GetResult(colorB, colorA, colorK, colorF);
					r -= GetResult(colorB, colorA, colorH, colorN);
					r += GetResult(colorA, colorB, colorL, colorO);

					if (r > 0) {
						product2 = colorA;
					} else if (r < 0) {
						product2 = colorB;
					} else {
						product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
					}
				}
			} else {
				product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);

				if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) {
					product = colorA;
				} else if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) {
					product = colorB;
				} else {
					product = INTERPOLATE(colorA, colorB);
				}

				if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) {
					product1 = colorA;
				} else if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) {
					product1 = colorC;
				} else {
					product1 = INTERPOLATE(colorA, colorC);
				}
			}

			(*dP) = colorA;
			(*(dP + nextlineDst)) = product1;
			dP++;
			(*dP) = product;
			(*(dP + nextlineDst)) = product2;
			dP++;

			bP++;
		}
		srcPtr += srcPitch;
		dstPtr += (dstPitch << 1);
	}
}
INLINE static void scaler_dotmatrix(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP = (uint32_t *)dstPtr;
	int i, j, ii, jj;

	for (j = 0, jj = 0; j < height; ++j, jj += 2) {
		for (i = 0, ii = 0; i < width; ++i, ii += 2) {
			uint32_t c = palette[(*(bP + i))];

			(*(dP + ii)) = DOT_16(c, jj, ii);
			(*(dP + ii + 1)) = DOT_16(c, jj, ii + 1);
			(*(dP + ii + nextlineDst)) = DOT_16(c, jj + 1, ii);
			(*(dP + ii + nextlineDst + 1)) = DOT_16(c, jj + 1, ii + 1);
		}
		bP += nextlineSrc;
		dP += (nextlineDst << 1);
	}
}
INLINE static void scaler_paltv1x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP, *bP0 = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP, *dP0 = (uint32_t *)dstPtr;
	BYTE r0, g0, b0, r1, g1, b1, r2, g2, b2, r3, g3, b3;
	int32_t y1, y2, u1, u2, v1, v2;
	int i, j;

	for (j = height; j; j--) {
		bP = bP0 - 1;
		dP = dP0;

		r2 = (palette[(*bP)] & redMask);
		g2 = (palette[(*bP)] & greenMask) >> 8;
		b2 = (palette[(*bP)] & blueMask) >> 16;

		bP++;
		r0 = (palette[(*bP)] & redMask);
		g0 = (palette[(*bP)] & greenMask) >> 8;
		b0 = (palette[(*bP)] & blueMask) >> 16;

		bP++;
		r1 = (palette[(*bP)] & redMask);
		g1 = (palette[(*bP)] & greenMask) >> 8;
		b1 = (palette[(*bP)] & blueMask) >> 16;

		bP++;
		u1 = (int32_t)((RGB_TO_U(r2, g2, b2) + 2 * RGB_TO_U(r0, g0, b0) + RGB_TO_U(r1, g1, b1)) >> 2);
		v1 = (int32_t)((RGB_TO_V(r2, g2, b2) + 2 * RGB_TO_V(r0, g0, b0) + RGB_TO_V(r1, g1, b1)) >> 2);

		for (i = width; i; i -= 2) {
			r2 = (palette[(*bP)] & redMask);
			g2 = (palette[(*bP)] & greenMask) >> 8;
			b2 = (palette[(*bP)] & blueMask) >> 16;

			bP++;
			r3 = (palette[(*bP)] & redMask);
			g3 = (palette[(*bP)] & greenMask) >> 8;
			b3 = (palette[(*bP)] & blueMask) >> 16;

			bP++;
			y1 = RGB_TO_Y(r0, g0, b0);
			y2 = RGB_TO_Y(r1, g1, b1);

			u2 = (int32_t)(RGB_TO_U( r1, g1, b1) + 2 * RGB_TO_U(r2, g2, b2) + RGB_TO_U(r3, g3, b3)) >> 2;
			v2 = (int32_t)(RGB_TO_V( r1, g1, b1) + 2 * RGB_TO_V(r2, g2, b2) + RGB_TO_V(r3, g3, b3)) >> 2;

			r0 = YUV_TO_R(y1, u1, v1);
			g0 = YUV_TO_G(y1, u1, v1);
			b0 = YUV_TO_B(y1, u1, v1);

			u1 = (u1 + u2) >> 1;
			v1 = (v1 + v2) >> 1;

			r1 = YUV_TO_R(y2, u1, v1);
			g1 = YUV_TO_G(y2, u1, v1);
			b1 = YUV_TO_B(y2, u1, v1);

			*dP++ = r0 + (g0 << 8) + (b0 << 16);
			*dP++ = r1 + (g1 << 8) + (b1 << 16);

			u1 = u2;
			v1 = v2;
			r0 = r2;
			g0 = g2;
			b0 = b2;
			r1 = r3;
			g1 = g3;
			b1 = b3;
		}
		bP0 += nextlineSrc;
		dP0 += nextlineDst;
	}
}
INLINE static void scaler_paltv2x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP, *bP0 = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP, *dP0 = (uint32_t *)dstPtr;
	BYTE r0, g0, b0, r1, g1, b1, rx, gx, bx;
	int32_t y1, y2, u1, v1, u2, v2;
	int i, j;

	for (j = height; j; j--) {
		bP = bP0 - 1;
		dP = dP0;

		r0 = (palette[(*bP)] & redMask);
		g0 = (palette[(*bP)] & greenMask) >> 8;
		b0 = (palette[(*bP)] & blueMask) >> 16;

		bP++;
		r1 = (palette[(*bP)] & redMask);
		g1 = (palette[(*bP)] & greenMask) >> 8;
		b1 = (palette[(*bP)] & blueMask) >> 16;

		y1 = RGB_TO_Y(r1, g1, b1);
		u1 = (int32_t)(RGB_TO_U(r0, g0, b0) + 3 * RGB_TO_U(r1, g1, b1)) >> 2;
		v1 = (int32_t)(RGB_TO_V(r0, g0, b0) + 3 * RGB_TO_V(r1, g1, b1)) >> 2;

		for (i = width; i; i--) {
			bP++;
			r0 = (palette[(*bP)] & redMask);
			g0 = (palette[(*bP)] & greenMask) >> 8;
			b0 = (palette[(*bP)] & blueMask) >> 16;

			y2 = RGB_TO_Y(r0, g0, b0);
			u2 = (int32_t)(RGB_TO_U(r1, g1, b1) + 3 * RGB_TO_U(r0, g0, b0)) >> 2;
			v2 = (int32_t)(RGB_TO_V(r1, g1, b1) + 3 * RGB_TO_V(r0, g0, b0)) >> 2;

			rx = YUV_TO_R(y1, u1, v1);
			gx = YUV_TO_G(y1, u1, v1);
			bx = YUV_TO_B(y1, u1, v1);

			u1 = (u1 + u2) >> 1;
			v1 = (v1 + v2) >> 1;

			r1 = YUV_TO_R(y1, u1, v1);
			g1 = YUV_TO_G(y1, u1, v1);
			b1 = YUV_TO_B(y1, u1, v1);

			(*dP) = rx + (gx << 8) + (bx << 16);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + nextlineDst)) = (((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + nextlineDst)) = (*dP);
#endif

			dP++;
			(*dP) = r1 + (g1 << 8) + (b1 << 16);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + nextlineDst)) = (((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + nextlineDst)) = (*dP);
#endif

			dP++;

			y1 = y2;
			u1 = u2;
			v1 = v2;
			r1 = r0;
			g1 = g0;
			b1 = b0;
		}
		bP0 += nextlineSrc;
		dP0 += (nextlineDst << 1);
	}
}
INLINE static void scaler_paltv3x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP, *bP0 = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP, *dP0 = (uint32_t *)dstPtr;
	BYTE r0, g0, b0, r1, g1, b1, r2, g2, b2, rx, gx, bx;
	int32_t y1, y2, u1, v1, u2, v2;
	int i, j;

	for (j = height; j; j--) {
		bP = bP0 - 1;
		dP = dP0;

		r0 = (palette[(*bP)] & redMask);
		g0 = (palette[(*bP)] & greenMask) >> 8;
		b0 = (palette[(*bP)] & blueMask) >> 16;

		bP++;
		r1 = (palette[(*bP)] & redMask);
		g1 = (palette[(*bP)] & greenMask) >> 8;
		b1 = (palette[(*bP)] & blueMask) >> 16;

		y1 = RGB_TO_Y(r1, g1, b1);
		u1 = (int32_t)(RGB_TO_U(r0, g0, b0) + 3 * RGB_TO_U(r1, g1, b1)) >> 2;
		v1 = (int32_t)(RGB_TO_V(r0, g0, b0) + 3 * RGB_TO_V(r1, g1, b1)) >> 2;

		for (i = width; i; i--) {
			bP++;
			r0 = (palette[(*bP)] & redMask);
			g0 = (palette[(*bP)] & greenMask) >> 8;
			b0 = (palette[(*bP)] & blueMask) >> 16;

			y2 = RGB_TO_Y(r0, g0, b0);
			u2 = (int32_t)(RGB_TO_U(r1, g1, b1) + 3 * RGB_TO_U(r0, g0, b0)) >> 2;
			v2 = (int32_t)(RGB_TO_V(r1, g1, b1) + 3 * RGB_TO_V(r0, g0, b0)) >> 2;

			rx = YUV_TO_R(y1, u1, v1);
			gx = YUV_TO_G(y1, u1, v1);
			bx = YUV_TO_B(y1, u1, v1);

			u1 = (u1 + u2) >> 1;
			v1 = (v1 + v2) >> 1;

			r1 = YUV_TO_R(y1, u1, v1);
			g1 = YUV_TO_G(y1, u1, v1);
			b1 = YUV_TO_B(y1, u1, v1);

			r2 = ((int)rx + r1) >> 1;
			g2 = ((int)gx + g1) >> 1;
			b2 = ((int)bx + b1) >> 1;

			(*dP) = rx + (gx << 8) + (bx << 16);
			(*(dP + nextlineDst)) = (*dP);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + (nextlineDst << 1))) = (((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + (nextlineDst << 1))) = (*dP);
#endif

			dP++;
			(*dP) = r2 + (g2 << 8) + (b2 << 16);
			(*(dP + nextlineDst)) = (*dP);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + (nextlineDst << 1))) = (((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + (nextlineDst << 1))) = (*dP);
#endif

			dP++;
			(*dP) = r1 + (g1 << 8) + (b1 << 16);
			(*(dP + nextlineDst)) = (*dP);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + (nextlineDst << 1))) = (((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + (nextlineDst << 1))) = (*dP);
#endif

			dP++;

			y1 = y2;
			u1 = u2;
			v1 = v2;
			r1 = r0;
			g1 = g0;
			b1 = b0;
		}
		bP0 += nextlineSrc;
		dP0 += (nextlineDst << 1) + nextlineDst;
	}
}
INLINE static void scaler_paltv4x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP, *bP0 = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP, *dP0 = (uint32_t *)dstPtr;
	BYTE r0, g0, b0, r1, g1, b1, rx, gx, bx;
	int32_t y1, y2, u1, v1, u2, v2;
	int i, j;

	for (j = height; j; j--) {
		bP = bP0 - 1;
		dP = dP0;

		r0 = (palette[(*bP)] & redMask);
		g0 = (palette[(*bP)] & greenMask) >> 8;
		b0 = (palette[(*bP)] & blueMask) >> 16;

		bP++;
		r1 = (palette[(*bP)] & redMask);
		g1 = (palette[(*bP)] & greenMask) >> 8;
		b1 = (palette[(*bP)] & blueMask) >> 16;

		y1 = RGB_TO_Y(r1, g1, b1);
		u1 = (int32_t)(RGB_TO_U(r0, g0, b0) + 3 * RGB_TO_U(r1, g1, b1)) >> 2;
		v1 = (int32_t)(RGB_TO_V(r0, g0, b0) + 3 * RGB_TO_V(r1, g1, b1)) >> 2;

		for (i = width; i; i--) {
			bP++;
			r0 = (palette[(*bP)] & redMask);
			g0 = (palette[(*bP)] & greenMask) >> 8;
			b0 = (palette[(*bP)] & blueMask) >> 16;

			y2 = RGB_TO_Y(r0, g0, b0);
			u2 = (int32_t)(RGB_TO_U(r1, g1, b1) + 3 * RGB_TO_U(r0, g0, b0)) >> 2;
			v2 = (int32_t)(RGB_TO_V(r1, g1, b1) + 3 * RGB_TO_V(r0, g0, b0)) >> 2;

			rx = YUV_TO_R(y1, u1, v1);
			gx = YUV_TO_G(y1, u1, v1);
			bx = YUV_TO_B(y1, u1, v1);

			u1 = (u1 + u2) >> 1;
			v1 = (v1 + v2) >> 1;

			r1 = YUV_TO_R(y1, u1, v1);
			g1 = YUV_TO_G(y1, u1, v1);
			b1 = YUV_TO_B(y1, u1, v1);

			(*dP) = (*(dP + 1)) = rx + (gx << 8) + (bx << 16);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + nextlineDst)) =
			(*(dP + nextlineDst + 1)) =
			(*(dP + 2 * nextlineDst)) =
			(*(dP + 2 * nextlineDst + 1)) =
			(*(dP + 3 * nextlineDst)) =
			(*(dP + 3 * nextlineDst + 1)) =
				(((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + nextlineDst)) =
			(*(dP + nextlineDst + 1)) =
			(*(dP + 2 * nextlineDst)) =
			(*(dP + 2 * nextlineDst + 1)) =
			(*(dP + 3 * nextlineDst)) =
			(*(dP + 3 * nextlineDst + 1)) = (*dP);
#endif

			dP++;
			dP++;
			(*dP) = (*(dP + 1)) = r1 + (g1 << 8) + (b1 << 16);
#if defined (SCALER_PALTV2X_SCANLINE)
			(*(dP + nextlineDst)) =
			(*(dP + nextlineDst + 1)) =
			(*(dP + 2 * nextlineDst)) =
			(*(dP + 2 * nextlineDst + 1)) =
			(*(dP + 3 * nextlineDst)) =
			(*(dP + 3 * nextlineDst + 1)) =
				(((((*dP) & redblueMask) * 7) >> 3) & redblueMask) | (((((*dP) & greenMask) * 7) >> 3) & greenMask);
#else
			(*(dP + nextlineDst)) =
			(*(dP + nextlineDst + 1)) =
			(*(dP + 2 * nextlineDst)) =
			(*(dP + 2 * nextlineDst + 1)) =
			(*(dP + 3 * nextlineDst)) =
			(*(dP + 3 * nextlineDst + 1)) = (*dP);
#endif

			dP++;
			dP++;

			y1 = y2;
			u1 = u2;
			v1 = v2;
			r1 = r0;
			g1 = g0;
			b1 = b0;
		}
		bP0 += nextlineSrc;
		dP0 += (nextlineDst << 2);
	}
}
INLINE static void scaler_super2xsai(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP;
	int i;

	while (height--) {
		bP = (const WORD *)srcPtr;
		dP = (uint32_t *)dstPtr;

		for (i = 0; i < width; ++i) {
			uint32_t color4, color5, color6;
			uint32_t color1, color2, color3;
			uint32_t colorA0, colorA1, colorA2, colorA3, colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
			uint32_t product1a, product1b, product2a, product2b;

			colorB0 = palette[(*(bP - nextlineSrc - 1))];
			colorB1 = palette[(*(bP - nextlineSrc))];
			colorB2 = palette[(*(bP - nextlineSrc + 1))];
			colorB3 = palette[(*(bP - nextlineSrc + 2))];

			color4 = palette[(*(bP - 1))];
			color5 = palette[(*(bP))];
			color6 = palette[(*(bP + 1))];
			colorS2 = palette[(*(bP + 2))];

			color1 = palette[(*(bP + nextlineSrc - 1))];
			color2 = palette[(*(bP + nextlineSrc))];
			color3 = palette[(*(bP + nextlineSrc + 1))];
			colorS1 = palette[(*(bP + nextlineSrc + 2))];

			colorA0 = palette[(*(bP + 2 * nextlineSrc - 1))];
			colorA1 = palette[(*(bP + 2 * nextlineSrc))];
			colorA2 = palette[(*(bP + 2 * nextlineSrc + 1))];
			colorA3 = palette[(*(bP + 2 * nextlineSrc + 2))];

			if (color2 == color6 && color5 != color3) {
				product2b = product1b = color2;
			} else if (color5 == color3 && color2 != color6) {
				product2b = product1b = color5;
			} else if (color5 == color3 && color2 == color6) {
				int r = 0;

				r += GetResult(color6, color5, color1, colorA1);
				r += GetResult(color6, color5, color4, colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0) {
					product2b = product1b = color6;
				} else if (r < 0) {
					product2b = product1b = color5;
				} else {
					product2b = product1b = INTERPOLATE(color5, color6);
				}
			} else {
				if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0) {
					product2b = Q_INTERPOLATE(color3, color3, color3, color2);
				} else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3) {
					product2b = Q_INTERPOLATE(color2, color2, color2, color3);
				} else {
					product2b = INTERPOLATE(color2, color3);
				}

				if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0) {
					product1b = Q_INTERPOLATE(color6, color6, color6, color5);
				} else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3) {
					product1b = Q_INTERPOLATE(color6, color5, color5, color5);
				} else {
					product1b = INTERPOLATE(color5, color6);
				}
			}

			if ((color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2) ||
				(color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)) {
				product2a = INTERPOLATE(color2, color5);
			} else {
				product2a = color2;
			}

			if ((color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2) ||
				(color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)) {
				product1a = INTERPOLATE(color2, color5);
			} else {
				product1a = color5;
			}

			(*dP) = product1a;
			(*(dP + nextlineDst)) = product2a;
			dP++;
			(*dP) = product1b;
			(*(dP + nextlineDst)) = product2b;
			dP++;

			bP++;
		}
		srcPtr += srcPitch;
		dstPtr += (dstPitch << 1);
	}

}
INLINE static void scaler_supereagle(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP;
	int i;

	while (height--) {
		bP = (const WORD *)srcPtr;
		dP = (uint32_t *)dstPtr;

		for (i = 0; i < width; ++i) {
			uint32_t color4, color5, color6;
			uint32_t color1, color2, color3;
			uint32_t colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
			uint32_t product1a, product1b, product2a, product2b;

			colorB1 = palette[(*(bP - nextlineSrc))];
			colorB2 = palette[(*(bP - nextlineSrc + 1))];

			color4 = palette[(*(bP - 1))];
			color5 = palette[(*(bP))];
			color6 = palette[(*(bP + 1))];
			colorS2 = palette[(*(bP + 2))];

			color1 = palette[(*(bP + nextlineSrc - 1))];
			color2 = palette[(*(bP + nextlineSrc))];
			color3 = palette[(*(bP + nextlineSrc + 1))];
			colorS1 = palette[(*(bP + nextlineSrc + 2))];

			colorA1 = palette[(*(bP + 2 * nextlineSrc))];
			colorA2 = palette[(*(bP + 2 * nextlineSrc + 1))];

			/* -------------------------------------- */
			if (color5 != color3) {
				if (color2 == color6) {
					product1b = product2a = color2;
					if ((color1 == color2) || (color6 == colorB2)) {
						product1a = INTERPOLATE(color2, color5);
						product1a = INTERPOLATE(color2, product1a);
					} else {
						product1a = INTERPOLATE(color5, color6);
					}

					if ((color6 == colorS2) || (color2 == colorA1)) {
						product2b = INTERPOLATE(color2, color3);
						product2b = INTERPOLATE(color2, product2b);
					} else {
						product2b = INTERPOLATE(color2, color3);
					}
				} else {
					product2b = product1a = INTERPOLATE(color2, color6);
					product2b = Q_INTERPOLATE(color3, color3, color3, product2b);
					product1a = Q_INTERPOLATE(color5, color5, color5, product1a);

					product2a = product1b = INTERPOLATE(color5, color3);
					product2a = Q_INTERPOLATE(color2, color2, color2, product2a);
					product1b = Q_INTERPOLATE(color6, color6, color6, product1b);
				}
			} else /*if (color5 == color3) */ {
				if (color2 != color6) {
					product2b = product1a = color5;

					if ((colorB1 == color5) || (color3 == colorS1)) {
						product1b = INTERPOLATE(color5, color6);
						product1b = INTERPOLATE(color5, product1b);
					} else {
						product1b = INTERPOLATE(color5, color6);
					}

					if ((color3 == colorA2) || (color4 == color5)) {
						product2a = INTERPOLATE(color5, color2);
						product2a = INTERPOLATE(color5, product2a);
					} else {
						product2a = INTERPOLATE(color2, color3);
					}

				} else /* if (color2 != color6) */ {
					int r = 0;

					r += GetResult(color6, color5, color1, colorA1);
					r += GetResult(color6, color5, color4, colorB1);
					r += GetResult(color6, color5, colorA2, colorS1);
					r += GetResult(color6, color5, colorB2, colorS2);

					if (r > 0) {
						product1b = product2a = color2;
						product1a = product2b = INTERPOLATE(color5, color6);
					} else if (r < 0) {
						product2b = product1a = color5;
						product1b = product2a = INTERPOLATE(color5, color6);
					} else {
						product2b = product1a = color5;
						product1b = product2a = color2;
					}
				}
			}

			(*dP) = product1a;
			(*(dP + nextlineDst)) = product2a;
			dP++;
			(*dP) = product1b;
			(*(dP + nextlineDst)) = product2b;
			dP++;

			bP++;
		}
		srcPtr += srcPitch;
		dstPtr += (dstPitch << 1);
	}
}
INLINE static void scaler_tv2x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP = (uint32_t *)dstPtr;
	int i, j;

	while (height--) {
		for (i = 0, j = 0; i < width; ++i, j += 2) {
			uint32_t p1 = palette[(*(bP + i))];
			uint32_t pi;

			pi = (((p1 & redblueMask) * 7) >> 3) & redblueMask;
			pi |= (((p1 & greenMask) * 7) >> 3) & greenMask;

			(*(dP + j)) = p1;
			(*(dP + j + 1)) = p1;
			(*(dP + j + nextlineDst)) = pi;
			(*(dP + j + nextlineDst + 1)) = pi;
		}
		bP += nextlineSrc;
		dP += (nextlineDst << 1);
	}
}
INLINE static void scaler_tv3x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP = (uint32_t *)dstPtr;
	int i, j;

	while (height--) {
		for (i = 0, j = 0; i < width; ++i, j += 3) {
			uint32_t p1 = palette[(*(bP + i))];
			uint32_t pi;

			pi = (((p1 & redblueMask) * 7) >> 3) & redblueMask;
			pi |= (((p1 & greenMask) * 7) >> 3) & greenMask;

			(*(dP + j)) = p1;
			(*(dP + j + 1)) = p1;
			(*(dP + j + 2)) = p1;

			(*(dP + j + nextlineDst)) = p1;
			(*(dP + j + nextlineDst + 1)) = p1;
			(*(dP + j + nextlineDst + 2)) = p1;

			(*(dP + j + (nextlineDst << 1))) = pi;
			(*(dP + j + (nextlineDst << 1) + 1)) = pi;
			(*(dP + j + (nextlineDst << 1) + 2)) = pi;
		}
		bP += nextlineSrc;
		dP += (nextlineDst * 3);
	}
}
INLINE static void scaler_tv4x(const BYTE *srcPtr, BYTE *dstPtr, uint32_t dstPitch, int width, int height, const uint32_t *palette) {
	WORD nextlineSrc = srcPitch / sizeof(WORD);
	const WORD *bP = (const WORD *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(uint32_t);
	uint32_t *dP = (uint32_t *)dstPtr;
	int i, j;

	while (height--) {
		for (i = 0, j = 0; i < width; ++i, j += 4) {
			uint32_t p1 = palette[(*(bP + i))];
			uint32_t pi;

			pi = (((p1 & redblueMask) * 7) >> 3) & redblueMask;
			pi |= (((p1 & greenMask) * 7) >> 3) & greenMask;

			(*(dP + j)) = p1;
			(*(dP + j + 1)) = p1;
			(*(dP + j + 2)) = p1;
			(*(dP + j + 3)) = p1;

			(*(dP + j + nextlineDst)) = p1;
			(*(dP + j + nextlineDst + 1)) = p1;
			(*(dP + j + nextlineDst + 2)) = p1;
			(*(dP + j + nextlineDst + 3)) = p1;

			(*(dP + j + (nextlineDst << 1))) = pi;
			(*(dP + j + (nextlineDst << 1) + 1)) = pi;
			(*(dP + j + (nextlineDst << 1) + 2)) = pi;
			(*(dP + j + (nextlineDst << 1) + 3)) = pi;

			(*(dP + j + (nextlineDst * 3))) = pi;
			(*(dP + j + (nextlineDst * 3) + 1)) = pi;
			(*(dP + j + (nextlineDst * 3) + 2)) = pi;
			(*(dP + j + (nextlineDst * 3) + 3)) = pi;
		}
		bP += nextlineSrc;
		dP += (nextlineDst << 2);
	}
}

INLINE static int GetResult(uint32_t A, uint32_t B, uint32_t C, uint32_t D) {
	const int ac = (A == C);
	const int bc = (B == C);
	const int x1 = ac;
	const int y1 = (bc & !ac);
	const int ad = (A == D);
	const int bd = (B == D);
	const int x2 = ad;
	const int y2 = (bd & !ad);
	const int x = x1 + x2;
	const int y = y1 + y2;
	static const int rmap[3][3] = {
		{ 0, 0, -1 },
		{ 0, 0, -1 },
		{ 1, 1,  0 }
	};

	return (rmap[y][x]);
}
INLINE static uint32_t INTERPOLATE(uint32_t A, uint32_t B) {
	if (A != B) {
		return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask));
	} else {
		return (A);
	}
}
INLINE static uint32_t Q_INTERPOLATE(uint32_t A, uint32_t B, uint32_t C, uint32_t D) {
	uint32_t x = ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	uint32_t y = (A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

	y = (y >> 2) & qlowpixelMask;
	return (x + y);
}
INLINE static uint32_t DOT_16(uint32_t c, int j, int i) {
	return (c - ((c >> 2) & *(dotmatrix + ((j & 3) << 2) + (i & 3))));
}
