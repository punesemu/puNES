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

#ifndef FPS_H_
#define FPS_H_

#include "common.h"

enum fps_values {
	FPS_DEFAULT,
	FPS_60,
	FPS_59,
	FPS_58,
	FPS_57,
	FPS_56,
	FPS_55,
	FPS_54,
	FPS_53,
	FPS_52,
	FPS_51,
	FPS_50,
	FPS_49,
	FPS_48,
	FPS_47,
	FPS_46,
	FPS_45,
	FPS_44
};
enum ff_velocity_values { FF_2X = 2, FF_3X, FF_4X, FF_5X };

#define fps_machine_ms(factor)\
	machine.ms_frame = fps.ms = (1000.0f / (double) machine.fps) * factor;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _fps {
	uint8_t fast_forward;
	int counter;
	int frames_before_skip;
	int max_frames_skipped;
	int frames_skipped;
	uint32_t total_frames_skipped;
	double ms;
	double next_frame;
	double second_start;
	double second_end;
	double avarage;
	double nominal;
} fps;
EXTERNC struct _framerate {
	uint32_t interval;
	double value;
	double last_time;
} framerate;

EXTERNC void fps_init(void);
EXTERNC void fps_fast_forward(void);
EXTERNC void fps_normalize(void);
EXTERNC void fps_frameskip(void);

#undef EXTERNC

#endif /* FPS_H_ */
