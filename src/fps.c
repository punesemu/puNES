/*
 * fps.c
 *
 *  Created on: 13/dic/2011
 *      Author: fhorse
 */

#include <string.h>
#include "fps.h"
#include "clock.h"
#include "gui.h"
#include "ppu.h"
#include "snd.h"
#include "cfg_file.h"

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

	memset (&fps, 0, sizeof(fps));

	fps_machine_ms(1.0)

	fps.nominal = 1000.0 / machine.ms_frame;
	fps.avarage = fps.nominal;
	fps_normalize();
	input_turbo_buttons_frequency();
}
void fps_fast_forward(void) {
	fps.fast_forward = TRUE;
	fps.frames_before_skip = 1;
	fps.max_frames_skipped = machine.fps / 3;
	fps.ms = (int) (1000 / (machine.fps * 2));
}
void fps_normalize(void) {
	fps.frames_before_skip = 1;
	if (cfg->frameskip == 0) {
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

	frame_end = guiGetMs();

	ppu.skip_draw = FALSE;

	diff = (frame_end - fps.next_frame);

	if (diff < 0) {
		guiSleep(fps.next_frame - frame_end);
	}

	fps.next_frame += fps.ms;

	if (diff >= (machine.ms_frame * fps.frames_before_skip)) {
		if (fps.frames_skipped >= fps.max_frames_skipped) {
			fps.next_frame = guiGetMs();
			fps.frames_skipped = 0;
		} else {
			fps.frames_skipped++;
			fps.total_frames_skipped++;
			ppu.skip_draw = TRUE;
		}
	}

	if ((diff = frame_end - fps.second_start) >= 1000.0) {
		fps.avarage = (fps.avarage + (double) fps.counter) / 2.0;
		fps.second_start = guiGetMs() - (diff - 1000.0);
		fps.counter = 0;
	} else {
		fps.counter++;
	}
}
