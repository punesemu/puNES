/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef NTSC_H_
#define NTSC_H_

#include "video/filters/nes_ntsc.h"
#include "common.h"
#include "video/gfx.h"
#include "palette.h"

enum ntsc_mode { COMPOSITE, SVIDEO, RGBMODE };

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC gfx_filter_function(ntsc_surface);
EXTERNC BYTE ntsc_init(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in,
		BYTE *palette_out);
EXTERNC void ntsc_quit(void);
EXTERNC void ntsc_set(nes_ntsc_t *ntsc_in, BYTE effect, BYTE color, BYTE *palette_base,
		BYTE *palette_in, BYTE *palette_out);

#undef EXTERNC

#endif /* NTSC_H_ */
