/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef FPS_H_
#define FPS_H_

#include "common.h"

enum ff_velocity_values { FF_2X = 2, FF_3X, FF_4X, FF_5X };

#define fps_machine_ms(factor)\
	machine.ms_frame = fps.frame.estimated_ms = (1000.0f / (double)machine.fps) * factor;

typedef struct _fps {
	double gfx;
	uint8_t fast_forward;
	uint32_t frames_skipped;
	uint32_t frames_emu_too_long;
	struct _frame {
		double estimated_ms;
		double expected_end;
	} frame;
} _fps;

extern _fps fps;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void fps_init(void);
EXTERNC void fps_fast_forward(void);
EXTERNC void fps_normalize(void);

#undef EXTERNC

#endif /* FPS_H_ */
