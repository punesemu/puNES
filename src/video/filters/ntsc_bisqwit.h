/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef NTSC_BISQWIT_H_
#define NTSC_BISQWIT_H_

#include "common.h"

typedef struct _ntsc_bisqwit_setup_t {
	double hue;        /* -1 = -180 degrees     +1 = +180 degrees */
	double saturation; /* -1 = grayscale (0.0)  +1 = oversaturated colors (2.0) */
	double contrast;   /* -1 = dark (0.5)       +1 = light (1.5) */
	double brightness; /* -1 = dark (0.5)       +1 = light (1.5) */
	int ywidth;
	int iwidth;
	int qwidth;
	int merge_fields;
	int vertical_blend;
	double scanline_intensity;
} _ntsc_bisqwit_setup_t;

extern _ntsc_bisqwit_setup_t ntsc_bisqwit;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void ntsc_bisqwit_init(void);
EXTERNC void ntsc_bisqwit_surface(BYTE nidx);

EXTERNC void ntsc_bisqwit_filter_parameters_changed(void);
EXTERNC void ntsc_bisqwit_filter_parameters_default(void);
EXTERNC void ntsc_bisqwit_filter_parameter_default(int index);
EXTERNC void ntsc_bisqwit_filter_parameter_mv_default(void);

#undef EXTERNC

#endif /* NTSC_BISQWIT_H_ */
