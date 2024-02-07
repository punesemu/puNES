/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef DRAW_ON_SCREEN_H_
#define DRAW_ON_SCREEN_H_

#include "common.h"
enum _dos_text_curtain_modes {
	DOS_TEXT_CURTAIN_INIT,
	DOS_TEXT_CURTAIN_TICK,
	DOS_TEXT_CURTAIN_QUIT
};
enum _dos_color {
	DOS_GY01 = 0x002D, DOS_GY02 = 0x0000, DOS_GY03 = 0x0010, DOS_GY04 = 0x003D,
	DOS_BL01 = 0x0001, DOS_BL02 = 0x0011, DOS_BL03 = 0x0021, DOS_BL04 = 0x0031,
	DOS_RB01 = 0x0002, DOS_RB02 = 0x0012, DOS_RB03 = 0x0022, DOS_RB04 = 0x0032,
	DOS_IN01 = 0x0003, DOS_IN02 = 0x0013, DOS_IN03 = 0x0023, DOS_IN04 = 0x0033,
	DOS_PU01 = 0x0004, DOS_PU02 = 0x0014, DOS_PU03 = 0x0024, DOS_PU04 = 0x0034,
	DOS_MA01 = 0x0005, DOS_MA02 = 0x0015, DOS_MA03 = 0x0025, DOS_MA04 = 0x0035,
	DOS_PI01 = 0x0006, DOS_PI02 = 0x0016, DOS_PI03 = 0x0026, DOS_PI04 = 0x0036,
	DOS_BR01 = 0x0007, DOS_BR02 = 0x0017, DOS_BR03 = 0x0027, DOS_BR04 = 0x0037,
	DOS_OL01 = 0x0008, DOS_OL02 = 0x0018, DOS_OL03 = 0x0028, DOS_OL04 = 0x0038,
	DOS_PE01 = 0x0009, DOS_PE02 = 0x0019, DOS_PE03 = 0x0029, DOS_PE04 = 0x0039,
	DOS_GR01 = 0x000A, DOS_GR02 = 0x001A, DOS_GR03 = 0x002A, DOS_GR04 = 0x003A,
	DOS_TL01 = 0x000B, DOS_TL02 = 0x001B, DOS_TL03 = 0x002B, DOS_TL04 = 0x003B,
	DOS_CY01 = 0x000C, DOS_CY02 = 0x001C, DOS_CY03 = 0x002C, DOS_CY04 = 0x003C,
	DOS_WHITE = 0x0030,
	DOS_BLACK = 0x000D,
	DOS_NORMAL = DOS_WHITE,
	DOS_RED = DOS_PI03,
	DOS_YELLOW = DOS_OL04,
	DOS_GREEN = DOS_GR03,
	DOS_CYAN = DOS_CY03,
	DOS_BROWN = DOS_BR03,
	DOS_BLUE = DOS_BL02,
	DOS_GRAY = DOS_GY02,
	DOS_NONE = 0xFFFF,
	DOS_TRASPARENT = DOS_NONE - 1,
	DOS_BCK = DOS_TRASPARENT - 1,
	DOS_IMAGE = DOS_BCK - 1,
	DOS_ENDIMAGE = DOS_IMAGE - 1,
	DOS_ALIGNTOP = DOS_ENDIMAGE - 1,
	DOS_ALIGNBOTTOM = DOS_ALIGNTOP - 1,
	DOS_ALIGNVCENTER = DOS_ALIGNBOTTOM - 1,
	DOS_ALIGNLEFT = DOS_ALIGNVCENTER - 1,
	DOS_ALIGNRIGHT = DOS_ALIGNLEFT - 1,
	DOS_ALIGNHCENTER = DOS_ALIGNRIGHT - 1
};
#define dclr(clr) dos_tag_desc_from_value(clr)

typedef struct _dos_text_ppu_image {
	WORD *data;
	int w, h;
} _dos_text_ppu_image;
typedef struct _dos_rect {
	int w, h;
	int x, y;
} _dos_rect;
typedef struct _dos_text_scroll {
	_dos_rect rect;
	_dos_text_ppu_image pimage;
	double timer;
	double reload;
	int velocity;
} _dos_text_scroll;
typedef struct _dos_text_curtain {
	_dos_rect image;
	int h;
	int count;
	int index;
	BYTE pause;
	double timer;
	_dos_text_ppu_image *line;
	struct _dos_text_curtain_reload {
		double r1;
		double r2;
	} reload;
	struct _dos_text_curtain_redraw {
		BYTE all;
	} redraw;
} _dos_text_curtain;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void dos_text(BYTE nidx, int ppu_x, int ppu_y, int img_x, int img_y, int img_w, int img_h,
	const WORD fg_def, const WORD bg_def, const uTCHAR *font_family, const int font_size, const uTCHAR *fmt, ...);

EXTERNC WORD *dos_text_to_ppu_image(int rect_x, int rect_y, int rect_w, int rect_h, const WORD fg_def, const WORD bg_def,
	const uTCHAR *font_family, const int font_size, const uTCHAR *fmt, ...);

EXTERNC void dos_text_scroll_tick(BYTE nidx, int ppu_x, int ppu_y, const WORD fg_def, const WORD bg_def,
	const uTCHAR *font_family, const int font_size, _dos_text_scroll *scroll, const uTCHAR *fmt, ...);

EXTERNC void dos_text_curtain(BYTE nidx, int ppu_x, int ppu_y, _dos_text_curtain *curtain, BYTE mode);
EXTERNC void dos_text_curtain_add_line(_dos_text_curtain *curtain, const WORD fg_def, const WORD bg_def,
	const uTCHAR *font_family, const int font_size, const uTCHAR *fmt, ...);

EXTERNC void dos_text_pixels_size(int *w, int *h, const uTCHAR *font_family, int font_size, const uTCHAR *txt);
EXTERNC int dos_text_pixels_w(const uTCHAR *font_family, const int font_size, const uTCHAR *txt);
EXTERNC int dos_text_pixels_h(const uTCHAR *font_family, const int font_size, const uTCHAR *txt);

EXTERNC void dos_vline(BYTE nidx, int ppu_x, int ppu_y, int h, WORD color);
EXTERNC void dos_hline(BYTE nidx, int ppu_x, int ppu_y, int w, WORD color);
EXTERNC void dos_box(BYTE nidx, int ppu_x, int ppu_y, int w, int h, WORD color1, WORD color2, WORD bck);

EXTERNC void dos_image(BYTE nidx, int ppu_x, int ppu_y, int rect_x, int rect_y, int rect_w, int rect_h,
	const uTCHAR *resource, WORD *ppu_image, uint32_t pitch);

EXTERNC void dos_draw_ppu_image(BYTE nidx, int ppu_x, int ppu_y, int rect_x, int rect_y, int rect_w, int rect_h,
	WORD *ppu_image);

EXTERNC int dos_resource_w(const uTCHAR *resource);
EXTERNC int dos_resource_h(const uTCHAR *resource);
EXTERNC void dos_resource_size(int *w, int *h, const uTCHAR *resource);

EXTERNC uTCHAR *dos_tag_desc_from_value(const WORD value);
EXTERNC WORD dos_tag_value(const uTCHAR *desc);
EXTERNC WORD dos_tag_value_from_opt(const signed long long opt);

#undef EXTERNC

#endif /* DRAW_ON_SCREEN_H_ */
