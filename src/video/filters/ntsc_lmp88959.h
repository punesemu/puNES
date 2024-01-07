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

#ifndef NTSC_LMP88959_H_
#define NTSC_LMP88959_H_

#include "common.h"

typedef struct _ntsc_lmp88959_setup_t {
	int hue;
	int saturation;
	int contrast;
	int brightness;
	int black_point;
	int white_point;
	int noise;
	int merge_fields;
	int vertical_blend;
	int scanline;
} _ntsc_lmp88959_setup_t;
typedef struct _lmp88959_thread_phospohor {
	uint32_t *pix;
	int width;
	int height;
} _lmp88959_thread_phospohor;

extern _ntsc_lmp88959_setup_t ntsc_lmp88959;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE ntsc_lmp88959_init(void);
EXTERNC void ntsc_lmp88959_surface(BYTE nidx);

EXTERNC void ntsc_lmp88959_filter_parameters_changed(void);
EXTERNC void ntsc_lmp88959_filter_parameters_default(void);

EXTERNC void ntsc_lmp88959_filter_parameter_default(int index);
EXTERNC void ntsc_lmp88959_filter_parameter_smv_default(void);

EXTERNC void lmp88959_phosphor_decay(void);

#undef EXTERNC

#endif /* NTSC_LMP88959_H_ */
