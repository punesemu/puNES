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

#include <string.h>
#include "fps.h"
#include "clock.h"
#include "conf.h"
#include "ppu.h"

#define ff_estimated_ms()\
	fps.frame.estimated_ms = 1000.0f / (double)(machine.fps * cfg->ff_velocity)
#define max_estimated_ms()\
	fps.frame.estimated_ms = 1000.0f / (double)(machine.fps * FF_MAX_SPEED)

_fps fps;

void fps_init(void) {
	if (machine.type == NTSC) {
		machine.fps = 60;
	} else {
		machine.fps = 50;
	}

	memset(&fps.frame, 0x00, sizeof(fps.frame));
	memset(&fps.info, 0x00, sizeof(fps.info));
	fps.gfx = 0;

	if (fps_fast_forward_enabled() == FALSE) {
		fps_machine_ms(1.0f);
	}

	fps_fast_forward_stop();
	fps_max_speed_stop();
}

void fps_fast_forward_estimated_ms(void) {
	if (fps.fast_forward) {
		if (fps.max_speed == FALSE) {
			ff_estimated_ms();
		}
	} else if (fps.max_speed) {
		max_estimated_ms();
	} else {
		fps.frame.estimated_ms = machine.ms_frame;
	}
}
void fps_fast_forward_start(void) {
	if (fps.fast_forward) {
		return;
	}
	ppu_draw_screen_pause();
	fps.fast_forward = TRUE;
	fps_fast_forward_estimated_ms();
}
void fps_fast_forward_stop(void) {
	if (fps.fast_forward == FALSE) {
		return;
	}
	fps.fast_forward = FALSE;
	fps_fast_forward_estimated_ms();
	ppu_draw_screen_continue();
}

void fps_max_speed_estimated_ms(void) {
	if (fps.max_speed) {
		max_estimated_ms();
	} else if (fps.fast_forward) {
		ff_estimated_ms();
	} else {
		fps.frame.estimated_ms = machine.ms_frame;
	}
}
void fps_max_speed_start(void) {
	if (fps.max_speed) {
		return;
	}
	ppu_draw_screen_pause();
	fps.max_speed = TRUE;
	fps_max_speed_estimated_ms();
}
void fps_max_speed_stop(void) {
	if (fps.max_speed == FALSE) {
		return;
	}
	fps.max_speed = FALSE;
	fps_max_speed_estimated_ms();
	ppu_draw_screen_continue();
}
