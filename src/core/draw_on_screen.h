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

#ifndef DRAW_ON_SCREEN_H_
#define DRAW_ON_SCREEN_H_

#include "common.h"

enum dos_position {
	DOS_CENTER = 65000,
	DOS_LEFT,
	DOS_RIGHT,
	DOS_UP,
	DOS_DOWN
};
enum dos_tgs {
	DOS_NORMAL,
	DOS_RED,
	DOS_YELLOW,
	DOS_GREEN,
	DOS_CYAN,
	DOS_BROWN,
	DOS_BLUE,
	DOS_GRAY,
	DOS_BLACK,
	DOS_BACKGROUND_COLOR,
	DOS_BUTTON_LEFT,
	DOS_BUTTON_RIGHT,
	DOS_BUTTON_UP,
	DOS_BUTTON_DOWN,
	DOS_BUTTON_SELECT1,
	DOS_BUTTON_SELECT2,
	DOS_BUTTON_SELECT3,
	DOS_BUTTON_START1,
	DOS_BUTTON_START2,
	DOS_BUTTON_START3,
	DOS_BUTTON_A,
	DOS_BUTTON_B,
	DOS_FLOPPY,
	DOS_PLAY,
	DOS_NEXT,
	DOS_PREV,
	DOS_PAUSE,
	DOS_STOP,
	DOS_MOUSE_RIGHT,
	DOS_MOUSE_LEFT,
	DOS_MOUSE_VRC7A,
	DOS_MOUSE_VRC7B,
	DOS_MOUSE_VRC6A,
	DOS_MOUSE_VRC6B,
	DOS_MOUSE_MMC5A,
	DOS_MOUSE_MMC5B,
	DOS_MOUSE_N163A,
	DOS_MOUSE_N163B,
	DOS_MOUSE_S5B1,
	DOS_MOUSE_S5B2,
	DOS_MOUSE_FDS1,
	DOS_MOUSE_FDS2,
	DOS_TAGS
};

#define dos_text(x, y, ...) _dos_text(x, y, -1, -1, -1, -1, __VA_ARGS__);
#define dospf(a) (a * 8)

#if defined (_DOS_STATIC_)
#define doscolor(clr) dos_table[clr]

static WORD dos_table[] = {
	0x0030, 0x0026, 0x0038, 0x002A,
	0x002C, 0x0027, 0x0011, 0x0000,
	0x000D
};
#endif

void _dos_text(int x, int y, int l, int r, int b, int t, const char *fmt, ...);
int dos_strlen(const char *fmt, ...);
int dos_is_tag(const char *text, int *tag_founded);

void dos_vline(int x, int y, int h, WORD color);
void dos_hline(int x, int y, int w, WORD color);

void dos_box(int x, int y, int w, int h, WORD color1, WORD color2, WORD bck);

#endif /* DRAW_ON_SCREEN_H_ */
