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
#include "sdlsnd.h"
#include "cfgfile.h"

void fpsInit(void) {
	switch (cfg->fps) {
		case FPSDEFAULT:
			if (machine.type == NTSC) {
				machine.fps = 60;
			} else {
				machine.fps = 50;
			}
			break;
		case FPS60:
			machine.fps = 60;
			break;
		case FPS59:
			machine.fps = 59;
			break;
		case FPS58:
			machine.fps = 58;
			break;
		case FPS57:
			machine.fps = 57;
			break;
		case FPS56:
			machine.fps = 56;
			break;
		case FPS55:
			machine.fps = 55;
			break;
		case FPS54:
			machine.fps = 54;
			break;
		case FPS53:
			machine.fps = 53;
			break;
		case FPS52:
			machine.fps = 52;
			break;
		case FPS51:
			machine.fps = 51;
			break;
		case FPS50:
			machine.fps = 50;
			break;
		case FPS49:
			machine.fps = 49;
			break;
		case FPS48:
			machine.fps = 48;
			break;
		case FPS47:
			machine.fps = 47;
			break;
		case FPS46:
			machine.fps = 46;
			break;
		case FPS45:
			machine.fps = 45;
			break;
		case FPS44:
			machine.fps = 44;
			break;
	}

	machine.msFrame = 1000.0 / (double) machine.fps;

	memset (&fps, 0, sizeof(fps));

	fps.nominal = 1000.0 / machine.msFrame;
	fps.avarage = fps.nominal;
	fpsNormalize();
	inputTurboButtonsFrequency();
}
void fpsFastForward(void) {
	fps.fastforward = TRUE;
	fps.frames_before_skip = 1;
	fps.max_frames_skipped = machine.fps / 3;
	fps.ms = (int) (1000 / (machine.fps * 2));
}
void fpsNormalize(void) {
	fps.frames_before_skip = 1;
	if (cfg->frameskip == 0) {
		fps.max_frames_skipped = machine.fps;
	} else {
		fps.max_frames_skipped = cfg->frameskip;
	}
	fps.ms = machine.msFrame;
	fps.fastforward = FALSE;
}
void fpsFrameskip(void) {
	double diff;
	double frame_end;

	frame_end = guiGetMs();

	ppu.skipDraw = FALSE;

	diff = (frame_end - fps.next_frame);

	if (diff < 0) {
		guiSleep(fps.next_frame - frame_end);
	}

	fps.next_frame += fps.ms;

	if (diff >= (machine.msFrame * fps.frames_before_skip)) {
		if (fps.frames_skipped >= fps.max_frames_skipped) {
			fps.next_frame = guiGetMs();
			fps.frames_skipped = 0;
		} else {
			fps.frames_skipped++;
			fps.total_frames_skipped++;
			ppu.skipDraw = TRUE;
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
