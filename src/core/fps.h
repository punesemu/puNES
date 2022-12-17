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

#ifndef FPS_H_
#define FPS_H_

#include "common.h"

enum ff_velocity_values { FF_2X = 2, FF_3X, FF_4X, FF_5X, FF_MAX_SPEED = FF_5X };

#define fps_machine_ms(factor)\
	machine.ms_frame = fps.frame.estimated_ms = (1000.0f / (double)machine.fps) * factor
#define fps_fast_forward_enabled()\
	(fps.fast_forward | fps.max_speed)

typedef struct _fps {
	double gfx;
	BYTE fast_forward;
	BYTE max_speed;
	struct _fps_frame {
		double estimated_ms;
		double expected_end;
	} frame;
	struct _fps_info {
		uint32_t skipped;
		uint32_t emu_too_long;
	} info;
} _fps;

extern _fps fps;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void fps_init(void);

EXTERNC void fps_fast_forward_estimated_ms(void);
EXTERNC void fps_fast_forward_start(void);
EXTERNC void fps_fast_forward_stop(void);

EXTERNC void fps_max_speed_estimated_ms(void);
EXTERNC void fps_max_speed_start(void);
EXTERNC void fps_max_speed_stop(void);

#undef EXTERNC

#endif /* FPS_H_ */
