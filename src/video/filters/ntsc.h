/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

enum ntsc_mode { COMPOSITE, SVIDEO, RGBMODE };

typedef struct _ntsc_filter {
	nes_ntsc_setup_t format[3];
	nes_ntsc_t *ntsc;
} _ntsc_filter;

extern _ntsc_filter ntsc_filter;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

extern nes_ntsc_setup_t ntsc_format[3];

EXTERNC BYTE ntsc_init(void);
EXTERNC void ntsc_quit(void);
EXTERNC void ntsc_set(nes_ntsc_t *ntsc, BYTE create_palette, BYTE color, const BYTE *palette_base, const BYTE *palette_in, BYTE *palette_out);
EXTERNC void ntsc_rgb_modifier(nes_ntsc_t *ntsc, BYTE *palette, SWORD min, SWORD red, SWORD green, SWORD blue);
EXTERNC void ntsc_filter_parameters_changed(void);
EXTERNC void ntsc_filter_parameters_default(void);
EXTERNC void ntsc_filter_parameter_default(int index);
EXTERNC void ntsc_filter_parameter_mv_default(void);
EXTERNC void ntsc_surface(BYTE nidx);

#undef EXTERNC

#endif /* NTSC_H_ */
