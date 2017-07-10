/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "snd.h"
#include "ppu.h"
#include "conf.h"
#include "gui.h"

void fps_init(void) {
	switch (cfg->fps) {
		case FPS_DEFAULT:
			if (machine.type == NTSC) {
				machine.fps = 60;
			} else {
				machine.fps = 50;
			}
			break;
		case FPS_60:
			machine.fps = 60;
			break;
		case FPS_59:
			machine.fps = 59;
			break;
		case FPS_58:
			machine.fps = 58;
			break;
		case FPS_57:
			machine.fps = 57;
			break;
		case FPS_56:
			machine.fps = 56;
			break;
		case FPS_55:
			machine.fps = 55;
			break;
		case FPS_54:
			machine.fps = 54;
			break;
		case FPS_53:
			machine.fps = 53;
			break;
		case FPS_52:
			machine.fps = 52;
			break;
		case FPS_51:
			machine.fps = 51;
			break;
		case FPS_50:
			machine.fps = 50;
			break;
		case FPS_49:
			machine.fps = 49;
			break;
		case FPS_48:
			machine.fps = 48;
			break;
		case FPS_47:
			machine.fps = 47;
			break;
		case FPS_46:
			machine.fps = 46;
			break;
		case FPS_45:
			machine.fps = 45;
			break;
		case FPS_44:
			machine.fps = 44;
			break;
	}

	memset(&fps, 0x00, sizeof(fps));
	memset(&framerate, 0x00, sizeof(framerate));

	if (fps.fast_forward == FALSE) {
		fps_machine_ms(1.0)
	}

	fps.nominal = 1000.0f / machine.ms_frame;
	fps.avarage = fps.nominal;
	fps_normalize();

	framerate.interval = fps.nominal / 4;
}
void fps_fast_forward(void) {
	fps.fast_forward = TRUE;
	fps.frames_before_skip = 1;
	fps.max_frames_skipped = (machine.fps * cfg->ff_velocity) / 15;
	fps.ms = (int) (1000 / (machine.fps * cfg->ff_velocity));
}
void fps_normalize(void) {
	fps.frames_before_skip = 1;
	if (cfg->frameskip == 255) {
		fps.max_frames_skipped = machine.fps;
	} else {
		fps.max_frames_skipped = cfg->frameskip;
	}
	fps.ms = machine.ms_frame;
	fps.fast_forward = FALSE;
}
void fps_frameskip(void) {
	double diff;
	double frame_end;

	frame_end = gui_get_ms();

	if ((ppu.frames % framerate.interval) == 0) {
		framerate.value = 1000.0f * ((double) framerate.interval / (frame_end - framerate.last_time));
		framerate.last_time = frame_end;
	}

	ppu.skip_draw = FALSE;

	diff = (frame_end - fps.next_frame);

	if (diff < 0) {
		gui_sleep(fps.next_frame - frame_end);
	}

	//fps.next_frame = gui_get_ms() + fps.ms;
	fps.next_frame += fps.ms;

	if (diff >= (machine.ms_frame * (double) fps.frames_before_skip)) {
		if (fps.frames_skipped >= fps.max_frames_skipped) {
			fps.next_frame = gui_get_ms();
			fps.frames_skipped = 0;
		} else {
			fps.frames_skipped++;
			fps.total_frames_skipped++;
			ppu.skip_draw = TRUE;
		}
	}

	if ((diff = frame_end - fps.second_start) >= 1000.0f) {
		fps.avarage = (fps.avarage + (double) fps.counter) / 2.0f;
		fps.second_start = gui_get_ms() - (diff - 1000.0f);
		fps.counter = 0;
	} else {
		fps.counter++;
	}
}
