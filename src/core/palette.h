/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef PALETTE_H_
#define PALETTE_H_

#define NUM_COLORS 512

enum palettes_types {
	PALETTE_PAL,
	PALETTE_NTSC,
	PALETTE_SONY,
	PALETTE_MONO,
	PALETTE_GREEN,
	PALETTE_FILE,
	PALETTE_FRBX_NOSTALGIA,
	PALETTE_FRBX_YUV
};

typedef struct _color_RGB {
	BYTE r;
	BYTE g;
	BYTE b;
} _color_RGB;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _color_RGB palette_base_file[64];
EXTERNC struct _palette_RGB {
	_color_RGB *in_use;
	_color_RGB noswap[NUM_COLORS];
	_color_RGB swapped[NUM_COLORS];
} palette_RGB;

EXTERNC void palette_save_on_file(const uTCHAR *file);
EXTERNC BYTE palette_load_from_file(const uTCHAR *file);

#undef EXTERNC

#endif /* PALETTE_H_ */
